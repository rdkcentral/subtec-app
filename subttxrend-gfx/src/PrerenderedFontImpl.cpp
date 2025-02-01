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


#include "PrerenderedFontImpl.hpp"
#include "FontStripImpl.hpp"
#include <subttxrend/common/Logger.hpp>
#include <ftcpp/Bitmap.hpp>

#include <cmath>
#include <cwctype>
#include <functional>
#include <map>
#include <memory>

#include <hb.h>
#include <hb-ft.h>

namespace
{
subttxrend::common::Logger g_logger("Gfx", "PrerenderedFontImpl");
}

namespace subttxrend
{
namespace gfx
{

PrerenderedFontImpl::~PrerenderedFontImpl()
{
    FT_Done_Face(face);
    FT_Stroker_Done(stroker);
}

void PrerenderedFontImpl::render(ftcpp::Library &ftcpplib,
                                 FT_Face face,
                                 const Rectangle &rect,
                                 AlphaPixmap &pixmap,
                                 const FT_Bitmap& glyphBitmap)
{
    std::unique_ptr<ftcpp::Bitmap> bitmapptr = ftcpplib.newBitmap();
    ftcpp::Bitmap &bitmap = *(bitmapptr.get());

    bitmap.convert(glyphBitmap);

    std::uint32_t levelsDivider = bitmap.getNativeObject()->num_grays;

    if (levelsDivider <= 1)
    {
        levelsDivider = 1;
    }
    else
    {
        --levelsDivider;
    }

    for (std::size_t bitmapY = 0; bitmapY < bitmap->rows; ++bitmapY)
    {
        auto p = reinterpret_cast<std::uint8_t *>(bitmap->buffer) + (bitmapY * bitmap->pitch);

        int pixelY = rect.m_y + bitmapY;

        for (std::size_t bitmapX = 0; bitmapX < bitmap->width; ++bitmapX)
        {
            int pixelX = rect.m_x + bitmapX;

            std::uint32_t value = p[bitmapX];
            value *= 255;
            value /= levelsDivider;

            auto pix = pixmap.getLine(pixelY) + pixelX;
            *pix = value;
        }
    }
}

void PrerenderedFontImpl::addAtlasPage()
{
    atlasPages.emplace_back();
    atlasPages.back().surface->resize(ATLAS_PAGE_WIDTH, ATLAS_PAGE_HEIGHT, 0);
}

void PrerenderedFontImpl::renderToAtlasPage(CharacterInformation &charInfo, AtlasPage &page,
                                            const FT_Bitmap &glyphBitmap, int bitmapLeft, int bitmapTop)
{
    if (page.currentRowWidth + glyphBitmap.width + 1 >= ATLAS_PAGE_WIDTH)
    {
        page.currentRowY += page.currentRowHeight;
        page.currentRowWidth = 0;
        page.currentRowHeight = 0;
    }

    Rectangle renderRect = {
            page.currentRowWidth,
            page.currentRowY,
            static_cast<std::int32_t>(glyphBitmap.width),
            static_cast<std::int32_t>(glyphBitmap.rows) };

    render(*(freetypeLib.get()), face, renderRect, page.surface->getPixmap(), glyphBitmap);

    charInfo.bitmapWidth = glyphBitmap.width;
    charInfo.bitmapHeight = glyphBitmap.rows;

    charInfo.bitmapLeft = bitmapLeft;
    charInfo.bitmapTop = bitmapTop;

    charInfo.descender = static_cast<std::int32_t>(std::ceil(face->descender / 64.f));

    charInfo.atlasXOffset = renderRect.m_x;
    charInfo.atlasYOffset = renderRect.m_y;

    page.currentRowHeight = std::max(page.currentRowHeight, static_cast<int>(glyphBitmap.rows));
    page.currentRowWidth += glyphBitmap.width + 1;
}

const CharacterInformation *PrerenderedFontImpl::getCharInfo(std::uint32_t glyphIndex, int outlineSize)
{
    if (outlineSize <= 0) {
        auto ci = characterInfosCache.find(glyphIndex);
        if (ci != characterInfosCache.end()) {
            return &ci->second;
        }
    } else {
        auto ci = outlineInfosCache.find(glyphIndex);
        if (ci != outlineInfosCache.end()) {
            return &ci->second;
        }
    }

    std::unique_ptr<ftcpp::Bitmap> bitmapPtr = freetypeLib->newBitmap();
    auto bitmap = bitmapPtr->getNativeObject();
    int bitmapLeft{0};
    int bitmapTop{0};

    if (outlineSize > 0) {
        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT)) {
            g_logger.warning("Loading character at idx %d failed!", glyphIndex);
            return nullptr;
        }

