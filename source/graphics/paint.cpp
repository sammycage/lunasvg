#include "paint.h"
#include "rgb.h"
#include "gradient.h"
#include "pattern.h"

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

} // namespace lunasvg
