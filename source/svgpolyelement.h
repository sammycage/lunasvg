#ifndef SVGPOLYELEMENT_H
#define SVGPOLYELEMENT_H

#include "svggeometryelement.h"
#include "svgpointlist.h"

namespace lunasvg {

class SVGPolyElement : public SVGGeometryElement
{
public:
    SVGPolyElement(DOMElementID elementId, SVGDocument* document);
    bool isSVGPolyElement() const { return true; }
    const DOMSVGPointList& points() const { return m_points; }
    virtual Rect makeBoundingBox(const RenderState&) const;
    virtual void render(RenderContext& context) const;

private:
    DOMSVGPointList m_points;
};

} // namespace lunasvg

#endif // SVGPOLYELEMENT_H
