#pragma once
#include "../common/geometry.h"
#include <algorithm>
#include <vector>

// Graham's Scan — ταξινόμηση κατά x, ξεχωριστό άνω και κάτω hull
inline std::vector<Point2D> convex_hull_incremental(std::vector<Point2D> pts) {
    int n = pts.size();
    if (n < 3) return pts;

    std::sort(pts.begin(), pts.end());

    std::vector<Point2D> upper, lower;

    // άνω hull: αφαιρούμε όταν δεν έχουμε δεξιά στροφή
    for (int i = 0; i < n; i++) {
        while (upper.size() >= 2 &&
               ccw(upper[upper.size()-2], upper[upper.size()-1], pts[i]) >= 0)
            upper.pop_back();
        upper.push_back(pts[i]);
    }

    // κάτω hull: ίδια λογική από δεξιά προς αριστερά
    for (int i = n - 1; i >= 0; i--) {
        while (lower.size() >= 2 &&
               ccw(lower[lower.size()-2], lower[lower.size()-1], pts[i]) >= 0)
            lower.pop_back();
        lower.push_back(pts[i]);
    }

    // τα άκρα μετράνε και στα δύο, αφαιρούμε τα διπλά
    upper.pop_back();
    lower.pop_back();

    for (auto& p : lower) upper.push_back(p);
    return upper;
}
