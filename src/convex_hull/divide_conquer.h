#pragma once
#include "../common/geometry.h"
#include <algorithm>
#include <vector>

// Διαίρει & Βασίλευε — κρατάμε χωριστά το άνω και κάτω hull (x-monotone)
// ώστε να αποφύγουμε προβλήματα με κυκλικούς δείκτες στο bridge finding

namespace dc_internal {

// άνω hull σε ήδη ταξινομημένη ακολουθία (αριστερά → δεξιά)
static std::vector<Point2D> upper_scan(const std::vector<Point2D>& pts) {
    std::vector<Point2D> h;
    for (auto& p : pts) {
        while (h.size() >= 2 && ccw(h[h.size()-2], h[h.size()-1], p) >= 0)
            h.pop_back();
        h.push_back(p);
    }
    return h;
}

// κάτω hull — ίδιο, αλλά αναποδοστρέφουμε την ccw συνθήκη
static std::vector<Point2D> lower_scan(const std::vector<Point2D>& pts) {
    std::vector<Point2D> h;
    for (auto& p : pts) {
        while (h.size() >= 2 && ccw(h[h.size()-2], h[h.size()-1], p) <= 0)
            h.pop_back();
        h.push_back(p);
    }
    return h;
}

// άνω κοινή εφαπτόμενη μεταξύ A (αριστερά) και B (δεξιά)
// ξεκινάμε από τα άκρα που γειτονεύουν και μετακινούμαστε μέχρι να σταθεροποιηθούμε
static std::pair<int,int> upper_bridge(const std::vector<Point2D>& A,
                                       const std::vector<Point2D>& B) {
    int a = (int)A.size() - 1;
    int b = 0;

    bool done = false;
    while (!done) {
        done = true;
        while (a > 0 && ccw(A[a-1], A[a], B[b]) >= 0) { a--; done = false; }
        while (b < (int)B.size()-1 && ccw(A[a], B[b], B[b+1]) >= 0) { b++; done = false; }
    }
    return {a, b};
}

// κάτω κοινή εφαπτόμενη — ίδια λογική, αντίστροφη συνθήκη ccw
static std::pair<int,int> lower_bridge(const std::vector<Point2D>& A,
                                       const std::vector<Point2D>& B) {
    int a = (int)A.size() - 1;
    int b = 0;

    bool done = false;
    while (!done) {
        done = true;
        while (a > 0 && ccw(A[a-1], A[a], B[b]) <= 0) { a--; done = false; }
        while (b < (int)B.size()-1 && ccw(A[a], B[b], B[b+1]) <= 0) { b++; done = false; }
    }
    return {a, b};
}

struct HullPair {
    std::vector<Point2D> upper; // left→right
    std::vector<Point2D> lower; // left→right
};

// συγχώνευση δύο hull-pairs: βρίσκουμε τις εφαπτόμενες και κόβουμε τα εσωτερικά σημεία
static HullPair merge(const HullPair& L, const HullPair& R) {
    auto [u_a, u_b] = upper_bridge(L.upper, R.upper);
    std::vector<Point2D> upper(L.upper.begin(), L.upper.begin() + u_a + 1);
    upper.insert(upper.end(), R.upper.begin() + u_b, R.upper.end());

    auto [l_a, l_b] = lower_bridge(L.lower, R.lower);
    std::vector<Point2D> lower(L.lower.begin(), L.lower.begin() + l_a + 1);
    lower.insert(lower.end(), R.lower.begin() + l_b, R.lower.end());

    return {upper, lower};
}

static HullPair dc(const std::vector<Point2D>& pts, int l, int r) {
    if (l == r) return {{pts[l]}, {pts[l]}};
    if (r - l == 1) return {{pts[l], pts[r]}, {pts[l], pts[r]}};

    int mid = (l + r) / 2;
    auto L = dc(pts, l, mid);
    auto R = dc(pts, mid+1, r);
    return merge(L, R);
}

} // namespace dc_internal

// επιστρέφει το κυρτό περίβλημα σε δεξιόστροφη σειρά
inline std::vector<Point2D> convex_hull_divide_conquer(std::vector<Point2D> pts) {
    int n = pts.size();
    if (n < 2) return pts;
    std::sort(pts.begin(), pts.end());
    pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
    n = pts.size();
    if (n < 2) return pts;

    auto hp = dc_internal::dc(pts, 0, n - 1);

    // ενώνουμε: άνω (α→δ) + κάτω ανεστραμμένο (χωρίς τα άκρα που ήδη έχουμε)
    std::vector<Point2D> hull = hp.upper;
    for (int i = (int)hp.lower.size() - 2; i >= 1; i--)
        hull.push_back(hp.lower[i]);

    return hull;
}
