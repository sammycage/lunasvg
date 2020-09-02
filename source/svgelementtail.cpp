#include "svgelementtail.h"
#include "svgelementhead.h"

namespace lunasvg {

SVGElementTail::SVGElementTail(SVGDocument* document)
    : SVGElementImpl(document)
{
}

void SVGElementTail::render(RenderContext& context) const
{
    parent->renderTail(context);
}

void SVGElementTail::externalise(std::string& out, std::uint32_t& indent) const
{
    parent->externaliseTail(out, indent);
}

SVGElementImpl* SVGElementTail::clone(SVGDocument* document) const
{
    return new SVGElementTail(document);
}

} // namespace lunasvg
