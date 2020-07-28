#ifndef SVGCIRCLEELEMENT_H
#define SVGCIRCLEELEMENT_H

#include "svggeometryelement.h"
#include "svglength.h"

namespace lunasvg {

class SVGCircleElement : public SVGGeometryElement
{
public:
    SVGCircleElement(SVGDocument* document);
    const DOMSVGLength& cx() const { return m_cx; }
    const DOMSVGLength& cy() const { return m_cy; }
    const DOMSVGLength& r() const { return m_r; }
    Path makePath(const RenderState& state) const;
    Rect makeBoundingBox(const RenderState& state) const;
    void render(RenderContext& context) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_cx;
    DOMSVGLength m_cy;
    DOMSVGLength m_r;
};

} // namespace lunasvg

#endif // SVGCIRCLEELEMENT_H
