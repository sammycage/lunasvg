#include "plutovg-private.h"
#include "plutovg-utils.h"

#include <assert.h>
#include <limits.h>

#define COLOR_TABLE_SIZE 1024
typedef struct {
    plutovg_matrix_t matrix;
    plutovg_spread_method_t spread;
    uint32_t colortable[COLOR_TABLE_SIZE];
    union {
        struct {
            float x1, y1;
            float x2, y2;
        } linear;
        struct {
            float cx, cy, cr;
            float fx, fy, fr;
        } radial;
    } values;
} gradient_data_t;

typedef struct {
    plutovg_matrix_t matrix;
    uint8_t* data;
    int width;
    int height;
    int stride;
    int const_alpha;
} texture_data_t;

typedef struct {
    float dx;
    float dy;
    float l;
    float off;
} linear_gradient_values_t;

typedef struct {
    float dx;
    float dy;
    float dr;
    float sqrfr;
    float a;
    bool extended;
} radial_gradient_values_t;

static inline uint32_t premultiply_color_with_opacity(const plutovg_color_t* color, float opacity)
{
    uint32_t alpha = lroundf(color->a * opacity * 255);
    uint32_t pr = lroundf(color->r * alpha);
    uint32_t pg = lroundf(color->g * alpha);
    uint32_t pb = lroundf(color->b * alpha);
    return (alpha << 24) | (pr << 16) | (pg << 8) | (pb);
}

static inline uint32_t INTERPOLATE_PIXEL_255(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
    uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static inline uint32_t INTERPOLATE_PIXEL_256(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
    uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

static inline uint32_t BYTE_MUL(uint32_t x, uint32_t a)
{
    uint32_t t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a;
    x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
    x &= 0xff00ff00;
    x |= t;
    return x;
}

#ifdef __SSE2__

#include <emmintrin.h>

void plutovg_memfill32(unsigned int* dest, int length, unsigned int value)
{
    __m128i vector_data = _mm_set_epi32(value, value, value, value);
    while(length && ((uintptr_t)dest & 0xf)) {
        *dest++ = value;
        length--;
    }

    while(length >= 32) {
        _mm_store_si128((__m128i*)(dest), vector_data);
        _mm_store_si128((__m128i*)(dest + 4), vector_data);
        _mm_store_si128((__m128i*)(dest + 8), vector_data);
        _mm_store_si128((__m128i*)(dest + 12), vector_data);
        _mm_store_si128((__m128i*)(dest + 16), vector_data);
        _mm_store_si128((__m128i*)(dest + 20), vector_data);
        _mm_store_si128((__m128i*)(dest + 24), vector_data);
        _mm_store_si128((__m128i*)(dest + 28), vector_data);

        dest += 32;
        length -= 32;
    }

    if(length >= 16) {
        _mm_store_si128((__m128i*)(dest), vector_data);
        _mm_store_si128((__m128i*)(dest + 4), vector_data);
        _mm_store_si128((__m128i*)(dest + 8), vector_data);
        _mm_store_si128((__m128i*)(dest + 12), vector_data);

        dest += 16;
        length -= 16;
    }

    if(length >= 8) {
        _mm_store_si128((__m128i*)(dest), vector_data);
        _mm_store_si128((__m128i*)(dest + 4), vector_data);

        dest += 8;
        length -= 8;
    }

    if(length >= 4) {
        _mm_store_si128((__m128i*)(dest), vector_data);

        dest += 4;
        length -= 4;
    }

    while(length) {
        *dest++ = value;
        length--;
    }
}

#else

void plutovg_memfill32(unsigned int* dest, int length, unsigned int value)
{
    while(length--) {
        *dest++ = value;
    }
}

#endif // __SSE2__

static inline int gradient_clamp(const gradient_data_t* gradient, int ipos)
{
    if(gradient->spread == PLUTOVG_SPREAD_METHOD_REPEAT) {
        ipos = ipos % COLOR_TABLE_SIZE;
        ipos = ipos < 0 ? COLOR_TABLE_SIZE + ipos : ipos;
    } else if(gradient->spread == PLUTOVG_SPREAD_METHOD_REFLECT) {
        const int limit = COLOR_TABLE_SIZE * 2;
        ipos = ipos % limit;
        ipos = ipos < 0 ? limit + ipos : ipos;
        ipos = ipos >= COLOR_TABLE_SIZE ? limit - 1 - ipos : ipos;
    } else {
        if(ipos < 0) {
            ipos = 0;
        } else if(ipos >= COLOR_TABLE_SIZE) {
            ipos = COLOR_TABLE_SIZE - 1;
        }
    }

    return ipos;
}

#define FIXPT_BITS 8
#define FIXPT_SIZE (1 << FIXPT_BITS)
static inline uint32_t gradient_pixel_fixed(const gradient_data_t* gradient, int fixed_pos)
{
    int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;
    return gradient->colortable[gradient_clamp(gradient, ipos)];
}

static inline uint32_t gradient_pixel(const gradient_data_t* gradient, float pos)
{
    int ipos = (int)(pos * (COLOR_TABLE_SIZE - 1) + 0.5f);
    return gradient->colortable[gradient_clamp(gradient, ipos)];
}

static void fetch_linear_gradient(uint32_t* buffer, const linear_gradient_values_t* v, const gradient_data_t* gradient, int y, int x, int length)
{
    float t, inc;
    float rx = 0, ry = 0;

    if(v->l == 0.f) {
        t = inc = 0;
    } else {
        rx = gradient->matrix.c * (y + 0.5f) + gradient->matrix.a * (x + 0.5f) + gradient->matrix.e;
        ry = gradient->matrix.d * (y + 0.5f) + gradient->matrix.b * (x + 0.5f) + gradient->matrix.f;
        t = v->dx * rx + v->dy * ry + v->off;
        inc = v->dx * gradient->matrix.a + v->dy * gradient->matrix.b;
        t *= (COLOR_TABLE_SIZE - 1);
        inc *= (COLOR_TABLE_SIZE - 1);
    }

    const uint32_t* end = buffer + length;
    if(inc > -1e-5f && inc < 1e-5f) {
        plutovg_memfill32(buffer, length, gradient_pixel_fixed(gradient, (int)(t * FIXPT_SIZE)));
    } else {
        if(t + inc * length < (float)(INT_MAX >> (FIXPT_BITS + 1)) && t + inc * length > (float)(INT_MIN >> (FIXPT_BITS + 1))) {
            int t_fixed = (int)(t * FIXPT_SIZE);
            int inc_fixed = (int)(inc * FIXPT_SIZE);
            while(buffer < end) {
                *buffer = gradient_pixel_fixed(gradient, t_fixed);
                t_fixed += inc_fixed;
                ++buffer;
            }
        } else {
            while(buffer < end) {
                *buffer = gradient_pixel(gradient, t / COLOR_TABLE_SIZE);
                t += inc;
                ++buffer;
            }
        }
    }
}

static void fetch_radial_gradient(uint32_t* buffer, const radial_gradient_values_t* v, const gradient_data_t* gradient, int y, int x, int length)
{
    if(v->a == 0.f) {
        plutovg_memfill32(buffer, length, 0);
        return;
    }

    float rx = gradient->matrix.c * (y + 0.5f) + gradient->matrix.e + gradient->matrix.a * (x + 0.5f);
    float ry = gradient->matrix.d * (y + 0.5f) + gradient->matrix.f + gradient->matrix.b * (x + 0.5f);

    rx -= gradient->values.radial.fx;
    ry -= gradient->values.radial.fy;

    float inv_a = 1.f / (2.f * v->a);
    float delta_rx = gradient->matrix.a;
    float delta_ry = gradient->matrix.b;

    float b = 2 * (v->dr * gradient->values.radial.fr + rx * v->dx + ry * v->dy);
    float delta_b = 2 * (delta_rx * v->dx + delta_ry * v->dy);
    float b_delta_b = 2 * b * delta_b;
    float delta_b_delta_b = 2 * delta_b * delta_b;

    float bb = b * b;
    float delta_bb = delta_b * delta_b;

    b *= inv_a;
    delta_b *= inv_a;

    float rxrxryry = rx * rx + ry * ry;
    float delta_rxrxryry = delta_rx * delta_rx + delta_ry * delta_ry;
    float rx_plus_ry = 2 * (rx * delta_rx + ry * delta_ry);
    float delta_rx_plus_ry = 2 * delta_rxrxryry;

    inv_a *= inv_a;

    float det = (bb - 4 * v->a * (v->sqrfr - rxrxryry)) * inv_a;
    float delta_det = (b_delta_b + delta_bb + 4 * v->a * (rx_plus_ry + delta_rxrxryry)) * inv_a;
    float delta_delta_det = (delta_b_delta_b + 4 * v->a * delta_rx_plus_ry) * inv_a;

    const uint32_t* end = buffer + length;
    if(v->extended) {
        while(buffer < end) {
            uint32_t result = 0;
            if(det >= 0) {
                float w = sqrtf(det) - b;
                if(gradient->values.radial.fr + v->dr * w >= 0) {
                    result = gradient_pixel(gradient, w);
                }
            }

            *buffer = result;
            det += delta_det;
            delta_det += delta_delta_det;
            b += delta_b;
            ++buffer;
        }
    } else {
        while(buffer < end) {
            *buffer++ = gradient_pixel(gradient, sqrtf(det) - b);
            det += delta_det;
            delta_det += delta_delta_det;
            b += delta_b;
        }
    }
}

static void composition_solid_clear(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        plutovg_memfill32(dest, length, 0);
    } else {
        uint32_t ialpha = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(dest[i], ialpha);
        }
    }
}

