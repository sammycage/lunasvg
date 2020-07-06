#ifndef SVGSOILDCOLORELEMENT_H
#define SVGSOILDCOLORELEMENT_H

#include "svgpaintelement.h"

namespace lunasvg {

class SVGSolidColorElement : public SVGPaintElement
{
public:
    SVGSolidColorElement(SVGDocument* document);
    Paint getPaint(const RenderState&) const;
    SVGElementImpl* clone(SVGDocument* document) const;
};

} // namespace lunasvg

#endif // SVGSOILDCOLORELEMENT_H
