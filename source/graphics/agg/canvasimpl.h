#ifndef CANVASIMPL_H
#define CANVASIMPL_H

#include "canvas.h"

#include "agg_pixfmt_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_trans_affine.h"

namespace lunasvg {

typedef agg::pixfmt_bgra32 pixel_format_t;
typedef agg::renderer_base<pixel_format_t> renderer_base_t;
typedef agg::renderer_scanline_aa_solid<renderer_base_t> renderer_solid_t;
typedef agg::rasterizer_scanline_aa_nogamma<> rasterizer_t;

class Pattern;
class Gradient;

class CanvasImpl
{
public:
    ~CanvasImpl();
    CanvasImpl(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);
    CanvasImpl(unsigned int width, unsigned int height);

    void clear(const Rgb& color);
    void blend(const Canvas& source, BlendMode mode, double opacity, double dx, double dy);
    void draw(const Path& path, const AffineTransform& matrix, WindRule fillRule, const Paint& fillPaint, const Paint& strokePaint, const StrokeData& strokeData);
    void updateLuminance();
    void convertToRGBA();

    unsigned char* data() const;
    unsigned int width() const;
    unsigned int height() const;
    unsigned int stride() const;

private:
    template<typename gradient_adaptor_t>
    void render_gradient_spread(gradient_adaptor_t& gradient_adaptor, const Gradient* gradient, double opacity, const agg::trans_affine& matrix);

    template<typename gradient_function_t>
    void render_gradient(gradient_function_t& gradient_function, const Gradient* gradient, double opacity, const agg::trans_affine& matrix);

    template<typename source_t, typename span_generator_t>
    void render_pattern(const Pattern* pattern, const agg::trans_affine& matrix);

    void render_scanlines(const Paint& paint, const agg::trans_affine& matrix);

private:
    std::unique_ptr<std::uint8_t[]> m_data;
    agg::rendering_buffer m_buffer;
    pixel_format_t m_pixelFormat;
    renderer_base_t m_rendererBase;
    renderer_solid_t m_rendererSolid;
    rasterizer_t m_rasterizer;
};

} // namespace lunasvg

#endif // CANVASIMPL_H
