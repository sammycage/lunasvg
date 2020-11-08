#include "plutovg-private.h"

#include "sw_ft_raster.h"
#include "sw_ft_stroker.h"
#include "sw_ft_types.h"

#include <math.h>

typedef struct {
    SW_FT_Outline ft;
    int max_points;
    int max_contours;
} outline_t;

static outline_t* outline_create(void)
{
    outline_t* outline = malloc(sizeof(outline_t));
    memset(outline, 0, sizeof(outline_t));
    return outline;
}

static void outline_destroy(outline_t* outline)
{
    SW_FT_Outline* ft = &outline->ft;

    free(ft->points);
    free(ft->tags);
    free(ft->contours);
    free(ft->contours_flag);
    free(outline);
}

static void outline_ensure(outline_t* outline, int points, int contours)
{
    SW_FT_Outline* ft = &outline->ft;
    if(ft->n_points + points > outline->max_points)
    {
        int capacity = ft->n_points + points;
        int newcapacity = outline->max_points == 0 ? 8 : outline->max_points;
        while(newcapacity < capacity) { newcapacity *= 2; }
        ft->points = realloc(ft->points, (size_t)newcapacity * sizeof(SW_FT_Vector));
        ft->tags = realloc(ft->tags, (size_t)newcapacity * sizeof(char));
        outline->max_points = newcapacity;
    }

    if(ft->n_contours + contours > outline->max_contours)
    {
        int capacity = ft->n_contours + contours;
        int newcapacity = outline->max_contours == 0 ? 8 : outline->max_contours;
        while(newcapacity < capacity) { newcapacity *= 2; }
        ft->contours = realloc(ft->contours, (size_t)newcapacity * sizeof(short));
        ft->contours_flag = realloc(ft->contours_flag, (size_t)newcapacity * sizeof(char));
        outline->max_contours = newcapacity;
    }
}

#define FT_COORD(x) (SW_FT_Pos)((x) * 64)
static void outline_move_to(outline_t* outline, double x, double y)
{
    outline_ensure(outline, 1, 1);

    SW_FT_Outline* ft = &outline->ft;

    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_ON;
    if(ft->n_points)
    {
        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }

    ft->contours_flag[ft->n_contours] = 1;
    ft->n_points++;
}

