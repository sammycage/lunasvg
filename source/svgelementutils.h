#ifndef SVGELEMENTUTILS_H
#define SVGELEMENTUTILS_H

#include <string>

namespace lunasvg {

enum ElementID
{
    ElementIdUnknown = 0,
    ElementIdA,
    ElementIdAnimate,
    ElementIdAnimateColor,
    ElementIdAnimateMotion,
    ElementIdAnimateTransform,
    ElementIdCircle,
    ElementIdClipPath,
    ElementIdDefs,
    ElementIdDesc,
    ElementIdEllipse,
    ElementIdG,
    ElementIdImage,
    ElementIdLine,
    ElementIdLinearGradient,
    ElementIdMarker,
    ElementIdMask,
    ElementIdMetadata,
    ElementIdMpath,
    ElementIdPath,
    ElementIdPattern,
    ElementIdPolygon,
    ElementIdPolyline,
    ElementIdRadialGradient,
    ElementIdRect,
    ElementIdSet,
    ElementIdSolidColor,
    ElementIdStop,
    ElementIdStyle,
    ElementIdSvg,
    ElementIdSwitch,
    ElementIdSymbol,
    ElementIdText,
    ElementIdTextPath,
    ElementIdTitle,
    ElementIdTref,
    ElementIdTspan,
    ElementIdUse,
    ElementIdView,
    ElementLastId
};

class SVGElementHead;
class SVGDocument;

namespace Utils {

const std::string& elementName(ElementID nameId);
ElementID elementId(const std::string& name);
bool isElementPermitted(ElementID parentId, ElementID childId);
SVGElementHead* createElement(ElementID elementId, SVGDocument* document);

} // namespace Utils

} // namespace lunasvg

#endif // SVGELEMENTUTILS_H
