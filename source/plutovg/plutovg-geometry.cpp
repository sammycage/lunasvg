#include "plutovg-private.h"

#include <math.h>

void plutovg_rect_init(plutovg_rect_t* rect, double x, double y, double w, double h)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

void plutovg_rect_init_zero(plutovg_rect_t* rect)
{
    rect->x = 0.0;
    rect->y = 0.0;
    rect->w = 0.0;
    rect->h = 0.0;
}

void plutovg_matrix_init(plutovg_matrix_t* matrix, double m00, double m10, double m01, double m11, double m02, double m12)
{
    matrix->m00 = m00;
    matrix->m10 = m10;
    matrix->m01 = m01;
    matrix->m11 = m11;
    matrix->m02 = m02;
    matrix->m12 = m12;
}

void plutovg_matrix_init_identity(plutovg_matrix_t* matrix)
{
    matrix->m00 = 1.0;
    matrix->m10 = 0.0;
    matrix->m01 = 0.0;
    matrix->m11 = 1.0;
    matrix->m02 = 0.0;
    matrix->m12 = 0.0;
}

void plutovg_matrix_init_translate(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_init(matrix, 1.0, 0.0, 0.0, 1.0, x, y);
}

void plutovg_matrix_init_scale(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_init(matrix, x, 0.0, 0.0, y, 0.0, 0.0);
}

void plutovg_matrix_init_shear(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_init(matrix, 1.0, tan(y), tan(x), 1.0, 0.0, 0.0);
}

void plutovg_matrix_init_rotate(plutovg_matrix_t* matrix, double radians, double x, double y)
{
    double c = cos(radians);
    double s = sin(radians);

    double cx = x * (1 - c) + y * s;
    double cy = y * (1 - c) - x * s;

    plutovg_matrix_init(matrix, c, s, -s, c, cx, cy);
}

