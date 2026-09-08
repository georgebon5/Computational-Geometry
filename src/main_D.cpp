#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <cstdlib>
#include <string>

#include "geo_search/knn.h"

// ── Utilities ──────────────────────────────────────────────────────────────

template<typename Func>
static double time_ms(Func&& f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

using Pt = KNNTree::Point;

// 3 Gaussian clusters — τα κέντρα σχηματίζουν περίπου ισόπλευρο τρίγωνο
static std::vector<Pt> make_training(int per_class, unsigned seed) {
    std::mt19937 rng(seed);
    std::normal_distribution<double> n01(0.0, 1.0);
    const double cx[] = {-50.0,  50.0,   0.0};
    const double cy[] = {-40.0, -40.0,  55.0};
    const double sigma = 20.0;
    std::vector<Pt> pts;
    pts.reserve(3 * per_class);
    for (int cls = 0; cls < 3; cls++)
        for (int i = 0; i < per_class; i++)
            pts.push_back({cx[cls] + sigma * n01(rng),
                           cy[cls] + sigma * n01(rng), cls});
    return pts;
}

// τυχαία σημεία για τα benchmarks (χωρίς clusters)
static std::vector<Pt> make_random(int n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> u(-90.0, 90.0);
    std::vector<Pt> pts;
    pts.reserve(n);
    for (int i = 0; i < n; i++)
        pts.push_back({u(rng), u(rng), i % 3});
    return pts;
}

// ταξινόμηση με majority vote μεταξύ k κοντινότερων γειτόνων
static int predict(const KNNTree& tree, double qx, double qy, int k) {
    auto nbrs = tree.knn(qx, qy, k);
    int votes[3] = {};
    for (int ni : nbrs) votes[tree.nodes[ni].label]++;
    return (int)(std::max_element(votes, votes + 3) - votes);
}

// γραμμική σάρωση για σύγκριση — O(N) ανά ερώτημα
static int predict_linear(const std::vector<Pt>& train,
                           double qx, double qy, int k) {
    using E = std::pair<double, int>;
    std::priority_queue<E> heap;
    for (int i = 0; i < (int)train.size(); i++) {
        double d2 = (train[i].x - qx)*(train[i].x - qx)
                  + (train[i].y - qy)*(train[i].y - qy);
        if ((int)heap.size() < k) heap.push({d2, i});
        else if (d2 < heap.top().first) { heap.pop(); heap.push({d2, i}); }
    }
    int votes[3] = {};
    while (!heap.empty()) {
        votes[train[heap.top().second].label]++;
        heap.pop();
    }
    return (int)(std::max_element(votes, votes + 3) - votes);
}

// ── Main ───────────────────────────────────────────────────────────────────

int main() {
    system("mkdir -p results");

    std::cout << "=== Υλοποίηση Δ: Ταξινόμηση k-Πλησιέστερων Γειτόνων (k-NN) ===\n\n";
    std::cout << "Εφαρμογή: Ταξινόμηση 2D σημείων σε 3 κλάσεις μέσω kd-tree k-NN\n";

    const int PER_CLASS = 100;   // training points per class
    const int N_QUERY   = 30;    // query points for classification demo
    const int K         = 5;     // nearest neighbours
    const int GRID_N    = 81;    // grid resolution for decision-boundary plot

    // ── Training data ─────────────────────────────────────────────────────
    auto train = make_training(PER_CLASS, 42);

    // Write training points split by class
    {
        std::ofstream tf[3];
        for (int c = 0; c < 3; c++)
            tf[c].open("results/knn_train_c" + std::to_string(c) + ".dat");
        for (auto& p : train)
            tf[p.label] << p.x << " " << p.y << "\n";
    }

    // ── Build kd-tree ─────────────────────────────────────────────────────
    KNNTree tree;
    tree.build(train);

    std::cout << "Σύνολο εκπαίδευσης : " << train.size()
              << " σημεία (100 ανά κλάση)\n";
    std::cout << "Παράμετρος k        : " << K << "\n\n";

    // ── Classify N_QUERY random query points ──────────────────────────────
    std::mt19937 rng_q(123);
    std::uniform_real_distribution<double> u_q(-85.0, 85.0);

    std::vector<std::pair<double,double>> queries;
    queries.reserve(N_QUERY);
    for (int i = 0; i < N_QUERY; i++)
        queries.push_back({u_q(rng_q), u_q(rng_q)});

    {
        std::ofstream qf[3];
        for (int c = 0; c < 3; c++)
            qf[c].open("results/knn_query_c" + std::to_string(c) + ".dat");

        int qcnt[3] = {};
        for (auto& q : queries) {
            int pred = predict(tree, q.first, q.second, K);
            qf[pred] << q.first << " " << q.second << "\n";
            qcnt[pred]++;
        }

        std::cout << "Ταξινόμηση " << N_QUERY << " ερωτημάτων:\n";
        std::cout << "  → Κλάση 0 : " << qcnt[0] << "\n";
        std::cout << "  → Κλάση 1 : " << qcnt[1] << "\n";
        std::cout << "  → Κλάση 2 : " << qcnt[2] << "\n\n";
    }

    // ── Decision-boundary grid ────────────────────────────────────────────
    std::cout << "Υπολογισμός ορίου απόφασης ("
              << GRID_N << "×" << GRID_N << " = "
              << GRID_N*GRID_N << " σημεία)...\n";

    {
        std::ofstream gf[3];
        for (int c = 0; c < 3; c++)
            gf[c].open("results/knn_grid_c" + std::to_string(c) + ".dat");

        for (int i = 0; i < GRID_N; i++) {
            double x = -100.0 + i * 200.0 / (GRID_N - 1);
            for (int j = 0; j < GRID_N; j++) {
                double y = -100.0 + j * 200.0 / (GRID_N - 1);
                gf[predict(tree, x, y, K)] << x << " " << y << "\n";
            }
        }
    }

    // ── Timing comparison: kd-tree vs linear scan ─────────────────────────
    std::cout << "\n=== Σύγκριση χρόνων εκτέλεσης (ms) ===\n";
    std::cout << "(100 ερωτήματα, k=" << K
              << ", μέσος όρος 3 επαναλήψεων)\n\n";

    const int M_TIMING = 100;
    const int REPS     = 3;
    const std::vector<int> sizes = {500, 1000, 5000, 10000, 50000};

    std::cout << std::left
              << std::setw(9)  << "N"
              << std::setw(18) << "Κατ. kd-tree"
              << std::setw(18) << "k-NN kd-tree"
              << std::setw(18) << "Γραμ. σάρωση"
              << "\n"
              << std::string(63, '-') << "\n";

    std::ofstream timing_file("results/knn_timing.dat");
    timing_file << "# N  build_kd  kd_query  linear_query\n";

    for (int N : sizes) {
        auto t_train = make_random(N, N * 3 + 7);

        std::mt19937 rng_t(55555);
        std::uniform_real_distribution<double> u_t(-85.0, 85.0);
        std::vector<std::pair<double,double>> t_queries;
        t_queries.reserve(M_TIMING);
        for (int i = 0; i < M_TIMING; i++)
            t_queries.push_back({u_t(rng_t), u_t(rng_t)});

        double tb = 0, tkd = 0, tlin = 0;
        KNNTree t;
        for (int r = 0; r < REPS; r++) {
            tb  += time_ms([&]{ t.build(t_train); });
            tkd += time_ms([&]{
                for (auto& q : t_queries)
                    predict(t, q.first, q.second, K);
            });
            tlin += time_ms([&]{
                for (auto& q : t_queries)
                    predict_linear(t_train, q.first, q.second, K);
            });
        }
        tb /= REPS; tkd /= REPS; tlin /= REPS;

        std::cout << std::left
                  << std::setw(9) << N
                  << std::fixed << std::setprecision(3)
                  << std::setw(18) << tb
                  << std::setw(18) << tkd
                  << std::setw(18) << tlin
                  << "\n";

        timing_file << N << " " << tb << " " << tkd << " " << tlin << "\n";
    }
    timing_file.close();
    std::cout << "\n";

    // ── Gnuplot ───────────────────────────────────────────────────────────
    int r1 = system("gnuplot plot/plot_knn.gp 2>/dev/null");
    int r2 = system("gnuplot plot/plot_knn_timing.gp 2>/dev/null");

    if (r1 == 0) std::cout << "Αποθηκεύτηκε: results/knn.png\n";
    if (r2 == 0) std::cout << "Αποθηκεύτηκε: results/knn_timing.png\n";

    std::cout << "\nΟλοκληρώθηκε.\n";
    return 0;
}
