#include "svglengthlist.h"

namespace lunasvg {

SVGLengthList::SVGLengthList()
    : SVGListProperty<SVGLength>(PropertyTypeLengthList)
{
}

std::vector<double> SVGLengthList::values(const RenderState& state, LengthMode mode) const
{
    std::vector<double> v(length());
    for(unsigned int i = 0;i < length();i++)
        v[i] = at(i)->value(state, mode);
    return v;
}

void SVGLengthList::setValueAsString(const std::string& value)
{
    clear();
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    double number;
    LengthUnit unit;
    while(*ptr)
    {
        if(!SVGLength::parseLength(ptr, number, unit))
            return;
        SVGLength* item = new SVGLength(number, unit);
        appendItem(item);
        Utils::skipWsComma(ptr);
    }
}

SVGProperty* SVGLengthList::clone() const
{
    SVGLengthList* property = new SVGLengthList();
    baseClone(property);

    return property;
}

} // namespace lunasvg
