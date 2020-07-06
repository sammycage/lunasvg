#include "svgellipseelement.h"

namespace lunasvg {

SVGEllipseElement::SVGEllipseElement(SVGDocument* document) :
    SVGGeometryElement(ElementIdEllipse, document),
    m_cx(DOMPropertyIdCx, LengthModeWidth, AllowNegativeLengths),
    m_cy(DOMPropertyIdCy, LengthModeHeight, AllowNegativeLengths),
    m_rx(DOMPropertyIdRx, LengthModeWidth, ForbidNegativeLengths),
    m_ry(DOMPropertyIdRy, LengthModeHeight, ForbidNegativeLengths)
{
    addToPropertyMap(m_cx);
    addToPropertyMap(m_cy);
    addToPropertyMap(m_rx);
    addToPropertyMap(m_ry);
}

Rect SVGEllipseElement::makePathAndBoundingBox(const RenderState& state, Path& path) const
{
    double _cx = m_cx.value(state);
    double _cy = m_cy.value(state);
    double _rx = m_rx.value(state);
    double _ry = m_ry.value(state);
    path.addEllipse(_cx, _cy, _rx, _ry);

    return Rect(_cx-_rx, _cy-_ry, _rx+_rx, _ry+_ry);
}

void SVGEllipseElement::render(RenderContext& context) const
{
    if(m_rx.isZero() || m_ry.isZero())
    {
        context.skipElement();
        return;
    }

    SVGGeometryElement::render(context);
}

SVGElementImpl* SVGEllipseElement::clone(SVGDocument* document) const
{
    SVGEllipseElement* e = new SVGEllipseElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
