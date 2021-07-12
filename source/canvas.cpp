#include "canvas.h"

#include "plutovg.h"

namespace lunasvg {

static plutovg_matrix_t to_plutovg_matrix(const Transform& matrix);
static plutovg_fill_rule_t to_plutovg_fill_rule(WindRule winding);
static plutovg_operator_t to_plutovg_operator(BlendMode mode);
static plutovg_line_cap_t to_plutovg_line_cap(LineCap cap);
static plutovg_line_join_t to_plutovg_line_join(LineJoin join);
static plutovg_spread_method_t to_plutovg_spread_methood(SpreadMethod spread);
static void to_plutovg_stops(plutovg_gradient_t* gradient, const GradientStops& stops);
static void to_plutovg_path(plutovg_t* pluto, const Path& path);

class CanvasImpl
{
public:
    CanvasImpl(int width, int height);
    CanvasImpl(unsigned char* data, int width, int height, int stride);
    ~CanvasImpl();

public:
    plutovg_surface_t* surface;
    plutovg_t* pluto;
};

CanvasImpl::CanvasImpl(int width, int height)
    : surface(plutovg_surface_create(width, height)),
      pluto(plutovg_create(surface))
{
}

CanvasImpl::CanvasImpl(unsigned char* data, int width, int height, int stride)
    : surface(plutovg_surface_create_for_data(data, width, height, stride)),
      pluto(plutovg_create(surface))
{
}

CanvasImpl::~CanvasImpl()
{
    plutovg_surface_destroy(surface);
    plutovg_destroy(pluto);
}

void Canvas::setMatrix(const Transform& matrix)
{
    auto transform = to_plutovg_matrix(matrix);
    plutovg_set_matrix(d->pluto, &transform);
}

void Canvas::setOpacity(double opacity)
{
    plutovg_set_opacity(d->pluto, opacity);
}

void Canvas::setColor(const Color &color)
{
    plutovg_set_source_rgba(d->pluto, color.r, color.g, color.b, color.a);
}

void Canvas::setLinearGradient(const LinearGradientValues& values, const Transform& matrix, SpreadMethod spread, const GradientStops& stops)
{
    auto gradient = plutovg_gradient_create_linear(values.x1, values.y1, values.x2, values.y2);
    auto transform = to_plutovg_matrix(matrix);
    to_plutovg_stops(gradient, stops);
    plutovg_gradient_set_spread(gradient, to_plutovg_spread_methood(spread));
    plutovg_gradient_set_matrix(gradient, &transform);
    plutovg_set_source_gradient(d->pluto, gradient);
    plutovg_gradient_destroy(gradient);
}

void Canvas::setRadialGradient(const RadialGradientValues& values, const Transform& matrix, SpreadMethod spread, const GradientStops& stops)
{
    auto gradient = plutovg_gradient_create_radial(values.cx, values.cy, values.r, values.fx, values.fy, 0);
    auto transform = to_plutovg_matrix(matrix);
    to_plutovg_stops(gradient, stops);
    plutovg_gradient_set_spread(gradient, to_plutovg_spread_methood(spread));
    plutovg_gradient_set_matrix(gradient, &transform);
    plutovg_set_source_gradient(d->pluto, gradient);
    plutovg_gradient_destroy(gradient);
}

void Canvas::setTexture(const Canvas* source, TextureType type, const Transform& matrix)
{
    auto texture = plutovg_texture_create(source->d->surface);
    auto transform = to_plutovg_matrix(matrix);
    if(type == TextureType::Plain)
        plutovg_texture_set_type(texture, plutovg_texture_type_plain);
    else
        plutovg_texture_set_type(texture, plutovg_texture_type_tiled);

    plutovg_texture_set_matrix(texture, &transform);
    plutovg_set_source_texture(d->pluto, texture);
    plutovg_texture_destroy(texture);
}

void Canvas::setWinding(WindRule winding)
{
    plutovg_set_fill_rule(d->pluto, to_plutovg_fill_rule(winding));
}

void Canvas::setLineWidth(double width)
{
    plutovg_set_line_width(d->pluto, width);
}

void Canvas::setLineCap(LineCap cap)
{
    plutovg_set_line_cap(d->pluto, to_plutovg_line_cap(cap));
}

void Canvas::setLineJoin(LineJoin join)
{
    plutovg_set_line_join(d->pluto, to_plutovg_line_join(join));
}

void Canvas::setMiterLimit(double miterlimit)
{
    plutovg_set_miter_limit(d->pluto, miterlimit);
}

void Canvas::setDash(const DashData& dash)
{
    plutovg_set_dash(d->pluto, dash.offset, dash.array.data(), static_cast<int>(dash.array.size()));
}

void Canvas::fill(const Path& path)
{
    plutovg_new_path(d->pluto);
    to_plutovg_path(d->pluto, path);
    plutovg_set_operator(d->pluto, plutovg_operator_src_over);
    plutovg_fill(d->pluto);
}

void Canvas::stroke(const Path& path)
{
    plutovg_new_path(d->pluto);
    to_plutovg_path(d->pluto, path);
    plutovg_set_operator(d->pluto, plutovg_operator_src_over);
    plutovg_stroke(d->pluto);
}

void Canvas::blend(const Canvas* source, BlendMode mode, double opacity)
{
    plutovg_set_source_surface(d->pluto, source->d->surface, 0, 0);
    plutovg_set_operator(d->pluto, to_plutovg_operator(mode));
    plutovg_set_opacity(d->pluto, opacity);
    plutovg_identity_matrix(d->pluto);
    plutovg_paint(d->pluto);
}

void Canvas::clear(unsigned int value)
{
    auto r = (value >> 24) & 0xFF;
    auto g = (value >> 16) & 0xFF;
    auto b = (value >> 8) & 0xFF;
    auto a = (value >> 0) & 0xFF;

    plutovg_set_operator(d->pluto, plutovg_operator_src);
    plutovg_set_source_rgba(d->pluto, r, g, b, a);
    plutovg_paint(d->pluto);
}

void Canvas::rgba()
{
    auto data = plutovg_surface_get_data(d->surface);
    auto width = plutovg_surface_get_width(d->surface);
    auto height = plutovg_surface_get_height(d->surface);
    auto stride = plutovg_surface_get_stride(d->surface);
    for(int y = 0;y < height;y++)
    {
        auto row = reinterpret_cast<uint32_t*>(data + stride * y);
        for(int x = 0;x < width;x++)
        {
            auto pixel = row[x];
            auto a = (pixel >> 24) & 0xFF;
            if(a == 0)
                continue;

            auto r = (pixel >> 16) & 0xFF;
            auto g = (pixel >> 8) & 0xFF;
            auto b = (pixel >> 0) & 0xFF;
            if(a != 255)
            {
                r = (r * 255) / a;
                g = (g * 255) / a;
                b = (b * 255) / a;
            }

            row[x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
}

void Canvas::luminance()
{
    auto data = plutovg_surface_get_data(d->surface);
    auto width = plutovg_surface_get_width(d->surface);
    auto height = plutovg_surface_get_height(d->surface);
    auto stride = plutovg_surface_get_stride(d->surface);
    for(int y = 0;y < height;y++)
    {
        auto row = reinterpret_cast<uint32_t*>(data + stride * y);
        for(int x = 0;x < width;x++)
        {
            auto pixel = row[x];
            auto r = (pixel >> 16) & 0xFF;
            auto g = (pixel >> 8) & 0xFF;
            auto b = (pixel >> 0) & 0xFF;
            auto l = (2*r + 3*g + b) / 6;

            row[x] = l << 24;
        }
    }
}

unsigned int Canvas::width() const
{
    return static_cast<unsigned int>(plutovg_surface_get_width(d->surface));
}

unsigned int Canvas::height() const
{
    return static_cast<unsigned int>(plutovg_surface_get_height(d->surface));
}

unsigned int Canvas::stride() const
{
    return static_cast<unsigned int>(plutovg_surface_get_stride(d->surface));
}

unsigned char* Canvas::data() const
{
    return plutovg_surface_get_data(d->surface);
}

std::shared_ptr<Canvas> Canvas::create(unsigned int width, unsigned int height)
{
    return std::shared_ptr<Canvas>(new Canvas(width, height));
}

std::shared_ptr<Canvas> Canvas::create(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride)
{
    return std::shared_ptr<Canvas>(new Canvas(data, width, height, stride));
}

Canvas::~Canvas()
{
}

Canvas::Canvas(unsigned int width, unsigned int height)
    : d(new CanvasImpl(static_cast<int>(width), static_cast<int>(height)))
{
}

Canvas::Canvas(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride)
    : d(new CanvasImpl(data, static_cast<int>(width), static_cast<int>(height), static_cast<int>(stride)))
{
}

LinearGradientValues::LinearGradientValues(double x1, double y1, double x2, double y2)
    : x1(x1), y1(y1), x2(x2), y2(y2)
{
}

RadialGradientValues::RadialGradientValues(double cx, double cy, double r, double fx, double fy)
    : cx(cx), cy(cy), r(r), fx(fx), fy(fy)
{
}

plutovg_matrix_t to_plutovg_matrix(const Transform& matrix)
{
    plutovg_matrix_t m;
    plutovg_matrix_init(&m, matrix.m00, matrix.m10, matrix.m01, matrix.m11, matrix.m02, matrix.m12);
    return m;
}

plutovg_fill_rule_t to_plutovg_fill_rule(WindRule winding)
{
    return winding == WindRule::EvenOdd ? plutovg_fill_rule_even_odd : plutovg_fill_rule_non_zero;
}

plutovg_operator_t to_plutovg_operator(BlendMode mode)
{
    return mode == BlendMode::Dst_In ? plutovg_operator_dst_in : plutovg_operator_src_over;
}

plutovg_line_cap_t to_plutovg_line_cap(LineCap cap)
{
    return cap == LineCap::Butt ? plutovg_line_cap_butt : cap == LineCap::Round ? plutovg_line_cap_round : plutovg_line_cap_square;
}

plutovg_line_join_t to_plutovg_line_join(LineJoin join)
{
    return join == LineJoin::Miter ? plutovg_line_join_miter : join == LineJoin::Round ? plutovg_line_join_round : plutovg_line_join_bevel;
}

static plutovg_spread_method_t to_plutovg_spread_methood(SpreadMethod spread)
{
    return spread == SpreadMethod::Pad ? plutovg_spread_method_pad : spread == SpreadMethod::Reflect ? plutovg_spread_method_reflect : plutovg_spread_method_repeat;
}

static void to_plutovg_stops(plutovg_gradient_t* gradient, const GradientStops& stops)
{
    for(auto& stop : stops)
    {
        auto offset = std::get<0>(stop);
        auto& color = std::get<1>(stop);
        plutovg_gradient_add_stop_rgba(gradient, offset, color.r, color.g, color.b, color.a);
    }
}

void to_plutovg_path(plutovg_t* pluto, const Path& path)
{
    PathIterator it(path);
    std::array<Point, 3> p;
    while(!it.isDone())
    {
        switch(it.currentSegment(p)) {
        case PathCommand::MoveTo:
            plutovg_move_to(pluto, p[0].x, p[0].y);
            break;
        case PathCommand::LineTo:
            plutovg_line_to(pluto, p[0].x, p[0].y);
            break;
        case PathCommand::CubicTo:
            plutovg_cubic_to(pluto, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            break;
        case PathCommand::Close:
            plutovg_close_path(pluto);
            break;
        }

        it.next();
    }
}

} // namespace lunasvg
