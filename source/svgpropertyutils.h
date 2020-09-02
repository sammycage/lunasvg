#ifndef SVGPROPERTYUTILS_H
#define SVGPROPERTYUTILS_H

#include <string>

namespace lunasvg {

enum DOMPropertyID
{
    DOMPropertyIdUnknown = 0,
    DOMPropertyIdClass,
    DOMPropertyIdClipPathUnits,
    DOMPropertyIdCx,
    DOMPropertyIdCy,
    DOMPropertyIdD,
    DOMPropertyIdDx,
    DOMPropertyIdDy,
    DOMPropertyIdFx,
    DOMPropertyIdFy,
    DOMPropertyIdFr,
    DOMPropertyIdGradientTransform,
    DOMPropertyIdGradientUnits,
    DOMPropertyIdHeight,
    DOMPropertyIdHref,
    DOMPropertyIdId,
    DOMPropertyIdMarkerHeight,
    DOMPropertyIdMarkerUnits,
    DOMPropertyIdMarkerWidth,
    DOMPropertyIdMaskContentUnits,
    DOMPropertyIdMaskUnits,
    DOMPropertyIdOffset,
    DOMPropertyIdOrient,
    DOMPropertyIdPath,
    DOMPropertyIdPatternContentUnits,
    DOMPropertyIdPatternTransform,
    DOMPropertyIdPatternUnits,
    DOMPropertyIdPoints,
    DOMPropertyIdPreserveAspectRatio,
    DOMPropertyIdR,
    DOMPropertyIdRefX,
    DOMPropertyIdRefY,
    DOMPropertyIdRotate,
    DOMPropertyIdRx,
    DOMPropertyIdRy,
    DOMPropertyIdSpreadMethod,
    DOMPropertyIdStartOffset,
    DOMPropertyIdStyle,
    DOMPropertyIdTransform,
    DOMPropertyIdType,
    DOMPropertyIdViewBox,
    DOMPropertyIdWidth,
    DOMPropertyIdX,
    DOMPropertyIdX1,
    DOMPropertyIdX2,
    DOMPropertyIdY,
    DOMPropertyIdY1,
    DOMPropertyIdY2,
    DOMPropertyLastId
};

enum SMILPropertyID
{
    SMILPropertyIdUnknown = 0,
    SMILPropertyIdAccumulate,
    SMILPropertyIdAdditive,
    SMILPropertyIdAttributeName,
    SMILPropertyIdAttributeType,
    SMILPropertyIdBegin,
    SMILPropertyIdBy,
    SMILPropertyIdCalcMode,
    SMILPropertyIdDur,
    SMILPropertyIdEnd,
    SMILPropertyIdFill,
    SMILPropertyIdFrom,
    SMILPropertyIdKeyPoints,
    SMILPropertyIdKeySplines,
    SMILPropertyIdKeyTimes,
    SMILPropertyIdMax,
    SMILPropertyIdMin,
    SMILPropertyIdOrigin,
    SMILPropertyIdPath,
    SMILPropertyIdRepeatCount,
    SMILPropertyIdRepeatDur,
    SMILPropertyIdRestart,
    SMILPropertyIdRotate,
    SMILPropertyIdTo,
    SMILPropertyIdType,
    SMILPropertyIdValues,
    SMILPropertyLastId
};

enum CSSPropertyID
{
    CSSPropertyIdUnknown = 0,
    CSSPropertyIdClip_Path,
    CSSPropertyIdClip_Rule,
    CSSPropertyIdColor,
    CSSPropertyIdDisplay,
    CSSPropertyIdFill,
    CSSPropertyIdFill_Opacity,
    CSSPropertyIdFill_Rule,
    CSSPropertyIdFont_Family,
    CSSPropertyIdFont_Size,
    CSSPropertyIdFont_Style,
    CSSPropertyIdFont_Weight,
    CSSPropertyIdMarker_End,
    CSSPropertyIdMarker_Mid,
    CSSPropertyIdMarker_Start,
    CSSPropertyIdMask,
    CSSPropertyIdOpacity,
    CSSPropertyIdOverflow,
    CSSPropertyIdSolid_Color,
    CSSPropertyIdSolid_Opacity,
    CSSPropertyIdStop_Color,
    CSSPropertyIdStop_Opacity,
    CSSPropertyIdStroke,
    CSSPropertyIdStroke_Dasharray,
    CSSPropertyIdStroke_Dashoffset,
    CSSPropertyIdStroke_Linecap,
    CSSPropertyIdStroke_Linejoin,
    CSSPropertyIdStroke_Miterlimit,
    CSSPropertyIdStroke_Opacity,
    CSSPropertyIdStroke_Width,
    CSSPropertyIdText_Anchor,
    CSSPropertyIdText_Decoration,
    CSSPropertyIdVisibility,
    CSSPropertyLastId
};

namespace Utils {

const std::string& domPropertyName(DOMPropertyID nameId);
DOMPropertyID domPropertyId(const std::string& name);
const std::string& smilPropertyName(SMILPropertyID nameId);
SMILPropertyID smilPropertyId(const std::string& name);
const std::string& cssPropertyName(CSSPropertyID nameId);
CSSPropertyID cssPropertyId(const std::string& name);

} // namespace Utils

} // namespace lunasvg

#endif // SVGPROPERTYUTILS_H
