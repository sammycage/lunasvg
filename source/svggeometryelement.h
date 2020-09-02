#ifndef SVGGEOMETRYELEMENT_H
#define SVGGEOMETRYELEMENT_H

#include "svggraphicselement.h"
#include "path.h"

namespace lunasvg {

class SVGGeometryElement : public SVGGraphicsElement
{
public:
    SVGGeometryElement(DOMElementID elementId, SVGDocument* document);
    bool isSVGGeometryElement() const { return true; }
    virtual Path makePath(const RenderState& state) const = 0;
    virtual Rect makeBoundingBox(const RenderState& state) const = 0;
    virtual void render(RenderContext& context) const;
};

} // namespace lunasvg

#endif // SVGGEOMETRYELEMENT_H
