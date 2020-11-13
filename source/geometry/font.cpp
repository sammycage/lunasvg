#include "font.h"

#include <fstream>
#include <map>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace lunasvg {

class FontImpl
{
public:
    bool init(std::streambuf* stream);
    bool init(const std::string& filename);

    const Glyph& getGlyph(std::uint32_t codepoint) const;
    double getKerning(std::uint32_t first, std::uint32_t second) const;
    double getScale() const;

private:
    stbtt_fontinfo m_info;
    std::unique_ptr<std::uint8_t[]> m_data;
    mutable std::map<int, Glyph> m_glyphCache;
};

std::unique_ptr<Font> Font::loadFromFile(const std::string& filename)
{
    std::unique_ptr<Font> font(new Font);
    if(font->d->init(filename))
        return font;

    return nullptr;
}

std::unique_ptr<Font> Font::loadFromStream(std::streambuf* stream)
{
    std::unique_ptr<Font> font(new Font);
    if (font->d->init(stream))
        return font;

    return nullptr;
}

const Glyph& Font::getGlyph(std::uint32_t codepoint) const
{
    return d->getGlyph(codepoint);
}

double Font::getKerning(std::uint32_t first, std::uint32_t second) const
{
    return d->getKerning(first, second);
}

double Font::getScale() const
{
    return d->getScale();
}

Font::~Font()
{
}

Font::Font()
    : d(new FontImpl)
{
}

bool FontImpl::init(std::streambuf* stream)
{
    if (stream)
    {
        std::streampos size = stream->pubseekoff(0, std::ios_base::end);
        stream->pubseekoff(0, std::ios_base::beg);
        m_data.reset(new std::uint8_t[size]);
        std::streampos read = stream->sgetn((char*)m_data.get(), size);
        assert(read == size);

        if (!stbtt_InitFont(&m_info, m_data.get(), 0))
            return false;

        return true;
    }

    return false;
}

bool FontImpl::init(const std::string& filename)
{
    std::ifstream fs(filename, std::ios::binary);
    if(!fs.is_open())
        return false;
    
    return init(fs.rdbuf());
}

const Glyph& FontImpl::getGlyph(std::uint32_t codepoint) const
{
    int index = stbtt_FindGlyphIndex(&m_info, static_cast<int>(codepoint));
    std::map<int, Glyph>::const_iterator it = m_glyphCache.find(index);
    if(it != m_glyphCache.end())
        return it->second;

    Glyph& glyph = m_glyphCache[index];
    int advance, bearing;
    stbtt_GetGlyphHMetrics(&m_info, index, &advance, &bearing);
    glyph.advance = advance;
    stbtt_vertex* v;
    int num_vertices = stbtt_GetGlyphShape(&m_info, index, &v);
    for(int i = 0;i < num_vertices;i++)
    {
        switch(v[i].type)
        {
        case STBTT_vmove:
            glyph.data.moveTo(v[i].x, v[i].y);
            break;
        case STBTT_vline:
            glyph.data.lineTo(v[i].x, v[i].y);
            break;
        case STBTT_vcurve:
            glyph.data.quadTo(v[i].cx, v[i].cy, v[i].x, v[i].y);
            break;
        case STBTT_vcubic:
            glyph.data.cubicTo(v[i].cx, v[i].cy, v[i].cx1, v[i].cy1, v[i].x, v[i].y);
            break;
        }
    }

    stbtt_FreeShape(&m_info, v);
    return glyph;
}

double FontImpl::getKerning(std::uint32_t first, std::uint32_t second) const
{
    return stbtt_GetCodepointKernAdvance(&m_info, static_cast<int>(first), static_cast<int>(second));
}

double FontImpl::getScale() const
{
    return static_cast<double>(stbtt_ScaleForMappingEmToPixels(&m_info, 1.f));
}

} // namespace lunasvg