static void composition_solid_source(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        plutovg_memfill32(dest, length, color);
    } else {
        uint32_t ialpha = 255 - const_alpha;
        color = BYTE_MUL(color, const_alpha);
        for(int i = 0; i < length; i++) {
            dest[i] = color + BYTE_MUL(dest[i], ialpha);
        }
    }
}

static void composition_solid_destination(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
}

static void composition_solid_source_over(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    uint32_t ialpha = 255 - plutovg_alpha(color);
    for(int i = 0; i < length; i++) {
        dest[i] = color + BYTE_MUL(dest[i], ialpha);
    }
}

static void composition_solid_destination_over(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    for(int i = 0; i < length; i++) {
        uint32_t d = dest[i];
        dest[i] = d + BYTE_MUL(color, plutovg_alpha(~d));
    }
}

static void composition_solid_source_in(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(color, plutovg_alpha(dest[i]));
        }
    } else {
        color = BYTE_MUL(color, const_alpha);
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(color, plutovg_alpha(d), d, cia);
        }
    }
}

static void composition_solid_destination_in(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    uint32_t a = plutovg_alpha(color);
    if(const_alpha != 255)
        a = BYTE_MUL(a, const_alpha) + 255 - const_alpha;
    for(int i = 0; i < length; i++) {
        dest[i] = BYTE_MUL(dest[i], a);
    }
}

static void composition_solid_source_out(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(color, plutovg_alpha(~dest[i]));
        }
    } else {
        color = BYTE_MUL(color, const_alpha);
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(color, plutovg_alpha(~d), d, cia);
        }
    }
}

static void composition_solid_destination_out(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    uint32_t a = plutovg_alpha(~color);
    if(const_alpha != 255)
        a = BYTE_MUL(a, const_alpha) + 255 - const_alpha;
    for(int i = 0; i < length; i++) {
        dest[i] = BYTE_MUL(dest[i], a);
    }
}

static void composition_solid_source_atop(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    uint32_t sia = plutovg_alpha(~color);
    for(int i = 0; i < length; i++) {
        uint32_t d = dest[i];
        dest[i] = INTERPOLATE_PIXEL_255(color, plutovg_alpha(d), d, sia);
    }
}

static void composition_solid_destination_atop(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    uint32_t a = plutovg_alpha(color);
    if(const_alpha != 255) {
        color = BYTE_MUL(color, const_alpha);
        a = plutovg_alpha(color) + 255 - const_alpha;
    }

    for(int i = 0; i < length; i++) {
        uint32_t d = dest[i];
        dest[i] = INTERPOLATE_PIXEL_255(d, a, color, plutovg_alpha(~d));
    }
}

static void composition_solid_xor(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha)
{
    if(const_alpha != 255)
        color = BYTE_MUL(color, const_alpha);
    uint32_t sia = plutovg_alpha(~color);
    for(int i = 0; i < length; i++) {
        uint32_t d = dest[i];
        dest[i] = INTERPOLATE_PIXEL_255(color, plutovg_alpha(~d), d, sia);
    }
}

typedef void(*composition_solid_function_t)(uint32_t* dest, int length, uint32_t color, uint32_t const_alpha);

static const composition_solid_function_t composition_solid_table[] = {
    composition_solid_clear,
    composition_solid_source,
    composition_solid_destination,
    composition_solid_source_over,
    composition_solid_destination_over,
    composition_solid_source_in,
    composition_solid_destination_in,
    composition_solid_source_out,
    composition_solid_destination_out,
    composition_solid_source_atop,
    composition_solid_destination_atop,
    composition_solid_xor
};

static void composition_clear(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        plutovg_memfill32(dest, length, 0);
    } else {
        uint32_t ialpha = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(dest[i], ialpha);
        }
    }
}

