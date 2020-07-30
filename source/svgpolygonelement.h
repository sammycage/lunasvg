#ifndef SVGPOLYGONELEMENT_H
#define SVGPOLYGONELEMENT_H

#include "svgpolyelement.h"

namespace lunasvg {

class SVGPolygonElement : public SVGPolyElement
{
public:
    SVGPolygonElement(SVGDocument* document);
    Path makePath(const RenderState&) const;
    SVGElementImpl* clone(SVGDocument* document) const;
};

} // namespace lunasvg

#endif // SVGPOLYGONELEMENT_H
