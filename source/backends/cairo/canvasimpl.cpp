#include "canvasimpl.h"
#include "affinetransform.h"
#include "pathiterator.h"
#include "strokedata.h"
#include "paint.h"
#include "rgb.h"
#include "gradient.h"
#include "pattern.h"

namespace lunasvg {

static cairo_pattern_t* to_cairo_pattern(const Paint& paint);
static cairo_matrix_t to_cairo_matrix(const AffineTransform& transform);
static cairo_fill_rule_t to_cairo_fill_rule(WindRule fillRule);
static cairo_line_cap_t to_cairo_line_cap(LineCap cap);
static cairo_line_join_t to_cairo_line_join(LineJoin join);
static cairo_operator_t to_cairo_operator(BlendMode mode);

CanvasImpl::~CanvasImpl()
{
    cairo_destroy(m_cr);
    cairo_surface_destroy(m_surface);
}

CanvasImpl::CanvasImpl(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride)
    : m_surface(cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, int(width), int(height), int(stride))),
      m_cr(cairo_create(m_surface))
{
}

CanvasImpl::CanvasImpl(unsigned int width, unsigned int height)
    : m_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, int(width), int(height))),
      m_cr(cairo_create(m_surface))
{
}

void CanvasImpl::clear(const Rgb& color)
{
    std::uint32_t a = color.a;
    std::uint32_t pr = (color.r * a) / 255;
    std::uint32_t pg = (color.g * a) / 255;
    std::uint32_t pb = (color.b * a) / 255;
    std::uint32_t solid = (a << 24) | (pr << 16) | (pg << 8) | pb;

    std::uint32_t width = this->width();
    std::uint32_t height = this->height();
    std::uint32_t stride = this->stride();
    std::uint8_t* data = this->data();
    for(std::uint32_t y = 0;y < height;y++)
    {
        std::uint32_t* pixels = reinterpret_cast<std::uint32_t*>(data + stride * y);
        for(std::uint32_t x = 0;x < width;x++)
        {
            pixels[x] = solid;
        }
    }
}

void CanvasImpl::blend(const Canvas& source, BlendMode mode, double opacity, double dx, double dy)
{
    cairo_save(m_cr);
    cairo_set_source_surface(m_cr, source.impl()->surface(), dx, dy);
    cairo_set_operator(m_cr, to_cairo_operator(mode));
    if(opacity == 1.0) cairo_paint(m_cr);
    else cairo_paint_with_alpha(m_cr, opacity);
    cairo_restore(m_cr);
}

void CanvasImpl::draw(const Path& path, const AffineTransform& matrix, WindRule fillRule, const Paint& fillPaint, const Paint& strokePaint, const StrokeData& strokeData)
{
    if(fillPaint.isNone() && strokePaint.isNone())
        return;

    cairo_save(m_cr);

    cairo_matrix_t m = to_cairo_matrix(matrix);
    cairo_set_matrix(m_cr, &m);

    PathIterator it(path);
    double c[6];
    while(!it.isDone())
    {
        switch(it.currentSegment(c))
        {
        case SegTypeMoveTo:
            cairo_move_to(m_cr, c[0], c[1]);
            break;
        case SegTypeLineTo:
            cairo_line_to(m_cr, c[0], c[1]);
            break;
        case SegTypeCubicTo:
            cairo_curve_to(m_cr, c[0], c[1], c[2], c[3], c[4], c[5]);
            break;
        case SegTypeClose:
            cairo_close_path(m_cr);
            break;
        case SegTypeQuadTo:
        {
            double x0, y0;
            cairo_get_current_point(m_cr, &x0, &y0);
            cairo_curve_to(m_cr,
                    2.0 / 3.0 * c[0] + 1.0 / 3.0 * x0,
                    2.0 / 3.0 * c[1] + 1.0 / 3.0 * y0,
                    2.0 / 3.0 * c[0] + 1.0 / 3.0 * c[2],
                    2.0 / 3.0 * c[1] + 1.0 / 3.0 * c[3],
                    c[2], c[3]);
            break;
        }
        }
        it.next();
    }

    if(!fillPaint.isNone())
    {
        cairo_pattern_t* pattern = to_cairo_pattern(fillPaint);
        cairo_set_fill_rule(m_cr, to_cairo_fill_rule(fillRule));
        cairo_set_source(m_cr, pattern);
        cairo_fill_preserve(m_cr);
        cairo_pattern_destroy(pattern);
    }

    if(!strokePaint.isNone())
    {
        cairo_pattern_t* pattern = to_cairo_pattern(strokePaint);
        cairo_set_line_width(m_cr, strokeData.width());
        cairo_set_miter_limit(m_cr, strokeData.miterLimit());
        cairo_set_line_cap(m_cr, to_cairo_line_cap(strokeData.cap()));
        cairo_set_line_join(m_cr, to_cairo_line_join(strokeData.join()));
        cairo_set_dash(m_cr, strokeData.dash().data(), int(strokeData.dash().size()), strokeData.dashOffset());
        cairo_set_source(m_cr, pattern);
        cairo_stroke_preserve(m_cr);
        cairo_pattern_destroy(pattern);
    }

    cairo_new_path(m_cr);
    cairo_restore(m_cr);
}