static void composition_source(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        memcpy(dest, src, length * sizeof(uint32_t));
    } else {
        uint32_t ialpha = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            dest[i] = INTERPOLATE_PIXEL_255(src[i], const_alpha, dest[i], ialpha);
        }
    }
}

static void composition_destination(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
}

static void composition_source_over(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            uint32_t s = src[i];
            if(s >= 0xff000000) {
                dest[i] = s;
            } else if(s != 0) {
                dest[i] = s + BYTE_MUL(dest[i], plutovg_alpha(~s));
            }
        }
    } else {
        for(int i = 0; i < length; i++) {
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            dest[i] = s + BYTE_MUL(dest[i], plutovg_alpha(~s));
        }
    }
}

static void composition_destination_over(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            dest[i] = d + BYTE_MUL(src[i], plutovg_alpha(~d));
        }
    } else {
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            dest[i] = d + BYTE_MUL(s, plutovg_alpha(~d));
        }
    }
}

static void composition_source_in(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(src[i], plutovg_alpha(dest[i]));
        }
    } else {
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(d), d, cia);
        }
    }
}

static void composition_destination_in(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(dest[i], plutovg_alpha(src[i]));
        }
    } else {
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t a = BYTE_MUL(plutovg_alpha(src[i]), const_alpha) + cia;
            dest[i] = BYTE_MUL(dest[i], a);
        }
    }
}

static void composition_source_out(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(src[i], plutovg_alpha(~dest[i]));
        }
    } else {
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(~d), d, cia);
        }
    }
}

static void composition_destination_out(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            dest[i] = BYTE_MUL(dest[i], plutovg_alpha(~src[i]));
        }
    } else {
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t sia = BYTE_MUL(plutovg_alpha(~src[i]), const_alpha) + cia;
            dest[i] = BYTE_MUL(dest[i], sia);
        }
    }
}

static void composition_source_atop(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            uint32_t s = src[i];
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(d), d, plutovg_alpha(~s));
        }
    } else {
        for(int i = 0; i < length; i++) {
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(d), d, plutovg_alpha(~s));
        }
    }
}

static void composition_destination_atop(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            uint32_t s = src[i];
            uint32_t d = dest[i];
            dest[i] = INTERPOLATE_PIXEL_255(d, plutovg_alpha(s), s, plutovg_alpha(~d));
        }
    } else {
        uint32_t cia = 255 - const_alpha;
        for(int i = 0; i < length; i++) {
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            uint32_t d = dest[i];
            uint32_t a = plutovg_alpha(s) + cia;
            dest[i] = INTERPOLATE_PIXEL_255(d, a, s, plutovg_alpha(~d));
        }
    }
}

static void composition_xor(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha)
{
    if(const_alpha == 255) {
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            uint32_t s = src[i];
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(~d), d, plutovg_alpha(~s));
        }
    } else {
        for(int i = 0; i < length; i++) {
            uint32_t d = dest[i];
            uint32_t s = BYTE_MUL(src[i], const_alpha);
            dest[i] = INTERPOLATE_PIXEL_255(s, plutovg_alpha(~d), d, plutovg_alpha(~s));
        }
    }
}

typedef void(*composition_function_t)(uint32_t* dest, int length, const uint32_t* src, uint32_t const_alpha);

static const composition_function_t composition_table[] = {
    composition_clear,
    composition_source,
    composition_destination,
    composition_source_over,
    composition_destination_over,
    composition_source_in,
    composition_destination_in,
    composition_source_out,
    composition_destination_out,
    composition_source_atop,
    composition_destination_atop,
    composition_xor
};

static void blend_solid(plutovg_surface_t* surface, plutovg_operator_t op, uint32_t solid, const plutovg_span_buffer_t* span_buffer)
{
    composition_solid_function_t func = composition_solid_table[op];
    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + spans->x;
        func(target, spans->len, solid, spans->coverage);
        ++spans;
    }
}

#define BUFFER_SIZE 1024
static void blend_linear_gradient(plutovg_surface_t* surface, plutovg_operator_t op, const gradient_data_t* gradient, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];
    unsigned int buffer[BUFFER_SIZE];

    linear_gradient_values_t v;
    v.dx = gradient->values.linear.x2 - gradient->values.linear.x1;
    v.dy = gradient->values.linear.y2 - gradient->values.linear.y1;
    v.l = v.dx * v.dx + v.dy * v.dy;
    v.off = 0.f;
    if(v.l != 0.f) {
        v.dx /= v.l;
        v.dy /= v.l;
        v.off = -v.dx * gradient->values.linear.x1 - v.dy * gradient->values.linear.y1;
    }

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        int length = spans->len;
        int x = spans->x;
        while(length) {
            int l = plutovg_min(length, BUFFER_SIZE);
            fetch_linear_gradient(buffer, &v, gradient, spans->y, x, l);
            uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + x;
            func(target, l, buffer, spans->coverage);
            x += l;
            length -= l;
        }

        ++spans;
    }
}

static void blend_radial_gradient(plutovg_surface_t* surface, plutovg_operator_t op, const gradient_data_t* gradient, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];
    unsigned int buffer[BUFFER_SIZE];

    radial_gradient_values_t v;
    v.dx = gradient->values.radial.cx - gradient->values.radial.fx;
    v.dy = gradient->values.radial.cy - gradient->values.radial.fy;
    v.dr = gradient->values.radial.cr - gradient->values.radial.fr;
    v.sqrfr = gradient->values.radial.fr * gradient->values.radial.fr;
    v.a = v.dr * v.dr - v.dx * v.dx - v.dy * v.dy;
    v.extended = gradient->values.radial.fr != 0.f || v.a <= 0.f;

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        int length = spans->len;
        int x = spans->x;
        while(length) {
            int l = plutovg_min(length, BUFFER_SIZE);
            fetch_radial_gradient(buffer, &v, gradient, spans->y, x, l);
            uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + x;
            func(target, l, buffer, spans->coverage);
            x += l;
            length -= l;
        }

        ++spans;
    }
}

static void blend_untransformed_argb(plutovg_surface_t* surface, plutovg_operator_t op, const texture_data_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];

    const int image_width = texture->width;
    const int image_height = texture->height;

    int xoff = (int)(texture->matrix.e);
    int yoff = (int)(texture->matrix.f);

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = xoff + x;
        int sy = yoff + spans->y;
        if(sy >= 0 && sy < image_height && sx < image_width) {
            if(sx < 0) {
                x -= sx;
                length += sx;
                sx = 0;
            }

            if(sx + length > image_width)
                length = image_width - sx;
            if(length > 0) {
                const int coverage = (spans->coverage * texture->const_alpha) >> 8;
                const uint32_t* src = (const uint32_t*)(texture->data + sy * texture->stride) + sx;
                uint32_t* dest = (uint32_t*)(surface->data + spans->y * surface->stride) + x;
                func(dest, length, src, coverage);
            }
        }

        ++spans;
    }
}

