#include "svgpathelement.h"

namespace lunasvg {

SVGPathElement::SVGPathElement(SVGDocument* document)
    : SVGGeometryElement(DOMElementIdPath, document),
      m_d(DOMPropertyIdD)
{
    addToPropertyMap(m_d);
}

Path SVGPathElement::makePath(const RenderState&) const
{
    return m_d.property()->value();
}

Rect SVGPathElement::makeBoundingBox(const RenderState&) const
{
    const Path& path = m_d.property()->value();
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
