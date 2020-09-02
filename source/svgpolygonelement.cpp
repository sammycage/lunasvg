#include "svgpolygonelement.h"

namespace lunasvg {

SVGPolygonElement::SVGPolygonElement(SVGDocument* document)
    : SVGPolyElement(DOMElementIdPolygon, document)
{
}

Path SVGPolygonElement::makePath(const RenderState&) const
{
    Path path;
    path.addPoints(points().property()->values());
    path.closePath();
    return path;
}

SVGElementImpl* SVGPolygonElement::clone(SVGDocument* document) const
{
    SVGPolygonElement* e = new SVGPolygonElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
