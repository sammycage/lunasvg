#include "svgnumberlist.h"

namespace lunasvg {

SVGNumberList::SVGNumberList()
    : SVGListProperty<SVGNumber>(PropertyTypeNumberList)
{
}

std::vector<double> SVGNumberList::values() const
{
    std::vector<double> v(length());
    for(unsigned int i = 0;i < length();i++)
        v[i] = at(i)->value();
    return v;
}

void SVGNumberList::setValueAsString(const std::string& value)
{
    clear();
    if(value.empty())
        return;

    const char* ptr = value.c_str();
    Utils::skipWs(ptr);
    double number;
    while(*ptr)
    {
        if(!Utils::parseNumber(ptr, number))
            return;
        SVGNumber* item = new SVGNumber;
        item->setValue(number);
        appendItem(item);
        Utils::skipWsComma(ptr);
    }
}

SVGProperty* SVGNumberList::clone() const
{
    SVGNumberList* property = new SVGNumberList();
    baseClone(property);

    return property;
}

} // namespace lunasvg
