#include "svgpaintelement.h"

namespace lunasvg {

SVGPaintElement::SVGPaintElement(DOMElementID elementId, SVGDocument* document)
    : SVGStyledElement(elementId, document)
{
}

void SVGPaintElement::render(RenderContext& context) const
{
    context.skipElement();
}

} // namespace lunasvg