void plutovg_matrix_translate(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_translate(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_scale(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_scale(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_shear(plutovg_matrix_t* matrix, double x, double y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_shear(&m, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_rotate(plutovg_matrix_t* matrix, double radians, double x, double y)
{
    plutovg_matrix_t m;
    plutovg_matrix_init_rotate(&m, radians, x, y);
    plutovg_matrix_multiply(matrix, &m, matrix);
}

void plutovg_matrix_multiply(plutovg_matrix_t* matrix, const plutovg_matrix_t* a, const plutovg_matrix_t* b)
{
    double m00 = a->m00 * b->m00 + a->m10 * b->m01;
    double m10 = a->m00 * b->m10 + a->m10 * b->m11;
    double m01 = a->m01 * b->m00 + a->m11 * b->m01;
    double m11 = a->m01 * b->m10 + a->m11 * b->m11;
    double m02 = a->m02 * b->m00 + a->m12 * b->m01 + b->m02;
    double m12 = a->m02 * b->m10 + a->m12 * b->m11 + b->m12;

    plutovg_matrix_init(matrix, m00, m10, m01, m11, m02, m12);
}

int plutovg_matrix_invert(plutovg_matrix_t* matrix)
{
    double det = (matrix->m00 * matrix->m11 - matrix->m10 * matrix->m01);
    if (det == 0.0)
        return 0;

    double inv_det = 1.0 / det;
    double m00 = matrix->m00 * inv_det;
    double m10 = matrix->m10 * inv_det;
    double m01 = matrix->m01 * inv_det;
    double m11 = matrix->m11 * inv_det;
    double m02 = (matrix->m01 * matrix->m12 - matrix->m11 * matrix->m02) * inv_det;
    double m12 = (matrix->m10 * matrix->m02 - matrix->m00 * matrix->m12) * inv_det;

    plutovg_matrix_init(matrix, m11, -m10, -m01, m00, m02, m12);
    return 1;
}

void plutovg_matrix_map(const plutovg_matrix_t* matrix, double x, double y, double* _x, double* _y)
{
    *_x = x * matrix->m00 + y * matrix->m01 + matrix->m02;
    *_y = x * matrix->m10 + y * matrix->m11 + matrix->m12;
}

void plutovg_matrix_map_point(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst)
{
    plutovg_matrix_map(matrix, src->x, src->y, &dst->x, &dst->y);
}

void plutovg_matrix_map_rect(const plutovg_matrix_t* matrix, const plutovg_rect_t* src, plutovg_rect_t* dst)
{
    plutovg_point_t p[4];
    p[0].x = src->x;
    p[0].y = src->y;
    p[1].x = src->x + src->w;
    p[1].y = src->y;
    p[2].x = src->x + src->w;
    p[2].y = src->y + src->h;
    p[3].x = src->x;
    p[3].y = src->y + src->h;

    plutovg_matrix_map_point(matrix, &p[0], &p[0]);
    plutovg_matrix_map_point(matrix, &p[1], &p[1]);
    plutovg_matrix_map_point(matrix, &p[2], &p[2]);
    plutovg_matrix_map_point(matrix, &p[3], &p[3]);

    double l = p[0].x;
    double t = p[0].y;
    double r = p[0].x;
    double b = p[0].y;

    for (int i = 0; i < 4; i++)
    {
        if (p[i].x < l)
            l = p[i].x;
        if (p[i].x > r)
            r = p[i].x;
        if (p[i].y < t)
            t = p[i].y;
        if (p[i].y > b)
            b = p[i].y;
    }

    dst->x = l;
    dst->y = t;
    dst->w = r - l;
    dst->h = b - t;
}

plutovg_path_t* plutovg_path_create(void)
{
    return new plutovg_path;
}

plutovg_path_t* plutovg_path_reference(plutovg_path_t* path)
{
    ++path->ref;
    return path;
}

void plutovg_path_destroy(plutovg_path_t* path)
{
    if (path == nullptr)
        return;

    if (--path->ref == 0)
    {
        delete path;
    }
}

int plutovg_path_get_reference_count(const plutovg_path_t* path)
{
    return path->ref;
}

void plutovg_path_move_to(plutovg_path_t* path, double x, double y)
{
    path->elements.emplace_back(plutovg_path_element_move_to);
    path->points.emplace_back(plutovg_point_t { x, y });

    path->contours += 1;

    path->start.x = x;
    path->start.y = y;
}

void plutovg_path_line_to(plutovg_path_t* path, double x, double y)
{
    path->elements.emplace_back(plutovg_path_element_line_to);
    path->points.emplace_back(plutovg_point_t { x, y });
}

void plutovg_path_quad_to(plutovg_path_t* path, double x1, double y1, double x2, double y2)
{
    double x, y;
    plutovg_path_get_current_point(path, &x, &y);

    double cx = 2.0 / 3.0 * x1 + 1.0 / 3.0 * x;
    double cy = 2.0 / 3.0 * y1 + 1.0 / 3.0 * y;
    double cx1 = 2.0 / 3.0 * x1 + 1.0 / 3.0 * x2;
    double cy1 = 2.0 / 3.0 * y1 + 1.0 / 3.0 * y2;
    plutovg_path_cubic_to(path, cx, cy, cx1, cy1, x2, y2);
}

void plutovg_path_cubic_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double x3, double y3)
{
    path->elements.emplace_back(plutovg_path_element_cubic_to);
    path->points.emplace_back(plutovg_point_t { x1, y1 });
    path->points.emplace_back(plutovg_point_t { x2, y2 });
    path->points.emplace_back(plutovg_point_t { x3, y3 });
}

void plutovg_path_close(plutovg_path_t* path)
{
    if (path->elements.empty())
        return;

    if (path->elements.back() == plutovg_path_element_close)
        return;

    path->elements.emplace_back(plutovg_path_element_close);
    path->points.emplace_back(plutovg_point_t { path->start.x, path->start.y });
}

static inline void rel_to_abs(const plutovg_path_t* path, double* x, double* y)
{
    double _x, _y;
    plutovg_path_get_current_point(path, &_x, &_y);

    *x += _x;
    *y += _y;
}

void plutovg_path_rel_move_to(plutovg_path_t* path, double x, double y)
{
    rel_to_abs(path, &x, &y);
    plutovg_path_move_to(path, x, y);
}

void plutovg_path_rel_line_to(plutovg_path_t* path, double x, double y)
{
    rel_to_abs(path, &x, &y);
    plutovg_path_line_to(path, x, y);
}

void plutovg_path_rel_quad_to(plutovg_path_t* path, double x1, double y1, double x2, double y2)
{
    rel_to_abs(path, &x1, &y1);
    rel_to_abs(path, &x2, &y2);
    plutovg_path_quad_to(path, x1, y1, x2, y2);
}

void plutovg_path_rel_cubic_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double x3, double y3)
{
    rel_to_abs(path, &x1, &y1);
    rel_to_abs(path, &x2, &y2);
    rel_to_abs(path, &x3, &y3);
    plutovg_path_cubic_to(path, x1, y1, x2, y2, x3, y3);
}

void plutovg_path_add_rect(plutovg_path_t* path, double x, double y, double w, double h)
{
    plutovg_path_move_to(path, x, y);
    plutovg_path_line_to(path, x + w, y);
    plutovg_path_line_to(path, x + w, y + h);
    plutovg_path_line_to(path, x, y + h);
    plutovg_path_line_to(path, x, y);
    plutovg_path_close(path);
}

void plutovg_path_add_round_rect(plutovg_path_t* path, double x, double y, double w, double h, double rx, double ry)
{
    double right = x + w;
    double bottom = y + h;

    double cpx = rx * plutovg_kappa;
    double cpy = ry * plutovg_kappa;

    plutovg_path_move_to(path, x, y + ry);
    plutovg_path_cubic_to(path, x, y + ry - cpy, x + rx - cpx, y, x + rx, y);
    plutovg_path_line_to(path, right - rx, y);
    plutovg_path_cubic_to(path, right - rx + cpx, y, right, y + ry - cpy, right, y + ry);
    plutovg_path_line_to(path, right, bottom - ry);
    plutovg_path_cubic_to(path, right, bottom - ry + cpy, right - rx + cpx, bottom, right - rx, bottom);
    plutovg_path_line_to(path, x + rx, bottom);
    plutovg_path_cubic_to(path, x + rx - cpx, bottom, x, bottom - ry + cpy, x, bottom - ry);
    plutovg_path_line_to(path, x, y + ry);
    plutovg_path_close(path);
}

void plutovg_path_add_ellipse(plutovg_path_t* path, double cx, double cy, double rx, double ry)
{
    double left = cx - rx;
    double top = cy - ry;
    double right = cx + rx;
    double bottom = cy + ry;

    double cpx = rx * plutovg_kappa;
    double cpy = ry * plutovg_kappa;

    plutovg_path_move_to(path, cx, top);
    plutovg_path_cubic_to(path, cx + cpx, top, right, cy - cpy, right, cy);
    plutovg_path_cubic_to(path, right, cy + cpy, cx + cpx, bottom, cx, bottom);
    plutovg_path_cubic_to(path, cx - cpx, bottom, left, cy + cpy, left, cy);
    plutovg_path_cubic_to(path, left, cy - cpy, cx - cpx, top, cx, top);
    plutovg_path_close(path);
}

void plutovg_path_add_circle(plutovg_path_t* path, double cx, double cy, double r)
{
    plutovg_path_add_ellipse(path, cx, cy, r, r);
}

void plutovg_path_add_path(plutovg_path_t* path, const plutovg_path_t* source, const plutovg_matrix_t* matrix)
{
    for (auto& iter: source->elements)
    {
        path->elements.emplace_back(iter);
    }

    for (auto& iter: source->points)
    {
        path->points.emplace_back(iter);
        if (matrix)
        {
            auto& dst = path->points.back();
            plutovg_matrix_map(matrix, iter.x, iter.y, &dst.x, &dst.y);
        }
    }

    path->contours += source->contours;
    path->start = source->start;
}

void plutovg_path_transform(plutovg_path_t* path, const plutovg_matrix_t* matrix)
{
    for (int i = 0; i < path->points.size(); i++)
    {
        plutovg_matrix_map_point(matrix, &path->points[i], &path->points[i]);
    }
}

void plutovg_path_get_current_point(const plutovg_path_t* path, double* x, double* y)
{
    if (path->points.empty())
    {
        *x = 0.0;
        *y = 0.0;
        return;
    }

    *x = path->points.back().x;
    *y = path->points.back().y;
}

int plutovg_path_get_element_count(const plutovg_path_t* path)
{
    return static_cast<int>(path->elements.size());
}
const plutovg_path_element_t* plutovg_path_get_elements(const plutovg_path_t* path)
{
    return path->elements.data();
}

int plutovg_path_get_point_count(const plutovg_path_t* path)
{
    return static_cast<int>(path->points.size());
}

const plutovg_point_t* plutovg_path_get_points(const plutovg_path_t* path)
{
    return path->points.data();
}

void plutovg_path_clear(plutovg_path_t* path)
{
    path->elements.clear();
    path->points.clear();
    path->contours = 0;
    path->start.x = 0.0;
    path->start.y = 0.0;
}

int plutovg_path_empty(const plutovg_path_t* path)
{
    return path->elements.empty();
}

plutovg_path_t* plutovg_path_clone(const plutovg_path_t* path)
{
    plutovg_path_t* result = plutovg_path_create();
    result->elements = path->elements;
    result->points = path->points;
    result->contours = path->contours;
    result->start = path->start;
    return result;
}

typedef struct
{
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
    double x4;
    double y4;
} bezier_t;

static inline void split(const bezier_t* b, bezier_t* first, bezier_t* second)
{
    double c = (b->x2 + b->x3) * 0.5;
    first->x2 = (b->x1 + b->x2) * 0.5;
    second->x3 = (b->x3 + b->x4) * 0.5;
    first->x1 = b->x1;
    second->x4 = b->x4;
    first->x3 = (first->x2 + c) * 0.5;
    second->x2 = (second->x3 + c) * 0.5;
    first->x4 = second->x1 = (first->x3 + second->x2) * 0.5;

    c = (b->y2 + b->y3) * 0.5;
    first->y2 = (b->y1 + b->y2) * 0.5;
    second->y3 = (b->y3 + b->y4) * 0.5;
    first->y1 = b->y1;
    second->y4 = b->y4;
    first->y3 = (first->y2 + c) * 0.5;
    second->y2 = (second->y3 + c) * 0.5;
    first->y4 = second->y1 = (first->y3 + second->y2) * 0.5;
}

static void flatten(plutovg_path_t* path, const plutovg_point_t* p0, const plutovg_point_t* p1, const plutovg_point_t* p2,
                    const plutovg_point_t* p3)
{
    bezier_t beziers[32];
    beziers[0].x1 = p0->x;
    beziers[0].y1 = p0->y;
    beziers[0].x2 = p1->x;
    beziers[0].y2 = p1->y;
    beziers[0].x3 = p2->x;
    beziers[0].y3 = p2->y;
    beziers[0].x4 = p3->x;
    beziers[0].y4 = p3->y;

    const double threshold = 0.25;

    bezier_t* b = beziers;
    while (b >= beziers)
    {
        double y4y1 = b->y4 - b->y1;
        double x4x1 = b->x4 - b->x1;
        double l = fabs(x4x1) + fabs(y4y1);
        double d;
        if (l > 1.0)
        {
            d = fabs((x4x1) * (b->y1 - b->y2) - (y4y1) * (b->x1 - b->x2)) +
                fabs((x4x1) * (b->y1 - b->y3) - (y4y1) * (b->x1 - b->x3));
        }
        else
        {
            d = fabs(b->x1 - b->x2) + fabs(b->y1 - b->y2) + fabs(b->x1 - b->x3) + fabs(b->y1 - b->y3);
            l = 1.0;
        }

        if (d < threshold * l || b == beziers + 31)
        {
            plutovg_path_line_to(path, b->x4, b->y4);
            --b;
        }
        else
        {
            split(b, b + 1, b);
            ++b;
        }
    }
}

plutovg_path_t* plutovg_path_clone_flat(const plutovg_path_t* path)
{
    plutovg_path_t* result = plutovg_path_create();
    for (auto& iter: path->points)
    {
        result->points.emplace_back(iter);
    }

    const plutovg_point_t* points = path->points.data();
    for (auto& iter: path->elements)
    {
        switch (iter)
        {
            case plutovg_path_element_move_to:
                plutovg_path_move_to(result, points[0].x, points[0].y);
                points += 1;
                break;
            case plutovg_path_element_line_to:
                plutovg_path_line_to(result, points[0].x, points[0].y);
                points += 1;
                break;
            case plutovg_path_element_close:
                plutovg_path_line_to(result, points[0].x, points[0].y);
                points += 1;
                break;
            case plutovg_path_element_cubic_to:
                {
                    plutovg_point_t p0;
                    plutovg_path_get_current_point(result, &p0.x, &p0.y);
                    flatten(result, &p0, points, points + 1, points + 2);
                    points += 3;
                    break;
                }
        }
    }

    return result;
}