#define FIXED_SCALE (1 << 16)
#define PI_F 3.14159265358979323846f

static inline uint32_t fetch_pixel_clamped(const texture_data_t* texture, int px, int py)
{
    if(px < 0) px = 0;
    else if(px >= texture->width) px = texture->width - 1;
    if(py < 0) py = 0;
    else if(py >= texture->height) py = texture->height - 1;
    return ((const uint32_t*)(texture->data + py * texture->stride))[px];
}

static inline int clamp_int(int val, int min_val, int max_val)
{
    if(val < min_val) return min_val;
    if(val > max_val) return max_val;
    return val;
}

// Sinc function: sin(pi*x) / (pi*x)
static inline float sinc(float x)
{
    if(fabsf(x) < 0.0001f) return 1.0f;
    float pix = PI_F * x;
    return sinf(pix) / pix;
}

// Lanczos kernel with parameter a (typically 2 or 3)
static inline float lanczos_weight(float x, int a)
{
    if(x == 0.0f) return 1.0f;
    if(fabsf(x) >= (float)a) return 0.0f;
    return sinc(x) * sinc(x / (float)a);
}

// Mitchell-Netravali filter weight
// B=1/3, C=1/3 - the "recommended" values by Mitchell & Netravali
// This filter is specifically designed for image resizing
static inline float mitchell_weight(float x)
{
    const float B = 1.0f / 3.0f;
    const float C = 1.0f / 3.0f;
    
    float ax = fabsf(x);
    
    if(ax < 1.0f) {
        return ((12.0f - 9.0f * B - 6.0f * C) * ax * ax * ax +
                (-18.0f + 12.0f * B + 6.0f * C) * ax * ax +
                (6.0f - 2.0f * B)) / 6.0f;
    } else if(ax < 2.0f) {
        return ((-B - 6.0f * C) * ax * ax * ax +
                (6.0f * B + 30.0f * C) * ax * ax +
                (-12.0f * B - 48.0f * C) * ax +
                (8.0f * B + 24.0f * C)) / 6.0f;
    }
    return 0.0f;
}

// ARGB color components stored as floats for interpolation
typedef struct {
    float a, r, g, b;
} color_components_t;

// Apply Mitchell-Netravali filter to a single sample point (4x4 kernel)
static inline color_components_t mitchell_filter_sample(
    const texture_data_t* texture, int sx, int sy)
{
    color_components_t result = {0, 0, 0, 0};
    float weight_total = 0;
    
    // Mitchell uses a 4x4 kernel (-1 to +2 in each direction)
    for(int bj = -1; bj <= 2; bj++) {
        float wy = mitchell_weight((float)bj);
        for(int bi = -1; bi <= 2; bi++) {
            float wx = mitchell_weight((float)bi);
            float weight = wx * wy;
            uint32_t pixel = fetch_pixel_clamped(texture, sx + bi, sy + bj);
            
            result.a += (float)((pixel >> 24) & 0xff) * weight;
            result.r += (float)((pixel >> 16) & 0xff) * weight;
            result.g += (float)((pixel >> 8) & 0xff) * weight;
            result.b += (float)(pixel & 0xff) * weight;
            weight_total += weight;
        }
    }
    
    if(weight_total > 0.0f) {
        result.a /= weight_total;
        result.r /= weight_total;
        result.g /= weight_total;
        result.b /= weight_total;
    }
    return result;
}

// Pack float color components to uint32_t ARGB
static inline uint32_t pack_argb(float a, float r, float g, float b)
{
    int ia = clamp_int((int)roundf(a), 0, 255);
    int ir = clamp_int((int)roundf(r), 0, 255);
    int ig = clamp_int((int)roundf(g), 0, 255);
    int ib = clamp_int((int)roundf(b), 0, 255);
    return ((uint32_t)ia << 24) | ((uint32_t)ir << 16) | ((uint32_t)ig << 8) | (uint32_t)ib;
}

