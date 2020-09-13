#include "svggeometryelement.h"
#include "svgmarkerelement.h"
#include "pathiterator.h"
#include "paint.h"
#include "strokedata.h"

namespace lunasvg {

#define PI 3.14159265358979323846

SVGGeometryElement::SVGGeometryElement(DOMElementID elementId, SVGDocument* document)
    : SVGGraphicsElement(elementId, document)
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
        state.bbox.width += strokeWidth;
        state.bbox.height += strokeWidth;
    }

    if(context.mode() == RenderModeBounding)
        return;

    Path path = makePath(state);
    if(context.mode() == RenderModeClipping)
    {
        state.canvas.draw(path, state.matrix, state.style.clipRule(), KRgbBlack, Paint(), StrokeData());
        return;
    }

    StrokeData strokeData = state.style.strokeData(state);
    Paint fillPaint = state.style.fillPaint(state);
    Paint strokePaint = state.style.strokePaint(state);
    WindRule fillRule = state.style.fillRule();

    fillPaint.setOpacity(state.style.fillOpacity() * state.style.opacity());
    strokePaint.setOpacity(state.style.strokeOpacity() * state.style.opacity());

    state.canvas.draw(path, state.matrix, fillRule, fillPaint, strokePaint, strokeData);

    const SVGMarkerElement* markerStart = state.style.markerStart(document());
    const SVGMarkerElement* markerMid = state.style.markerMid(document());
    const SVGMarkerElement* markerEnd = state.style.markerEnd(document());

    if(markerStart==nullptr && markerMid==nullptr && markerEnd==nullptr)
        return;

    Point origin;
    Point startPoint;
    Point inslopePoints[2];
    Point outslopePoints[2];

    int index = 0;
    PathIterator it(path);
    double c[6];
    while(!it.isDone())
    {
        switch(it.currentSegment(c))
        {
        case SegTypeMoveTo:
            startPoint = Point(c[0], c[1]);
            inslopePoints[0] = origin;
            inslopePoints[1] = Point(c[0], c[1]);
            origin = Point(c[0], c[1]);
            break;
        case SegTypeLineTo:
            inslopePoints[0] = origin;
            inslopePoints[1] = Point(c[0], c[1]);
            origin = Point(c[0], c[1]);
            break;
        case SegTypeQuadTo:
            origin = Point(c[2], c[3]);
            break;
        case SegTypeCubicTo:
            inslopePoints[0] = Point(c[2], c[3]);
            inslopePoints[1] = Point(c[4], c[5]);
            origin = Point(c[4], c[5]);
            break;
        case SegTypeClose:
            inslopePoints[0] = origin;
            inslopePoints[1] = Point(c[0], c[1]);
            origin = startPoint;
            startPoint = Point();
            break;
        }

        index += 1;
        it.next();

        if(!it.isDone() && (markerStart || markerMid))
        {
            it.currentSegment(c);
            outslopePoints[0] = origin;
            outslopePoints[1] = Point(c[0], c[1]);

            if(index == 1 && markerStart)
            {
                Point slope(outslopePoints[1].x - outslopePoints[0].x, outslopePoints[1].y - outslopePoints[0].y);
                double angle = 180.0 * std::atan2(slope.y, slope.x) / PI;

                markerStart->renderMarker(state, origin, angle);
            }

            if(index > 1 && markerMid)
            {
                Point inslope(inslopePoints[1].x - inslopePoints[0].x, inslopePoints[1].y - inslopePoints[0].y);
                Point outslope(outslopePoints[1].x - outslopePoints[0].x, outslopePoints[1].y - outslopePoints[0].y);
                double inangle = 180.0 * std::atan2(inslope.y, inslope.x) / PI;
                double outangle = 180.0 * std::atan2(outslope.y, outslope.x) / PI;

                markerMid->renderMarker(state, origin, (inangle + outangle) * 0.5);
            }
        }

        if(it.isDone() && markerEnd)
        {
            Point slope(inslopePoints[1].x - inslopePoints[0].x, inslopePoints[1].y - inslopePoints[0].y);
            double angle = 180.0 * std::atan2(slope.y, slope.x) / PI;

            markerEnd->renderMarker(state, origin, angle);
        }
    }
}

} // namespace lunasvg
