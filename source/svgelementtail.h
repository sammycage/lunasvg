#ifndef SVGELEMENTTAIL_H
#define SVGELEMENTTAIL_H

#include "svgelementimpl.h"

namespace lunasvg {

class SVGElementTail : public SVGElementImpl
{
public:
    SVGElementTail(SVGDocument* document);
    bool isSVGElementTail() const { return true; }
    void render(RenderContext& context) const;
    void externalise(std::string& out, std::uint32_t& indent) const;
    SVGElementImpl* clone(SVGDocument* document) const;
};

} // namespace lunasvg

#endif // SVGELEMENTTAIL_H
