#ifndef CANVAS_H
#define CANVAS_H

#include "property.h"

#include <memory>

namespace lunasvg {

using GradientStop = std::pair<double, Color>;
using GradientStops = std::vector<GradientStop>;

class LinearGradientValues
{
public:
    LinearGradientValues(double x1, double y1, double x2, double y2);

public:
    double x1;
    double y1;
    double x2;
    double y2;
};

class RadialGradientValues
{
public:
    RadialGradientValues(double cx, double cy, double r, double fx, double fy);

public:
    double cx;
    double cy;
    double r;
    double fx;
    double fy;
};

using DashArray = std::vector<double>;

class DashData
{
public:
    DashData() = default;

public:
    DashArray array;
    double offset{1.0};
};

enum class TextureType
{
    Plain,
    Tiled
};

enum class BlendMode
{
    Src,
    Src_Over,
    Dst_In,
    Dst_Out
};

class CanvasImpl;

class Canvas
{
public:
    static std::shared_ptr<Canvas> create(unsigned int width, unsigned int height);
    static std::shared_ptr<Canvas> create(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);

    void setColor(const Color& color);
    void setLinearGradient(const LinearGradientValues& values, SpreadMethod spread, const GradientStops& stops, const Transform& transform);
    void setRadialGradient(const RadialGradientValues& values, SpreadMethod spread, const GradientStops& stops, const Transform& transform);
    void setTexture(const Canvas* source, TextureType type, const Transform& transform);

    void fill(const Path& path, const Transform& transform, WindRule winding, double opacity);
    void stroke(const Path& path, const Transform& transform, double width, LineCap cap, LineJoin join, double miterlimit, const DashData& dash, double opacity);
    void blend(const Canvas* source, BlendMode mode, double opacity);

    void clear(unsigned int value);
    void rgba();
    void luminance();

    unsigned int width() const;
    unsigned int height() const;
    unsigned int stride() const;
    unsigned char* data() const;

    ~Canvas();
private:
    Canvas(unsigned int width, unsigned int height);
    Canvas(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);

    std::unique_ptr<CanvasImpl> d;
};

} // namespace lunasvg

#endif // CANVAS_H