        FT_Stroker_Set(stroker, outlineSize * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

        FT_Glyph glyph;
        if (FT_Get_Glyph(face->glyph, &glyph)) {
            g_logger.warning("Getting glyph at idx %d from slot failed!", glyphIndex);
            FT_Done_Glyph(glyph);
            return nullptr;
        }

        if (FT_Glyph_StrokeBorder(&glyph, stroker, false, true)) {
            g_logger.warning("Stroking outline glyph at idx %d with the stroker failed!", glyphIndex);
            FT_Done_Glyph(glyph);
            return nullptr;
        }

        if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true)) {
            g_logger.warning("Converting glyph at idx %d to BitmapGlyph failed!", glyphIndex);
            FT_Done_Glyph(glyph);
            return nullptr;
        }

        FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
        FT_Bitmap_Copy(freetypeLib->getNativeObject(), &(bitmapGlyph->bitmap), bitmap);
        bitmapLeft = bitmapGlyph->left;
        bitmapTop = bitmapGlyph->top;
        FT_Done_Glyph(glyph);
    } else {
        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER)) {
            g_logger.warning("Loading character at idx %d failed!", glyphIndex);
            return nullptr;
        }

        FT_Bitmap_Copy(freetypeLib->getNativeObject(), &(face->glyph->bitmap), bitmap);
        bitmapLeft = face->glyph->bitmap_left;
        bitmapTop = face->glyph->bitmap_top;
    }

    if (bitmap->width > ATLAS_PAGE_WIDTH) {
        g_logger.warning("Loading character at idx %d : too wide for atlas page!", glyphIndex);
        return nullptr;
    }

    if (std::size_t(bitmap->rows) > ATLAS_PAGE_HEIGHT) {
        g_logger.warning("Loading character at idx %d : too high for atlas page!", glyphIndex);
        return nullptr;
    }

    const AtlasPage &page = atlasPages.back();

    int rowHeight = page.currentRowHeight;
    int rowY = page.currentRowY;
    int rowWidth = page.currentRowWidth;

    if (rowWidth + bitmap->width + 1 >= ATLAS_PAGE_WIDTH) {
        rowY += rowHeight;
    }

    if (rowY + bitmap->rows + 1 >= ATLAS_PAGE_HEIGHT) {
        // need new atlas page, this one will not fit
        addAtlasPage();
    }

    CharacterInformation* charInfo {nullptr};
    if (outlineSize > 0) {
        auto outlineInfoIt = outlineInfosCache.emplace(glyphIndex, CharacterInformation()).first;
        charInfo = &(outlineInfoIt->second);
    } else {
        auto charInfoIt = characterInfosCache.emplace(glyphIndex, CharacterInformation()).first;
        charInfo = &(charInfoIt->second);
    }

    try {
        renderToAtlasPage(*charInfo, atlasPages.back(), *bitmap, bitmapLeft, bitmapTop);
    }  catch (ftcpp::Exception& ex) {
        g_logger.warning("rendering glyph at idx %d: failed. Exception: %s", glyphIndex, ex.what());
        return nullptr;
    }

    charInfo->surface = atlasPages.back().surface.get();

    return charInfo;
}

