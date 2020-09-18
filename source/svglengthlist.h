#ifndef SVGLENGTHLIST_H
#define SVGLENGTHLIST_H

#include "svglistproperty.h"
#include "svglength.h"

namespace lunasvg {

class SVGLengthList : public SVGListProperty<SVGLength>
{
public:
    SVGLengthList();

    std::vector<double> values(const RenderState& state, LengthMode mode = LengthModeBoth) const;
    void setValueAsString(const std::string& value);
    SVGPropertyBase* clone() const;
};

typedef DOMSVGProperty<SVGLengthList> DOMSVGLengthList;

} // namespace lunasvg

#endif // SVGLENGTHLIST_H
