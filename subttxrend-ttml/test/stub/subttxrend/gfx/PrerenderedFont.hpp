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

#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace subttxrend
{
namespace gfx
{

struct GlyphData
{
    float xOffset = 0.f;
    float advanceX = 0.f;
    std::uint32_t glyphIndex = 0;
    std::uint32_t codepoint = 0;
};

struct TextTokenData
{
    float totalAdvanceX = 0;
    std::vector<GlyphData> glyphs;
    bool isWhite = false;
    bool forceNewline = false;
};

namespace testsupport
{

struct FontRequest
{
    std::string fontName;
    int faceHeight = 0;
    bool strictHeight = false;
    bool italics = false;
};

struct FontState
{
    std::map<std::string, std::vector<TextTokenData>> tokensByText;
    std::set<std::string> fontsThatThrow;
    std::vector<FontRequest> requests;
    int clearCalls = 0;
};

inline FontState& state()
{
    static FontState currentState;
    return currentState;
}

inline std::vector<GlyphData> makeGlyphs(std::size_t glyphCount)
{
    std::vector<GlyphData> glyphs;
    glyphs.reserve(glyphCount);

    for (std::size_t index = 0; index < glyphCount; ++index)
    {
        GlyphData glyph;
        glyph.glyphIndex = static_cast<std::uint32_t>(index + 1);
        glyph.codepoint = static_cast<std::uint32_t>(index + 1);
        glyphs.push_back(glyph);
    }

    return glyphs;
}

inline TextTokenData makeToken(float totalAdvanceX,
                               bool isWhite = false,
                               bool forceNewline = false,
                               std::size_t glyphCount = 1)
{
    TextTokenData token;
    token.totalAdvanceX = totalAdvanceX;
    token.isWhite = isWhite;
    token.forceNewline = forceNewline;
    token.glyphs = makeGlyphs(isWhite ? 0U : glyphCount);
    return token;
}

inline void reset()
{
    state() = FontState{};
}

inline void setTokens(const std::string& text,
                      std::initializer_list<TextTokenData> tokens)
{
    state().tokensByText[text] = std::vector<TextTokenData>(tokens);
}

inline void failFont(const std::string& fontName)
{
    state().fontsThatThrow.insert(fontName);
}

} // namespace testsupport

class PrerenderedFont
{
public:
    PrerenderedFont() = default;
    virtual ~PrerenderedFont() = default;

    virtual std::vector<TextTokenData> textToTokens(const std::string& text) = 0;
    virtual std::int32_t getFontHeight() const = 0;
    virtual std::int32_t getFontDescender() const = 0;
    virtual std::int32_t getFontAscender() const = 0;
    virtual std::int32_t getMaxAdvance() const = 0;
};

class StubPrerenderedFont : public PrerenderedFont
{
public:
    StubPrerenderedFont(std::string fontName,
                        int fontHeight)
        : m_fontName(std::move(fontName)),
          m_fontHeight(fontHeight)
    {
    }

    const std::string& getName() const
    {
        return m_fontName;
    }

    std::vector<TextTokenData> textToTokens(const std::string& text) override
    {
        const auto configured = testsupport::state().tokensByText.find(text);
        if (configured != testsupport::state().tokensByText.end())
        {
            return configured->second;
        }

        std::vector<TextTokenData> tokens;
        if (text.empty())
        {
            return tokens;
        }

        std::size_t index = 0;
        while (index < text.size())
        {
            const bool isWhite = std::isspace(static_cast<unsigned char>(text[index])) != 0;
            std::size_t next = index;
            while (next < text.size()
                   && ((std::isspace(static_cast<unsigned char>(text[next])) != 0) == isWhite))
            {
                ++next;
            }

            const std::size_t spanLength = next - index;
            tokens.push_back(testsupport::makeToken(
                static_cast<float>((isWhite ? 5 : 10) * spanLength),
                isWhite,
                false,
                spanLength));

            index = next;
        }

        return tokens;
    }

    std::int32_t getFontHeight() const override
    {
        return m_fontHeight;
    }

    std::int32_t getFontDescender() const override
    {
        return 0;
    }

    std::int32_t getFontAscender() const override
    {
        return m_fontHeight;
    }

    std::int32_t getMaxAdvance() const override
    {
        return m_fontHeight;
    }

private:
    std::string m_fontName;
    int m_fontHeight;
};

class PrerenderedFontCache
{
public:
    std::shared_ptr<PrerenderedFont> getFont(const std::string& fontName,
                                             int faceHeight,
                                             bool strictHeight = false,
                                             bool italics = false)
    {
        testsupport::state().requests.push_back({fontName, faceHeight, strictHeight, italics});

        if (testsupport::state().fontsThatThrow.count(fontName) != 0U)
        {
            throw std::runtime_error("Configured font failure");
        }

        const auto key = std::make_tuple(fontName, faceHeight, strictHeight, italics);
        const auto found = m_fontPathAndSizeToFont.find(key);
        if (found != m_fontPathAndSizeToFont.end())
        {
            return found->second;
        }

        auto font = std::make_shared<StubPrerenderedFont>(fontName, faceHeight);
        m_fontPathAndSizeToFont[key] = font;
        return font;
    }

    void clear()
    {
        ++testsupport::state().clearCalls;
        m_fontPathAndSizeToFont.clear();
    }

private:
    std::map<std::tuple<std::string, int, bool, bool>, std::shared_ptr<PrerenderedFont>> m_fontPathAndSizeToFont;
};

} // namespace gfx
} // namespace subttxrend

#endif // _SUBTTXREND_GFX_PRERENDEREDFONT_HPP_