#include "svgfittoviewbox.h"
#include "svgelementhead.h"
#include "affinetransform.h"

namespace lunasvg {

SVGFitToViewBox::SVGFitToViewBox(SVGElementHead* element) :
    m_viewBox(DOMPropertyIdViewBox),
    m_preserveAspectRatio(DOMPropertyIdPreserveAspectRatio)
{
    element->addToPropertyMap(m_viewBox);
    element->addToPropertyMap(m_preserveAspectRatio);
}

AffineTransform SVGFitToViewBox::calculateViewBoxTransform(const Rect& viewPort) const
{
    const Rect& _viewBox = m_viewBox.isSpecified() && m_viewBox.property()->isValid() ? m_viewBox.property()->value() : viewPort;
    const SVGPreserveAspectRatio* positioning = m_preserveAspectRatio.isSpecified() ? m_preserveAspectRatio.property() : SVGPreserveAspectRatio::defaultValue();

    return positioning->getMatrix(viewPort, _viewBox);
}

} // namespace lunasvg
