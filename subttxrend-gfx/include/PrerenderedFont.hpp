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


#ifndef _SUBTTXREND_GFX_PRERENDEREDFONT_HPP_
#define _SUBTTXREND_GFX_PRERENDEREDFONT_HPP_

#include <set>
#include <uchar.h>
#include <memory>
#include <vector>
#include <map>
#include <cstdint>

namespace subttxrend
{
namespace gfx
{

/**
 * Glyph data.
 */
struct GlyphData
{
    /** How much the line advances after drawing this glyph. */
    float xOffset = 0.f;
    /** How much the glyph moves on the X-axis before drawing it. */
    float advanceX = 0.f;
    /** Glyph index in atlas. */
    std::uint32_t glyphIndex = 0;
    /** Character codepoint. */
    std::uint32_t codepoint = 0;
};

/**
 * Text token representation.
 */
struct TextTokenData
{
    /** Total token advence. */
    float totalAdvanceX = 0;
    /** List of glyphs forming the token.  */
    std::vector<GlyphData> glyphs;
    /** Whitespace flag. */
    bool isWhite = false;
    /** New line flag. */
    bool forceNewline = false;
};

/**
 * Represents prerendered font.
 */
class PrerenderedFont
{
public:

    /**
     * Constructor.
     */
    PrerenderedFont() = default;

    /**
     * Destructor
     */
    virtual ~PrerenderedFont() = default;

    /**
     *  Transform text to list of tokens.
     *
     * @param text
     *      Text to transform.
     * @param out
     *      Vector of tokens.
     */
    virtual std::vector<TextTokenData> textToTokens(const std::string& text) = 0;

    /**
     * Return font height.
     *
     * @return
     *      Size of the font in pixels.
     */
    virtual std::int32_t getFontHeight() const = 0;

    /**
     * Return font descender.
     *
     * @return
     *      For scalable fonts returns the negative size of descender in pixels.
     *      For non-scalable returns 0.
     */
    virtual std::int32_t getFontDescender() const = 0;

    /**
     * Return font ascender.
     *
     * @return
     *      For scalable fonts returns ascender in pixels.
     *      For non-scalable returns 0.
     */
    virtual std::int32_t getFontAscender() const = 0;

    /**
     * Return font max advance in pixels.
     *
     * @return
     *      Maximal advance in pixels.
     */
    virtual std::int32_t getMaxAdvance() const = 0;
};

/**
 * Caches for prerendered fonts.
 */
class PrerenderedFontCache
{

public:

    /** 
     * Gets the font corresponding to fontName & face size. Not thread safe!
     *
     * @param fontName
     *         Fontconfig font name, like "Bitstream Vera Sans Mono Bold" or "Liberation Mono Bold".
     * @param faceHeight
     *      Font face Height it will be about the intended height.
     * @param strictHeight
     *      Enforce strict font height. As normally you can receive font bigger than requested.
     *
     * @return
     *      Pointer to prerendered font.
     */
    std::shared_ptr<PrerenderedFont> getFont(const std::string& fontName, int faceHeight, bool strictHeight=false, bool italics=false);

    /** 
     * Clears the cache. Not thread safe!
     */
    void clear();

private:
    struct Height
    {
        int height;
        bool strict;
        bool operator<(const Height& rhs) const
        {
            return height < rhs.height && strict == rhs.strict;
        }
    };
    /** Font name <-> class mapping. */
    std::map<std::tuple<std::string, Height, bool/*italics*/>, std::shared_ptr<PrerenderedFont>> m_fontPathAndSizeToFont;
};

}
}
#endif // _SUBTTXREND_GFX_PRERENDEREDFONT_HPP_