// Fast bilinear sampling for when we already have mipmaps
static inline uint32_t bilinear_sample(const texture_data_t* texture, int x_fixed, int y_fixed)
{
    float fx = (float)x_fixed / (float)FIXED_SCALE;
    float fy = (float)y_fixed / (float)FIXED_SCALE;
    
    int px = (int)floorf(fx);
    int py = (int)floorf(fy);
    
    // Check bounds
    if(px < -1 || px >= texture->width || py < -1 || py >= texture->height) {
        return 0x00000000;
    }
    
    float tx = fx - (float)px;
    float ty = fy - (float)py;
    
    // Fetch 2x2 neighborhood
    uint32_t p00 = fetch_pixel_clamped(texture, px, py);
    uint32_t p10 = fetch_pixel_clamped(texture, px + 1, py);
    uint32_t p01 = fetch_pixel_clamped(texture, px, py + 1);
    uint32_t p11 = fetch_pixel_clamped(texture, px + 1, py + 1);
    
    // Bilinear interpolation using integer math (faster)
    int itx = (int)(tx * 256.0f);
    int ity = (int)(ty * 256.0f);
    int itx_inv = 256 - itx;
    int ity_inv = 256 - ity;
    
    // Interpolate each channel
    uint32_t a00 = (p00 >> 24) & 0xff, a10 = (p10 >> 24) & 0xff;
    uint32_t a01 = (p01 >> 24) & 0xff, a11 = (p11 >> 24) & 0xff;
    uint32_t r00 = (p00 >> 16) & 0xff, r10 = (p10 >> 16) & 0xff;
    uint32_t r01 = (p01 >> 16) & 0xff, r11 = (p11 >> 16) & 0xff;
    uint32_t g00 = (p00 >> 8) & 0xff, g10 = (p10 >> 8) & 0xff;
    uint32_t g01 = (p01 >> 8) & 0xff, g11 = (p11 >> 8) & 0xff;
    uint32_t b00 = p00 & 0xff, b10 = p10 & 0xff;
    uint32_t b01 = p01 & 0xff, b11 = p11 & 0xff;
    
    // Top row interpolation, then bottom, then vertical
    uint32_t a_top = (a00 * itx_inv + a10 * itx) >> 8;
    uint32_t a_bot = (a01 * itx_inv + a11 * itx) >> 8;
    uint32_t a = (a_top * ity_inv + a_bot * ity) >> 8;
    
    uint32_t r_top = (r00 * itx_inv + r10 * itx) >> 8;
    uint32_t r_bot = (r01 * itx_inv + r11 * itx) >> 8;
    uint32_t r = (r_top * ity_inv + r_bot * ity) >> 8;
    
    uint32_t g_top = (g00 * itx_inv + g10 * itx) >> 8;
    uint32_t g_bot = (g01 * itx_inv + g11 * itx) >> 8;
    uint32_t g = (g_top * ity_inv + g_bot * ity) >> 8;
    
    uint32_t b_top = (b00 * itx_inv + b10 * itx) >> 8;
    uint32_t b_bot = (b01 * itx_inv + b11 * itx) >> 8;
    uint32_t b = (b_top * ity_inv + b_bot * ity) >> 8;
    
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Lanczos-3 sampling (6x6 kernel) - good balance of quality and speed
// Used only for final sampling after progressive mipmapping has reduced scale to <2x
static inline uint32_t lanczos3_sample(const texture_data_t* texture, int x_fixed, int y_fixed)
{
    const int KERNEL_SIZE = 6;
    const int KERNEL_OFFSET = 2;
    const int LANCZOS_A = 3;
    
    float fx = (float)x_fixed / (float)FIXED_SCALE;
    float fy = (float)y_fixed / (float)FIXED_SCALE;
    
    int px = (int)floorf(fx);
    int py = (int)floorf(fy);
    
    // Check if completely outside the image
    if(px < -(KERNEL_OFFSET + 1) || px >= texture->width + KERNEL_OFFSET ||
       py < -(KERNEL_OFFSET + 1) || py >= texture->height + KERNEL_OFFSET) {
        return 0x00000000;
    }
    
    float tx = fx - (float)px;
    float ty = fy - (float)py;
    
    // Precompute Lanczos weights
    float lanczos_wx[6], lanczos_wy[6];
    float lx_sum = 0, ly_sum = 0;
    for(int i = 0; i < KERNEL_SIZE; i++) {
        lanczos_wx[i] = lanczos_weight(tx - (float)(i - KERNEL_OFFSET), LANCZOS_A);
        lanczos_wy[i] = lanczos_weight(ty - (float)(i - KERNEL_OFFSET), LANCZOS_A);
        lx_sum += lanczos_wx[i];
        ly_sum += lanczos_wy[i];
    }
    
    // Normalize Lanczos weights
    if(lx_sum > 0.0f) {
        float inv = 1.0f / lx_sum;
        for(int i = 0; i < KERNEL_SIZE; i++) lanczos_wx[i] *= inv;
    }
    if(ly_sum > 0.0f) {
        float inv = 1.0f / ly_sum;
        for(int i = 0; i < KERNEL_SIZE; i++) lanczos_wy[i] *= inv;
    }
    
    // Apply Lanczos interpolation
    float out_a = 0, out_r = 0, out_g = 0, out_b = 0;
    
    for(int j = 0; j < KERNEL_SIZE; j++) {
        int sy = py + j - KERNEL_OFFSET;
        float wy = lanczos_wy[j];
        for(int i = 0; i < KERNEL_SIZE; i++) {
            int sx = px + i - KERNEL_OFFSET;
            uint32_t pixel = fetch_pixel_clamped(texture, sx, sy);
            float weight = lanczos_wx[i] * wy;
            out_a += (float)((pixel >> 24) & 0xff) * weight;
            out_r += (float)((pixel >> 16) & 0xff) * weight;
            out_g += (float)((pixel >> 8) & 0xff) * weight;
            out_b += (float)(pixel & 0xff) * weight;
        }
    }
    
    return pack_argb(out_a, out_r, out_g, out_b);
}

// Calculate the scale ratio from transformation matrix
// Returns the approximate magnification factor (> 1.0 means downscaling source to smaller dest)
static inline float get_scale_ratio(const texture_data_t* texture)
{
    // The matrix transforms destination coords to source coords
    // So scale = sqrt(a*a + b*b) gives us how many source pixels per dest pixel
    float scale_x = sqrtf(texture->matrix.a * texture->matrix.a + texture->matrix.b * texture->matrix.b);
    float scale_y = sqrtf(texture->matrix.c * texture->matrix.c + texture->matrix.d * texture->matrix.d);
    float scale = (scale_x + scale_y) * 0.5f;  // Average scale
    
    // Sanity check - avoid issues with degenerate matrices
    if(scale < 0.001f) scale = 0.001f;
    if(scale > 1000.0f) scale = 1000.0f;
    
    return scale;
}

// Fast 2x2 box filter for mipmap generation (much faster than full Mitchell-Lanczos)
static inline uint32_t box_filter_2x2(const texture_data_t* texture, int sx, int sy)
{
    // Get 2x2 block of pixels
    uint32_t p00 = fetch_pixel_clamped(texture, sx, sy);
    uint32_t p10 = fetch_pixel_clamped(texture, sx + 1, sy);
    uint32_t p01 = fetch_pixel_clamped(texture, sx, sy + 1);
    uint32_t p11 = fetch_pixel_clamped(texture, sx + 1, sy + 1);
    
    // Average each channel
    uint32_t a = (((p00 >> 24) & 0xff) + ((p10 >> 24) & 0xff) + 
                  ((p01 >> 24) & 0xff) + ((p11 >> 24) & 0xff) + 2) >> 2;
    uint32_t r = (((p00 >> 16) & 0xff) + ((p10 >> 16) & 0xff) + 
                  ((p01 >> 16) & 0xff) + ((p11 >> 16) & 0xff) + 2) >> 2;
    uint32_t g = (((p00 >> 8) & 0xff) + ((p10 >> 8) & 0xff) + 
                  ((p01 >> 8) & 0xff) + ((p11 >> 8) & 0xff) + 2) >> 2;
    uint32_t b = ((p00 & 0xff) + (p10 & 0xff) + 
                  (p01 & 0xff) + (p11 & 0xff) + 2) >> 2;
    
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Create a downscaled version of a texture using fast box filter
// This is optimized for 2x downscaling in mipmap chain
static plutovg_surface_t* create_downscaled_surface(const texture_data_t* src_texture, float scale_factor)
{
    // Sanity checks to prevent hangs or crashes
    if(scale_factor < 1.0f) scale_factor = 1.0f;  // Never upscale
    if(scale_factor > 16.0f) scale_factor = 16.0f;  // Limit max reduction per pass
    
    int new_width = (int)(src_texture->width / scale_factor);
    int new_height = (int)(src_texture->height / scale_factor);
    
    // Ensure minimum size
    if(new_width < 1) new_width = 1;
    if(new_height < 1) new_height = 1;
    
    // Don't create surface if it's the same size or larger
    if(new_width >= src_texture->width && new_height >= src_texture->height) {
        return NULL;
    }
    
    plutovg_surface_t* surface = plutovg_surface_create(new_width, new_height);
    if(!surface) return NULL;
    
    // Sample each pixel using fast box filter
    uint32_t* dest_data = (uint32_t*)surface->data;
    int dest_stride = surface->stride / 4;
    
    // For 2x downscale, use optimized 2x2 box filter
    if(scale_factor >= 1.9f && scale_factor <= 2.1f) {
        for(int y = 0; y < new_height; y++) {
            int sy = y * 2;
            uint32_t* dest_row = dest_data + y * dest_stride;
            for(int x = 0; x < new_width; x++) {
                int sx = x * 2;
                dest_row[x] = box_filter_2x2(src_texture, sx, sy);
            }
        }
    } else {
        // For other scale factors, use area averaging
        int scale_int = (int)ceilf(scale_factor);
        float inv_area = 1.0f / (scale_factor * scale_factor);
        
        for(int y = 0; y < new_height; y++) {
            float src_y = y * scale_factor;
            int sy = (int)src_y;
            uint32_t* dest_row = dest_data + y * dest_stride;
            
            for(int x = 0; x < new_width; x++) {
                float src_x = x * scale_factor;
                int sx = (int)src_x;
                
                // Sum pixels in the area
                float sum_a = 0, sum_r = 0, sum_g = 0, sum_b = 0;
                int count = 0;
                for(int j = 0; j < scale_int && sy + j < src_texture->height; j++) {
                    for(int i = 0; i < scale_int && sx + i < src_texture->width; i++) {
                        uint32_t p = fetch_pixel_clamped(src_texture, sx + i, sy + j);
                        sum_a += (float)((p >> 24) & 0xff);
                        sum_r += (float)((p >> 16) & 0xff);
                        sum_g += (float)((p >> 8) & 0xff);
                        sum_b += (float)(p & 0xff);
                        count++;
                    }
                }
                
                if(count > 0) {
                    float inv_count = 1.0f / (float)count;
                    dest_row[x] = pack_argb(sum_a * inv_count, sum_r * inv_count, 
                                            sum_g * inv_count, sum_b * inv_count);
                } else {
                    dest_row[x] = 0;
                }
            }
        }
    }
    
    return surface;
}

// Progressive downscaling - creates mipmap-style intermediate levels
// Returns the final intermediate texture to sample from, or NULL to use original
typedef struct {
    plutovg_surface_t* surfaces[4];  // Up to 4 intermediate levels
    int num_levels;
    texture_data_t final_texture;
    float adjusted_matrix_scale;
} progressive_mipmap_t;

static void init_progressive_mipmap(progressive_mipmap_t* mipmap, const texture_data_t* texture)
{
    mipmap->num_levels = 0;
    mipmap->final_texture = *texture;
    mipmap->adjusted_matrix_scale = 1.0f;
    
    for(int i = 0; i < 4; i++) {
        mipmap->surfaces[i] = NULL;
    }
    
    float scale = get_scale_ratio(texture);
    
    // Only use progressive downscaling if scale > 2.0 (significant downscale)
    // Also check for valid texture dimensions
    if(scale <= 2.0f || texture->width < 2 || texture->height < 2) {
        return;
    }
    
    // Calculate number of passes needed (each pass reduces by 2x)
    // For scale of 8x, we need about 3 passes: 8->4->2->final
    float remaining_scale = scale;
    float accumulated_scale = 1.0f;
    int max_iterations = 10;  // Safety limit to prevent infinite loops
    
    while(remaining_scale > 2.0f && mipmap->num_levels < 4 && max_iterations-- > 0) {
        // Each pass reduces by exactly 2x
        float pass_scale = 2.0f;
        
        // Ensure pass_scale is valid (>= 1.0)
        if(pass_scale < 1.0f) pass_scale = 1.0f;
        
        accumulated_scale *= pass_scale;
        
        // Create intermediate surface from previous level or original
        texture_data_t src_tex;
        if(mipmap->num_levels == 0) {
            src_tex = *texture;
        } else {
            plutovg_surface_t* prev_surface = mipmap->surfaces[mipmap->num_levels - 1];
            if(!prev_surface) break;  // Safety check
            
            src_tex.data = prev_surface->data;
            src_tex.width = prev_surface->width;
            src_tex.height = prev_surface->height;
            src_tex.stride = prev_surface->stride;
            src_tex.const_alpha = texture->const_alpha;
            plutovg_matrix_init_identity(&src_tex.matrix);
        }
        
        // Check if source is too small to downsample further
        if(src_tex.width < 4 || src_tex.height < 4) {
            break;
        }
        
        mipmap->surfaces[mipmap->num_levels] = create_downscaled_surface(&src_tex, pass_scale);
        if(!mipmap->surfaces[mipmap->num_levels]) {
            break;  // Surface creation failed
        }
        
        mipmap->num_levels++;
        remaining_scale /= pass_scale;
        
        // Safety: if remaining_scale isn't decreasing, break to prevent infinite loop
        if(remaining_scale >= scale) {
            break;
        }
    }
    
    // Set up final texture to sample from
    if(mipmap->num_levels > 0) {
        plutovg_surface_t* final_surface = mipmap->surfaces[mipmap->num_levels - 1];
        mipmap->final_texture.data = final_surface->data;
        mipmap->final_texture.width = final_surface->width;
        mipmap->final_texture.height = final_surface->height;
        mipmap->final_texture.stride = final_surface->stride;
        
        // Adjust the matrix to account for the pre-scaling
        mipmap->final_texture.matrix = texture->matrix;
        mipmap->final_texture.matrix.a /= accumulated_scale;
        mipmap->final_texture.matrix.b /= accumulated_scale;
        mipmap->final_texture.matrix.c /= accumulated_scale;
        mipmap->final_texture.matrix.d /= accumulated_scale;
        mipmap->final_texture.matrix.e /= accumulated_scale;
        mipmap->final_texture.matrix.f /= accumulated_scale;
        
        mipmap->adjusted_matrix_scale = accumulated_scale;
    }
}

static void destroy_progressive_mipmap(progressive_mipmap_t* mipmap)
{
    for(int i = 0; i < mipmap->num_levels; i++) {
        if(mipmap->surfaces[i]) {
            plutovg_surface_destroy(mipmap->surfaces[i]);
            mipmap->surfaces[i] = NULL;
        }
    }
    mipmap->num_levels = 0;
}

static void blend_transformed_argb(plutovg_surface_t* surface, plutovg_operator_t op, const texture_data_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];
    uint32_t buffer[BUFFER_SIZE];
    
    // Initialize progressive mipmap for large downscales
    progressive_mipmap_t mipmap;
    init_progressive_mipmap(&mipmap, texture);
    
    // Use the potentially pre-scaled texture
    const texture_data_t* sample_texture = &mipmap.final_texture;
    
    // Determine remaining scale after mipmapping
    float remaining_scale = get_scale_ratio(sample_texture);
    
    // Choose sampling method based on remaining scale
    // If mipmaps reduced scale to near 1:1, use fast bilinear
    // Otherwise use Lanczos-3 for quality
    int use_bilinear = (remaining_scale < 1.5f);

    int fdx = (int)(sample_texture->matrix.a * FIXED_SCALE);
    int fdy = (int)(sample_texture->matrix.b * FIXED_SCALE);

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + spans->x;

        const float cx = spans->x + 0.5f;
        const float cy = spans->y + 0.5f;

        int x = (int)((sample_texture->matrix.c * cy + sample_texture->matrix.a * cx + sample_texture->matrix.e) * FIXED_SCALE);
        int y = (int)((sample_texture->matrix.d * cy + sample_texture->matrix.b * cx + sample_texture->matrix.f) * FIXED_SCALE);

        int length = spans->len;
        const int coverage = (spans->coverage * sample_texture->const_alpha) >> 8;
        while(length) {
            int l = plutovg_min(length, BUFFER_SIZE);
            const uint32_t* end = buffer + l;
            uint32_t* b = buffer;
            
            if(use_bilinear) {
                while(b < end) {
                    *b = bilinear_sample(sample_texture, x, y);
                    x += fdx;
                    y += fdy;
                    ++b;
                }
            } else {
                while(b < end) {
                    *b = lanczos3_sample(sample_texture, x, y);
                    x += fdx;
                    y += fdy;
                    ++b;
                }
            }

            func(target, l, buffer, coverage);
            target += l;
            length -= l;
        }

        ++spans;
    }
    
    // Clean up intermediate surfaces
    destroy_progressive_mipmap(&mipmap);
}

