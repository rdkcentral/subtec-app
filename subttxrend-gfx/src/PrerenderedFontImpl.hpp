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


#ifndef ATLAS_FONT_IMPL_H__
#define ATLAS_FONT_IMPL_H__

#include <map>
#include <memory>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "PrerenderedFont.hpp"

#include "Surface.hpp"
#include <Types.hpp>

#include <ftcpp/Library.hpp>
#include <ftcpp/Freetype.hpp>

namespace subttxrend
{
namespace gfx
{

/**
 * Prerendered character information.
 */
struct CharacterInformation
{
    /** Bitmap width. */
    std::int32_t bitmapWidth;
    /** Bitmap height. */
    std::int32_t bitmapHeight;
    /** Bitmap left. */
    std::int32_t bitmapLeft;
    /** Bitmap top. */
    std::int32_t bitmapTop;

    /** Glyph descent. */
    int32_t descender;

    /** Glyph x offset in surface. */
    int atlasXOffset;
    /** Glyph y offset in surface. */
    int atlasYOffset;

    /** Glyphs surface. */
    const AlphaSurface *surface = nullptr;
};

/**
 * Prerendered font class implementation.
 */
class PrerenderedFontImpl : public PrerenderedFont
{

public:

    /**
     * Constructor.
     *
     * @param fontFilePath
     *      Path to *.ttf file.
     * @param fontFaceSize
     *      Font face size.
     */
    PrerenderedFontImpl(const char *fontFilePath,
                        int fontFaceSize, bool strictHeight = false, bool italics=false);

    /**
     * Destructor
     */
    virtual ~PrerenderedFontImpl();

    /** @copydoc PrerenderedFont::textToTokens */
    std::vector<TextTokenData> textToTokens(const std::string &str) override;

    /** @copydoc PrerenderedFont::getFontHeight */
    std::int32_t getFontHeight() const override;

    /** @copydoc PrerenderedFont::getMaxAdvance */
    std::int32_t getMaxAdvance() const override;

    /** @copydoc PrerenderedFont::getFontDescender */
    std::int32_t getFontDescender() const override;

    /** @copydoc PrerenderedFont::getFontDescender */
    std::int32_t getFontAscender() const override;

    /**
     * Character info getter.
     *
     * @param codepoint
     *      Info to return data for.
     *
     * @param outlineSize
     *      Outline size in pixels.
     *
     * @return
     *      Character info for given char.
     */
    const CharacterInformation* getCharInfo(std::uint32_t codepoint, int outlineSize);

private:

    /**
     * Font atlas page.
     */
    struct AtlasPage
    {
        AtlasPage() :
                surface(new AlphaSurface())
        {
            // noop
        }

        /** Surface where font is rendered. */
        std::unique_ptr<AlphaSurface> surface;

        /** Current row height. */
        int currentRowHeight = 0;

        /** Current row. */
        int currentRowY = 0;

        /** Current row width. */
        int currentRowWidth = 0;
    };

    /**
     * Prerenders font in given rectangle on pixmap.
     *
     * @param ftcpplib
     *      Freetype library.
     * @param face
     *      Font face.
     * @param rect
     *      Where to draw.
     * @param pixmap
     *      Destination pixmap.
     */
    void render(ftcpp::Library &ftcpplib,
                FT_Face face,
                const Rectangle &rect,
                AlphaPixmap &pixmap,
                const FT_Bitmap &glyphBitmap);

    /**
     * Adds atlas page.
     */
    void addAtlasPage();

    /**
     * Renders the bitmap currently loaded via FT_Load_Char.
     *
     * @param charInfo
     *      Character information.
     * @param page
     *      Atlas page to add to.
     */
    void renderToAtlasPage(CharacterInformation &charInfo,
                           AtlasPage &page,
                           const FT_Bitmap &glyphBitmap,
                           int bitmapLeft,
                           int bitmapTop);

    /**
     * Is given codepoint whitespace.
     *
     * @param codepoint
     * @return
     *      True if given codepoint is whitespace, false otherwise.
     */
    bool isWhite(uint32_t codepoint);

    /**
     * Is given codepoint breakline.
     *
     * @param codepoint
     * @return
     *      True if given codepoint is breakline, false otherwise.
     */
    bool isBreakline(uint32_t codepoint);

    // TODO: make that dependant of font size
    enum
    {
        ATLAS_PAGE_WIDTH = 128,
        ATLAS_PAGE_HEIGHT = 128
    };

    // TODO: maybe unordered/hashset?
    std::map<uint32_t, CharacterInformation> characterInfosCache;
    std::map<uint32_t, CharacterInformation> outlineInfosCache;


    /** Font face. */
    FT_Face face;

    /** Font stroker. */
    FT_Stroker stroker;

    /** Font height. */
    std::int32_t m_fontHeight;

    /** Font max width. */
    std::int32_t m_maxAdvance;

    /** Font descender. */
    std::int32_t m_descender;

    /** Font ascender. */
    std::int32_t m_ascender;

    /** Collection of atlas pages. */
    std::vector<AtlasPage> atlasPages;

    /** Freetype library. */
    std::unique_ptr<ftcpp::Library> freetypeLib;

};

} // namespace gfx
} // namespace subttxrend

#endif
