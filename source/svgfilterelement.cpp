#include "svgfilterelement.h"
#include "svglayoutstate.h"
#include "svgrenderstate.h"

extern "C" {
#include "plutovg-filter.h"
}

namespace lunasvg {

// --- SVGFilterContext ---

SVGFilterContext::SVGFilterContext(const Canvas& sourceGraphic, const Rect& filterRegion)
    : m_sourceGraphic(sourceGraphic.surface())
    , m_filterRegion(filterRegion)
{
    plutovg_surface_reference(m_sourceGraphic);
}

SVGFilterContext::~SVGFilterContext()
{
    plutovg_surface_destroy(m_sourceGraphic);
    if(m_sourceAlpha)
        plutovg_surface_destroy(m_sourceAlpha);
    for(auto* s : m_ownedSurfaces)
        plutovg_surface_destroy(s);
}

plutovg_surface_t* SVGFilterContext::getInput(const std::string& name) const
{
    if(name.empty()) {
        return m_lastResult ? m_lastResult : m_sourceGraphic;
    }

    if(name == "SourceGraphic")
        return m_sourceGraphic;
    if(name == "SourceAlpha")
        return const_cast<SVGFilterContext*>(this)->sourceAlpha();

    auto it = m_buffers.find(name);
    if(it != m_buffers.end())
        return it->second;
    return m_lastResult ? m_lastResult : m_sourceGraphic;
}

void SVGFilterContext::setOutput(const std::string& name, plutovg_surface_t* surface)
{
    m_lastResult = surface;
    if(!name.empty()) {
        m_buffers[name] = surface;
    }
}

plutovg_surface_t* SVGFilterContext::sourceAlpha()
{
    if(m_sourceAlpha)
        return m_sourceAlpha;

    int w = plutovg_surface_get_width(m_sourceGraphic);
    int h = plutovg_surface_get_height(m_sourceGraphic);
    m_sourceAlpha = plutovg_surface_create(w, h);

    const uint32_t* src = (const uint32_t*)plutovg_surface_get_data(m_sourceGraphic);
    uint32_t* dst = (uint32_t*)plutovg_surface_get_data(m_sourceAlpha);
    int src_stride = plutovg_surface_get_stride(m_sourceGraphic) / 4;
    int dst_stride = plutovg_surface_get_stride(m_sourceAlpha) / 4;

    for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
            uint32_t alpha = (src[y * src_stride + x] >> 24) & 0xFF;
            dst[y * dst_stride + x] = alpha << 24; // premultiplied black with same alpha
        }
    }

    return m_sourceAlpha;
}

plutovg_surface_t* SVGFilterContext::createSurface() const
{
    int w = plutovg_surface_get_width(m_sourceGraphic);
    int h = plutovg_surface_get_height(m_sourceGraphic);
    auto* surface = plutovg_surface_create(w, h);
    const_cast<SVGFilterContext*>(this)->m_ownedSurfaces.push_back(surface);
    return surface;
}

// --- SVGFilterElement ---

SVGFilterElement::SVGFilterElement(Document* document)
    : SVGElement(document, ElementID::Filter)
    , m_filterUnits(PropertyID::FilterUnits, Units::ObjectBoundingBox)
    , m_primitiveUnits(PropertyID::PrimitiveUnits, Units::UserSpaceOnUse)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, -10.f, LengthUnits::Percent)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, -10.f, LengthUnits::Percent)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 120.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 120.f, LengthUnits::Percent)
{
    addProperty(m_filterUnits);
    addProperty(m_primitiveUnits);
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);
}

Rect SVGFilterElement::filterRegion(const SVGElement* target) const
{
    LengthContext lengthContext(this, m_filterUnits.value());
    auto x = lengthContext.valueForLength(m_x);
    auto y = lengthContext.valueForLength(m_y);
    auto w = lengthContext.valueForLength(m_width);
    auto h = lengthContext.valueForLength(m_height);

    if(m_filterUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = target->fillBoundingBox();
        x = bbox.x + x * bbox.w;
        y = bbox.y + y * bbox.h;
        w = w * bbox.w;
        h = h * bbox.h;
    }

    return Rect(x, y, w, h);
}

void SVGFilterElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
}

void SVGFilterElement::applyFilter(SVGRenderState& state) const
{
    auto& canvas = *state.canvas();
    auto region = filterRegion(state.element());
    auto deviceRegion = state.currentTransform().mapRect(region);
    deviceRegion.intersect(canvas.extents());

    if(deviceRegion.isEmpty())
        return;

    SVGFilterContext context(canvas, deviceRegion);

    // Execute each filter primitive child in document order
    for(auto& child : children()) {
        auto element = toSVGElement(child);
        if(!element)
            continue;
        auto primitive = dynamic_cast<const SVGFilterPrimitiveElement*>(element);
        if(!primitive)
            continue;
        primitive->apply(context);
    }

    // Replace canvas content with filter result
    auto* result = context.lastResult();
    if(!result)
        return;

    auto* dst = canvas.surface();
    int dst_width = plutovg_surface_get_width(dst);
    int dst_height = plutovg_surface_get_height(dst);
    int dst_stride = plutovg_surface_get_stride(dst);

    int src_width = plutovg_surface_get_width(result);
    int src_height = plutovg_surface_get_height(result);
    int src_stride = plutovg_surface_get_stride(result);

    auto* dst_data = plutovg_surface_get_data(dst);
    auto* src_data = plutovg_surface_get_data(result);

    // Clear the canvas
    memset(dst_data, 0, dst_height * dst_stride);

    // Copy result back: both surfaces have the same dimensions (filter region = canvas extents intersection)
    int copyW = (std::min)(dst_width, src_width);
    int copyH = (std::min)(dst_height, src_height);
    for(int y = 0; y < copyH; ++y) {
        memcpy(dst_data + y * dst_stride, src_data + y * src_stride, copyW * 4);
    }
}

// --- SVGFilterPrimitiveElement ---

SVGFilterPrimitiveElement::SVGFilterPrimitiveElement(Document* document, ElementID id)
    : SVGElement(document, id)
    , m_in(PropertyID::In)
    , m_result(PropertyID::Result)
{
    addProperty(m_in);
    addProperty(m_result);
}

// --- SVGFeFloodElement ---

SVGFeFloodElement::SVGFeFloodElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeFlood)
{
}

void SVGFeFloodElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Flood_Color) {
            // Parse flood-color: try currentColor first, then color value
            std::string_view input(attribute.value());
            plutovg_color_t color;
            int length = plutovg_color_parse(&color, input.data(), input.length());
            if(length > 0) {
                m_floodColor = Color(plutovg_color_to_argb32(&color));
            }
        } else if(attribute.id() == PropertyID::Flood_Opacity) {
            std::string_view input(attribute.value());
            float value = 1.f;
            // Simple float parse
            char* end;
            value = strtof(std::string(input).c_str(), &end);
            m_floodOpacity = std::clamp(value, 0.f, 1.f);
        }
    }
}

