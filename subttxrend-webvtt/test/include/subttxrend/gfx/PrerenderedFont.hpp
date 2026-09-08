/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <cstdint>

namespace subttxrend
{
namespace gfx
{

static constexpr int FONT_WIDTH_PIXELS = 20;
static constexpr int FONT_HEIGHT_PIXELS = 40;

class GlyphData
{
public:
    float xOffset = 0.f;
    float advanceX = 0.f;
    std::uint32_t glyphIndex = 0;
    std::uint32_t codepoint = 0;
};

class TextTokenData
{
public:
    TextTokenData(const std::string &str = {}) : mStr(str), totalAdvanceX(static_cast<float>(str.length()*FONT_WIDTH_PIXELS)), glyphs(), isWhite(false), forceNewline(false)
    {
        glyphs.reserve(str.length());
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            GlyphData glyph;
            glyph.xOffset = static_cast<float>(i * FONT_WIDTH_PIXELS);
            glyph.advanceX = static_cast<float>(FONT_WIDTH_PIXELS);
            glyph.codepoint = static_cast<std::uint32_t>(static_cast<unsigned char>(str[i]));
            glyph.glyphIndex = glyph.codepoint;
            glyphs.push_back(glyph);
        }
    }

    std::string mStr;
    float totalAdvanceX;
    std::vector<GlyphData> glyphs;
    bool isWhite;
    bool forceNewline;
};

class PrerenderedFont
{
public:
    virtual ~PrerenderedFont() = default;

    virtual std::vector<TextTokenData> textToTokens(const std::string& text) = 0;

    virtual std::int32_t getFontHeight() const = 0;

    virtual std::int32_t getFontDescender() const = 0;

    virtual std::int32_t getFontAscender() const = 0;

    virtual std::int32_t getMaxAdvance() const = 0;
};

class MockPrerenderedFont : public PrerenderedFont
{
public:
    std::vector<TextTokenData> textToTokens(const std::string& text) override
    {
        std::istringstream iss(text);
        std::vector<TextTokenData> tokens;
        std::string token;
        TextTokenData ttd;

        while (iss >> token)
        {
            ttd = TextTokenData{token};
            tokens.push_back(ttd);
        }

        return tokens;
    }

    std::int32_t getFontHeight() const override { return FONT_HEIGHT_PIXELS; }

    std::int32_t getFontDescender() const override { return -8; }

    std::int32_t getFontAscender() const override { return FONT_HEIGHT_PIXELS - 8; }

    std::int32_t getMaxAdvance() const override { return FONT_WIDTH_PIXELS; }
};

class PrerenderedFontCache
{
public:
    std::shared_ptr<PrerenderedFont> getFont(const std::string & fontFamily, int height, bool strictHeight = false, bool italics = false)
    {
        (void)fontFamily;
        (void)height;
        (void)strictHeight;
        (void)italics;
        return std::make_shared<MockPrerenderedFont>();
    }

    void clear()
    {
    }
};
}
}
