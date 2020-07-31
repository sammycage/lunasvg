#include "affinetransform.h"
#include "point.h"
#include "rect.h"

#include <cmath>
#include <limits>

namespace lunasvg {

AffineTransform::AffineTransform()
{
    m_matrix[0] = 1.0; m_matrix[1] = 0.0;
    m_matrix[2] = 0.0; m_matrix[3] = 1.0;
    m_matrix[4] = 0.0; m_matrix[5] = 0.0;
}

AffineTransform::AffineTransform(const AffineTransform& transform)
{
   const double* m = transform.m_matrix;
   m_matrix[0] = m[0]; m_matrix[1] = m[1];
   m_matrix[2] = m[2]; m_matrix[3] = m[3];
   m_matrix[4] = m[4]; m_matrix[5] = m[5];
}

AffineTransform::AffineTransform(double m00, double m10, double m01, double m11, double m02, double m12)
{
    m_matrix[0] = m00; m_matrix[1] = m10;
    m_matrix[2] = m01; m_matrix[3] = m11;
    m_matrix[4] = m02; m_matrix[5] = m12;
}

bool AffineTransform::isIdentity() const
{
    return (m_matrix[0] == 1.0 && m_matrix[1] == 0.0 &&
            m_matrix[2] == 0.0 && m_matrix[3] == 1.0 &&
            m_matrix[4] == 0.0 && m_matrix[5] == 0.0);
}

void AffineTransform::setMatrix(double m00, double m10, double m01, double m11, double m02, double m12)
{
    m_matrix[0] = m00; m_matrix[1] = m10;
    m_matrix[2] = m01; m_matrix[3] = m11;
    m_matrix[4] = m02; m_matrix[5] = m12;
}

void AffineTransform::multiply(const double* a, const double* b)
{
    double m00 = a[0] * b[0] + a[1] * b[2];
    double m10 = a[0] * b[1] + a[1] * b[3];
    double m01 = a[2] * b[0] + a[3] * b[2];
    double m11 = a[2] * b[1] + a[3] * b[3];
    double m02 = a[4] * b[0] + a[5] * b[2] + b[4];
    double m12 = a[4] * b[1] + a[5] * b[3] + b[5];

    m_matrix[0] = m00; m_matrix[1] = m10;
    m_matrix[2] = m01; m_matrix[3] = m11;
    m_matrix[4] = m02; m_matrix[5] = m12;
}

AffineTransform& AffineTransform::multiply(const AffineTransform& transform)
{
    const double* a = m_matrix;
    const double* b = transform.m_matrix;
    multiply(b, a);

    return *this;
}

AffineTransform& AffineTransform::postmultiply(const AffineTransform& transform)
{
    const double* a = m_matrix;
    const double* b = transform.m_matrix;
    multiply(a, b);

    return *this;
}

AffineTransform AffineTransform::inverted() const
{
    const double* m = m_matrix;
    double det = (m[0] * m[3] - m[1] * m[2]);
    if(det == 0.0)
        return AffineTransform();

    double inv_det = 1.0 / det;
    double m00 = m[0] * inv_det;
    double m10 = m[1] * inv_det;
    double m01 = m[2] * inv_det;
    double m11 = m[3] * inv_det;
    double m02 = (m[2] *  m[5] - m[3] * m[4]) * inv_det;
    double m12 = (m[1] *  m[4] - m[0] * m[5]) * inv_det;

    return AffineTransform(m11, -m10,
                           -m01, m00,
                           m02, m12);
}

void AffineTransform::reset()
{
    m_matrix[0] = 1.0; m_matrix[1] = 0.0;
    m_matrix[2] = 0.0; m_matrix[3] = 1.0;
    m_matrix[4] = 0.0; m_matrix[5] = 0.0;
}

AffineTransform AffineTransform::makeIdentity()
{
    return AffineTransform(1, 0,
                           0, 1,
                           0, 0);
}

AffineTransform AffineTransform::makeRotate(double radians)
{
    double c = std::cos(radians);
    double s = std::sin(radians);

    return AffineTransform(c, s,
                           -s, c,
                           0, 0);
}

AffineTransform AffineTransform::makeRotate(double radians, double cx, double cy)
{
    double c = std::cos(radians);
    double s = std::sin(radians);

    double x = cx * (1 - c) + cy * s;
    double y = cy * (1 - c) - cx * s;

    return AffineTransform(c, s,
                           -s, c,
                           x, y);
}

AffineTransform AffineTransform::makeScale(double sx, double sy)
{
    return AffineTransform(sx, 0,
                           0, sy,
                           0, 0);
}

AffineTransform AffineTransform::makeShear(double shx, double shy)
{
    double x = std::tan(shx);
    double y = std::tan(shy);

    return AffineTransform(1, y,
                           x, 1,
                           0, 0);
}

AffineTransform AffineTransform::makeTranslate(double tx, double ty)
{
    return AffineTransform(1, 0,
                           0, 1,
                           tx, ty);
}

void AffineTransform::rotate(double radians)
{
    multiply(makeRotate(radians));
}

void AffineTransform::rotate(double radians, double cx, double cy)
{
    multiply(makeRotate(radians, cx, cy));
}

void AffineTransform::scale(double sx, double sy)
{
    multiply(makeScale(sx, sy));
}

void AffineTransform::shear(double shx, double shy)
{
    multiply(makeShear(shx, shy));
}

void AffineTransform::translate(double cx, double cy)
{
    multiply(makeTranslate(cx, cy));
}

void AffineTransform::map(double x, double y, double& _x, double& _y) const
{
    _x = x * m_matrix[0] + y * m_matrix[2] + m_matrix[4];
    _y = x * m_matrix[1] + y * m_matrix[3] + m_matrix[5];
}

Point AffineTransform::mapPoint(const Point& point) const
{
    Point p;
    map(point.x, point.y, p.x, p.y);
    return p;
}

Rect AffineTransform::mapRect(const Rect& rect) const
{
    Point p[4];
    p[0].x = rect.x;
    p[0].y = rect.y;
    p[1].x = rect.x + rect.width;
    p[1].y = rect.y;
    p[2].x = rect.x + rect.width;
    p[2].y = rect.y + rect.height;
    p[3].x = rect.x;
    p[3].y = rect.y + rect.height;

    map(p[0].x, p[0].y, p[0].x, p[0].y);
    map(p[1].x, p[1].y, p[1].x, p[1].y);
    map(p[2].x, p[2].y, p[2].x, p[2].y);
    map(p[3].x, p[3].y, p[3].x, p[3].y);

    double xMin = std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::min();
    double yMax = std::numeric_limits<double>::min();

    for(int i = 0;i < 4;i++)
    {
        if(p[i].x < xMin) xMin = p[i].x;
        if(p[i].x > xMax) xMax = p[i].x;
        if(p[i].y < yMin) yMin = p[i].y;
        if(p[i].y > yMax) yMax = p[i].y;
    }

    return Rect(xMin, yMin, xMax-xMin, yMax-yMin);
}

} //namespace lunasvg
