#include "plutovg-filter.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

static inline uint32_t premultiply_pixel(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    r = (r * a + 127) / 255;
    g = (g * a + 127) / 255;
    b = (b * a + 127) / 255;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static inline uint32_t clamp255(int v)
{
    return v < 0 ? 0 : (v > 255 ? 255 : (uint32_t)v);
}

void plutovg_filter_flood(
    plutovg_surface_t* dst,
    float r, float g, float b, float a,
    int x, int y, int width, int height)
{
    uint32_t ir = clamp255((int)(r * 255.f + 0.5f));
    uint32_t ig = clamp255((int)(g * 255.f + 0.5f));
    uint32_t ib = clamp255((int)(b * 255.f + 0.5f));
    uint32_t ia = clamp255((int)(a * 255.f + 0.5f));
    uint32_t pixel = premultiply_pixel(ir, ig, ib, ia);

    int dst_width = plutovg_surface_get_width(dst);
    int dst_height = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst);
    unsigned char* dst_data = plutovg_surface_get_data(dst);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_width ? dst_width : (x + width);
    int y1 = (y + height) > dst_height ? dst_height : (y + height);

    for(int row = y0; row < y1; ++row) {
        uint32_t* dst_row = (uint32_t*)(dst_data + row * dst_stride);
        for(int col = x0; col < x1; ++col) {
            dst_row[col] = pixel;
        }
    }
}

void plutovg_filter_offset(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int dx, int dy,
    int x, int y, int width, int height)
{
    int dst_width = plutovg_surface_get_width(dst);
    int dst_height = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst);
    unsigned char* dst_data = plutovg_surface_get_data(dst);

    int src_width = plutovg_surface_get_width(src);
    int src_height = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src);
    const unsigned char* src_data = plutovg_surface_get_data(src);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_width ? dst_width : (x + width);
    int y1 = (y + height) > dst_height ? dst_height : (y + height);

    // Clear destination region
    for(int row = y0; row < y1; ++row) {
        memset(dst_data + row * dst_stride + x0 * 4, 0, (x1 - x0) * 4);
    }

    // Copy offset pixels
    for(int row = y0; row < y1; ++row) {
        int src_row = row - dy;
        if(src_row < 0 || src_row >= src_height)
            continue;
        uint32_t* dst_pixels = (uint32_t*)(dst_data + row * dst_stride);
        const uint32_t* src_pixels = (const uint32_t*)(src_data + src_row * src_stride);
        for(int col = x0; col < x1; ++col) {
            int src_col = col - dx;
            if(src_col < 0 || src_col >= src_width)
                continue;
            dst_pixels[col] = src_pixels[src_col];
        }
    }
}

void plutovg_filter_merge(
    plutovg_surface_t* dst,
    const plutovg_surface_t** inputs, int inputCount,
    int x, int y, int width, int height)
{
    int dst_width = plutovg_surface_get_width(dst);
    int dst_height = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst);
    unsigned char* dst_data = plutovg_surface_get_data(dst);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_width ? dst_width : (x + width);
    int y1 = (y + height) > dst_height ? dst_height : (y + height);

    // Clear destination region
    for(int row = y0; row < y1; ++row) {
        memset(dst_data + row * dst_stride + x0 * 4, 0, (x1 - x0) * 4);
    }

    // Composite each input using source-over (premultiplied alpha)
    for(int i = 0; i < inputCount; ++i) {
        const plutovg_surface_t* src = inputs[i];
        if(!src)
            continue;

        int src_width = plutovg_surface_get_width(src);
        int src_height = plutovg_surface_get_height(src);
        int src_stride = plutovg_surface_get_stride(src);
        const unsigned char* src_data = plutovg_surface_get_data(src);

        for(int row = y0; row < y1; ++row) {
            if(row < 0 || row >= src_height)
                continue;
            uint32_t* dp = (uint32_t*)(dst_data + row * dst_stride);
            const uint32_t* sp = (const uint32_t*)(src_data + row * src_stride);
            for(int col = x0; col < x1; ++col) {
                if(col < 0 || col >= src_width)
                    continue;
                uint32_t s = sp[col];
                uint32_t d = dp[col];

                uint32_t sa = (s >> 24) & 0xFF;
                if(sa == 0)
                    continue;
                if(sa == 255) {
                    dp[col] = s;
                    continue;
                }

                // source-over: dst = src + dst * (1 - src_alpha)
                uint32_t sr = (s >> 16) & 0xFF;
                uint32_t sg = (s >> 8) & 0xFF;
                uint32_t sb = s & 0xFF;
                uint32_t da = (d >> 24) & 0xFF;
                uint32_t dr = (d >> 16) & 0xFF;
                uint32_t dg = (d >> 8) & 0xFF;
                uint32_t db = d & 0xFF;

                uint32_t inv_sa = 255 - sa;
                uint32_t oa = sa + ((da * inv_sa + 127) / 255);
                uint32_t or_ = sr + ((dr * inv_sa + 127) / 255);
                uint32_t og = sg + ((dg * inv_sa + 127) / 255);
                uint32_t ob = sb + ((db * inv_sa + 127) / 255);

                dp[col] = (clamp255(oa) << 24) | (clamp255(or_) << 16) | (clamp255(og) << 8) | clamp255(ob);
            }
        }
    }
}

// --- Gaussian Blur (box-blur approximation, 3-pass) ---

// Box blur uses a sliding window sum. 3 passes of box blur approximates Gaussian.
// We compute the box radius from stdDeviation per the SVG spec:
//   d = floor(s * 3 * sqrt(2*PI) / 4 + 0.5)
// Then 3 passes of box blur with that radius.

