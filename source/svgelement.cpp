#include "svgelement.h"
#include "svgpaintelement.h"
#include "svggeometryelement.h"
#include "svgtextelement.h"
#include "svgproperty.h"
#include "svglayoutstate.h"
#include "svgrenderstate.h"

#include <cassert>

namespace lunasvg {

// user-defined implementation (checkbox shown as example)
bool createNamespaceElement(ElementID id, std::string& markup);
#if 0
{
    markup = std::string("<svg viewbox=\"0 0 32 32\"><path id=\"border\" d=\"M26,4H6A2,2,0,0,0,4,6V26a2,2,0,0,0,2,2H26a2,2,0,0,0,2-2V6A2,2,0,0,0,26,4ZM6,26V6H26V26Z\"/><polygon id=\"check\" points=\"14 21.5 9 16.54 10.59 15 14 18.35 21.41 11 23 12.58 14 21.5\"/><rect style=\"fill: none;\" width=\"32\" height=\"32\"/></svg>");

    return true;
}
#endif

ElementID hashelementid(const std::string_view& name)
{
    union NamespaceElementID
    {
        ElementID id;
        struct
        {
            uint32_t elementId;
            uint32_t namespaceId;
        } hash;

        operator ElementID() { return id; }
    };

    if(!name.empty() && name.find(':') != std::string_view::npos) {
        NamespaceElementID id;

        auto separator = name.find(':');

        if(separator <= 0) {
            id.hash.elementId = (uint32_t) std::hash<std::string_view>{}(name);
            id.hash.namespaceId = 0;
        }
        else {
            id.hash.elementId = (uint32_t) std::hash<std::string_view>{}(name.substr(separator + 1));
            id.hash.namespaceId = (uint32_t) std::hash<std::string_view>{}(name.substr(0, separator));
        }

        if(id.hash.elementId < 256)
            id.hash.elementId |= 0xFFFFFF00; // avoid collisions with original enum (uint8_t) values

        return id;
    }

    return ElementID::Unknown;
}

ElementID elementid(const std::string_view& name)
{
    static const struct {
        std::string_view name;
        ElementID value;
    } table[] = {
        {"a", ElementID::G},
        {"animate", ElementID::Animate},
        {"animateMotion", ElementID::AnimateMotion},
        {"animateTransform", ElementID::AnimateTransform},
        {"circle", ElementID::Circle},
        {"clipPath", ElementID::ClipPath},
        {"conicalGradient", ElementID::ConicalGradient},
        {"defs", ElementID::Defs},
        {"ellipse", ElementID::Ellipse},
        {"foreignObject", ElementID::ForeignObject},
        {"g", ElementID::G},
        {"image", ElementID::Image},
        {"line", ElementID::Line},
        {"linearGradient", ElementID::LinearGradient},
        {"marker", ElementID::Marker},
        {"mask", ElementID::Mask},
        {"path", ElementID::Path},
        {"pattern", ElementID::Pattern},
        {"polygon", ElementID::Polygon},
        {"polyline", ElementID::Polyline},
        {"radialGradient", ElementID::RadialGradient},
        {"rect", ElementID::Rect},
        {"set", ElementID::Set},
        {"stop", ElementID::Stop},
        {"style", ElementID::Style},
        {"svg", ElementID::Svg},
        {"symbol", ElementID::Symbol},
        {"text", ElementID::Text},
        {"tspan", ElementID::Tspan},
        {"use", ElementID::Use}
    };

    auto it = std::lower_bound(table, std::end(table), name, [](const auto& item, const auto& name) { return item.name < name; });
    if(it == std::end(table) || it->name != name)
        return !false ? hashelementid(name) : ElementID::Unknown;
    return it->value;
}

SVGTextNode::SVGTextNode(Document* document)
    : SVGNode(document)
{
}

void SVGTextNode::setData(const std::string& data)
{
    rootElement()->setNeedsLayout();
    m_data.assign(data);
}

std::unique_ptr<SVGNode> SVGTextNode::clone(bool deep) const
{
    auto node = std::make_unique<SVGTextNode>(document());
    node->setData(m_data);
    return node;
}

const std::string emptyString;

std::unique_ptr<SVGElement> SVGElement::create(Document* document, ElementID id, SVGElement* parent)
{
    switch(id) {
    case ElementID::Svg:
        return std::make_unique<SVGSVGElement>(document);
    case ElementID::Path:
        return std::make_unique<SVGPathElement>(document);
    case ElementID::G:
        return std::make_unique<SVGGElement>(document);
    case ElementID::Rect:
        return std::make_unique<SVGRectElement>(document);
    case ElementID::Circle:
        return std::make_unique<SVGCircleElement>(document);
    case ElementID::Ellipse:
        return std::make_unique<SVGEllipseElement>(document);
    case ElementID::ForeignObject:
        return std::make_unique<SVGForeignObjectElement>(document);
    case ElementID::Line:
        return std::make_unique<SVGLineElement>(document);
    case ElementID::Defs:
        return std::make_unique<SVGDefsElement>(document);
    case ElementID::Polygon:
    case ElementID::Polyline:
        return std::make_unique<SVGPolyElement>(document, id);
    case ElementID::Stop:
        return std::make_unique<SVGStopElement>(document);
    case ElementID::LinearGradient:
        return std::make_unique<SVGLinearGradientElement>(document);
    case ElementID::RadialGradient:
        return std::make_unique<SVGRadialGradientElement>(document);
    case ElementID::ConicalGradient:
        return std::make_unique<SVGConicalGradientElement>(document);
    case ElementID::Symbol:
        return std::make_unique<SVGSymbolElement>(document);
    case ElementID::Use:
        return std::make_unique<SVGUseElement>(document);
    case ElementID::Pattern:
        return std::make_unique<SVGPatternElement>(document);
    case ElementID::Mask:
        return std::make_unique<SVGMaskElement>(document);
    case ElementID::ClipPath:
        return std::make_unique<SVGClipPathElement>(document);
    case ElementID::Marker:
        return std::make_unique<SVGMarkerElement>(document);
    case ElementID::Image:
        return std::make_unique<SVGImageElement>(document);
    case ElementID::Style:
        return std::make_unique<SVGStyleElement>(document);
    case ElementID::Text:
        return std::make_unique<SVGTextElement>(document);
    case ElementID::Tspan:
        return std::make_unique<SVGTSpanElement>(document);
    case ElementID::Animate:
        return std::make_unique<SVGAnimateElement>(document);
    case ElementID::AnimateMotion:
        return std::make_unique<SVGAnimateMotionElement>(document);
    case ElementID::AnimateTransform:
        return std::make_unique<SVGAnimateTransformElement>(document);
    case ElementID::Set:
        return std::make_unique<SVGSetElement>(document);
    default:

        if(parent && parent->id() == ElementID::ForeignObject) {
            return static_cast<SVGForeignObjectElement*>(parent)->loadContent(document, id);
        }

        assert(false);
    }

    return nullptr;
}

SVGElement::SVGElement(Document* document, ElementID id)
    : SVGNode(document)
    , m_id(id)
{
}

bool SVGElement::hasAttribute(const std::string_view& name) const
{
    auto id = propertyid(name);
    if(id == PropertyID::Unknown)
        return false;
    return hasAttribute(id);
}

const std::string& SVGElement::getAttribute(const std::string_view& name) const
{
    auto id = propertyid(name);
    if(id == PropertyID::Unknown)
        return emptyString;
    return getAttribute(id);
}

bool SVGElement::setAttribute(const std::string_view& name, const std::string& value)
{
    auto id = propertyid(name);
    if(id == PropertyID::Unknown)
        return false;
    return setAttribute(0x1000, id, value);
}

const Attribute* SVGElement::findAttribute(PropertyID id) const
{
    for(const auto& attribute : m_attributes) {
        if(id == attribute.id()) {
            return &attribute;
        }
    }

    return nullptr;
}

bool SVGElement::hasAttribute(PropertyID id) const
{
    for(const auto& attribute : m_attributes) {
        if(id == attribute.id()) {
            return true;
        }
    }

    return false;
}

const std::string& SVGElement::getAttribute(PropertyID id) const
{
    for(const auto& attribute : m_attributes) {
        if(id == attribute.id()) {
            return attribute.value();
        }
    }

    return emptyString;
}

bool SVGElement::setAttribute(int specificity, PropertyID id, const std::string& value)
{
    for(auto& attribute : m_attributes) {
        if(id == attribute.id()) {
            if(specificity < attribute.specificity())
                return false;
            parseAttribute(id, value);
            attribute = Attribute(specificity, id, value);
            return true;
        }
    }

    parseAttribute(id, value);
    m_attributes.emplace_front(specificity, id, value);
    return true;
}

void SVGElement::setAttributes(const AttributeList& attributes)
{
    for(const auto& attribute : attributes) {
        setAttribute(attribute);
    }
}

bool SVGElement::setAttribute(const Attribute& attribute)
{
    return setAttribute(attribute.specificity(), attribute.id(), attribute.value());
}

void SVGElement::parseAttribute(PropertyID id, const std::string& value)
{
    rootElement()->setNeedsLayout();
    if(auto property = getProperty(id)) {
        property->parse(value);
    }
}

SVGElement* SVGElement::previousElement() const
{
    auto parent = parentElement();
    if(parent == nullptr)
        return nullptr;
    const auto& children = parent->children();
    auto it = children.begin();
    auto end = children.end();
    SVGElement* element = nullptr;
    for(; it != end; ++it) {
        SVGNode* node = &**it;
        if(node->isTextNode())
            continue;
        if(node == this)
            return element;
        element = static_cast<SVGElement*>(node);
    }

    return nullptr;
}

SVGElement* SVGElement::nextElement() const
{
    auto parent = parentElement();
    if(parent == nullptr)
        return nullptr;
    const auto& children = parent->children();
    auto it = children.rbegin();
    auto end = children.rend();
    SVGElement* element = nullptr;
    for(; it != end; ++it) {
        SVGNode* node = &**it;
        if(node->isTextNode())
            continue;
        if(node == this)
            return element;
        element = static_cast<SVGElement*>(node);
    }

    return nullptr;
}

SVGNode* SVGElement::addChild(std::unique_ptr<SVGNode> child)
{
    child->setParentElement(this);
    m_children.push_back(std::move(child));
    return &*m_children.back();
}

SVGNode* SVGElement::firstChild() const
{
    if(m_children.empty())
        return nullptr;
    return &*m_children.front();
}

SVGNode* SVGElement::lastChild() const
{
    if(m_children.empty())
        return nullptr;
    return &*m_children.back();
}

Rect SVGElement::fillBoundingBox() const
{
    auto fillBoundingBox = Rect::Invalid;
    for(const auto& child : m_children) {
        auto element = toSVGElement(child);
        if(element && !element->isHiddenElement()) {
            fillBoundingBox.unite(element->localTransform().mapRect(element->fillBoundingBox()));
        }
    }

    if(!fillBoundingBox.isValid())
        fillBoundingBox = Rect::Empty;
    return fillBoundingBox;
}

Rect SVGElement::strokeBoundingBox() const
{
    auto strokeBoundingBox = Rect::Invalid;
    for(const auto& child : m_children) {
        auto element = toSVGElement(child);
        if(element && !element->isHiddenElement()) {
            strokeBoundingBox.unite(element->localTransform().mapRect(element->strokeBoundingBox()));
        }
    }

    if(!strokeBoundingBox.isValid())
        strokeBoundingBox = Rect::Empty;
    return strokeBoundingBox;
}

Rect SVGElement::paintBoundingBox() const
{
    if(m_paintBoundingBox.isValid())
        return m_paintBoundingBox;
    m_paintBoundingBox = Rect::Empty;
    m_paintBoundingBox = strokeBoundingBox();
    assert(m_paintBoundingBox.isValid());
    if(m_clipper) m_paintBoundingBox.intersect(m_clipper->clipBoundingBox(this));
    if(m_masker) m_paintBoundingBox.intersect(m_masker->maskBoundingBox(this));
    return m_paintBoundingBox;
}

SVGMarkerElement* SVGElement::getMarker(const std::string_view& id) const
{
    auto element = rootElement()->getElementById(id);
    if(element && element->id() == ElementID::Marker)
        return static_cast<SVGMarkerElement*>(element);
    return nullptr;
}

SVGClipPathElement* SVGElement::getClipper(const std::string_view& id) const
{
    auto element = rootElement()->getElementById(id);
    if(element && element->id() == ElementID::ClipPath)
        return static_cast<SVGClipPathElement*>(element);
    return nullptr;
}

SVGMaskElement* SVGElement::getMasker(const std::string_view& id) const
{
    auto element = rootElement()->getElementById(id);
    if(element && element->id() == ElementID::Mask)
        return static_cast<SVGMaskElement*>(element);
    return nullptr;
}

SVGPaintElement* SVGElement::getPainter(const std::string_view& id) const
{
    auto element = rootElement()->getElementById(id);
    if(element && element->isPaintElement())
        return static_cast<SVGPaintElement*>(element);
    return nullptr;
}

SVGElement* SVGElement::elementFromPoint(float x, float y)
{
    auto it = m_children.rbegin();
    auto end = m_children.rend();
    for(; it != end; ++it) {
        auto child = toSVGElement(*it);
        if(child && !child->isHiddenElement()) {
            if(auto element = child->elementFromPoint(x, y)) {
                return element;
            }
        }
    }

    if(isPointableElement()) {
        auto transform = localTransform();
        for(auto parent = parentElement(); parent; parent = parent->parentElement())
            transform.postMultiply(parent->localTransform());
        auto bbox = transform.mapRect(paintBoundingBox());
        if(bbox.contains(x, y)) {
            return this;
        }
    }

    return nullptr;
}

void SVGElement::addProperty(SVGProperty& value)
{
    m_properties.push_front(&value);
}

SVGProperty* SVGElement::getProperty(PropertyID id) const
{
    for(auto property : m_properties) {
        if(id == property->id()) {
            return property;
        }
    }

    return nullptr;
}

Size SVGElement::currentViewportSize() const
{
    auto parent = parentElement();
    if(parent == nullptr) {
        auto element = static_cast<const SVGSVGElement*>(this);
        const auto& viewBox = element->viewBox();
        if(viewBox.value().isValid())
            return viewBox.value().size();
        return Size(300, 150);
    }

    switch (parent->id())
    {
        case ElementID::Svg:
        case ElementID::ForeignObject:
        {
            auto element = static_cast<const SVGSVGElement*>(parent);
            const auto& viewBox = element->viewBox();
            if(viewBox.value().isValid())
                return viewBox.value().size();
            LengthContext lengthContext(element);
            auto width = lengthContext.valueForLength(element->width());
            auto height = lengthContext.valueForLength(element->height());
            return Size(width, height);
        }
    }

    return parent->currentViewportSize();
}

void SVGElement::cloneChildren(SVGElement* parentElement) const
{
    for(const auto& child : m_children) {
        parentElement->addChild(child->clone(true));
    }
}

std::unique_ptr<SVGNode> SVGElement::clone(bool deep) const
{
    auto element = SVGElement::create(document(), m_id);
    element->setAttributes(m_attributes);
    if(deep) { cloneChildren(element.get()); }
    return element;
}

void SVGElement::build()
{
    for(const auto& child : m_children) {
        if(auto element = toSVGElement(child)) {
            element->build();
        }
    }
}

void SVGElement::layoutElement(const SVGLayoutState& state)
{
    m_paintBoundingBox = Rect::Invalid;
    m_clipper = getClipper(state.clip_path());
    m_masker = getMasker(state.mask());
    m_opacity = state.opacity();

    m_font_size = state.font_size();
    m_display = state.display();
    m_overflow = state.overflow();
    m_visibility = state.visibility();
    m_pointer_events = state.pointer_events();
}

void SVGElement::layoutChildren(SVGLayoutState& state)
{
    for(const auto& child : m_children) {
        if(auto element = toSVGElement(child)) {
            element->layout(state);
        }
    }
}

void SVGElement::layout(SVGLayoutState& state)
{
    SVGLayoutState newState(state, this);
    layoutElement(newState);
    layoutChildren(newState);
}

void SVGElement::renderChildren(SVGRenderState& state) const
{
    for(const auto& child : m_children) {
        if(auto element = toSVGElement(child)) {
            element->render(state);
        }
    }
}

void SVGElement::render(SVGRenderState& state) const
{
}

bool SVGElement::isHiddenElement() const
{
    if(isDisplayNone())
        return true;
    switch(m_id) {
    case ElementID::Defs:
    case ElementID::Symbol:
    case ElementID::Marker:
    case ElementID::ClipPath:
    case ElementID::Mask:
    case ElementID::LinearGradient:
    case ElementID::RadialGradient:
    case ElementID::ConicalGradient:
    case ElementID::Pattern:
    case ElementID::Stop:
        return true;
    default:
        return false;
    }
}

bool SVGElement::isPointableElement() const
{
    if(m_pointer_events != PointerEvents::None
        && m_visibility != Visibility::Hidden
        && m_display != Display::None
        && m_opacity != 0.f) {
        switch(m_id) {
        case ElementID::Line:
        case ElementID::Rect:
        case ElementID::Ellipse:
        case ElementID::ForeignObject:
        case ElementID::Circle:
        case ElementID::Polyline:
        case ElementID::Polygon:
        case ElementID::Path:
        case ElementID::Text:
        case ElementID::Image:
            return true;
        default:
            break;
        }
    }

    return false;
}

SVGStyleElement::SVGStyleElement(Document* document)
    : SVGElement(document, ElementID::Style)
{
}

SVGFitToViewBox::SVGFitToViewBox(SVGElement* element)
    : m_viewBox(PropertyID::ViewBox)
    , m_preserveAspectRatio(PropertyID::PreserveAspectRatio)
{
    element->addProperty(m_viewBox);
    element->addProperty(m_preserveAspectRatio);
}

Transform SVGFitToViewBox::viewBoxToViewTransform(const Size& viewportSize) const
{
    const auto& viewBoxRect = m_viewBox.value();
    if(viewBoxRect.isEmpty() || viewportSize.isEmpty())
        return Transform::Identity;
    return m_preserveAspectRatio.getTransform(viewBoxRect, viewportSize);
}

Rect SVGFitToViewBox::getClipRect(const Size& viewportSize) const
{
    const auto& viewBoxRect = m_viewBox.value();
    if(viewBoxRect.isEmpty() || viewportSize.isEmpty())
        return Rect(0, 0, viewportSize.w, viewportSize.h);
    return m_preserveAspectRatio.getClipRect(viewBoxRect, viewportSize);
}

SVGURIReference::SVGURIReference(SVGElement* element)
    : m_href(PropertyID::Href)
{
    element->addProperty(m_href);
}

SVGElement* SVGURIReference::getTargetElement(const Document* document) const
{
    std::string_view value(m_href.value());
    if(value.empty() || value.front() != '#')
        return nullptr;
    return document->rootElement()->getElementById(value.substr(1));
}

bool SVGPaintServer::isRenderable(const SVGElement* element) const
{
    if(element && element->isPointableElement())
        return true;

    return m_opacity > 0.f && (m_element || m_color.alpha() > 0);
}

bool SVGPaintServer::applyPaint(SVGRenderState& state) const
{
    auto canvas = state.canvas();

    if(canvas) {
        const SVGElement* element = state.element();
        if(element == nullptr) element = m_element;

        plutovg_surface_set_context(canvas->surface(), element ? element->identity() : 0 );
    }

    if(!isRenderable(state.element()))
        return false;
    if(m_element) return m_element->applyPaint(state, m_opacity);
    state->setColor(m_color.colorWithAlpha(m_opacity));
    return true;
}

SVGGraphicsElement::SVGGraphicsElement(Document* document, ElementID id)
    : SVGElement(document, id)
    , m_transform(PropertyID::Transform)
{
    addProperty(m_transform);
}

SVGPaintServer SVGGraphicsElement::getPaintServer(const Paint& paint, float opacity) const
{
    if(paint.isNone())
        return SVGPaintServer();
    if(auto element = getPainter(paint.id()))
        return SVGPaintServer(element, paint.color(), opacity);
    return SVGPaintServer(nullptr, paint.color(), opacity);
}

StrokeData SVGGraphicsElement::getStrokeData(const SVGLayoutState& state) const
{
    LengthContext lengthContext(this);
    StrokeData strokeData(lengthContext.valueForLength(state.stroke_width(), LengthDirection::Diagonal));
    strokeData.setMiterLimit(state.stroke_miterlimit());
    strokeData.setLineCap(state.stroke_linecap());
    strokeData.setLineJoin(state.stroke_linejoin());
    strokeData.setDashOffset(lengthContext.valueForLength(state.stroke_dashoffset(), LengthDirection::Diagonal));

    DashArray dashArray;
    for(const auto& dash : state.stroke_dasharray())
        dashArray.push_back(lengthContext.valueForLength(dash, LengthDirection::Diagonal));
    strokeData.setDashArray(std::move(dashArray));
    return strokeData;
}

SVGSVGElement::SVGSVGElement(Document* document)
    : SVGGraphicsElement(document, ElementID::Svg)
    , SVGFitToViewBox(this)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
{
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);
}

