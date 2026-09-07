#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <optional>
#include <string>
#include <cstdlib>
#include "lp/seidel.h"

// ── Helpers for feasible-region polygon ─────────────────────────────────────

static std::optional<std::pair<double,double>>
line_intersect(const HalfPlane& h1, const HalfPlane& h2)
{
    double det = h1.a * h2.b - h2.a * h1.b;
    if (std::abs(det) < 1e-10) return std::nullopt;
    double x = (h1.c * h2.b - h2.c * h1.b) / det;
    double y = (h1.a * h2.c - h2.a * h1.c) / det;
    return {{x, y}};
}

static bool all_satisfied(double x, double y, const std::vector<HalfPlane>& hs)
{
    for (const auto& h : hs)
        if (h.a * x + h.b * y > h.c + 1e-8) return false;
    return true;
}

// Returns vertices of the feasible polytope sorted CCW.
static std::vector<std::pair<double,double>>
feasible_vertices(const std::vector<HalfPlane>& hs)
{
    int n = (int)hs.size();
    std::vector<std::pair<double,double>> verts;

    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            auto p = line_intersect(hs[i], hs[j]);
            if (p && all_satisfied(p->first, p->second, hs))
                verts.push_back(*p);
        }

    // Remove near-duplicate points
    std::sort(verts.begin(), verts.end());
    verts.erase(std::unique(verts.begin(), verts.end(),
        [](const auto& a, const auto& b){
            return std::hypot(a.first - b.first, a.second - b.second) < 1e-6;
        }), verts.end());

    if (verts.size() < 2) return verts;

    // Sort CCW by angle from centroid
    double cx = 0, cy = 0;
    for (auto [vx, vy] : verts) { cx += vx; cy += vy; }
    cx /= (double)verts.size();
    cy /= (double)verts.size();

    std::sort(verts.begin(), verts.end(), [cx, cy](const auto& a, const auto& b){
        return std::atan2(a.second - cy, a.first - cx)
             < std::atan2(b.second - cy, b.first - cx);
    });
    return verts;
}

// ── Constraint segment clipped to plot window ────────────────────────────────

