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

#include <map>

namespace lunasvg {

namespace Utils {

#define KElementIdsAnimation \
    ElementIdAnimate, \
    ElementIdAnimateMotion, \
    ElementIdAnimateTransform, \
    ElementIdAnimateColor, \
    ElementIdSet

#define KElementIdsDescriptive \
    ElementIdDesc, \
    ElementIdTitle, \
    ElementIdMetadata

#define KElementIdsGradient \
    ElementIdLinearGradient, \
    ElementIdRadialGradient

#define KElementIdsStructural \
    ElementIdDefs, \
    ElementIdG, \
    ElementIdSvg, \
    ElementIdSymbol, \
    ElementIdUse

#define KElementIdsShape \
    ElementIdCircle, \
    ElementIdEllipse, \
    ElementIdLine, \
    ElementIdPath, \
    ElementIdPolygon, \
    ElementIdPolyline, \
    ElementIdRect

#define KElementIdsTextContentChild \
    ElementIdTextPath, \
    ElementIdTref, \
    ElementIdTspan

static const ElementID contentmodel_1[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    KElementIdsStructural,
    KElementIdsGradient,
    ElementIdA,
    ElementIdClipPath,
    ElementIdImage,
    ElementIdMarker,
    ElementIdMask,
    ElementIdPattern,
    ElementIdStyle,
    ElementIdSwitch,
    ElementIdText,
    ElementIdView,
    ElementIdUnknown
};

static const ElementID contentmodel_2[] = {
    KElementIdsAnimation,
    KElementIdsDescriptive,
    ElementIdUnknown
};

static const ElementID contentmodel_3[] = {
    //KElementAllIds,
    ElementIdUnknown
};

static const ElementID contentmodel_4[] = {
    KElementIdsDescriptive,
    ElementIdUnknown
};

static const ElementID contentmodel_5[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    ElementIdA,
    ElementIdG,
    ElementIdImage,
    ElementIdSvg,
    ElementIdSwitch,
    ElementIdText,
    ElementIdUse,
    ElementIdUnknown
};

static const ElementID contentmodel_6[] = {
    KElementIdsAnimation,
    KElementIdsDescriptive,
    KElementIdsTextContentChild,
    ElementIdA,
    ElementIdUnknown
};

static const ElementID contentmodel_7[] = {
    KElementIdsDescriptive,
    ElementIdA,
    ElementIdAnimate,
    ElementIdAnimateColor,
    ElementIdSet,
    ElementIdTref,
    ElementIdTspan,
    ElementIdUnknown
};

static const ElementID contentmodel_8[] = {
    KElementIdsDescriptive,
    ElementIdAnimate,
    ElementIdAnimateColor,
    ElementIdSet,
    ElementIdUnknown
};

static const ElementID contentmodel_9[] = {
    KElementIdsGradient,
    ElementIdSolidColor,
    ElementIdPattern,
    ElementIdAnimate,
    ElementIdAnimateColor,
    ElementIdSet,
    ElementIdUnknown
};

static const ElementID contentmodel_11[] = {
    KElementIdsAnimation,
    KElementIdsShape,
    KElementIdsDescriptive,
    ElementIdUse,
    ElementIdText,
    ElementIdUnknown
};

static const ElementID contentmodel_12[] = {
    KElementIdsDescriptive,
    ElementIdAnimate,
    ElementIdAnimateTransform,
    ElementIdSet,
    ElementIdStop,
    ElementIdUnknown
};

static const ElementID contentmodel_13[] = {
    ElementIdAnimate,
    ElementIdAnimateColor,
    ElementIdSet,
    ElementIdUnknown
};

static const ElementID contentmodel_23[] = {
    KElementIdsDescriptive,
    ElementIdMpath,
    ElementIdUnknown
};

static const struct
{
    ElementID id;
    const ElementID* content;
} contentmodelmap[] = {
    {ElementIdUnknown, nullptr},
    {ElementIdA, contentmodel_1},
    {ElementIdAnimate, contentmodel_4},
    {ElementIdAnimateColor, contentmodel_4},
    {ElementIdAnimateMotion, contentmodel_23},
    {ElementIdAnimateTransform, contentmodel_4},
    {ElementIdCircle, contentmodel_2},
    {ElementIdClipPath, contentmodel_11},
    {ElementIdDefs, contentmodel_1},
    {ElementIdDesc, contentmodel_3},
    {ElementIdEllipse, contentmodel_2},
    {ElementIdG, contentmodel_1},
    {ElementIdImage, contentmodel_2},
    {ElementIdLine, contentmodel_2},
    {ElementIdLinearGradient, contentmodel_12},
    {ElementIdMarker, contentmodel_1},
    {ElementIdMask, contentmodel_1},
    {ElementIdMetadata, contentmodel_3},
    {ElementIdMpath, contentmodel_23},
    {ElementIdPath, contentmodel_2},
    {ElementIdPattern, contentmodel_1},
    {ElementIdPolygon, contentmodel_2},
    {ElementIdPolyline, contentmodel_2},
    {ElementIdRadialGradient, contentmodel_12},
    {ElementIdRect, contentmodel_2},
    {ElementIdSet, contentmodel_4},
    {ElementIdSolidColor, contentmodel_9},
    {ElementIdStop, contentmodel_13},
    {ElementIdStyle, contentmodel_3},
    {ElementIdSvg, contentmodel_1},
    {ElementIdSwitch, contentmodel_5},
    {ElementIdSymbol, contentmodel_1},
    {ElementIdText, contentmodel_6},
    {ElementIdTextPath, contentmodel_7},
    {ElementIdTitle, contentmodel_3},
    {ElementIdTref, contentmodel_8},
    {ElementIdTspan, contentmodel_7},
    {ElementIdUse, contentmodel_2},
    {ElementIdView, contentmodel_4},
    {ElementLastId, nullptr}
};

static const std::map<std::string, ElementID> elementmap = {
    {"a", ElementIdA},
    {"animate", ElementIdAnimate},
    {"animateColor", ElementIdAnimateColor},
    {"animateMotion", ElementIdAnimateMotion},
    {"animateTransform", ElementIdAnimateTransform},
    {"circle", ElementIdCircle},
    {"clipPath", ElementIdClipPath},
    {"defs", ElementIdDefs},
    {"desc", ElementIdDesc},
    {"ellipse", ElementIdEllipse},
    {"g", ElementIdG},
    {"image", ElementIdImage},
    {"line", ElementIdLine},
    {"linearGradient", ElementIdLinearGradient},
    {"marker", ElementIdMarker},
    {"mask", ElementIdMask},
    {"metadata", ElementIdMetadata},
    {"mpath", ElementIdMpath},
    {"path", ElementIdPath},
    {"pattern", ElementIdPattern},
    {"polygon", ElementIdPolygon},
    {"polyline", ElementIdPolyline},
    {"radialGradient", ElementIdRadialGradient},
    {"rect", ElementIdRect},
    {"set", ElementIdSet},
    {"stop", ElementIdStop},
    {"solidColor", ElementIdSolidColor},
    {"style", ElementIdStyle},
    {"svg", ElementIdSvg},
    {"switch", ElementIdSwitch},
    {"symbol", ElementIdSymbol},
    {"text", ElementIdText},
    {"textPath", ElementIdTextPath},
    {"title", ElementIdTitle},
    {"tref", ElementIdTref},
    {"tspan", ElementIdTspan},
    {"use", ElementIdUse},
    {"view", ElementIdView},
};

bool isElementPermitted(ElementID parentId, ElementID childId)
{
    if(const ElementID* content = contentmodelmap[parentId].content)
    {
        for(int i = 0;content[i] != ElementIdUnknown;i++)
            if(content[i] == childId)
                return true;
    }

    return false;
}

SVGElementHead* createElement(ElementID elementId, SVGDocument* document)
{
    switch(elementId)
    {
    case ElementIdSvg:
        return new SVGSVGElement(document);
    case ElementIdPath:
        return new SVGPathElement(document);
    case ElementIdG:
        return new SVGGElement(document);
    case ElementIdRect:
        return new SVGRectElement(document);
    case ElementIdCircle:
        return new SVGCircleElement(document);
    case ElementIdEllipse:
        return new SVGEllipseElement(document);
    case ElementIdLine:
        return new SVGLineElement(document);
    case ElementIdDefs:
        return new SVGDefsElement(document);
    case ElementIdPolygon:
        return new SVGPolygonElement(document);
    case ElementIdPolyline:
        return new SVGPolylineElement(document);
    case ElementIdStop:
        return new SVGStopElement(document);
    case ElementIdLinearGradient:
        return new SVGLinearGradientElement(document);
    case ElementIdRadialGradient:
        return new SVGRadialGradientElement(document);
    case ElementIdSymbol:
        return new SVGSymbolElement(document);
    case ElementIdUse:
        return new SVGUseElement(document);
    case ElementIdPattern:
        return new SVGPatternElement(document);
    case ElementIdMask:
        return new SVGMaskElement(document);
    case ElementIdClipPath:
        return new SVGClipPathElement(document);
    case ElementIdSolidColor:
        return new SVGSolidColorElement(document);
    default:
        break;
    }

    return nullptr;
}

const std::string& elementName(ElementID nameId)
{
    std::map<std::string, ElementID>::const_iterator it = elementmap.begin();
    std::map<std::string, ElementID>::const_iterator end = elementmap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

ElementID elementId(const std::string& name)
{
    std::map<std::string, ElementID>::const_iterator it = elementmap.find(name);
    return it != elementmap.end() ? it->second : ElementIdUnknown;
}

} // namespace Utils

} // namespace lunasvg
