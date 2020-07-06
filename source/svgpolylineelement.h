#ifndef SVGPOLYLINEELEMENT_H
#define SVGPOLYLINEELEMENT_H

#include "svgpolyelement.h"

namespace lunasvg {

class SVGPolylineElement : public SVGPolyElement
{
public:
    SVGPolylineElement(SVGDocument* document);
    Rect makePathAndBoundingBox(const RenderState&, Path& path) const;
    SVGElementImpl* clone(SVGDocument* document) const;
};

} // namespace lunasvg

#endif // SVGPOLYLINEELEMENT_H
