#include "svgsolidcolorelement.h"
#include "svgcolor.h"
#include "svgnumber.h"

namespace lunasvg {

SVGSolidColorElement::SVGSolidColorElement(SVGDocument* document)
    : SVGPaintElement(DOMElementIdSolidColor, document)
{
}

Paint SVGSolidColorElement::getPaint(const RenderState&) const
{
    if(!style().isSpecified())
        return KRgbBlack;

    Rgb color;
    if(const CSSPropertyBase* item = style().property()->getItem(CSSPropertyIdSolid_Color))
    {
        const SVGProperty* property = !item->isInherited() ? item->property() : findInheritedProperty(CSSPropertyIdSolid_Color);
        if(property)
        {
            const SVGColor* solidColor = to<SVGColor>(property);
            color = solidColor->colorType() == ColorTypeCurrentColor ? currentColor() : solidColor->value();
        }
    }

    if(const CSSPropertyBase* item = style().property()->getItem(CSSPropertyIdSolid_Opacity))
    {
        const SVGProperty* property = !item->isInherited() ? item->property() : findInheritedProperty(CSSPropertyIdSolid_Opacity);
        if(property)
        {
            const SVGNumber* solidOpacity = to<SVGNumber>(property);
            color.a = std::uint8_t(solidOpacity->value() * 255.0);
        }
    }

    return color;
}

SVGElementImpl* SVGSolidColorElement::clone(SVGDocument* document) const
{
    SVGSolidColorElement* e = new SVGSolidColorElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
