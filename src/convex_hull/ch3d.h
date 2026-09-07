#pragma once
#include "../common/geometry.h"
#include <algorithm>
#include <vector>
#include <map>

// 3D Κυρτό Περίβλημα — αλγόριθμος Beneath-Beyond
// Κάθε έδρα αποθηκεύεται με εξωτερική κατεύθυνση (orient3d < 0 ως προς εσωτερικό)
// Μια έδρα είναι ορατή από P αν orient3d(a,b,c,P) > 0

struct Face3D { int a, b, c; };

inline std::vector<Face3D> convex_hull_3d(std::vector<Point3D>& pts) {
    int n = pts.size();
    if (n < 4) {
        std::vector<Face3D> res;
        if (n == 3) res.push_back({0, 1, 2});
        return res;
    }

    for (int i = 0; i < n; i++) pts[i].id = i;

    // ταξινόμηση κατά φθίνον x για να ξεκινήσουμε από άκρα σημεία
    std::vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return pts[a].x > pts[b].x;
    });

    // βρίσκουμε 4 σημεία που δεν είναι συνεπίπεδα για το αρχικό τετράεδρο
    int p0 = order[0], p1 = order[1], p2 = -1, p3 = -1;
    for (int k = 2; k < n && p2 == -1; k++) {
        int pk = order[k];
        // collinear check: cross product of (p1-p0) and (pk-p0) non-zero
        double cx = (pts[p1].y-pts[p0].y)*(pts[pk].z-pts[p0].z)
                   -(pts[p1].z-pts[p0].z)*(pts[pk].y-pts[p0].y);
        double cy = (pts[p1].z-pts[p0].z)*(pts[pk].x-pts[p0].x)
                   -(pts[p1].x-pts[p0].x)*(pts[pk].z-pts[p0].z);
        double cz = (pts[p1].x-pts[p0].x)*(pts[pk].y-pts[p0].y)
                   -(pts[p1].y-pts[p0].y)*(pts[pk].x-pts[p0].x);
        if (cx*cx + cy*cy + cz*cz > 1e-12) { p2 = pk; }
    }
    for (int k = 0; k < n && p3 == -1; k++) {
        int pk = order[k];
        if (pk == p0 || pk == p1 || pk == p2) continue;
        if (std::abs(orient3d(pts[p0], pts[p1], pts[p2], pts[pk])) > 1e-12)
            p3 = pk;
    }
    if (p2 == -1 || p3 == -1) return {}; // degenerate input

    // κέντρο βάρους τετραέδρου — το χρησιμοποιούμε ως εσωτερικό σημείο αναφοράς
    Point3D interior = {
        (pts[p0].x + pts[p1].x + pts[p2].x + pts[p3].x) / 4.0,
        (pts[p0].y + pts[p1].y + pts[p2].y + pts[p3].y) / 4.0,
        (pts[p0].z + pts[p1].z + pts[p2].z + pts[p3].z) / 4.0, -1
    };

    // διορθώνουμε την κατεύθυνση της έδρας ώστε το εσωτερικό να είναι στην αρνητική πλευρά
    auto make_face = [&](int a, int b, int c) -> Face3D {
        if (orient3d(pts[a], pts[b], pts[c], interior) > 0)
            return {a, c, b}; // flip
        return {a, b, c};
    };

    std::vector<Face3D> faces = {
        make_face(p0, p1, p2),
        make_face(p0, p1, p3),
        make_face(p0, p2, p3),
        make_face(p1, p2, p3)
    };

    std::vector<bool> in_hull(n, false);
    for (int idx : {p0, p1, p2, p3}) {
        // Mark by order position
        for (int k = 0; k < n; k++) if (order[k] == idx) { in_hull[k] = true; break; }
    }

    // Process remaining points
    for (int k = 0; k < n; k++) {
        if (in_hull[k]) continue;
        int pk = order[k];
        const Point3D& P = pts[pk];

        // ποιες έδρες βλέπουν το νέο σημείο P;
        int nf = faces.size();
        std::vector<bool> visible(nf, false);
        bool any = false;
        for (int fi = 0; fi < nf; fi++) {
            if (orient3d(pts[faces[fi].a], pts[faces[fi].b], pts[faces[fi].c], P) > 0) {
                visible[fi] = true;
                any = true;
            }
        }
        if (!any) continue; // το P είναι ήδη εντός του hull

        // χτίζουμε τον χάρτη ακμών για να βρούμε γρήγορα τις horizon ακμές
        std::map<std::pair<int,int>, int> edge_face;
        for (int fi = 0; fi < nf; fi++) {
            int v[3] = {faces[fi].a, faces[fi].b, faces[fi].c};
            for (int e = 0; e < 3; e++)
                edge_face[{v[e], v[(e+1)%3]}] = fi;
        }

        // horizon: ακμές μεταξύ ορατής και μη ορατής έδρας
        std::vector<std::pair<int,int>> horizon;
        for (int fi = 0; fi < nf; fi++) {
            if (!visible[fi]) continue;
            int v[3] = {faces[fi].a, faces[fi].b, faces[fi].c};
            for (int e = 0; e < 3; e++) {
                int u = v[e], w = v[(e+1)%3];
                auto it = edge_face.find({w, u});
                if (it != edge_face.end() && !visible[it->second])
                    horizon.push_back({u, w});
            }
        }

        // κρατάμε τις μη ορατές έδρες, προσθέτουμε νέες από P προς κάθε horizon ακμή
        std::vector<Face3D> new_faces;
        new_faces.reserve(nf - (int)horizon.size() + (int)horizon.size());
        for (int fi = 0; fi < nf; fi++)
            if (!visible[fi]) new_faces.push_back(faces[fi]);

        for (auto& [u, v] : horizon)
            new_faces.push_back(make_face(pk, v, u));

        faces = std::move(new_faces);
        in_hull[k] = true;

        // ενημερώνουμε το εσωτερικό σημείο ώστε να παραμένει εντός του hull
        interior.x = (interior.x * 3 + P.x) / 4.0;
        interior.y = (interior.y * 3 + P.y) / 4.0;
        interior.z = (interior.z * 3 + P.z) / 4.0;
    }

    return faces;
}
