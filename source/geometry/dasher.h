#ifndef DASHER_H
#define DASHER_H

#include "path.h"
#include "strokedata.h"

namespace lunasvg {

class Dasher
{
public:
    Dasher(const DashArray& dashArray, double dashOffset);
    void dashed(const Path& path, Path& result);

private:
    void moveTo(const Point& p);
    void lineTo(const Point& p);
    void cubicTo(const Point& cp1, const Point& cp2, const Point& e);
    void close();
    void addLine(const Point& p);
    void addCubic(const Point& cp1, const Point& cp2, const Point& e);
    void updateActiveSegment();

    void dashHelper(const Path& path, Path& result);

private:

    struct Dash {
        double length;
        double gap;
    };
    std::vector<Dash> m_dash_array;

    Point m_cur_point;
    size_t m_index{ 0 }; /* index to the dash Array */
    double m_cur_length;
    double m_dash_offset{ 0 };
    Path* m_result{ nullptr };
    bool m_discard{ false };
    bool m_start_new_segment{ true };
    bool m_no_length{ true };
    bool n_no_gap{ true };

};

} // namespace lunasvg

#endif // DASHER_H