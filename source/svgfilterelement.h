#ifndef LUNASVG_SVGFILTERELEMENT_H
#define LUNASVG_SVGFILTERELEMENT_H

#include "svgelement.h"

#include <vector>
#include <map>

namespace lunasvg {

class SVGFilterElement;
class SVGFilterPrimitiveElement;

// Named filter buffer management
class SVGFilterContext {
public:
    SVGFilterContext(const Canvas& sourceGraphic, const Rect& filterRegion);
    ~SVGFilterContext();

    plutovg_surface_t* getInput(const std::string& name) const;
    void setOutput(const std::string& name, plutovg_surface_t* surface);
    plutovg_surface_t* lastResult() const { return m_lastResult; }

    plutovg_surface_t* sourceGraphic() const { return m_sourceGraphic; }
    plutovg_surface_t* sourceAlpha();
    const Rect& filterRegion() const { return m_filterRegion; }

    // Allocate temporary surface matching filter region dimensions
    plutovg_surface_t* createSurface() const;

private:
    std::map<std::string, plutovg_surface_t*> m_buffers;
    plutovg_surface_t* m_sourceGraphic;
    plutovg_surface_t* m_sourceAlpha = nullptr;
    plutovg_surface_t* m_lastResult = nullptr;
    Rect m_filterRegion;
    std::vector<plutovg_surface_t*> m_ownedSurfaces; // for cleanup
};

// <filter> container element
class SVGFilterElement final : public SVGElement {
public:
    SVGFilterElement(Document* document);

    void applyFilter(SVGRenderState& state) const;

    Rect filterRegion(const SVGElement* target) const;

    void layoutElement(const SVGLayoutState& state) override;

private:
    SVGEnumeration<Units> m_filterUnits;        // default: objectBoundingBox
    SVGEnumeration<Units> m_primitiveUnits;     // default: userSpaceOnUse
    SVGLength m_x;   // default: -10%
    SVGLength m_y;   // default: -10%
    SVGLength m_width;  // default: 120%
    SVGLength m_height; // default: 120%
};

// Base class for all <fe*> elements
class SVGFilterPrimitiveElement : public SVGElement {
public:
    SVGFilterPrimitiveElement(Document* document, ElementID id);

    virtual void apply(SVGFilterContext& context) const = 0;

    const std::string& in() const { return m_in.value(); }
    const std::string& result() const { return m_result.value(); }

protected:
    SVGString m_in;
    SVGString m_result;
};

// <feFlood>
class SVGFeFloodElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeFloodElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    Color m_floodColor = Color::Black;
    float m_floodOpacity = 1.f;
};

// <feOffset>
class SVGFeOffsetElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeOffsetElement(Document* document);
    void apply(SVGFilterContext& context) const override;

private:
    SVGLength m_dx;
    SVGLength m_dy;
};

// <feMerge>
class SVGFeMergeElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeMergeElement(Document* document);
    void apply(SVGFilterContext& context) const override;
};

// <feMergeNode>
class SVGFeMergeNodeElement final : public SVGElement {
public:
    SVGFeMergeNodeElement(Document* document);
    const std::string& in() const { return m_in.value(); }

private:
    SVGString m_in;
};

// <feGaussianBlur>
class SVGFeGaussianBlurElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeGaussianBlurElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    float m_stdDeviationX = 0.f;
    float m_stdDeviationY = 0.f;
};

// <feBlend>
class SVGFeBlendElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeBlendElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    SVGString m_in2;
    int m_mode = 0; // 0=normal, 1=multiply, 2=screen, 3=darken, 4=lighten
};

// <feColorMatrix>
class SVGFeColorMatrixElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeColorMatrixElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    int m_type = 0; // 0=matrix, 1=saturate, 2=hueRotate, 3=luminanceToAlpha
    float m_matrix[20];
};

// <feComposite>
class SVGFeCompositeElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeCompositeElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    SVGString m_in2;
    int m_operator = 0; // 0=over, 1=in, 2=out, 3=atop, 4=xor, 5=arithmetic
    float m_k1 = 0.f, m_k2 = 0.f, m_k3 = 0.f, m_k4 = 0.f;
};

// --- Phase F3 classes ---

// <feComponentTransfer>
class SVGFeComponentTransferElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeComponentTransferElement(Document* document);
    void apply(SVGFilterContext& context) const override;
};

// <feFuncR>, <feFuncG>, <feFuncB>, <feFuncA>
class SVGFeFuncElement final : public SVGElement {
public:
    SVGFeFuncElement(Document* document, ElementID id);
    void layoutElement(const SVGLayoutState& state) override;