void SVGFeFloodElement::apply(SVGFilterContext& context) const
{
    auto* output = context.createSurface();
    auto r = m_floodColor.redF();
    auto g = m_floodColor.greenF();
    auto b = m_floodColor.blueF();
    auto a = m_floodColor.alphaF() * m_floodOpacity;

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_flood(output, r, g, b, a, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeOffsetElement ---

SVGFeOffsetElement::SVGFeOffsetElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeOffset)
    , m_dx(PropertyID::Dx, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_dy(PropertyID::Dy, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
{
    addProperty(m_dx);
    addProperty(m_dy);
}

void SVGFeOffsetElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    LengthContext lengthContext(this);
    int dx = (int)lengthContext.valueForLength(m_dx);
    int dy = (int)lengthContext.valueForLength(m_dy);

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_offset(output, input, dx, dy, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeMergeElement ---

SVGFeMergeElement::SVGFeMergeElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeMerge)
{
}

void SVGFeMergeElement::apply(SVGFilterContext& context) const
{
    std::vector<const plutovg_surface_t*> inputs;
    for(auto& child : children()) {
        auto element = toSVGElement(child);
        if(!element)
            continue;
        if(element->id() != ElementID::FeMergeNode)
            continue;
        auto mergeNode = static_cast<const SVGFeMergeNodeElement*>(element);
        auto* surface = context.getInput(mergeNode->in());
        if(surface) {
            inputs.push_back(surface);
        }
    }

    auto* output = context.createSurface();
    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);

    if(!inputs.empty()) {
        plutovg_filter_merge(output, inputs.data(), (int)inputs.size(), 0, 0, w, h);
    }

    context.setOutput(result(), output);
}

// --- SVGFeMergeNodeElement ---

SVGFeMergeNodeElement::SVGFeMergeNodeElement(Document* document)
    : SVGElement(document, ElementID::FeMergeNode)
    , m_in(PropertyID::In)
{
    addProperty(m_in);
}

// --- SVGFeGaussianBlurElement ---

SVGFeGaussianBlurElement::SVGFeGaussianBlurElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeGaussianBlur)
{
}

void SVGFeGaussianBlurElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::StdDeviation) {
            std::string_view input(attribute.value());
            // Parse one or two float values
            char* end;
            m_stdDeviationX = strtof(std::string(input).c_str(), &end);
            if(m_stdDeviationX < 0.f) m_stdDeviationX = 0.f;
            // Skip whitespace/comma
            const char* p = end;
            while(*p == ' ' || *p == ',' || *p == '\t') ++p;
            if(*p != '\0') {
                m_stdDeviationY = strtof(p, &end);
                if(m_stdDeviationY < 0.f) m_stdDeviationY = 0.f;
            } else {
                m_stdDeviationY = m_stdDeviationX;
            }
        }
    }
}

void SVGFeGaussianBlurElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_gaussian_blur(output, input, m_stdDeviationX, m_stdDeviationY, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeBlendElement ---

SVGFeBlendElement::SVGFeBlendElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeBlend)
    , m_in2(PropertyID::In2)
{
    addProperty(m_in2);
}

void SVGFeBlendElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Mode) {
            auto val = attribute.value();
            if(val == "multiply") m_mode = 1;
            else if(val == "screen") m_mode = 2;
            else if(val == "darken") m_mode = 3;
            else if(val == "lighten") m_mode = 4;
            else m_mode = 0; // normal
        }
    }
}

