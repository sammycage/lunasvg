#ifndef SVGELEMENT_H
#define SVGELEMENT_H

#include "graphicselement.h"

namespace lunasvg {

class Document;
class LayoutSymbol;

class SVGElement final : public GraphicsElement {
public:
    SVGElement();

    Length x() const;
    Length y() const;
    Length width() const;
    Length height() const;

    Rect viewBox() const;
    PreserveAspectRatio preserveAspectRatio() const;
    std::unique_ptr<LayoutSymbol> layoutTree(const Document* document) const;
    void layout(LayoutContext* context, LayoutContainer* current) const final;
};

} // namespace lunasvg

#endif // SVGELEMENT_H