static void box_blur_h(const uint32_t* src, uint32_t* dst, int w, int h, int src_stride, int dst_stride, int radius)
{
    if(radius <= 0) {
        for(int y = 0; y < h; ++y)
            memcpy(dst + y * dst_stride, src + y * src_stride, w * 4);
        return;
    }
    float iarr = 1.f / (float)(radius + radius + 1);
    for(int y = 0; y < h; ++y) {
        const uint32_t* sp = src + y * src_stride;
        uint32_t* dp = dst + y * dst_stride;

        int ti = 0;
        int li = 0;
        int ri = radius;

        uint32_t fv = sp[0];
        uint32_t lv = sp[w - 1];

        int32_t val_r = (int32_t)((fv >> 16) & 0xFF) * (radius + 1);
        int32_t val_g = (int32_t)((fv >> 8) & 0xFF) * (radius + 1);
        int32_t val_b = (int32_t)(fv & 0xFF) * (radius + 1);
        int32_t val_a = (int32_t)((fv >> 24) & 0xFF) * (radius + 1);

        for(int j = 0; j < radius; ++j) {
            int idx = j < w ? j : w - 1;
            uint32_t p = sp[idx];
            val_r += (int32_t)((p >> 16) & 0xFF);
            val_g += (int32_t)((p >> 8) & 0xFF);
            val_b += (int32_t)(p & 0xFF);
            val_a += (int32_t)((p >> 24) & 0xFF);
        }

        for(int j = 0; j <= radius; ++j) {
            int idx = ri < w ? ri : w - 1;
            uint32_t p = sp[idx];
            val_r += (int32_t)((p >> 16) & 0xFF) - (int32_t)((fv >> 16) & 0xFF);
            val_g += (int32_t)((p >> 8) & 0xFF) - (int32_t)((fv >> 8) & 0xFF);
            val_b += (int32_t)(p & 0xFF) - (int32_t)(fv & 0xFF);
            val_a += (int32_t)((p >> 24) & 0xFF) - (int32_t)((fv >> 24) & 0xFF);
            ri++;
            dp[ti++] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
        }

        for(int j = radius + 1; j < w - radius; ++j) {
            int ri_idx = ri < w ? ri : w - 1;
            int li_idx = li < w ? li : w - 1;
            uint32_t rp = sp[ri_idx];
            uint32_t lp = sp[li_idx];
            val_r += (int32_t)((rp >> 16) & 0xFF) - (int32_t)((lp >> 16) & 0xFF);
            val_g += (int32_t)((rp >> 8) & 0xFF) - (int32_t)((lp >> 8) & 0xFF);
            val_b += (int32_t)(rp & 0xFF) - (int32_t)(lp & 0xFF);
            val_a += (int32_t)((rp >> 24) & 0xFF) - (int32_t)((lp >> 24) & 0xFF);
            ri++;
            li++;
            dp[ti++] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
        }

        for(int j = w - radius; j < w; ++j) {
            int li_idx = li < w ? li : w - 1;
            uint32_t lp = sp[li_idx];
            val_r += (int32_t)((lv >> 16) & 0xFF) - (int32_t)((lp >> 16) & 0xFF);
            val_g += (int32_t)((lv >> 8) & 0xFF) - (int32_t)((lp >> 8) & 0xFF);
            val_b += (int32_t)(lv & 0xFF) - (int32_t)(lp & 0xFF);
            val_a += (int32_t)((lv >> 24) & 0xFF) - (int32_t)((lp >> 24) & 0xFF);
            li++;
            dp[ti++] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
        }
    }
}

static void box_blur_v(const uint32_t* src, uint32_t* dst, int w, int h, int src_stride, int dst_stride, int radius)
{
    if(radius <= 0) {
        for(int y = 0; y < h; ++y)
            memcpy(dst + y * dst_stride, src + y * src_stride, w * 4);
        return;
    }
    float iarr = 1.f / (float)(radius + radius + 1);
    for(int x = 0; x < w; ++x) {
        int ti = 0;
        int li = 0;
        int ri = radius;

        uint32_t fv = src[x];
        uint32_t lv = src[(h - 1) * src_stride + x];

        int32_t val_r = (int32_t)((fv >> 16) & 0xFF) * (radius + 1);
        int32_t val_g = (int32_t)((fv >> 8) & 0xFF) * (radius + 1);
        int32_t val_b = (int32_t)(fv & 0xFF) * (radius + 1);
        int32_t val_a = (int32_t)((fv >> 24) & 0xFF) * (radius + 1);

        for(int j = 0; j < radius; ++j) {
            int idx = j < h ? j : h - 1;
            uint32_t p = src[idx * src_stride + x];
            val_r += (int32_t)((p >> 16) & 0xFF);
            val_g += (int32_t)((p >> 8) & 0xFF);
            val_b += (int32_t)(p & 0xFF);
            val_a += (int32_t)((p >> 24) & 0xFF);
        }

        for(int j = 0; j <= radius; ++j) {
            int idx = ri < h ? ri : h - 1;
            uint32_t p = src[idx * src_stride + x];
            val_r += (int32_t)((p >> 16) & 0xFF) - (int32_t)((fv >> 16) & 0xFF);
            val_g += (int32_t)((p >> 8) & 0xFF) - (int32_t)((fv >> 8) & 0xFF);
            val_b += (int32_t)(p & 0xFF) - (int32_t)(fv & 0xFF);
            val_a += (int32_t)((p >> 24) & 0xFF) - (int32_t)((fv >> 24) & 0xFF);
            ri++;
            dst[ti * dst_stride + x] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
            ti++;
        }

        for(int j = radius + 1; j < h - radius; ++j) {
            int ri_idx = ri < h ? ri : h - 1;
            int li_idx = li < h ? li : h - 1;
            uint32_t rp = src[ri_idx * src_stride + x];
            uint32_t lp = src[li_idx * src_stride + x];
            val_r += (int32_t)((rp >> 16) & 0xFF) - (int32_t)((lp >> 16) & 0xFF);
            val_g += (int32_t)((rp >> 8) & 0xFF) - (int32_t)((lp >> 8) & 0xFF);
            val_b += (int32_t)(rp & 0xFF) - (int32_t)(lp & 0xFF);
            val_a += (int32_t)((rp >> 24) & 0xFF) - (int32_t)((lp >> 24) & 0xFF);
            ri++;
            li++;
            dst[ti * dst_stride + x] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
            ti++;
        }

        for(int j = h - radius; j < h; ++j) {
            int li_idx = li < h ? li : h - 1;
            uint32_t lp = src[li_idx * src_stride + x];
            val_r += (int32_t)((lv >> 16) & 0xFF) - (int32_t)((lp >> 16) & 0xFF);
            val_g += (int32_t)((lv >> 8) & 0xFF) - (int32_t)((lp >> 8) & 0xFF);
            val_b += (int32_t)(lv & 0xFF) - (int32_t)(lp & 0xFF);
            val_a += (int32_t)((lv >> 24) & 0xFF) - (int32_t)((lp >> 24) & 0xFF);
            li++;
            dst[ti * dst_stride + x] = ((uint32_t)(int32_t)(val_a * iarr + 0.5f) << 24)
                                     | ((uint32_t)(int32_t)(val_r * iarr + 0.5f) << 16)
                                     | ((uint32_t)(int32_t)(val_g * iarr + 0.5f) << 8)
                                     | (uint32_t)(int32_t)(val_b * iarr + 0.5f);
            ti++;
        }
    }
}

static int box_radius_for_gaussian(float stdDev)
{
    // SVG spec: d = floor(stdDev * 3 * sqrt(2*PI) / 4 + 0.5)
    // For 3-pass box blur, each pass uses radius = (d - 1) / 2
    if(stdDev <= 0.f)
        return 0;
    int d = (int)(stdDev * 3.f * 2.5066f / 4.f + 0.5f);
    if(d < 1) d = 1;
    // Make d odd for symmetric kernel
    if(d % 2 == 0) d++;
    return (d - 1) / 2;
}

