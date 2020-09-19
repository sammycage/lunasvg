#ifndef PAINT_H
#define PAINT_H

#include "svgenumeration.h"
#include "affinetransform.h"
#include "canvas.h"

#include <memory>
#include <vector>
#include <utility>

namespace lunasvg {

enum PaintType
{
    PaintTypeNone,
    PaintTypeColor,
    PaintTypeGradient,
    PaintTypePattern
};

class Rgb;
class Gradient;
class Pattern;
class PaintData;

class Paint
{
public:
    ~Paint();
    Paint();
    Paint(const Rgb& color);
    Paint(const Gradient& gradient);
    Paint(const Pattern& pattern);

    void setColor(const Rgb& color);
    void setGradient(const Gradient& gradient);
    void setPattern(const Pattern& pattern);
    void setOpacity(double opacity);

    const Rgb* color() const;
    const Gradient* gradient() const;
    const Pattern* pattern() const;
    bool isNone() const;
    double opacity() const;
    PaintType type() const;

private:
    std::shared_ptr<PaintData> m_data;
};

class Rgb
{
public:
    Rgb();
    Rgb(unsigned int value);
    Rgb(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255);

    bool operator==(const Rgb& rgb) const;
    bool operator!=(const Rgb& rgb) const;
    unsigned int value() const;

public:
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
};

#define KRgbBlack Rgb(0, 0, 0)
#define KRgbWhite Rgb(255, 255, 255)
#define KRgbRed Rgb(255, 0, 0)
#define KRgbGreen Rgb(0, 255, 0)
#define KRgbBlue Rgb(0, 0, 255)
#define KRgbYellow Rgb(255, 255, 0)
#define KRgbTransparent Rgb(0, 0, 0, 0)

enum GradientType
{
   GradientTypeUnknown,
   GradientTypeLinear,
   GradientTypeRadial
};

typedef std::pair<double, Rgb> GradientStop;
typedef std::vector<GradientStop> GradientStops;

class Gradient
{
public:
    Gradient();

    void setSpread(SpreadMethod spread) { m_spread = spread; }
    void setStops(const GradientStops& stops) { m_stops = stops; }
    void setMatrix(const AffineTransform& matrix) { m_matrix = matrix; }
    SpreadMethod spread() const { return m_spread; }
    const GradientStops& stops() const { return m_stops; }
    const AffineTransform& matrix() const { return m_matrix; }
    GradientType type() const { return m_type; }
    const double* values() const { return m_values; }

private:
    friend class LinearGradient;
    friend class RadialGradient;
    GradientType m_type;
    GradientStops m_stops;
    SpreadMethod m_spread;
    AffineTransform m_matrix;
    double m_values[5];
};

class LinearGradient : public Gradient
{
public:
   LinearGradient();
   LinearGradient(double x1, double y1, double x2, double y2);

   void setX1(double x1) { m_values[0] = x1; }
   void setY1(double y1) { m_values[1] = y1; }
   void setX2(double x2) { m_values[2] = x2; }
   void setY2(double y2) { m_values[3] = y2; }
   double x1() const { return m_values[0]; }
   double y1() const { return m_values[1]; }
   double x2() const { return m_values[2]; }
   double y2() const { return m_values[3]; }
};

class RadialGradient : public Gradient
{
public:
   RadialGradient();
   RadialGradient(double cx, double cy, double r, double fx, double fy);

   void setCx(double cx) { m_values[0] = cx; }
   void setCy(double cy) { m_values[1] = cy; }
   void setR(double r) { m_values[2] = r; }
   void setFx(double fx) { m_values[3] = fx; }
   void setFy(double fy) { m_values[4] = fy; }
   double cx() const { return m_values[0]; }
   double cy() const { return m_values[1]; }
   double r() const { return m_values[2]; }
   double fx() const { return m_values[3]; }
   double fy() const { return m_values[4]; }
};

enum TileMode
{
    TileModePad,
    TileModeReflect,
    TileModeRepeat
};

class Pattern
{
public:
    Pattern();
    Pattern(const Canvas& canvas);

    void setTile(const Canvas& canvas) { m_tile = canvas; }
    void setTileMode(TileMode mode) { m_tileMode = mode; }
    void setMatrix(const AffineTransform& matrix) { m_matrix = matrix; }
    const Canvas& tile() const { return m_tile; }
    TileMode tileMode() const { return m_tileMode; }
    const AffineTransform& matrix() const { return m_matrix; }

private:
    Canvas m_tile;
    TileMode m_tileMode;
    AffineTransform m_matrix;
};

} // namespace lunasvg

#endif // PAINT_H
