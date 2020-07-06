#include "svgpathelement.h"

namespace lunasvg {

SVGPathElement::SVGPathElement(SVGDocument *document) :
    SVGGeometryElement(ElementIdPath, document),
    m_d(DOMPropertyIdD)
{
    addToPropertyMap(m_d);
}

Rect SVGPathElement::makePathAndBoundingBox(const RenderState&, Path& path) const
{
    path = m_d.property()->value();
    return path.boundingBox();
}

void SVGPathElement::render(RenderContext& context) const
{
    if(!m_d.isSpecified())
    {
        context.skipElement();
        return;
    }

    SVGGeometryElement::render(context);
}

SVGElementImpl* SVGPathElement::clone(SVGDocument* document) const
{
    SVGPathElement* e = new SVGPathElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
