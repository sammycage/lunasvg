/*
 * Copyright (c) 2020-2024 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef LUNASVG_H
#define LUNASVG_H

#include <cstdint>
#include <memory>
#include <string>

#if !defined(LUNASVG_BUILD_STATIC) && (defined(_WIN32) || defined(__CYGWIN__))
#define LUNASVG_EXPORT __declspec(dllexport)
#define LUNASVG_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define LUNASVG_EXPORT __attribute__((__visibility__("default")))
#define LUNASVG_IMPORT
#else
#define LUNASVG_EXPORT
#define LUNASVG_IMPORT
#endif

#ifdef LUNASVG_BUILD
#define LUNASVG_API LUNASVG_EXPORT
#else
#define LUNASVG_API LUNASVG_IMPORT
#endif

#define LUNASVG_VERSION_MAJOR 3
#define LUNASVG_VERSION_MINOR 0
#define LUNASVG_VERSION_MICRO 0

#define LUNASVG_VERSION_ENCODE(major, minor, micro) (((major) * 10000) + ((minor) * 100) + ((micro) * 1))
#define LUNASVG_VERSION LUNASVG_VERSION_ENCODE(LUNASVG_VERSION_MAJOR, LUNASVG_VERSION_MINOR, LUNASVG_VERSION_MICRO)

#define LUNASVG_VERSION_XSTRINGIZE(major, minor, micro) #major"."#minor"."#micro
#define LUNASVG_VERSION_STRINGIZE(major, minor, micro) LUNASVG_VERSION_XSTRINGIZE(major, minor, micro)
#define LUNASVG_VERSION_STRING LUNASVG_VERSION_STRINGIZE(LUNASVG_VERSION_MAJOR, LUNASVG_VERSION_MINOR, LUNASVG_VERSION_MICRO)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plutovg_surface plutovg_surface_t;
typedef struct plutovg_matrix plutovg_matrix_t;

/**
 * @brief lunasvg_version
 * @return
 */
LUNASVG_API int lunasvg_version(void);

/**
 * @brief lunasvg_version_string
 * @return
 */
LUNASVG_API const char* lunasvg_version_string(void);

#ifdef __cplusplus
}
#endif

namespace lunasvg {

/**
* @note Bitmap pixel format is ARGB32_Premultiplied.
*/
class LUNASVG_API Bitmap {
public:
    /**
     * @brief Bitmap
     */
    Bitmap() = default;

    /**
     * @brief Bitmap
     * @param data
     * @param width
     * @param height
     * @param stride
     */
    Bitmap(uint8_t* data, int width, int height, int stride);

    /**
     * @brief Bitmap
     * @param width
     * @param height
     */
    Bitmap(int width, int height);

    /**
     * @brief Bitmap
     * @param bitmap
     */
    Bitmap(const Bitmap& bitmap);

    /**
     * @brief Bitmap
     * @param bitmap
     */
    Bitmap(Bitmap&& bitmap);

    /**
     * @brief Bitmap
     * @param surface
     */
    Bitmap(plutovg_surface_t* surface) : m_surface(surface) {}

    /**
     * @brief ~Bitmap
     */
    ~Bitmap();

    /**
     * @brief Copy assignment operator
     * @param bitmap
     * @return
     */
    Bitmap& operator=(const Bitmap& bitmap);

    /**
     * @brief Move assignment operator
     * @param bitmap
     * @return
     */
    Bitmap& operator=(Bitmap&& bitmap);

    /**
     * @brief swap
     * @param bitmap
     */
    void swap(Bitmap& bitmap);

    /**
     * @brief data
     * @return
     */
    uint8_t* data() const;

    /**
     * @brief width
     * @return
     */
    int width() const;

    /**
     * @brief height
     * @return
     */
    int height() const;

    /**
     * @brief stride
     * @return
     */
    int stride() const;

    /**
     * @brief clear
     * @param value
     */
    void clear(uint32_t value);

    /**
     * @brief convertToRGBA
     */
    void convertToRGBA();

    /**
     * @brief isNull
     * @return
     */
    bool isNull() const { return m_surface == nullptr; }