void SVGFeBlendElement::apply(SVGFilterContext& context) const
{
    auto* in1 = context.getInput(in());
    auto* in2 = context.getInput(m_in2.value());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_blend(output, in1, in2, m_mode, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeColorMatrixElement ---

SVGFeColorMatrixElement::SVGFeColorMatrixElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeColorMatrix)
{
    // Default to identity matrix
    memset(m_matrix, 0, sizeof(m_matrix));
    m_matrix[0] = 1.f;  // R->R
    m_matrix[6] = 1.f;  // G->G
    m_matrix[12] = 1.f; // B->B
    m_matrix[18] = 1.f; // A->A
}

void SVGFeColorMatrixElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    float rawValue = 0.f;
    bool hasValues = false;
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Type) {
            auto val = attribute.value();
            if(val == "saturate") m_type = 1;
            else if(val == "hueRotate") m_type = 2;
            else if(val == "luminanceToAlpha") m_type = 3;
            else m_type = 0; // matrix
        } else if(attribute.id() == PropertyID::Values) {
            hasValues = true;
            std::string_view input(attribute.value());
            std::string str(input);
            const char* p = str.c_str();
            float vals[20];
            int nvals = 0;
            while(*p && nvals < 20) {
                while(*p == ' ' || *p == ',' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
                if(*p == '\0') break;
                char* end;
                vals[nvals++] = strtof(p, &end);
                p = end;
            }
            if(nvals >= 20) {
                memcpy(m_matrix, vals, 20 * sizeof(float));
            } else if(nvals >= 1) {
                rawValue = vals[0];
            }
        }
    }

    // Build the actual matrix based on type
    if(m_type == 1) {
        // saturate: values="<number>" in [0,1]
        float s = hasValues ? rawValue : 1.f;
        memset(m_matrix, 0, sizeof(m_matrix));
        m_matrix[0]  = 0.213f + 0.787f * s;
        m_matrix[1]  = 0.715f - 0.715f * s;
        m_matrix[2]  = 0.072f - 0.072f * s;
        m_matrix[5]  = 0.213f - 0.213f * s;
        m_matrix[6]  = 0.715f + 0.285f * s;
        m_matrix[7]  = 0.072f - 0.072f * s;
        m_matrix[10] = 0.213f - 0.213f * s;
        m_matrix[11] = 0.715f - 0.715f * s;
        m_matrix[12] = 0.072f + 0.928f * s;
        m_matrix[18] = 1.f;
    } else if(m_type == 2) {
        // hueRotate: values="<angle>" in degrees
        float angle = (hasValues ? rawValue : 0.f) * 3.14159265358979f / 180.f;
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        memset(m_matrix, 0, sizeof(m_matrix));
        m_matrix[0]  = 0.213f + 0.787f * cosA - 0.213f * sinA;
        m_matrix[1]  = 0.715f - 0.715f * cosA - 0.715f * sinA;
        m_matrix[2]  = 0.072f - 0.072f * cosA + 0.928f * sinA;
        m_matrix[5]  = 0.213f - 0.213f * cosA + 0.143f * sinA;
        m_matrix[6]  = 0.715f + 0.285f * cosA + 0.140f * sinA;
        m_matrix[7]  = 0.072f - 0.072f * cosA - 0.283f * sinA;
        m_matrix[10] = 0.213f - 0.213f * cosA - 0.787f * sinA;
        m_matrix[11] = 0.715f - 0.715f * cosA + 0.715f * sinA;
        m_matrix[12] = 0.072f + 0.928f * cosA + 0.072f * sinA;
        m_matrix[18] = 1.f;
    } else if(m_type == 3) {
        // luminanceToAlpha
        memset(m_matrix, 0, sizeof(m_matrix));
        m_matrix[15] = 0.2126f;
        m_matrix[16] = 0.7152f;
        m_matrix[17] = 0.0722f;
    }
    // else m_type == 0: m_matrix was already filled from values
}

void SVGFeColorMatrixElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_color_matrix(output, input, m_matrix, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeCompositeElement ---

SVGFeCompositeElement::SVGFeCompositeElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeComposite)
    , m_in2(PropertyID::In2)
{
    addProperty(m_in2);
}

void SVGFeCompositeElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Operator) {
            auto val = attribute.value();
            if(val == "in") m_operator = 1;
            else if(val == "out") m_operator = 2;
            else if(val == "atop") m_operator = 3;
            else if(val == "xor") m_operator = 4;
            else if(val == "arithmetic") m_operator = 5;
            else m_operator = 0; // over
        } else if(attribute.id() == PropertyID::K1) {
            m_k1 = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::K2) {
            m_k2 = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::K3) {
            m_k3 = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::K4) {
            m_k4 = strtof(std::string(attribute.value()).c_str(), nullptr);
        }
    }
}