static void outline_line_to(outline_t* outline, double x, double y)
{
    outline_ensure(outline, 1, 0);

    SW_FT_Outline* ft = &outline->ft;

    ft->points[ft->n_points].x = FT_COORD(x);
    ft->points[ft->n_points].y = FT_COORD(y);
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void outline_cubic_to(outline_t* outline, double x1, double y1, double x2, double y2, double x3, double y3)
{
    outline_ensure(outline, 3, 0);

    SW_FT_Outline* ft = &outline->ft;

    ft->points[ft->n_points].x = FT_COORD(x1);
    ft->points[ft->n_points].y = FT_COORD(y1);
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x2);
    ft->points[ft->n_points].y = FT_COORD(y2);
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_CUBIC;
    ft->n_points++;

    ft->points[ft->n_points].x = FT_COORD(x3);
    ft->points[ft->n_points].y = FT_COORD(y3);
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void outline_close(outline_t* outline)
{
    outline_ensure(outline, 1, 1);

    SW_FT_Outline* ft = &outline->ft;

    ft->contours_flag[ft->n_contours] = 0;

    int index = ft->n_contours ? ft->contours[ft->n_contours - 1] + 1 : 0;
    if(index == ft->n_points)
        return;

    ft->points[ft->n_points].x = ft->points[index].x;
    ft->points[ft->n_points].y = ft->points[index].y;
    ft->tags[ft->n_points] = SW_FT_CURVE_TAG_ON;
    ft->n_points++;
}

static void outline_end(outline_t* outline)
{
    SW_FT_Outline* ft = &outline->ft;
    if(ft->n_points)
    {
        outline_ensure(outline, 0, 1);

        ft->contours[ft->n_contours] = ft->n_points - 1;
        ft->n_contours++;
    }
}

static void generation_callback(int count, const SW_FT_Span* spans, void* user)
{
    plutovg_rle_t* rle = user;
    plutovg_array_ensure(rle->spans, count);
    plutovg_span_t* data = rle->spans.data + rle->spans.size;
    memcpy(data, spans, (size_t)count * sizeof(plutovg_span_t));
    rle->spans.size += count;
}

static void bbox_callback(int x, int y, int w, int h, void* user)
{
    plutovg_rle_t* rle = user;
    rle->x = x;
    rle->y = y;
    rle->w = w;
    rle->h = h;
}

#define SQRT2 1.41421356237309504880
plutovg_rle_t* plutovg_rle_render(const plutovg_path_t* path, const plutovg_matrix_t* matrix, const plutovg_rect_t* clip, const plutovg_stroke_data_t* stroke, plutovg_fill_rule_t winding)
{
    outline_t* outline = outline_create();
    plutovg_path_element_t* elements = path->elements.data;
    plutovg_point_t* points = path->points.data;
    plutovg_point_t p[3];
    for(int i = 0;i < path->elements.size;i++)
    {
        switch(elements[i])
        {
        case plutovg_path_element_move_to:
            plutovg_matrix_map_point(matrix, &points[0], &p[0]);
            outline_move_to(outline, p[0].x, p[0].y);
            points += 1;
            break;
        case plutovg_path_element_line_to:
            plutovg_matrix_map_point(matrix, &points[0], &p[0]);
            outline_line_to(outline, p[0].x, p[0].y);
            points += 1;
            break;
        case plutovg_path_element_cubic_to:
            plutovg_matrix_map_point(matrix, &points[0], &p[0]);
            plutovg_matrix_map_point(matrix, &points[1], &p[1]);
            plutovg_matrix_map_point(matrix, &points[2], &p[2]);
            outline_cubic_to(outline, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            points += 3;
            break;
        case plutovg_path_element_close:
            outline_close(outline);
            points += 1;
            break;
        }
    }

    outline_end(outline);

    plutovg_rle_t* rle = malloc(sizeof(plutovg_rle_t));
    plutovg_array_init(rle->spans);

    SW_FT_Raster_Params params;
    params.flags = SW_FT_RASTER_FLAG_DIRECT | SW_FT_RASTER_FLAG_AA;
    params.gray_spans = generation_callback;
    params.bbox_cb = bbox_callback;
    params.user = rle;

    if(clip)
    {
        params.flags |= SW_FT_RASTER_FLAG_CLIP;
        params.clip_box.xMin = (SW_FT_Pos)clip->x;
        params.clip_box.yMin = (SW_FT_Pos)clip->y;
        params.clip_box.xMax = (SW_FT_Pos)(clip->x + clip->w);
        params.clip_box.yMax = (SW_FT_Pos)(clip->y + clip->h);
    }

    if(stroke)
    {
        SW_FT_Stroker_LineCap ftCap;
        SW_FT_Stroker_LineJoin ftJoin;
        SW_FT_Fixed ftWidth;
        SW_FT_Fixed ftMiterLimit;

        plutovg_point_t p1 = {0, 0};
        plutovg_point_t p2 = {SQRT2, SQRT2};
        plutovg_point_t p3;

        plutovg_matrix_map_point(matrix, &p1, &p1);
        plutovg_matrix_map_point(matrix, &p2, &p2);

        p3.x = p2.x - p1.x;
        p3.y = p2.y - p1.y;

        double scale = sqrt(p3.x*p3.x + p3.y*p3.y) / 2.0;

        ftWidth = (SW_FT_Fixed)(stroke->width * scale * 0.5 * (1 << 6));
        ftMiterLimit = (SW_FT_Fixed)(stroke->miterlimit * (1 << 16));

        switch(stroke->cap)
        {
        case plutovg_line_cap_square:
            ftCap = SW_FT_STROKER_LINECAP_SQUARE;
            break;
        case plutovg_line_cap_round:
            ftCap = SW_FT_STROKER_LINECAP_ROUND;
            break;
        default:
            ftCap = SW_FT_STROKER_LINECAP_BUTT;
            break;
        }

        switch(stroke->join)
        {
        case plutovg_line_join_bevel:
            ftJoin = SW_FT_STROKER_LINEJOIN_BEVEL;
            break;
        case plutovg_line_join_round:
            ftJoin = SW_FT_STROKER_LINEJOIN_ROUND;
            break;
        default:
            ftJoin = SW_FT_STROKER_LINEJOIN_MITER_FIXED;
            break;
        }

        SW_FT_Stroker stroker;
        SW_FT_Stroker_New(&stroker);
        SW_FT_Stroker_Set(stroker, ftWidth, ftCap, ftJoin, ftMiterLimit);
        SW_FT_Stroker_ParseOutline(stroker, &outline->ft);

        SW_FT_UInt points;
        SW_FT_UInt contours;
        SW_FT_Stroker_GetCounts(stroker, &points, &contours);

        outline_t* strokeOutline = outline_create();
        outline_ensure(strokeOutline, (int)points, (int)contours);
        SW_FT_Stroker_Export(stroker, &strokeOutline->ft);
        SW_FT_Stroker_Done(stroker);

        strokeOutline->ft.flags = SW_FT_OUTLINE_NONE;
        params.source = &strokeOutline->ft;
        sw_ft_grays_raster.raster_render(NULL, &params);
        outline_destroy(strokeOutline);
    }
    else
    {
        outline->ft.flags = winding == plutovg_fill_rule_even_odd ? SW_FT_OUTLINE_EVEN_ODD_FILL : SW_FT_OUTLINE_NONE;
        params.source = &outline->ft;
        sw_ft_grays_raster.raster_render(NULL, &params);
    }

    outline_destroy(outline);
    return rle;
}

void plutovg_rle_destroy(plutovg_rle_t* rle)
{
    if(rle==NULL)
        return;

    free(rle->spans.data);
    free(rle);
}

void plutovg_rle_clip_path(plutovg_rle_t* rle, const plutovg_rle_t* clip)
{
    (void)rle;
    (void)clip;
}

void plutovg_rle_clip_rect(plutovg_rle_t* rle, const plutovg_rect_t* clip)
{
    (void)rle;
    (void)clip;
}

plutovg_rle_t* plutovg_rle_clone(const plutovg_rle_t* rle)
{
    if(rle==NULL)
        return NULL;

    plutovg_rle_t* r = malloc(sizeof(plutovg_rle_t));
    plutovg_array_init(r->spans);
    plutovg_array_ensure(r->spans, rle->spans.size);

    memcpy(r->spans.data, rle->spans.data, (size_t)rle->spans.size * sizeof(plutovg_span_t));
    r->spans.size = rle->spans.size;
    r->x = rle->x;
    r->y = rle->y;
    r->w = rle->w;
    r->h = rle->h;
    return r;
}
