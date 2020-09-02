#include "svgsvgelement.h"

namespace lunasvg {

SVGSVGElement::SVGSVGElement(SVGDocument* document)
    : SVGGraphicsElement(DOMElementIdSvg, document),
      SVGFitToViewBox(this),
      m_x(DOMPropertyIdX, LengthModeWidth, AllowNegativeLengths),
      m_y(DOMPropertyIdY, LengthModeHeight, AllowNegativeLengths),
      m_width(DOMPropertyIdWidth, LengthModeWidth, ForbidNegativeLengths),
      m_height(DOMPropertyIdHeight, LengthModeHeight, ForbidNegativeLengths)
{
    m_height.setDefaultValue(hundredPercent());
    m_width.setDefaultValue(hundredPercent());

    addToPropertyMap(m_x);
    addToPropertyMap(m_y);
    addToPropertyMap(m_width);
    addToPropertyMap(m_height);
}

void SVGSVGElement::render(RenderContext& context) const
{
    if(m_width.isZero() || m_height.isZero() || style().isDisplayNone())
    {
        context.skipElement();
        return;
    }

    const RenderState& state = context.state();
    Rect viewPort;
    if(state.element->elementId() == DOMElementIdUse)
    {
        viewPort = state.viewPort;
    }
    else
    {
        viewPort.x = m_x.value(state);
        viewPort.y = m_y.value(state);
        viewPort.width = m_width.value(state);
        viewPort.height = m_height.value(state);
    }

    SVGGraphicsElement::render(context);
    RenderState& newState = context.state();
    if(viewBox().isSpecified() && viewBox().property()->isValid())
    {
        newState.matrix.multiply(calculateViewBoxTransform(viewPort, viewBox().property()->value()));
        newState.viewPort = viewBox().property()->value();
    }
    else
    {
        newState.matrix.translate(viewPort.x, viewPort.y);
    }
}

SVGElementImpl* SVGSVGElement::clone(SVGDocument* document) const
{
    SVGSVGElement* e = new SVGSVGElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
