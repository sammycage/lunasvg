#include "plutovg-private.h"

plutovg_surface_t* plutovg_surface_create(int width, int height)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    const size_t size = (size_t)(width * height * 4);
    surface->ref = 1;
    surface->owndata = 1;
    surface->data = malloc(size);
    memset(surface->data, 0, size);
    surface->width = width;
    surface->height = height;
    surface->stride = width * 4;
    return surface;
}

plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref = 1;
    surface->owndata = 0;
    surface->data = data;
    surface->width = width;
    surface->height = height;
    surface->stride = stride;
    return surface;
}

plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface)
{
    ++surface->ref;
    return surface;
}

void plutovg_surface_destroy(plutovg_surface_t* surface)
{
    if(surface==NULL)
        return;

    if(--surface->ref==0)
    {
        if(surface->owndata)
            free(surface->data);
        free(surface);
    }
}

int plutovg_surface_get_reference_count(const plutovg_surface_t* surface)
{
    return surface->ref;
}

unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface)
{
    return surface->data;
}

int plutovg_surface_get_width(const plutovg_surface_t* surface)
{
    return surface->width;
}

int plutovg_surface_get_height(const plutovg_surface_t* surface)
{
    return surface->height;
}

int plutovg_surface_get_stride(const plutovg_surface_t* surface)
{
    return surface->stride;
}

plutovg_state_t* plutovg_state_create(void)
{
    plutovg_state_t* state = malloc(sizeof(plutovg_state_t));
    state->clip = NULL;
    state->source = plutovg_paint_create_rgb(0, 0, 0);
    plutovg_matrix_init_identity(&state->matrix);
    state->winding = plutovg_fill_rule_non_zero;
    state->stroke.width = 1.0;
    state->stroke.miterlimit = 4.0;
    state->stroke.cap = plutovg_line_cap_butt;
    state->stroke.join = plutovg_line_join_miter;
    state->op = plutovg_operator_src_over;
    state->opacity = 1.0;
    state->next = NULL;
    return state;
}

plutovg_state_t* plutovg_state_clone(const plutovg_state_t* state)
{
    plutovg_state_t* newstate = malloc(sizeof(plutovg_state_t));
    newstate->clip = plutovg_rle_clone(state->clip);
    newstate->source = plutovg_paint_reference(state->source); /** FIXME: **/
    newstate->matrix = state->matrix;
    newstate->winding = state->winding;
    newstate->stroke.width = state->stroke.width;
    newstate->stroke.miterlimit = state->stroke.miterlimit;
    newstate->stroke.cap = state->stroke.cap;
    newstate->stroke.join = state->stroke.join;
    newstate->op = state->op;
    newstate->opacity = state->opacity;
    newstate->next = NULL;
    return newstate;
}

void plutovg_state_destroy(plutovg_state_t* state)
{
    plutovg_rle_destroy(state->clip);
    plutovg_paint_destroy(state->source);
    free(state);
}

plutovg_t* plutovg_create(plutovg_surface_t* surface)
{
    plutovg_t* pluto = malloc(sizeof(plutovg_t));
    pluto->ref = 1;
    pluto->surface = plutovg_surface_reference(surface);
    pluto->state = plutovg_state_create();
    pluto->path = plutovg_path_create();
    pluto->clip.x = 0.0;
    pluto->clip.y = 0.0;
    pluto->clip.w = surface->width;
    pluto->clip.h = surface->height;
    return pluto;
}

plutovg_t* plutovg_reference(plutovg_t* pluto)
{
    ++pluto->ref;
    return pluto;
}

void plutovg_destroy(plutovg_t* pluto)
{
    if(pluto==NULL)
        return;

    if(--pluto->ref==0)
    {
        while(pluto->state)
        {
            plutovg_state_t* state = pluto->state;
            pluto->state = state->next;
            plutovg_state_destroy(state);
        }

        plutovg_surface_destroy(pluto->surface);
        plutovg_path_destroy(pluto->path);
        free(pluto);
    }
}

int plutovg_get_reference_count(const plutovg_t* pluto)
{
    return pluto->ref;
}

void plutovg_save(plutovg_t* pluto)
{
    plutovg_state_t* newstate = plutovg_state_clone(pluto->state);
    newstate->next = pluto->state;
    pluto->state = newstate;
}