    /**
     * @brief writeToPng
     * @param filename
     * @return
     */
    bool writeToPng(const std::string& filename) const;

    /**
     * @brief surface
     * @return
     */
    plutovg_surface_t* surface() const { return m_surface; }

private:
    plutovg_surface_t* release();
    plutovg_surface_t* m_surface{nullptr};
};

class Rect;
class Matrix;

class LUNASVG_API Box {
public:
    /**
     * @brief Box
     */
    Box() = default;

    /**
     * @brief Box
     * @param x
     * @param y
     * @param w
     * @param h
     */
    Box(float x, float y, float w, float h);

    /**
     * @brief Box
     * @param rect
     */
    Box(const Rect& rect);

    /**
     * @brief transform
     * @param matrix
     * @return
     */
    Box& transform(const Matrix& matrix);

    /**
     * @brief transformed
     * @param matrix
     * @return
     */
    Box transformed(const Matrix& matrix) const;

    float x{0};
    float y{0};
    float w{0};
    float h{0};
};

class Transform;

class LUNASVG_API Matrix {
public:
    /**
     * @brief Matrix
     */
    Matrix() = default;

    /**
     * @brief Matrix
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     */
    Matrix(float a, float b, float c, float d, float e, float f);

    /**
     * @brief Matrix
     * @param matrix
     */
    Matrix(const plutovg_matrix_t& matrix);

    /**
     * @brief Matrix
     * @param transform
     */
    Matrix(const Transform& transform);

    /**
     * @brief multiply
     * @param matrix
     * @return
     */
    Matrix& multiply(const Matrix& matrix);

    /**
     * @brief rotate
     * @param angle
     * @return
     */
    Matrix& rotate(float angle);

    /**
     * @brief rotate
     * @param angle
     * @param cx
     * @param cy
     * @return
     */
    Matrix& rotate(float angle, float cx, float cy);

    /**
     * @brief scale
     * @param sx
     * @param sy
     * @return
     */
    Matrix& scale(float sx, float sy);

    /**
     * @brief shear
     * @param shx
     * @param shy
     * @return
     */
    Matrix& shear(float shx, float shy);

    /**
     * @brief translate
     * @param tx
     * @param ty
     * @return
     */
    Matrix& translate(float tx, float ty);

    /**
     * @brief postMultiply
     * @param matrix
     * @return
     */
    Matrix& postMultiply(const Matrix& matrix);

    /**
     * @brief postRotate
     * @param angle
     * @return
     */
    Matrix& postRotate(float angle);

    /**
     * @brief postRotate
     * @param angle
     * @param cx
     * @param cy
     * @return
     */
    Matrix& postRotate(float angle, float cx, float cy);

    /**
     * @brief postScale
     * @param sx
     * @param sy
     * @return
     */
    Matrix& postScale(float sx, float sy);

    /**
     * @brief postShear
     * @param shx
     * @param shy
     * @return
     */
    Matrix& postShear(float shx, float shy);

    /**
     * @brief postTranslate
     * @param tx
     * @param ty
     * @return
     */
    Matrix& postTranslate(float tx, float ty);

    /**
     * @brief invert
     * @return
     */
    Matrix& invert();

    /**
     * @brief inverse
     * @return
     */
    Matrix inverse() const;

    /**
     * @brief reset
     */
    void reset();

    /**
     * @brief rotated
     * @param angle
     * @return
     */
    static Matrix rotated(float angle);

    /**
     * @brief rotated
     * @param angle
     * @param cx
     * @param cy
     * @return
     */
    static Matrix rotated(float angle, float cx, float cy);

    /**
     * @brief scaled
     * @param sx
     * @param sy
     * @return
     */
    static Matrix scaled(float sx, float sy);

    /**
     * @brief sheared
     * @param shx
     * @param shy
     * @return
     */
    static Matrix sheared(float shx, float shy);

    /**
     * @brief translated
     * @param tx
     * @param ty
     * @return
     */
    static Matrix translated(float tx, float ty);

    float a{1};
    float b{0};
    float c{0};
    float d{1};
    float e{0};
    float f{0};
};

class SVGElement;

class LUNASVG_API Element {
public:
    /**
     * @brief Element
     */
    Element() = default;

