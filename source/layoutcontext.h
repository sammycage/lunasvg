#ifndef LAYOUTCONTEXT_H
#define LAYOUTCONTEXT_H

#include "property.h"
#include "canvas.h"

#include <list>
#include <map>

namespace lunasvg {

enum class LayoutId
{
    Root,
    Group,
    Shape,
    Mask,
    ClipPath,
    Marker,
    LinearGradient,
    RadialGradient,
    Pattern,
    SolidColor
};

class RenderState;

class LayoutObject
{
public:
    LayoutObject(LayoutId id);
    virtual ~LayoutObject();
    virtual void render(RenderState&) const;
    virtual void apply(RenderState&) const;
    virtual Rect map(const Rect&) const;

    virtual const Rect& fillBoundingBox() const { return Rect::Invalid;}
    virtual const Rect& strokeBoundingBox() const { return Rect::Invalid;}

    bool isPaint() const { return id == LayoutId::LinearGradient || id == LayoutId::RadialGradient || id == LayoutId::Pattern || id == LayoutId::SolidColor; }
    bool isHidden() const { return isPaint() || id == LayoutId::ClipPath || id == LayoutId::Mask || id == LayoutId::Marker; }

public:
    LayoutId id;
};

using LayoutList = std::list<std::unique_ptr<LayoutObject>>;

class LayoutContainer : public LayoutObject
{
public:
    LayoutContainer(LayoutId id);

    const Rect& fillBoundingBox() const;
    const Rect& strokeBoundingBox() const;

    LayoutObject* addChild(std::unique_ptr<LayoutObject> child);
    LayoutObject* addChildIfNotEmpty(std::unique_ptr<LayoutContainer> child);
    void renderChildren(RenderState& state) const;

public:
    LayoutList children;

protected:
    mutable Rect m_fillBoundingBox{Rect::Invalid};
    mutable Rect m_strokeBoundingBox{Rect::Invalid};
};

class LayoutClipPath : public LayoutContainer
{
public:
    LayoutClipPath();

    void apply(RenderState& state) const;

public:
    Units units;
    Transform transform;
    const LayoutClipPath* clipper;
};

class LayoutMask : public LayoutContainer
{
public:
    LayoutMask();

    void apply(RenderState& state) const;

public:
    double x;
    double y;
    double width;
    double height;
    Units units;
    Units contentUnits;
    double opacity;
    const LayoutMask* masker;
    const LayoutClipPath* clipper;
};

class LayoutRoot : public LayoutContainer
{
public:
    LayoutRoot();

    void render(RenderState& state) const;
    Rect map(const Rect& rect) const;

public:
    double width;
    double height;
    Transform transform;
    double opacity;
    const LayoutMask* masker;
    const LayoutClipPath* clipper;
};

class LayoutGroup : public LayoutContainer
{
public:
    LayoutGroup();

    void render(RenderState& state) const;
    Rect map(const Rect& rect) const;

public:
    Transform transform;
    double opacity;
    const LayoutMask* masker;
    const LayoutClipPath* clipper;
};

class LayoutMarker : public LayoutContainer
{
public:
    LayoutMarker();

    Transform markerTransform(const Point& origin, double angle, double strokeWidth) const;
    void renderMarker(RenderState& state, const Point& origin, double angle, double strokeWidth) const;

public:
    double refX;
    double refY;
    Transform transform;
    Angle orient;
    MarkerUnits units;
    double opacity;
    const LayoutMask* masker;
    const LayoutClipPath* clipper;
};

class LayoutPattern : public LayoutContainer
{
public:
    LayoutPattern();

    void apply(RenderState& state) const;

public:
    double x;
    double y;
    double width;
    double height;
    Transform transform;
    Units units;
    Units contentUnits;
    Rect viewBox;
    PreserveAspectRatio preserveAspectRatio;
};

class LayoutGradient : public LayoutObject
{
public:
    LayoutGradient(LayoutId id);

public:
    Transform transform;
    SpreadMethod spreadMethod;
    Units units;
    GradientStops stops;
};

class LayoutLinearGradient : public LayoutGradient
{
public:
    LayoutLinearGradient();

