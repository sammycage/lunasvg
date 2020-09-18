#ifndef SVGELEMENTUTILS_H
#define SVGELEMENTUTILS_H

#include <string>

namespace lunasvg {

enum DOMElementID
{
    DOMElementIdUnknown = 0,
    DOMElementIdCircle,
    DOMElementIdClipPath,
    DOMElementIdDefs,
    DOMElementIdEllipse,
    DOMElementIdG,
    DOMElementIdLine,
    DOMElementIdLinearGradient,
    DOMElementIdMarker,
    DOMElementIdMask,
    DOMElementIdPath,
    DOMElementIdPattern,
    DOMElementIdPolygon,
    DOMElementIdPolyline,
    DOMElementIdRadialGradient,
    DOMElementIdRect,
    DOMElementIdSolidColor,
    DOMElementIdStop,
    DOMElementIdSvg,
    DOMElementIdSymbol,
    DOMElementIdUse,
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