void SVGFeCompositeElement::apply(SVGFilterContext& context) const
{
    auto* in1 = context.getInput(in());
    auto* in2 = context.getInput(m_in2.value());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_composite(output, in1, in2, m_operator, m_k1, m_k2, m_k3, m_k4, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeComponentTransferElement ---

SVGFeComponentTransferElement::SVGFeComponentTransferElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeComponentTransfer)
{
}

void SVGFeComponentTransferElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    // Build identity defaults
    plutovg_transfer_func_t funcR = {0, nullptr, 0, 1.f, 0.f, 1.f, 1.f, 0.f};
    plutovg_transfer_func_t funcG = funcR;
    plutovg_transfer_func_t funcB = funcR;
    plutovg_transfer_func_t funcA = funcR;

    // Find feFuncR/G/B/A children
    for(auto& child : children()) {
        auto element = toSVGElement(child);
        if(!element)
            continue;
        auto func = dynamic_cast<const SVGFeFuncElement*>(element);
        if(!func)
            continue;
        plutovg_transfer_func_t tf;
        tf.type = func->funcType();
        tf.tableValues = func->tableValues().empty() ? nullptr : func->tableValues().data();
        tf.tableSize = (int)func->tableValues().size();
        tf.slope = func->slope();
        tf.intercept = func->intercept();
        tf.amplitude = func->amplitude();
        tf.exponent = func->exponent();
        tf.offset = func->offset();

        switch(element->id()) {
        case ElementID::FeFuncR: funcR = tf; break;
        case ElementID::FeFuncG: funcG = tf; break;
        case ElementID::FeFuncB: funcB = tf; break;
        case ElementID::FeFuncA: funcA = tf; break;
        default: break;
        }
    }

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_component_transfer(output, input, &funcR, &funcG, &funcB, &funcA, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeFuncElement ---

SVGFeFuncElement::SVGFeFuncElement(Document* document, ElementID id)
    : SVGElement(document, id)
{
}

static int parseFuncType(const std::string& val)
{
    if(val == "table") return 1;
    if(val == "discrete") return 2;
    if(val == "linear") return 3;
    if(val == "gamma") return 4;
    return 0; // identity
}

static std::vector<float> parseFloatList(const std::string& val)
{
    std::vector<float> result;
    const char* p = val.c_str();
    while(*p) {
        while(*p == ' ' || *p == ',' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
        if(*p == '\0') break;
        char* end;
        float v = strtof(p, &end);
        if(end == p) break;
        result.push_back(v);
        p = end;
    }
    return result;
}

void SVGFeFuncElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        switch(attribute.id()) {
        case PropertyID::Type:
            m_funcType = parseFuncType(attribute.value());
            break;
        case PropertyID::TableValues:
            m_tableValues = parseFloatList(attribute.value());
            break;
        case PropertyID::Slope:
            m_slope = strtof(std::string(attribute.value()).c_str(), nullptr);
            break;
        case PropertyID::Intercept:
            m_intercept = strtof(std::string(attribute.value()).c_str(), nullptr);
            break;
        case PropertyID::Amplitude:
            m_amplitude = strtof(std::string(attribute.value()).c_str(), nullptr);
            break;
        case PropertyID::Exponent:
            m_exponent = strtof(std::string(attribute.value()).c_str(), nullptr);
            break;
        case PropertyID::Offset:
            m_offset = strtof(std::string(attribute.value()).c_str(), nullptr);
            break;
        default: break;
        }
    }
}

// --- SVGFeMorphologyElement ---

SVGFeMorphologyElement::SVGFeMorphologyElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeMorphology)
{
}

void SVGFeMorphologyElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Operator) {
            auto val = attribute.value();
            m_operator = (val == "dilate") ? 1 : 0;
        } else if(attribute.id() == PropertyID::Radius) {
            std::string str(attribute.value());
            char* end;
            m_radiusX = strtof(str.c_str(), &end);
            if(m_radiusX < 0.f) m_radiusX = 0.f;
            const char* p = end;
            while(*p == ' ' || *p == ',' || *p == '\t') ++p;
            if(*p != '\0') {
                m_radiusY = strtof(p, &end);
                if(m_radiusY < 0.f) m_radiusY = 0.f;
            } else {
                m_radiusY = m_radiusX;
            }
        }
    }
}

void SVGFeMorphologyElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_morphology(output, input, m_operator, m_radiusX, m_radiusY, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeTileElement ---

SVGFeTileElement::SVGFeTileElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeTile)
{
}

void SVGFeTileElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    // Use full input dimensions as source tile region
    int srcW = plutovg_surface_get_width(input);
    int srcH = plutovg_surface_get_height(input);
    plutovg_filter_tile(output, input, 0, 0, srcW, srcH, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeImageElement (stub) ---

SVGFeImageElement::SVGFeImageElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeImage)
{
}

