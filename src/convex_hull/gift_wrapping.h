#pragma once
#include "../common/geometry.h"
#include <algorithm>
#include <vector>
#include <limits>

// Jarvis March (Gift Wrapping) — σε κάθε βήμα βρίσκουμε το επόμενο σημείο
// που είναι "πιο δεξιά" από όλα τα υπόλοιπα
inline std::vector<Point2D> convex_hull_gift_wrapping(const std::vector<Point2D>& pts) {
    int n = pts.size();
    if (n < 3) return pts;

    // ξεκινάμε από το χαμηλότερο-αριστερότερο σημείο
    int start = 0;
    for (int i = 1; i < n; i++) {
        if (pts[i].y < pts[start].y ||
            (pts[i].y == pts[start].y && pts[i].x < pts[start].x))
            start = i;
    }

    std::vector<Point2D> hull;
    int current = start;

    do {
        hull.push_back(pts[current]);
        int next = (current == 0) ? 1 : 0;

        for (int i = 0; i < n; i++) {
            if (i == current) continue;
            long double cross = ccw(pts[current], pts[next], pts[i]);
            if (cross < 0) {
                next = i; // βρήκαμε καλύτερο υποψήφιο
            } else if (cross == 0) {
                // συγγραμμικά: κρατάμε το πιο μακρινό
                if (dist2(pts[current], pts[i]) > dist2(pts[current], pts[next]))
                    next = i;
            }
        }
        current = next;
    } while (current != start);

    return hull;
}
