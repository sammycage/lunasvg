#include "svgdefselement.h"

namespace lunasvg {

SVGDefsElement::SVGDefsElement(SVGDocument* document) :
    SVGGraphicsElement(ElementIdDefs, document)
{
}

void SVGDefsElement::render(RenderContext& context) const
{
    context.skipElement();
}

SVGElementImpl* SVGDefsElement::clone(SVGDocument* document) const
{
    SVGDefsElement* e = new SVGDefsElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
