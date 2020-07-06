#include "svgelementimpl.h"
#include "svgelementhead.h"
#include "svgelementiter.h"
#include "svgdocumentimpl.h"

namespace lunasvg {

Bitmap::Bitmap(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride) :
    m_data(data), m_width(width), m_height(height), m_stride(stride)
{
}

SVGElement::SVGElement(SVGDocument* document) :
    m_document(document)
{
}

SVGElement::~SVGElement()
{
}

SVGElement* SVGElement::insertContent(const std::string& content, InsertPosition position)
{
    return document()->impl()->insertContent(content, to<SVGElementImpl>(this), position);
}

SVGElement* SVGElement::appendContent(const std::string& content)
{
    return insertContent(content, BeforeEnd);
}

void SVGElement::clearContent()
{
    document()->impl()->clearContent(to<SVGElementImpl>(this));
}

void SVGElement::removeElement()
{
    document()->impl()->removeElement(to<SVGElementImpl>(this));
}

SVGElement* SVGElement::insertElement(const SVGElement* element, InsertPosition position)
{
    return document()->impl()->copyElement(to<SVGElementImpl>(element), to<SVGElementImpl>(this), position);
}

SVGElement* SVGElement::appendElement(const SVGElement* element)
{
    return insertElement(element, BeforeEnd);
}

SVGElement* SVGElement::getElementById(const std::string& id, int index) const
{
    SVGElementIter it(this, id);
    while(it.next())
    {
        if(index-- == 0)
            return it.currentElement();
    }

    return nullptr;
}

SVGElement* SVGElement::getElementByTag(const std::string& tagName, int index) const
{
    SVGElementIter it(this, KEmptyString, tagName);
    while(it.next())
    {
        if(index-- == 0)
            return it.currentElement();
    }

    return nullptr;
}

SVGElement* SVGElement::parentElement() const
{
    return to<SVGElementImpl>(this)->parent;
}

std::string SVGElement::toString() const
{
    return document()->impl()->toString(to<SVGElementImpl>(this));
}

} // namespace lunasvg
