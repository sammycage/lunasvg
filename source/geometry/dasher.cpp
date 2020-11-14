#include "dasher.h"
#include "point.h"
#include "line.h"
#include "bezier.h"

namespace lunasvg {

static constexpr double tolerance = 0.1;

Dasher::Dasher(const DashArray& dashArray, double dashOffset)
{
    assert(dashArray.size() % 2 == 0);
    m_dash_array.resize(dashArray.size() / 2);
    for (size_t i = 0; i < m_dash_array.size(); ++i)
    {
        m_dash_array[i] = { dashArray[i*2], dashArray[(i*2+1) % dashArray.size()] };
    }

    m_dash_offset = dashOffset;
    m_index = 0;
    m_cur_length = 0;
    m_discard = false;
    //if the dash array contains ZERO length
    // segments or ZERO lengths gaps we could
    // optimize those usecase.
    for (size_t i = 0; i < m_dash_array.size(); i++) {
        if (!almostEqual(m_dash_array[i].length, 0.0))
            m_no_length = false;
        if (!almostEqual(m_dash_array[i].gap, 0.0))
            n_no_gap = false;
    }
}

void Dasher::moveTo(const Point& p)
{
    m_discard = false;
    m_start_new_segment = true;
    m_cur_point = p;
    m_index = 0;

    if (!almostEqual(m_dash_offset, 0.0)) {
        double totalLength = 0.0;
        for (size_t i = 0; i < m_dash_array.size(); i++) {
            totalLength = m_dash_array[i].length + m_dash_array[i].gap;
        }
        double normalizeLen = std::fmod(m_dash_offset, totalLength);
        if (normalizeLen < 0.0f) {
            normalizeLen = totalLength + normalizeLen;
        }
        // now the length is less than total length and +ve
        // findout the current dash index , dashlength and gap.
        for (size_t i = 0; i < m_dash_array.size(); i++) {
            if (normalizeLen < m_dash_array[i].length) {
                m_index = i;
                m_cur_length = m_dash_array[i].length - normalizeLen;
                m_discard = false;
                break;
            }
            normalizeLen -= m_dash_array[i].length;
            if (normalizeLen < m_dash_array[i].gap) {
                m_index = i;
                m_cur_length = m_dash_array[i].gap - normalizeLen;
                m_discard = true;
                break;
            }
            normalizeLen -= m_dash_array[i].gap;
        }
    }
    else {
        m_cur_length = m_dash_array[m_index].length;
    }
    if (almostZero(m_cur_length)) updateActiveSegment();
}

void Dasher::addLine(const Point& p)
{
    if (m_discard) return;

    if (m_start_new_segment) {
        m_result->moveTo(m_cur_point);
        m_start_new_segment = false;
    }
    m_result->lineTo(p);
}

void Dasher::updateActiveSegment()
{
    m_start_new_segment = true;

    if (m_discard) {
        m_discard = false;
        m_index = (m_index + 1) % m_dash_array.size();
        m_cur_length = m_dash_array[m_index].length;
    }
    else {
        m_discard = true;
        m_cur_length = m_dash_array[m_index].gap;
    }
    if (almostZero(m_cur_length)) updateActiveSegment();
}

void Dasher::lineTo(const Point& p)
{
    Line left, right;
    Line line(m_cur_point, p);
    double length = line.length();

    if (length <= m_cur_length) {
        m_cur_length -= length;
        addLine(p);
    }
    else {
        while (length > m_cur_length) {
            length -= m_cur_length;
            line.splitAtLength(m_cur_length, left, right);

            addLine(left.p2);
            updateActiveSegment();

            line = right;
            m_cur_point = line.p1;
        }
        // handle remainder
        if (length > tolerance) {
            m_cur_length -= length;
            addLine(line.p2);
        }
    }

    if (m_cur_length < tolerance) updateActiveSegment();

    m_cur_point = p;
}

void Dasher::addCubic(const Point& cp1, const Point& cp2, const Point& e)
{
    if (m_discard) return;

    if (m_start_new_segment) {
        m_result->moveTo(m_cur_point);
        m_start_new_segment = false;
    }
    m_result->cubicTo(cp1, cp2, e);
}

void Dasher::cubicTo(const Point& cp1, const Point& cp2, const Point& e)
{
    Bezier left, right;
    Bezier b = Bezier::fromPoints(m_cur_point, cp1, cp2, e);
    double   bezLen = b.length();

    if (bezLen <= m_cur_length) {
        m_cur_length -= bezLen;
        addCubic(cp1, cp2, e);
    }
    else {
        while (bezLen > m_cur_length) {
            bezLen -= m_cur_length;
            b.splitAtLength(m_cur_length, &left, &right);

            addCubic(left.p2, left.p3, left.p4);
            updateActiveSegment();

            b = right;
            m_cur_point = b.p1;
        }
        // handle remainder
        if (bezLen > tolerance) {
            m_cur_length -= bezLen;
            addCubic(b.p2, b.p3, b.p4);
        }
    }

    if (m_cur_length < tolerance) updateActiveSegment();

    m_cur_point = e;
}

void Dasher::dashHelper(const Path& path, Path& result)
{
    m_result = &result;
    m_result->grow_reserve(path.coordinates().size(), path.segments().size());
    m_index = 0;
    const std::vector<SegType>& elms = path.segments();
    const std::vector<Point>& pts = path.coordinates();
    const Point* ptPtr = pts.data();

    for (auto& i : elms) {
        switch (i) {
        case SegTypeMoveTo: {
            moveTo(*ptPtr++);
            break;
        }
        case SegTypeLineTo: {
            lineTo(*ptPtr++);
            break;
        }
        case SegTypeQuadTo:
        {
            const Point& p = m_cur_point;
            const Point& p1 = *ptPtr;
            const Point& p2 = *(ptPtr + 1);

            Point cp1,cp2;
            cp1.x = 2.0 / 3.0 * p1.x + 1.0 / 3.0 * p.x;
            cp1.y = 2.0 / 3.0 * p1.y + 1.0 / 3.0 * p.y;
            cp2.x = 2.0 / 3.0 * p1.x + 1.0 / 3.0 * p2.x;
            cp2.y = 2.0 / 3.0 * p1.y + 1.0 / 3.0 * p2.y;
            cubicTo(cp1, cp2, p2);

            ptPtr += 2;
        }
        break;
        case SegTypeCubicTo: {
            cubicTo(*ptPtr, *(ptPtr + 1), *(ptPtr + 2));
            ptPtr += 3;
            break;
        }
        case SegTypeClose: {            
            lineTo(path.startPoint());
            break;
        }
        }
    }
    m_result = nullptr;
}

void Dasher::dashed(const Path& path, Path& result)
{
    if (m_no_length && n_no_gap) return result.reset();

    if (path.coordinates().empty() || m_no_length) return result.reset();


    if (n_no_gap)
    {
        result = path;
        return;
    }

    result.reset();

    dashHelper(path, result);
}

} //namespace lunasvg