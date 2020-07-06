#ifndef SVGDOCUMENT_H
#define SVGDOCUMENT_H

#include "svgelement.h"

namespace lunasvg {

class SVGDocumentImpl;

class SVGDocument
{
public:
    SVGDocument();
    ~SVGDocument();

    /**
     * @brief Finds an element that matches the given id in this document.
     *
     * @param id The id to match on. An empty string matches all ids.
     * @param index The index of the element collection with the id.
     * @return A pointer to the element on success, otherwise NULL.
     */
    SVGElement* getElementById(const std::string& id, int index = 0) const;

    /**
     * @brief Finds an element that matches the given tag name in this document.
     *
     * @param tagName The tagName to match on. An empty string matches all tags.
     * @param index The index of the element collection with the tag name.
     * @return A pointer to the element on success, otherwise NULL.
     */
    SVGElement* getElementByTag(const std::string& tagName, int index = 0) const;

    /**
     * @brief Inserts a copy of an element to this document.
     *
     * @param element A pointer to the element.
     * @param position A position relative to the root element.
     * @return A pointer to the copied element on success, otherwise NULL.
     */
    SVGElement* insertElement(const SVGElement* element, InsertPosition position);

    /**
     * @brief Appends a copy of an element to this document.
     *
     * @param element A pointer to the element.
     * @return A pointer to the copied element on success, otherwise NULL.
     */
    SVGElement* appendElement(const SVGElement* element);

    /**
     * @brief Inserts SVG content to this document.
     *
     * @param content SVG content.
     * @param position A position relative to the root element.
     * @return A pointer to the first element in the content on success, otherwise NULL.
     */
    SVGElement* insertContent(const std::string& content, InsertPosition position);

    /**
     * @brief Appends SVG content to this document.
     *
     * @param content SVG content.
     * @return A pointer to the first element in the content on success, otherwise NULL.
     */
    SVGElement* appendContent(const std::string& content);

    /**
     * @brief Clears all the content of this document.
     */
    void clearContent();

    /**
     * @brief Loads SVG content from the given filename.
     *
     * @param filename A SVG resource filename
     * @return True on success, otherwise false.
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Loads SVG content from the given data.
     *
     * @param content A string containing a SVG content.
     * @return True on success, otherwise false.
     */
    bool loadFromData(const std::string& content);

    /**
     * @brief Returns the width of the document as specified in the SVG file.
     * @param dpi Dots per inch to use for units conversion to pixels.
     * @return The height in pixels, or -1 if the width cannot be determined.
     */
    double documentWidth(double dpi = 96.0) const;

    /**
     * @brief Returns the height of the document as specified in the SVG file.
     * @param dpi Dots per inch to use for units conversion to pixels.
     * @return The height in pixels, or -1 if the height cannot be determined.
     */
    double documentHeight(double dpi = 96.0) const;

    /**
     * @brief Renders this document to bitmap. The width and height of the bitmap will be used as the viewPort.
     *
     * @param bitmap Bitmap in which content will be drawn.
     * @param dpi Dots per inch to use for units conversion to pixels.
     * @param bgColor The background color in 0xRRGGBBAA.
     */
    void render(Bitmap bitmap, double dpi = 96.0, unsigned int bgColor = 0x00000000) const;

    /**
     * @brief This is the top level element.
     *
     * @return A pointer to the root element.
     */
    SVGElement* rootElement() const;

    /**
     * @brief toString
     * @return
     */
    std::string toString() const;

    /**
     * @brief hasAnimation
     * @return
     */
    bool hasAnimation() const;

    /**
     * @brief animationDuration
     * @return
     */
    double animationDuration() const;

    /**
     * @brief setCurrentTime
     * @param time
     * @param seekToTime
     * @return
     */
    bool setCurrentTime(double time, bool seekToTime=false);

    /**
     * @brief currentTime
     * @return
     */
    double currentTime() const;

    /**
     * @brief impl
     * @return
     */
    SVGDocumentImpl* impl() const;

private:
    SVGDocumentImpl* m_impl;
};

inline SVGDocumentImpl* SVGDocument::impl() const
{
    return m_impl;
}

} //namespace lunasvg

#endif // SVGDOCUMENT_H
