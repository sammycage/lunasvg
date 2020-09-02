#ifndef SVGGRADIENTELEMENT_H
#define SVGGRADIENTELEMENT_H

#include "svgpaintelement.h"
#include "svgurireference.h"
#include "svgenumeration.h"
#include "svgtransform.h"
#include "gradient.h"

namespace lunasvg {

class SVGStopElement;

struct GradientAttributes
{
    const SVGTransform* gradientTransform{nullptr};
    const SVGEnumeration<SpreadMethod>* spreadMethod{nullptr};
    const SVGEnumeration<UnitType>* gradientUnits{nullptr};
    std::vector<const SVGStopElement*> gradientStops;
};

class SVGGradientElement : public SVGPaintElement,
        public SVGURIReference
{
public:
    SVGGradientElement(DOMElementID elementId, SVGDocument* document);
    bool isSVGGradientElement() const { return true; }
    const DOMSVGTransform& gradientTransform() const { return m_gradientTransform; }
    const DOMSVGEnumeration<SpreadMethod>& spreadMethod() const { return m_spreadMethod; }
    const DOMSVGEnumeration<UnitType>& gradientUnits() const { return m_gradientUnits; }
    void setGradientAttributes(GradientAttributes& attributes) const;
    GradientStops buildGradientStops(const std::vector<const SVGStopElement*>& gradientStops) const;

private:
    DOMSVGTransform m_gradientTransform;
    DOMSVGEnumeration<SpreadMethod> m_spreadMethod;
    DOMSVGEnumeration<UnitType> m_gradientUnits;
};

} // namespace lunasvg

#endif // SVGGRADIENTELEMENT_H
