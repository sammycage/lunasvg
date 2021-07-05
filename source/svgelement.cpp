#include "svgelement.h"
#include "parser.h"
#include "layoutcontext.h"

namespace lunasvg {

SVGElement::SVGElement()
    : GraphicsElement(ElementId::Svg)
{
}

Length SVGElement::x() const
{
    auto& value = get(PropertyId::X);
    return Parser::parseLength(value, AllowNegativeLengths, Length::Zero);
}

Length SVGElement::y() const
{
    auto& value = get(PropertyId::Y);
    return Parser::parseLength(value, AllowNegativeLengths, Length::Zero);
}

Length SVGElement::width() const
{
    auto& value = get(PropertyId::Width);
    return Parser::parseLength(value, ForbidNegativeLengths, Length::HundredPercent);
}

Length SVGElement::height() const
{
    auto& value = get(PropertyId::Height);
    return Parser::parseLength(value, ForbidNegativeLengths, Length::HundredPercent);
}

Rect SVGElement::viewBox() const
{
    auto& value = get(PropertyId::ViewBox);
    return Parser::parseViewBox(value);
}

PreserveAspectRatio SVGElement::preserveAspectRatio() const
{
    auto& value = get(PropertyId::PreserveAspectRatio);
    return Parser::parsePreserveAspectRatio(value);
}

std::unique_ptr<LayoutRoot> SVGElement::layoutDocument(const ParseDocument* document) const
{
    if(isDisplayNone())
        return nullptr;

    auto w = this->width();
    auto h = this->height();
    if(w.isZero() || h.isZero())
        return nullptr;

    LengthContext lengthContext(this);
    auto _x = lengthContext.valueForLength(x(), LengthMode::Width);
    auto _y = lengthContext.valueForLength(y(), LengthMode::Height);
    auto _w = lengthContext.valueForLength(w, LengthMode::Width);
    auto _h = lengthContext.valueForLength(h, LengthMode::Height);

    auto preserveAspectRatio = this->preserveAspectRatio();
    auto viewTranslation = Transform::translated(_x, _y);
    auto viewTransform = preserveAspectRatio.getMatrix(_w, _h, viewBox());

    auto root = std::make_unique<LayoutRoot>();
    root->width = _w;
    root->height = _h;
    root->transform = (viewTranslation * viewTransform) * transform();
    root->opacity = opacity();

    LayoutContext context{document, root.get()};
    root->masker = context.getMasker(mask());
    root->clipper = context.getClipper(clip_path());
    layoutChildren(&context, root.get());
    return root;
}

void SVGElement::layout(LayoutContext* context, LayoutContainer* current) const
{
    if(isDisplayNone())
        return;

    auto w = this->width();
    auto h = this->height();
    if(w.isZero() || h.isZero())
        return;

    LengthContext lengthContext(this);
    auto _x = lengthContext.valueForLength(x(), LengthMode::Width);
    auto _y = lengthContext.valueForLength(y(), LengthMode::Height);
    auto _w = lengthContext.valueForLength(w, LengthMode::Width);
    auto _h = lengthContext.valueForLength(h, LengthMode::Height);

    auto preserveAspectRatio = this->preserveAspectRatio();
    auto viewTranslation = Transform::translated(_x, _y);
    auto viewTransform = preserveAspectRatio.getMatrix(_w, _h, viewBox());

    auto group = std::make_unique<LayoutGroup>();
    group->transform = (viewTranslation * viewTransform) * transform();
    group->opacity = opacity();
    group->masker = context->getMasker(mask());
    group->clipper = context->getClipper(clip_path());
    layoutChildren(context, group.get());
    current->addChildIfNotEmpty(std::move(group));
}

std::unique_ptr<Node> SVGElement::clone() const
{
    return cloneElement<SVGElement>();
}

} // namespace lunasvg
