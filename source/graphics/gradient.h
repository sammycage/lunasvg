#ifndef GRADIENT_H
#define GRADIENT_H

#include "svgenumeration.h"
#include "affinetransform.h"
#include "rgb.h"

#include <utility>
#include <vector>

namespace lunasvg {

enum GradientType
{
   GradientTypeUnknown,
   GradientTypeLinear,
   GradientTypeRadial
};

typedef std::pair<double, Rgb> GradientStop;
typedef std::vector<GradientStop> GradientStops;

class Gradient
{
public:
    Gradient();

    void setSpread(SpreadMethod spread) { m_spread = spread; }
    void setStops(const GradientStops& stops) { m_stops = stops; }
    void setMatrix(const AffineTransform& matrix) { m_matrix = matrix; }
    SpreadMethod spread() const { return m_spread; }
    const GradientStops& stops() const { return m_stops; }
    const AffineTransform& matrix() const { return m_matrix; }
    GradientType type() const { return m_type; }
    const double* values() const { return m_values; }

private:
    friend class LinearGradient;
    friend class RadialGradient;
    GradientType m_type;
    GradientStops m_stops;
    SpreadMethod m_spread;
    AffineTransform m_matrix;
    double m_values[5];
};

class LinearGradient : public Gradient
{
public:
   LinearGradient();
   LinearGradient(double x1, double y1, double x2, double y2);

   void setX1(double x1) { m_values[0] = x1; }
   void setY1(double y1) { m_values[1] = y1; }
   void setX2(double x2) { m_values[2] = x2; }
   void setY2(double y2) { m_values[3] = y2; }
   double x1() const { return m_values[0]; }
   double y1() const { return m_values[1]; }
   double x2() const { return m_values[2]; }
   double y2() const { return m_values[3]; }
};

class RadialGradient : public Gradient
{
public:
   RadialGradient();
   RadialGradient(double cx, double cy, double r, double fx, double fy);

   void setCx(double cx) { m_values[0] = cx; }
   void setCy(double cy) { m_values[1] = cy; }
   void setR(double r) { m_values[2] = r; }
   void setFx(double fx) { m_values[3] = fx; }
   void setFy(double fy) { m_values[4] = fy; }
   double cx() const { return m_values[0]; }
   double cy() const { return m_values[1]; }
   double r() const { return m_values[2]; }
   double fx() const { return m_values[3]; }
   double fy() const { return m_values[4]; }
};

} // namespace lunasvg

#endif // GRADIENT_H
