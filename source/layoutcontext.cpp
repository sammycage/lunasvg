#include "layoutcontext.h"
#include "parser.h"

#include "maskelement.h"
#include "clippathelement.h"
#include "paintelement.h"
#include "markerelement.h"

#include <cmath>

using namespace lunasvg;

LayoutObject::LayoutObject(LayoutId id)
    : id(id)
{
}

LayoutObject::~LayoutObject()
{
}

void LayoutObject::render(RenderState&) const
{
}

void LayoutObject::apply(RenderState&) const
{
}

Rect LayoutObject::map(const Rect&) const
{
    return Rect::Invalid;
}

LayoutContainer::LayoutContainer(LayoutId id)
    : LayoutObject(id)
{
}

const Rect& LayoutContainer::fillBoundingBox() const
{
    if(m_fillBoundingBox.valid())
        return m_fillBoundingBox;

    for(const auto& child : children)
    {
        if(child->isHidden())
            continue;
        m_fillBoundingBox.unite(child->map(child->fillBoundingBox()));
    }

    return m_fillBoundingBox;
}

const Rect& LayoutContainer::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.valid())
        return m_strokeBoundingBox;

    for(const auto& child : children)
    {
        if(child->isHidden())
            continue;
        m_strokeBoundingBox.unite(child->map(child->fillBoundingBox()));
    }

    return m_strokeBoundingBox;
}

LayoutObject* LayoutContainer::addChild(std::unique_ptr<LayoutObject> child)
{
    children.push_back(std::move(child));
    return &*children.back();
}

LayoutObject* LayoutContainer::addChildIfNotEmpty(std::unique_ptr<LayoutContainer> child)
{
    if(child->children.empty())
        return nullptr;

    return addChild(std::move(child));
}

void LayoutContainer::renderChildren(RenderState& state) const
{
    for(const auto& child : children)
        child->render(state);
}

LayoutClipPath::LayoutClipPath()
    : LayoutContainer(LayoutId::ClipPath)
{
}

void LayoutClipPath::apply(RenderState& state) const
{
    RenderState newState(this, RenderMode::Clipping);
    newState.canvas = Canvas::create(state.canvas->width(), state.canvas->height());
    newState.matrix = state.matrix;
    if(units == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        newState.matrix.translate(box.x, box.y);
        newState.matrix.scale(box.w, box.h);
    }

    newState.matrix.premultiply(transform);
    renderChildren(newState);
    if(clipper != nullptr)
        clipper->apply(newState);

    state.canvas->blend(*newState.canvas, BlendMode::Dst_In, 1.0);
}

LayoutMask::LayoutMask()
    : LayoutContainer(LayoutId::Mask)
{
}

void LayoutMask::apply(RenderState& state) const
{
    RenderState newState(this, state.mode());
    newState.canvas = Canvas::create(state.canvas->width(), state.canvas->height());
    newState.matrix = state.matrix;
    if(contentUnits == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        newState.matrix.translate(box.x, box.y);
        newState.matrix.scale(box.w, box.h);
    }

    renderChildren(newState);
    if(clipper != nullptr)
        clipper->apply(newState);

    if(masker != nullptr)
        masker->apply(newState);

    newState.canvas->luminance();
    state.canvas->blend(*newState.canvas, BlendMode::Dst_In, opacity);
}

LayoutGroup::LayoutGroup()
    : LayoutContainer(LayoutId::Group)
{
}

void LayoutGroup::render(RenderState& state) const
{
    RenderState newState(this, state.mode());
    newState.matrix = transform * state.matrix;
    newState.beginGroup(state, clipper, masker, opacity);
    renderChildren(newState);
    newState.endGroup(state, clipper, masker, opacity);
}

Rect LayoutGroup::map(const Rect& rect) const
{
    return transform.map(rect);
}

LayoutMarker::LayoutMarker()
    : LayoutContainer(LayoutId::Marker)
{
}

