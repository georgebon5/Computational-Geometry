#pragma once
#include "../common/geometry.h"
#include <algorithm>
#include <vector>
#include <cmath>

namespace qh_internal {

// το πιο απομακρυσμένο σημείο από την ευθεία AB (δεξιά πλευρά, ccw < 0)
int farthest(const std::vector<Point2D>& pts,
             const std::vector<int>& indices,
             const Point2D& A, const Point2D& B) {
    int best = -1;
    long double bestDist = 0;
    for (int i : indices) {
        long double d = -ccw(A, B, pts[i]);
        if (d > bestDist) { bestDist = d; best = i; }
    }
    return best;
}

void quickhull_rec(const std::vector<Point2D>& pts,
                   const std::vector<int>& indices,
                   const Point2D& A, const Point2D& B,
                   std::vector<Point2D>& hull) {
    if (indices.empty()) return;

    int ci = farthest(pts, indices, A, B);
    if (ci == -1) return;

    const Point2D& C = pts[ci];

    // διαχωρισμός σημείων: δεξιά του A→C και δεξιά του C→B
    std::vector<int> leftAC, leftCB;
    for (int i : indices) {
        if (i == ci) continue;
        if (ccw(A, C, pts[i]) < 0) leftAC.push_back(i);
        if (ccw(C, B, pts[i]) < 0) leftCB.push_back(i);
    }

    quickhull_rec(pts, leftAC, A, C, hull);
    hull.push_back(C);
    quickhull_rec(pts, leftCB, C, B, hull);
}

} // namespace qh_internal

// QuickHull — χωρίζουμε στα δύο με βάση το αριστερότερο/δεξιότερο σημείο
inline std::vector<Point2D> convex_hull_quickhull(const std::vector<Point2D>& pts) {
    int n = pts.size();
    if (n < 3) return pts;

    int minIdx = 0, maxIdx = 0;
    for (int i = 1; i < n; i++) {
        if (pts[i].x < pts[minIdx].x || (pts[i].x == pts[minIdx].x && pts[i].y < pts[minIdx].y))
            minIdx = i;
        if (pts[i].x > pts[maxIdx].x || (pts[i].x == pts[maxIdx].x && pts[i].y > pts[maxIdx].y))
            maxIdx = i;
    }

    const Point2D& A = pts[minIdx];
    const Point2D& B = pts[maxIdx];

    // τα υπόλοιπα πάνε στο άνω ή κάτω μισό
    std::vector<int> upper, lower;
    for (int i = 0; i < n; i++) {
        if (i == minIdx || i == maxIdx) continue;
        if (ccw(A, B, pts[i]) < 0) upper.push_back(i);
        if (ccw(B, A, pts[i]) < 0) lower.push_back(i);
    }

    std::vector<Point2D> hull;
    hull.push_back(A);
    qh_internal::quickhull_rec(pts, upper, A, B, hull);
    hull.push_back(B);
    qh_internal::quickhull_rec(pts, lower, B, A, hull);

    return hull;
}
