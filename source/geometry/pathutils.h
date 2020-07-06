#ifndef PATHUTILS_H
#define PATHUTILS_H

namespace lunasvg
{

class Path;

namespace Utils
{

void pathArcTo(Path& path, double rx, double ry, double xAxisRotation, bool largeArcFlag, bool sweepFlag, double x, double y, bool rel);

} // namespace Utils

} // namespace lunasvg

#endif // PATHUTILS_H
