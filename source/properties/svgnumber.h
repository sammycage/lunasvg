#ifndef SVGNUMBER_H
#define SVGNUMBER_H

#include "svgproperty.h"

namespace lunasvg {

class SVGNumber : public SVGProperty
{
public:
    SVGNumber();

    void setValue(double value) { m_value = value; }
    double value() const { return  m_value; }

    std::string valueAsString() const;
    virtual void setValueAsString(const std::string& value);
    virtual SVGProperty* clone() const;
    static PropertyType classType() { return PropertyTypeNumber; }

protected:
    double m_value;
};

class SVGNumberPercentage : public SVGNumber
{
public:
    SVGNumberPercentage();

    void setValueAsString(const std::string& value);
    SVGProperty* clone() const;
};

typedef DOMSVGProperty<SVGNumber> DOMSVGNumber;
typedef DOMSVGProperty<SVGNumberPercentage> DOMSVGNumberPercentage;

} // namespace lunasvg

#endif // SVGNUMBER_H
