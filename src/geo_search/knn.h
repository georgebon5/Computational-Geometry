#pragma once
#include <vector>
#include <algorithm>
#include <queue>

// kd-tree για k-NN ερωτήματα (ξεχωριστό από το KDTree του Μέρους Γ)
// Κατασκευή O(n log n), k-NN ερώτημα O(k log n) αναμενόμενο

struct KNNNode {
    double x, y;
    int    label;
    int    left  = -1;
    int    right = -1;
    int    axis;          // 0 = split on x, 1 = split on y
};

class KNNTree {
public:
    struct Point { double x, y; int label; };

    std::vector<KNNNode> nodes;

    void build(std::vector<Point> pts) {
        nodes.clear();
        root_ = -1;
        if (pts.empty()) return;
        nodes.reserve(pts.size()); // αποφεύγουμε reallocation κατά την αναδρομή
        root_ = build_rec(pts, 0, (int)pts.size() - 1, 0);
    }

    // επιστρέφει τους k κοντινότερους γείτονες (ως δείκτες στο nodes[])
    std::vector<int> knn(double qx, double qy, int k) const {
        using E = std::pair<double, int>;
        std::priority_queue<E> heap; // max-heap μεγέθους k
        knn_rec(root_, qx, qy, k, heap);
        std::vector<int> res;
        res.reserve(heap.size());
        while (!heap.empty()) { res.push_back(heap.top().second); heap.pop(); }
        return res;
    }

private:
    int root_ = -1;

    int build_rec(std::vector<Point>& pts, int l, int r, int depth) {
        if (l > r) return -1;
        int axis = depth % 2;
        int mid  = (l + r) / 2;
        std::nth_element(pts.begin() + l, pts.begin() + mid, pts.begin() + r + 1,
            [axis](const Point& a, const Point& b) {
                return axis == 0 ? a.x < b.x : a.y < b.y;
            });
        int idx = (int)nodes.size();
        nodes.push_back({pts[mid].x, pts[mid].y, pts[mid].label, -1, -1, axis});
        nodes[idx].left  = build_rec(pts, l,       mid - 1, depth + 1);
        nodes[idx].right = build_rec(pts, mid + 1, r,       depth + 1);
        return idx;
    }

    void knn_rec(int idx, double qx, double qy, int k,
                 std::priority_queue<std::pair<double,int>>& heap) const {
        if (idx < 0) return;
        const KNNNode& n = nodes[idx];

        // Update heap with this node
        double d2 = (n.x - qx)*(n.x - qx) + (n.y - qy)*(n.y - qy);
        if ((int)heap.size() < k)
            heap.push({d2, idx});
        else if (d2 < heap.top().first) {
            heap.pop();
            heap.push({d2, idx});
        }

        double axis_val = (n.axis == 0) ? n.x : n.y;
        double q_val    = (n.axis == 0) ? qx  : qy;
        double plane_d2 = (q_val - axis_val) * (q_val - axis_val);

        int near = (q_val <= axis_val) ? n.left  : n.right;
        int far_ = (q_val <= axis_val) ? n.right : n.left;

        knn_rec(near, qx, qy, k, heap); // πάντα ψάχνουμε την κοντινή πλευρά

        // αποκοπή: αν η απόσταση από το hyperplane > k-οστός γείτονας, παρακάμπτουμε
        double worst = ((int)heap.size() == k) ? heap.top().first : 1e300;
        if (plane_d2 <= worst)
            knn_rec(far_, qx, qy, k, heap);
    }
};
