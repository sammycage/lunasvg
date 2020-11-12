#include "bezier.h"
#include "line.h"

namespace lunasvg
{

Bezier Bezier::fromPoints(const Point& p1, const Point& p2,
    const Point& p3, const Point& p4)
{
    Bezier b;
    b.p1.x = p1.x;
    b.p1.y = p1.y;
    b.p2.x = p2.x;
    b.p2.y = p2.y;
    b.p3.x = p3.x;
    b.p3.y = p3.y;
    b.p4.x = p4.x;
    b.p4.y = p4.y;
    return b;
}

double Bezier::length() const
{
    const auto len = Line::length(p1.x, p1.y, p2.x, p2.y) +
        Line::length(p2.x, p2.y, p3.x, p3.y) +
        Line::length(p3.x, p3.y, p4.x, p4.y);

    const auto chord = Line::length(p1.x, p1.y, p4.x, p4.y);

    if ((len - chord) > 0.01) {
        Bezier left, right;
        split(&left, &right);
        return left.length() + right.length();
    }

    return len;
}

Bezier Bezier::onInterval(double t0, double t1) const
{
    if (t0 == 0 && t1 == 1) return *this;

    Bezier bezier = *this;

    Bezier result;
    bezier.parameterSplitLeft(t0, &result);
    double trueT = (t1 - t0) / (1 - t0);
    bezier.parameterSplitLeft(trueT, &result);

    return result;
}

double Bezier::tAtLength(double l, double totalLength) const
{
    double       t = 1.0;
    const double error = 0.01f;
    if (l > totalLength || almostEqual(l, totalLength)) return t;

    t *= 0.5;

    double lastBigger = 1.0;
    while (1) {
        Bezier right = *this;
        Bezier left;
        right.parameterSplitLeft(t, &left);
        double lLen = left.length();
        if (fabs(lLen - l) < error) break;

        if (lLen < l) {
            t += (lastBigger - t) * 0.5f;
        }
        else {
            lastBigger = t;
            t -= t * 0.5f;
        }
    }
    return t;
}

void Bezier::splitAtLength(double len, Bezier* left, Bezier* right)
{
    double t;

    *right = *this;
    t = right->tAtLength(len);
    right->parameterSplitLeft(t, left);
}

Point Bezier::derivative(double t) const
{
    // p'(t) = 3 * (-(1-2t+t^2) * p0 + (1 - 4 * t + 3 * t^2) * p1 + (2 * t - 3 *
    // t^2) * p2 + t^2 * p3)

    double m_t = 1.0f - t;

    double d = t * t;
    double a = -m_t * m_t;
    double b = 1 - 4 * t + 3 * d;
    double c = 2 * t - 3 * d;


    return Point(3 * (a * p1.x + b * p2.x + c * p3.x + d * p4.x), 3 * (a * p1.y + b * p2.y + c * p3.y + d * p4.y));
}

double Bezier::angleAt(double t) const
{
    if (t < 0 || t > 1) {
        return 0;
    }
    return Line({}, derivative(t)).angle();
}

} // namespace lunasvg