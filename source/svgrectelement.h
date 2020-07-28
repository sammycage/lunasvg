#ifndef SVGRECTELEMENT_H
#define SVGRECTELEMENT_H

#include "svggeometryelement.h"
#include "svglength.h"

namespace lunasvg {

class SVGRectElement : public SVGGeometryElement
{
public:
    SVGRectElement(SVGDocument* document);
    const DOMSVGLength& x() const { return m_x; }
    const DOMSVGLength& y() const { return m_y; }
    const DOMSVGLength& rx() const { return m_rx; }
    const DOMSVGLength& ry() const { return m_ry; }
    const DOMSVGLength& width() const { return m_width; }
    const DOMSVGLength& height() const { return m_heigth; }
    Path makePath(const RenderState& state) const;
    Rect makeBoundingBox(const RenderState& state) const;
    void render(RenderContext& context) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_x;
    DOMSVGLength m_y;
    DOMSVGLength m_rx;
    DOMSVGLength m_ry;
    DOMSVGLength m_width;
    DOMSVGLength m_heigth;
};

} // namespace lunasvg

#endif // SVGRECTELEMENT_H
