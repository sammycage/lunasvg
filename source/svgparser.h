#ifndef SVGPARSER_H
#define SVGPARSER_H

#include "svgelementutils.h"
#include "svgparserutils.h"

#include <vector>
#include <stack>
#include <utility>
#include <string>

namespace lunasvg {

class SVGDocument;
class SVGElementImpl;
class SVGElementHead;

class SVGParser
{
public:
    SVGParser(SVGDocument* document);
    SVGElementImpl* parse(const std::string& source, SVGElementHead* parent);

private:
    typedef std::pair<std::string, std::string> Attribute;
    typedef std::vector<Attribute> AttributeList;
    bool enumTag(const char*& ptr, int& tagType, std::string& tagName, std::string& content, AttributeList& attributes);
    bool appendHead(DOMElementID elementId, const AttributeList& attributes);
    bool appendTail(DOMElementID elementId);
    bool appendText(const std::string&);
    void appendElement(SVGElementImpl* newElement);

private:
    SVGDocument* m_document;
    SVGElementImpl* m_current;
    std::stack<SVGElementHead*> m_blocks;
};

} // namespace lunasvg

#endif // SVGPARSER_H
