#ifndef PAINT_H
#define PAINT_H

#include <memory>

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

} // namespace lunasvg

#endif // PAINT_H