void plutovg_filter_gaussian_blur(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float stdDevX, float stdDevY,
    int x, int y, int width, int height)
{
    int w = plutovg_surface_get_width(dst);
    int h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;

    // Copy src to dst first
    unsigned char* dst_data = plutovg_surface_get_data(dst);
    const unsigned char* src_data = plutovg_surface_get_data(src);
    int copyW = w < src_w ? w : src_w;
    int copyH = h < src_h ? h : src_h;
    memset(dst_data, 0, h * dst_stride * 4);
    for(int row = 0; row < copyH; ++row) {
        memcpy(dst_data + row * dst_stride * 4, src_data + row * src_stride * 4, copyW * 4);
    }

    int radiusX = box_radius_for_gaussian(stdDevX);
    int radiusY = box_radius_for_gaussian(stdDevY);

    if(radiusX <= 0 && radiusY <= 0)
        return;

    // Allocate temp buffer
    uint32_t* tmp = (uint32_t*)malloc(w * h * 4);
    if(!tmp)
        return;

    uint32_t* pixels = (uint32_t*)dst_data;

    // 3 passes of box blur
    for(int pass = 0; pass < 3; ++pass) {
        if(radiusX > 0) {
            box_blur_h(pixels, tmp, w, h, dst_stride, w, radiusX);
            box_blur_v(tmp, pixels, w, h, w, dst_stride, radiusY > 0 ? radiusY : 0);
        } else {
            box_blur_v(pixels, tmp, w, h, dst_stride, w, radiusY);
            // Copy back
            for(int row = 0; row < h; ++row)
                memcpy(pixels + row * dst_stride, tmp + row * w, w * 4);
        }
    }

    free(tmp);
}

// --- Blend ---

void plutovg_filter_blend(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    int mode,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int in1_w = plutovg_surface_get_width(in1);
    int in1_h = plutovg_surface_get_height(in1);
    int in1_stride = plutovg_surface_get_stride(in1) / 4;
    const uint32_t* in1_data = (const uint32_t*)plutovg_surface_get_data(in1);

    int in2_w = plutovg_surface_get_width(in2);
    int in2_h = plutovg_surface_get_height(in2);
    int in2_stride = plutovg_surface_get_stride(in2) / 4;
    const uint32_t* in2_data = (const uint32_t*)plutovg_surface_get_data(in2);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            uint32_t c1 = 0, c2 = 0;
            if(row < in1_h && col < in1_w)
                c1 = in1_data[row * in1_stride + col];
            if(row < in2_h && col < in2_w)
                c2 = in2_data[row * in2_stride + col];

            // Unpremultiply for blending
            uint32_t a1 = (c1 >> 24) & 0xFF;
            uint32_t r1 = a1 ? ((c1 >> 16) & 0xFF) * 255 / a1 : 0;
            uint32_t g1 = a1 ? ((c1 >> 8) & 0xFF) * 255 / a1 : 0;
            uint32_t b1 = a1 ? (c1 & 0xFF) * 255 / a1 : 0;

            uint32_t a2 = (c2 >> 24) & 0xFF;
            uint32_t r2 = a2 ? ((c2 >> 16) & 0xFF) * 255 / a2 : 0;
            uint32_t g2 = a2 ? ((c2 >> 8) & 0xFF) * 255 / a2 : 0;
            uint32_t b2 = a2 ? (c2 & 0xFF) * 255 / a2 : 0;

            uint32_t rr, rg, rb;
            switch(mode) {
            case 1: // multiply
                rr = r1 * r2 / 255;
                rg = g1 * g2 / 255;
                rb = b1 * b2 / 255;
                break;
            case 2: // screen
                rr = r1 + r2 - r1 * r2 / 255;
                rg = g1 + g2 - g1 * g2 / 255;
                rb = b1 + b2 - b1 * b2 / 255;
                break;
            case 3: // darken
                rr = r1 < r2 ? r1 : r2;
                rg = g1 < g2 ? g1 : g2;
                rb = b1 < b2 ? b1 : b2;
                break;
            case 4: // lighten
                rr = r1 > r2 ? r1 : r2;
                rg = g1 > g2 ? g1 : g2;
                rb = b1 > b2 ? b1 : b2;
                break;
            default: // 0 = normal (source-over)
                rr = r2;
                rg = g2;
                rb = b2;
                break;
            }

            // Composite: Fa = a1 + a2 - a1*a2/255
            uint32_t ra = a1 + a2 - a1 * a2 / 255;
            if(ra > 255) ra = 255;

            // Premultiply result
            dst_data[row * dst_stride + col] = premultiply_pixel(
                clamp255(rr), clamp255(rg), clamp255(rb), ra);
        }
    }
}

// --- Color Matrix ---

void plutovg_filter_color_matrix(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const float matrix[20],
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;
    const uint32_t* src_data = (const uint32_t*)plutovg_surface_get_data(src);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            uint32_t p = 0;
            if(row < src_h && col < src_w)
                p = src_data[row * src_stride + col];

            // Unpremultiply
            uint32_t a = (p >> 24) & 0xFF;
            float R, G, B, A;
            if(a == 0) {
                R = G = B = A = 0.f;
            } else {
                R = (float)((p >> 16) & 0xFF) / (float)a;
                G = (float)((p >> 8) & 0xFF) / (float)a;
                B = (float)(p & 0xFF) / (float)a;
                A = (float)a / 255.f;
            }

            // Apply 4x5 matrix (row-major): [R' G' B' A'] = M * [R G B A 1]
            float nr = matrix[0]*R + matrix[1]*G + matrix[2]*B + matrix[3]*A + matrix[4];
            float ng = matrix[5]*R + matrix[6]*G + matrix[7]*B + matrix[8]*A + matrix[9];
            float nb = matrix[10]*R + matrix[11]*G + matrix[12]*B + matrix[13]*A + matrix[14];
            float na = matrix[15]*R + matrix[16]*G + matrix[17]*B + matrix[18]*A + matrix[19];

            // Clamp to [0,1]
            if(nr < 0.f) nr = 0.f; if(nr > 1.f) nr = 1.f;
            if(ng < 0.f) ng = 0.f; if(ng > 1.f) ng = 1.f;
            if(nb < 0.f) nb = 0.f; if(nb > 1.f) nb = 1.f;
            if(na < 0.f) na = 0.f; if(na > 1.f) na = 1.f;

            // Premultiply and store
            uint32_t ia = clamp255((int)(na * 255.f + 0.5f));
            uint32_t ir = clamp255((int)(nr * 255.f + 0.5f));
            uint32_t ig = clamp255((int)(ng * 255.f + 0.5f));
            uint32_t ib = clamp255((int)(nb * 255.f + 0.5f));
            dst_data[row * dst_stride + col] = premultiply_pixel(ir, ig, ib, ia);
        }
    }
}

// --- Composite ---

