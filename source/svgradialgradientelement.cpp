#include "svgradialgradientelement.h"
#include "svgdocumentimpl.h"

namespace lunasvg {

SVGRadialGradientElement::SVGRadialGradientElement(SVGDocument* document)
    : SVGGradientElement(DOMElementIdRadialGradient, document),
      m_cx(DOMPropertyIdCx, LengthModeWidth, AllowNegativeLengths),
      m_cy(DOMPropertyIdCy, LengthModeHeight, AllowNegativeLengths),
      m_r(DOMPropertyIdR, LengthModeBoth, ForbidNegativeLengths),
      m_fx(DOMPropertyIdFx, LengthModeWidth, AllowNegativeLengths),
      m_fy(DOMPropertyIdFy, LengthModeHeight, AllowNegativeLengths)
{
    m_cx.setDefaultValue(fiftyPercent());
    m_cy.setDefaultValue(fiftyPercent());
    m_r.setDefaultValue(fiftyPercent());

    addToPropertyMap(m_cx);
    addToPropertyMap(m_cy);
    addToPropertyMap(m_r);
    addToPropertyMap(m_fx);
    addToPropertyMap(m_fy);
}

void SVGRadialGradientElement::collectGradientAttributes(RadialGradientAttributes& attributes) const
{
    std::set<const SVGGradientElement*> processedGradients;
    const SVGGradientElement* current = this;

    while(true)
    {
        current->setGradientAttributes(attributes);
        if(current->elementId() == DOMElementIdRadialGradient)
        {
            const SVGRadialGradientElement* radial = to<SVGRadialGradientElement>(current);
            if(!attributes.cx && radial->m_cx.isSpecified())
                attributes.cx = radial->m_cx.property();
            if(!attributes.cy && radial->m_cy.isSpecified())
                attributes.cy = radial->m_cy.property();
            if(!attributes.r && radial->m_r.isSpecified())
                attributes.r = radial->m_r.property();
            if(!attributes.fx && radial->m_fx.isSpecified())
                attributes.fx = radial->m_fx.property();
            if(!attributes.fy && radial->m_fy.isSpecified())
                attributes.fy = radial->m_fy.property();
        }

        processedGradients.insert(current);
        SVGElementImpl* ref = document()->impl()->resolveIRI(current->hrefValue());
        if(!ref || !ref->isSVGGradientElement())
            break;

        current = to<SVGGradientElement>(ref);
        if(processedGradients.find(current)!=processedGradients.end())
            break;
    }
}

Paint SVGRadialGradientElement::getPaint(const RenderState& state) const
{
    RadialGradientAttributes attributes;
    collectGradientAttributes(attributes);
    if(attributes.gradientStops.empty())
        return Paint();

    double cx, cy, r, fx, fy;
    UnitType units = attributes.gradientUnits ? attributes.gradientUnits->enumValue() : UnitTypeObjectBoundingBox;
    if(units == UnitTypeObjectBoundingBox)
    {
        cx = attributes.cx ? attributes.cx->value(state, 1) : 0.5;
        cy = attributes.cy ? attributes.cy->value(state, 1) : 0.5;
        r = attributes.r ? attributes.r->value(state, 1) : 0.5;
        fx = attributes.fx ? attributes.fx->value(state, 1) : cx;
        fy = attributes.fy ? attributes.fy->value(state, 1) : cy;
    }
    else
    {
        cx = attributes.cx ? attributes.cx->valueX(state) : fiftyPercent()->valueX(state);
        cy = attributes.cy ? attributes.cy->valueY(state) : fiftyPercent()->valueY(state);
        r = attributes.r ? attributes.r->value(state) : fiftyPercent()->value(state);
        fx = attributes.fx ? attributes.fx->valueX(state) : cx;
        fy = attributes.fy ? attributes.fy->valueY(state) : cy;
    }

    GradientStops stops = buildGradientStops(attributes.gradientStops);
    if(r == 0.0 || stops.size() == 1)
        return stops.back().second;

    SpreadMethod spread = attributes.spreadMethod ? attributes.spreadMethod->enumValue() : SpreadMethodPad;
    AffineTransform matrix;
    if(units == UnitTypeObjectBoundingBox)
    {
        matrix.translate(state.bbox.x, state.bbox.y);
        matrix.scale(state.bbox.width, state.bbox.height);
    }

    if(attributes.gradientTransform)
        matrix.multiply(attributes.gradientTransform->value());

    RadialGradient gradient(cx, cy, r, fx, fy);
    gradient.setStops(stops);
    gradient.setSpread(spread);
    gradient.setMatrix(matrix);

    return gradient;
}

SVGElementImpl* SVGRadialGradientElement::clone(SVGDocument* document) const
{
    SVGRadialGradientElement* e = new SVGRadialGradientElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
