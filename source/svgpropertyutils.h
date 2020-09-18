#ifndef SVGPROPERTYUTILS_H
#define SVGPROPERTYUTILS_H

#include <string>

namespace lunasvg {

enum DOMPropertyID
{
    DOMPropertyIdUnknown = 0,
    DOMPropertyIdClipPathUnits,
    DOMPropertyIdCx,
    DOMPropertyIdCy,
    DOMPropertyIdD,
    DOMPropertyIdFx,
    DOMPropertyIdFy,
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
    DOMPropertyIdStyle,
    DOMPropertyIdTransform,
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
    CSSPropertyIdMarker_End,
    CSSPropertyIdMarker_Mid,
    CSSPropertyIdMarker_Start,
    CSSPropertyIdMask,
    CSSPropertyIdOpacity,
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
    CSSPropertyIdVisibility,
    CSSPropertyLastId
};

namespace Utils {

const std::string& domPropertyName(DOMPropertyID nameId);
DOMPropertyID domPropertyId(const std::string& name);
const std::string& cssPropertyName(CSSPropertyID nameId);
CSSPropertyID cssPropertyId(const std::string& name);

} // namespace Utils

} // namespace lunasvg

#endif // SVGPROPERTYUTILS_H