Transform SVGSVGElement::localTransform() const
{
    LengthContext lengthContext(this);
    const Rect viewportRect = {
        lengthContext.valueForLength(m_x),
        lengthContext.valueForLength(m_y),
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    if(isRootElement())
        return viewBoxToViewTransform(viewportRect.size());
    return SVGGraphicsElement::localTransform() * Transform::translated(viewportRect.x, viewportRect.y) * viewBoxToViewTransform(viewportRect.size());
}

void SVGSVGElement::render(SVGRenderState& state) const
{
    if(isDisplayNone())
        return;
    LengthContext lengthContext(this);
    const Size viewportSize = {
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    if(viewportSize.isEmpty())
        return;
    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, localTransform());
    newState.beginGroup(blendInfo);
    if(isOverflowHidden())
        newState->clipRect(getClipRect(viewportSize), FillRule::NonZero, newState.currentTransform());
    renderChildren(newState);
    newState.endGroup(blendInfo);
}

SVGRootElement::SVGRootElement(Document* document)
    : SVGSVGElement(document)
{
}

SVGRootElement* SVGRootElement::layoutIfNeeded()
{
    if(needsLayout())
        forceLayout();
    return this;
}

SVGElement* SVGRootElement::getElementById(const Identity& id) const
{
    if(id.document != identity().document)
        return nullptr;

    auto it = m_uniqueIdCache.find(id.hash());
    if(it == m_uniqueIdCache.end())
        return nullptr;
    return toSVGElement(it->second);
}

SVGElement* SVGRootElement::getElementById(const std::string_view& id) const
{
    auto it = m_idCache.find(id);
    if(it == m_idCache.end())
        return nullptr;
    return it->second;
}

void SVGRootElement::addElementById(const Identity& id, SVGNode* element)
{
    m_uniqueIdCache.emplace(id.hash(), element);
}

void SVGRootElement::addElementById(const std::string& id, SVGElement* element)
{
    m_idCache.emplace(id, element);
}

void SVGRootElement::layout(SVGLayoutState& state)
{
    SVGSVGElement::layout(state);

    LengthContext lengthContext(this);
    if(!width().isPercent()) {
        m_intrinsicWidth = lengthContext.valueForLength(width());
    } else {
        m_intrinsicWidth = 0.f;
    }

    if(!height().isPercent()) {
        m_intrinsicHeight = lengthContext.valueForLength(height());
    } else {
        m_intrinsicHeight = 0.f;
    }

    const auto& viewBoxRect = viewBox().value();
    if(!viewBoxRect.isEmpty() && (!m_intrinsicWidth || !m_intrinsicHeight)) {
        auto intrinsicRatio = viewBoxRect.w / viewBoxRect.h;
        if(!m_intrinsicWidth && m_intrinsicHeight)
            m_intrinsicWidth = m_intrinsicHeight * intrinsicRatio;
        else if(m_intrinsicWidth && !m_intrinsicHeight) {
            m_intrinsicHeight = m_intrinsicWidth / intrinsicRatio;
        }
    }

    if(viewBoxRect.isValid() && (!m_intrinsicWidth || !m_intrinsicHeight)) {
        m_intrinsicWidth = viewBoxRect.w;
        m_intrinsicHeight = viewBoxRect.h;
    }

    if(!m_intrinsicWidth || !m_intrinsicHeight) {
        auto boundingBox = paintBoundingBox();
        if(!m_intrinsicWidth)
            m_intrinsicWidth = boundingBox.right();
        if(!m_intrinsicHeight) {
            m_intrinsicHeight = boundingBox.bottom();
        }
    }
}

void SVGRootElement::forceLayout()
{
    SVGLayoutState state;
    layout(state);
}

SVGUseElement::SVGUseElement(Document* document)
    : SVGGraphicsElement(document, ElementID::Use)
    , SVGURIReference(this)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
{
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);
}

