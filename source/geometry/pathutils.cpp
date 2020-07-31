#include "pathutils.h"
#include "path.h"

#include <cmath>

#define K_PI 3.14159265358979323846

namespace lunasvg
{

namespace Utils
{

static void path_arc_segment(Path& path, double xc, double yc, double th0, double th1, double rx, double ry, double xAxisRotation)
{
    double sinTh, cosTh;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double thHalf;

    sinTh = sin(xAxisRotation * double(K_PI / 180.0));
    cosTh = cos(xAxisRotation * double(K_PI / 180.0));

    a00 =  cosTh * rx;
    a01 = -sinTh * ry;
    a10 =  sinTh * rx;
    a11 =  cosTh * ry;

    thHalf = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin(thHalf * 0.5) * sin(thHalf * 0.5) / sin(thHalf);
    x1 = xc + cos(th0) - t * sin(th0);
    y1 = yc + sin(th0) + t * cos(th0);
    x3 = xc + cos(th1);
    y3 = yc + sin(th1);
    x2 = x3 + t * sin(th1);
    y2 = y3 - t * cos(th1);

    path.cubicTo(a00 * x1 + a01 * y1,
                 a10 * x1 + a11 * y1,
                 a00 * x2 + a01 * y2,
                 a10 * x2 + a11 * y2,
                 a00 * x3 + a01 * y3,
                 a10 * x3 + a11 * y3);
}

void pathArcTo(Path& path, double rx, double ry, double xAxisRotation, bool largeArcFlag, bool sweepFlag, double x, double y, bool rel)
{
    Point cp = path.currentPoint();
    if(rel)
    {
        x += cp.x;
        y += cp.y;
    }

    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int i, n_segs;
    double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

    rx = fabs(rx);
    ry = fabs(ry);

    sin_th = sin(xAxisRotation * double(K_PI / 180.0));
    cos_th = cos(xAxisRotation * double(K_PI / 180.0));

    dx = (cp.x - x) / 2.0;
    dy = (cp.y - y) / 2.0;
    dx1 =  cos_th * dx + sin_th * dy;
    dy1 = -sin_th * dx + cos_th * dy;
    Pr1 = rx * rx;
    Pr2 = ry * ry;
    Px = dx1 * dx1;
    Py = dy1 * dy1;
    check = Px / Pr1 + Py / Pr2;
    if(check > 1)
    {
        rx = rx * sqrt(check);
        ry = ry * sqrt(check);
    }

    a00 =  cos_th / rx;
    a01 =  sin_th / rx;
    a10 = -sin_th / ry;
    a11 =  cos_th / ry;
    x0 = a00 * cp.x + a01 * cp.y;
    y0 = a10 * cp.x + a11 * cp.y;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if(sfactor_sq < 0) sfactor_sq = 0;
    sfactor = sqrt(sfactor_sq);
    if(sweepFlag == largeArcFlag) sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);

    th0 = atan2(y0 - yc, x0 - xc);
    th1 = atan2(y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if(th_arc < 0 && sweepFlag)
        th_arc += double(2 * K_PI);
    else if(th_arc > 0 && !sweepFlag)
        th_arc -= double(2 * K_PI);

    n_segs = int(ceil(fabs(th_arc / double(K_PI * 0.5 + 0.001))));
    for(i = 0; i < n_segs; i++)
        path_arc_segment(path, xc, yc, th0 + i * th_arc / n_segs, th0 + (i + 1) * th_arc / n_segs, rx, ry, xAxisRotation);
}

} // namespace Utils

} // namespace lunasvg