void plutovg_filter_composite(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    int op, float k1, float k2, float k3, float k4,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int in1_w = plutovg_surface_get_width(in1);
    int in1_h = plutovg_surface_get_height(in1);
    int in1_stride = plutovg_surface_get_stride(in1) / 4;
    const uint32_t* in1_data = (const uint32_t*)plutovg_surface_get_data(in1);

    int in2_w = plutovg_surface_get_width(in2);
    int in2_h = plutovg_surface_get_height(in2);
    int in2_stride = plutovg_surface_get_stride(in2) / 4;
    const uint32_t* in2_data = (const uint32_t*)plutovg_surface_get_data(in2);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            uint32_t c1 = 0, c2 = 0;
            if(row < in1_h && col < in1_w)
                c1 = in1_data[row * in1_stride + col];
            if(row < in2_h && col < in2_w)
                c2 = in2_data[row * in2_stride + col];

            // All compositing operates on premultiplied values
            uint32_t r1 = (c1 >> 16) & 0xFF;
            uint32_t g1 = (c1 >> 8) & 0xFF;
            uint32_t b1 = c1 & 0xFF;
            uint32_t a1 = (c1 >> 24) & 0xFF;

            uint32_t r2 = (c2 >> 16) & 0xFF;
            uint32_t g2 = (c2 >> 8) & 0xFF;
            uint32_t b2 = c2 & 0xFF;
            uint32_t a2 = (c2 >> 24) & 0xFF;

            uint32_t rr, rg, rb, ra;

            switch(op) {
            case 0: // over: in1 over in2
            {
                uint32_t inv_a1 = 255 - a1;
                ra = a1 + ((a2 * inv_a1 + 127) / 255);
                rr = r1 + ((r2 * inv_a1 + 127) / 255);
                rg = g1 + ((g2 * inv_a1 + 127) / 255);
                rb = b1 + ((b2 * inv_a1 + 127) / 255);
                break;
            }
            case 1: // in: in1 in in2
                ra = (a1 * a2 + 127) / 255;
                rr = (r1 * a2 + 127) / 255;
                rg = (g1 * a2 + 127) / 255;
                rb = (b1 * a2 + 127) / 255;
                break;
            case 2: // out: in1 out in2
            {
                uint32_t inv_a2 = 255 - a2;
                ra = (a1 * inv_a2 + 127) / 255;
                rr = (r1 * inv_a2 + 127) / 255;
                rg = (g1 * inv_a2 + 127) / 255;
                rb = (b1 * inv_a2 + 127) / 255;
                break;
            }
            case 3: // atop: in1 atop in2
            {
                uint32_t inv_a1 = 255 - a1;
                ra = a2;
                rr = (r1 * a2 + r2 * inv_a1 + 127) / 255;
                rg = (g1 * a2 + g2 * inv_a1 + 127) / 255;
                rb = (b1 * a2 + b2 * inv_a1 + 127) / 255;
                break;
            }
            case 4: // xor
            {
                uint32_t inv_a1 = 255 - a1;
                uint32_t inv_a2 = 255 - a2;
                ra = (a1 * inv_a2 + a2 * inv_a1 + 127) / 255;
                rr = (r1 * inv_a2 + r2 * inv_a1 + 127) / 255;
                rg = (g1 * inv_a2 + g2 * inv_a1 + 127) / 255;
                rb = (b1 * inv_a2 + b2 * inv_a1 + 127) / 255;
                break;
            }
            case 5: // arithmetic: k1*i1*i2 + k2*i1 + k3*i2 + k4
            {
                float fr = k1 * r1 * r2 / 255.f + k2 * r1 + k3 * r2 + k4 * 255.f;
                float fg = k1 * g1 * g2 / 255.f + k2 * g1 + k3 * g2 + k4 * 255.f;
                float fb = k1 * b1 * b2 / 255.f + k2 * b1 + k3 * b2 + k4 * 255.f;
                float fa = k1 * a1 * a2 / 255.f + k2 * a1 + k3 * a2 + k4 * 255.f;
                ra = clamp255((int)(fa + 0.5f));
                rr = clamp255((int)(fr + 0.5f));
                rg = clamp255((int)(fg + 0.5f));
                rb = clamp255((int)(fb + 0.5f));
                break;
            }
            default:
                ra = rr = rg = rb = 0;
                break;
            }

            dst_data[row * dst_stride + col] = (clamp255(ra) << 24) | (clamp255(rr) << 16) | (clamp255(rg) << 8) | clamp255(rb);
        }
    }
}

void plutovg_filter_unpremultiply(uint32_t* pixels, int count)
{
    for(int i = 0; i < count; ++i) {
        uint32_t p = pixels[i];
        uint32_t a = (p >> 24) & 0xFF;
        if(a == 0 || a == 255)
            continue;
        uint32_t r = ((p >> 16) & 0xFF) * 255 / a;
        uint32_t g = ((p >> 8) & 0xFF) * 255 / a;
        uint32_t b = (p & 0xFF) * 255 / a;
        pixels[i] = (a << 24) | (clamp255(r) << 16) | (clamp255(g) << 8) | clamp255(b);
    }
}

void plutovg_filter_premultiply(uint32_t* pixels, int count)
{
    for(int i = 0; i < count; ++i) {
        uint32_t p = pixels[i];
        uint32_t a = (p >> 24) & 0xFF;
        if(a == 0 || a == 255)
            continue;
        uint32_t r = ((p >> 16) & 0xFF) * a / 255;
        uint32_t g = ((p >> 8) & 0xFF) * a / 255;
        uint32_t b = (p & 0xFF) * a / 255;
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
}

// ============================================================
// Phase F3: Component Transfer, Morphology, Tile
// ============================================================

static float apply_transfer_func(float value, const plutovg_transfer_func_t* func)
{
    if(!func)
        return value;

    switch(func->type) {
    case 0: // identity
        return value;
    case 1: // table
    {
        if(func->tableSize < 2 || !func->tableValues)
            return value;
        float scaled = value * (func->tableSize - 1);
        int k = (int)scaled;
        if(k < 0) k = 0;
        if(k >= func->tableSize - 1)
            return func->tableValues[func->tableSize - 1];
        float frac = scaled - k;
        return func->tableValues[k] + frac * (func->tableValues[k + 1] - func->tableValues[k]);
    }
    case 2: // discrete
    {
        if(func->tableSize < 1 || !func->tableValues)
            return value;
        int k = (int)(value * func->tableSize);
        if(k < 0) k = 0;
        if(k >= func->tableSize) k = func->tableSize - 1;
        return func->tableValues[k];
    }
    case 3: // linear: C' = slope * C + intercept
        return func->slope * value + func->intercept;
    case 4: // gamma: C' = amplitude * pow(C, exponent) + offset
        return func->amplitude * powf(value, func->exponent) + func->offset;
    default:
        return value;
    }
}

void plutovg_filter_component_transfer(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const plutovg_transfer_func_t* funcR,
    const plutovg_transfer_func_t* funcG,
    const plutovg_transfer_func_t* funcB,
    const plutovg_transfer_func_t* funcA,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;
    const uint32_t* src_data = (const uint32_t*)plutovg_surface_get_data(src);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            uint32_t p = 0;
            if(row < src_h && col < src_w)
                p = src_data[row * src_stride + col];

            // Unpremultiply
            uint32_t a = (p >> 24) & 0xFF;
            float R, G, B, A;
            if(a == 0) {
                R = G = B = A = 0.f;
            } else {
                R = (float)((p >> 16) & 0xFF) / (float)a;
                G = (float)((p >> 8) & 0xFF) / (float)a;
                B = (float)(p & 0xFF) / (float)a;
                A = (float)a / 255.f;
            }

            // Apply transfer functions
            R = apply_transfer_func(R, funcR);
            G = apply_transfer_func(G, funcG);
            B = apply_transfer_func(B, funcB);
            A = apply_transfer_func(A, funcA);

            // Clamp
            if(R < 0.f) R = 0.f; if(R > 1.f) R = 1.f;
            if(G < 0.f) G = 0.f; if(G > 1.f) G = 1.f;
            if(B < 0.f) B = 0.f; if(B > 1.f) B = 1.f;
            if(A < 0.f) A = 0.f; if(A > 1.f) A = 1.f;

            // Premultiply and store
            uint32_t ia = clamp255((int)(A * 255.f + 0.5f));
            uint32_t ir = clamp255((int)(R * 255.f + 0.5f));
            uint32_t ig = clamp255((int)(G * 255.f + 0.5f));
            uint32_t ib = clamp255((int)(B * 255.f + 0.5f));
            dst_data[row * dst_stride + col] = premultiply_pixel(ir, ig, ib, ia);
        }
    }
}

