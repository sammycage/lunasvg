#include "svgpropertyutils.h"
#include "svgstring.h"

#include <unordered_map>

namespace lunasvg {

namespace Utils {

static const std::unordered_map<std::string, DOMPropertyID> dompropertymap = {
    {"class", DOMPropertyIdClass},
    {"clipPathUnits", DOMPropertyIdClipPathUnits},
    {"cx", DOMPropertyIdCx},
    {"cy", DOMPropertyIdCy},
    {"d", DOMPropertyIdD},
    {"dx", DOMPropertyIdDx},
    {"dy", DOMPropertyIdDy},
    {"fx", DOMPropertyIdFx},
    {"fy", DOMPropertyIdFy},
    {"fr", DOMPropertyIdFr},
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
    {"startOffset", DOMPropertyIdStartOffset},
    {"style", DOMPropertyIdStyle},
    {"transform", DOMPropertyIdTransform},
    {"type", DOMPropertyIdType},
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

static const std::unordered_map<std::string, SMILPropertyID> smilpropertymap = {
    {"accumulate", SMILPropertyIdAccumulate},
    {"additive", SMILPropertyIdAdditive},
    {"attributeName", SMILPropertyIdAttributeName},
    {"attributeType", SMILPropertyIdAttributeType},
    {"begin", SMILPropertyIdBegin},
    {"by", SMILPropertyIdBy},
    {"calcMode", SMILPropertyIdCalcMode},
    {"dur", SMILPropertyIdDur},
    {"end", SMILPropertyIdEnd},
    {"fill", SMILPropertyIdFill},
    {"from", SMILPropertyIdFrom},
    {"keyPoints", SMILPropertyIdKeyPoints},
    {"keySplines", SMILPropertyIdKeySplines},
    {"keyTimes", SMILPropertyIdKeyTimes},
    {"max", SMILPropertyIdMax},
    {"min", SMILPropertyIdMin},
    {"origin", SMILPropertyIdOrigin},
    {"path", SMILPropertyIdPath},
    {"repeatCount", SMILPropertyIdRepeatCount},
    {"repeatDur", SMILPropertyIdRepeatDur},
    {"restart", SMILPropertyIdRestart},
    {"rotate", SMILPropertyIdRotate},
    {"to", SMILPropertyIdTo},
    {"type", SMILPropertyIdType},
    {"values", SMILPropertyIdValues},
};

static const std::unordered_map<std::string, CSSPropertyID> csspropertymap = {
    {"clip-path", CSSPropertyIdClip_Path},
    {"clip-rule", CSSPropertyIdClip_Rule},
    {"color", CSSPropertyIdColor},
    {"display", CSSPropertyIdDisplay},
    {"fill", CSSPropertyIdFill},
    {"fill-opacity", CSSPropertyIdFill_Opacity},
    {"fill-rule", CSSPropertyIdFill_Rule},
    {"font-family", CSSPropertyIdFont_Family},
    {"font-size", CSSPropertyIdFont_Size},
    {"font-style", CSSPropertyIdFont_Style},
    {"font-weight", CSSPropertyIdFont_Weight},
    {"marker-end", CSSPropertyIdMarker_End},
    {"marker-mid", CSSPropertyIdMarker_Mid},
    {"marker-start", CSSPropertyIdMarker_Start},
    {"mask", CSSPropertyIdMask},
    {"opacity", CSSPropertyIdOpacity},
    {"overflow", CSSPropertyIdOverflow},
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
    {"text-anchor", CSSPropertyIdText_Anchor},
    {"text-decoration", CSSPropertyIdText_Decoration},
    {"visibility", CSSPropertyIdVisibility},
};

const std::string& domPropertyName(DOMPropertyID nameId)
{
    std::unordered_map<std::string, DOMPropertyID>::const_iterator it = dompropertymap.begin();
    std::unordered_map<std::string, DOMPropertyID>::const_iterator end = dompropertymap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

DOMPropertyID domPropertyId(const std::string& name)
{
    std::unordered_map<std::string, DOMPropertyID>::const_iterator it = dompropertymap.find(name);
    return it != dompropertymap.end() ? it->second : DOMPropertyIdUnknown;
}

const std::string& smilPropertyName(SMILPropertyID nameId)
{
    std::unordered_map<std::string, SMILPropertyID>::const_iterator it = smilpropertymap.begin();
    std::unordered_map<std::string, SMILPropertyID>::const_iterator end = smilpropertymap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

SMILPropertyID smilPropertyId(const std::string& name)
{
    std::unordered_map<std::string, SMILPropertyID>::const_iterator it = smilpropertymap.find(name);
    return it != smilpropertymap.end() ? it->second : SMILPropertyIdUnknown;
}

const std::string& cssPropertyName(CSSPropertyID nameId)
{
    std::unordered_map<std::string, CSSPropertyID>::const_iterator it = csspropertymap.begin();
    std::unordered_map<std::string, CSSPropertyID>::const_iterator end = csspropertymap.end();

    for(;it != end;++it)
        if(it->second == nameId)
            return it->first;

    return KEmptyString;
}

CSSPropertyID cssPropertyId(const std::string& name)
{
    std::unordered_map<std::string, CSSPropertyID>::const_iterator it = csspropertymap.find(name);
    return it != csspropertymap.end() ? it->second : CSSPropertyIdUnknown;
}

} // namespace Utils

} // namespace lunasvg