static void write_constraint_segment(FILE* f, const HalfPlane& h,
                                     double xlo, double xhi)
{
    if (std::abs(h.b) > 1e-9) {
        double y1 = (h.c - h.a * xlo) / h.b;
        double y2 = (h.c - h.a * xhi) / h.b;
        fprintf(f, "%.6f %.6f\n%.6f %.6f\n\n", xlo, y1, xhi, y2);
    } else if (std::abs(h.a) > 1e-9) {
        double xv = h.c / h.a;
        // Write a vertical segment tall enough for gnuplot to clip
        fprintf(f, "%.6f %.6f\n%.6f %.6f\n\n", xv, -100.0, xv, 100.0);
    }
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    system("mkdir -p results");

    // ── Problem definition ───────────────────────────────────────────────────
    //
    //   max{ -5x₁ + 12x₂ }  ≡  min{ 5x₁ - 12x₂ }
    //
    //   Subject to (ax + by ≤ c):
    //     (1)  -x  +  3y ≤   3
    //     (2)  -6x -   y ≤  -8   (6x + y ≥ 8)
    //     (3)  -2x +  7y ≤  25
    //     (4)  -x  +  8y ≤  31
    //     (5)  -x  +   y ≤ -12   (x - y ≥ 12)
    //     (6)   x  -  6y ≤   2
    //     (7)   3x - 12y ≤  -6   (x - 4y ≥ 2)
    //     (8)  -x        ≤   0   (x ≥ 0)
    //     (9)       -y   ≤   0   (y ≥ 0)
    //
    //   Known optimum: x₁* = 127/7 ≈ 18.1429,  x₂* = 43/7 ≈ 6.1429
    //   Active constraints at optimum: (4) and (5).

    std::vector<HalfPlane> hs = {
        {-1,  3,   3, 1},
        {-6, -1,  -8, 2},
        {-2,  7,  25, 3},
        {-1,  8,  31, 4},
        {-1,  1, -12, 5},
        { 1, -6,   2, 6},
        { 3,-12,  -6, 7},
        {-1,  0,   0, 8},
        { 0, -1,   0, 9},
    };

    // ── Seidel's algorithm: minimise 5x - 12y ───────────────────────────────
    auto res = seidel_lp(5.0, -12.0, hs, /*seed=*/42);

    printf("=== Μέρος Β: Γραμμικός Προγραμματισμός (Αλγόριθμος Seidel) ===\n\n");
    printf("Πρόβλημα: max{ -5x₁ + 12x₂ }  ≡  min{ 5x₁ - 12x₂ }\n");
    printf("         %d περιορισμοί (halfplanes)\n\n", (int)hs.size());

    if (!res.feasible) {
        printf("Αποτέλεσμα: ΜΗ ΕΦΙΚΤΟ\n");
        return 1;
    }

    printf("Αποτέλεσμα:\n");
    printf("  Βέλτιστο: x* = (%.8f, %.8f)\n", res.x, res.y);
    printf("  min{ 5x - 12y }  = %.8f\n", res.obj);
    printf("  max{-5x + 12y }  = %.8f\n", -res.obj);
    printf("  Αναφορά (ακριβές): x* = (127/7, 43/7) = (%.8f, %.8f)\n",
           127.0/7, 43.0/7);
    printf("  Αναφορά (αντικ.):  max = -119/7         = %.8f\n", -119.0/7);
    printf("  Επαναλήψεις 1D LP: %d\n\n", res.lp1d_calls);

    // ── Feasible-region vertices ─────────────────────────────────────────────
    auto verts = feasible_vertices(hs);

    printf("Κορυφές εφικτής περιοχής (%d κορυφές):\n", (int)verts.size());
    for (auto [vx, vy] : verts)
        printf("  (%.6f,  %.6f)\n", vx, vy);
    printf("\n");

    // ── Write data files ─────────────────────────────────────────────────────
    const double XPLOT_LO = 12.0, XPLOT_HI = 32.0;

    // Feasible-region polygon (closed)
    {
        FILE* f = fopen("results/lp_region.dat", "w");
        for (auto [vx, vy] : verts)
            fprintf(f, "%.8f  %.8f\n", vx, vy);
        if (!verts.empty())
            fprintf(f, "%.8f  %.8f\n", verts[0].first, verts[0].second);
        fclose(f);
    }

    // Constraint boundary lines (segments within [XPLOT_LO, XPLOT_HI])
    {
        FILE* f = fopen("results/lp_constraints.dat", "w");
        for (const auto& h : hs)
            write_constraint_segment(f, h, XPLOT_LO, XPLOT_HI);
        fclose(f);
    }

    // Objective-function contour lines:  -5x + 12y = k  →  y = (k + 5x)/12
    {
        FILE* f = fopen("results/lp_objectives.dat", "w");
        for (double k : {-17.0, -27.3, -37.5, -48.0}) {
            double y1 = (k + 5.0 * XPLOT_LO) / 12.0;
            double y2 = (k + 5.0 * XPLOT_HI) / 12.0;
            fprintf(f, "%.6f  %.6f\n%.6f  %.6f\n\n", XPLOT_LO, y1, XPLOT_HI, y2);
        }
        fclose(f);
    }

    // Optimal point
    {
        FILE* f = fopen("results/lp_optimal.dat", "w");
        fprintf(f, "%.8f  %.8f\n", res.x, res.y);
        fclose(f);
    }

    // ── Gnuplot ──────────────────────────────────────────────────────────────
    int gret = system("which gnuplot > /dev/null 2>&1 "
                      "&& gnuplot plot/plot_lp.gp "
                      "|| echo 'gnuplot not found'");
    if (gret == 0)
        printf("Γράφημα: results/lp.png\n");

    return 0;
}
