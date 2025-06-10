/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#include "FontStripImpl.hpp"

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/StringUtils.hpp>
#include "ftcpp/Freetype.hpp"
#include "ftcpp/MonospaceRenderer.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

common::Logger g_logger("Gfx", "FontStripImpl");

/**
 * Renderer for font glyphs.
 */
class GlyphRenderer : public ftcpp::MonospaceRenderer
{
public:
    /**
     * Constructor.
     *
     * @param library
     *      Freetype library pointer.
     * @param pixmap
     *      Pixmap on which glyph is to be drawn.
     * @param rect
     *      Allowed drawing rectangle.
     */
    GlyphRenderer(ftcpp::Library* library,
                  AlphaPixmap& pixmap,
                  Rectangle rect) :
            ftcpp::MonospaceRenderer(library),
            m_pixmap(pixmap),
            m_rect(rect)
    {
        g_logger.trace("%s - pixmap:%d,%d rect=%d,%d,%d,%d", __func__,
                pixmap.getWidth(), pixmap.getHeight(), rect.m_x, rect.m_y,
                rect.m_w, rect.m_h);
    }

    /**
     * Dumps the character to logger.
     */
    void dump()
    {
        std::unique_ptr<char[]> tmp(new char[m_rect.m_w + 1]);

        for (int y = 0; y < m_rect.m_h; ++y)
        {
            for (int x = 0; x < m_rect.m_w; ++x)
            {
                int dx = x + m_rect.m_x;
                int dy = y + m_rect.m_y;

                auto line = m_pixmap.getLine(dy) + dx;

                if (*line <= 0x3F)
                {
                    tmp[x] = '.';
                }
                else if (*line <= 0x7F)
                {
                    tmp[x] = ';';
                }
                else if (*line <= 0xBF)
                {
                    tmp[x] = 'x';
                }
                else
                {
                    tmp[x] = '@';
                }
            }
            tmp[m_rect.m_w] = 0;
            g_logger.trace("%s - %s", __func__, tmp.get());
        }
    }

protected:
    /** @copydoc ftcpp::MonospaceRenderer::clear */
    virtual void clear(FT_Long width,
                       FT_Long height) override
    {
        g_logger.trace("%s - %d,%d", __func__, static_cast<int>(width),
                static_cast<int>(height));

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                setPixel(x, y, 0);
            }
        }
    }

    /** @copydoc ftcpp::MonospaceRenderer::setPixel */
    virtual void setPixel(FT_Long x,
                          FT_Long y,
                          uint8_t value) override
    {
        if ((x < 0) || (y < 0) || (x >= m_rect.m_w) || (y >= m_rect.m_h))
        {
            g_logger.trace("%s - skipping %d,%d,%02x", __func__,
                    static_cast<int>(x), static_cast<int>(y), value);
            return;
        }

        int dx = x + m_rect.m_x;
        int dy = y + m_rect.m_y;

        auto line = m_pixmap.getLine(dy) + dx;

        *line = value;
    }

private:
    /** Pixmap on which glyph is to be drawn. */
    AlphaPixmap& m_pixmap;

    /** Allowed drawing rectangle. */
    Rectangle m_rect;
};

void copyGlyph(AlphaPixmap& pixmap,
               const Rectangle& dstRect,
               const std::uint8_t* data)
{
    for (std::int32_t y = 0; y < dstRect.m_h; ++y)
    {
        auto line = pixmap.getLine(dstRect.m_y + y) + dstRect.m_x;

        for (std::int32_t x = 0; x < dstRect.m_w; ++x)
        {
            *line = *data;
            ++line;
            ++data;
        }
    }
}

}

FcConfig* FontStripImpl::m_font_config = FcInitLoadConfigAndFonts();

FontStripImpl::FontStripImpl(const Size& glyphSize,
                             const std::size_t glyphCount) :
        m_glyphSize(glyphSize),
        m_glyphCount(glyphCount)
{
    const Size stripSize(glyphSize.m_w * glyphCount, glyphSize.m_h);

    m_surface.resize(stripSize.m_w, stripSize.m_h, 0);
}

FontStripImpl::~FontStripImpl()
{
    // noop
}

bool FontStripImpl::loadGlyph(std::int32_t glyphIndex,
                              const std::uint8_t* data,
                              const std::size_t size)
{
    Rectangle rect = getGlyphRect(glyphIndex);
    if ((rect.m_w == 0) || (rect.m_h == 0))
    {
        g_logger.trace("%s - bad glyph index", __func__);
        return false;
    }

    const std::size_t expectedSize = m_glyphSize.m_w * m_glyphSize.m_h;
    if (expectedSize != size)
    {
        g_logger.trace("%s - size mismatch", __func__);
        return false;
    }

    copyGlyph(m_surface.getPixmap(), rect, data);
    return true;
}

