#include "svgpolylineelement.h"

namespace lunasvg {

SVGPolylineElement::SVGPolylineElement(SVGDocument* document)
    : SVGPolyElement(DOMElementIdPolyline, document)
{
}

Path SVGPolylineElement::makePath(const RenderState&) const
{
    Path path;
    path.addPoints(points().property()->values());
    return path;
}

SVGElementImpl* SVGPolylineElement::clone(SVGDocument* document) const
{
    SVGPolylineElement* e = new SVGPolylineElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
