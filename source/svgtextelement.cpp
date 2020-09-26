#include "svgtextelement.h"
#include "svgdocumentimpl.h"
#include "font.h"
#include "paint.h"
#include "strokedata.h"

namespace lunasvg {

SVGTextElement::SVGTextElement(SVGDocument* document)
    : SVGGraphicsElement(DOMElementIdText, document),
      m_x(DOMPropertyIdX),
      m_y(DOMPropertyIdY)
{
    addToPropertyMap(m_x);
    addToPropertyMap(m_y);
}

inline bool parseEntity(const char* ptr, const char* end, std::uint32_t& codepoint)
{
    if(*ptr == '#')
    {
        ++ptr;
        int base = 10;
        if(*ptr == 'x')
        {
            ++ptr;
            base = 16;
        }

        if(!Utils::parseInteger(ptr, codepoint, base))
            return false;
        return ptr == end;
    }

    static const std::map<std::string, std::uint32_t> entitymap =
    {
        {"amp", 38},
        {"lt", 60},
        {"gt", 62},
        {"quot", 34},
        {"apos", 39},
    };

    std::string name(ptr, end);
    std::map<std::string, std::uint32_t>::const_iterator it = entitymap.find(name);
    if(it == entitymap.end())
        return false;
    codepoint = it->second;
    return true;
}

inline const char* decode(const char* begin, const char* end, std::uint32_t& output)
{
    static const int trailing[256] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
    };

    static const std::uint32_t offsets[6] =
    {
        0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080
    };

    int trailingBytes = trailing[static_cast<std::uint8_t>(*begin)];
    if(begin + trailingBytes < end)
    {
        output = 0;
        switch(trailingBytes)
        {
        case 5: output += static_cast<std::uint8_t>(*begin++); output <<= 6;
        case 4: output += static_cast<std::uint8_t>(*begin++); output <<= 6;
        case 3: output += static_cast<std::uint8_t>(*begin++); output <<= 6;
        case 2: output += static_cast<std::uint8_t>(*begin++); output <<= 6;
        case 1: output += static_cast<std::uint8_t>(*begin++); output <<= 6;
        case 0: output += static_cast<std::uint8_t>(*begin++);
        }

        output -= offsets[trailingBytes];
    }
    else
    {
        begin = end;
    }

    return begin;
}

std::vector<std::uint32_t> lunasvg::SVGTextElement::buildTextContent() const
{
    std::string content;
    const SVGElementImpl* e = next;
    while(e != tail)
    {
        if(e->isSVGElementText())
            content += to<SVGElementText>(e)->content();
        e = e->next;
    }

    std::vector<std::uint32_t> out;
    const char* ptr = content.c_str();
    const char* end = ptr + content.size();
    while(ptr < end)
    {
        if(*ptr == '&')
        {
            ++ptr;
            const char* start = ptr;
            while(ptr < end && *ptr!=';')
                ++ptr;
            if(ptr >= end || *ptr!=';')
                break;
            std::uint32_t codepoint = 0;
            if(!parseEntity(start, ptr, codepoint))
                break;
            out.push_back(codepoint);
            ++ptr;
            continue;
        }
        else
        {
            std::uint32_t codepoint = 0;
            ptr = decode(ptr, end, codepoint);
            out.push_back(codepoint);
        }
    }

    return out;
}

void SVGTextElement::render(RenderContext& context) const
{
    const Font* font = document()->impl()->font();
    if(style().isDisplayNone() || font==nullptr)
    {
        context.skipElement();
        return;
    }

    SVGGraphicsElement::render(context);
    RenderState& state = context.state();
    if(state.style.isHidden())
        return;

    double size = state.style.fontSize(state);
    double scale = font->getScale();

    std::vector<double> x;
    std::vector<double> y;

    if(m_x.isSpecified()) x = m_x.property()->values(state);
    if(m_y.isSpecified()) y = m_y.property()->values(state);

    std::vector<std::uint32_t> content = buildTextContent();
    Path path;
    Point position;
    std::uint32_t prevChar = 0;
    for(unsigned int i = 0;i < content.size();i++)
    {
        if(i < x.size()) position.x = x[i];
        if(i < y.size()) position.y = y[i];

        std::uint32_t curChar = content[i];
        position.x += font->getKerning(prevChar, curChar) * scale * size;
        prevChar = curChar;

        AffineTransform matrix;
        matrix.translate(position.x, position.y);
        matrix.scale(scale, -scale);
        matrix.scale(size, size);

        const Glyph& glyph = font->getGlyph(curChar);
        path.addPath(glyph.data, matrix);

        position.x += glyph.advance * scale * size;
    }

    state.bbox = path.boundingBox();
    if(state.style.hasStroke())
    {
        double strokeWidth = state.style.strokeWidth(state);
        state.bbox.x -= strokeWidth * 0.5;
        state.bbox.y -= strokeWidth * 0.5;
        state.bbox.width += strokeWidth;
        state.bbox.height += strokeWidth;
    }

    TextAnchor anchor = state.style.textAnchor();
    if(anchor == TextAnchorMiddle)
        state.matrix.translate(-state.bbox.width * 0.5, 0);
    else if(anchor == TextAnchorEnd)
        state.matrix.translate(-state.bbox.width, 0);

    if(context.mode() == RenderModeBounding)
        return;

    if(context.mode() == RenderModeClipping)
    {
        state.canvas.draw(path, state.matrix, state.style.clipRule(), KRgbBlack, Paint(), StrokeData());
        return;
    }

    StrokeData strokeData = state.style.strokeData(state);
    Paint fillPaint = state.style.fillPaint(state);
    Paint strokePaint = state.style.strokePaint(state);
    WindRule fillRule = state.style.fillRule();

    fillPaint.setOpacity(state.style.fillOpacity() * state.style.opacity());
    strokePaint.setOpacity(state.style.strokeOpacity() * state.style.opacity());

    state.canvas.draw(path, state.matrix, fillRule, fillPaint, strokePaint, strokeData);
}

SVGElementImpl* SVGTextElement::clone(SVGDocument* document) const
{
    SVGTextElement* e = new SVGTextElement(document);
    baseClone(*e);
    return e;
}

} // namespace lunasvg
