#ifndef SVGELEMENT_H
#define SVGELEMENT_H

#include <string>

namespace lunasvg {

class Bitmap
{
public:
    /**
     * @brief Bitmap object constructor.
     *
     * @param data bitmap data.
     * @param width bitmap width.
     * @param height bitmap height.
     * @param stride bitmap stride.
     * @note Default bitmap format is ARGB32_Premultipied.
     */
    Bitmap(unsigned char* data, unsigned int width, unsigned int height, unsigned int stride);

    /**
     * @brief Returns data attached to the bitmap.
     *
     * @return data attached to the bitmap.
     */
    unsigned char* data() const { return m_data; }

    /**
     * @brief Returns width of the bitmap.
     *
     * @return bitmap width.
     */
    unsigned int width() const { return m_width; }

    /**
     * @brief Returns height of the bitmap.
     *
     * @return bitmap height.
     */
    unsigned int height() const { return m_height; }

    /**
     * @brief Returns stride of the bitmap.
     *
     * @return bitmap stride.
     */
    unsigned int stride() const { return m_stride; }

private:
    unsigned char* m_data;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_stride;
};

enum InsertPosition
{
    BeforeBegin,
    AfterBegin,
    BeforeEnd,
    AfterEnd
};

class SVGDocument;

class SVGElement
{
public:
    /**
     * @brief Adds a new attribute. If an attribute with that name is already
     * present in this element, its value is changed to that of the value parameter.
     *
     * @param name The name of the attribute to create or change.
     * @param value The value to set in string form.
     */
    virtual void setAttribute(const std::string& name, const std::string& value) = 0;

    /**
     * @brief Retrieves an attribute value by name.
     *
     * @param name The name of the attribute to retrieve.
     * @return The attribute value as a string, or the empty string
     * if that attribute does not have a specified value.
     */
    virtual std::string getAttribute(const std::string& name) const = 0;

    /**
     * @brief Returns true when an attribute with a given name is specified on this
     * element otherwise false.
     *
     * @param name The name of the attribute to look for.
     * @return True if an attribute with the given name is specified on this
     * element, otherwise false.
     */
    virtual bool hasAttribute(const std::string& name) const = 0;

    /**
     * @brief Removes an attribute by name. If the removed attribute has a
     * default value it is immediately replaced.
     *
     * @param name The name of the attribute to remove or replace.
     */
    virtual void removeAttribute(const std::string& name) = 0;

    /**
     * @brief The name of this element.
     *
     * @return The tag name of this element as string.
     */
    virtual const std::string& tagName() const = 0;

    /**
     * @brief The id attribute of this element.
     *
     * @return The id attribute value as a string.
     */
    virtual const std::string& id() const = 0;

    /**
     * @brief Finds an element that matches a given id in this element.
     *
     * @param id The id to match on. An empty string matches all ids.
     * @param index The index of the element collection with the id.
     * @return A pointer to the element on success, otherwise NULL.
     */
    SVGElement* getElementById(const std::string& id, int index = 0) const;

    /**
     * @brief Finds an element that matches a given tag name in this element.
     *
     * @param tagName The tagName to match on. An empty string matches all tags.
     * @param index The index of the element collection with the tag name.
     * @return A pointer to the element on success, otherwise NULL.
     */
    SVGElement* getElementByTag(const std::string& tagName, int index = 0) const;

    /**
     * @brief Inserts a copy of an element to this element.
     *
     * @param element A pointer to the element.
     * @param position A position relative to this element.
     * @return A pointer to the copied element on success, otherwise NULL.
     */
    SVGElement* insertElement(const SVGElement* element, InsertPosition position);

    /**
     * @brief Appends a copy of an element to this element.
     *
     * @param element A pointer to the element.
     * @return A pointer to the copied element on success otherwise, NULL.
     */
    SVGElement* appendElement(const SVGElement* element);

    /**
     * @brief Inserts SVG content to this element.
     *
     * @param content SVG content.
     * @param position A position relative to this element.
     * @return A pointer to the first element in the content on success, otherwise NULL.
     */
    SVGElement* insertContent(const std::string& content, InsertPosition position);

    /**
     * @brief Appends SVG content to this element.
     *
     * @param content SVG content.
     * @return A pointer to the first element in the content on success, otherwise NULL.
     */
    SVGElement* appendContent(const std::string& content);

    /**
     * @brief Clears all the content of this element.
     */
    void clearContent();

    /**
     * @brief Removes this element.
     */
    void removeElement();

    /**
     * @brief Returns the owner document.
     *
     * @return A pointer to the owner document.
     */
    SVGDocument* document() const;

    /**
     * @brief Returns the parent element.
     *
     * @return A pointer to the parent element.
     */
    SVGElement* parentElement() const;

    /**
     * @brief toString
     * @return
     */
    std::string toString() const;

protected:
    virtual ~SVGElement();

protected:
    SVGElement(SVGDocument* document);

private:
    SVGDocument* m_document;
};

inline SVGDocument* SVGElement::document() const
{
    return m_document;
}

} // namespace lunasvg

#endif // SVGELEMENT_H
