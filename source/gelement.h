#ifndef GELEMENT_H
#define GELEMENT_H

#include "graphicselement.h"

namespace lunasvg {

class GElement final : public GraphicsElement {
public:
    GElement();

    void layout(LayoutContext* context, LayoutContainer* current) const;
};

} // namespace lunasvg

#endif // GELEMENT_H