void LayoutMarker::renderMarker(RenderState& state, const Point& origin, double angle, double strokeWidth) const
{
    RenderState newState(this, state.mode());
    newState.matrix = state.matrix;
    newState.matrix.translate(origin.x, origin.y);
    if(orient.type() == MarkerOrient::Auto)
        newState.matrix.rotate(angle);
    else
        newState.matrix.rotate(orient.value());

    if(units == MarkerUnits::StrokeWidth)
        newState.matrix.scale(strokeWidth, strokeWidth);

    newState.matrix.translate(-refX, -refY);
    newState.matrix.premultiply(transform);
    newState.beginGroup(state, clipper, masker, opacity);
    renderChildren(newState);
    newState.endGroup(state, clipper, masker, opacity);
}

LayoutGradient::LayoutGradient(LayoutId id)
    : LayoutObject(id)
{
}

LayoutLinearGradient::LayoutLinearGradient()
    : LayoutGradient(LayoutId::LinearGradient)
{
}

void LayoutLinearGradient::apply(RenderState& state) const
{
    Transform matrix;
    if(units == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        matrix.translate(box.x, box.y);
        matrix.scale(box.w, box.h);
    }

    LinearGradientValues values{x1, y1, x2, y2};
    state.canvas->setGradient(values, transform * matrix, spreadMethod, stops);
}

LayoutRadialGradient::LayoutRadialGradient()
    : LayoutGradient(LayoutId::RadialGradient)
{
}

void LayoutRadialGradient::apply(RenderState& state) const
{
    Transform matrix;
    if(units == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        matrix.translate(box.x, box.y);
        matrix.scale(box.w, box.h);
    }

    RadialGradientValues values{cx, cy, r, fx, fy};
    state.canvas->setGradient(values, transform * matrix, spreadMethod, stops);
}

LayoutPattern::LayoutPattern()
    : LayoutContainer(LayoutId::Pattern)
{
}

void LayoutPattern::apply(RenderState& state) const
{
    Rect rect{x, y, width, height};
    if(units == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        rect.x = rect.x * box.w + box.x;
        rect.y = rect.y * box.h + box.y;
        rect.w = rect.w * box.w;
        rect.h = rect.h * box.h;
    }

    auto scalex = std::sqrt(state.matrix.m00 * state.matrix.m00 + state.matrix.m01 * state.matrix.m01);
    auto scaley = std::sqrt(state.matrix.m10 * state.matrix.m10 + state.matrix.m11 * state.matrix.m11);

    auto width = static_cast<std::uint32_t>(std::ceil(rect.w * scalex));
    auto height = static_cast<std::uint32_t>(std::ceil(rect.h * scaley));

    RenderState newState(this, RenderMode::Display);
    newState.canvas = Canvas::create(width, height);
    newState.matrix.scale(scalex, scaley);

    if(viewBox.valid())
    {
        auto viewTransform = preserveAspectRatio.getMatrix(rect.w, rect.h, viewBox);
        newState.matrix.premultiply(viewTransform);
    }
    else if(contentUnits == Units::ObjectBoundingBox)
    {
        const auto& box = state.objectBoundingBox();
        newState.matrix.scale(box.w, box.h);
    }

    renderChildren(newState);
    Transform matrix{1.0/scalex, 0, 0, 1.0/scaley, rect.x, rect.y};
    state.canvas->setPattern(*newState.canvas, matrix * transform, TileMode::Tiled);
}

LayoutSolidColor::LayoutSolidColor()
    : LayoutObject(LayoutId::SolidColor)
{
}

void LayoutSolidColor::apply(RenderState& state) const
{
    state.canvas->setColor(color);
}

void FillData::render(RenderState& state, const Path& path) const
{
    if(opacity == 0.0 || (painter == nullptr && color.isNone()))
        return;

    if(painter == nullptr)
        state.canvas->setColor(color);
    else
        painter->apply(state);

    state.canvas->setMatrix(state.matrix);
    state.canvas->setOpacity(opacity);
    state.canvas->setWinding(fillRule);
    state.canvas->fill(path);
}

void StrokeData::render(RenderState& state, const Path& path) const
{
    if(opacity == 0.0 || (painter == nullptr && color.isNone()))
        return;

    if(painter == nullptr)
        state.canvas->setColor(color);
    else
        painter->apply(state);

    state.canvas->setMatrix(state.matrix);
    state.canvas->setOpacity(opacity);
    state.canvas->setLineWidth(width);
    state.canvas->setMiterlimit(miterlimit);
    state.canvas->setLineCap(cap);
    state.canvas->setLineJoin(join);
    state.canvas->setDash(dash);
    state.canvas->stroke(path);
}

