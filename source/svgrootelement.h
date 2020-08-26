#ifndef SVGROOTELEMENT_H
#define SVGROOTELEMENT_H

#include "svgsvgelement.h"

namespace lunasvg {

class SVGRootElement : public SVGSVGElement
{
public:
    SVGRootElement(SVGDocument* document);
    bool isSVGRootElement() const { return true; }
    void renderToBitmap(Bitmap& bitmap, const Rect& viewBox, double dpi, std::uint32_t bgColor) const;
};

} // namespace lunasvg

#endif // SVGROOTELEMENT_H
