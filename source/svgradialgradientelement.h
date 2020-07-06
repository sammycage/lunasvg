#ifndef SVGRADIALGRADIENTELEMENT_H
#define SVGRADIALGRADIENTELEMENT_H

#include "svggradientelement.h"
#include "svglength.h"

namespace lunasvg {

struct RadialGradientAttributes : public GradientAttributes
{
    const SVGLength* cx{nullptr};
    const SVGLength* cy{nullptr};
    const SVGLength* r{nullptr};
    const SVGLength* fx{nullptr};
    const SVGLength* fy{nullptr};
};

class SVGRadialGradientElement : public SVGGradientElement
{
public:
    SVGRadialGradientElement(SVGDocument* document);
    const DOMSVGLength& cx() const { return m_cx; }
    const DOMSVGLength& cy() const { return m_cy; }
    const DOMSVGLength& r() const { return m_r; }
    const DOMSVGLength& fx() const { return m_fx; }
    const DOMSVGLength& fy() const { return m_fy; }
    void collectGradientAttributes(RadialGradientAttributes& attributes) const;
    Paint getPaint(const RenderState& state) const;
    SVGElementImpl* clone(SVGDocument* document) const;

private:
    DOMSVGLength m_cx;
    DOMSVGLength m_cy;
    DOMSVGLength m_r;
    DOMSVGLength m_fx;
    DOMSVGLength m_fy;
};

} // namespace lunasvg

#endif // SVGRADIALGRADIENTELEMENT_H