Transform SVGUseElement::localTransform() const
{
    LengthContext lengthContext(this);
    const Point translation = {
        lengthContext.valueForLength(m_x),
        lengthContext.valueForLength(m_y)
    };

    return SVGGraphicsElement::localTransform() * Transform::translated(translation.x, translation.y);
}

void SVGUseElement::render(SVGRenderState& state) const
{
    if(isDisplayNone())
        return;
    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, localTransform());
    newState.beginGroup(blendInfo);
    renderChildren(newState);
    newState.endGroup(blendInfo);
}

void SVGUseElement::build()
{
    if(auto targetElement = getTargetElement(document())) {
        if(auto newElement = cloneTargetElement(targetElement)) {
            addChild(std::move(newElement));
        }
    }

    SVGGraphicsElement::build();
}

inline bool isDisallowedElement(const SVGElement* element)
{
    switch(element->id()) {
    case ElementID::Circle:
    case ElementID::Ellipse:
    case ElementID::ForeignObject:
    case ElementID::G:
    case ElementID::Image:
    case ElementID::Line:
    case ElementID::Path:
    case ElementID::Polygon:
    case ElementID::Polyline:
    case ElementID::Rect:
    case ElementID::Svg:
    case ElementID::Symbol:
    case ElementID::Text:
    case ElementID::Tspan:
    case ElementID::Use:
        return false;
    default:
        return true;
    }
}

