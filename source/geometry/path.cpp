#include "path.h"
#include "pathiterator.h"
#include "affinetransform.h"

#include <limits>
#include <algorithm>

namespace lunasvg {

#define BEZIER_ARC_FACTOR 0.5522847498

Path::Path()
{
}

void Path::moveTo(double x, double y, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x += p.x;
        y += p.y;
    }

    m_pointSegs.push_back(SegTypeMoveTo);
    m_pointCoords.push_back(x);
    m_pointCoords.push_back(y);

    m_startPoint = Point(x, y);
}

void Path::lineTo(double x, double y, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x += p.x;
        y += p.y;
    }

    m_pointSegs.push_back(SegTypeLineTo);
    m_pointCoords.push_back(x);
    m_pointCoords.push_back(y);
}

void Path::quadTo(double x1, double y1, double x2, double y2, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x1 += p.x;
        y1 += p.y;
        x2 += p.x;
        y2 += p.y;
    }

    m_pointSegs.push_back(SegTypeQuadTo);
    m_pointCoords.push_back(x1);
    m_pointCoords.push_back(y1);
    m_pointCoords.push_back(x2);
    m_pointCoords.push_back(y2);
}

void Path::cubicTo(double x1, double y1, double x2, double y2, double x3, double y3, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x1 += p.x;
        y1 += p.y;
        x2 += p.x;
        y2 += p.y;
        x3 += p.x;
        y3 += p.y;
    }

    m_pointSegs.push_back(SegTypeCubicTo);
    m_pointCoords.push_back(x1);
    m_pointCoords.push_back(y1);
    m_pointCoords.push_back(x2);
    m_pointCoords.push_back(y2);
    m_pointCoords.push_back(x3);
    m_pointCoords.push_back(y3);
}

void Path::smoothQuadTo(double x2, double y2, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x2 += p.x;
        y2 += p.y;
    }

    Point p1;
    std::size_t count = m_pointSegs.size();
    if(count > 0 && m_pointSegs[count - 1] == SegTypeQuadTo)
        p1 = controlPoint();
    else
        p1 = currentPoint();

    quadTo(p1.x, p1.y, x2, y2, false);
}

void Path::smoothCubicTo(double x2, double y2, double x3, double y3, bool rel)
{
    if(rel)
    {
        Point p = currentPoint();
        x2 += p.x;
        y2 += p.y;
        x3 += p.x;
        y3 += p.y;
    }

    Point p1;
    std::size_t count = m_pointSegs.size();
    if(count > 0 && m_pointSegs[count - 1] == SegTypeCubicTo)
        p1 = controlPoint();
    else
        p1 = currentPoint();

    cubicTo(p1.x, p1.y, x2, y2, x3, y3, false);
}

void Path::horizontalTo(double x, bool rel)
{
    Point p = currentPoint();

    if(rel)
        x += p.x;

    m_pointSegs.push_back(SegTypeLineTo);
    m_pointCoords.push_back(x);
    m_pointCoords.push_back(p.y);
}

void Path::verticalTo(double y, bool rel)
{
    Point p = currentPoint();

    if(rel)
        y += p.y;

    m_pointSegs.push_back(SegTypeLineTo);
    m_pointCoords.push_back(p.x);
    m_pointCoords.push_back(y);
}

void Path::closePath()
{
    std::size_t count = m_pointSegs.size();
    if(count > 0 && m_pointSegs[count - 1] != SegTypeClose)
        m_pointSegs.push_back(SegTypeClose);
}

Point Path::currentPoint() const
{
    if(m_pointCoords.size() < 2)
        return Point();

    if(m_pointSegs[m_pointSegs.size() - 1] == SegTypeClose)
        return m_startPoint;

    std::size_t count = m_pointCoords.size();
    double x = m_pointCoords[count - 2];
    double y = m_pointCoords[count - 1];

    return Point(x, y);
}

