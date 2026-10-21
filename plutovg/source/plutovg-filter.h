#ifndef PLUTOVG_FILTER_H
#define PLUTOVG_FILTER_H

#include "plutovg-private.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Surface-to-surface filter operations
// All operate on premultiplied ARGB32 pixel buffers
// Parameters use device-space coordinates

void plutovg_filter_flood(
    plutovg_surface_t* dst,
    float r, float g, float b, float a,
    int x, int y, int width, int height);

void plutovg_filter_offset(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int dx, int dy,
    int x, int y, int width, int height);

void plutovg_filter_merge(
    plutovg_surface_t* dst,
    const plutovg_surface_t** inputs, int inputCount,
    int x, int y, int width, int height);

void plutovg_filter_gaussian_blur(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float stdDevX, float stdDevY,
    int x, int y, int width, int height);

void plutovg_filter_blend(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    int mode,
    int x, int y, int width, int height);

void plutovg_filter_color_matrix(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const float matrix[20],
    int x, int y, int width, int height);

void plutovg_filter_composite(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    int op, float k1, float k2, float k3, float k4,
    int x, int y, int width, int height);

// --- Phase F3 primitives ---

// Component transfer function specification
typedef struct {
    int type; // 0=identity, 1=table, 2=discrete, 3=linear, 4=gamma
    const float* tableValues;
    int tableSize;
    float slope;      // for linear
    float intercept;   // for linear
    float amplitude;   // for gamma
    float exponent;    // for gamma
    float offset;      // for gamma
} plutovg_transfer_func_t;

void plutovg_filter_component_transfer(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const plutovg_transfer_func_t* funcR,
    const plutovg_transfer_func_t* funcG,
    const plutovg_transfer_func_t* funcB,
    const plutovg_transfer_func_t* funcA,
    int x, int y, int width, int height);

void plutovg_filter_morphology(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int op, float radiusX, float radiusY,
    int x, int y, int width, int height);

void plutovg_filter_tile(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    int srcX, int srcY, int srcW, int srcH,
    int x, int y, int width, int height);

// --- Phase F4 primitives ---

void plutovg_filter_turbulence(
    plutovg_surface_t* dst,
    float baseFreqX, float baseFreqY,
    int numOctaves, float seed, int stitchTiles, int fractalNoise,
    int x, int y, int width, int height);

void plutovg_filter_convolve_matrix(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    const float* kernelMatrix, int orderX, int orderY,
    int targetX, int targetY, float divisor, float bias,
    int edgeMode, int preserveAlpha,
    int x, int y, int width, int height);

void plutovg_filter_displacement_map(
    plutovg_surface_t* dst,
    const plutovg_surface_t* in1, const plutovg_surface_t* in2,
    float scale, int xChannel, int yChannel,
    int x, int y, int width, int height);

// --- Phase F5: Lighting primitives ---

// Light source type identifiers
#define PLUTOVG_LIGHT_DISTANT 0
#define PLUTOVG_LIGHT_POINT   1
#define PLUTOVG_LIGHT_SPOT    2

typedef struct {
    int type; // PLUTOVG_LIGHT_DISTANT, POINT, or SPOT
    // Distant light
    float azimuth;   // degrees
    float elevation;  // degrees
    // Point/Spot light position
    float x, y, z;
    // Spot light target & params
    float pointsAtX, pointsAtY, pointsAtZ;
    float specularExponent; // spot exponent (cone falloff)
    float limitingConeAngle; // degrees, <=0 means no limit
    int hasLimitingConeAngle;
} plutovg_light_source_t;

void plutovg_filter_diffuse_lighting(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float surfaceScale, float diffuseConstant,
    float lightR, float lightG, float lightB,
    const plutovg_light_source_t* light,
    int x, int y, int width, int height);

void plutovg_filter_specular_lighting(
    plutovg_surface_t* dst, const plutovg_surface_t* src,
    float surfaceScale, float specularConstant, float specularExponent,
    float lightR, float lightG, float lightB,
    const plutovg_light_source_t* light,
    int x, int y, int width, int height);

// Helper: premultiply/unpremultiply for filters that need linear RGB
void plutovg_filter_unpremultiply(uint32_t* pixels, int count);
void plutovg_filter_premultiply(uint32_t* pixels, int count);

#ifdef __cplusplus
}
#endif

#endif // PLUTOVG_FILTER_H
