#ifndef SVGDOCUMENTIMPL_H
#define SVGDOCUMENTIMPL_H

#include "svgdocument.h"

#include <map>

namespace lunasvg {

class SVGElementImpl;
class SVGRootElement;
class SVGParser;

class SVGDocumentImpl
{
public:
    SVGDocumentImpl(SVGDocument* document);
    ~SVGDocumentImpl();
    bool loadFromFile(const std::string& filename);
    bool loadFromData(const std::string& content);
    double documentWidth(double dpi) const;
    double documentHeight(double dpi) const;
    Box getBBox(double dpi) const;
    Bitmap renderToBitmap(std::uint32_t width, std::uint32_t height, double dpi, std::uint32_t bgColor) const;
    void render(Bitmap& bitmap, double dpi, std::uint32_t bgColor) const;
    void updateIdCache(const std::string& oldValue, const std::string& newValue, SVGElementImpl* element);
    SVGElementImpl* resolveIRI(const std::string& href) const;
    SVGElementImpl* insertContent(const std::string& content, SVGElementImpl* target, InsertPosition position);
    SVGElementImpl* copyElement(const SVGElementImpl* element, SVGElementImpl* target, InsertPosition position);
    SVGElementImpl* moveElement(SVGElementImpl* element, SVGElementImpl* target, InsertPosition position);
    void clearContent(SVGElementImpl* element);
    void removeElement(SVGElementImpl* element);
    void insertElement(SVGElementImpl* head, SVGElementImpl* tail, SVGElementImpl* target, InsertPosition position);
    void freeElement(SVGElementImpl* head, SVGElementImpl* tail);
    std::string toString(const SVGElementImpl* element) const;
    bool hasAnimation() const;
    double animationDuration() const;
    bool setCurrentTime(double, bool);
    double currentTime() const;
    SVGRootElement* rootElement() const { return m_rootElement; }

private:
    void dispatchElementRemoveEvent(const SVGElementImpl*, const SVGElementImpl*);
    void dispatchElementInsertEvent(const SVGElementImpl*, const SVGElementImpl*);

private:
    SVGRootElement* m_rootElement;
    SVGParser* m_svgParser;
    std::multimap<std::string, SVGElementImpl*> m_idCache;
};

} // namespace lunasvg

#endif // SVGDOCUMENTIMPL_H
