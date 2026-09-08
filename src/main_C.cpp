#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include "geo_search/kdtree.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<Point2D> random_points_2d(int n, unsigned seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    std::vector<Point2D> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i)
        pts.push_back({dist(rng), dist(rng)});
    return pts;
}

// Write splitting segments for one axis-parity (x-splits or y-splits)
static void write_splits(const char* path,
                         const std::vector<KDSplit>& splits, int axis_parity)
{
    FILE* f = fopen(path, "w");
    for (const auto& s : splits)
        if (s.depth % 2 == axis_parity)
            fprintf(f, "%.6f %.6f\n%.6f %.6f\n\n", s.x1, s.y1, s.x2, s.y2);
    fclose(f);
}

// Write points (one per line)
static void write_pts(const char* path, const std::vector<Point2D>& pts)
{
    FILE* f = fopen(path, "w");
    for (const auto& p : pts)
        fprintf(f, "%.6f %.6f\n", p.x, p.y);
    fclose(f);
}

// Write closed rectangle (5 points)
static void write_rect(const char* path, const KDRect& r)
{
    FILE* f = fopen(path, "w");
    fprintf(f, "%.4f %.4f\n%.4f %.4f\n%.4f %.4f\n%.4f %.4f\n%.4f %.4f\n",
            r.xlo, r.ylo, r.xhi, r.ylo,
            r.xhi, r.yhi, r.xlo, r.yhi,
            r.xlo, r.ylo);
    fclose(f);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    system("mkdir -p results");

    // ═══════════════════════════════════════════════════════════════════════
    // ΜΕΡΟΣ 1: Κατασκευή kd-tree σε 12 επιλεγμένα σημεία (οπτικοποίηση)
    // ═══════════════════════════════════════════════════════════════════════

    printf("=== Μέρος Γ: Γεωμετρική Αναζήτηση (kd-tree) ===\n\n");

    // 12 manually chosen points in [0,10]×[0,10] — spread to give a clear tree
    std::vector<Point2D> small_pts = {
        {1.0, 5.0}, {2.5, 2.0}, {3.0, 8.5}, {4.5, 3.5},
        {5.0, 4.5}, {5.5, 7.0}, {6.0, 1.5}, {7.0, 9.0},
        {8.0, 4.5}, {9.0, 6.5}, {2.0, 6.5}, {7.5, 7.5}
    };

    KDTree small_tree;
    small_tree.build(small_pts, {0, 10, 0, 10});

    printf("--- Παράδειγμα κατασκευής (%d σημεία) ---\n", (int)small_pts.size());
    printf("Κόμβοι δέντρου : %d\n", (int)small_tree.nodes.size());
    printf("Τομές (splits) : %d  (x-τομές: %d,  y-τομές: %d)\n\n",
           (int)small_tree.splits.size(),
           (int)std::count_if(small_tree.splits.begin(), small_tree.splits.end(),
                              [](const KDSplit& s){ return s.depth % 2 == 0; }),
           (int)std::count_if(small_tree.splits.begin(), small_tree.splits.end(),
                              [](const KDSplit& s){ return s.depth % 2 == 1; }));

    // Print tree structure
    printf("%-6s %-12s %-6s %-6s %-6s\n", "Node", "Point", "Axis", "Left", "Right");
    for (int i = 0; i < (int)small_tree.nodes.size(); ++i) {
        const auto& nd = small_tree.nodes[i];
        printf("%-6d (%-5.2f,%-5.2f) %-6s %-6d %-6d\n",
               i, nd.pt.x, nd.pt.y,
               nd.axis == 0 ? "x" : "y",
               nd.left, nd.right);
    }
    printf("\n");

    write_pts("results/kdtree_small_pts.dat",   small_pts);
    write_splits("results/kdtree_small_splits_x.dat", small_tree.splits, 0);
    write_splits("results/kdtree_small_splits_y.dat", small_tree.splits, 1);

    // ═══════════════════════════════════════════════════════════════════════
    // ΜΕΡΟΣ 2: Ορθογώνια αναζήτηση σε 150 τυχαία σημεία
    // ═══════════════════════════════════════════════════════════════════════

    auto pts150 = random_points_2d(150, 42);

    KDTree tree;
    tree.build(pts150);

    // Query rectangle
    const KDRect query = {-30.0, 60.0, -20.0, 70.0};

    int nodes_visited = 0;
    auto found = tree.range_query(query, &nodes_visited);

    printf("--- Ορθογώνιο ερώτημα σε 150 σημεία ---\n");
    printf("Σύνολο σημείων     : 150\n");
    printf("Ερώτημα            : x ∈ [%.0f, %.0f],  y ∈ [%.0f, %.0f]\n",
           query.xlo, query.xhi, query.ylo, query.yhi);
    printf("Σημεία εντός       : %d\n", (int)found.size());
    printf("Κόμβοι επισκέφθηκαν: %d  (σύνολο %d)\n",
           nodes_visited, (int)tree.nodes.size());
    printf("Θεωρητικό O(√n+k)  : %.1f + %d = %.1f\n\n",
           std::sqrt(150.0), (int)found.size(),
           std::sqrt(150.0) + found.size());

    printf("Σημεία εντός ορθογωνίου:\n");
    for (const auto& p : found)
        printf("  (%.4f, %.4f)\n", p.x, p.y);
    printf("\n");

    write_pts("results/search_all_pts.dat",   pts150);
    write_pts("results/search_found_pts.dat", found);
    write_rect("results/search_query.dat",    query);
    write_splits("results/search_splits_x.dat", tree.splits, 0);
    write_splits("results/search_splits_y.dat", tree.splits, 1);

    // ── Gnuplot ───────────────────────────────────────────────────────────
    int ok = system("which gnuplot > /dev/null 2>&1");
    if (ok == 0) {
        system("gnuplot plot/plot_kdtree.gp");
        system("gnuplot plot/plot_search.gp");
        printf("Γραφήματα: results/kdtree_construction.png\n");
        printf("           results/search_result.png\n");
    } else {
        printf("(gnuplot not found — skip plots)\n");
    }

    return 0;
}
