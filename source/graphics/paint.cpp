#include "paint.h"

namespace lunasvg {

class PaintData
{
public:
    PaintType type{PaintTypeNone};
    double opacity{1.0};
};

struct ColorPaintData : public PaintData
{
    Rgb color;
};

struct GradientPaintData : public PaintData
{
    Gradient gradient;
};

struct PatternPaintData : public PaintData
{
    Pattern pattern;
};

Paint::~Paint()
{
}

Paint::Paint()
{
}

Paint::Paint(const Rgb& color)
{
    setColor(color);
}

Paint::Paint(const Gradient& gradient)
{
    setGradient(gradient);
}

Paint::Paint(const Pattern& pattern)
{
    setPattern(pattern);
}

void Paint::setColor(const Rgb& color)
{
    m_data.reset(new ColorPaintData);

    ColorPaintData* colorData = static_cast<ColorPaintData*>(m_data.get());
    colorData->color = color;
    colorData->type = PaintTypeColor;
}

void Paint::setGradient(const Gradient& gradient)
{
    m_data.reset(new GradientPaintData);

    GradientPaintData* gradientData = static_cast<GradientPaintData*>(m_data.get());
    gradientData->gradient = gradient;
    gradientData->type = PaintTypeGradient;
}

void Paint::setPattern(const Pattern& pattern)
{
    m_data.reset(new PatternPaintData);

    PatternPaintData* patternData = static_cast<PatternPaintData*>(m_data.get());
    patternData->pattern = pattern;
    patternData->type = PaintTypePattern;
}

void Paint::setOpacity(double opacity)
{
    if(m_data)
        m_data->opacity = opacity;
}

const Rgb* Paint::color() const
{
    return m_data && m_data->type == PaintTypeColor ? &static_cast<const ColorPaintData*>(m_data.get())->color : nullptr;
}

const Gradient* Paint::gradient() const
{
    return m_data && m_data->type == PaintTypeGradient ? &static_cast<const GradientPaintData*>(m_data.get())->gradient : nullptr;
}

const Pattern* Paint::pattern() const
{
    return m_data && m_data->type == PaintTypePattern ? &static_cast<const PatternPaintData*>(m_data.get())->pattern : nullptr;
}

bool Paint::isNone() const
{
    return !m_data;
}

double Paint::opacity() const
{
    return m_data ? m_data->opacity : 0.0;
}

PaintType Paint::type() const
{
    return m_data ? m_data->type : PaintTypeNone;
}

Rgb::Rgb()
    : r(0), g(0), b(0), a(255)
{
}

Rgb::Rgb(unsigned int value)
    : r((value&0xff000000)>>24),
      g((value&0x00ff0000)>>16),
      b((value&0x0000ff00)>>8 ),
      a((value&0x000000ff)>>0 )
{
}

Rgb::Rgb(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    : r(red), g(green), b(blue), a(alpha)
{
}

unsigned int Rgb::value() const
{
    return (r<<24) | (g<<16) | (b<<8) | a;
}

bool Rgb::operator==(const Rgb& rgb) const
{
    return (rgb.r == r)
            && (rgb.g == g)
            && (rgb.b == b)
            && (rgb.a == a);
}

bool Rgb::operator!=(const Rgb& rgb) const
{
    return !operator==(rgb);
}

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

Pattern::Pattern()
{
}

Pattern::Pattern(const Canvas& canvas)
    : m_tile(canvas),
      m_tileMode(TileModeRepeat)
{
}

} // namespace lunasvg
