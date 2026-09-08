#pragma once
#include "../common/geometry.h"
#include <vector>
#include <algorithm>

// Axis-aligned rectangle [xlo,xhi] x [ylo,yhi]
struct KDRect {
    double xlo, xhi, ylo, yhi;

    bool contains(double x, double y) const {
        return x >= xlo && x <= xhi && y >= ylo && y <= yhi;
    }
    bool intersects(const KDRect& r) const {
        return xlo <= r.xhi && xhi >= r.xlo && ylo <= r.yhi && yhi >= r.ylo;
    }
};

struct KDNode {
    Point2D pt;
    int     axis;        // 0 = x-split, 1 = y-split
    int     left  = -1;
    int     right = -1;
};

// τμήμα διαχωρισμού — το κρατάμε για να το σχεδιάσουμε στο gnuplot
struct KDSplit {
    double x1, y1, x2, y2;
    int    depth;
};

// 2D kd-tree — κατασκευή O(n log n) με nth_element, ερώτημα O(√n + k)

class KDTree {
public:
    std::vector<KDNode>  nodes;
    std::vector<KDSplit> splits;
    int root = -1;

    void build(std::vector<Point2D> pts,
               KDRect bbox = {-110, 110, -110, 110});

    std::vector<Point2D> range_query(const KDRect& q,
                                     int* nodes_visited = nullptr) const;

private:
    int  build_rec(std::vector<Point2D>& pts, int l, int r,
                   int depth, KDRect region);
    void query_rec(int idx, const KDRect& region, const KDRect& q,
                   std::vector<Point2D>& res, int& cnt) const;
};

// ── Implementation ────────────────────────────────────────────────────────────

inline void KDTree::build(std::vector<Point2D> pts, KDRect bbox)
{
    nodes.clear();
    splits.clear();
    root = -1;
    if (pts.empty()) return;
    nodes.reserve(pts.size());
    splits.reserve(pts.size());
    root = build_rec(pts, 0, (int)pts.size() - 1, 0, bbox);
}

inline int KDTree::build_rec(std::vector<Point2D>& pts,
                              int l, int r, int depth, KDRect region)
{
    if (l > r) return -1;

    int axis = depth % 2;
    int mid  = (l + r) / 2;

    // nth_element: O(n) ανά επίπεδο, αποφεύγουμε πλήρη ταξινόμηση
    std::nth_element(pts.begin() + l,
                     pts.begin() + mid,
                     pts.begin() + r + 1,
        [axis](const Point2D& a, const Point2D& b) {
            return axis == 0 ? a.x < b.x : a.y < b.y;
        });

    // Record splitting segment bounded by current cell
    if (axis == 0)
        splits.push_back({pts[mid].x, region.ylo,
                          pts[mid].x, region.yhi, depth});
    else
        splits.push_back({region.xlo, pts[mid].y,
                          region.xhi, pts[mid].y, depth});

    int idx = (int)nodes.size();
    nodes.push_back({pts[mid], axis, -1, -1});

    // Narrow the bounding region for each child
    KDRect lr = region, rr = region;
    if (axis == 0) { lr.xhi = pts[mid].x; rr.xlo = pts[mid].x; }
    else           { lr.yhi = pts[mid].y; rr.ylo = pts[mid].y; }

    // το reserve() παραπάνω εγγυάται ότι δεν γίνεται reallocation εδώ
    nodes[idx].left  = build_rec(pts, l,     mid - 1, depth + 1, lr);
    nodes[idx].right = build_rec(pts, mid + 1, r,     depth + 1, rr);
    return idx;
}

inline std::vector<Point2D> KDTree::range_query(const KDRect& q, int* nv) const
{
    std::vector<Point2D> res;
    int cnt = 0;
    query_rec(root, {-1e9, 1e9, -1e9, 1e9}, q, res, cnt);
    if (nv) *nv = cnt;
    return res;
}

inline void KDTree::query_rec(int idx, const KDRect& region,
                               const KDRect& q,
                               std::vector<Point2D>& res, int& cnt) const
{
    if (idx == -1) return;

    // αποκοπή: αν το κελί δεν τέμνεται με το ερώτημα, δεν προχωράμε
    if (!region.intersects(q)) return;

    ++cnt;
    const KDNode& nd = nodes[idx];

    if (q.contains(nd.pt.x, nd.pt.y))
        res.push_back(nd.pt);

    KDRect lr = region, rr = region;
    if (nd.axis == 0) { lr.xhi = nd.pt.x; rr.xlo = nd.pt.x; }
    else              { lr.yhi = nd.pt.y; rr.ylo = nd.pt.y; }

    query_rec(nd.left,  lr, q, res, cnt);
    query_rec(nd.right, rr, q, res, cnt);
}