void plutovg_restore(plutovg_t* pluto)
{
    plutovg_state_t* oldstate = pluto->state;
    pluto->state = oldstate->next;
    plutovg_state_destroy(oldstate);
}

void plutovg_set_source_rgb(plutovg_t* pluto, double r, double g, double b)
{
    plutovg_set_source_rgba(pluto, r, g, b, 1.0);
}

void plutovg_set_source_rgba(plutovg_t* pluto, double r, double g, double b, double a)
{
    plutovg_paint_t* source = plutovg_paint_create_rgba(r, g, b, a);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_surface(plutovg_t* pluto, plutovg_surface_t* surface, double x, double y)
{
    plutovg_paint_t* source = plutovg_paint_create_for_surface(surface);
    plutovg_texture_t* texture = plutovg_paint_get_texture(source);
    plutovg_matrix_t matrix;
    plutovg_matrix_init_translate(&matrix, x, y);
    plutovg_texture_set_matrix(texture, &matrix);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_color(plutovg_t* pluto, const plutovg_color_t* color)
{
    plutovg_set_source_rgba(pluto, color->r, color->g, color->b, color->a);
}

void plutovg_set_source_gradient(plutovg_t* pluto, plutovg_gradient_t* gradient)
{
    plutovg_paint_t* source = plutovg_paint_create_gradient(gradient);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_texture(plutovg_t* pluto, plutovg_texture_t* texture)
{
    plutovg_paint_t* source = plutovg_paint_create_texture(texture);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source(plutovg_t* pluto, plutovg_paint_t* source)
{
    plutovg_paint_destroy(pluto->state->source);
    pluto->state->source = plutovg_paint_reference(source);
}

void plutovg_set_fill_rule(plutovg_t* pluto, plutovg_fill_rule_t fill_rule)
{
    pluto->state->winding = fill_rule;
}

void plutovg_set_line_width(plutovg_t* pluto, double width)
{
    pluto->state->stroke.width = width;
}

void plutovg_set_line_cap(plutovg_t* pluto, plutovg_line_cap_t cap)
{
    pluto->state->stroke.cap = cap;
}

void plutovg_set_line_join(plutovg_t* pluto, plutovg_line_join_t join)
{
    pluto->state->stroke.join = join;
}

void plutovg_set_miter_limit(plutovg_t* pluto, double limit)
{
    pluto->state->stroke.miterlimit = limit;
}

void plutovg_set_operator(plutovg_t* pluto, plutovg_operator_t op)
{
    pluto->state->op = op;
}

void plutovg_set_opacity(plutovg_t* pluto, double opacity)
{
    pluto->state->opacity = opacity;
}

void plutovg_translate(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_translate(&pluto->state->matrix, x, y);
}

void plutovg_scale(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_scale(&pluto->state->matrix, x, y);
}

void plutovg_rotate(plutovg_t* pluto, double radians, double x, double y)
{
    plutovg_matrix_rotate(&pluto->state->matrix, radians, x, y);
}

void plutovg_transform(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    plutovg_matrix_multiply(&pluto->state->matrix, matrix, &pluto->state->matrix);
}

void plutovg_set_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    memcpy(&pluto->state->matrix, matrix, sizeof(plutovg_matrix_t));
}

void plutovg_identity_matrix(plutovg_t* pluto)
{
    plutovg_matrix_init_identity(&pluto->state->matrix);
}

void plutovg_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_move_to(pluto->path, x, y);
}

void plutovg_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_line_to(pluto->path, x, y);
}

void plutovg_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rel_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_move_to(pluto->path, x, y);
}

void plutovg_rel_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_line_to(pluto->path, x, y);
}

void plutovg_rel_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_rel_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_rel_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_rel_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rect(plutovg_t* pluto, double x, double y, double w, double h)
{
    plutovg_path_add_rect(pluto->path, x, y, w, h);
}

void plutovg_round_rect(plutovg_t* pluto, double x, double y, double w, double h, double rx, double ry)
{
    plutovg_path_add_round_rect(pluto->path, x, y, w, h, rx, ry);
}

void plutovg_ellipse(plutovg_t* pluto, double cx, double cy, double rx, double ry)
{
    plutovg_path_add_ellipse(pluto->path, cx, cy, rx, ry);
}

void plutovg_circle(plutovg_t* pluto, double cx, double cy, double r)
{
    plutovg_ellipse(pluto, cx, cy, r, r);
}