void CanvasImpl::updateLuminance()
{
    std::uint32_t width = this->width();
    std::uint32_t height = this->height();
    std::uint32_t stride = this->stride();
    std::uint8_t* data = this->data();
    for(std::uint32_t y = 0;y < height;y++)
    {
        std::uint32_t* pixels = reinterpret_cast<std::uint32_t*>(data + stride * y);
        for(std::uint32_t x = 0;x < width;x++)
        {
            std::uint32_t r = (pixels[x] >> 16) & 0xff;
            std::uint32_t g = (pixels[x] >> 8) & 0xff;
            std::uint32_t b = (pixels[x] >> 0) & 0xff;

            std::uint32_t luminosity = (2*r + 3*g + b) / 6;
            pixels[x] = luminosity << 24;
        }
    }
}

void CanvasImpl::convertToRGBA()
{
    std::uint32_t width = this->width();
    std::uint32_t height = this->height();
    std::uint32_t stride = this->stride();
    std::uint8_t* data = this->data();
    for(std::uint32_t y = 0;y < height;y++)
    {
        std::uint32_t* pixels = reinterpret_cast<std::uint32_t*>(data + stride * y);
        for(std::uint32_t x = 0;x < width;x++)
        {
            std::uint32_t a = pixels[x] >> 24;
            if(a != 0)
            {
                std::uint32_t r = (((pixels[x] >> 16) & 0xff) * 255) / a;
                std::uint32_t g = (((pixels[x] >> 8) & 0xff) * 255) / a;
                std::uint32_t b = (((pixels[x] >> 0) & 0xff) * 255) / a;

                pixels[x] = (a << 24) | (b << 16) | (g << 8) | r;
            }
            else
            {
                pixels[x] = 0;
            }
        }
    }
}

unsigned char* CanvasImpl::data() const
{
    return cairo_image_surface_get_data(m_surface);
}

unsigned int CanvasImpl::width() const
{
    return std::uint32_t(cairo_image_surface_get_width(m_surface));
}

unsigned int CanvasImpl::height() const
{
    return std::uint32_t(cairo_image_surface_get_height(m_surface));
}

unsigned int CanvasImpl::stride() const
{
    return std::uint32_t(cairo_image_surface_get_stride(m_surface));
}

cairo_matrix_t to_cairo_matrix(const AffineTransform& transform)
{
#define T(v) std::max(-32767.0, std::min(v, 32767.0))
    const double* m = transform.getMatrix();
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, T(m[0]), T(m[1]), T(m[2]), T(m[3]), T(m[4]), T(m[5]));
    return matrix;
}

