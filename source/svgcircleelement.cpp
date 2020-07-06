#include "svgcircleelement.h"

namespace lunasvg {

SVGCircleElement::SVGCircleElement(SVGDocument* document) :
    SVGGeometryElement(ElementIdCircle, document),
    m_cx(DOMPropertyIdCx, LengthModeWidth, AllowNegativeLengths),
    m_cy(DOMPropertyIdCy, LengthModeHeight, AllowNegativeLengths),
    m_r(DOMPropertyIdR, LengthModeBoth, ForbidNegativeLengths)
{
    addToPropertyMap(m_cx);
    addToPropertyMap(m_cy);
    addToPropertyMap(m_r);
}

Rect SVGCircleElement::makePathAndBoundingBox(const RenderState& state, Path& path) const
{
    double _cx = m_cx.value(state);
    double _cy = m_cy.value(state);
    double _r = m_r.value(state);
    path.addEllipse(_cx, _cy, _r, _r);

    return Rect(_cx-_r, _cy-_r, _r+_r, _r+_r);
}

void SVGCircleElement::render(RenderContext& context) const
{
    if(m_r.isZero())
    {
        context.skipElement();
        return;
    }

    SVGGeometryElement::render(context);
}

SVGElementImpl* SVGCircleElement::clone(SVGDocument* document) const
{
    SVGCircleElement* e = new SVGCircleElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