void plutovg_filter_morphology(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int op, float radiusX, float radiusY,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;
    const uint32_t* src_data = (const uint32_t*)plutovg_surface_get_data(src);

    int rx = (int)(radiusX + 0.5f);
    int ry = (int)(radiusY + 0.5f);
    if(rx < 0) rx = 0;
    if(ry < 0) ry = 0;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            uint32_t best_r, best_g, best_b, best_a;

            if(op == 0) { // erode: find minimum
                best_r = best_g = best_b = best_a = 255;
            } else { // dilate: find maximum
                best_r = best_g = best_b = best_a = 0;
            }

            for(int ky = -ry; ky <= ry; ++ky) {
                int sy = row + ky;
                if(sy < 0 || sy >= src_h) continue;
                for(int kx = -rx; kx <= rx; ++kx) {
                    int sx = col + kx;
                    if(sx < 0 || sx >= src_w) continue;
                    uint32_t p = src_data[sy * src_stride + sx];
                    uint32_t r = (p >> 16) & 0xFF;
                    uint32_t g = (p >> 8) & 0xFF;
                    uint32_t b = p & 0xFF;
                    uint32_t a = (p >> 24) & 0xFF;

                    if(op == 0) { // erode
                        if(r < best_r) best_r = r;
                        if(g < best_g) best_g = g;
                        if(b < best_b) best_b = b;
                        if(a < best_a) best_a = a;
                    } else { // dilate
                        if(r > best_r) best_r = r;
                        if(g > best_g) best_g = g;
                        if(b > best_b) best_b = b;
                        if(a > best_a) best_a = a;
                    }
                }
            }

            dst_data[row * dst_stride + col] = (best_a << 24) | (best_r << 16) | (best_g << 8) | best_b;
        }
    }
}

void plutovg_filter_tile(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int srcX, int srcY, int srcW, int srcH,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;
    const uint32_t* src_data = (const uint32_t*)plutovg_surface_get_data(src);

    if(srcW <= 0 || srcH <= 0)
        return;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            // Map dst pixel to tiled source pixel
            int tx = ((col - srcX) % srcW + srcW) % srcW + srcX;
            int ty = ((row - srcY) % srcH + srcH) % srcH + srcY;

            uint32_t p = 0;
            if(tx >= 0 && tx < src_w && ty >= 0 && ty < src_h)
                p = src_data[ty * src_stride + tx];

            dst_data[row * dst_stride + col] = p;
        }
    }
}

// ============================================================
// Phase F4: Turbulence, Convolve Matrix, Displacement Map
// ============================================================

// --- Perlin Turbulence (per SVG spec appendix) ---

#define PERLIN_B  0x100
#define PERLIN_BM 0xFF
#define PERLIN_N  0x1000

typedef struct {
    int latticeSelector[PERLIN_B + PERLIN_B + 2];
    double gradient[4][PERLIN_B + PERLIN_B + 2][2];
} perlin_noise_t;

static double s_curve(double t)
{
    return t * t * (3.0 - 2.0 * t);
}

static double lerp_d(double t, double a, double b)
{
    return a + t * (b - a);
}

static void perlin_init(perlin_noise_t* pn, float seed)
{
    // Initialize with seed
    // Use a simple LCG seeded from the float seed
    unsigned int s = (unsigned int)(seed * 2147483647.0f);
    if(s == 0) s = 1;

    for(int k = 0; k < 4; ++k) {
        for(int i = 0; i < PERLIN_B; ++i) {
            pn->latticeSelector[i] = i;
            for(int j = 0; j < 2; ++j) {
                s = s * 1103515245u + 12345u;
                double val = (double)((int)(s >> 1) % (PERLIN_B + PERLIN_B)) - PERLIN_B;
                pn->gradient[k][i][j] = val / PERLIN_B;
            }
            // normalize gradient
            double s2 = pn->gradient[k][i][0] * pn->gradient[k][i][0]
                      + pn->gradient[k][i][1] * pn->gradient[k][i][1];
            if(s2 > 0.0001) {
                double inv = 1.0 / sqrt(s2);
                pn->gradient[k][i][0] *= inv;
                pn->gradient[k][i][1] *= inv;
            }
        }
    }

    // Shuffle lattice selector
    for(int i = PERLIN_B - 1; i > 0; --i) {
        s = s * 1103515245u + 12345u;
        int j = (int)((s >> 1) % (unsigned int)(i + 1));
        int tmp = pn->latticeSelector[i];
        pn->latticeSelector[i] = pn->latticeSelector[j];
        pn->latticeSelector[j] = tmp;
    }

    // Extend arrays
    for(int i = 0; i < PERLIN_B + 2; ++i) {
        pn->latticeSelector[PERLIN_B + i] = pn->latticeSelector[i];
        for(int k = 0; k < 4; ++k) {
            pn->gradient[k][PERLIN_B + i][0] = pn->gradient[k][i][0];
            pn->gradient[k][PERLIN_B + i][1] = pn->gradient[k][i][1];
        }
    }
}

static double perlin_noise2(const perlin_noise_t* pn, int channel, double x, double y)
{
    double t = x + PERLIN_N;
    int bx0 = ((int)t) & PERLIN_BM;
    int bx1 = (bx0 + 1) & PERLIN_BM;
    double rx0 = t - (int)t;
    double rx1 = rx0 - 1.0;

    t = y + PERLIN_N;
    int by0 = ((int)t) & PERLIN_BM;
    int by1 = (by0 + 1) & PERLIN_BM;
    double ry0 = t - (int)t;
    double ry1 = ry0 - 1.0;

    int i = pn->latticeSelector[bx0];
    int j = pn->latticeSelector[bx1];

    int b00 = pn->latticeSelector[i + by0];
    int b10 = pn->latticeSelector[j + by0];
    int b01 = pn->latticeSelector[i + by1];
    int b11 = pn->latticeSelector[j + by1];

    double sx = s_curve(rx0);
    double sy = s_curve(ry0);

    const double* q;
    double u, v;

    q = pn->gradient[channel][b00]; u = rx0 * q[0] + ry0 * q[1];
    q = pn->gradient[channel][b10]; v = rx1 * q[0] + ry0 * q[1];
    double a2 = lerp_d(sx, u, v);

    q = pn->gradient[channel][b01]; u = rx0 * q[0] + ry1 * q[1];
    q = pn->gradient[channel][b11]; v = rx1 * q[0] + ry1 * q[1];
    double b2 = lerp_d(sx, u, v);

    return lerp_d(sy, a2, b2);
}

