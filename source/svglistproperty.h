#ifndef SVGLISTPROPERTY_H
#define SVGLISTPROPERTY_H

#include "svgproperty.h"
#include <vector>

namespace lunasvg {

template <typename T>
class SVGListProperty : public SVGProperty
{
public:
    virtual ~SVGListProperty();
    virtual std::string valueAsString() const;
    const T* at(std::size_t index) const { return m_values.at(index); }
    T* at(std::size_t index) { return m_values.at(index); }
    std::size_t length() const { return  m_values.size(); }
    bool isEmpty() const { return m_values.empty(); }
    void appendItem(T* item) { m_values.push_back(item); }
    void insertItem(T* item, std::size_t index) { m_values.insert(m_values.begin() + index, item); }
    void replaceItem(T* item, std::size_t index)
    {
        delete m_values[index];
        m_values[index] = item;
    }

    void removeItem(std::size_t index)
    {
        delete m_values[index];
        m_values.erase(m_values.begin() + index);
    }

    void clear()
    {
        for(std::size_t i = 0;i < m_values.size();i++)
            delete m_values[i];
        m_values.clear();
    }

protected:
    SVGListProperty(PropertyType propertyType) : SVGProperty(propertyType) {}
    void baseClone(SVGListProperty<T>* property) const
    {
        for(std::size_t i = 0;i < length();i++)
        {
            T* item = to<T>(at(i)->clone());
            property->appendItem(item);
        }
    }

private:
    std::vector<T*> m_values;
};

template<typename T>
SVGListProperty<T>::~SVGListProperty()
{
    for(std::size_t i = 0;i < m_values.size();i++)
        delete m_values[i];
}

template<typename T>
std::string SVGListProperty<T>::valueAsString() const
{
    std::string out;
    for(std::size_t i = 0;i < length();i++)
    {
        if(i != 0)
            out += ' ';
        out += at(i)->valueAsString();
    }

    return out;
}

} // namespace lunasvg

#endif // SVGLISTPROPERTY_H
