#ifndef SVGSTRING_H
#define SVGSTRING_H

#include "svgproperty.h"

namespace lunasvg {

class SVGString : public SVGProperty
{
public:
    SVGString();

    void setValue(std::string value) { m_value = value; }
    const std::string& value() const { return  m_value; }

    virtual void setValueAsString(const std::string& value);
    virtual std::string valueAsString() const;
    virtual SVGProperty* clone() const;
    static PropertyType classType() { return PropertyTypeString; }

protected:
    std::string m_value;
};

class SVGURIString : public SVGString
{
public:
    SVGURIString();

    void setValueAsString(const std::string& value);
    std::string valueAsString() const;
    SVGProperty* clone() const;
};

typedef DOMSVGProperty<SVGString> DOMSVGString;

} // namespace lunasvg

#endif // SVGSTRING_H