void plutovg_filter_turbulence(
    plutovg_surface_t* dst,
    float baseFreqX, float baseFreqY,
    int numOctaves, float seed, int stitchTiles, int fractalNoise,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    perlin_noise_t pn;
    perlin_init(&pn, seed);

    if(numOctaves < 1) numOctaves = 1;
    if(numOctaves > 8) numOctaves = 8;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    (void)stitchTiles; // TODO: implement tile stitching

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            double channelValues[4] = {0, 0, 0, 0};

            for(int ch = 0; ch < 4; ++ch) {
                double freq_x = baseFreqX;
                double freq_y = baseFreqY;
                double amplitude = 1.0;
                double sum = 0.0;

                for(int oct = 0; oct < numOctaves; ++oct) {
                    double noise = perlin_noise2(&pn, ch, col * freq_x, row * freq_y);
                    if(fractalNoise) {
                        sum += noise * amplitude;
                    } else {
                        sum += fabs(noise) * amplitude;
                    }
                    freq_x *= 2.0;
                    freq_y *= 2.0;
                    amplitude *= 0.5;
                }

                channelValues[ch] = sum;
            }

            uint32_t r, g, b, a;
            if(fractalNoise) {
                // fractalNoise: result in [-1,1], map to [0,1]
                r = clamp255((int)((channelValues[0] * 0.5 + 0.5) * 255.0 + 0.5));
                g = clamp255((int)((channelValues[1] * 0.5 + 0.5) * 255.0 + 0.5));
                b = clamp255((int)((channelValues[2] * 0.5 + 0.5) * 255.0 + 0.5));
                a = clamp255((int)((channelValues[3] * 0.5 + 0.5) * 255.0 + 0.5));
            } else {
                // turbulence: result in [0,1]
                r = clamp255((int)(channelValues[0] * 255.0 + 0.5));
                g = clamp255((int)(channelValues[1] * 255.0 + 0.5));
                b = clamp255((int)(channelValues[2] * 255.0 + 0.5));
                a = clamp255((int)(channelValues[3] * 255.0 + 0.5));
            }

            // Premultiply
            dst_data[row * dst_stride + col] = premultiply_pixel(r, g, b, a);
        }
    }
}

// --- Convolve Matrix ---

void plutovg_filter_convolve_matrix(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const float* kernelMatrix, int orderX, int orderY,
    int targetX, int targetY, float divisor, float bias,
    int edgeMode, int preserveAlpha,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int src_w = plutovg_surface_get_width(src);
    int src_h = plutovg_surface_get_height(src);
    int src_stride = plutovg_surface_get_stride(src) / 4;
    const uint32_t* src_data = (const uint32_t*)plutovg_surface_get_data(src);

    if(!kernelMatrix || orderX <= 0 || orderY <= 0)
        return;

    if(targetX < 0) targetX = orderX / 2;
    if(targetY < 0) targetY = orderY / 2;

    // Auto-compute divisor
    if(divisor == 0.f) {
        float sum = 0.f;
        for(int i = 0; i < orderX * orderY; ++i)
            sum += kernelMatrix[i];
        divisor = (sum != 0.f) ? sum : 1.f;
    }

    float invDivisor = 1.f / divisor;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            float sumR = 0.f, sumG = 0.f, sumB = 0.f, sumA = 0.f;

            for(int ky = 0; ky < orderY; ++ky) {
                for(int kx = 0; kx < orderX; ++kx) {
                    int sx = col + kx - targetX;
                    int sy = row + ky - targetY;

                    uint32_t p;
                    if(sx < 0 || sx >= src_w || sy < 0 || sy >= src_h) {
                        switch(edgeMode) {
                        case 0: // duplicate
                            if(sx < 0) sx = 0;
                            if(sx >= src_w) sx = src_w - 1;
                            if(sy < 0) sy = 0;
                            if(sy >= src_h) sy = src_h - 1;
                            p = src_data[sy * src_stride + sx];
                            break;
                        case 1: // wrap
                            sx = ((sx % src_w) + src_w) % src_w;
                            sy = ((sy % src_h) + src_h) % src_h;
                            p = src_data[sy * src_stride + sx];
                            break;
                        default: // none
                            p = 0;
                            break;
                        }
                    } else {
                        p = src_data[sy * src_stride + sx];
                    }

                    // SVG spec: kernel index is (orderY-1-ky)*orderX + (orderX-1-kx)
                    // for the standard flipped convolution
                    float kv = kernelMatrix[ky * orderX + kx];

                    // Unpremultiply for correct math
                    uint32_t pa = (p >> 24) & 0xFF;
                    float pr, pg, pb, paf;
                    if(pa == 0) {
                        pr = pg = pb = paf = 0.f;
                    } else {
                        pr = (float)((p >> 16) & 0xFF) / (float)pa * 255.f;
                        pg = (float)((p >> 8) & 0xFF) / (float)pa * 255.f;
                        pb = (float)(p & 0xFF) / (float)pa * 255.f;
                        paf = (float)pa;
                    }

                    sumR += pr * kv;
                    sumG += pg * kv;
                    sumB += pb * kv;
                    sumA += paf * kv;
                }
            }

            float outR = sumR * invDivisor + bias * 255.f;
            float outG = sumG * invDivisor + bias * 255.f;
            float outB = sumB * invDivisor + bias * 255.f;
            float outA;

            if(preserveAlpha) {
                // Use original alpha
                uint32_t origP = 0;
                if(row < src_h && col < src_w)
                    origP = src_data[row * src_stride + col];
                outA = (float)((origP >> 24) & 0xFF);
            } else {
                outA = sumA * invDivisor + bias * 255.f;
            }

            uint32_t ia = clamp255((int)(outA + 0.5f));
            uint32_t ir = clamp255((int)(outR + 0.5f));
            uint32_t ig = clamp255((int)(outG + 0.5f));
            uint32_t ib = clamp255((int)(outB + 0.5f));
            dst_data[row * dst_stride + col] = premultiply_pixel(ir, ig, ib, ia);
        }
    }
}

// --- Displacement Map ---

