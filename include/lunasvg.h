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
 * @brief Returns the version of the lunasvg library encoded in a single integer.
 *
 * Encodes the version of the lunasvg library into a single integer for easier comparison.
 * The version is typically represented by combining major, minor, and patch numbers into one integer.
 *
 * @return The lunasvg library version as a single integer.
 */
LUNASVG_API int lunasvg_version(void);

/**
 * @brief Returns the lunasvg library version as a human-readable string in "X.Y.Z" format.
 *
 * Provides the version of the lunasvg library as a human-readable string in the format "X.Y.Z",
 * where X represents the major version, Y the minor version, and Z the patch version.
 *
 * @return A pointer to a string containing the version in "X.Y.Z" format.
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
     * @brief Constructs a null bitmap.
     */
    Bitmap() = default;

    /**
     * @brief Constructs a bitmap with the specified width and height.
     * @note A null bitmap will be returned if memory cannot be allocated.
     * @param width The width of the bitmap in pixels.
     * @param height The height of the bitmap in pixels.
     */
    Bitmap(int width, int height);

    /**
     * @brief Constructs a bitmap with the provided pixel data, width, height, and stride.
     *
     * @param data A pointer to the raw pixel data in ARGB32 Premultiplied format.
     * @param width The width of the bitmap in pixels.
     * @param height The height of the bitmap in pixels.
     * @param stride The number of bytes per row of pixel data (stride).
    */
    Bitmap(uint8_t* data, int width, int height, int stride);

    /**
     * @brief Copy constructor.
     * @param bitmap The bitmap to copy.
     */
    Bitmap(const Bitmap& bitmap);

    /**
     * @brief Move constructor.
     * @param bitmap The bitmap to move.
     */
    Bitmap(Bitmap&& bitmap);

    /**
     * @internal
     */
    Bitmap(plutovg_surface_t* surface) : m_surface(surface) {}

    /**
     * @brief Cleans up any resources associated with the bitmap.
     */
    ~Bitmap();

    /**
     * @brief Copy assignment operator.
     * @param bitmap The bitmap to copy.
     * @return A reference to this bitmap.
     */
    Bitmap& operator=(const Bitmap& bitmap);

    /**
     * @brief Move assignment operator.
     * @param bitmap The bitmap to move.
     * @return A reference to this bitmap.
     */
    Bitmap& operator=(Bitmap&& bitmap);

    /**
     * @brief Swaps the content of this bitmap with another.
     * @param bitmap The bitmap to swap with.
     */
    void swap(Bitmap& bitmap);

    /**
     * @brief Gets the pointer to the raw pixel data.
     * @return A pointer to the raw pixel data.
     */
    uint8_t* data() const;

    /**
     * @brief Gets the width of the bitmap.
     * @return The width of the bitmap in pixels.
     */
    int width() const;

    /**
     * @brief Gets the height of the bitmap.
     * @return The height of the bitmap in pixels.
     */
    int height() const;

    /**
     * @brief Gets the stride of the bitmap.
     * @return The number of bytes per row of pixel data (stride).
     */
    int stride() const;

    /**
     * @brief Clears the bitmap with the specified color.
     * @param The color value in 0xRRGGBBAA format.
     */
    void clear(uint32_t value);

    /**
     * @brief Converts the bitmap pixel data from ARGB32 Premultiplied to RGBA Plain format in place.
     */
    void convertToRGBA();

    /**
     * @brief Checks if the bitmap is null.
     * @return True if the bitmap is null, false otherwise.
     */
    bool isNull() const { return m_surface == nullptr; }

    /**
     * @brief Writes the bitmap to a PNG file.
     * @param filename The name of the file to write.
     * @return True if the file was written successfully, false otherwise.
     */
    bool writeToPng(const std::string& filename) const;

    /**
     * @internal
     */
    plutovg_surface_t* surface() const { return m_surface; }

private:
    plutovg_surface_t* release();
    plutovg_surface_t* m_surface{nullptr};
};

class Rect;
class Matrix;

/**
 * @brief Represents a 2D axis-aligned bounding box.
 */
class LUNASVG_API Box {
public:
    /**
     * @brief Constructs a box with zero dimensions.
     */
    Box() = default;

    /**
     * @brief Constructs a box with the specified position and size.
     * @param x The x-coordinate of the box's origin.
     * @param y The y-coordinate of the box's origin.
     * @param w The width of the box.
     * @param h The height of the box.
     */
    Box(float x, float y, float w, float h);

    /**
     * @internal
     */
    Box(const Rect& rect);

    /**
     * @brief Transforms the box using the specified matrix.
     * @param matrix The transformation matrix.
     * @return A reference to this box, modified by the transformation.
     */
    Box& transform(const Matrix& matrix);

    /**
     * @brief Returns a new box transformed by the specified matrix.
     * @param matrix The transformation matrix.
     * @return A new box, transformed by the matrix.
     */
    Box transformed(const Matrix& matrix) const;

    float x{0}; ///< The x-coordinate of the box's origin.
    float y{0}; ///< The y-coordinate of the box's origin.
    float w{0}; ///< The width of the box.
    float h{0}; ///< The height of the box.
};

class Transform;

/**
 * @brief Represents a 2D transformation matrix.
 */
class LUNASVG_API Matrix {
public:
    /**
     * @brief Initializes the matrix to the identity matrix.
     */
    Matrix() = default;

    /**
     * @brief Constructs a matrix with the specified values.
     * @param a The horizontal scaling factor.
     * @param b The vertical shearing factor.
     * @param c The horizontal shearing factor.
     * @param d The vertical scaling factor.
     * @param e The horizontal translation offset.
     * @param f The vertical translation offset.
     */
    Matrix(float a, float b, float c, float d, float e, float f);