static void blend_untransformed_tiled_argb(plutovg_surface_t* surface, plutovg_operator_t op, const texture_data_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];

    int image_width = texture->width;
    int image_height = texture->height;

    int xoff = (int)(texture->matrix.e) % image_width;
    int yoff = (int)(texture->matrix.f) % image_height;

    if(xoff < 0)
        xoff += image_width;
    if(yoff < 0) {
        yoff += image_height;
    }

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        int x = spans->x;
        int length = spans->len;
        int sx = (xoff + spans->x) % image_width;
        int sy = (spans->y + yoff) % image_height;
        if(sx < 0)
            sx += image_width;
        if(sy < 0) {
            sy += image_height;
        }

        const int coverage = (spans->coverage * texture->const_alpha) >> 8;
        while(length) {
            int l = plutovg_min(image_width - sx, length);
            if(BUFFER_SIZE < l)
                l = BUFFER_SIZE;
            const uint32_t* src = (const uint32_t*)(texture->data + sy * texture->stride) + sx;
            uint32_t* dest = (uint32_t*)(surface->data + spans->y * surface->stride) + x;
            func(dest, l, src, coverage);
            x += l;
            sx += l;
            length -= l;
            if(sx >= image_width) {
                sx = 0;
            }
        }

        ++spans;
    }
}