void plutovg_add_path(plutovg_t* pluto, const plutovg_path_t* path)
{
    plutovg_path_add_path(pluto->path, path, NULL);
}

void plutovg_new_path(plutovg_t* pluto)
{
    plutovg_path_clear(pluto->path);
}

void plutovg_close_path(plutovg_t* pluto)
{
    plutovg_path_close(pluto->path);
}

void plutovg_fill(plutovg_t* pluto)
{
    plutovg_fill_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_stroke(plutovg_t* pluto)
{
    plutovg_stroke_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_clip(plutovg_t* pluto)
{
    plutovg_clip_preserve(pluto);
    plutovg_new_path(pluto);
}

static void plutovg_paint_rle(plutovg_t* pluto, const plutovg_rle_t* rle);

void plutovg_fill_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_t* rle = plutovg_rle_render(pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    plutovg_rle_clip_path(rle, state->clip);
    plutovg_paint_rle(pluto, rle);
    plutovg_rle_destroy(rle);
}

void plutovg_stroke_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_t* rle = plutovg_rle_render(pluto->path, &state->matrix, &pluto->clip, &state->stroke, state->winding);
    plutovg_rle_clip_path(rle, state->clip);
    plutovg_paint_rle(pluto, rle);
    plutovg_rle_destroy(rle);
}

void plutovg_clip_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_t* rle = plutovg_rle_render(pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    if(state->clip)
    {
        plutovg_rle_clip_path(state->clip, rle);
        plutovg_rle_destroy(rle);
    }
    else
    {
        state->clip = rle;
    }
}

void plutovg_paint(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    if(state->clip==NULL)
    {
        plutovg_path_t* path = plutovg_path_create();
        plutovg_path_add_rect(path, pluto->clip.x, pluto->clip.y, pluto->clip.w, pluto->clip.h);
        plutovg_rle_t* rle = plutovg_rle_render(path, &state->matrix, &pluto->clip, NULL, state->winding);
        plutovg_paint_rle(pluto, rle);
        plutovg_path_destroy(path);
        plutovg_rle_destroy(rle);
    }
    else
    {
        plutovg_paint_rle(pluto, state->clip);
    }
}

plutovg_fill_rule_t plutovg_get_fill_rule(const plutovg_t* pluto)
{
    return pluto->state->winding;
}

double plutovg_get_line_width(const plutovg_t* pluto)
{
    return pluto->state->stroke.width;
}

plutovg_line_cap_t plutovg_get_line_cap(const plutovg_t* pluto)
{
    return pluto->state->stroke.cap;
}

plutovg_line_join_t plutovg_get_line_join(const plutovg_t* pluto)
{
    return pluto->state->stroke.join;
}

double plutovg_get_miter_limit(const plutovg_t* pluto)
{
    return pluto->state->stroke.miterlimit;
}

plutovg_operator_t plutovg_get_operator(const plutovg_t* pluto)
{
    return pluto->state->op;
}

double plutovg_get_opacity(const plutovg_t* pluto)
{
    return pluto->state->opacity;
}

void plutovg_get_matrix(const plutovg_t* pluto, plutovg_matrix_t* matrix)
{
    memcpy(matrix, &pluto->state->matrix, sizeof(plutovg_matrix_t));
}

plutovg_path_t* plutovg_get_path(const plutovg_t* pluto)
{
    return pluto->path;
}

plutovg_paint_t* plutovg_get_source(const plutovg_t* pluto)
{
    return pluto->state->source;
}

static inline uint32_t premultiply_color(const plutovg_color_t* color, double opacity)
{
    uint32_t alpha = (uint8_t)(color->a * opacity * 255);
    uint32_t pr = (uint8_t)(color->r * alpha);
    uint32_t pg = (uint8_t)(color->g * alpha);
    uint32_t pb = (uint8_t)(color->b * alpha);

    return (alpha << 24) | (pr << 16) | (pg << 8) | (pb);
}

static void plutovg_paint_rle_color(plutovg_t* pluto, const plutovg_color_t* color, const plutovg_rle_t* rle)
{
    plutovg_state_t* state = pluto->state;
    uint32_t solid = premultiply_color(color, state->opacity);
    plutovg_blend_solid(pluto->surface, state->op, rle, solid);
}

static inline uint32_t interpolate_pixel(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
    uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;
    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static void plutovg_paint_rle_gradient(plutovg_t* pluto, const plutovg_gradient_t* gradient, const plutovg_rle_t* rle)
{
    plutovg_state_t* state = pluto->state;
    plutovg_gradient_data_t data;
    if(gradient->type==plutovg_gradient_type_linear)
    {
        data.type = plutovg_gradient_type_linear;
        data.linear.x1 = gradient->values[0];
        data.linear.y1 = gradient->values[1];
        data.linear.x2 = gradient->values[2];
        data.linear.y2 = gradient->values[3];
    }
    else
    {
        data.type = plutovg_gradient_type_radial;
        data.radial.cx = gradient->values[0];
        data.radial.cy = gradient->values[1];
        data.radial.cr = gradient->values[2];
        data.radial.fx = gradient->values[3];
        data.radial.fy = gradient->values[4];
        data.radial.fr = gradient->values[5];
    }

    int dist, idist, pos = 0;
    int i;
    int alpha = 0;
    int nstop = gradient->stops.size;
    const plutovg_gradient_stop_t *curr, *next, *start;
    uint32_t curr_color, next_color;
    double delta, t, incr, fpos;
    double opacity = state->opacity * gradient->opacity;

    if(opacity != 1.0) alpha = 1;

    start = gradient->stops.data;
    curr = start;
    if(curr->color.a != 0.0) alpha = 1;
    curr_color = premultiply_color(&curr->color, opacity);
    incr = 1.0 / COLOR_TABLE_SIZE;
    fpos = 1.5 * incr;

    data.colortable[pos++] = curr_color;

    while(fpos <= curr->offset)
    {
        data.colortable[pos] = data.colortable[pos - 1];
        pos++;
        fpos += incr;
    }

    for(i = 0;i < nstop - 1;i++)
    {
        curr = (start + i);
        next = (start + i + 1);
        delta = 1.0 / (next->offset - curr->offset);
        if(next->color.a != 0.0) alpha = 1;
        next_color = premultiply_color(&next->color, opacity);
        while(fpos < next->offset && pos < COLOR_TABLE_SIZE)
        {
            t = (fpos - curr->offset) * delta;
            dist = (int)(255 * t);
            idist = 255 - dist;
            data.colortable[pos] = interpolate_pixel(curr_color, (uint32_t)idist, next_color, (uint32_t)dist);
            ++pos;
            fpos += incr;
        }

        curr_color = next_color;
    }

    for(;pos < COLOR_TABLE_SIZE;++pos) data.colortable[pos] = curr_color;
    data.colortable[COLOR_TABLE_SIZE - 1] = curr_color;
    data.spread = gradient->spread;

    data.matrix = gradient->matrix;
    plutovg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
    plutovg_matrix_invert(&data.matrix);

    plutovg_blend_gradient(pluto->surface, state->op, rle, &data);
}

static void plutovg_paint_rle_texture(plutovg_t* pluto, const plutovg_texture_t* texture, const plutovg_rle_t* rle)
{
    plutovg_state_t* state = pluto->state;
    plutovg_texture_data_t data;
    data.type = texture->type;
    data.data = texture->surface->data;
    data.width = texture->surface->width;
    data.height = texture->surface->height;
    data.stride = texture->surface->stride;
    data.const_alpha = (int)(state->opacity * texture->opacity * 255.0);

    data.matrix = texture->matrix;
    plutovg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
    plutovg_matrix_invert(&data.matrix);

    plutovg_blend_texture(pluto->surface, state->op, rle, &data);
}

static void plutovg_paint_rle(plutovg_t* pluto, const plutovg_rle_t* rle)
{
    plutovg_paint_t* source = pluto->state->source;
    if(source->type==plutovg_paint_type_color)
    {
        const plutovg_color_t* color = plutovg_paint_get_color(source);
        plutovg_paint_rle_color(pluto, color, rle);
    }
    else if(source->type==plutovg_paint_type_gradient)
    {
        const plutovg_gradient_t* gradient = plutovg_paint_get_gradient(source);
        plutovg_paint_rle_gradient(pluto, gradient, rle);
    }
    else
    {
        const plutovg_texture_t* texture = plutovg_paint_get_texture(source);
        plutovg_paint_rle_texture(pluto, texture, rle);
    }
}
