#ifndef SVGELEMENTUTILS_H
#define SVGELEMENTUTILS_H

#include <string>

namespace lunasvg {

enum DOMElementID
{
    DOMElementIdUnknown = 0,
    DOMElementIdA,
    DOMElementIdAnimate,
    DOMElementIdAnimateColor,
    DOMElementIdAnimateMotion,
    DOMElementIdAnimateTransform,
    DOMElementIdCircle,
    DOMElementIdClipPath,
    DOMElementIdDefs,
    DOMElementIdDesc,
    DOMElementIdEllipse,
    DOMElementIdG,
    DOMElementIdImage,
    DOMElementIdLine,
    DOMElementIdLinearGradient,
    DOMElementIdMarker,
    DOMElementIdMask,
    DOMElementIdMetadata,
    DOMElementIdMpath,
    DOMElementIdPath,
    DOMElementIdPattern,
    DOMElementIdPolygon,
    DOMElementIdPolyline,
    DOMElementIdRadialGradient,
    DOMElementIdRect,
    DOMElementIdSet,
    DOMElementIdSolidColor,
    DOMElementIdStop,
    DOMElementIdStyle,
    DOMElementIdSvg,
    DOMElementIdSwitch,
    DOMElementIdSymbol,
    DOMElementIdText,
    DOMElementIdTextPath,
    DOMElementIdTitle,
    DOMElementIdTref,
    DOMElementIdTspan,
    DOMElementIdUse,
    DOMElementIdView,
    DOMElementLastId
};

class SVGElementHead;
class SVGDocument;

namespace Utils {

const std::string& domElementName(DOMElementID nameId);
DOMElementID domElementId(const std::string& name);
bool isElementPermitted(DOMElementID parentId, DOMElementID childId);
SVGElementHead* createElement(DOMElementID elementId, SVGDocument* document);

} // namespace Utils

} // namespace lunasvg

#endif // SVGELEMENTUTILS_H
