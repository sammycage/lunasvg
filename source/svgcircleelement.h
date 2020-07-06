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
    Rect makePathAndBoundingBox(const RenderState& state, Path& path) const;
    void render(RenderContext& context) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_cx;
    DOMSVGLength m_cy;
    DOMSVGLength m_r;
};

} // namespace lunasvg

#endif // SVGCIRCLEELEMENT_H