MarkerPosition::MarkerPosition(const LayoutMarker* marker, const Point& origin, double angle)
    : marker(marker), origin(origin), angle(angle)
{
}

void MarkerPosition::render(RenderState& state, double strokeWidth) const
{
    marker->renderMarker(state, origin, angle, strokeWidth);
}

LayoutShape::LayoutShape()
    : LayoutObject(LayoutId::Shape)
{
}

void LayoutShape::render(RenderState& state) const
{
    if(visibility == Visibility::Hidden)
        return;

    RenderState newState(this, state.mode());
    newState.matrix = transform * state.matrix;
    newState.beginGroup(state, clipper, masker, 1.0);

    if(newState.mode() == RenderMode::Display)
    {
        fillData.render(newState, path);
        strokeData.render(newState, path);
    }
    else
    {
        newState.canvas->setMatrix(newState.matrix);
        newState.canvas->setColor(Color::Black);
        newState.canvas->setOpacity(1.0);
        newState.canvas->setWinding(clipRule);
        newState.canvas->fill(path);
    }

    for(auto& marker : markers)
        marker.render(newState, strokeData.width);

    newState.endGroup(state, clipper, masker, 1.0);
}

Rect LayoutShape::map(const Rect& rect) const
{
    return transform.map(rect);
}

const Rect& LayoutShape::fillBoundingBox() const
{
    if(m_fillBoundingBox.valid())
        return m_fillBoundingBox;

    m_fillBoundingBox = path.box();
    return m_fillBoundingBox;
}

const Rect& LayoutShape::strokeBoundingBox() const
{
    if(m_strokeBoundingBox.valid())
        return m_strokeBoundingBox;

    m_strokeBoundingBox = fillBoundingBox();
    return m_strokeBoundingBox;
}

LayoutRoot::LayoutRoot()
    : LayoutContainer(LayoutId::Root)
{
}

void LayoutRoot::render(RenderState& state) const
{
    RenderState newState(this, state.mode());
    newState.matrix = transform * state.matrix;
    newState.beginGroup(state, clipper, masker, opacity);
    renderChildren(newState);
    newState.endGroup(state, clipper, masker, opacity);
}

Rect LayoutRoot::map(const Rect& rect) const
{
    return transform.map(rect);
}

RenderState::RenderState(const LayoutObject* object, RenderMode mode)
    : m_object(object), m_mode(mode)
{
}

void RenderState::beginGroup(RenderState& state, const LayoutClipPath* clipper, const LayoutMask* masker, double opacity)
{
    if(clipper || (m_mode == RenderMode::Display && (masker || opacity < 1.0)))
        canvas = Canvas::create(state.canvas->width(), state.canvas->height());
    else
        canvas = state.canvas;
}

void RenderState::endGroup(RenderState& state, const LayoutClipPath* clipper, const LayoutMask* masker, double opacity)
{
    if(state.canvas == canvas)
        return;

    if(clipper && (m_mode == RenderMode::Display || m_mode == RenderMode::Clipping))
        clipper->apply(*this);

    if(masker && m_mode == RenderMode::Display)
        masker->apply(*this);

    state.canvas->blend(*canvas, BlendMode::Src_Over, m_mode == RenderMode::Display ? opacity : 1.0);
}

LayoutContext::LayoutContext(const ParseDocument* document, LayoutRoot* root)
    : m_document(document), m_root(root)
{
}

Element* LayoutContext::getElementById(const std::string& id) const
{
    return m_document->getElementById(id);
}

LayoutObject* LayoutContext::getResourcesById(const std::string& id) const
{
    auto it = m_resourcesCache.find(id);
    if(it == m_resourcesCache.end())
        return nullptr;
    return it->second;
}

LayoutObject* LayoutContext::addToResourcesCache(const std::string& id, std::unique_ptr<LayoutObject> resources)
{
    if(resources == nullptr)
        return nullptr;

    m_resourcesCache.emplace(id, resources.get());
    return m_root->addChild(std::move(resources));
}