    /**
     * @internal
     */
    Matrix(const plutovg_matrix_t& matrix);

    /**
     * @internal
     */
    Matrix(const Transform& transform);

    /**
     * @brief Multiplies this matrix with another matrix.
     * @param matrix The matrix to multiply with.
     * @return A reference to this matrix after multiplication.
     */
    Matrix& multiply(const Matrix& matrix);

    /**
     * @brief Rotates this matrix by the specified angle.
     * @param angle The rotation angle in degrees.
     * @return A reference to this matrix after rotation.
     */
    Matrix& rotate(float angle);

    /**
     * @brief Rotates this matrix by the specified angle around a point.
     * @param angle The rotation angle in degrees.
     * @param cx The x-coordinate of the center of rotation.
     * @param cy The y-coordinate of the center of rotation.
     * @return A reference to this matrix after rotation.
     */
    Matrix& rotate(float angle, float cx, float cy);

    /**
     * @brief Scales this matrix by the specified factors.
     * @param sx The horizontal scaling factor.
     * @param sy The vertical scaling factor.
     * @return A reference to this matrix after scaling.
     */
    Matrix& scale(float sx, float sy);

    /**
     * @brief Shears this matrix by the specified factors.
     * @param shx The horizontal shearing factor.
     * @param shy The vertical shearing factor.
     * @return A reference to this matrix after shearing.
     */
    Matrix& shear(float shx, float shy);

    /**
     * @brief Translates this matrix by the specified offsets.
     * @param tx The horizontal translation offset.
     * @param ty The vertical translation offset.
     * @return A reference to this matrix after translation.
     */
    Matrix& translate(float tx, float ty);

    /**
     * @brief Post-multiplies this matrix with another matrix.
     * @param matrix The matrix to post-multiply with.
     * @return A reference to this matrix after post-multiplication.
     */
    Matrix& postMultiply(const Matrix& matrix);

    /**
     * @brief Post-rotates this matrix by the specified angle.
     * @param angle The rotation angle in degrees.
     * @return A reference to this matrix after post-rotation.
     */
    Matrix& postRotate(float angle);

    /**
     * @brief Post-rotates this matrix by the specified angle around a point.
     * @param angle The rotation angle in degrees.
     * @param cx The x-coordinate of the center of rotation.
     * @param cy The y-coordinate of the center of rotation.
     * @return A reference to this matrix after post-rotation.
     */
    Matrix& postRotate(float angle, float cx, float cy);

    /**
     * @brief Post-scales this matrix by the specified factors.
     * @param sx The horizontal scaling factor.
     * @param sy The vertical scaling factor.
     * @return A reference to this matrix after post-scaling.
     */
    Matrix& postScale(float sx, float sy);

    /**
     * @brief Post-shears this matrix by the specified factors.
     * @param shx The horizontal shearing factor.
     * @param shy The vertical shearing factor.
     * @return A reference to this matrix after post-shearing.
     */
    Matrix& postShear(float shx, float shy);

    /**
     * @brief Post-translates this matrix by the specified offsets.
     * @param tx The horizontal translation offset.
     * @param ty The vertical translation offset.
     * @return A reference to this matrix after post-translation.
     */
    Matrix& postTranslate(float tx, float ty);

    /**
     * @brief Inverts this matrix.
     * @return A reference to this matrix after inversion.
     */
    Matrix& invert();

    /**
     * @brief Returns the inverse of this matrix.
     * @return A new matrix that is the inverse of this matrix.
     */
    Matrix inverse() const;

    /**
     * @brief Resets this matrix to the identity matrix.
     */
    void reset();

    /**
     * @brief Creates a rotation matrix with the specified angle.
     * @param angle The rotation angle in degrees.
     * @return A new rotation matrix.
     */
    static Matrix rotated(float angle);

    /**
     * @brief Creates a rotation matrix with the specified angle around a point.
     * @param angle The rotation angle in degrees.
     * @param cx The x-coordinate of the center of rotation.
     * @param cy The y-coordinate of the center of rotation.
     * @return A new rotation matrix.
     */
    static Matrix rotated(float angle, float cx, float cy);

    /**
     * @brief Creates a scaling matrix with the specified factors.
     * @param sx The horizontal scaling factor.
     * @param sy The vertical scaling factor.
     * @return A new scaling matrix.
     */
    static Matrix scaled(float sx, float sy);

    /**
     * @brief Creates a shearing matrix with the specified factors.
     * @param shx The horizontal shearing factor.
     * @param shy The vertical shearing factor.
     * @return A new shearing matrix.
     */
    static Matrix sheared(float shx, float shy);

    /**
     * @brief Creates a translation matrix with the specified offsets.
     * @param tx The horizontal translation offset.
     * @param ty The vertical translation offset.
     * @return A new translation matrix.
     */
    static Matrix translated(float tx, float ty);

    float a{1}; ///< The horizontal scaling factor.
    float b{0}; ///< The vertical shearing factor.
    float c{0}; ///< The horizontal shearing factor.
    float d{1}; ///< The vertical scaling factor.
    float e{0}; ///< The horizontal translation offset.
    float f{0}; ///< The vertical translation offset.
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
     * @param bold
     * @param italic
     * @param filename
     * @return
     */
    static bool addFontFace(const std::string& family, bool bold, bool italic, const std::string& filename);

    /**
     * @brief addFontFace
     * @param family
     * @param bold
     * @param italic
     * @param data
     * @param length
     * @return
     */
    static bool addFontFace(const std::string& family, bool bold, bool italic, const void* data, size_t length);

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