void SVGFeImageElement::apply(SVGFilterContext& context) const
{
    // Stub: no external image loading support
    // Just pass through SourceGraphic
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    int srcW = plutovg_surface_get_width(input);
    int srcH = plutovg_surface_get_height(input);
    int copyW = (std::min)(w, srcW);
    int copyH = (std::min)(h, srcH);
    int dstStride = plutovg_surface_get_stride(output);
    int srcStride = plutovg_surface_get_stride(input);
    auto* dstData = plutovg_surface_get_data(output);
    auto* srcData = plutovg_surface_get_data(input);
    for(int y = 0; y < copyH; ++y) {
        memcpy(dstData + y * dstStride, srcData + y * srcStride, copyW * 4);
    }
    context.setOutput(result(), output);
}

// --- SVGFeTurbulenceElement ---

SVGFeTurbulenceElement::SVGFeTurbulenceElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeTurbulence)
{
}

void SVGFeTurbulenceElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::BaseFrequency) {
            std::string str(attribute.value());
            char* end;
            m_baseFrequencyX = strtof(str.c_str(), &end);
            const char* p = end;
            while(*p == ' ' || *p == ',' || *p == '\t') ++p;
            if(*p != '\0') {
                m_baseFrequencyY = strtof(p, &end);
            } else {
                m_baseFrequencyY = m_baseFrequencyX;
            }
        } else if(attribute.id() == PropertyID::NumOctaves) {
            m_numOctaves = (int)strtol(std::string(attribute.value()).c_str(), nullptr, 10);
            if(m_numOctaves < 1) m_numOctaves = 1;
        } else if(attribute.id() == PropertyID::Seed) {
            m_seed = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::StitchTiles) {
            m_stitchTiles = (attribute.value() == "stitch") ? 1 : 0;
        } else if(attribute.id() == PropertyID::Type) {
            m_type = (attribute.value() == "fractalNoise") ? 1 : 0;
        }
    }
}

void SVGFeTurbulenceElement::apply(SVGFilterContext& context) const
{
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_turbulence(output, m_baseFrequencyX, m_baseFrequencyY,
        m_numOctaves, m_seed, m_stitchTiles, m_type, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeConvolveMatrixElement ---

SVGFeConvolveMatrixElement::SVGFeConvolveMatrixElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeConvolveMatrix)
{
}

void SVGFeConvolveMatrixElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Order) {
            std::string str(attribute.value());
            char* end;
            m_orderX = (int)strtol(str.c_str(), &end, 10);
            if(m_orderX < 1) m_orderX = 3;
            const char* p = end;
            while(*p == ' ' || *p == ',' || *p == '\t') ++p;
            if(*p != '\0') {
                m_orderY = (int)strtol(p, &end, 10);
                if(m_orderY < 1) m_orderY = 3;
            } else {
                m_orderY = m_orderX;
            }
        } else if(attribute.id() == PropertyID::KernelMatrix) {
            m_kernelMatrix = parseFloatList(attribute.value());
        } else if(attribute.id() == PropertyID::TargetX) {
            m_targetX = (int)strtol(std::string(attribute.value()).c_str(), nullptr, 10);
        } else if(attribute.id() == PropertyID::TargetY) {
            m_targetY = (int)strtol(std::string(attribute.value()).c_str(), nullptr, 10);
        } else if(attribute.id() == PropertyID::Divisor) {
            m_divisor = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Bias) {
            m_bias = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::EdgeMode) {
            auto val = attribute.value();
            if(val == "wrap") m_edgeMode = 1;
            else if(val == "none") m_edgeMode = 2;
            else m_edgeMode = 0; // duplicate
        } else if(attribute.id() == PropertyID::PreserveAlpha) {
            m_preserveAlpha = (attribute.value() == "true");
        }
    }
}

void SVGFeConvolveMatrixElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);

    // Use defaults if targetX/Y not set
    int tx = (m_targetX >= 0) ? m_targetX : m_orderX / 2;
    int ty = (m_targetY >= 0) ? m_targetY : m_orderY / 2;

    const float* kernel = m_kernelMatrix.empty() ? nullptr : m_kernelMatrix.data();
    plutovg_filter_convolve_matrix(output, input, kernel, m_orderX, m_orderY,
        tx, ty, m_divisor, m_bias, m_edgeMode, m_preserveAlpha ? 1 : 0, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeDisplacementMapElement ---

SVGFeDisplacementMapElement::SVGFeDisplacementMapElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeDisplacementMap)
    , m_in2(PropertyID::In2)
{
    addProperty(m_in2);
}

static int parseChannelSelector(const std::string& val)
{
    if(val == "R") return 0;
    if(val == "G") return 1;
    if(val == "B") return 2;
    return 3; // A (default)
}

void SVGFeDisplacementMapElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Scale) {
            m_scale = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::XChannelSelector) {
            m_xChannelSelector = parseChannelSelector(attribute.value());
        } else if(attribute.id() == PropertyID::YChannelSelector) {
            m_yChannelSelector = parseChannelSelector(attribute.value());
        }
    }
}

void SVGFeDisplacementMapElement::apply(SVGFilterContext& context) const
{
    auto* in1 = context.getInput(in());
    auto* in2 = context.getInput(m_in2.value());
    auto* output = context.createSurface();

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_displacement_map(output, in1, in2, m_scale, m_xChannelSelector, m_yChannelSelector, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeDistantLightElement ---

SVGFeDistantLightElement::SVGFeDistantLightElement(Document* document)
    : SVGElement(document, ElementID::FeDistantLight)
{
}

void SVGFeDistantLightElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::Azimuth) {
            m_azimuth = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Elevation) {
            m_elevation = strtof(std::string(attribute.value()).c_str(), nullptr);
        }
    }
}

// --- SVGFePointLightElement ---

SVGFePointLightElement::SVGFePointLightElement(Document* document)
    : SVGElement(document, ElementID::FePointLight)
{
}

void SVGFePointLightElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::X) {
            m_x = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Y) {
            m_y = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Z) {
            m_z = strtof(std::string(attribute.value()).c_str(), nullptr);
        }
    }
}

// --- SVGFeSpotLightElement ---

SVGFeSpotLightElement::SVGFeSpotLightElement(Document* document)
    : SVGElement(document, ElementID::FeSpotLight)
{
}

void SVGFeSpotLightElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::X) {
            m_x = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Y) {
            m_y = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::Z) {
            m_z = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::PointsAtX) {
            m_pointsAtX = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::PointsAtY) {
            m_pointsAtY = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::PointsAtZ) {
            m_pointsAtZ = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::SpecularExponent) {
            m_specularExponent = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::LimitingConeAngle) {
            m_limitingConeAngle = strtof(std::string(attribute.value()).c_str(), nullptr);
            m_hasLimitingConeAngle = true;
        }
    }
}

// Helper: find light source child and fill plutovg_light_source_t
static bool findLightSource(const SVGElement* parent, plutovg_light_source_t& light)
{
    memset(&light, 0, sizeof(light));
    light.specularExponent = 1.0f;

    for(auto& child : parent->children()) {
        auto element = toSVGElement(child);
        if(!element)
            continue;
        if(element->id() == ElementID::FeDistantLight) {
            auto dl = static_cast<const SVGFeDistantLightElement*>(element);
            light.type = PLUTOVG_LIGHT_DISTANT;
            light.azimuth = dl->azimuth();
            light.elevation = dl->elevation();
            return true;
        } else if(element->id() == ElementID::FePointLight) {
            auto pl = static_cast<const SVGFePointLightElement*>(element);
            light.type = PLUTOVG_LIGHT_POINT;
            light.x = pl->x();
            light.y = pl->y();
            light.z = pl->z();
            return true;
        } else if(element->id() == ElementID::FeSpotLight) {
            auto sl = static_cast<const SVGFeSpotLightElement*>(element);
            light.type = PLUTOVG_LIGHT_SPOT;
            light.x = sl->x();
            light.y = sl->y();
            light.z = sl->z();
            light.pointsAtX = sl->pointsAtX();
            light.pointsAtY = sl->pointsAtY();
            light.pointsAtZ = sl->pointsAtZ();
            light.specularExponent = sl->spotExponent();
            light.limitingConeAngle = sl->limitingConeAngle();
            light.hasLimitingConeAngle = sl->hasLimitingConeAngle() ? 1 : 0;
            return true;
        }
    }
    return false;
}

