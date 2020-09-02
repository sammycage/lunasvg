#include "svgpoint.h"

namespace lunasvg {

SVGPoint::SVGPoint()
    : SVGProperty(PropertyTypePoint)
{
}

void SVGPoint::setValueAsString(const std::string& value)
{
    m_value = Point();
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    Utils::parseNumber(ptr, m_value.x);
    Utils::skipWsComma(ptr);
    Utils::parseNumber(ptr, m_value.y);
}

std::string SVGPoint::valueAsString() const
{
    std::string out;
    out += Utils::toString(x());
    out += ' ';
    out += Utils::toString(y());

    return out;
}

SVGProperty* SVGPoint::clone() const
{
    SVGPoint* property = new SVGPoint();
    property->m_value = m_value;

    return property;
}

} // namespace lunasvg
