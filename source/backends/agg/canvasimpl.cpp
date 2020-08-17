#include "canvasimpl.h"
#include "pathiterator.h"
#include "affinetransform.h"
#include "paint.h"
#include "gradient.h"
#include "pattern.h"
#include "rgb.h"
#include "strokedata.h"

#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_curve.h"
#include "agg_gradient_lut.h"
#include "agg_span_gradient.h"
#include "agg_span_allocator.h"
#include "agg_span_interpolator_linear.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"

#include <numeric>

namespace lunasvg {

inline agg::filling_rule_e to_agg_fill_rule(WindRule fillRule)
{
    return fillRule==WindRuleNonZero ? agg::fill_non_zero : agg::fill_even_odd;
}

inline agg::line_cap_e to_agg_line_cap(LineCap cap)
{
    return cap==LineCapButt ? agg::butt_cap : cap==LineCapRound ? agg::round_cap : agg::square_cap;
}

inline agg::line_join_e to_agg_line_join(LineJoin join)
{
    return join==LineJoinBevel ? agg::bevel_join : join==LineJoinMiter ? agg::miter_join : agg::round_join;
}

inline agg::trans_affine to_agg_transform(const AffineTransform& matrix)
{
    const double* m = matrix.getMatrix();
    return agg::trans_affine(m[0], m[1], m[2], m[3], m[4], m[5]);
}

inline agg::comp_op_e to_agg_comp_op(BlendMode mode)
{
    return mode == BlendModeDst_In ? agg::comp_op_dst_in : agg::comp_op_src_over;
}

CanvasImpl::~CanvasImpl()
{
}

CanvasImpl::CanvasImpl(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride)
{
    m_buffer.attach(data, width, height, int(stride));
    m_pixelFormat.attach(m_buffer);
    m_rendererBase.attach(m_pixelFormat);
    m_rendererSolid.attach(m_rendererBase);
    m_rasterizer.clip_box(0, 0, width, height);
}

CanvasImpl::CanvasImpl(unsigned int width, unsigned int height)
{
    m_data.reset(new std::uint8_t[width*height*4]);
    m_buffer.attach(m_data.get(), width, height, int(width * 4));
    m_buffer.clear(0);
    m_pixelFormat.attach(m_buffer);
    m_rendererBase.attach(m_pixelFormat);
    m_rendererSolid.attach(m_rendererBase);
    m_rasterizer.clip_box(0, 0, width, height);
}

void CanvasImpl::clear(const Rgb& color)
{
    m_rendererBase.clear(agg::rgba8(color.r, color.g, color.b, color.a));
}

unsigned char* CanvasImpl::data() const
{
    return const_cast<std::uint8_t*>(m_buffer.buf());
}

unsigned int CanvasImpl::width() const
{
    return m_buffer.width();
}

unsigned int CanvasImpl::height() const
{
    return m_buffer.height();
}

unsigned int CanvasImpl::stride() const
{
    return std::uint32_t(m_buffer.stride());
}

void CanvasImpl::blend(const Canvas& source, BlendMode mode, double opacity, double dx, double dy)
{
    typedef agg::comp_op_adaptor_rgba<agg::rgba8, agg::order_bgra> blender_adaptor_t;
    typedef agg::pixfmt_custom_blend_rgba<blender_adaptor_t, agg::rendering_buffer> pixfmt_blender_t;
    typedef agg::renderer_base<pixfmt_blender_t> renderer_base_blender_t;

    pixfmt_blender_t pixfmt(m_buffer, to_agg_comp_op(mode));
    renderer_base_blender_t ren(pixfmt);
    ren.blend_from(source.impl()->m_pixelFormat, nullptr, int(dx), int(dy), agg::cover_type(opacity * 255));
}

void CanvasImpl::draw(const Path& path, const AffineTransform& matrix, WindRule fillRule, const Paint& fillPaint, const Paint& strokePaint, const StrokeData& strokeData)
{
    if(fillPaint.isNone() && strokePaint.isNone())
        return;

    agg::trans_affine _matrix = to_agg_transform(matrix);
    agg::path_storage _path;
    PathIterator it(path);
    double c[6];
    while(!it.isDone())
    {
        switch(it.currentSegment(c))
        {
        case SegTypeMoveTo:
            _path.move_to(c[0], c[1]);
            break;
        case SegTypeLineTo:
            _path.line_to(c[0], c[1]);
            break;
        case SegTypeQuadTo:
            _path.curve3(c[0], c[1], c[2], c[3]);
            break;
        case SegTypeCubicTo:
            _path.curve4(c[0], c[1], c[2], c[3], c[4], c[5]);
            break;
        case SegTypeClose:
            _path.end_poly();
            _path.move_to(c[0], c[1]);
            break;
        }
        it.next();
    }

    if(!fillPaint.isNone())
    {
        m_rasterizer.reset();
        m_rasterizer.filling_rule(to_agg_fill_rule(fillRule));
        agg::conv_curve<agg::path_storage> curved(_path);
        curved.approximation_scale(_matrix.scale());
        curved.angle_tolerance(0.0);
        agg::conv_transform<agg::conv_curve<agg::path_storage>> curved_transform(curved, _matrix);
        m_rasterizer.add_path(curved_transform);
        render_scanlines(fillPaint, _matrix);
    }

    if(!strokePaint.isNone())
    {
        m_rasterizer.reset();
        m_rasterizer.filling_rule(agg::fill_non_zero);
        agg::conv_curve<agg::path_storage> curved(_path);
        curved.approximation_scale(_matrix.scale());
        curved.angle_tolerance(0.0);
        if(std::accumulate(strokeData.dash().begin(), strokeData.dash().end(), 0.0) != 0.0)
        {
            agg::conv_dash<agg::conv_curve<agg::path_storage>> curved_dash(curved);
            const std::vector<double>& dashes = strokeData.dash();
            std::size_t num_dash = dashes.size() % 2 == 0 ? dashes.size() : dashes.size() * 2;
            for(unsigned int i = 0;i < num_dash;i += 2)
                curved_dash.add_dash(dashes[i % dashes.size()], dashes[(i+1)%dashes.size()]);
            curved_dash.dash_start(strokeData.dashOffset());
            agg::conv_stroke<agg::conv_dash<agg::conv_curve<agg::path_storage>>> curved_dash_stroke(curved_dash);
            curved_dash_stroke.width(strokeData.width());
            curved_dash_stroke.line_cap(to_agg_line_cap(strokeData.cap()));
            curved_dash_stroke.line_join(to_agg_line_join(strokeData.join()));
            curved_dash_stroke.miter_limit(strokeData.miterLimit());
            agg::conv_transform<agg::conv_stroke<agg::conv_dash<agg::conv_curve<agg::path_storage>>>> curved_dash_stroke_transform(curved_dash_stroke, _matrix);
            m_rasterizer.add_path(curved_dash_stroke_transform);
        }
        else
        {
            agg::conv_stroke<agg::conv_curve<agg::path_storage>> curved_stroke(curved);
            curved_stroke.width(strokeData.width());
            curved_stroke.line_cap(to_agg_line_cap(strokeData.cap()));
            curved_stroke.line_join(to_agg_line_join(strokeData.join()));
            curved_stroke.miter_limit(strokeData.miterLimit());
            agg::conv_transform<agg::conv_stroke<agg::conv_curve<agg::path_storage>>> curved_stroke_transform(curved_stroke, _matrix);
            m_rasterizer.add_path(curved_stroke_transform);
        }

        render_scanlines(strokePaint, _matrix);
    }
}

void CanvasImpl::updateLuminance()
{
    std::uint32_t width = this->width();
    std::uint32_t height = this->height();
    std::uint32_t stride = this->stride();
    std::uint8_t* data = this->data();
    for(std::uint32_t y = 0;y < height;y++)
    {
        std::uint8_t* row = data + stride * y;
        for(std::uint32_t x = 0;x < width;x++)
        {
            std::uint8_t b = row[0];
            std::uint8_t g = row[1];
            std::uint8_t r = row[2];

            row[3] = (2*r + 3*g + b) / 6;

            row += 4;
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
        std::uint8_t* row = data + stride * y;
        for(std::uint32_t x = 0;x < width;x++)
        {
            std::uint8_t a = row[3];
            if(a != 0)
            {
                std::uint8_t r = row[2];
                std::uint8_t g = row[1];
                std::uint8_t b = row[0];

                row[0] = (r * 255) / a;
                row[1] = (g * 255) / a;
                row[2] = (b * 255) / a;
                row[3] = a;
            }
            else
            {
                row[0] = 0;
                row[1] = 0;
                row[2] = 0;
                row[3] = 0;
            }

            row += 4;
        }
    }
}

static const double KGradientScale = 100.0;

template<typename gradient_adaptor_t>
void CanvasImpl::render_gradient_spread(gradient_adaptor_t& gradient_adaptor, const Gradient* gradient, double opacity, const agg::trans_affine& matrix)
{
    agg::trans_affine transform;
    if(gradient->type() == GradientTypeLinear)
    {
        const LinearGradient* linear = static_cast<const LinearGradient*>(gradient);
        double dx = linear->x2() - linear->x1();
        double dy = linear->y2() - linear->y1();
        transform *= agg::trans_affine_scaling(std::sqrt(dx * dx + dy * dy));
        transform *= agg::trans_affine_rotation(std::atan2(dy, dx));
        transform *= agg::trans_affine_translation(linear->x1(), linear->y1());
    }
    else
    {
        const RadialGradient* radial = static_cast<const RadialGradient*>(gradient);
        transform *= agg::trans_affine_scaling(radial->r());
        transform *= agg::trans_affine_translation(radial->cx(), radial->cy());
    }

    transform.premultiply(agg::trans_affine_scaling(1.0 / KGradientScale));
    transform.multiply(to_agg_transform(gradient->matrix()));
    transform.multiply(matrix);
    transform.invert();

    typedef agg::gradient_lut<agg::color_interpolator<agg::rgba8>> color_function_t;
    typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_t;
    typedef agg::span_gradient<agg::rgba8, interpolator_t, gradient_adaptor_t, color_function_t> span_gradient_t;
    typedef agg::span_allocator<agg::rgba8> span_allocator_t;

    color_function_t colorFunction;
    const GradientStops& stops = gradient->stops();
    for(unsigned int i = 0;i < stops.size();i++)
    {
        double offset = stops[i].first;
        const Rgb& c = stops[i].second;
        colorFunction.add_color(offset, agg::rgba8(c.r, c.g, c.b, std::uint8_t(c.a * opacity)));
    }

    colorFunction.build_lut();

    interpolator_t interpolator(transform);
    span_gradient_t sg(interpolator, gradient_adaptor, colorFunction, 0.0, KGradientScale);
    span_allocator_t allocator;

    agg::scanline_u8 scanline;
    agg::render_scanlines_aa(m_rasterizer, scanline, m_rendererBase, allocator, sg);
}

template<typename gradient_function_t>
void CanvasImpl::render_gradient(gradient_function_t& gradient_function, const Gradient* gradient, double opacity, const agg::trans_affine& matrix)
{
    if(gradient->spread() == SpreadMethodReflect)
    {
        typedef agg::gradient_reflect_adaptor<gradient_function_t> gradient_adaptor_t;
        gradient_adaptor_t adaptor(gradient_function);
        render_gradient_spread(adaptor, gradient, opacity, matrix);
    }
    else if(gradient->spread() == SpreadMethodRepeat)
    {
        typedef agg::gradient_repeat_adaptor<gradient_function_t> gradient_adaptor_t;
        gradient_adaptor_t adaptor(gradient_function);
        render_gradient_spread(adaptor, gradient, opacity, matrix);
    }
    else
    {
        render_gradient_spread(gradient_function, gradient, opacity, matrix);
    }
}

template<typename source_t, typename span_generator_t>
void CanvasImpl::render_pattern(const Pattern* pattern, const agg::trans_affine& matrix)
{
    typedef agg::span_interpolator_linear<> span_interpolator_t;
    typedef agg::span_allocator<agg::rgba8> span_allocator_t;
    typedef agg::renderer_scanline_aa<renderer_base_t, span_allocator_t, span_generator_t> pattern_renderer_t;

    agg::trans_affine transform(to_agg_transform(pattern->matrix()));
    transform.multiply(matrix);
    transform.invert();

    source_t source(pattern->tile().impl()->m_pixelFormat);
    span_interpolator_t span_interpolator(transform);
    span_generator_t span_generator(source, span_interpolator);

    span_allocator_t span_allocator;
    pattern_renderer_t pattern_renderer(m_rendererBase, span_allocator, span_generator);

    agg::scanline_u8 scanline;
    agg::render_scanlines(m_rasterizer, scanline, pattern_renderer);
}

void CanvasImpl::render_scanlines(const Paint& paint, const agg::trans_affine& matrix)
{
    if(paint.type() == PaintTypeColor)
    {
        const Rgb* c = paint.color();
        agg::rgba8 color(c->r, c->g, c->b, std::uint8_t(c->a * paint.opacity()));
        m_rendererSolid.color(color);
        agg::scanline_p8 scanline;
        agg::render_scanlines(m_rasterizer, scanline, m_rendererSolid);
    }
    else if(paint.type() == PaintTypeGradient)
    {
        const Gradient* g = paint.gradient();
        if(g->type() == GradientTypeLinear)
        {
            agg::gradient_x gradient_function;
            render_gradient(gradient_function, g, paint.opacity(), matrix);
        }
        else
        {
            const RadialGradient* radial = static_cast<const RadialGradient*>(g);
            agg::gradient_radial_focus gradient_function(KGradientScale, KGradientScale*(radial->fx() - radial->cx())/ radial->r(), KGradientScale*(radial->fy() - radial->cy())/ radial->r());
            render_gradient(gradient_function, g, paint.opacity(), matrix);
        }
    }
    else if(paint.type() == PaintTypePattern)
    {
        const Pattern* p = paint.pattern();
        if(p->tileMode() == TileModeReflect)
        {
            typedef agg::wrap_mode_reflect wrap_reflect_t;
            typedef agg::image_accessor_wrap<pixel_format_t, wrap_reflect_t, wrap_reflect_t> source_reflect_t;
            typedef agg::span_interpolator_linear<> interpolator_t;
            typedef agg::span_image_filter_rgba_nn<source_reflect_t,  interpolator_t> nearest_reflect_t;

            render_pattern<source_reflect_t, nearest_reflect_t>(p, matrix);
        }
        else if(p->tileMode() == TileModeRepeat)
        {
            typedef agg::wrap_mode_repeat wrap_repeat_t;
            typedef agg::image_accessor_wrap<pixel_format_t, wrap_repeat_t, wrap_repeat_t> source_repeat_t;
            typedef agg::span_interpolator_linear<> interpolator_t;
            typedef agg::span_image_filter_rgba_nn<source_repeat_t,  interpolator_t> nearest_repeat_t;

            render_pattern<source_repeat_t, nearest_repeat_t>(p, matrix);
        }
    }
}

} // namespace lunasvg
