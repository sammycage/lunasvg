#include "svglineargradientelement.h"
#include "svgdocumentimpl.h"

namespace lunasvg {

SVGLinearGradientElement::SVGLinearGradientElement(SVGDocument* document)
    : SVGGradientElement(DOMElementIdLinearGradient, document),
      m_x1(DOMPropertyIdX1, LengthModeWidth, AllowNegativeLengths),
      m_y1(DOMPropertyIdY1, LengthModeHeight, AllowNegativeLengths),
      m_x2(DOMPropertyIdX2, LengthModeWidth, AllowNegativeLengths),
      m_y2(DOMPropertyIdY2, LengthModeHeight, AllowNegativeLengths)
{
    m_x2.setDefaultValue(hundredPercent());

    addToPropertyMap(m_x1);
    addToPropertyMap(m_y1);
    addToPropertyMap(m_x2);
    addToPropertyMap(m_y2);
}

void SVGLinearGradientElement::collectGradientAttributes(LinearGradientAttributes& attributes) const
{
    std::set<const SVGGradientElement*> processedGradients;
    const SVGGradientElement* current = this;

    while(true)
    {
        current->setGradientAttributes(attributes);
        if(current->elementId() == DOMElementIdLinearGradient)
        {
            const SVGLinearGradientElement* linear = to<SVGLinearGradientElement>(current);
            if(!attributes.x1 && linear->m_x1.isSpecified())
                attributes.x1 = linear->m_x1.property();
            if(!attributes.y1 && linear->m_y1.isSpecified())
                attributes.y1 = linear->m_y1.property();
            if(!attributes.x2 && linear->m_x2.isSpecified())
                attributes.x2 = linear->m_x2.property();
            if(!attributes.y2 && linear->m_y2.isSpecified())
                attributes.y2 = linear->m_y2.property();
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

Paint SVGLinearGradientElement::getPaint(const RenderState& state) const
{
    LinearGradientAttributes attributes;
    collectGradientAttributes(attributes);
    if(attributes.gradientStops.empty())
        return Paint();

    double x1, y1, x2, y2;
    UnitType units = attributes.gradientUnits ? attributes.gradientUnits->enumValue() : UnitTypeObjectBoundingBox;
    if(units == UnitTypeObjectBoundingBox)
    {
        x1 = attributes.x1 ? attributes.x1->value(state, 1) : 0.0;
        y1 = attributes.y1 ? attributes.y1->value(state, 1) : 0.0;
        x2 = attributes.x2 ? attributes.x2->value(state, 1) : 1.0;
        y2 = attributes.y2 ? attributes.y2->value(state, 1) : 0.0;
    }
    else
    {
        x1 = attributes.x1 ? attributes.x1->valueX(state) : 0.0;
        y1 = attributes.y1 ? attributes.y1->valueY(state) : 0.0;
        x2 = attributes.x2 ? attributes.x2->valueX(state) : hundredPercent()->valueX(state);
        y2 = attributes.y2 ? attributes.y2->valueY(state) : 0.0;
    }

    GradientStops stops = buildGradientStops(attributes.gradientStops);
    if((x1 == x2 && y1 == y2) || stops.size() == 1)
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

    LinearGradient gradient(x1, y1, x2, y2);
    gradient.setStops(stops);
    gradient.setSpread(spread);
    gradient.setMatrix(matrix);

    return gradient;
}

SVGElementImpl* SVGLinearGradientElement::clone(SVGDocument* document) const
{
    SVGLinearGradientElement* e = new SVGLinearGradientElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