std::unique_ptr<SVGElement> SVGUseElement::cloneTargetElement(SVGElement* targetElement)
{
    if(targetElement == this || isDisallowedElement(targetElement))
        return nullptr;
    const auto& idAttr = targetElement->getAttribute(PropertyID::Id);
    auto parent = parentElement();
    while(parent) {
        auto attribute = parent->findAttribute(PropertyID::Id);
        if(attribute && idAttr == attribute->value())
            return nullptr;
        parent = parent->parentElement();
    }

    auto tagId = targetElement->id();
    if(tagId == ElementID::Symbol) {
        tagId = ElementID::Svg;
    }

    auto newElement = SVGElement::create(document(), tagId);
    newElement->setAttributes(targetElement->attributes());
    if(newElement->id() == ElementID::Svg) {
        for(const auto& attribute : attributes()) {
            if(attribute.id() == PropertyID::Width || attribute.id() == PropertyID::Height) {
                newElement->setAttribute(attribute);
            }
        }
    }

    if(newElement->id() != ElementID::Use)
        targetElement->cloneChildren(newElement.get());
    return newElement;
}

SVGImageElement::SVGImageElement(Document* document)
    : SVGGraphicsElement(document, ElementID::Image)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
    , m_preserveAspectRatio(PropertyID::PreserveAspectRatio)
{
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);
    addProperty(m_preserveAspectRatio);
}