    /**
     * @brief Element
     * @param element
     */
    Element(SVGElement* element) : m_element(element) {}

    /**
     * @brief hasAttribute
     * @param name
     * @return
     */
    bool hasAttribute(const std::string& name) const;

    /**
     * @brief getAttribute
     * @param name
     * @return
     */
    const std::string& getAttribute(const std::string& name) const;

    /**
     * @brief setAttribute
     * @param name
     * @param value
     */
    void setAttribute(const std::string& name, const std::string& value);

    /**
     * @brief render
     * @param bitmap
     * @param matrix
     */
    void render(Bitmap& bitmap, const Matrix& matrix = Matrix()) const;

    /**
     * @brief renderToBitmap
     * @param width
     * @param height
     * @param backgroundColor
     * @return
     */
    Bitmap renderToBitmap(int width = -1, int height = -1, uint32_t backgroundColor = 0x00000000) const;

    /**
     * @brief getLocalMatrix
     * @return
     */
    Matrix getLocalMatrix() const;

    /**
     * @brief getGlobalMatrix
     * @return
     */
    Matrix getGlobalMatrix() const;

    /**
     * @brief getLocalBoundingBox
     * @return
     */
    Box getLocalBoundingBox() const;

    /**
     * @brief getGlobalBoundingBox
     * @return
     */
    Box getGlobalBoundingBox() const;

    /**
     * @brief getBoundingBox
     * @return
     */
    Box getBoundingBox() const;

    /**
     * @brief isNull
     * @return
     */
    bool isNull() const { return m_element == nullptr; }

    /**
     * @brief get
     * @return
     */
    SVGElement* get() { return m_element; }

private:
    SVGElement* m_element{nullptr};
};

class SVGRootElement;

class LUNASVG_API Document {
public:
    /**
     * @brief loadFromFile
     * @param filename
     * @return
     */
    static std::unique_ptr<Document> loadFromFile(const std::string& filename);

    /**
     * @brief loadFromData
     * @param string
     * @return
     */
    static std::unique_ptr<Document> loadFromData(const std::string& string);

    /**
     * @brief loadFromData
     * @param data
     * @return
     */
    static std::unique_ptr<Document> loadFromData(const char* data);

    /**
     * @brief loadFromData
     * @param data
     * @param length
     * @return
     */
    static std::unique_ptr<Document> loadFromData(const char* data, size_t length);

    /**
     * @brief addFontFace
     * @param family
     * @param italic
     * @param bold
     * @param filename
     * @return
     */
    static bool addFontFace(const std::string& family, bool italic, bool bold, const std::string& filename);

    /**
     * @brief addFontFace
     * @param family
     * @param italic
     * @param bold
     * @param data
     * @param length
     * @return
     */
    static bool addFontFace(const std::string& family, bool italic, bool bold, const void* data, size_t length);

    /**
     * @brief width
     * @return
     */
    float width() const;

    /**
     * @brief height
     * @return
     */
    float height() const;

    /**
     * @brief boundingBox
     * @return
     */
    Box boundingBox() const;

    /**
     * @brief updateLayout
     */
    void updateLayout();

    /**
     * @brief render
     * @param bitmap
     * @param matrix
     */
    void render(Bitmap& bitmap, const Matrix& matrix = Matrix()) const;

    /**
     * @brief renderToBitmap
     * @param width
     * @param height
     * @param backgroundColor
     * @return
     */
    Bitmap renderToBitmap(int width = -1, int height = -1, uint32_t backgroundColor = 0x00000000) const;

    /**
     * @brief getElementById
     * @param id
     * @return
     */
    Element getElementById(const std::string& id) const;

    /**
     * @brief documentElement
     * @return
     */
    Element documentElement() const;

    /**
     * @brief rootElement
     * @return
     */
    SVGRootElement* rootElement() const { return m_rootElement.get(); }

    Document(Document&&);
    Document& operator=(Document&&);
    ~Document();

private:
    Document();
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    bool parse(const char* data, size_t length);
    std::unique_ptr<SVGRootElement> m_rootElement;
};

} //namespace lunasvg

#endif // LUNASVG_H
