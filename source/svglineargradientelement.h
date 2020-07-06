#ifndef SVGLINEARGRADIENTELEMENT_H
#define SVGLINEARGRADIENTELEMENT_H

#include "svggradientelement.h"
#include "svglength.h"

namespace lunasvg {

struct LinearGradientAttributes : public GradientAttributes
{
    const SVGLength* x1{nullptr};
    const SVGLength* y1{nullptr};
    const SVGLength* x2{nullptr};
    const SVGLength* y2{nullptr};
};

class SVGLinearGradientElement : public SVGGradientElement
{
public:
    SVGLinearGradientElement(SVGDocument* document);
    const DOMSVGLength& x1() const { return m_x1; }
    const DOMSVGLength& y1() const { return m_y1; }
    const DOMSVGLength& x2() const { return m_x2; }
    const DOMSVGLength& y2() const { return m_y2; }
    void collectGradientAttributes(LinearGradientAttributes& attributes) const;
    Paint getPaint(const RenderState& state) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_x1;
    DOMSVGLength m_y1;
    DOMSVGLength m_x2;
    DOMSVGLength m_y2;
};

} // namespace lunasvg

#endif // SVGLINEARGRADIENTELEMENT_H