Rect SVGImageElement::fillBoundingBox() const
{
    LengthContext lengthContext(this);
    const Rect viewportRect = {
        lengthContext.valueForLength(m_x),
        lengthContext.valueForLength(m_y),
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    return viewportRect;
}

Rect SVGImageElement::strokeBoundingBox() const
{
    return fillBoundingBox();
}

void SVGImageElement::render(SVGRenderState& state) const
{
    if(m_image.isNull() || isDisplayNone() || isVisibilityHidden())
        return;
    Rect dstRect(fillBoundingBox());
    Rect srcRect(0, 0, m_image.width(), m_image.height());
    if(dstRect.isEmpty() || srcRect.isEmpty())
        return;
    m_preserveAspectRatio.transformRect(dstRect, srcRect);

    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, localTransform());
    newState.beginGroup(blendInfo);
    newState->drawImage(m_image, dstRect, srcRect, newState.currentTransform());
    newState.endGroup(blendInfo);
}

static Bitmap loadImageResource(const std::string& href)
{
    if(href.compare(0, 5, "data:") == 0) {
        std::string_view input(href);
        auto index = input.find(',', 5);
        if(index == std::string_view::npos)
            return Bitmap();
        input.remove_prefix(index + 1);
        return plutovg_surface_load_from_image_base64(input.data(), input.length());
    }

    return plutovg_surface_load_from_image_file(href.data());
}

void SVGImageElement::parseAttribute(PropertyID id, const std::string& value)
{
    if(id == PropertyID::Href) {
        m_image = loadImageResource(value);
    } else {
        SVGGraphicsElement::parseAttribute(id, value);
    }
}

SVGSymbolElement::SVGSymbolElement(Document* document)
    : SVGGraphicsElement(document, ElementID::Symbol)
    , SVGFitToViewBox(this)
{
}

SVGGElement::SVGGElement(Document* document)
    : SVGGraphicsElement(document, ElementID::G)
{
}

void SVGGElement::render(SVGRenderState& state) const
{
    if(isDisplayNone())
        return;
    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, localTransform());
    newState.beginGroup(blendInfo);
    renderChildren(newState);
    newState.endGroup(blendInfo);
}

SVGDefsElement::SVGDefsElement(Document* document)
    : SVGGraphicsElement(document, ElementID::Defs)
{
}

