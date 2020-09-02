#include "svglineelement.h"

namespace lunasvg {

SVGLineElement::SVGLineElement(SVGDocument* document)
    : SVGGeometryElement(DOMElementIdLine, document),
      m_x1(DOMPropertyIdX1, LengthModeWidth, AllowNegativeLengths),
      m_y1(DOMPropertyIdY1, LengthModeHeight, AllowNegativeLengths),
      m_x2(DOMPropertyIdX2, LengthModeWidth, AllowNegativeLengths),
      m_y2(DOMPropertyIdY2, LengthModeHeight, AllowNegativeLengths)
{
    addToPropertyMap(m_x1);
    addToPropertyMap(m_y1);
    addToPropertyMap(m_x2);
    addToPropertyMap(m_y2);
}

Path SVGLineElement::makePath(const RenderState& state) const
{
    double _x1 = m_x1.value(state);
    double _y1 = m_y1.value(state);
    double _x2 = m_x2.value(state);
    double _y2 = m_y2.value(state);

    Path path;
    path.moveTo(_x1, _y1);
    path.lineTo(_x2, _y2);
    return path;
}

Rect SVGLineElement::makeBoundingBox(const RenderState& state) const
{
    double _x1 = m_x1.value(state);
    double _y1 = m_y1.value(state);
    double _x2 = m_x2.value(state);
    double _y2 = m_y2.value(state);

    Rect bbox;
    bbox.x = std::min(_x1, _x2);
    bbox.y = std::min(_y1, _y2);
    bbox.width = std::abs(_x2 - _x1);
    bbox.height = std::abs(_y2 - _y1);
    return bbox;
}

SVGElementImpl* SVGLineElement::clone(SVGDocument* document) const
{
    SVGLineElement* e = new SVGLineElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
