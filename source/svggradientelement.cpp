#include "svggradientelement.h"
#include "svgelementtail.h"
#include "svgstopelement.h"

namespace lunasvg {

SVGGradientElement::SVGGradientElement(DOMElementID elementId, SVGDocument* document)
    : SVGPaintElement(elementId, document),
      SVGURIReference (this),
      m_gradientTransform(DOMPropertyIdGradientTransform),
      m_spreadMethod(DOMPropertyIdSpreadMethod),
      m_gradientUnits(DOMPropertyIdGradientUnits)
{
    addToPropertyMap(m_gradientTransform);
    addToPropertyMap(m_spreadMethod);
    addToPropertyMap(m_gradientUnits);
}

void SVGGradientElement::setGradientAttributes(GradientAttributes& attributes) const
{
    if(!attributes.gradientTransform && m_gradientTransform.isSpecified())
        attributes.gradientTransform = m_gradientTransform.property();
    if(!attributes.spreadMethod && m_spreadMethod.isSpecified())
        attributes.spreadMethod = m_spreadMethod.property();
    if(!attributes.gradientUnits && m_gradientUnits.isSpecified())
        attributes.gradientUnits = m_gradientUnits.property();
    if(attributes.gradientStops.empty())
    {
        const SVGElementImpl* e = next;
        while(e != tail)
        {
            if(e->elementId()==DOMElementIdStop)
                attributes.gradientStops.push_back(to<SVGStopElement>(e));
            e = e->next;
        }
    }
}

GradientStops SVGGradientElement::buildGradientStops(const std::vector<const SVGStopElement*>& gradientStops) const
{
    GradientStops stops(gradientStops.size());
    double prevOffset = 0.0;
    for(unsigned int i = 0;i < gradientStops.size();i++)
    {
        const SVGStopElement* stop = gradientStops[i];
        double offset = std::min(std::max(prevOffset, stop->offsetValue()), 1.0);
        prevOffset = offset;
        stops[i] = GradientStop(offset, stop->stopColorWithOpacity());
    }

    return stops;
}

} // namespace lunasvg
