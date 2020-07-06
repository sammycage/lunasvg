#include "gradient.h"

namespace lunasvg {

Gradient::Gradient()
{
    m_type = GradientTypeUnknown;
    m_spread = SpreadMethodPad;
}

LinearGradient::LinearGradient()
{
    m_type = GradientTypeLinear;
    m_values[0] = 0.0;
    m_values[1] = 0.0;
    m_values[2] = 1.0;
    m_values[3] = 1.0;
    m_values[4] = 0.0;
}

LinearGradient::LinearGradient(double x1, double y1, double x2, double y2)
{
    m_type = GradientTypeLinear;
    m_values[0] = x1;
    m_values[1] = y1;
    m_values[2] = x2;
    m_values[3] = y2;
    m_values[4] = 0.0;
}

RadialGradient::RadialGradient()
{
    m_type = GradientTypeRadial;
    m_values[0] = 0.0;
    m_values[1] = 0.0;
    m_values[2] = 1.0;
    m_values[3] = 0.0;
    m_values[4] = 0.0;
}

RadialGradient::RadialGradient(double cx, double cy, double r, double fx, double fy)
{
    m_type = GradientTypeRadial;
    m_values[0] = cx;
    m_values[1] = cy;
    m_values[2] = r;
    m_values[3] = fx;
    m_values[4] = fy;
}

} // namespace lunasvg