PrerenderedFontImpl::PrerenderedFontImpl(const char *fontFilePath, int height, bool strict, bool italics)
{
    freetypeLib = ftcpp::Freetype::open();

    // TODO: use the wrappers instead!
    FT_Library library = freetypeLib->getNativeObject();

    FT_Error error = FT_New_Face(library,
                                 fontFilePath,
                                 0,
                                 &face);

    if (error == FT_Err_Unknown_File_Format)
    {
        throw std::runtime_error("font file could be opened - format is unsupported");
    }
    else if (error)
    {
        throw std::runtime_error("font could not be opened");
    }

    FT_Stroker_New(library, &stroker);

    if(italics)
    {
        const double shear_angle_degrees = 10.0;
        const double shear_angle = shear_angle_degrees/180*M_PI;

        g_logger.info("%s using artificial italics. Shear angle = %lf degrees", __LOGGER_FUNC__, shear_angle_degrees);

        FT_Matrix mat;
        mat.xx = 1 * (1<<16);
        mat.xy = 0 * (1<<16);
        mat.yx = -mat.xy;
        mat.yy = mat.xx;

        const FT_Fixed f = (FT_Fixed)(tan(shear_angle) * (1<<16));

        mat.xy += FT_MulFix(f, mat.xx);
        mat.yy += FT_MulFix(f, mat.yx);

        //to make it fit into the same rectangle as character without italics
        mat.xx -= mat.xy;

        FT_Set_Transform( face, &mat, nullptr );
    }

    FT_Select_Charmap(face, FT_Encoding::FT_ENCODING_UNICODE);
    if (strict)
    {
        int check_height = height;
        int32_t returned_size = 0;
        do
        {
           FT_Set_Pixel_Sizes(face, 0, check_height);
           returned_size = static_cast<std::int32_t>(std::ceil(face->size->metrics.height  / 64.f));
           check_height--;
        } while (returned_size > height && check_height > 1);
    }
    else
    {
       FT_Set_Pixel_Sizes(face, 0, height);
    }

    addAtlasPage();

    m_fontHeight = static_cast<std::int32_t>(std::ceil(face->size->metrics.height  / 64.f));
    m_maxAdvance = static_cast<std::int32_t>(std::ceil(face->size->metrics.max_advance / 64.f));

    if (FT_IS_SCALABLE(face))
    {
        m_ascender = static_cast<std::int32_t>(std::floor(FT_MulFix(face->ascender, face->size->metrics.y_scale) / 64.f));
        m_descender = static_cast<std::int32_t>(std::ceil(FT_MulFix(face->descender, face->size->metrics.y_scale) / 64.f));
    }
    else
    {
        // per https://www.freetype.org/freetype2/docs/tutorial/step2.html
        // bbox is only valid for scalable fonts
        m_ascender = 0;
        m_descender = 0;
    }

    g_logger.info("%s path: %s requestHeight: %d(strict=%d) fontHeight: %d ascender: %d descender: %d italics: %s",
        __LOGGER_FUNC__,
        fontFilePath,
        height,
        strict,
        getFontHeight(),
        getFontAscender(), getFontDescender(),
        italics ? "true" : "false");
}

bool PrerenderedFontImpl::isWhite(uint32_t codepoint)
{
    return std::iswspace(codepoint);
}

bool PrerenderedFontImpl::isBreakline(uint32_t codepoint)
{
    // https://en.wikipedia.org/wiki/Newline#Unicode
    return
        codepoint == 0xA ||
        codepoint == 0xB ||
        codepoint == 0xC ||
        codepoint == 0x85 ||
        codepoint == 0x2028 ||
        codepoint == 0x2029;
}

