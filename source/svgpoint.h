#ifndef SVGPOINT_H
#define SVGPOINT_H

#include "svgproperty.h"
#include "point.h"

namespace lunasvg {

class SVGPoint : public SVGProperty
{
public:
    SVGPoint();

    double x() const { return m_value.x; }
    double y() const { return m_value.y; }
    void setX(double x) { m_value.x = x; }
    void setY(double y) { m_value.y = y; }

    void setValue(const Point& value) { m_value = value; }
    const Point& value() const { return  m_value; }

    void setValueAsString(const std::string& value);
    std::string valueAsString() const;
    SVGProperty* clone() const;
    static PropertyType classType() { return PropertyTypePoint; }

private:
    Point m_value;
};

} // namespace lunasvg

#endif // SVGPOINT_H