// --- SVGFeDiffuseLightingElement ---

SVGFeDiffuseLightingElement::SVGFeDiffuseLightingElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeDiffuseLighting)
{
}

void SVGFeDiffuseLightingElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::SurfaceScale) {
            m_surfaceScale = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::DiffuseConstant) {
            m_diffuseConstant = strtof(std::string(attribute.value()).c_str(), nullptr);
            if(m_diffuseConstant < 0.f) m_diffuseConstant = 0.f;
        } else if(attribute.id() == PropertyID::Lighting_Color) {
            std::string_view input(attribute.value());
            plutovg_color_t color;
            int length = plutovg_color_parse(&color, input.data(), input.length());
            if(length > 0) {
                m_lightingColor = Color(plutovg_color_to_argb32(&color));
            }
        }
    }
}

void SVGFeDiffuseLightingElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    plutovg_light_source_t light;
    if(!findLightSource(this, light)) {
        // No light source — output black
        context.setOutput(result(), output);
        return;
    }

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_diffuse_lighting(output, input,
        m_surfaceScale, m_diffuseConstant,
        m_lightingColor.redF(), m_lightingColor.greenF(), m_lightingColor.blueF(),
        &light, 0, 0, w, h);
    context.setOutput(result(), output);
}

// --- SVGFeSpecularLightingElement ---

SVGFeSpecularLightingElement::SVGFeSpecularLightingElement(Document* document)
    : SVGFilterPrimitiveElement(document, ElementID::FeSpecularLighting)
{
}

void SVGFeSpecularLightingElement::layoutElement(const SVGLayoutState& state)
{
    SVGElement::layoutElement(state);
    for(const auto& attribute : attributes()) {
        if(attribute.id() == PropertyID::SurfaceScale) {
            m_surfaceScale = strtof(std::string(attribute.value()).c_str(), nullptr);
        } else if(attribute.id() == PropertyID::SpecularConstant) {
            m_specularConstant = strtof(std::string(attribute.value()).c_str(), nullptr);
            if(m_specularConstant < 0.f) m_specularConstant = 0.f;
        } else if(attribute.id() == PropertyID::SpecularExponent) {
            m_specularExponent = strtof(std::string(attribute.value()).c_str(), nullptr);
            m_specularExponent = std::clamp(m_specularExponent, 1.f, 128.f);
        } else if(attribute.id() == PropertyID::Lighting_Color) {
            std::string_view input(attribute.value());
            plutovg_color_t color;
            int length = plutovg_color_parse(&color, input.data(), input.length());
            if(length > 0) {
                m_lightingColor = Color(plutovg_color_to_argb32(&color));
            }
        }
    }
}

void SVGFeSpecularLightingElement::apply(SVGFilterContext& context) const
{
    auto* input = context.getInput(in());
    auto* output = context.createSurface();

    plutovg_light_source_t light;
    if(!findLightSource(this, light)) {
        context.setOutput(result(), output);
        return;
    }

    int w = plutovg_surface_get_width(output);
    int h = plutovg_surface_get_height(output);
    plutovg_filter_specular_lighting(output, input,
        m_surfaceScale, m_specularConstant, m_specularExponent,
        m_lightingColor.redF(), m_lightingColor.greenF(), m_lightingColor.blueF(),
        &light, 0, 0, w, h);
    context.setOutput(result(), output);
}

} // namespace lunasvg
