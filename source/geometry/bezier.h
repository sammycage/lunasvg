#ifndef BEZIER_H
#define BEZIER_H

#include "point.h"

namespace lunasvg {

class Bezier
{
public:
    Bezier() = default;
    Point pointAt(double t) const;
    double angleAt(double t) const;
    Bezier onInterval(double t0, double t1) const;
    double length() const;
    static void coefficients(double t, double& a, double& b, double& c, double& d);
    static Bezier fromPoints(const Point& start, const Point& cp1, const Point& cp2, const Point& end);
    inline void parameterSplitLeft(double t, Bezier* left);
    inline void split(Bezier* firstHalf, Bezier* secondHalf) const;
    double tAtLength(double len) const { return tAtLength(len, length()); }
    double tAtLength(double len, double totalLength) const;
    void splitAtLength(double len, Bezier* left, Bezier* right);
    Point derivative(double t) const;

public:
    Point p1, p2, p3, p4;
};

inline void Bezier::coefficients(double t, double& a, double& b, double& c, double& d)
{
    double m_t = 1.0f - t;
    b = m_t * m_t;
    c = t * t;
    d = c * t;
    a = b * m_t;
    b *= 3.0f * t;
    c *= 3.0f * m_t;
}

inline Point Bezier::pointAt(double t) const
{
    // numerically more stable:
    double x, y;

    double m_t = 1.0f - t;
    {
        double a = p1.x * m_t + p2.x * t;
        double b = p2.x * m_t + p3.x * t;
        double c = p3.x * m_t + p4.x * t;
        a = a * m_t + b * t;
        b = b * m_t + c * t;
        x = a * m_t + b * t;
    }
    {
        double a = p1.y * m_t + p2.y * t;
        double b = p2.y * m_t + p3.y * t;
        double c = p3.y * m_t + p4.y * t;
        a = a * m_t + b * t;
        b = b * m_t + c * t;
        y = a * m_t + b * t;
    }
    return { x, y };
}

inline void Bezier::parameterSplitLeft(double t, Bezier* left)
{
    left->p1.x = p1.x;
    left->p1.y = p1.y;

    left->p2.x = p1.x + t * (p2.x - p1.x);
    left->p2.y = p1.y + t * (p2.y - p1.y);

    left->p3.x = p2.x + t * (p3.x - p2.x);  // temporary holding spot
    left->p3.y = p2.y + t * (p3.y - p2.y);  // temporary holding spot

    p3.x = p3.x + t * (p4.x - p3.x);
    p3.y = p3.y + t * (p4.y - p3.y);

    p2.x = left->p3.x + t * (p3.x - left->p3.x);
    p2.y = left->p3.y + t * (p3.y - left->p3.y);

    left->p3.x = left->p2.x + t * (left->p3.x - left->p2.x);
    left->p3.y = left->p2.y + t * (left->p3.y - left->p2.y);

    left->p4.x = p1.x = left->p3.x + t * (p2.x - left->p3.x);
    left->p4.y = p1.y = left->p3.y + t * (p2.y - left->p3.y);
}

inline void Bezier::split(Bezier* firstHalf, Bezier* secondHalf) const
{
    double c = (p2.x + p3.x) * 0.5f;
    firstHalf->p2.x = (p1.x + p2.x) * 0.5f;
    secondHalf->p3.x = (p3.x + p4.x) * 0.5f;
    firstHalf->p1.x = p1.x;
    secondHalf->p4.x = p4.x;
    firstHalf->p3.x = (firstHalf->p2.x + c) * 0.5f;
    secondHalf->p2.x = (secondHalf->p3.x + c) * 0.5f;
    firstHalf->p4.x = secondHalf->p1.x = (firstHalf->p3.x + secondHalf->p2.x) * 0.5f;

    c = (p2.y + p3.y) / 2;
    firstHalf->p2.y = (p1.y + p2.y) * 0.5f;
    secondHalf->p3.y = (p3.y + p4.y) * 0.5f;
    firstHalf->p1.y = p1.y;
    secondHalf->p4.y = p4.y;
    firstHalf->p3.y = (firstHalf->p2.y + c) * 0.5f;
    secondHalf->p2.y = (secondHalf->p3.y + c) * 0.5f;
    firstHalf->p4.y = secondHalf->p1.y = (firstHalf->p3.y + secondHalf->p2.y) * 0.5f;
}

} // namespace lunasvg

#endif // BEZIER_H