bool FontStripImpl::loadFont(const std::string& fontName,
                             const Size& charSize,
                             const FontStripMap& charMap)
{
    std::string fontFile = findFontFile(fontName);

    g_logger.info("%s - Loading font: %s size=%dx%d", __func__,
            fontFile.c_str(), charSize.m_w, charSize.m_h);

    bool result = false;

    try
    {
        FT_Int32 loadFlags = FT_LOAD_RENDER;

        auto fontFileLowerCase = subttxrend::common::StringUtils::toLower(
                fontFile);
        if (!subttxrend::common::StringUtils::endWith(fontFileLowerCase,
                ".ttf"))
        {
            loadFlags |= FT_LOAD_MONOCHROME;
        }

        auto library = ftcpp::Freetype::open();
        auto face = library->newFace(fontFile.c_str(), 0);

        FT_F26Dot6 fontWidth = charSize.m_w * 64;
        FT_F26Dot6 fontHeight = charSize.m_h * 64;

        face->setCharSize(fontWidth, fontHeight, 0, 0);

        for (std::size_t glyphIndex = 0; glyphIndex < m_glyphCount;
                ++glyphIndex)
        {
            auto character = charMap.toCharacter(glyphIndex);
            if (character < 0)
            {
                continue;
            }

            Rectangle rect = getGlyphRect(glyphIndex);
            if ((rect.m_w == 0) || (rect.m_h == 0))
            {
                continue;
            }

            g_logger.trace("%s - loading glyph %zd (char: %d (%c))", __func__,
                    glyphIndex, character, character);

            GlyphRenderer renderer(library.get(), m_surface.getPixmap(), rect);

            face->loadChar(character, loadFlags);

            renderer.render(*face, m_glyphSize.m_w, m_glyphSize.m_h);

#if VERBOSE_LOGGING
            renderer.dump();
#endif
        }

        result = true;
    }
    catch (ftcpp::Exception& ex)
    {
        g_logger.error("%s - Font '%s' loading failed. Exception: %s", __func__,
                fontFile.c_str(), ex.what());
    }

    return result;
}

const AlphaPixmap& FontStripImpl::getPixmap() const
{
    return m_surface.getPixmap();
}

Rectangle FontStripImpl::getGlyphRect(std::int32_t glyphIndex) const
{
    if ((glyphIndex < 0)
            || (static_cast<std::size_t>(glyphIndex) >= m_glyphCount))
    {
        static const Rectangle EMPTY_RECT(0, 0, 0, 0);
        return EMPTY_RECT;
    }

    Rectangle rect =
    { glyphIndex * m_glyphSize.m_w, 0, m_glyphSize.m_w, m_glyphSize.m_h };

#if VERBOSE_LOGGING
    g_logger.trace("%s - glyph=%d rect=%d,%d,%d,%d", __func__, glyphIndex,
            rect.m_x, rect.m_y, rect.m_w, rect.m_h);
#endif

    return rect;
}

std::string FontStripImpl::findFontFile(const std::string& fontName)
{
    if ((fontName.length() > 0) && (fontName[0] == '/'))
    {
        // absolute path, return immediately
        return fontName;
    }

    std::string fontFile;

    FcPattern* searchPattern = nullptr;
    FcPattern* matchPattern = nullptr;

    do
    {
        if (fontName.length() == 0)
        {
            g_logger.warning("%s - Font name is empty", __func__);
        }

        if (!m_font_config)
        {
            g_logger.error("%s - Cannot load config", __func__);
            break;
        }

        // create pattern
        searchPattern = FcPatternCreate();
        if (!searchPattern)
        {
            g_logger.error("%s - Cannot create pattern", __func__);
            break;
        }

        if (fontName.length() > 0)
        {
            if (FcPatternAddString(searchPattern, FC_FULLNAME,
                    reinterpret_cast<const FcChar8*>(fontName.c_str())) != FcTrue)
            {
                g_logger.error("%s - Cannot add full name", __func__);
                break;
            }
        }

        bool familySetError = false;
        std::string family = fontName;
        while (family.length() > 0)
        {
            g_logger.trace("%s - Add family name: %s", __func__,
                    family.c_str());

            if (FcPatternAddString(searchPattern, FC_FAMILY,
                    reinterpret_cast<const FcChar8*>(family.c_str())) != FcTrue)
            {
                g_logger.error("%s - Cannot add family name", __func__);
                familySetError = true;
                break;
            }

            auto pos = family.find_last_of(' ');
            if (pos != std::string::npos)
            {
                family = family.substr(0, pos);
            }
            else
            {
                break;
            }
        }
        if (familySetError)
        {
            break;
        }

        bool styleSetError = false;
        std::string style = fontName;
        while (style.length() > 0)
        {
            auto pos = style.find_first_of(' ');
            if (pos != std::string::npos)
            {
                style = style.substr(pos + 1);
            }
            else
            {
                break;
            }

            g_logger.trace("%s - Add style: %s", __func__, style.c_str());

            if (FcPatternAddString(searchPattern, FC_STYLE,
                    reinterpret_cast<const FcChar8*>(style.c_str())) != FcTrue)
            {
                g_logger.error("%s - Cannot add style", __func__);
                styleSetError = true;
                break;
            }
        }
        if (styleSetError)
        {
            break;
        }

        if (FcConfigSubstitute(m_font_config, searchPattern, FcMatchPattern) != FcTrue)
        {
            g_logger.error("%s - Cannot do config substitute", __func__);
            break;
        }

        FcDefaultSubstitute(searchPattern);

        // find the font
        FcResult result = FcResultNoMatch;
        matchPattern = FcFontMatch(m_font_config, searchPattern, &result);
        if (matchPattern && (result == FcResultMatch))
        {
            FcChar8* file = NULL;
            if (FcPatternGetString(matchPattern, FC_FILE, 0, &file)
                    == FcResultMatch)
            {
                fontFile = std::string(reinterpret_cast<const char*>(file));

                g_logger.trace("%s - Matching font found: %s", __func__,
                        fontFile.c_str());
            }
            else
            {
                g_logger.error("%s - Cannot get matching font file", __func__);
            }
        }
        else
        {
            g_logger.error("%s - Cannot find match", __func__);
        }
    } while (0);

    if (matchPattern)
    {
        FcPatternDestroy(matchPattern);
    }

    if (searchPattern)
    {
        FcPatternDestroy(searchPattern);
    }

    return fontFile;
}

} // namespace gfx
} // namespace subttxrend