SVGMarkerElement::SVGMarkerElement(Document* document)
    : SVGElement(document, ElementID::Marker)
    , SVGFitToViewBox(this)
    , m_refX(PropertyID::RefX, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_refY(PropertyID::RefY, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_markerWidth(PropertyID::MarkerWidth, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 3.f, LengthUnits::None)
    , m_markerHeight(PropertyID::MarkerHeight, LengthDirection::Vertical, LengthNegativeMode::Forbid, 3.f, LengthUnits::None)
    , m_markerUnits(PropertyID::MarkerUnits, MarkerUnits::StrokeWidth)
    , m_orient(PropertyID::Orient)
{
    addProperty(m_refX);
    addProperty(m_refY);
    addProperty(m_markerWidth);
    addProperty(m_markerHeight);
    addProperty(m_markerUnits);
    addProperty(m_orient);
}

Point SVGMarkerElement::refPoint() const
{
    LengthContext lengthContext(this);
    const Point refPoint = {
        lengthContext.valueForLength(m_refX),
        lengthContext.valueForLength(m_refY)
    };

    return refPoint;
}

Size SVGMarkerElement::markerSize() const
{
    LengthContext lengthContext(this);
    const Size markerSize = {
        lengthContext.valueForLength(m_markerWidth),
        lengthContext.valueForLength(m_markerHeight)
    };

    return markerSize;
}

Transform SVGMarkerElement::markerTransform(const Point& origin, float angle, float strokeWidth) const
{
    auto transform = Transform::translated(origin.x, origin.y);
    if(m_orient.orientType() == SVGAngle::OrientType::Angle) {
        transform.rotate(m_orient.value());
    } else {
        transform.rotate(angle);
    }

    auto viewTransform = viewBoxToViewTransform(markerSize());
    auto refOrigin = viewTransform.mapPoint(refPoint());
    if(m_markerUnits.value() == MarkerUnits::StrokeWidth)
        transform.scale(strokeWidth, strokeWidth);
    transform.translate(-refOrigin.x, -refOrigin.y);
    return transform * viewTransform;
}

Rect SVGMarkerElement::markerBoundingBox(const Point& origin, float angle, float strokeWidth) const
{
    return markerTransform(origin, angle, strokeWidth).mapRect(paintBoundingBox());
}

void SVGMarkerElement::renderMarker(SVGRenderState& state, const Point& origin, float angle, float strokeWidth) const
{
    if(state.hasCycleReference(this))
        return;
    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, markerTransform(origin, angle, strokeWidth));
    newState.beginGroup(blendInfo);
    if(isOverflowHidden())
        newState->clipRect(getClipRect(markerSize()), FillRule::NonZero, newState.currentTransform());
    renderChildren(newState);
    newState.endGroup(blendInfo);
}

Transform SVGMarkerElement::localTransform() const
{
    return viewBoxToViewTransform(markerSize());
}

SVGClipPathElement::SVGClipPathElement(Document* document)
    : SVGGraphicsElement(document, ElementID::ClipPath)
    , m_clipPathUnits(PropertyID::ClipPathUnits, Units::UserSpaceOnUse)
{
    addProperty(m_clipPathUnits);
}

Rect SVGClipPathElement::clipBoundingBox(const SVGElement* element) const
{
    auto clipBoundingBox = paintBoundingBox();
    if(m_clipPathUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = element->fillBoundingBox();
        clipBoundingBox.x = clipBoundingBox.x * bbox.w + bbox.x;
        clipBoundingBox.y = clipBoundingBox.y * bbox.h + bbox.y;
        clipBoundingBox.w = clipBoundingBox.w * bbox.w;
        clipBoundingBox.h = clipBoundingBox.h * bbox.h;
    }

    return localTransform().mapRect(clipBoundingBox);
}

void SVGClipPathElement::applyClipMask(SVGRenderState& state) const
{
    if(state.hasCycleReference(this))
        return;
    auto maskImage = Canvas::create(state.currentTransform().mapRect(state.paintBoundingBox()));
    auto currentTransform = state.currentTransform() * localTransform();
    if(m_clipPathUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = state.fillBoundingBox();
        currentTransform.translate(bbox.x, bbox.y);
        currentTransform.scale(bbox.w, bbox.h);
    }

    SVGRenderState newState(this, &state, currentTransform, SVGRenderMode::Clipping, maskImage);
    renderChildren(newState);
    if(clipper()) {
        clipper()->applyClipMask(newState);
    }

    state->blendCanvas(*maskImage, BlendMode::Dst_In, 1.f);
}

inline const SVGGeometryElement* toSVGGeometryElement(const SVGNode* node)
{
    if(node && node->isGeometryElement())
        return static_cast<const SVGGeometryElement*>(node);
    return nullptr;
}

void SVGClipPathElement::applyClipPath(SVGRenderState& state) const
{
    auto currentTransform = state.currentTransform() * localTransform();
    if(m_clipPathUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = state.fillBoundingBox();
        currentTransform.translate(bbox.x, bbox.y);
        currentTransform.scale(bbox.w, bbox.h);
    }

    for(const auto& child : children()) {
        auto element = toSVGElement(child);
        if(element == nullptr || element->isDisplayNone())
            continue;
        Transform clipTransform(currentTransform);
        auto shapeElement = toSVGGeometryElement(element);
        if(shapeElement == nullptr) {
            if(element->id() != ElementID::Use)
                continue;
            clipTransform.multiply(element->localTransform());
            shapeElement = toSVGGeometryElement(element->firstChild());
        }

        if(shapeElement == nullptr || !shapeElement->isRenderable())
            continue;
        state->clipPath(shapeElement->path(), shapeElement->clip_rule(), clipTransform * shapeElement->localTransform());
        return;
    }

    state->clipRect(Rect::Empty, FillRule::NonZero, Transform::Identity);
}

bool SVGClipPathElement::requiresMasking() const
{
    if(clipper())
        return true;
    const SVGGeometryElement* prevShapeElement = nullptr;
    for(const auto& child : children()) {
        auto element = toSVGElement(child);
        if(element == nullptr || element->isDisplayNone())
            continue;
        auto shapeElement = toSVGGeometryElement(element);
        if(shapeElement == nullptr) {
            if(element->isTextPositioningElement())
                return true;
            if(element->id() != ElementID::Use)
                continue;
            if(element->clipper())
                return true;
            shapeElement = toSVGGeometryElement(element->firstChild());
        }

        if(shapeElement == nullptr || !shapeElement->isRenderable())
            continue;
        if(prevShapeElement || shapeElement->clipper())
            return true;
        prevShapeElement = shapeElement;
    }

    return false;
}

SVGMaskElement::SVGMaskElement(Document* document)
    : SVGElement(document, ElementID::Mask)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, -10.f, LengthUnits::Percent)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, -10.f, LengthUnits::Percent)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 120.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 120.f, LengthUnits::Percent)
    , m_maskUnits(PropertyID::MaskUnits, Units::ObjectBoundingBox)
    , m_maskContentUnits(PropertyID::MaskContentUnits, Units::UserSpaceOnUse)
{
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);
    addProperty(m_maskUnits);
    addProperty(m_maskContentUnits);
}

