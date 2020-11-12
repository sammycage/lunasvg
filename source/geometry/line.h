#ifndef LINE_H
#define LINE_H

#include "point.h"

namespace lunasvg {

class Line
{
public:
    Line() : p1(), p2() {}
    Line(double x1, double y1, double x2, double y2)
    : p1(x1,y1), p2(x2, y2)
    {
    }
    Line(const Point& _p1, const Point& _p2)
    : p1(_p1), p2(_p2)
    {
    }
    double length() const { return length(p1.x, p1.y, p2.x, p2.y); }
    void splitAtLength(double length, Line& left, Line& right) const;
    double angle() const;
    static double length(double x1, double y1, double x2, double y2);

public:
    Point p1;
    Point p2;
};

inline double Line::angle() const
{
    static constexpr double K_PI = 3.141592;
    const double dx = p2.x - p1.x;
    const double dy = p2.y - p1.y;
    const double theta = std::atan2(dy, dx) * 180.0 / K_PI;
    return theta;
}

// approximate sqrt(x*x + y*y) using alpha max plus beta min algorithm.
// With alpha = 1, beta = 3/8, giving results with the largest error less
// than 7% compared to the exact value.
inline double Line::length(double x1, double y1, double x2, double y2)
{
    double x = x2 - x1;
    double y = y2 - y1;

    x = x < 0 ? -x : x;
    y = y < 0 ? -y : y;

    return (x > y ? x + 0.375 * y : y + 0.375 * x);
}

inline void Line::splitAtLength(double lengthAt, Line& left, Line& right) const
{
    double  len = length();
    double dx = ((p2.x - p1.x) / len) * lengthAt;
    double dy = ((p2.y - p1.y) / len) * lengthAt;

    left.p1.x = p1.x;
    left.p1.y = p1.y;
    left.p2.x = left.p1.x + dx;
    left.p2.y = left.p1.y + dy;

    right.p1.x = left.p2.x;
    right.p1.y = left.p2.y;
    right.p2.x = p2.x;
    right.p2.y = p2.y;
}

}

#endif // LINE_H