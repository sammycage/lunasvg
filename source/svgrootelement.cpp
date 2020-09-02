#include "svgrootelement.h"
#include "svgelementtail.h"
#include "svgdocumentimpl.h"

namespace lunasvg {

SVGRootElement::SVGRootElement(SVGDocument* document)
    : SVGSVGElement(document)
{
}

void SVGRootElement::renderToBitmap(Bitmap& bitmap, const Rect& viewBox, double dpi, std::uint32_t bgColor) const
{
    if(style().isDisplayNone() || next == tail)
        return;

    RenderContext context(this, RenderModeDisplay);
    RenderState& state = context.state();
    state.element = this;
    state.canvas.reset(bitmap.data(), bitmap.width(), bitmap.height(), bitmap.stride());
    state.canvas.clear(bgColor);
    state.viewPort = Rect(0, 0, bitmap.width(), bitmap.height());
    state.dpi = dpi;

    SVGGraphicsElement::render(context);
    RenderState& newState = context.state();
    newState.matrix.multiply(calculateViewBoxTransform(state.viewPort, viewBox));
    newState.viewPort = viewBox;

    context.render(next, tail);
    state.canvas.convertToRGBA();
}

} // namespace lunasvg
