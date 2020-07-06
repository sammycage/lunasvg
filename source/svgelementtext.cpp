#include "svgelementtext.h"

namespace lunasvg {

SVGElementText::SVGElementText(SVGDocument* document) :
    SVGElementImpl(document)
{
}

void SVGElementText::render(RenderContext&) const
{
}

void SVGElementText::externalise(std::string&, unsigned int&) const
{
}

SVGElementImpl* SVGElementText::clone(SVGDocument* document) const
{
    return new SVGElementText(document);
}

} // namespace lunasvg
