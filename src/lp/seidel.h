#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <optional>
#include <limits>

// ημιεπίπεδο της μορφής ax + by <= c
struct HalfPlane {
    double a, b, c;
    int    id = 0;
};

struct LPResult {
    bool   feasible  = false;
    double x         = 0;
    double y         = 0;
    double obj       = 0;   // value of cx*x + cy*y at optimum
    int    lp1d_calls = 0;  // πόσες φορές λύσαμε το 1D υποπρόβλημα
};

static constexpr double LP_EPS = 1e-9;

inline bool hp_ok(const HalfPlane& h, double x, double y) {
    return h.a * x + h.b * y <= h.c + LP_EPS;
}

// 1D LP: ελαχιστοποίηση του αντικειμενικού επί της ευθείας p0 + t*(dx,dy)
// Προβάλλουμε τα ημιεπίπεδα στο t και βρίσκουμε το [lo, hi] που τα ικανοποιεί
static std::optional<double>
lp_1d(double p0x, double p0y, double dx, double dy,
      double cx,  double cy,
      const std::vector<HalfPlane>& active, int n)
{
    double lo = -1e15, hi = 1e15;

    for (int j = 0; j < n; ++j) {
        const HalfPlane& h = active[j];
        double coef = h.a * dx + h.b * dy;
        double rhs  = h.c  - h.a * p0x - h.b * p0y;

        if (std::abs(coef) < LP_EPS) {
            if (rhs < -LP_EPS) return std::nullopt; // αδύνατο ανεξάρτητα από t
        } else if (coef > 0) {
            hi = std::min(hi, rhs / coef);
        } else {
            lo = std::max(lo, rhs / coef);
        }
    }

    if (lo > hi + LP_EPS) return std::nullopt;

    double dir = cx * dx + cy * dy;
    double t;
    if      (dir >  LP_EPS) t = lo;  // ελαχιστοποίηση → παίρνουμε το μικρότερο t
    else if (dir < -LP_EPS) t = hi;
    else                    t = lo;

    return std::clamp(t, lo, hi);
}

// Αλγόριθμος Seidel — τυχαία διάταξη περιορισμών, αναμενόμενο O(n)
// Αν το τρέχον βέλτιστο παραβιάζει κάποιον περιορισμό, λύνουμε 1D LP στο όριό του
inline LPResult seidel_lp(double cx, double cy,
                           std::vector<HalfPlane> hs,
                           unsigned seed = 42)
{
    const double BIG = 1e9;

    // ξεκινάμε με bounding box που είναι πάντα εφικτό
    std::vector<HalfPlane> active = {
        { 1,  0, BIG, -1},
        {-1,  0, BIG, -2},
        { 0,  1, BIG, -3},
        { 0, -1, BIG, -4},
    };

    // αρχικό βέλτιστο: γωνία του bounding box που ελαχιστοποιεί cx*x + cy*y
    double x = (cx <= 0) ?  BIG : -BIG;
    double y = (cy <= 0) ?  BIG : -BIG;

    // τυχαία ανακατανομή → αναμενόμενο O(n) συνολικά
    std::mt19937 rng(seed);
    std::shuffle(hs.begin(), hs.end(), rng);

    LPResult res;
    res.feasible = true;

    for (const HalfPlane& h : hs) {
        if (hp_ok(h, x, y)) {
            active.push_back(h);
            continue;
        }

        // το τρέχον x* παραβιάζει τον h → το νέο βέλτιστο βρίσκεται στο όριο του h
        ++res.lp1d_calls;

        double dx = -h.b, dy = h.a; // διεύθυνση κατά μήκος του ορίου

        // ένα σημείο πάνω στο όριο
        double p0x, p0y;
        if (std::abs(h.a) >= std::abs(h.b)) {
            p0y = 0.0;  p0x = h.c / h.a;
        } else {
            p0x = 0.0;  p0y = h.c / h.b;
        }

        auto t = lp_1d(p0x, p0y, dx, dy, cx, cy, active, (int)active.size());
        if (!t) { res.feasible = false; return res; }

        x = p0x + *t * dx;
        y = p0y + *t * dy;
        active.push_back(h);
    }

    res.x   = x;
    res.y   = y;
    res.obj = cx * x + cy * y;
    return res;
}