static void blend_transformed_tiled_argb(plutovg_surface_t* surface, plutovg_operator_t op, const texture_data_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];
    uint32_t buffer[BUFFER_SIZE];

    int image_width = texture->width;
    int image_height = texture->height;
    const int scanline_offset = texture->stride / 4;

    int fdx = (int)(texture->matrix.a * FIXED_SCALE);
    int fdy = (int)(texture->matrix.b * FIXED_SCALE);

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + spans->x;
        const uint32_t* image_bits = (const uint32_t*)texture->data;

        const float cx = spans->x + 0.5f;
        const float cy = spans->y + 0.5f;

        int x = (int)((texture->matrix.c * cy + texture->matrix.a * cx + texture->matrix.e) * FIXED_SCALE);
        int y = (int)((texture->matrix.d * cy + texture->matrix.b * cx + texture->matrix.f) * FIXED_SCALE);

        const int coverage = (spans->coverage * texture->const_alpha) >> 8;
        int length = spans->len;
        while(length) {
            int l = plutovg_min(length, BUFFER_SIZE);
            const uint32_t* end = buffer + l;
            uint32_t* b = buffer;
            while(b < end) {
                int px = x >> 16;
                int py = y >> 16;
                px %= image_width;
                py %= image_height;
                if(px < 0) px += image_width;
                if(py < 0) py += image_height;
                int y_offset = py * scanline_offset;

                assert(px >= 0 && px < image_width);
                assert(py >= 0 && py < image_height);

                *b = image_bits[y_offset + px];
                x += fdx;
                y += fdy;
                ++b;
            }

            func(target, l, buffer, coverage);
            target += l;
            length -= l;
        }

        ++spans;
    }
}

static inline uint32_t interpolate_4_pixels(uint32_t tl, uint32_t tr, uint32_t bl, uint32_t br, uint32_t distx, uint32_t disty)
{
    uint32_t idistx = 256 - distx;
    uint32_t idisty = 256 - disty;
    uint32_t xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
    uint32_t xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
    return INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
}

#define HALF_POINT (1 << 15)
static void blend_transformed_bilinear_tiled_argb(plutovg_surface_t* surface, plutovg_operator_t op, const texture_data_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    composition_function_t func = composition_table[op];
    uint32_t buffer[BUFFER_SIZE];

    int image_width = texture->width;
    int image_height = texture->height;

    int fdx = (int)(texture->matrix.a * FIXED_SCALE);
    int fdy = (int)(texture->matrix.b * FIXED_SCALE);

    int count = span_buffer->spans.size;
    const plutovg_span_t* spans = span_buffer->spans.data;
    while(count--) {
        uint32_t* target = (uint32_t*)(surface->data + spans->y * surface->stride) + spans->x;

        const float cx = spans->x + 0.5f;
        const float cy = spans->y + 0.5f;

        int fx = (int)((texture->matrix.c * cy + texture->matrix.a * cx + texture->matrix.e) * FIXED_SCALE);
        int fy = (int)((texture->matrix.d * cy + texture->matrix.b * cx + texture->matrix.f) * FIXED_SCALE);

        fx -= HALF_POINT;
        fy -= HALF_POINT;

        const int coverage = (spans->coverage * texture->const_alpha) >> 8;
        int length = spans->len;
        while(length) {
            int l = plutovg_min(length, BUFFER_SIZE);
            const uint32_t* end = buffer + l;
            uint32_t* b = buffer;
            while (b < end) {
                int x1 = (fx >> 16) % image_width;
                int y1 = (fy >> 16) % image_height;

                if(x1 < 0) x1 += image_width;
                if(y1 < 0) y1 += image_height;

                int x2 = (x1 + 1) % image_width;
                int y2 = (y1 + 1) % image_height;

                const uint32_t* s1 = (const uint32_t*)(texture->data + y1 * texture->stride);
                const uint32_t* s2 = (const uint32_t*)(texture->data + y2 * texture->stride);

                uint32_t tl = s1[x1];
                uint32_t tr = s1[x2];
                uint32_t bl = s2[x1];
                uint32_t br = s2[x2];

                int distx = (fx & 0x0000ffff) >> 8;
                int disty = (fy & 0x0000ffff) >> 8;
                *b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);

                fx += fdx;
                fy += fdy;
                ++b;
            }

            func(target, l, buffer, coverage);
            target += l;
            length -= l;
        }

        ++spans;
    }
}

