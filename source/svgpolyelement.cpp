#include "svgpolyelement.h"

namespace lunasvg {

SVGPolyElement::SVGPolyElement(ElementID elementId, SVGDocument* document) :
    SVGGeometryElement(elementId, document),
    m_points(DOMPropertyIdPoints)
{
    addToPropertyMap(m_points);
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
