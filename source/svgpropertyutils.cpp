#include "svgpropertyutils.h"
#include "svgstring.h"

#include <map>

namespace lunasvg {

namespace Utils {

static const std::map<std::string, DOMPropertyID> dompropertymap = {
    {"clipPathUnits", DOMPropertyIdClipPathUnits},
    {"cx", DOMPropertyIdCx},
    {"cy", DOMPropertyIdCy},
    {"d", DOMPropertyIdD},
    {"fx", DOMPropertyIdFx},
    {"fy", DOMPropertyIdFy},
    {"gradientTransform", DOMPropertyIdGradientTransform},
    {"gradientUnits", DOMPropertyIdGradientUnits},
    {"height", DOMPropertyIdHeight},
    {"id", DOMPropertyIdId},
    {"markerHeight", DOMPropertyIdMarkerHeight},
    {"markerUnits", DOMPropertyIdMarkerUnits},
    {"markerWidth", DOMPropertyIdMarkerWidth},
    {"maskContentUnits", DOMPropertyIdMaskContentUnits},
    {"maskUnits", DOMPropertyIdMaskUnits},
    {"offset", DOMPropertyIdOffset},
    {"orient", DOMPropertyIdOrient},
    {"path", DOMPropertyIdPath},
    {"patternContentUnits", DOMPropertyIdPatternContentUnits},
    {"patternTransform", DOMPropertyIdPatternTransform},
    {"patternUnits", DOMPropertyIdPatternUnits},
    {"points", DOMPropertyIdPoints},
    {"preserveAspectRatio", DOMPropertyIdPreserveAspectRatio},
    {"r", DOMPropertyIdR},
    {"refX", DOMPropertyIdRefX},
    {"refY", DOMPropertyIdRefY},
    {"rotate", DOMPropertyIdRotate},
    {"rx", DOMPropertyIdRx},
    {"ry", DOMPropertyIdRy},
    {"spreadMethod", DOMPropertyIdSpreadMethod},
    {"style", DOMPropertyIdStyle},
    {"transform", DOMPropertyIdTransform},
    {"viewBox", DOMPropertyIdViewBox},
    {"width", DOMPropertyIdWidth},
    {"x", DOMPropertyIdX},
    {"x1", DOMPropertyIdX1},
    {"x2", DOMPropertyIdX2},
    {"xlink:href", DOMPropertyIdHref},
    {"y", DOMPropertyIdY},
    {"y1", DOMPropertyIdY1},
    {"y2", DOMPropertyIdY2},
};

static const std::map<std::string, CSSPropertyID> csspropertymap = {
    {"clip-path", CSSPropertyIdClip_Path},
    {"clip-rule", CSSPropertyIdClip_Rule},
    {"color", CSSPropertyIdColor},
    {"display", CSSPropertyIdDisplay},
    {"fill", CSSPropertyIdFill},
    {"fill-opacity", CSSPropertyIdFill_Opacity},
    {"fill-rule", CSSPropertyIdFill_Rule},
    {"marker-end", CSSPropertyIdMarker_End},
    {"marker-mid", CSSPropertyIdMarker_Mid},
    {"marker-start", CSSPropertyIdMarker_Start},
    {"mask", CSSPropertyIdMask},
    {"opacity", CSSPropertyIdOpacity},
    {"solid-color", CSSPropertyIdSolid_Color},
    {"solid-opacity", CSSPropertyIdSolid_Opacity},
    {"stop-color", CSSPropertyIdStop_Color},
    {"stop-opacity", CSSPropertyIdStop_Opacity},
    {"stroke", CSSPropertyIdStroke},
    {"stroke-dasharray", CSSPropertyIdStroke_Dasharray},
    {"stroke-dashoffset", CSSPropertyIdStroke_Dashoffset},
    {"stroke-linecap", CSSPropertyIdStroke_Linecap},
    {"stroke-linejoin", CSSPropertyIdStroke_Linejoin},
    {"stroke-miterlimit", CSSPropertyIdStroke_Miterlimit},
    {"stroke-opacity", CSSPropertyIdStroke_Opacity},
    {"stroke-width", CSSPropertyIdStroke_Width},
    {"visibility", CSSPropertyIdVisibility},
};

const std::string& domPropertyName(DOMPropertyID nameId)
{
    std::map<std::string, DOMPropertyID>::const_iterator it = dompropertymap.begin();
    std::map<std::string, DOMPropertyID>::const_iterator end = dompropertymap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

DOMPropertyID domPropertyId(const std::string& name)
{
    std::map<std::string, DOMPropertyID>::const_iterator it = dompropertymap.find(name);
    return it != dompropertymap.end() ? it->second : DOMPropertyIdUnknown;
}

const std::string& cssPropertyName(CSSPropertyID nameId)
{
    std::map<std::string, CSSPropertyID>::const_iterator it = csspropertymap.begin();
    std::map<std::string, CSSPropertyID>::const_iterator end = csspropertymap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

CSSPropertyID cssPropertyId(const std::string& name)
{
    std::map<std::string, CSSPropertyID>::const_iterator it = csspropertymap.find(name);
    return it != csspropertymap.end() ? it->second : CSSPropertyIdUnknown;
}

} // namespace Utils

} // namespace lunasvg
