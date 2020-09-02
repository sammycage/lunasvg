#include "svgnumber.h"

namespace lunasvg {

SVGNumber::SVGNumber()
    : SVGProperty(PropertyTypeNumber),
      m_value(0)
{
}

void SVGNumber::setValueAsString(const std::string& value)
{
    m_value = 0;
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    Utils::parseNumber(ptr, m_value);
}

std::string SVGNumber::valueAsString() const
{
    return Utils::toString(m_value);
}

SVGProperty* SVGNumber::clone() const
{
    SVGNumber* property = new SVGNumber();
    property->m_value = m_value;

    return property;
}

SVGNumberPercentage::SVGNumberPercentage()
{
}

void SVGNumberPercentage::setValueAsString(const std::string& value)
{
    m_value = 0;
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    if(!Utils::parseNumber(ptr, m_value))
        return;

    if(Utils::skipDesc(ptr, "%", 1))
        m_value /= 100.0;
    m_value = (m_value < 0.0) ? 0.0 : (m_value > 1.0) ? 1.0 : m_value;
}

SVGProperty* SVGNumberPercentage::clone() const
{
    SVGNumberPercentage* property = new SVGNumberPercentage();
    property->m_value = m_value;

    return property;
}

} // namespace lunasvg
