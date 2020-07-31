#ifndef CANVASIMPL_H
#define CANVASIMPL_H

#include "canvas.h"

#include <cairo.h>

namespace lunasvg {

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

    cairo_t* cr() { return m_cr; }
    cairo_surface_t* surface() { return m_surface; }

private:
    cairo_surface_t* m_surface;
    cairo_t* m_cr;
};

} // namespace lunasvg

#endif // CANVASIMPL_H
