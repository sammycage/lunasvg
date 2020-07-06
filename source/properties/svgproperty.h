#ifndef SVGPROPERTY_H
#define SVGPROPERTY_H

#include "svgpropertyutils.h"
#include "svgparserutils.h"

#include <cassert>

namespace lunasvg {

enum PropertyType
{
    PropertyTypeUnknown = 0,
    PropertyTypeAngle,
    PropertyTypeColor,
    PropertyTypeCSSPropertyList,
    PropertyTypeEnumeration,
    PropertyTypeLength,
    PropertyTypeLengthList,
    PropertyTypeNumber,
    PropertyTypeNumberList,
    PropertyTypePath,
    PropertyTypePoint,
    PropertyTypePointList,
    PropertyTypePreserveAspectRatio,
    PropertyTypeRect,
    PropertyTypeString,
    PropertyTypeStringList,
    PropertyTypeTransform,
};

class SVGProperty;

template<typename T>
inline T* to(SVGProperty* property)
{
    return static_cast<T*>(property);
}

template<typename T>
inline const T* to(const SVGProperty* property)
{
    return static_cast<const T*>(property);
}

const static std::string KEmptyString;

class SVGProperty
{
public:
    virtual ~SVGProperty();
    virtual void setValueAsString(const std::string& value) = 0;
    virtual std::string valueAsString() const = 0;
    virtual SVGProperty* clone() const = 0;
    PropertyType propertyType() const { return m_propertyType; }

protected:
    SVGProperty(PropertyType propertyType) : m_propertyType(propertyType) {}

private:
    PropertyType m_propertyType;
};

class DOMSVGPropertyBase
{
public:
    virtual ~DOMSVGPropertyBase();
    virtual SVGProperty* ensurePropertyBase() = 0;
    virtual SVGProperty* propertyBase() const = 0;
    virtual void setPropertyAsString(const std::string& value) = 0;
    virtual std::string propertyAsString() const = 0;
    virtual void resetProperty() = 0;
    virtual bool isSpecified() const = 0;
    virtual void clone(const DOMSVGPropertyBase* property) = 0;
    DOMPropertyID propertyId() const { return m_propertyId; }
    PropertyType propertyType() const { return m_propertyType; }

protected:
    DOMSVGPropertyBase(DOMPropertyID propertyId, PropertyType propertyType)
        : m_propertyId(propertyId),
          m_propertyType(propertyType)
    {}

private:
    DOMPropertyID m_propertyId;
    PropertyType m_propertyType;
};

template<typename T>
class DOMSVGProperty : public DOMSVGPropertyBase
{
public:
    DOMSVGProperty(DOMPropertyID propertyId)
        : DOMSVGPropertyBase(propertyId, T::classType()),
          m_property(nullptr)
    {}

    SVGProperty* ensurePropertyBase() { return ensureProperty(); }
    SVGProperty* propertyBase() const { return property(); }
    T* ensureProperty();
    T* property() const { return m_property; }
    bool isSpecified() const { return m_property; }
    void resetProperty();
    void clone(const DOMSVGPropertyBase* property);
    virtual std::string propertyAsString() const;
    virtual void setPropertyAsString(const std::string& value);
    virtual ~DOMSVGProperty();

private:
    T* m_property;
};

template<typename T>
T* DOMSVGProperty<T>::ensureProperty()
{
    if(!m_property)
        m_property = new T();
    return m_property;
}

template<typename T>
void DOMSVGProperty<T>::resetProperty()
{
    delete m_property;
    m_property = nullptr;
}

template<typename T>
void DOMSVGProperty<T>::clone(const DOMSVGPropertyBase* property)
{
    assert(propertyId() == property->propertyId());
    delete m_property;
    m_property = property->isSpecified() ? to<T>(property->propertyBase()->clone()) : nullptr;
}

template<typename T>
std::string DOMSVGProperty<T>::propertyAsString() const
{
    return m_property ? m_property->valueAsString() : KEmptyString;
}

template<typename T>
void DOMSVGProperty<T>::setPropertyAsString(const std::string& value)
{
    ensureProperty()->setValueAsString(value);
}

template<typename T>
DOMSVGProperty<T>::~DOMSVGProperty()
{
    delete m_property;
}

} // namespace lunasvg

#endif // SVGPROPERTY_H
