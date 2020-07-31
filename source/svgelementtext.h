#ifndef SVGELEMENTCONTENT_H
#define SVGELEMENTCONTENT_H

#include "svgelementimpl.h"

namespace lunasvg {

class SVGElementText : public SVGElementImpl
{
public:
    SVGElementText(SVGDocument* document);
    bool isSVGElementText() const { return true; }
    void render(RenderContext&) const;
    void externalise(std::string&, std::uint32_t&) const;
    SVGElementImpl* clone(SVGDocument* document) const;
};

} // namespace lunasvg

#endif // SVGELEMENTCONTENT_H
