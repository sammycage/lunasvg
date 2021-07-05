#include "markerelement.h"
#include "parser.h"
#include "layoutcontext.h"

namespace lunasvg {

MarkerElement::MarkerElement()
    : StyledElement(ElementId::Marker)
{
}

Length MarkerElement::refX() const
{
    auto& value = get(PropertyId::RefX);
    return Parser::parseLength(value, AllowNegativeLengths, Length::Zero);
}

Length MarkerElement::refY() const
{
    auto& value = get(PropertyId::RefY);
    return Parser::parseLength(value, AllowNegativeLengths, Length::Zero);
}

Length MarkerElement::markerWidth() const
{
    auto& value = get(PropertyId::MarkerWidth);
    return Parser::parseLength(value, ForbidNegativeLengths, Length::ThreePercent);
}

Length MarkerElement::markerHeight() const
{
    auto& value = get(PropertyId::MarkerHeight);
    return Parser::parseLength(value, ForbidNegativeLengths, Length::ThreePercent);
}

Angle MarkerElement::orient() const
{
    auto& value = get(PropertyId::Orient);
    return Parser::parseAngle(value);
}

MarkerUnits MarkerElement::markerUnits() const
{
    auto& value = get(PropertyId::MarkerUnits);
    return Parser::parseMarkerUnits(value);
}

Rect MarkerElement::viewBox() const
{
    auto& value = get(PropertyId::ViewBox);
    return Parser::parseViewBox(value);
}

PreserveAspectRatio MarkerElement::preserveAspectRatio() const
{
    auto& value = get(PropertyId::PreserveAspectRatio);
    return Parser::parsePreserveAspectRatio(value);
}

std::unique_ptr<LayoutMarker> MarkerElement::getMarker(LayoutContext* context) const
{
    LengthContext lengthContext(this);
    auto _refX = lengthContext.valueForLength(refX(), LengthMode::Width);
    auto _refY = lengthContext.valueForLength(refY(), LengthMode::Height);
    auto _markerWidth = lengthContext.valueForLength(markerWidth(), LengthMode::Width);
    auto _markerHeight = lengthContext.valueForLength(markerHeight(), LengthMode::Height);

    auto preserveAspectRatio = this->preserveAspectRatio();
    auto viewTransform = preserveAspectRatio.getMatrix(_markerWidth, _markerHeight, viewBox());
    viewTransform.map(_refX, _refY, &_refX, &_refY);

    auto marker = std::make_unique<LayoutMarker>();
    marker->refX = _refX;
    marker->refY = _refY;
    marker->transform = viewTransform;
    marker->orient = orient();
    marker->units = markerUnits();
    marker->opacity = opacity();
    marker->masker = context->getMasker(mask());
    marker->clipper = context->getClipper(clip_path());
    layoutChildren(context, marker.get());
    return marker;
}

std::unique_ptr<Node> MarkerElement::clone() const
{
    return cloneElement<MarkerElement>();
}

} // namespace lunasvg
