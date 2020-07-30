#ifndef CANVAS_H
#define CANVAS_H

#include "svgenumeration.h"

#include <memory>
#include <vector>

namespace lunasvg {

enum BlendMode
{
    BlendModeClear,
    BlendModeSrc,
    BlendModeDst,
    BlendModeSrc_Over,
    BlendModeDst_Over,
    BlendModeSrc_In,
    BlendModeDst_In,
    BlendModeSrc_Out,
    BlendModeDst_Out,
    BlendModeSrc_Atop,
    BlendModeDst_Atop,
    BlendModeXnor,
    BlendModePlus,
    BlendModeMultiply,
    BlendModeScreen,
    BlendModeOverlay,
    BlendModeDarken,
    BlendModeLighten,
    BlendModeColor_Dodge,
    BlendModeColor_Burn,
    BlendModeHard_Light,
    BlendModeSoft_Light,
    BlendModeDifference,
    BlendModeExclusion
};

class Path;
class Rgb;
class Paint;
class StrokeData;
class AffineTransform;
class CanvasImpl;

class Canvas
{
public:
    Canvas();
    Canvas(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);
    Canvas(unsigned int width, unsigned int height);

    void reset(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);
    void reset(unsigned int width, unsigned int height);

    void clear(const Rgb& color);
    void blend(const Canvas& source, BlendMode mode, double opacity, double dx, double dy);
    void draw(const Path& path, const AffineTransform& matrix, WindRule fillRule, const Paint& fillPaint, const Paint& strokePaint, const StrokeData& strokeData);
    void updateLuminance();
    void convertToRGBA();

    unsigned char* data() const;
    unsigned int width() const;
    unsigned int height() const;
    unsigned int stride() const;

    CanvasImpl* impl() const { return m_impl.get(); }
    bool valid() const { return !!m_impl; }

private:
    std::shared_ptr<CanvasImpl> m_impl;
};

} // namespace lunasvg

#endif // CANVAS_H