    void apply(RenderState& state) const;

public:
    double x1;
    double y1;
    double x2;
    double y2;
};

class LayoutRadialGradient : public LayoutGradient
{
public:
    LayoutRadialGradient();

    void apply(RenderState& state) const;

public:
    double cx;
    double cy;
    double r;
    double fx;
    double fy;
};

class LayoutSolidColor : public LayoutObject
{
public:
    LayoutSolidColor();

    void apply(RenderState& state) const;

public:
    Color color;
};

class FillData
{
public:
    FillData() = default;

    void fill(RenderState& state, const Path& path) const;

public:
    const LayoutObject* painter{nullptr};
    Color color{Color::Transparent};
    double opacity{0};
    WindRule fillRule{WindRule::NonZero};
};

class StrokeData
{
public:
    StrokeData() = default;

    void stroke(RenderState& state, const Path& path) const;
    void inflate(Rect& box) const;

public:
    const LayoutObject* painter{nullptr};
    Color color{Color::Transparent};
    double opacity{0};
    double width{1};
    double miterlimit{4};
    LineCap cap{LineCap::Butt};
    LineJoin join{LineJoin::Miter};
    DashData dash;
};

class MarkerPosition
{
public:
    MarkerPosition(const LayoutMarker* marker, const Point& origin, double angle);

public:
    const LayoutMarker* marker;
    Point origin;
    double angle;
};

using MarkerPositionList = std::vector<MarkerPosition>;

class MarkerData
{
public:
    MarkerData() = default;

    void add(const LayoutMarker* marker, const Point& origin, double angle);
    void render(RenderState& state) const;
    void inflate(Rect& box) const;

public:
    MarkerPositionList positions;
    double strokeWidth{1};
};

class LayoutShape : public LayoutObject
{
public:
    LayoutShape();

    void render(RenderState& state) const;
    Rect map(const Rect& rect) const;
    const Rect& fillBoundingBox() const;
    const Rect& strokeBoundingBox() const;

public:
    Path path;
    Transform transform;
    FillData fillData;
    StrokeData strokeData;
    MarkerData markerData;
    Visibility visibility;
    WindRule clipRule;
    const LayoutMask* masker;
    const LayoutClipPath* clipper;

protected:
    mutable Rect m_fillBoundingBox{Rect::Invalid};
    mutable Rect m_strokeBoundingBox{Rect::Invalid};
};

enum class RenderMode
{
    Display,
    Clipping
};

class RenderState
{
public:
    RenderState(const LayoutObject* object, RenderMode mode);

    void beginGroup(RenderState& state, const LayoutClipPath* clipper, const LayoutMask* masker, double opacity);
    void endGroup(RenderState& state, const LayoutClipPath* clipper, const LayoutMask* masker, double opacity);

    const LayoutObject* object() const { return m_object;}
    RenderMode mode() const { return m_mode; }
    const Rect& objectBoundingBox() const { return m_object->fillBoundingBox(); }

public:
    std::shared_ptr<Canvas> canvas;
    Transform matrix;

private:
    const LayoutObject* m_object;
    RenderMode m_mode;
};

class ParseDocument;
class StyledElement;
class GeometryElement;

class LayoutContext
{
public:
    LayoutContext(const ParseDocument* document, LayoutRoot* root);

    Element* getElementById(const std::string& id) const;
    LayoutObject* getResourcesById(const std::string& id) const;
    LayoutObject* addToResourcesCache(const std::string& id, std::unique_ptr<LayoutObject> resources);
    LayoutMask* getMasker(const std::string& id);
    LayoutClipPath* getClipper(const std::string& id);
    LayoutMarker* getMarker(const std::string& id);
    LayoutObject* getPainter(const std::string& id);

    FillData fillData(const StyledElement* element);
    DashData dashData(const StyledElement* element);
    StrokeData strokeData(const StyledElement* element);
    MarkerData markerData(const GeometryElement* element, const Path& path);

private:
    const ParseDocument* m_document;
    LayoutRoot* m_root;
    std::map<std::string, LayoutObject*> m_resourcesCache;
};

} // namespace lunasvg

#endif // LAYOUTCONTEXT_H
