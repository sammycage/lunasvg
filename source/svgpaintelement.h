#ifndef SVGPAINTELEMENT_H
#define SVGPAINTELEMENT_H

#include "svgstyledelement.h"
#include "rendercontext.h"
#include "paint.h"

namespace lunasvg {

class SVGPaintElement : public SVGStyledElement
{
public:
    SVGPaintElement(DOMElementID elementId, SVGDocument* document);
    bool isSVGPaintElement() const { return true; }
    virtual Paint getPaint(const RenderState& state) const = 0;
    virtual void render(RenderContext& context) const;
};

} // namespace lunasvg

#endif // SVGPAINTELEMENT_H