    int funcType() const { return m_funcType; }
    const std::vector<float>& tableValues() const { return m_tableValues; }
    float slope() const { return m_slope; }
    float intercept() const { return m_intercept; }
    float amplitude() const { return m_amplitude; }
    float exponent() const { return m_exponent; }
    float offset() const { return m_offset; }

private:
    int m_funcType = 0; // 0=identity, 1=table, 2=discrete, 3=linear, 4=gamma
    std::vector<float> m_tableValues;
    float m_slope = 1.f;
    float m_intercept = 0.f;
    float m_amplitude = 1.f;
    float m_exponent = 1.f;
    float m_offset = 0.f;
};

// <feMorphology>
class SVGFeMorphologyElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeMorphologyElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    int m_operator = 0; // 0=erode, 1=dilate
    float m_radiusX = 0.f;
    float m_radiusY = 0.f;
};

// <feTile>
class SVGFeTileElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeTileElement(Document* document);
    void apply(SVGFilterContext& context) const override;
};

// <feImage> (stub - no external image loading support)
class SVGFeImageElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeImageElement(Document* document);
    void apply(SVGFilterContext& context) const override;
};

// --- Phase F4 classes ---

// <feTurbulence>
class SVGFeTurbulenceElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeTurbulenceElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    float m_baseFrequencyX = 0.f;
    float m_baseFrequencyY = 0.f;
    int m_numOctaves = 1;
    float m_seed = 0.f;
    int m_stitchTiles = 0; // 0=noStitch, 1=stitch
    int m_type = 0; // 0=turbulence, 1=fractalNoise
};

// <feConvolveMatrix>
class SVGFeConvolveMatrixElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeConvolveMatrixElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    std::vector<float> m_kernelMatrix;
    int m_orderX = 3, m_orderY = 3;
    int m_targetX = -1, m_targetY = -1; // -1 = floor(orderX/2)
    float m_divisor = 0.f; // 0 = auto (sum of kernel)
    float m_bias = 0.f;
    int m_edgeMode = 0; // 0=duplicate, 1=wrap, 2=none
    bool m_preserveAlpha = false;
};

// <feDisplacementMap>
class SVGFeDisplacementMapElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeDisplacementMapElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    SVGString m_in2;
    float m_scale = 0.f;
    int m_xChannelSelector = 3; // 0=R, 1=G, 2=B, 3=A
    int m_yChannelSelector = 3;
};

// --- Phase F5 classes ---

// <feDistantLight>
class SVGFeDistantLightElement final : public SVGElement {
public:
    SVGFeDistantLightElement(Document* document);
    void layoutElement(const SVGLayoutState& state) override;

    float azimuth() const { return m_azimuth; }
    float elevation() const { return m_elevation; }

private:
    float m_azimuth = 0.f;
    float m_elevation = 0.f;
};

// <fePointLight>
class SVGFePointLightElement final : public SVGElement {
public:
    SVGFePointLightElement(Document* document);
    void layoutElement(const SVGLayoutState& state) override;

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

private:
    float m_x = 0.f, m_y = 0.f, m_z = 0.f;
};

// <feSpotLight>
class SVGFeSpotLightElement final : public SVGElement {
public:
    SVGFeSpotLightElement(Document* document);
    void layoutElement(const SVGLayoutState& state) override;

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }
    float pointsAtX() const { return m_pointsAtX; }
    float pointsAtY() const { return m_pointsAtY; }
    float pointsAtZ() const { return m_pointsAtZ; }
    float spotExponent() const { return m_specularExponent; }
    float limitingConeAngle() const { return m_limitingConeAngle; }
    bool hasLimitingConeAngle() const { return m_hasLimitingConeAngle; }

private:
    float m_x = 0.f, m_y = 0.f, m_z = 0.f;
    float m_pointsAtX = 0.f, m_pointsAtY = 0.f, m_pointsAtZ = 0.f;
    float m_specularExponent = 1.f;
    float m_limitingConeAngle = 0.f;
    bool m_hasLimitingConeAngle = false;
};

// <feDiffuseLighting>
class SVGFeDiffuseLightingElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeDiffuseLightingElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    float m_surfaceScale = 1.f;
    float m_diffuseConstant = 1.f;
    Color m_lightingColor{0xFFFFFFFF}; // white default
};

// <feSpecularLighting>
class SVGFeSpecularLightingElement final : public SVGFilterPrimitiveElement {
public:
    SVGFeSpecularLightingElement(Document* document);
    void apply(SVGFilterContext& context) const override;
    void layoutElement(const SVGLayoutState& state) override;

private:
    float m_surfaceScale = 1.f;
    float m_specularConstant = 1.f;
    float m_specularExponent = 1.f;
    Color m_lightingColor{0xFFFFFFFF}; // white default
};

} // namespace lunasvg

#endif // LUNASVG_SVGFILTERELEMENT_H
