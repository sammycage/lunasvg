#ifndef SVGROOTELEMENT_H
#define SVGROOTELEMENT_H

#include "svgsvgelement.h"

namespace lunasvg {

class SVGRootElement : public SVGSVGElement
{
public:
    SVGRootElement(SVGDocument* document);
    bool isSVGRootElement() const { return true; }
    Bitmap renderToBitmap(std::uint32_t width, std::uint32_t height, double dpi, unsigned int bgColor) const;
};

} // namespace lunasvg

#endif // SVGROOTELEMENT_H
