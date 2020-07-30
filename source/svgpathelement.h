#ifndef SVGPATHELEMENT_H
#define SVGPATHELEMENT_H

#include "svggeometryelement.h"
#include "svgpath.h"

namespace lunasvg {

class SVGPathElement : public SVGGeometryElement
{
public:
    SVGPathElement(SVGDocument* document);
    const DOMSVGPath& d() const { return m_d; }
    Path makePath(const RenderState&) const;
    Rect makeBoundingBox(const RenderState&) const;
    void render(RenderContext& context) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGPath m_d;
};

} // namespace lunasvg

#endif // SVGPATHELEMENT_H
