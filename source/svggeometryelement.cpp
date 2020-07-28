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

    state.bbox = makeBoundingBox(state);
    if(state.style.hasStroke())
    {
        double strokeWidth = state.style.strokeWidth(state);
        state.bbox.x -= strokeWidth * 0.5;
        state.bbox.y -= strokeWidth * 0.5;
        state.bbox.x += strokeWidth;
        state.bbox.x += strokeWidth;
    }

    if(context.mode() == RenderModeBounding)
        return;

    Path path = makePath(state);
    if(context.mode() == RenderModeDisplay)
    {
        StrokeData strokeData = state.style.strokeData(state);
        Paint fillPaint = state.style.fillPaint(state);
        Paint strokePaint = state.style.strokePaint(state);
        WindRule fillRule = state.style.fillRule();

        fillPaint.setOpacity(state.style.fillOpacity());
        strokePaint.setOpacity(state.style.strokeOpacity());

        state.canvas.draw(path, state.matrix, fillRule, fillPaint, strokePaint, strokeData);
    }
    else
    {
        state.canvas.draw(path, state.matrix, state.style.clipRule(), KRgbBlack, Paint(), StrokeData());
    }
}

} // namespace lunasvg
