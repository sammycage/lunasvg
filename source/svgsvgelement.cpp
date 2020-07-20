#include "svgsvgelement.h"

namespace lunasvg {

SVGSVGElement::SVGSVGElement(SVGDocument* document) :
    SVGGraphicsElement(ElementIdSvg, document),
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
    Rect _viewPort;
    if(state.element->isSVGRootElement() || state.element->elementId() == ElementIdUse)
    {
        _viewPort = state.viewPort;
    }
    else
    {
        _viewPort.x = m_x.value(state);
        _viewPort.y = m_y.value(state);
        _viewPort.width = m_width.value(state);
        _viewPort.height = m_height.value(state);
    }

    Rect _viewBox;
    if(viewBox().isSpecified() && viewBox().property()->isValid())
    {
        _viewBox = viewBox().property()->value();
    }
    else
    {
        _viewBox.x = m_x.value(state);
        _viewBox.y = m_y.value(state);
        _viewBox.width = m_width.value(state);
        _viewBox.height = m_height.value(state);
    }

    SVGGraphicsElement::render(context);
    RenderState& newState = context.state();
    newState.matrix.multiply(calculateViewBoxTransform(_viewPort, _viewBox));
    newState.viewPort = _viewBox;
}

SVGElementImpl* SVGSVGElement::clone(SVGDocument* document) const
{
    SVGSVGElement* e = new SVGSVGElement(document);
    baseClone(*e);
    return  e;
}

} // namespace lunasvg
