#include "svggeometryelement.h"
#include "paint.h"
#include "strokedata.h"

namespace lunasvg {

SVGGeometryElement::SVGGeometryElement(ElementID elementId, SVGDocument* document) :
    SVGGraphicsElement(elementId, document)
{
}

void SVGGeometryElement::render(RenderContext& context) const
{
    if(style().isDisplayNone())
    {
        context.skipElement();
        return;
    }

    SVGGraphicsElement::render(context);
    RenderState& state = context.state();
    if(state.style.isHidden())
        return;

    Path path;
    state.bbox = makePathAndBoundingBox(state, path);
    if(context.mode() == RenderModeClip)
    {
        state.canvas.draw(path, state.matrix, state.style.clipRule(), KRgbBlack, Paint(), StrokeData());
        return;
    }

    StrokeData strokeData = state.style.strokeData(state);
    Paint fillPaint = state.style.fillPaint(state);
    Paint strokePaint = state.style.strokePaint(state);
    WindRule fillRule = state.style.fillRule();

    fillPaint.setOpacity(state.style.fillOpacity());
    strokePaint.setOpacity(state.style.strokeOpacity());

    state.canvas.draw(path, state.matrix, fillRule, fillPaint, strokePaint, strokeData);
}

} // namespace lunasvg
