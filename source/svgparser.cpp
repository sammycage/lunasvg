#include "svgparser.h"
#include "svgdocumentimpl.h"
#include "svgelementhead.h"
#include "svgelementtail.h"
#include "svgelementtext.h"
#include "svgsvgelement.h"

#define KTagUnknown 0
#define KTagOpen 1
#define KTagClose 2
#define KTagEmpty 3
#define KTagComment 4
#define KTagCData 5
#define KTagPCData 6
#define KTagInstruction 7
#define KTagDocType 8

namespace lunasvg {

SVGParser::SVGParser(SVGDocument* document)
    : m_document(document)
{
}

bool SVGParser::appendHead(DOMElementID elementId, const AttributeList& attributes)
{
    SVGElementHead* parent;
    if(m_blocks.empty())
        parent = m_current->parent;
    else
        parent = m_blocks.top();
    if(!Utils::isElementPermitted(parent->elementId(), elementId))
         return false;
    SVGElementHead* newElement = Utils::createElement(elementId, m_document);
    if(newElement==nullptr)
        return false;

    appendElement(newElement);
    m_blocks.push(newElement);
    for(unsigned int i = 0;i < attributes.size();i++)
        newElement->setAttribute(attributes[i].first, attributes[i].second);

    return true;
}

bool SVGParser::appendTail(DOMElementID elementId)
{
    if(m_blocks.empty() || elementId!=m_blocks.top()->elementId())
        return false;

    appendElement(new SVGElementTail(m_document));
    SVGElementHead* head = m_blocks.top();
    head->tail = to<SVGElementTail>(m_current);
    m_blocks.pop();

    return true;
}

bool SVGParser::appendText(const std::string&)
{
    return false;
}

void SVGParser::appendElement(SVGElementImpl* newElement)
{
    if(m_blocks.empty())
        newElement->parent = m_current->parent;
    else
        newElement->parent = m_blocks.top();
    m_current->next = newElement;
    newElement->prev = m_current;
    m_current = newElement;
}

SVGElementImpl* SVGParser::parse(const std::string& source, SVGElementHead* parent)
{
    SVGElementImpl* start = new SVGElementText(m_document);
    start->parent = parent;
    m_current = start;

    std::stack<std::string> unsupported;
    const char* ptr = source.c_str();
    int tagType;
    std::string tagName, content;
    AttributeList attributes;
    while(enumTag(ptr, tagType, tagName, content, attributes))
    {
        DOMElementID elementId = Utils::domElementId(tagName);
        if(tagType==KTagOpen || tagType==KTagEmpty)
        {
            if(!parent && m_blocks.empty())
            {
                if(elementId != DOMElementIdSvg)
                    break;
                SVGElementHead* rootElement = to<SVGElementHead>(m_document->rootElement());
                m_blocks.push(rootElement);
                for(unsigned int i = 0;i < attributes.size();i++)
                    rootElement->setAttribute(attributes[i].first, attributes[i].second);
            }
            else if(!unsupported.empty() || !elementId || !appendHead(elementId, attributes))
            {
                unsupported.push(tagName);
            }
        }
        if(tagType==KTagClose || tagType==KTagEmpty)
        {
            if(unsupported.empty())
            {
                if(m_blocks.empty())
                    break;
                if(!parent && m_blocks.top() == m_document->rootElement())
                {
                    if(elementId != DOMElementIdSvg)
                        break;
                    m_blocks.pop();
                    break;
                }
                if(!appendTail(elementId))
                    break;
            }
            else
            {
                if(tagName!=unsupported.top())
                    break;
                unsupported.pop();
            }
        }
    }

    m_current->next = start;
    start->prev = m_current;

    if(!m_blocks.empty() || !unsupported.empty())
    {
        m_document->impl()->freeElement(start, start->prev);
        std::stack<SVGElementHead*>().swap(m_blocks);
        return nullptr;
    }

    SVGElementImpl* head = start->next;
    SVGElementImpl* tail = start->prev;
    delete start;
    if(head==start)
        return nullptr;

    head->prev = tail;
    tail->next = head;

    return head;
}

#define IS_STARTNAMECHAR(c) (IS_ALPHA(c) ||  c == '_' || c == ':')
#define IS_NAMECHAR(c) (IS_STARTNAMECHAR(c) || IS_NUM(c) || c == '-' || c == '.')

bool SVGParser::enumTag(const char*& ptr, int& tagType, std::string& tagName, std::string& content, AttributeList& attributes)
{
    tagType = KTagUnknown;
    tagName.clear();
    content.clear();
    attributes.clear();
    Utils::skipWs(ptr);
    const char* start = ptr;
    while(*ptr && *ptr!='<')
        ++ptr;
    if(ptr!=start)
    {
        tagType = KTagPCData;
        content.assign(start, ptr);
        return true;
    }

    if(*ptr!='<')
        return false;
    ++ptr;

    if(*ptr=='/')
    {
        tagType = KTagClose;
        ++ptr;
    }
    else if(*ptr=='?')
    {
        tagType = KTagInstruction;
        ++ptr;
    }
    else if(*ptr=='!')
    {
        ++ptr;
        if(Utils::skipDesc(ptr, "--", 2))
        {
            const char* sub = strstr(ptr, "-->");
            if(!sub)
                return false;

            tagType = KTagComment;
            content.assign(ptr, sub);
            ptr += content.length() + 3;

            return true;
        }

        if(Utils::skipDesc(ptr, "[CDATA[", 7))
        {
            const char* sub = strstr(ptr, "]]>");
            if(!sub)
                return false;

            tagType = KTagCData;
            content.assign(ptr, sub);
            ptr += content.length() + 3;

            return true;
        }

        if(Utils::skipDesc(ptr, "DOCTYPE", 7))
        {
            start = ptr;
            while(*ptr && *ptr!='>')
            {
                if(*ptr=='[')
                {
                    ++ptr;
                    int depth = 1;
                    while(*ptr && depth > 0)
                    {
                        if(*ptr=='[') ++depth;
                        else if(*ptr==']') --depth;
                        ++ptr;
                    }
                }
                else
                {
                    ++ptr;
                }
            }

            if(*ptr!='>')
                return false;

            tagType = KTagDocType;
            content.assign(start, ptr);
            ptr += 1;

            return true;
        }

        return false;
    }

    if(!*ptr || !IS_STARTNAMECHAR(*ptr))
        return false;

    start = ptr;
    ++ptr;
    while(*ptr && IS_NAMECHAR(*ptr))
        ++ptr;
    tagName.assign(start, ptr);

    Utils::skipWs(ptr);
    if(tagType==KTagClose)
    {
        if(*ptr!='>')
            return false;
        ++ptr;
        return true;
    }

    while(*ptr && IS_STARTNAMECHAR(*ptr))
    {
        start = ptr;
        ++ptr;
        while(*ptr && IS_NAMECHAR(*ptr))
            ++ptr;
        Attribute attribute;
        attribute.first.assign(start, ptr);

        Utils::skipWs(ptr);
        if(*ptr!='=')
            return false;
        ++ptr;

        Utils::skipWs(ptr);
        if(*ptr!='"' && *ptr!='\'')
            return false;

        const char quote = *ptr;
        ++ptr;
        Utils::skipWs(ptr);
        start = ptr;
        while(*ptr && *ptr!=quote)
            ++ptr;
        if(*ptr!=quote)
            return false;
        attribute.second.assign(start, Utils::rtrim(start, ptr));
        attributes.push_back(attribute);
        ++ptr;
        Utils::skipWs(ptr);
    }

    if(*ptr=='>')
    {
        if(tagType!=KTagUnknown)
            return false;
        tagType = KTagOpen;
        ++ptr;
        return true;
    }

    if(*ptr=='/')
    {
        if(tagType!=KTagUnknown)
            return false;
        ++ptr;
        if(*ptr!='>')
            return false;
        tagType = KTagEmpty;
        ++ptr;
        return true;
    }

    if(*ptr=='?')
    {
        if(tagType!=KTagInstruction)
            return false;
        ++ptr;
        if(*ptr!='>')
            return false;
        ++ptr;
        return true;
    }

    return false;
}

} // namespace lunasvg