void plutovg_filter_displacement_map(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    float scale, int xChannel, int yChannel,
    int x, int y, int width, int height)
{
    int dst_w = plutovg_surface_get_width(dst);
    int dst_h = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);

    int in1_w = plutovg_surface_get_width(in1);
    int in1_h = plutovg_surface_get_height(in1);
    int in1_stride = plutovg_surface_get_stride(in1) / 4;
    const uint32_t* in1_data = (const uint32_t*)plutovg_surface_get_data(in1);

    int in2_w = plutovg_surface_get_width(in2);
    int in2_h = plutovg_surface_get_height(in2);
    int in2_stride = plutovg_surface_get_stride(in2) / 4;
    const uint32_t* in2_data = (const uint32_t*)plutovg_surface_get_data(in2);

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > dst_w ? dst_w : (x + width);
    int y1 = (y + height) > dst_h ? dst_h : (y + height);

    // Channel extraction helper: 0=R, 1=G, 2=B, 3=A
    // Displacement uses unpremultiplied values from in2
    for(int row = y0; row < y1; ++row) {
        for(int col = x0; col < x1; ++col) {
            // Get displacement from in2
            uint32_t dp = 0;
            if(row < in2_h && col < in2_w)
                dp = in2_data[row * in2_stride + col];

            // Unpremultiply displacement pixel
            uint32_t da = (dp >> 24) & 0xFF;
            float dr, dg, db;
            if(da == 0) {
                dr = dg = db = 0.f;
            } else {
                dr = (float)((dp >> 16) & 0xFF) / (float)da;
                dg = (float)((dp >> 8) & 0xFF) / (float)da;
                db = (float)(dp & 0xFF) / (float)da;
            }
            float daf = (float)da / 255.f;

            // Extract channel values (in [0,1] for RGB, [0,1] for A)
            float channels[4] = {dr, dg, db, daf};

            // Compute displaced coordinates
            // SVG formula: P'(x,y) = in1(x + scale*(xChannel-0.5), y + scale*(yChannel-0.5))
            float dispX = scale * (channels[xChannel] - 0.5f);
            float dispY = scale * (channels[yChannel] - 0.5f);

            int sx = (int)(col + dispX + 0.5f);
            int sy = (int)(row + dispY + 0.5f);

            uint32_t p = 0;
            if(sx >= 0 && sx < in1_w && sy >= 0 && sy < in1_h)
                p = in1_data[sy * in1_stride + sx];

            dst_data[row * dst_stride + col] = p;
        }
    }
}


// --- Phase F5: Lighting primitives ---

// Get alpha value at pixel (x,y), clamped to surface bounds
static float get_alpha(const plutovg_surface_t* src, int x, int y, int w, int h)
{
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x >= w) x = w - 1;
    if(y >= h) y = h - 1;
    const uint32_t* data = (const uint32_t*)plutovg_surface_get_data(src);
    int stride = plutovg_surface_get_stride(src) / 4;
    return (float)((data[y * stride + x] >> 24) & 0xFF) / 255.0f;
}

