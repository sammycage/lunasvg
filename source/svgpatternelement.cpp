#include "svgpatternelement.h"
#include "svgdocumentimpl.h"
#include "svgelementtail.h"
#include "pattern.h"

namespace lunasvg {

SVGPatternElement::SVGPatternElement(SVGDocument *document) :
    SVGPaintElement(ElementIdPattern, document),
    SVGURIReference(this),
    SVGFitToViewBox(this),
    m_x(DOMPropertyIdX, LengthModeWidth, AllowNegativeLengths),
    m_y(DOMPropertyIdY, LengthModeHeight, AllowNegativeLengths),
    m_width(DOMPropertyIdWidth, LengthModeWidth, ForbidNegativeLengths),
    m_height(DOMPropertyIdHeight, LengthModeHeight, ForbidNegativeLengths),
    m_patternTransform(DOMPropertyIdPatternTransform),
    m_patternUnits(DOMPropertyIdPatternUnits),
    m_patternContentUnits(DOMPropertyIdPatternContentUnits)
{
    addToPropertyMap(m_x);
    addToPropertyMap(m_y);
    addToPropertyMap(m_width);
    addToPropertyMap(m_height);
    addToPropertyMap(m_patternTransform);
    addToPropertyMap(m_patternUnits);
    addToPropertyMap(m_patternContentUnits);
}

void SVGPatternElement::collectPatternAttributes(PatternAttributes& attributes) const
{
    std::set<const SVGPatternElement*> processedGradients;
    const SVGPatternElement* current = this;

    while(true)
    {
        if(!attributes.x && current->x().isSpecified())
            attributes.x = current->x().property();
        if(!attributes.y && current->y().isSpecified())
            attributes.y = current->y().property();
        if(!attributes.width && current->width().isSpecified())
            attributes.width = current->width().property();
        if(!attributes.height && current->height().isSpecified())
            attributes.height = current->height().property();
        if(!attributes.patternTransform && current->patternTransform().isSpecified())
            attributes.patternTransform = current->patternTransform().property();
        if(!attributes.patternUnits && current->patternUnits().isSpecified())
            attributes.patternUnits = current->patternUnits().property();
        if(!attributes.patternContentUnits && current->patternContentUnits().isSpecified())
            attributes.patternContentUnits = current->patternContentUnits().property();
        if(!attributes.viewBox && current->viewBox().isSpecified())
            attributes.viewBox = current->viewBox().property();
        if(!attributes.preserveAspectRatio && current->preserveAspectRatio().isSpecified())
            attributes.preserveAspectRatio = current->preserveAspectRatio().property();
        if(!attributes.patternContentElement && current->next != current->tail)
            attributes.patternContentElement = current;

        processedGradients.insert(current);
        SVGElementImpl* ref = document()->impl()->resolveIRI(current->hrefValue());
        if(!ref || ref->elementId() != ElementIdPattern)
            break;

        current = to<SVGPatternElement>(ref);
        if(processedGradients.find(current)!=processedGradients.end())
            break;
    }
}

Paint SVGPatternElement::getPaint(const RenderState& state) const
{
    PatternAttributes attributes;
    collectPatternAttributes(attributes);

    double x, y, w, h;
    UnitType units = attributes.patternUnits ? attributes.patternUnits->enumValue() : UnitTypeObjectBoundingBox;
    if(units == UnitTypeObjectBoundingBox)
    {
        x = attributes.x ? state.bbox.x + attributes.x->value(state, 1) * state.bbox.width : 0;
        y = attributes.y ? state.bbox.y + attributes.y->value(state, 1) * state.bbox.height : 0;
        w = attributes.width ? attributes.width->value(state, 1) * state.bbox.width : 0;
        h = attributes.height ? attributes.height->value(state, 1) * state.bbox.height : 0;
    }
    else
    {
        x = attributes.x ? attributes.x->valueX(state) : 0;
        y = attributes.y ? attributes.y->valueY(state) : 0;
        w = attributes.width ? attributes.width->valueX(state) : 0;
        h = attributes.height ? attributes.height->valueY(state) : 0;
    }

    if(w == 0.0 || h == 0.0 || attributes.patternContentElement == nullptr || RenderBreaker::hasElement(attributes.patternContentElement))
        return Paint();

    RenderContext newContext(this, RenderModeDisplay);
    RenderState& newState = newContext.state();
    newState.element = attributes.patternContentElement;
    newState.canvas.reset(std::uint32_t(std::ceil(w)), std::uint32_t(std::ceil(h)));
    newState.style.add(style());
    newState.viewPort = state.viewPort;
    newState.color = KRgbBlack;
    newState.dpi = state.dpi;

    if(attributes.viewBox)
    {
        const SVGPreserveAspectRatio* positioning = attributes.preserveAspectRatio ? attributes.preserveAspectRatio : SVGPreserveAspectRatio::defaultValue();
        newState.matrix.multiply(positioning->getMatrix(Rect(0, 0, w, h), attributes.viewBox->value()));
    }
    else if(attributes.patternContentUnits && attributes.patternContentUnits->enumValue() == UnitTypeObjectBoundingBox)
    {
        newState.matrix.scale(state.bbox.width, state.bbox.height);
    }

    RenderBreaker::registerElement(attributes.patternContentElement);
    newContext.render(attributes.patternContentElement->next, attributes.patternContentElement->tail->prev);
    RenderBreaker::unregisterElement(attributes.patternContentElement);

    AffineTransform matrix(1, 0, 0, 1, x, y);
    if(attributes.patternTransform)
        matrix.postmultiply(attributes.patternTransform->value());

    Pattern pattern(newState.canvas);
    pattern.setTileMode(TileModeRepeat);
    pattern.setMatrix(matrix);

    return pattern;
}

SVGElementImpl* SVGPatternElement::clone(SVGDocument* document) const
{
    SVGPatternElement* e = new SVGPatternElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
