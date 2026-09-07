#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <set>

#include "convex_hull/incremental.h"
#include "convex_hull/gift_wrapping.h"
#include "convex_hull/divide_conquer.h"
#include "convex_hull/quickhull.h"
#include "convex_hull/ch3d.h"

// ── Utilities ──────────────────────────────────────────────────────────────

static std::vector<Point2D> random_points_2d(int n, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    std::vector<Point2D> pts;
    pts.reserve(n);
    // αποφεύγουμε διπλότυπα με quantization στο 1e-4
    std::set<std::pair<long long,long long>> seen;
    while ((int)pts.size() < n) {
        double x = dist(rng), y = dist(rng);
        long long qx = llround(x * 1e4), qy = llround(y * 1e4);
        if (seen.count({qx, qy})) continue;
        seen.insert({qx, qy});
        pts.push_back({x, y});
    }
    return pts;
}

static std::vector<Point3D> random_points_3d(int n, unsigned seed = 99) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    std::vector<Point3D> pts;
    pts.reserve(n);
    for (int i = 0; i < n; i++) {
        pts.push_back({dist(rng), dist(rng), dist(rng), i});
    }
    return pts;
}

static void write_points_2d(const std::string& path, const std::vector<Point2D>& pts) {
    std::ofstream f(path);
    for (auto& p : pts) f << p.x << " " << p.y << "\n";
}

static void write_hull_2d(const std::string& path, const std::vector<Point2D>& hull) {
    std::ofstream f(path);
    for (auto& p : hull) f << p.x << " " << p.y << "\n";
    // Close polygon
    if (!hull.empty()) f << hull[0].x << " " << hull[0].y << "\n";
}

static void write_points_3d(const std::string& path, const std::vector<Point3D>& pts) {
    std::ofstream f(path);
    for (auto& p : pts) f << p.x << " " << p.y << " " << p.z << "\n";
}

static void write_faces_3d(const std::string& path,
                           const std::vector<Point3D>& pts,
                           const std::vector<Face3D>& faces) {
    // γράφουμε τα τρίγωνα έτσι ώστε το gnuplot να τα αναγνωρίζει ως polygons
    std::ofstream f(path);
    for (auto& face : faces) {
        auto& a = pts[face.a]; auto& b = pts[face.b]; auto& c = pts[face.c];
        f << a.x << " " << a.y << " " << a.z << "\n";
        f << b.x << " " << b.y << " " << b.z << "\n";
        f << c.x << " " << c.y << " " << c.z << "\n";
        f << a.x << " " << a.y << " " << a.z << "\n";
        f << "\n\n"; // blank lines separate polygons
    }
}

template<typename Func>
double time_ms(Func&& f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// ── Main ───────────────────────────────────────────────────────────────────

int main() {
    // Create results directory
    system("mkdir -p results");

    // ── Part A.1-2: 150 random 2D points ─────────────────────────────────
    std::cout << "=== Υλοποίηση Α: Κυρτό Περίβλημα (2D) ===\n\n";

    auto pts150 = random_points_2d(150, 42);
    write_points_2d("results/points_2d.dat", pts150);

    auto hull_inc  = convex_hull_incremental(pts150);
    auto hull_gift = convex_hull_gift_wrapping(pts150);
    auto hull_dc   = convex_hull_divide_conquer(pts150);
    auto hull_qh   = convex_hull_quickhull(pts150);

    write_hull_2d("results/hull_incremental.dat",  hull_inc);
    write_hull_2d("results/hull_gift.dat",          hull_gift);
    write_hull_2d("results/hull_dc.dat",            hull_dc);
    write_hull_2d("results/hull_quickhull.dat",     hull_qh);

    std::cout << "Κορυφές κυρτού περιβλήματος (150 σημεία):\n";
    std::cout << "  Αυξητικός:         " << hull_inc.size()  << "\n";
    std::cout << "  Περιτυλίγματος:    " << hull_gift.size() << "\n";
    std::cout << "  Διαίρει & Βασίλ.:  " << hull_dc.size()   << "\n";
    std::cout << "  QuickHull:         " << hull_qh.size()   << "\n\n";

    // ── Part A.3: Timing comparison ───────────────────────────────────────
    std::cout << "=== Σύγκριση χρόνων εκτέλεσης (ms) ===\n\n";

    const int REPS = 5;
    std::vector<int> sizes = {100, 500, 1000, 5000, 10000, 50000};

    // Print table header
    std::cout << std::left
              << std::setw(8)  << "n"
              << std::setw(16) << "Αυξητικός"
              << std::setw(16) << "Περιτύλιγμα"
              << std::setw(16) << "Διαίρει&Βασ."
              << std::setw(16) << "QuickHull"
              << "\n";
    std::cout << std::string(72, '-') << "\n";

    // Also write timing data to file for potential plotting
    std::ofstream timing_file("results/timing.dat");
    timing_file << "# n incremental gift_wrapping divide_conquer quickhull\n";

    for (int sz : sizes) {
        auto pts = random_points_2d(sz, 7 * sz + 13);

        double t_inc = 0, t_gift = 0, t_dc = 0, t_qh = 0;
        for (int r = 0; r < REPS; r++) {
            auto p = pts;
            t_inc  += time_ms([&]{ convex_hull_incremental(p); });
            t_gift += time_ms([&]{ convex_hull_gift_wrapping(p); });
            t_dc   += time_ms([&]{ convex_hull_divide_conquer(p); });
            t_qh   += time_ms([&]{ convex_hull_quickhull(p); });
        }
        t_inc /= REPS; t_gift /= REPS; t_dc /= REPS; t_qh /= REPS;

        std::cout << std::left
                  << std::setw(8)  << sz
                  << std::setw(16) << std::fixed << std::setprecision(3) << t_inc
                  << std::setw(16) << t_gift
                  << std::setw(16) << t_dc
                  << std::setw(16) << t_qh
                  << "\n";

        timing_file << sz << " " << t_inc << " " << t_gift << " " << t_dc << " " << t_qh << "\n";
    }
    timing_file.close(); // κλείνουμε πριν το διαβάσει το gnuplot
    std::cout << "\n";

    // ── Part A.4-5: 3D Convex Hull ────────────────────────────────────────
    std::cout << "=== Υλοποίηση Α: Κυρτό Περίβλημα (3D) ===\n\n";

    auto pts3d = random_points_3d(150, 99);
    write_points_3d("results/points_3d.dat", pts3d);

    auto faces3d = convex_hull_3d(pts3d);
    write_faces_3d("results/faces_3d.dat", pts3d, faces3d);

    std::cout << "3D κυρτό περίβλημα (150 σημεία):\n";
    std::cout << "  Πλήθος εδρών: " << faces3d.size() << "\n\n";

    // ── Gnuplot visualization ─────────────────────────────────────────────
    int ret2d = system("gnuplot plot/plot_2d.gp 2>/dev/null");
    int ret3d = system("gnuplot plot/plot_3d.gp 2>/dev/null");
    int ret_timing = system("gnuplot plot/plot_timing.gp 2>/dev/null");

    if (ret2d == 0)     std::cout << "Αποθηκεύτηκε: results/convex_hull_2d.png\n";
    else                std::cout << "[Παράλειψη] gnuplot δεν βρέθηκε (2D)\n";
    if (ret3d == 0)     std::cout << "Αποθηκεύτηκε: results/convex_hull_3d.png\n";
    if (ret_timing == 0) std::cout << "Αποθηκεύτηκε: results/timing.png\n";

    std::cout << "\nΟλοκληρώθηκε.\n";
    return 0;
}
