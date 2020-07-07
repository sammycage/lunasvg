#include "pathiterator.h"
#include "point.h"

namespace lunasvg
{

PathIterator::PathIterator(const Path& path) :
    m_pointSegs(path.getPointSegs()),
    m_pointCoords(path.getPointCoords()),
    m_segIndex(0),
    m_coordIndex(0)
{
}

SegType PathIterator::currentSegment(double* coords) const
{
    SegType segType = m_pointSegs[m_segIndex];
    switch(segType)
    {
    case SegTypeMoveTo:
        coords[0] = m_pointCoords[m_coordIndex];
        coords[1] = m_pointCoords[m_coordIndex + 1];
        m_startPoint.x = coords[0];
        m_startPoint.y = coords[1];
        break;
    case SegTypeLineTo:
        coords[0] = m_pointCoords[m_coordIndex];
        coords[1] = m_pointCoords[m_coordIndex + 1];
        break;
    case SegTypeQuadTo:
        coords[0] = m_pointCoords[m_coordIndex];
        coords[1] = m_pointCoords[m_coordIndex + 1];
        coords[2] = m_pointCoords[m_coordIndex + 2];
        coords[3] = m_pointCoords[m_coordIndex + 3];
        break;
    case SegTypeCubicTo:
        coords[0] = m_pointCoords[m_coordIndex];
        coords[1] = m_pointCoords[m_coordIndex + 1];
        coords[2] = m_pointCoords[m_coordIndex + 2];
        coords[3] = m_pointCoords[m_coordIndex + 3];
        coords[4] = m_pointCoords[m_coordIndex + 4];
        coords[5] = m_pointCoords[m_coordIndex + 5];
        break;
    case SegTypeClose:
        coords[0] = m_startPoint.x;
        coords[1] = m_startPoint.y;
        break;
    }

    return segType;
}

bool PathIterator::isDone() const
{
    return (m_segIndex >= m_pointSegs.size());
}

void PathIterator::next()
{
    if(m_segIndex < m_pointSegs.size())
    {
        switch(m_pointSegs[m_segIndex])
        {
        case SegTypeMoveTo:
            m_coordIndex += 2;
            break;
        case SegTypeLineTo:
            m_coordIndex += 2;
            break;
        case SegTypeQuadTo:
            m_coordIndex += 4;
            break;
        case SegTypeCubicTo:
            m_coordIndex += 6;
            break;
        default:
            break;
        }
        m_segIndex += 1;
    }
}

} //namespace lunasvg
