#ifndef AFFINETRANSFORM_H
#define AFFINETRANSFORM_H

namespace lunasvg {

class Point;
class Rect;

class AffineTransform
{
public:
    AffineTransform();
    AffineTransform(const AffineTransform& transform);
    AffineTransform(double m00, double m10, double m01, double m11, double m02, double m12);

    void setMatrix(double m00, double m10, double m01, double m11, double m02, double m12);
    void multiply(const double* a, const double* b);
    AffineTransform& multiply(const AffineTransform& transform);
    AffineTransform& postmultiply(const AffineTransform& transform);
    void reset();
    void map(double x, double y, double& _x, double& _y) const;
    Point mapPoint(const Point& point) const;
    Rect mapRect(const Rect& rect) const;
    AffineTransform inverted() const;

    void rotate(double radians);
    void rotate(double radians, double cx, double cy);
    void scale(double sx, double sy);
    void shear(double angleX, double angleY);
    void translate(double cx, double cy);

    static AffineTransform makeIdentity();
    static AffineTransform makeRotate(double radians);
    static AffineTransform makeRotate(double radians, double cx, double cy);
    static AffineTransform makeScale(double sx, double sy);
    static AffineTransform makeShear(double shx, double shy);
    static AffineTransform makeTranslate(double tx, double ty);

    bool isIdentity() const;
    double determinant() const;
    const double* getMatrix() const;

private:
    double m_matrix[2*3];
};

inline const double* AffineTransform::getMatrix() const
{
   return m_matrix;
}

} //namespace lunasvg

#endif //AFFINETRANSFORM_H