Rect SVGMaskElement::maskRect(const SVGElement* element) const
{
    LengthContext lengthContext(this, m_maskUnits.value());
    Rect maskRect = {
        lengthContext.valueForLength(m_x),
        lengthContext.valueForLength(m_y),
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    if(m_maskUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = element->fillBoundingBox();
        maskRect.x = maskRect.x * bbox.w + bbox.x;
        maskRect.y = maskRect.y * bbox.h + bbox.y;
        maskRect.w = maskRect.w * bbox.w;
        maskRect.h = maskRect.h * bbox.h;
    }

    return maskRect;
}

Rect SVGMaskElement::maskBoundingBox(const SVGElement* element) const
{
    auto maskBoundingBox = paintBoundingBox();
    if(m_maskContentUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = element->fillBoundingBox();
        maskBoundingBox.x = maskBoundingBox.x * bbox.w + bbox.x;
        maskBoundingBox.y = maskBoundingBox.y * bbox.h + bbox.y;
        maskBoundingBox.w = maskBoundingBox.w * bbox.w;
        maskBoundingBox.h = maskBoundingBox.h * bbox.h;
    }

    return maskBoundingBox.intersected(maskRect(element));
}

void SVGMaskElement::applyMask(SVGRenderState& state) const
{
    if(state.hasCycleReference(this))
        return;
    auto maskImage = Canvas::create(state.currentTransform().mapRect(state.paintBoundingBox()));
    maskImage->clipRect(maskRect(state.element()), FillRule::NonZero, state.currentTransform());

    auto currentTransform = state.currentTransform();
    if(m_maskContentUnits.value() == Units::ObjectBoundingBox) {
        auto bbox = state.fillBoundingBox();
        currentTransform.translate(bbox.x, bbox.y);
        currentTransform.scale(bbox.w, bbox.h);
    }

    SVGRenderState newState(this, &state, currentTransform, SVGRenderMode::Painting, maskImage);
    renderChildren(newState);
    if(clipper())
        clipper()->applyClipMask(newState);
    if(masker()) {
        masker()->applyMask(newState);
    }

    if(m_mask_type == MaskType::Luminance)
        maskImage->convertToLuminanceMask();
    state->blendCanvas(*maskImage, BlendMode::Dst_In, 1.f);
}

void SVGMaskElement::layoutElement(const SVGLayoutState& state)
{
    m_mask_type = state.mask_type();
    SVGElement::layoutElement(state);
}

SVGForeignObjectElement::SVGForeignObjectElement(Document* document)
    : SVGGraphicsElement(document, ElementID::ForeignObject)
    , SVGFitToViewBox(this)
    , m_x(PropertyID::X, LengthDirection::Horizontal, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_y(PropertyID::Y, LengthDirection::Vertical, LengthNegativeMode::Allow, 0.f, LengthUnits::None)
    , m_width(PropertyID::Width, LengthDirection::Horizontal, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
    , m_height(PropertyID::Height, LengthDirection::Vertical, LengthNegativeMode::Forbid, 100.f, LengthUnits::Percent)
{
    addProperty(m_x);
    addProperty(m_y);
    addProperty(m_width);
    addProperty(m_height);

    manageDocument(true);
}

SVGForeignObjectElement::~SVGForeignObjectElement()
{
    manageDocument(false);
}

void SVGForeignObjectElement::manageDocument(bool advise)
{
    if(advise) {
        document()->m_foreignObjects.addItem(this);
    }
    else {
        document()->m_foreignObjects.removeItem(this);

        // !!! release root instance since already managed by parent document !!!
        if(m_containedDocument != NULL) m_containedDocument->m_rootElement.release();
    }
}

Transform SVGForeignObjectElement::localTransform() const
{
    LengthContext lengthContext(this);
    const Rect viewportRect = {
        lengthContext.valueForLength(m_x),
        lengthContext.valueForLength(m_y),
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    if(isRootElement())
        return viewBoxToViewTransform(viewportRect.size());
    return SVGGraphicsElement::localTransform() * Transform::translated(viewportRect.x, viewportRect.y) * viewBoxToViewTransform(viewportRect.size());
}

void SVGForeignObjectElement::render(SVGRenderState& state) const
{
    if(isDisplayNone())
        return;
    LengthContext lengthContext(this);
    const Size viewportSize = {
        lengthContext.valueForLength(m_width),
        lengthContext.valueForLength(m_height)
    };

    if(viewportSize.isEmpty())
        return;
    SVGBlendInfo blendInfo(this);
    SVGRenderState newState(this, state, localTransform());
    newState.beginGroup(blendInfo);
    if(isOverflowHidden())
        newState->clipRect(getClipRect(viewportSize), FillRule::NonZero, newState.currentTransform());
    renderChildren(newState);
    newState.endGroup(blendInfo);
}

std::unique_ptr<SVGElement> SVGForeignObjectElement::loadContent(Document* document, ElementID id)
{
    for(const auto& namespaceItem : document->m_namespaceList) {
        auto namespaceId = hashelementid(namespaceItem.namespaceId);

        // do something with the namespace, for now this isn't used.
    }

    std::string markup;
    if(createNamespaceElement(id, markup)) {
        auto containedDocument = Document::loadFromData(markup.data(), markup.length(), document);

        m_containedDocument = std::shared_ptr<Document>(containedDocument.release());

        if(m_containedDocument != nullptr) {
            auto rootElement = m_containedDocument->m_rootElement.get();
            rootElement->setParentElement(this);
            return std::unique_ptr<SVGElement>(rootElement);
        }
    }

    return nullptr;
}

bool SVGForeignObjectElement::contains(const Identity& identity) const
{
    if(m_containedDocument == nullptr)
        return false;

    return (identity.document == m_containedDocument->identity().document);
}

SVGElement* SVGForeignObjectElement::getElementById(const Identity& identity)
{
    if(m_containedDocument == nullptr)
        return nullptr;

    Identity lookup = identity; // special handshake
    if (lookup.document == 0) lookup.document = m_containedDocument->identity().document;

    return m_containedDocument->rootElement()->getElementById(lookup);
}

SVGAnimationElement::SVGAnimationElement(Document* document, ElementID id)
    : SVGElement(document, id)
    , m_attributeName(PropertyID::AttributeName)
    , m_startTime(PropertyID::StartTime, 0.f)
    , m_simpleDuration(PropertyID::Dur, 0.f)
    , m_repeatCount(PropertyID::RepeatCount, 0.f)
    , m_repeatDuration(PropertyID::RepeatDuration, 0.f)
    , m_from(PropertyID::From)
    , m_to(PropertyID::To)
    , m_begin(PropertyID::Begin)
    , m_end(PropertyID::End)
    , m_restart(PropertyID::Restart, Restart::Always)
    , m_elapsedTime(0.f)
{
    addProperty(m_attributeName);
    addProperty(m_startTime);
    addProperty(m_simpleDuration);
    addProperty(m_repeatCount);
    addProperty(m_repeatDuration);
    addProperty(m_from);
    addProperty(m_to);
    addProperty(m_begin);
    addProperty(m_end);
    addProperty(m_restart);

    manageDocument(true);
}

SVGAnimationElement::~SVGAnimationElement()
{
    manageDocument(false);
}

float SVGAnimationElement::calculatePosition(float elapsedTime, float distance)
{
    m_elapsedTime += elapsedTime;

    while(m_elapsedTime > m_simpleDuration.value()) {
        m_elapsedTime -= m_simpleDuration.value();
    }

    return (distance * m_elapsedTime / m_simpleDuration.value());
}

bool SVGAnimationElement::isRunning() const
{
    return (m_simpleDuration.value() > 0.f);
}

void SVGAnimationElement::manageDocument(bool advise)
{
    if(advise) {
        document()->m_animations.addItem(this);
    }
    else {
        document()->m_animations.removeItem(this);
    }
}

SVGAnimateElement::SVGAnimateElement(Document* document)
    : SVGAnimationElement(document, ElementID::Animate)
    , m_animationTarget(nullptr)
{
}

void SVGAnimateElement::updateAnimation(float elapsedTime)
{
    if(!isRunning()) return;

    auto target = parentElement();

    if(target == nullptr) return;

    if(m_animationTarget == nullptr && !m_attributeName.value().empty())
        m_animationTarget = target->getAnimationName(m_attributeName.value());

    if(m_animationTarget == nullptr) return;

    float value = m_animationTarget->getAnimationValue();
    m_animationTarget->setAnimationValue(value + 1.f);
}

SVGAnimateMotionElement::SVGAnimateMotionElement(Document* document)
    : SVGAnimationElement(document, ElementID::AnimateMotion)
    , m_path(PropertyID::Path)
{
    addProperty(m_path);
}

void SVGAnimateMotionElement::updateAnimation(float elapsedTime)
{
    if(!isRunning()) return;

    auto target = parentElement();

    if(target == nullptr) return;

    auto transformProperty = target->getProperty(PropertyID::Transform);

    if(transformProperty == nullptr) return;

    auto& transform = static_cast<SVGTransform*>(transformProperty)->m_value;

    auto& path = m_path.value();

    if(path.isNull() || path.isEmpty()) return;

    if(m_points.size() == 0) {

        m_baseTransform = transform;

        const plutovg_outline_t* outline = plutovg_outline_create(path.data(), nullptr);

        if(outline != nullptr) {
            const int count = plutovg_outline_point_count(outline);

            if(count > 1) {
                m_points.reserve(count);

                plutovg_point_t last;
                plutovg_outline_point_at(outline, 0, &last);

                float last_distance = 0.f;
                m_points.push_back( { { last.x, last.y }, last_distance } );

                for(int index = 1; index < count; index++) {
                    plutovg_point_t point;
                    plutovg_outline_point_at(outline, index, &point);

                    float dx = point.x - last.x;
                    float dy = point.y - last.y;

                    float point_distance = sqrt(dx * dx + dy * dy);

                    last_distance += point_distance;

                    last = point;

                    m_points.push_back( { { last.x, last.y }, last_distance } );
                }
            }

            plutovg_outline_destroy(outline);
        }

        // make sure this only runs once
        if(m_points.size() == 0) m_points.push_back( { { 0.f, 0.f }, 0.f} );
    }

    auto points = m_points.size();

    if(points <= 1) return; // nothing to animate

    const auto distance = calculatePosition(elapsedTime, m_points.at(points - 1).distance);

    const auto* previous = &m_points.front();

    for(const auto& current : m_points) {
        if(distance >= previous->distance && distance <= current.distance) {
            // subtract previous distance and scale remaining differential
            auto scale = (distance - previous->distance) / (current.distance - previous->distance);

            float x = previous->location.x + (current.location.x - previous->location.x) * scale;
            float y = previous->location.y + (current.location.y - previous->location.y) * scale;

            transform = m_baseTransform.translated(x, y);
            break;
        }
        previous = &current;
    }
}

SVGAnimateTransformElement::SVGAnimateTransformElement(Document* document)
    : SVGAnimationElement(document, ElementID::AnimateTransform)
    , m_type(PropertyID::Type, TransformType::Translate)
{
    addProperty(m_type);
}

void SVGAnimateTransformElement::updateAnimation(float elapsedTime)
{
    if(!isRunning()) return;

    auto target = parentElement();

    if(target == nullptr) return;

    auto transformProperty = target->getProperty(PropertyID::Transform);

    if(transformProperty == nullptr) return;

    auto& transform = static_cast<SVGTransform*>(transformProperty)->m_value;

    auto animationPosition = calculatePosition(elapsedTime);

    auto calculateNumber = [&](size_t pos)
    {
        auto from = m_from.values().at(pos);
        auto to = m_to.values().at(pos);

        return from + (to - from) * animationPosition;
    };

    switch (m_type.value())
    {
        case TransformType::Translate:
        {
            auto tx = calculateNumber(0);
            auto ty = calculateNumber(1);

            transform.translate(tx, ty);
            break;
        }
        case TransformType::Scale:
        {
            auto sx = calculateNumber(0);
            auto sy = calculateNumber(1);

            transform.scale(sx, sy);
            break;
        }
        case TransformType::Rotate:
        {
            auto angle = calculateNumber(0);
            auto cx = calculateNumber(1);
            auto cy = calculateNumber(2);

            transform.rotate(angle, cx, cy);
            break;
        }
        case TransformType::SkewX:
        {
            auto shx = calculateNumber(0);

            transform.shear(shx, 0.f);
            break;
        }
        case TransformType::SkewY:
        {
            auto shy = calculateNumber(0);

            transform.shear(0.f, shy);
            break;
        }
    }
}

SVGSetElement::SVGSetElement(Document* document)
    : SVGAnimationElement(document, ElementID::Set)
{
}

void SVGSetElement::updateAnimation(float elapsedTime)
{
}

} // namespace lunasvg
