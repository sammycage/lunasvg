#ifndef SVGELLIPSEELEMENT_H
#define SVGELLIPSEELEMENT_H

#include "svggeometryelement.h"
#include "svglength.h"

namespace lunasvg {

class SVGEllipseElement : public SVGGeometryElement
{
public:
    SVGEllipseElement(SVGDocument* document);
    const DOMSVGLength& cx() const { return m_cx; }
    const DOMSVGLength& cy() const { return m_cy; }
    const DOMSVGLength& rx() const { return m_rx; }
    const DOMSVGLength& ry() const { return m_ry; }
    Path makePath(const RenderState& state) const;
    Rect makeBoundingBox(const RenderState& state) const;
    void render(RenderContext& context) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_cx;
    DOMSVGLength m_cy;
    DOMSVGLength m_rx;
    DOMSVGLength m_ry;
};

} // namespace lunasvg

#endif // SVGELLIPSEELEMENT_H
