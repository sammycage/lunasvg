#include "svgpolylineelement.h"

namespace lunasvg {

SVGPolylineElement::SVGPolylineElement(SVGDocument* document) :
    SVGPolyElement(ElementIdPolyline, document)
{
}

Rect SVGPolylineElement::makePathAndBoundingBox(const RenderState&, Path& path) const
{
    path.addPoints(points().property()->values());
    return path.boundingBox();
}

SVGElementImpl* SVGPolylineElement::clone(SVGDocument* document) const
{
    SVGPolylineElement* e = new SVGPolylineElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
