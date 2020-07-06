#include "svgelementtail.h"
#include "svgelementhead.h"

namespace lunasvg {

SVGElementTail::SVGElementTail(SVGDocument* document) :
    SVGElementImpl(document)
{
}

void SVGElementTail::render(RenderContext& context) const
{
    parent->renderTail(context);
}

void SVGElementTail::externalise(std::string& out, unsigned int& indent) const
{
    parent->externaliseTail(out, indent);
}

SVGElementImpl* SVGElementTail::clone(SVGDocument* document) const
{
    return new SVGElementTail(document);
}

} // namespace lunasvg
