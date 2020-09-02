#include "svgrectelement.h"

namespace lunasvg {

SVGRectElement::SVGRectElement(SVGDocument* document)
    : SVGGeometryElement(DOMElementIdRect, document),
      m_x(DOMPropertyIdX, LengthModeWidth, AllowNegativeLengths),
      m_y(DOMPropertyIdY, LengthModeHeight, AllowNegativeLengths),
      m_rx(DOMPropertyIdRx, LengthModeWidth, ForbidNegativeLengths),
      m_ry(DOMPropertyIdRy, LengthModeHeight, ForbidNegativeLengths),
      m_width(DOMPropertyIdWidth, LengthModeWidth, ForbidNegativeLengths),
      m_heigth(DOMPropertyIdHeight, LengthModeHeight, ForbidNegativeLengths)
{
    addToPropertyMap(m_x);
    addToPropertyMap(m_y);
    addToPropertyMap(m_rx);
    addToPropertyMap(m_ry);
    addToPropertyMap(m_width);
    addToPropertyMap(m_heigth);
}

Path SVGRectElement::makePath(const RenderState& state) const
{
    double _x = m_x.value(state);
    double _y = m_y.value(state);
    double _w = m_width.value(state);
    double _h = m_heigth.value(state);

    double _rx, _ry;
    if(m_rx.isSpecified() && m_ry.isSpecified())
    {
        _rx = m_rx.value(state);
        _ry = m_ry.value(state);
    }
    else if(m_rx.isSpecified())
    {
        _rx = _ry = m_rx.value(state);
    }
    else if(m_ry.isSpecified())
    {
        _rx = _ry = m_ry.value(state);
    }
    else
    {
        _rx = _ry = 0.0;
    }

    Path path;
    path.addRect(_x, _y, _w, _h, _rx, _ry);
    return path;
}

Rect SVGRectElement::makeBoundingBox(const RenderState& state) const
{
    double _x = m_x.value(state);
    double _y = m_y.value(state);
    double _w = m_width.value(state);
    double _h = m_heigth.value(state);
    return Rect(_x, _y, _w, _h);
}

void SVGRectElement::render(RenderContext& context) const
{
    if(m_width.isZero() || m_heigth.isZero())
    {
        context.skipElement();
        return;
    }

    SVGGeometryElement::render(context);
}

SVGElementImpl* SVGRectElement::clone(SVGDocument* document) const
{
    SVGRectElement* e = new SVGRectElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
