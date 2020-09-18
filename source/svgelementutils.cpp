#include "svgelementutils.h"
#include "svgsvgelement.h"
#include "svggelement.h"
#include "svgrectelement.h"
#include "svgcircleelement.h"
#include "svgellipseelement.h"
#include "svgpathelement.h"
#include "svglineelement.h"
#include "svgdefselement.h"
#include "svgpolygonelement.h"
#include "svgpolylineelement.h"
#include "svglineargradientelement.h"
#include "svgradialgradientelement.h"
#include "svgstopelement.h"
#include "svgsymbolelement.h"
#include "svguseelement.h"
#include "svgpatternelement.h"
#include "svgmaskelement.h"
#include "svgclippathelement.h"
#include "svgsolidcolorelement.h"
#include "svgmarkerelement.h"

#include <map>

namespace lunasvg {

namespace Utils {

#define KElementIdsGradient \
    DOMElementIdLinearGradient, \
    DOMElementIdRadialGradient

#define KElementIdsStructural \
    DOMElementIdDefs, \
    DOMElementIdG, \
    DOMElementIdSvg, \
    DOMElementIdSymbol, \
    DOMElementIdUse

#define KElementIdsShape \
    DOMElementIdCircle, \
    DOMElementIdEllipse, \
    DOMElementIdLine, \
    DOMElementIdPath, \
    DOMElementIdPolygon, \
    DOMElementIdPolyline, \
    DOMElementIdRect

static const DOMElementID contentmodel_1[] = {
    KElementIdsShape,
    KElementIdsStructural,
    KElementIdsGradient,
    DOMElementIdClipPath,
    DOMElementIdMarker,
    DOMElementIdMask,
    DOMElementIdPattern,
    DOMElementIdSolidColor,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_2[] = {
    KElementIdsGradient,
    DOMElementIdSolidColor,
    DOMElementIdPattern,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_3[] = {
    KElementIdsShape,
    DOMElementIdUse,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_4[] = {
    DOMElementIdStop,
    DOMElementIdUnknown
};

static const struct
{
    DOMElementID id;
    const DOMElementID* content;
} contentmodelmap[] = {
    {DOMElementIdUnknown, nullptr},
    {DOMElementIdCircle, nullptr},
    {DOMElementIdClipPath, contentmodel_3},
    {DOMElementIdDefs, contentmodel_1},
    {DOMElementIdEllipse, nullptr},
    {DOMElementIdG, contentmodel_1},
    {DOMElementIdLine, nullptr},
    {DOMElementIdLinearGradient, contentmodel_4},
    {DOMElementIdMarker, contentmodel_1},
    {DOMElementIdMask, contentmodel_1},
    {DOMElementIdPath, nullptr},
    {DOMElementIdPattern, contentmodel_1},
    {DOMElementIdPolygon, nullptr},
    {DOMElementIdPolyline, nullptr},
    {DOMElementIdRadialGradient, contentmodel_4},
    {DOMElementIdRect, nullptr},
    {DOMElementIdSolidColor, contentmodel_2},
    {DOMElementIdStop, nullptr},
    {DOMElementIdSvg, contentmodel_1},
    {DOMElementIdSymbol, contentmodel_1},
    {DOMElementIdUse, nullptr},
    {DOMElementLastId, nullptr}
};

static const std::map<std::string, DOMElementID> domelementmap = {
    {"circle", DOMElementIdCircle},
    {"clipPath", DOMElementIdClipPath},
    {"defs", DOMElementIdDefs},
    {"ellipse", DOMElementIdEllipse},
    {"g", DOMElementIdG},
    {"line", DOMElementIdLine},
    {"linearGradient", DOMElementIdLinearGradient},
    {"marker", DOMElementIdMarker},
    {"mask", DOMElementIdMask},
    {"path", DOMElementIdPath},
    {"pattern", DOMElementIdPattern},
    {"polygon", DOMElementIdPolygon},
    {"polyline", DOMElementIdPolyline},
    {"radialGradient", DOMElementIdRadialGradient},
    {"rect", DOMElementIdRect},
    {"stop", DOMElementIdStop},
    {"solidColor", DOMElementIdSolidColor},
    {"svg", DOMElementIdSvg},
    {"symbol", DOMElementIdSymbol},
    {"use", DOMElementIdUse},
};

bool isElementPermitted(DOMElementID parentId, DOMElementID childId)
{
    if(const DOMElementID* content = contentmodelmap[parentId].content)
    {
        for(int i = 0;content[i] != DOMElementIdUnknown;i++)
            if(content[i] == childId)
                return true;
    }

    return false;
}

SVGElementHead* createElement(DOMElementID elementId, SVGDocument* document)
{
    switch(elementId)
    {
    case DOMElementIdSvg:
        return new SVGSVGElement(document);
    case DOMElementIdPath:
        return new SVGPathElement(document);
    case DOMElementIdG:
        return new SVGGElement(document);
    case DOMElementIdRect:
        return new SVGRectElement(document);
    case DOMElementIdCircle:
        return new SVGCircleElement(document);
    case DOMElementIdEllipse:
        return new SVGEllipseElement(document);
    case DOMElementIdLine:
        return new SVGLineElement(document);
    case DOMElementIdDefs:
        return new SVGDefsElement(document);
    case DOMElementIdPolygon:
        return new SVGPolygonElement(document);
    case DOMElementIdPolyline:
        return new SVGPolylineElement(document);
    case DOMElementIdStop:
        return new SVGStopElement(document);
    case DOMElementIdLinearGradient:
        return new SVGLinearGradientElement(document);
    case DOMElementIdRadialGradient:
        return new SVGRadialGradientElement(document);
    case DOMElementIdSymbol:
        return new SVGSymbolElement(document);
    case DOMElementIdUse:
        return new SVGUseElement(document);
    case DOMElementIdPattern:
        return new SVGPatternElement(document);
    case DOMElementIdMask:
        return new SVGMaskElement(document);
    case DOMElementIdClipPath:
        return new SVGClipPathElement(document);
    case DOMElementIdSolidColor:
        return new SVGSolidColorElement(document);
    case DOMElementIdMarker:
        return new SVGMarkerElement(document);
    default:
        break;
    }

    return nullptr;
}

const std::string& domElementName(DOMElementID nameId)
{
    std::map<std::string, DOMElementID>::const_iterator it = domelementmap.begin();
    std::map<std::string, DOMElementID>::const_iterator end = domelementmap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

DOMElementID domElementId(const std::string& name)
{
    std::map<std::string, DOMElementID>::const_iterator it = domelementmap.find(name);
    return it != domelementmap.end() ? it->second : DOMElementIdUnknown;
}

} // namespace Utils

} // namespace lunasvg
