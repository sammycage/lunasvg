#ifndef SVGSTRINGLIST_H
#define SVGSTRINGLIST_H

#include "svglistproperty.h"
#include "svgstring.h"

namespace lunasvg {

class SVGStringList : public SVGListProperty<SVGString>
{
public:
    SVGStringList();

    void setValueAsString(const std::string& value);
    SVGProperty* clone() const;
    static PropertyType classType() { return PropertyTypeStringList; }
};

} // namespace lunasvg

#endif // SVGSTRINGLIST_H
