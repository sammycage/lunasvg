#ifndef POINT_H
#define POINT_H

#include <cmath>

namespace lunasvg {

static const double EPSILON_DOUBLE = 0.000000000001;

static inline bool almostEqual(double p1, double p2)
{
    return (std::abs(p1 - p2) < EPSILON_DOUBLE);
}
static inline bool almostZero(double f)
{
    return (std::abs(f) <= EPSILON_DOUBLE);
}


class Point
{
public:
    Point() : x(0), y(0) {}
    Point(double _x, double _y) : x(_x), y(_y) {}

public:
    double x;
    double y;
};

} // namespace lunasvg

#endif // POINT_H
