#ifndef SVGLINEELEMENT_H
#define SVGLINEELEMENT_H

#include "svggeometryelement.h"
#include "svglength.h"

namespace lunasvg {

class SVGLineElement : public SVGGeometryElement
{
public:
    SVGLineElement(SVGDocument* document);
    const DOMSVGLength& x1() const { return m_x1; }
    const DOMSVGLength& y1() const { return m_y1; }
    const DOMSVGLength& x2() const { return m_x2; }
    const DOMSVGLength& y2() const { return m_y2; }
    Path makePath(const RenderState& state) const;
    Rect makeBoundingBox(const RenderState& state) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_x1;
    DOMSVGLength m_y1;
    DOMSVGLength m_x2;
    DOMSVGLength m_y2;
};

} // namespace lunasvg

#endif // SVGLINEELEMENT_H