// TODO: not really well tested
std::vector<TextTokenData> PrerenderedFontImpl::textToTokens(const std::string &str)
{
    std::unique_ptr<hb_buffer_t, std::function<void(hb_buffer_t *)>> buf_guard = {
        hb_buffer_create(),
        [](hb_buffer_t *buf) {
            hb_buffer_destroy(buf);
        }};
    hb_buffer_t *buf = buf_guard.get();

    // TODO: keep the font, do not recreate at each call (?)
    std::unique_ptr<hb_font_t, std::function<void(hb_font_t *)>> hb_ft_font_guard = {
        hb_ft_font_create(face, nullptr),
        [](hb_font_t *font) {
            hb_font_destroy(font);
        }};
    hb_font_t *hb_ft_font = hb_ft_font_guard.get();

    auto len = strlen(str.c_str());

    // TODO: UTF-16 handling?
    hb_buffer_add_utf8(buf, str.c_str(), len, 0, len);

    // TODO: language
    // const char* language = "sv";
    // hb_buffer_set_language(buf, hb_language_from_string(language, -1 /*int(strlen(language))*/));

    // Guess the script, language and direction of the buffer
    hb_buffer_guess_segment_properties(buf);

    unsigned int glyphCount = 0;

    hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);

    // before shaping - the buffer contains codepoints; save them for now
    const unsigned preshapeGlyphCount = glyphCount;
    std::map<std::size_t, std::uint32_t> clusterToCodepoint;
    for (unsigned i = 0; i < preshapeGlyphCount; ++i)
    {
        clusterToCodepoint[glyphInfo[i].cluster] = glyphInfo[i].codepoint;
    }

    hb_shape(hb_ft_font, buf, NULL, 0);

    // fetch the buffer again; 'codepoint' should now be 'glyph index' (after shaping)
    glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);

    hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);

    const auto getCodepoint = [&](uint32_t glyphIdx) -> uint32_t {
        auto cpiter = clusterToCodepoint.find(glyphInfo[glyphIdx].cluster);
        if (cpiter == clusterToCodepoint.end())
        {
            // somehow do not have the codepoint for this glyph
            // fall back to original string character ASCII
            return std::uint32_t(str[glyphInfo[glyphIdx].cluster]);
        }
        else
        {
            return cpiter->second;
        }
    };

    std::vector<TextTokenData> tokens;

    unsigned int i = 0;
    while (i < glyphCount)
    {
        tokens.emplace_back();
        TextTokenData &newToken = tokens.back();

        // TODO unicode
        // char character = str[glyph_info[i].cluster];
        // 'codepoint' at this stage is really 'glyph index'
        std::uint32_t unicodeCodepoint = getCodepoint(i);

        newToken.isWhite = isWhite(unicodeCodepoint);
        newToken.forceNewline = isBreakline(unicodeCodepoint);

        unsigned tokenEnd = i + 1;
        while (tokenEnd < glyphCount && isWhite(getCodepoint(tokenEnd)) == newToken.isWhite)
        {
            ++tokenEnd;
        }

        newToken.glyphs.reserve(tokenEnd - i);

        while (i < tokenEnd)
        {
            newToken.totalAdvanceX += glyphPos[i].x_advance;

            newToken.glyphs.emplace_back();
            GlyphData &glyph = newToken.glyphs.back();

            glyph.xOffset = glyphPos[i].x_offset / 64.f;
            glyph.advanceX = glyphPos[i].x_advance / 64.f;

            // hb_glyph_info_t::codepoint is 'either a Unicode code point (before shaping) or a glyph index (after shaping).'
            // at this point: it is glyph idx
            glyph.glyphIndex = glyphInfo[i].codepoint;

            glyph.codepoint = getCodepoint(i);
            if (isBreakline(glyph.codepoint))
            {
                newToken.forceNewline = true;
                i = tokenEnd;
                break;
            }
            ++i;
        }

        newToken.totalAdvanceX /= 64.f;
        // TODO: the width is now too big - we should substract the difference
        // TODO: between last character's advance and it's width
        // TODO: new_token.height
    }

    return tokens;
}

std::int32_t PrerenderedFontImpl::getFontHeight() const
{
    return m_fontHeight;
}

std::int32_t PrerenderedFontImpl::getMaxAdvance() const
{
    return m_maxAdvance;
}

std::int32_t PrerenderedFontImpl::getFontDescender() const
{
    if (!FT_IS_SCALABLE(face))
    {
        g_logger.error("Non scalable font chosen. Descender not valid");
    }
    return m_descender;
}

std::int32_t PrerenderedFontImpl::getFontAscender() const
{
    if (!FT_IS_SCALABLE(face))
    {
        g_logger.error("Non scalable font chosen. Ascender not valid");
    }
    return m_ascender;
}

} // namespace gfx
} // namespace subttxrend