static void plutovg_blend_color(plutovg_canvas_t* canvas, const plutovg_color_t* color, const plutovg_span_buffer_t* span_buffer)
{
    plutovg_state_t* state = canvas->state;
    uint32_t solid = premultiply_color_with_opacity(color, state->opacity);
    uint32_t alpha = plutovg_alpha(solid);

    if(alpha == 255 && state->op == PLUTOVG_OPERATOR_SRC_OVER) {
        blend_solid(canvas->surface, PLUTOVG_OPERATOR_SRC, solid, span_buffer);
    } else {
        blend_solid(canvas->surface, state->op, solid, span_buffer);
    }
}

static void plutovg_blend_gradient(plutovg_canvas_t* canvas, const plutovg_gradient_paint_t* gradient, const plutovg_span_buffer_t* span_buffer)
{
    if(gradient->nstops == 0)
        return;
    plutovg_state_t* state = canvas->state;
    gradient_data_t data;
    data.spread = gradient->spread;
    data.matrix = gradient->matrix;
    plutovg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
    if(!plutovg_matrix_invert(&data.matrix, &data.matrix))
        return;
    int i, pos = 0, nstops = gradient->nstops;
    const plutovg_gradient_stop_t *curr, *next, *start, *last;
    uint32_t curr_color, next_color, last_color;
    uint32_t dist, idist;
    float delta, t, incr, fpos;
    float opacity = state->opacity;

    start = gradient->stops;
    curr = start;
    curr_color = premultiply_color_with_opacity(&curr->color, opacity);

    data.colortable[pos++] = curr_color;
    incr = 1.0f / COLOR_TABLE_SIZE;
    fpos = 1.5f * incr;

    while(fpos <= curr->offset) {
        data.colortable[pos] = data.colortable[pos - 1];
        ++pos;
        fpos += incr;
    }

    for(i = 0; i < nstops - 1; i++) {
        curr = (start + i);
        next = (start + i + 1);
        if(curr->offset == next->offset)
            continue;
        delta = 1.f / (next->offset - curr->offset);
        next_color = premultiply_color_with_opacity(&next->color, opacity);
        while(fpos < next->offset && pos < COLOR_TABLE_SIZE) {
            t = (fpos - curr->offset) * delta;
            dist = (uint32_t)(255 * t);
            idist = 255 - dist;
            data.colortable[pos] = INTERPOLATE_PIXEL_255(curr_color, idist, next_color, dist);
            ++pos;
            fpos += incr;
        }

        curr_color = next_color;
    }

    last = start + nstops - 1;
    last_color = premultiply_color_with_opacity(&last->color, opacity);
    for(; pos < COLOR_TABLE_SIZE; ++pos) {
        data.colortable[pos] = last_color;
    }

    if(gradient->type == PLUTOVG_GRADIENT_TYPE_LINEAR) {
        data.values.linear.x1 = gradient->values[0];
        data.values.linear.y1 = gradient->values[1];
        data.values.linear.x2 = gradient->values[2];
        data.values.linear.y2 = gradient->values[3];
        blend_linear_gradient(canvas->surface, state->op, &data, span_buffer);
    } else {
        data.values.radial.cx = gradient->values[0];
        data.values.radial.cy = gradient->values[1];
        data.values.radial.cr = gradient->values[2];
        data.values.radial.fx = gradient->values[3];
        data.values.radial.fy = gradient->values[4];
        data.values.radial.fr = gradient->values[5];
        blend_radial_gradient(canvas->surface, state->op, &data, span_buffer);
    }
}

static void plutovg_blend_texture(plutovg_canvas_t* canvas, const plutovg_texture_paint_t* texture, const plutovg_span_buffer_t* span_buffer)
{
    if(texture->surface == NULL)
        return;
    plutovg_state_t* state = canvas->state;
    texture_data_t data;
    data.matrix = texture->matrix;
    data.data = texture->surface->data;
    data.width = texture->surface->width;
    data.height = texture->surface->height;
    data.stride = texture->surface->stride;
    data.const_alpha = lroundf(state->opacity * texture->opacity * 256);

    plutovg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
    if(!plutovg_matrix_invert(&data.matrix, &data.matrix))
        return;
    const plutovg_matrix_t* matrix = &data.matrix;
    if(matrix->a == 1 && matrix->b == 0 && matrix->c == 0 && matrix->d == 1) {
        if(texture->type == PLUTOVG_TEXTURE_TYPE_PLAIN) {
            blend_untransformed_argb(canvas->surface, state->op, &data, span_buffer);
        } else {
            blend_untransformed_tiled_argb(canvas->surface, state->op, &data, span_buffer);
        }
    } else {
        if(texture->type == PLUTOVG_TEXTURE_TYPE_PLAIN) {
            blend_transformed_argb(canvas->surface, state->op, &data, span_buffer);
        } else if(fabsf(matrix->b) > 1e-6f || fabsf(matrix->c) > 1e-6f) {
            blend_transformed_bilinear_tiled_argb(canvas->surface, state->op, &data, span_buffer);
        } else {
            blend_transformed_tiled_argb(canvas->surface, state->op, &data, span_buffer);
        }
    }
}

void plutovg_blend(plutovg_canvas_t* canvas, const plutovg_span_buffer_t* span_buffer)
{
    if(span_buffer->spans.size == 0)
        return;
    if(canvas->state->paint == NULL) {
        plutovg_blend_color(canvas, &canvas->state->color, span_buffer);
        return;
    }

    plutovg_paint_t* paint = canvas->state->paint;
    if(paint->type == PLUTOVG_PAINT_TYPE_COLOR) {
        plutovg_solid_paint_t* solid = (plutovg_solid_paint_t*)(paint);
        plutovg_blend_color(canvas, &solid->color, span_buffer);
    } else if(paint->type == PLUTOVG_PAINT_TYPE_GRADIENT) {
        plutovg_gradient_paint_t* gradient = (plutovg_gradient_paint_t*)(paint);
        plutovg_blend_gradient(canvas, gradient, span_buffer);
    } else {
        plutovg_texture_paint_t* texture = (plutovg_texture_paint_t*)(paint);
        plutovg_blend_texture(canvas, texture, span_buffer);
    }
}