LayoutMask* LayoutContext::getMasker(const std::string& id)
{
    if(id.empty())
        return nullptr;

    auto ref = getResourcesById(id);
    if(ref && ref->id == LayoutId::Mask)
        return static_cast<LayoutMask*>(ref);

    auto element = getElementById(id);
    if(element == nullptr || element->id != ElementId::Mask)
        return nullptr;

    auto masker = static_cast<MaskElement*>(element)->getMasker(this);
    return static_cast<LayoutMask*>(addToResourcesCache(id, std::move(masker)));
}

LayoutClipPath* LayoutContext::getClipper(const std::string& id)
{
    if(id.empty())
        return nullptr;

    auto ref = getResourcesById(id);
    if(ref && ref->id == LayoutId::ClipPath)
        return static_cast<LayoutClipPath*>(ref);

    auto element = getElementById(id);
    if(element == nullptr || element->id != ElementId::ClipPath)
        return nullptr;

    auto clipper = static_cast<ClipPathElement*>(element)->getClipper(this);
    return static_cast<LayoutClipPath*>(addToResourcesCache(id, std::move(clipper)));
}

LayoutMarker* LayoutContext::getMarker(const std::string& id)
{
    if(id.empty())
        return nullptr;

    auto ref = getResourcesById(id);
    if(ref && ref->id == LayoutId::Marker)
        return static_cast<LayoutMarker*>(ref);

    auto element = getElementById(id);
    if(element == nullptr || element->id != ElementId::Marker)
        return nullptr;

    auto marker = static_cast<MarkerElement*>(element)->getMarker(this);
    return static_cast<LayoutMarker*>(addToResourcesCache(id, std::move(marker)));
}

LayoutObject* LayoutContext::getPainter(const std::string& id)
{
    if(id.empty())
        return nullptr;

    auto ref = getResourcesById(id);
    if(ref && ref->isPaint())
        return ref;

    auto element = getElementById(id);
    if(element == nullptr || !(element->id == ElementId::LinearGradient || element->id == ElementId::RadialGradient || element->id == ElementId::Pattern || element->id == ElementId::SolidColor))
        return nullptr;

    auto painter = static_cast<PaintElement*>(element)->getPainter(this);
    return addToResourcesCache(id, std::move(painter));
}

FillData LayoutContext::fillData(const StyledElement* element)
{
    auto fill = element->fill();
    if(fill.isNone())
        return FillData{};

    FillData fillData;
    fillData.painter = getPainter(fill.ref());
    fillData.color = fill.color();
    fillData.opacity = element->opacity() * element->fill_opacity();
    fillData.fillRule = element->fill_rule();
    return fillData;
}

DashData LayoutContext::dashData(const StyledElement* element)
{
    auto dasharray = element->stroke_dasharray();
    if(dasharray.empty())
        return DashData{};

    LengthContext lengthContex(element);
    DashArray dashes;
    for(auto& dash : dasharray)
    {
        auto value = lengthContex.valueForLength(dash, LengthMode::Both);
        dashes.push_back(value);
    }

    auto num_dash = dashes.size();
    if(num_dash % 2)
        num_dash *= 2;

    DashData dashData;
    dashData.array.resize(num_dash);
    double sum = 0.0;
    for(std::size_t i = 0;i < num_dash;i++)
    {
        dashData.array[i] = dashes[i % dashes.size()];
        sum += dashData.array[i];
    }

    if(sum == 0.0)
        return DashData{};

    auto offset = lengthContex.valueForLength(element->stroke_dashoffset(), LengthMode::Both);
    dashData.offset = std::fmod(offset, sum);
    if(dashData.offset < 0.0)
        dashData.offset += sum;

    return dashData;
}

StrokeData LayoutContext::strokeData(const StyledElement* element)
{
    auto stroke = element->stroke();
    if(stroke.isNone())
        return StrokeData{};

    LengthContext lengthContex(element);
    StrokeData strokeData;
    strokeData.painter = getPainter(stroke.ref());
    strokeData.color = stroke.color();
    strokeData.opacity = element->opacity() * element->stroke_opacity();
    strokeData.width = lengthContex.valueForLength(element->stroke_width(), LengthMode::Both);
    strokeData.miterlimit = element->stroke_miterlimit();
    strokeData.cap = element->stroke_linecap();
    strokeData.join = element->stroke_linejoin();
    strokeData.dash = dashData(element);
    return strokeData;
}