cairo_pattern_t* to_cairo_pattern(const Paint& paint)
{
    if(paint.type() == PaintTypeColor)
    {
        const Rgb* c = paint.color();
        return cairo_pattern_create_rgba(c->r/255.0, c->g/255.0, c->b/255.0, (c->a/255.0)*paint.opacity());
    }

    if(paint.type() == PaintTypeGradient)
    {
        const Gradient* gradient = paint.gradient();
        cairo_pattern_t* pattern;
        if(gradient->type() == GradientTypeLinear)
        {
            const LinearGradient& linear = static_cast<const LinearGradient&>(*gradient);
            pattern = cairo_pattern_create_linear(linear.x1(), linear.y1(), linear.x2(), linear.y2());
        }
        else if(gradient->type() == GradientTypeRadial)
        {
            const RadialGradient& radial = static_cast<const RadialGradient&>(*gradient);
            pattern = cairo_pattern_create_radial(radial.fx(), radial.fy(), 0, radial.cx(), radial.cy(), radial.r());
        }
        else
        {
            return nullptr;
        }

        if(gradient->spread() == SpreadMethodReflect)
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
        }
        else if(gradient->spread() == SpreadMethodRepeat)
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        }
        else
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }

        const GradientStops& stops = gradient->stops();
        for(unsigned int i = 0;i < stops.size();i++)
        {
            double offset = stops[i].first;
            const Rgb& c = stops[i].second;
            cairo_pattern_add_color_stop_rgba(pattern, offset, c.r/255.0, c.g/255.0, c.b/255.0, (c.a/255.0)*paint.opacity());
        }

        cairo_matrix_t matrix = to_cairo_matrix(gradient->matrix());
        cairo_matrix_invert(&matrix);
        cairo_pattern_set_matrix(pattern, &matrix);
        return pattern;
    }

    if(paint.type() == PaintTypePattern)
    {
        const Pattern* p = paint.pattern();
        cairo_pattern_t* pattern = cairo_pattern_create_for_surface(p->tile().impl()->surface());

        if(p->tileMode() == TileModeReflect)
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
        }
        else if(p->tileMode() == TileModeRepeat)
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        }
        else
        {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }

        cairo_matrix_t matrix = to_cairo_matrix(p->matrix());
        cairo_matrix_invert(&matrix);
        cairo_pattern_set_matrix(pattern, &matrix);
        return pattern;
    }

    return nullptr;
}

cairo_fill_rule_t to_cairo_fill_rule(WindRule fillRule)
{
    return fillRule == WindRuleEvenOdd ? CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING;
}

cairo_line_cap_t to_cairo_line_cap(LineCap cap)
{
    return cap == LineCapButt ? CAIRO_LINE_CAP_BUTT : cap == LineCapRound ? CAIRO_LINE_CAP_ROUND : CAIRO_LINE_CAP_SQUARE;
}

cairo_line_join_t to_cairo_line_join(LineJoin join)
{
    return join == LineJoinMiter ? CAIRO_LINE_JOIN_MITER : join == LineJoinRound ? CAIRO_LINE_JOIN_ROUND : CAIRO_LINE_JOIN_BEVEL;
}

cairo_operator_t to_cairo_operator(BlendMode mode)
{
    switch(mode)
    {
    case BlendModeClear:
        return CAIRO_OPERATOR_CLEAR;
    case BlendModeSrc:
        return CAIRO_OPERATOR_SOURCE;
    case BlendModeDst:
        return CAIRO_OPERATOR_DEST;
    case BlendModeSrc_Over:
        return CAIRO_OPERATOR_OVER;
    case BlendModeDst_Over:
        return CAIRO_OPERATOR_DEST_OVER;
    case BlendModeSrc_In:
        return CAIRO_OPERATOR_IN;
    case BlendModeDst_In:
        return CAIRO_OPERATOR_DEST_IN;
    case BlendModeSrc_Out:
        return CAIRO_OPERATOR_OUT;
    case BlendModeDst_Out:
        return CAIRO_OPERATOR_DEST_OUT;
    case BlendModeSrc_Atop:
        return CAIRO_OPERATOR_ATOP;
    case BlendModeDst_Atop:
        return CAIRO_OPERATOR_DEST_ATOP;
    case BlendModeXnor:
        return CAIRO_OPERATOR_XOR;
    case BlendModePlus:
        return CAIRO_OPERATOR_ADD;
    case BlendModeMultiply:
        return CAIRO_OPERATOR_MULTIPLY;
    case BlendModeScreen:
        return CAIRO_OPERATOR_SCREEN;
    case BlendModeOverlay:
        return CAIRO_OPERATOR_OVERLAY;
    case BlendModeDarken:
        return CAIRO_OPERATOR_DARKEN;
    case BlendModeLighten:
        return CAIRO_OPERATOR_LIGHTEN;
    case BlendModeColor_Dodge:
        return CAIRO_OPERATOR_COLOR_DODGE;
    case BlendModeColor_Burn:
        return CAIRO_OPERATOR_COLOR_BURN;
    case BlendModeHard_Light:
        return CAIRO_OPERATOR_HARD_LIGHT;
    case BlendModeSoft_Light:
        return CAIRO_OPERATOR_SOFT_LIGHT;
    case BlendModeDifference:
        return CAIRO_OPERATOR_DIFFERENCE;
    case BlendModeExclusion:
        return CAIRO_OPERATOR_EXCLUSION;
    }

    return CAIRO_OPERATOR_OVER;
}

} // namespace lunasvg
