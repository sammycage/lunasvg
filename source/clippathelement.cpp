#include "clippathelement.h"
#include "parser.h"
#include "layoutcontext.h"

namespace lunasvg {

ClipPathElement::ClipPathElement()
    : GraphicsElement(ElementId::ClipPath)
{
}

Units ClipPathElement::clipPathUnits() const
{
    auto& value = get(PropertyId::ClipPathUnits);
    return Parser::parseUnits(value, Units::UserSpaceOnUse);
}

std::unique_ptr<LayoutClipPath> ClipPathElement::getClipper(LayoutContext* context) const
{
    if(context->hasReference(this))
        return nullptr;

    auto clipper = std::make_unique<LayoutClipPath>();
    clipper->units = clipPathUnits();
    clipper->transform = transform();
    clipper->clipper = context->getClipper(clip_path());

    context->addReference(this);
    layoutChildren(context, clipper.get());
    context->removeReference(this);

    return clipper;
}

std::unique_ptr<Node> ClipPathElement::clone() const
{
    return cloneElement<ClipPathElement>();
}

} // namespace lunasvg