Point Path::controlPoint() const
{
    if(m_pointCoords.size() < 4)
        return Point();

    std::size_t count = m_pointCoords.size();
    double x0 = m_pointCoords[count - 4];
    double y0 = m_pointCoords[count - 3];
    double x1 = m_pointCoords[count - 2];
    double y1 = m_pointCoords[count - 1];

    return Point(x1 + x1 - x0, y1 + y1 - y0);
}

void Path::addPath(const Path& path)
{
    PathIterator it(path);
    double c[6];
    while(!it.isDone())
    {
        switch(it.currentSegment(c))
        {
        case SegTypeMoveTo:
            moveTo(c[0], c[1]);
            break;
        case SegTypeLineTo:
            lineTo(c[0], c[1]);
            break;
        case SegTypeQuadTo:
            quadTo(c[0], c[1], c[2], c[3]);
            break;
        case SegTypeCubicTo:
            cubicTo(c[0], c[1], c[2], c[3], c[4], c[5]);
            break;
        case SegTypeClose:
            closePath();
            break;
        }
        it.next();
    }
}

void Path::addPoints(const std::vector<Point>& points)
{
    if(points.empty())
        return;

    moveTo(points[0].x, points[0].y);
    for(std::size_t i = 1;i < points.size();i++)
        lineTo(points[i].x, points[i].y);
}

void Path::addEllipse(double cx, double cy, double rx, double ry)
{
    double left = cx - rx;
    double top = cy - ry;
    double right = cx + rx;
    double bottom = cy + ry;

    double cpx = rx * BEZIER_ARC_FACTOR;
    double cpy = ry * BEZIER_ARC_FACTOR;

    moveTo(cx, top);
    cubicTo(cx+cpx, top, right, cy-cpy, right, cy);
    cubicTo(right, cy+cpy, cx+cpx, bottom, cx, bottom);
    cubicTo(cx-cpx, bottom, left, cy+cpy, left, cy);
    cubicTo(left, cy-cpy, cx-cpx, top, cx, top);
    closePath();
}

void Path::addRect(double x, double y, double w, double h, double rx, double ry)
{
    rx = std::min(rx, w * 0.5);
    ry = std::min(ry, h * 0.5);

    double right = x + w;
    double bottom = y + h;

    if(rx==0.0 && ry==0.0)
    {
        moveTo(x, y);
        lineTo(right, y);
        lineTo(right, bottom);
        lineTo(x, bottom);
        lineTo(x, y);
    }
    else
    {
        double cpx = rx * BEZIER_ARC_FACTOR;
        double cpy = ry * BEZIER_ARC_FACTOR;
        moveTo(x, y+ry);
        cubicTo(x, y+ry-cpy, x+rx-cpx, y, x+rx, y);
        lineTo(right-rx, y);
        cubicTo(right-rx+cpx, y, right, y+ry-cpy, right, y+ry);
        lineTo(right, bottom-ry);
        cubicTo(right, bottom-ry+cpy, right-rx+cpx, bottom, right-rx, bottom);
        lineTo(x+rx, bottom);
        cubicTo(x+rx-cpx, bottom, x, bottom-ry+cpy, x, bottom-ry);
        lineTo(x, y+ry);
    }

    closePath();
}

Rect Path::boundingBox() const
{
    double xMin = std::numeric_limits<double>::max();
    double yMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::min();
    double yMax = std::numeric_limits<double>::min();

    for(std::size_t i = 0;i < m_pointCoords.size();i += 2)
    {
        double x = m_pointCoords[i];
        double y = m_pointCoords[i+1];
        if(x < xMin) xMin = x;
        if(x > xMax) xMax = x;
        if(y < yMin) yMin = y;
        if(y > yMax) yMax = y;
    }

    return Rect(xMin, yMin, xMax - xMin, yMax - yMin);
}

void Path::transform(const AffineTransform& matrix)
{
    for(std::size_t i = 0;i < m_pointCoords.size();i += 2)
        matrix.map(m_pointCoords[i], m_pointCoords[i+1], m_pointCoords[i], m_pointCoords[i+1]);
}

void Path::reset()
{
    m_startPoint = Point();
    m_pointSegs.clear();
    m_pointCoords.clear();
}

} //namespace lunasvg
