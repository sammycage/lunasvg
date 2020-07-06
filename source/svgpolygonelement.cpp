#include "svgpolygonelement.h"

namespace lunasvg {

SVGPolygonElement::SVGPolygonElement(SVGDocument* document) :
    SVGPolyElement(ElementIdPolygon, document)
{
}

Rect SVGPolygonElement::makePathAndBoundingBox(const RenderState&, Path& path) const
{
    path.addPoints(points().property()->values());
    path.closePath();
    return path.boundingBox();
}

SVGElementImpl* SVGPolygonElement::clone(SVGDocument* document) const
{
    SVGPolygonElement* e = new SVGPolygonElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
