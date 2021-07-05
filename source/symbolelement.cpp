#include "symbolelement.h"
#include "parser.h"

using namespace lunasvg;

SymbolElement::SymbolElement()
    : StyledElement(ElementId::Symbol)
{
}

Length SymbolElement::x() const
{
    auto& value = get(PropertyId::X);
    if(value.empty())
        return Length{};

    return Parser::parseLength(value, AllowNegativeLengths);
}

Length SymbolElement::y() const
{
    auto& value = get(PropertyId::Y);
    if(value.empty())
        return Length{};

    return Parser::parseLength(value, AllowNegativeLengths);
}

Length SymbolElement::width() const
{
    auto& value = get(PropertyId::Width);
    if(value.empty())
        return Length{100, LengthUnits::Percent};

    return Parser::parseLength(value, ForbidNegativeLengths);
}

Length SymbolElement::height() const
{
    auto& value = get(PropertyId::Height);
    if(value.empty())
        return Length{100, LengthUnits::Percent};

    return Parser::parseLength(value, ForbidNegativeLengths);
}

Rect SymbolElement::viewBox() const
{
    auto& value = get(PropertyId::ViewBox);
    if(value.empty())
        return Rect{};

    return Parser::parseViewBox(value);
}

PreserveAspectRatio SymbolElement::preserveAspectRatio() const
{
    auto& value = get(PropertyId::PreserveAspectRatio);
    if(value.empty())
        return PreserveAspectRatio{};

    return Parser::parsePreserveAspectRatio(value);
}

std::unique_ptr<Node> SymbolElement::clone() const
{
    return cloneElement<SymbolElement>();
}