// Compute surface normal at pixel (x,y) using Sobel-like operators from SVG spec
// surfaceScale scales the alpha values to create the bump map height
static void compute_normal(const plutovg_surface_t* src, int px, int py,
    int w, int h, float surfaceScale, float* nx, float* ny, float* nz)
{
    // SVG spec normal computation using kernel factors
    // Interior pixels use standard 3x3 Sobel
    // Edge/corner pixels use modified kernels per spec
    float factorX, factorY;

    int isLeft = (px == 0);
    int isRight = (px == w - 1);
    int isTop = (py == 0);
    int isBottom = (py == h - 1);

    float dx, dy;

    if(isLeft && isTop) {
        // Top-left corner
        float a00 = get_alpha(src, px, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        float a11 = get_alpha(src, px + 1, py + 1, w, h);
        dx = (2.0f * (a10 - a00) + (a11 - a01)) / 3.0f;
        dy = (2.0f * (a01 - a00) + (a11 - a10)) / 3.0f;
        factorX = 2.0f / 3.0f;
        factorY = 2.0f / 3.0f;
    } else if(isRight && isTop) {
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        float a_11 = get_alpha(src, px - 1, py + 1, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        dx = (2.0f * (a00 - a_10) + (a01 - a_11)) / 3.0f;
        dy = (2.0f * (a01 - a00) + (a_11 - a_10)) / 3.0f;
        factorX = 2.0f / 3.0f;
        factorY = 2.0f / 3.0f;
    } else if(isLeft && isBottom) {
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a1_1 = get_alpha(src, px + 1, py - 1, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        dx = (2.0f * (a10 - a00) + (a1_1 - a0_1)) / 3.0f;
        dy = (2.0f * (a00 - a0_1) + (a10 - a1_1)) / 3.0f;
        factorX = 2.0f / 3.0f;
        factorY = 2.0f / 3.0f;
    } else if(isRight && isBottom) {
        float a_1_1 = get_alpha(src, px - 1, py - 1, w, h);
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        dx = (2.0f * (a00 - a_10) + (a0_1 - a_1_1)) / 3.0f;
        dy = (2.0f * (a00 - a0_1) + (a_10 - a_1_1)) / 3.0f;
        factorX = 2.0f / 3.0f;
        factorY = 2.0f / 3.0f;
    } else if(isTop) {
        // Top edge (not corner)
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        float a_11 = get_alpha(src, px - 1, py + 1, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        float a11 = get_alpha(src, px + 1, py + 1, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        dx = (a10 - a_10 + a11 - a_11) / 2.0f;
        dy = (2.0f * (a01 - a00) + (a11 - a10) + (a_11 - a_10)) / 4.0f;
        factorX = 1.0f / 2.0f;
        factorY = 2.0f / 3.0f;
    } else if(isBottom) {
        float a_1_1 = get_alpha(src, px - 1, py - 1, w, h);
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a1_1 = get_alpha(src, px + 1, py - 1, w, h);
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        dx = (a10 - a_10 + a1_1 - a_1_1) / 2.0f;
        dy = (2.0f * (a00 - a0_1) + (a10 - a1_1) + (a_10 - a_1_1)) / 4.0f;
        factorX = 1.0f / 2.0f;
        factorY = 2.0f / 3.0f;
    } else if(isLeft) {
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a1_1 = get_alpha(src, px + 1, py - 1, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        float a11 = get_alpha(src, px + 1, py + 1, w, h);
        dx = (2.0f * (a10 - a00) + (a11 - a01) + (a1_1 - a0_1)) / 4.0f;
        dy = (a01 - a0_1 + a11 - a1_1) / 2.0f;
        factorX = 2.0f / 3.0f;
        factorY = 1.0f / 2.0f;
    } else if(isRight) {
        float a_1_1 = get_alpha(src, px - 1, py - 1, w, h);
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a00 = get_alpha(src, px, py, w, h);
        float a_11 = get_alpha(src, px - 1, py + 1, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        dx = (2.0f * (a00 - a_10) + (a01 - a_11) + (a0_1 - a_1_1)) / 4.0f;
        dy = (a01 - a0_1 + a_11 - a_1_1) / 2.0f;
        factorX = 2.0f / 3.0f;
        factorY = 1.0f / 2.0f;
    } else {
        // Interior pixel — standard Sobel
        float a_1_1 = get_alpha(src, px - 1, py - 1, w, h);
        float a0_1 = get_alpha(src, px, py - 1, w, h);
        float a1_1 = get_alpha(src, px + 1, py - 1, w, h);
        float a_10 = get_alpha(src, px - 1, py, w, h);
        float a10 = get_alpha(src, px + 1, py, w, h);
        float a_11 = get_alpha(src, px - 1, py + 1, w, h);
        float a01 = get_alpha(src, px, py + 1, w, h);
        float a11 = get_alpha(src, px + 1, py + 1, w, h);
        dx = (-a_1_1 + a1_1 - 2.0f * a_10 + 2.0f * a10 - a_11 + a11) / 4.0f;
        dy = (-a_1_1 - 2.0f * a0_1 - a1_1 + a_11 + 2.0f * a01 + a_11) / 4.0f;
        factorX = 1.0f / 4.0f;
        factorY = 1.0f / 4.0f;
    }

    (void)factorX;
    (void)factorY;

    *nx = -surfaceScale * dx;
    *ny = -surfaceScale * dy;
    *nz = 1.0f;

    // Normalize
    float len = sqrtf((*nx) * (*nx) + (*ny) * (*ny) + (*nz) * (*nz));
    if(len > 0.0f) {
        *nx /= len;
        *ny /= len;
        *nz /= len;
    }
}

// Compute light vector at pixel (px, py) with alpha-derived height
static void compute_light_vector(
    const plutovg_light_source_t* light, int px, int py, float surfaceZ,
    float* lx, float* ly, float* lz, float* attenuation)
{
    *attenuation = 1.0f;

    if(light->type == PLUTOVG_LIGHT_DISTANT) {
        float az = light->azimuth * 3.14159265358979f / 180.0f;
        float el = light->elevation * 3.14159265358979f / 180.0f;
        *lx = cosf(az) * cosf(el);
        *ly = sinf(az) * cosf(el);
        *lz = sinf(el);
    } else {
        // Point or Spot: direction from surface point to light
        float dx = light->x - (float)px;
        float dy = light->y - (float)py;
        float dz = light->z - surfaceZ;
        float len = sqrtf(dx * dx + dy * dy + dz * dz);
        if(len > 0.0f) {
            *lx = dx / len;
            *ly = dy / len;
            *lz = dz / len;
        } else {
            *lx = 0.0f; *ly = 0.0f; *lz = 1.0f;
        }

        if(light->type == PLUTOVG_LIGHT_SPOT) {
            // Spot direction: from light to pointsAt
            float sx = light->pointsAtX - light->x;
            float sy = light->pointsAtY - light->y;
            float sz = light->pointsAtZ - light->z;
            float slen = sqrtf(sx * sx + sy * sy + sz * sz);
            if(slen > 0.0f) {
                sx /= slen; sy /= slen; sz /= slen;
            }

            // Dot product: -L · S (angle between light-to-point and light-to-target)
            float cosAngle = -(*lx) * sx + -(*ly) * sy + -(*lz) * sz;

            if(light->hasLimitingConeAngle) {
                float cosLimit = cosf(light->limitingConeAngle * 3.14159265358979f / 180.0f);
                if(cosAngle < cosLimit) {
                    *attenuation = 0.0f;
                    return;
                }
            }

            if(cosAngle > 0.0f) {
                *attenuation = powf(cosAngle, light->specularExponent);
            } else {
                *attenuation = 0.0f;
            }
        }
    }
}

void plutovg_filter_diffuse_lighting(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float surfaceScale, float diffuseConstant,
    float lightR, float lightG, float lightB,
    const plutovg_light_source_t* light,
    int x, int y, int width, int height)
{
    int w = plutovg_surface_get_width(dst);
    int h = plutovg_surface_get_height(dst);
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;

    int srcW = plutovg_surface_get_width(src);
    int srcH = plutovg_surface_get_height(src);

    for(int row = y; row < y + height && row < h; ++row) {
        for(int col = x; col < x + width && col < w; ++col) {
            float nx, ny, nz;
            compute_normal(src, col, row, srcW, srcH, surfaceScale, &nx, &ny, &nz);

            float surfaceZ = surfaceScale * get_alpha(src, col, row, srcW, srcH);
            float lx, ly, lz, atten;
            compute_light_vector(light, col, row, surfaceZ, &lx, &ly, &lz, &atten);

            // N · L
            float NdotL = nx * lx + ny * ly + nz * lz;
            if(NdotL < 0.0f) NdotL = 0.0f;

            float kd = diffuseConstant * NdotL * atten;

            int cr = (int)(kd * lightR * 255.0f + 0.5f);
            int cg = (int)(kd * lightG * 255.0f + 0.5f);
            int cb = (int)(kd * lightB * 255.0f + 0.5f);
            if(cr > 255) cr = 255;
            if(cg > 255) cg = 255;
            if(cb > 255) cb = 255;
            if(cr < 0) cr = 0;
            if(cg < 0) cg = 0;
            if(cb < 0) cb = 0;

            // Alpha = max(R,G,B) for diffuse lighting per spec
            int ca = cr;
            if(cg > ca) ca = cg;
            if(cb > ca) ca = cb;

            // Premultiply
            if(ca > 0) {
                cr = cr * ca / 255;
                cg = cg * ca / 255;
                cb = cb * ca / 255;
            }

            dst_data[row * dst_stride + col] = ((uint32_t)ca << 24) | ((uint32_t)cr << 16) | ((uint32_t)cg << 8) | (uint32_t)cb;
        }
    }
}

void plutovg_filter_specular_lighting(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float surfaceScale, float specularConstant, float specularExponent,
    float lightR, float lightG, float lightB,
    const plutovg_light_source_t* light,
    int x, int y, int width, int height)
{
    int w = plutovg_surface_get_width(dst);
    int h = plutovg_surface_get_height(dst);
    uint32_t* dst_data = (uint32_t*)plutovg_surface_get_data(dst);
    int dst_stride = plutovg_surface_get_stride(dst) / 4;

    int srcW = plutovg_surface_get_width(src);
    int srcH = plutovg_surface_get_height(src);

    // Eye vector (viewer at infinity along Z axis per SVG spec)
    float ex = 0.0f, ey = 0.0f, ez = 1.0f;

    for(int row = y; row < y + height && row < h; ++row) {
        for(int col = x; col < x + width && col < w; ++col) {
            float nx, ny, nz;
            compute_normal(src, col, row, srcW, srcH, surfaceScale, &nx, &ny, &nz);

            float surfaceZ = surfaceScale * get_alpha(src, col, row, srcW, srcH);
            float lx, ly, lz, atten;
            compute_light_vector(light, col, row, surfaceZ, &lx, &ly, &lz, &atten);

            // Half vector H = normalize(L + E)
            float hx = lx + ex;
            float hy = ly + ey;
            float hz = lz + ez;
            float hlen = sqrtf(hx * hx + hy * hy + hz * hz);
            if(hlen > 0.0f) {
                hx /= hlen; hy /= hlen; hz /= hlen;
            }

            // N · H
            float NdotH = nx * hx + ny * hy + nz * hz;
            if(NdotH < 0.0f) NdotH = 0.0f;

            float ks = specularConstant * powf(NdotH, specularExponent) * atten;

            int cr = (int)(ks * lightR * 255.0f + 0.5f);
            int cg = (int)(ks * lightG * 255.0f + 0.5f);
            int cb = (int)(ks * lightB * 255.0f + 0.5f);
            if(cr > 255) cr = 255;
            if(cg > 255) cg = 255;
            if(cb > 255) cb = 255;
            if(cr < 0) cr = 0;
            if(cg < 0) cg = 0;
            if(cb < 0) cb = 0;

            // Alpha = max(R,G,B) for specular lighting per spec
            int ca = cr;
            if(cg > ca) ca = cg;
            if(cb > ca) ca = cb;

            // Premultiply
            if(ca > 0) {
                cr = cr * ca / 255;
                cg = cg * ca / 255;
                cb = cb * ca / 255;
            }

            dst_data[row * dst_stride + col] = ((uint32_t)ca << 24) | ((uint32_t)cr << 16) | ((uint32_t)cg << 8) | (uint32_t)cb;
        }
    }
}
