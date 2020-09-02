#include "svgpointlist.h"

namespace lunasvg {

SVGPointList::SVGPointList()
    : SVGListProperty<SVGPoint>(PropertyTypePointList)
{
}

std::vector<Point> SVGPointList::values() const
{
    std::vector<Point> v(length());
    for(unsigned int i = 0;i < length();i++)
        v[i] = at(i)->value();
    return v;
}

void SVGPointList::setValueAsString(const std::string& value)
{
    clear();
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    double x;
    double y;
    while(*ptr)
    {
        if(!Utils::parseNumber(ptr, x)
                || !Utils::skipWsComma(ptr)
                || !Utils::parseNumber(ptr, y))
            return;
        SVGPoint* item = new SVGPoint;
        item->setValue(Point(x, y));
        appendItem(item);
        Utils::skipWsComma(ptr);
    }
}

SVGProperty* SVGPointList::clone() const
{
    SVGPointList* property = new SVGPointList();
    baseClone(property);

    return property;
}

} // namespace lunasvg
