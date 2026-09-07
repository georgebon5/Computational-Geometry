#pragma once
#include <cmath>
#include <vector>

struct Point2D {
    double x, y;
    bool operator<(const Point2D& o) const {
        return x < o.x || (x == o.x && y < o.y);
    }
    bool operator==(const Point2D& o) const { return x == o.x && y == o.y; }
};

struct Point3D {
    double x, y, z;
    int id;
};

// CCW > 0: αριστερή στροφή, CCW < 0: δεξιά στροφή, CCW = 0: συγγραμμικά
inline long double ccw(const Point2D& p0, const Point2D& p1, const Point2D& p2) {
    return (long double)(p1.x - p0.x) * (p2.y - p0.y)
         - (long double)(p2.x - p0.x) * (p1.y - p0.y);
}

// ορίζουσα 4x4 για τον προσανατολισμό στο 3D
inline long double orient3d(const Point3D& p0, const Point3D& p1,
                            const Point3D& p2, const Point3D& p3) {
    long double ax = p1.x - p0.x, ay = p1.y - p0.y, az = p1.z - p0.z;
    long double bx = p2.x - p0.x, by = p2.y - p0.y, bz = p2.z - p0.z;
    long double cx = p3.x - p0.x, cy = p3.y - p0.y, cz = p3.z - p0.z;
    return ax*(by*cz - bz*cy) - ay*(bx*cz - bz*cx) + az*(bx*cy - by*cx);
}

inline double dist2(const Point2D& a, const Point2D& b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return dx*dx + dy*dy;
}

// Distance from point p to line through a,b (signed, proportional to CCW)
inline long double distToLine(const Point2D& a, const Point2D& b, const Point2D& p) {
    return ccw(a, b, p);
}
