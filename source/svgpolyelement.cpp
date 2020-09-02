#include "svgpolyelement.h"

namespace lunasvg {

SVGPolyElement::SVGPolyElement(DOMElementID elementId, SVGDocument* document)
    : SVGGeometryElement(elementId, document),
      m_points(DOMPropertyIdPoints)
{
    addToPropertyMap(m_points);
}

Rect SVGPolyElement::makeBoundingBox(const RenderState&) const
{
    double xMin = std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::min();
    double yMax = std::numeric_limits<double>::min();

    const SVGPointList* points = m_points.property();
    for(unsigned int i = 0;i < points->length();i++)
    {
        const SVGPoint* p = points->at(i);
        if(p->x() < xMin) xMin = p->x();
        if(p->x() > xMax) xMax = p->x();
        if(p->y() < yMin) yMin = p->y();
        if(p->y() > yMax) yMax = p->y();
    }

    return Rect(xMin, yMin, xMax - xMin, yMax - yMin);
}

void SVGPolyElement::render(RenderContext& context) const
{
    if(!m_points.isSpecified())
    {
        context.skipElement();
        return;
    }

    SVGGeometryElement::render(context);
}

} // namespace lunasvg
