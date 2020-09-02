#ifndef SVGELEMENTHEAD_H
#define SVGELEMENTHEAD_H

#include "svgelementimpl.h"

#include <vector>

namespace lunasvg {

class SVGElementTail;

class SVGElementHead : public SVGElementImpl
{
public:
    virtual ~SVGElementHead();
    virtual void setAttribute(const std::string& name, const std::string& value);
    virtual std::string getAttribute(const std::string& name) const;
    virtual bool hasAttribute(const std::string& name) const;
    virtual void removeAttribute(const std::string& name);
    virtual void render(RenderContext&) const;
    virtual void renderTail(RenderContext&) const;
    virtual void externalise(std::string& out, std::uint32_t& indent) const;
    virtual void externaliseTail(std::string& out, std::uint32_t& indent) const;
    void setProperty(DOMPropertyID nameId, const std::string& value);
    std::string getProperty(DOMPropertyID nameId) const;
    bool hasProperty(DOMPropertyID nameId) const;
    void removeProperty(DOMPropertyID nameId);
    const std::string& tagName() const;
    const std::string& id() const { return m_id; }
    DOMElementID elementId() const { return m_elementId; }
    bool isSVGElementHead() const { return true; }

    SVGElementTail* tail;

    void addToPropertyMap(DOMSVGPropertyBase& property);
    void updateId(const std::string& newValue);

protected:
    SVGElementHead(DOMElementID elementId, SVGDocument* document);
    void baseClone(SVGElementHead& e) const;

private:
    DOMElementID m_elementId;
    std::string m_id;
    std::vector<DOMSVGPropertyBase*> m_properties;
};

} // namespace lunasvg

#endif // SVGELEMENTHEAD_H
