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

#define KElementIdsAnimation \
    DOMElementIdAnimate, \
    DOMElementIdAnimateMotion, \
    DOMElementIdAnimateTransform, \
    DOMElementIdAnimateColor, \
    DOMElementIdSet

#define KElementIdsDescriptive \
    DOMElementIdDesc, \
    DOMElementIdTitle, \
    DOMElementIdMetadata

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

#define KElementIdsTextContentChild \
    DOMElementIdTextPath, \
    DOMElementIdTref, \
    DOMElementIdTspan

static const DOMElementID contentmodel_1[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    KElementIdsStructural,
    KElementIdsGradient,
    DOMElementIdA,
    DOMElementIdClipPath,
    DOMElementIdImage,
    DOMElementIdMarker,
    DOMElementIdMask,
    DOMElementIdPattern,
    DOMElementIdStyle,
    DOMElementIdSwitch,
    DOMElementIdText,
    DOMElementIdView,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_2[] = {
    KElementIdsAnimation,
    KElementIdsDescriptive,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_3[] = {
    //KElementAllIds,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_4[] = {
    KElementIdsDescriptive,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_5[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    DOMElementIdA,
    DOMElementIdG,
    DOMElementIdImage,
    DOMElementIdSvg,
    DOMElementIdSwitch,
    DOMElementIdText,
    DOMElementIdUse,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_6[] = {
    KElementIdsAnimation,
    KElementIdsDescriptive,
    KElementIdsTextContentChild,
    DOMElementIdA,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_7[] = {
    KElementIdsDescriptive,
    DOMElementIdA,
    DOMElementIdAnimate,
    DOMElementIdAnimateColor,
    DOMElementIdSet,
    DOMElementIdTref,
    DOMElementIdTspan,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_8[] = {
    KElementIdsDescriptive,
    DOMElementIdAnimate,
    DOMElementIdAnimateColor,
    DOMElementIdSet,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_9[] = {
    KElementIdsGradient,
    DOMElementIdSolidColor,
    DOMElementIdPattern,
    DOMElementIdAnimate,
    DOMElementIdAnimateColor,
    DOMElementIdSet,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_11[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    DOMElementIdUse,
    DOMElementIdText,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_12[] = {
    KElementIdsDescriptive,
    DOMElementIdAnimate,
    DOMElementIdAnimateTransform,
    DOMElementIdSet,
    DOMElementIdStop,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_13[] = {
    DOMElementIdAnimate,
    DOMElementIdAnimateColor,
    DOMElementIdSet,
    DOMElementIdUnknown
};

static const DOMElementID contentmodel_23[] = {
    KElementIdsDescriptive,
    DOMElementIdMpath,
    DOMElementIdUnknown
};

static const struct
{
    DOMElementID id;
    const DOMElementID* content;
} contentmodelmap[] = {
    {DOMElementIdUnknown, nullptr},
    {DOMElementIdA, contentmodel_1},
    {DOMElementIdAnimate, contentmodel_4},
    {DOMElementIdAnimateColor, contentmodel_4},
    {DOMElementIdAnimateMotion, contentmodel_23},
    {DOMElementIdAnimateTransform, contentmodel_4},
    {DOMElementIdCircle, contentmodel_2},
    {DOMElementIdClipPath, contentmodel_11},
    {DOMElementIdDefs, contentmodel_1},
    {DOMElementIdDesc, contentmodel_3},
    {DOMElementIdEllipse, contentmodel_2},
    {DOMElementIdG, contentmodel_1},
    {DOMElementIdImage, contentmodel_2},
    {DOMElementIdLine, contentmodel_2},
    {DOMElementIdLinearGradient, contentmodel_12},
    {DOMElementIdMarker, contentmodel_1},
    {DOMElementIdMask, contentmodel_1},
    {DOMElementIdMetadata, contentmodel_3},
    {DOMElementIdMpath, contentmodel_23},
    {DOMElementIdPath, contentmodel_2},
    {DOMElementIdPattern, contentmodel_1},
    {DOMElementIdPolygon, contentmodel_2},
    {DOMElementIdPolyline, contentmodel_2},
    {DOMElementIdRadialGradient, contentmodel_12},
    {DOMElementIdRect, contentmodel_2},
    {DOMElementIdSet, contentmodel_4},
    {DOMElementIdSolidColor, contentmodel_9},
    {DOMElementIdStop, contentmodel_13},
    {DOMElementIdStyle, contentmodel_3},
    {DOMElementIdSvg, contentmodel_1},
    {DOMElementIdSwitch, contentmodel_5},
    {DOMElementIdSymbol, contentmodel_1},
    {DOMElementIdText, contentmodel_6},
    {DOMElementIdTextPath, contentmodel_7},
    {DOMElementIdTitle, contentmodel_3},
    {DOMElementIdTref, contentmodel_8},
    {DOMElementIdTspan, contentmodel_7},
    {DOMElementIdUse, contentmodel_2},
    {DOMElementIdView, contentmodel_4},
    {DOMElementLastId, nullptr}
};

static const std::map<std::string, DOMElementID> domelementmap = {
    {"a", DOMElementIdA},
    {"animate", DOMElementIdAnimate},
    {"animateColor", DOMElementIdAnimateColor},
    {"animateMotion", DOMElementIdAnimateMotion},
    {"animateTransform", DOMElementIdAnimateTransform},
    {"circle", DOMElementIdCircle},
    {"clipPath", DOMElementIdClipPath},
    {"defs", DOMElementIdDefs},
    {"desc", DOMElementIdDesc},
    {"ellipse", DOMElementIdEllipse},
    {"g", DOMElementIdG},
    {"image", DOMElementIdImage},
    {"line", DOMElementIdLine},
    {"linearGradient", DOMElementIdLinearGradient},
    {"marker", DOMElementIdMarker},
    {"mask", DOMElementIdMask},
    {"metadata", DOMElementIdMetadata},
    {"mpath", DOMElementIdMpath},
    {"path", DOMElementIdPath},
    {"pattern", DOMElementIdPattern},
    {"polygon", DOMElementIdPolygon},
    {"polyline", DOMElementIdPolyline},
    {"radialGradient", DOMElementIdRadialGradient},
    {"rect", DOMElementIdRect},
    {"set", DOMElementIdSet},
    {"stop", DOMElementIdStop},
    {"solidColor", DOMElementIdSolidColor},
    {"style", DOMElementIdStyle},
    {"svg", DOMElementIdSvg},
    {"switch", DOMElementIdSwitch},
    {"symbol", DOMElementIdSymbol},
    {"text", DOMElementIdText},
    {"textPath", DOMElementIdTextPath},
    {"title", DOMElementIdTitle},
    {"tref", DOMElementIdTref},
    {"tspan", DOMElementIdTspan},
    {"use", DOMElementIdUse},
    {"view", DOMElementIdView},
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
