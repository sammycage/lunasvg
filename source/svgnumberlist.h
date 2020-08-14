#ifndef SVGNUMBERLIST_H
#define SVGNUMBERLIST_H

#include "svglistproperty.h"
#include "svgnumber.h"

namespace lunasvg {

class SVGNumberList : public SVGListProperty<SVGNumber>
{
public:
    SVGNumberList();

    std::vector<double> values() const;
    void setValueAsString(const std::string& value);
    SVGProperty* clone() const;
    static PropertyType classType() { return PropertyTypeNumberList; }
};

typedef DOMSVGProperty<SVGNumberList> DOMSVGNumberList;

} // namespace lunasvg

#endif // SVGNUMBERLIST_H
