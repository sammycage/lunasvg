#ifndef SVGPATTERNELEMENT_H
#define SVGPATTERNELEMENT_H

#include "svgpaintelement.h"
#include "svgurireference.h"
#include "svgfittoviewbox.h"
#include "svgenumeration.h"
#include "svgtransform.h"
#include "svglength.h"

namespace lunasvg {

class SVGPatternElement;

struct PatternAttributes
{
    const SVGLength* x{nullptr};
    const SVGLength* y{nullptr};
    const SVGLength* width{nullptr};
    const SVGLength* height{nullptr};
    const SVGTransform* patternTransform{nullptr};
    const SVGEnumeration<UnitType>* patternUnits{nullptr};
    const SVGEnumeration<UnitType>* patternContentUnits{nullptr};
    const SVGRect* viewBox{nullptr};
    const SVGPreserveAspectRatio* preserveAspectRatio{nullptr};
    const SVGPatternElement* patternContentElement{nullptr};
};

class SVGPatternElement : public SVGPaintElement,
        public SVGURIReference,
        public SVGFitToViewBox
{
public:
    SVGPatternElement(SVGDocument* document);
    const DOMSVGLength& x() const { return m_x; }
    const DOMSVGLength& y() const { return m_y; }
    const DOMSVGLength& width() const { return m_width; }
    const DOMSVGLength& height() const { return m_height; }
    const DOMSVGTransform& patternTransform() const { return m_patternTransform; }
    const DOMSVGEnumeration<UnitType>& patternUnits() const { return m_patternUnits; }
    const DOMSVGEnumeration<UnitType>& patternContentUnits() const { return m_patternContentUnits; }
    void collectPatternAttributes(PatternAttributes& attributes) const;
    Paint getPaint(const RenderState& state) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_x;
    DOMSVGLength m_y;
    DOMSVGLength m_width;
    DOMSVGLength m_height;
    DOMSVGTransform m_patternTransform;
    DOMSVGEnumeration<UnitType> m_patternUnits;
    DOMSVGEnumeration<UnitType> m_patternContentUnits;
};

} // namespace lunasvg

#endif // SVGPATTERNELEMENT_H
