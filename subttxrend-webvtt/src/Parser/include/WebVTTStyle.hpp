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

#include <memory>
#include <map>

#include <subttxrend/gfx/ColorArgb.hpp>

namespace subttxrend
{
namespace webvttengine
{

/**
 * @brief Represents the text style for a WebVTTTextChunk.
 * Supports simple i, u, b tags, plus colours
 */
class Style
{
public:
    enum class FontStyleType : int
    {
        kNormal,
        kBold,
        kItalic,
        kUnderline,
        kDefault = kNormal
    };

    // Standard WebVTT colour classes
    static const gfx::ColorArgb kWhite;
    static const gfx::ColorArgb kLime;
    static const gfx::ColorArgb kCyan;
    static const gfx::ColorArgb kRed;
    static const gfx::ColorArgb kYellow;
    static const gfx::ColorArgb kMagenta;
    static const gfx::ColorArgb kBlue;
    static const gfx::ColorArgb kBlack;
    static const gfx::ColorArgb kTransparent;
    static const gfx::ColorArgb kMostlyBlack;

    static const gfx::ColorArgb kDefaultTextColour;
    static const gfx::ColorArgb kDefaultBgColour;

    static const FontStyleType kDefaultFontStyle;

    static const std::string kDefaultFontFamilyString;

    Style() : m_fontFamily(kDefaultFontFamilyString),
                m_fontStyle(kDefaultFontStyle),
                m_textColour(kDefaultTextColour),
                m_backgroundColour(kDefaultBgColour),
                m_edgeColour(kDefaultTextColour){}

    Style(std::string fontFamily, FontStyleType fontStyle, gfx::ColorArgb textColour, gfx::ColorArgb bgColour, gfx::ColorArgb edgeColour = {}) :
                m_fontFamily(fontFamily),
                m_fontStyle(fontStyle),
                m_textColour(textColour),
                m_backgroundColour(bgColour),
                m_edgeColour(edgeColour){}

    friend bool operator==(const Style& lhs, const Style& rhs)
    {
        return (
            lhs.m_fontFamily == rhs.m_fontFamily &&
            lhs.m_fontStyle == rhs.m_fontStyle &&
            lhs.m_textColour == rhs.m_textColour &&
            lhs.m_backgroundColour == rhs.m_backgroundColour &&
            lhs.m_edgeColour == rhs.m_edgeColour

        );
    }

    std::string     fontFamily() const { return m_fontFamily; }
    FontStyleType   fontStyle() const { return m_fontStyle; }
    gfx::ColorArgb  textColour() const { return m_textColour; }
    gfx::ColorArgb  bgColour() const { return m_backgroundColour; }
    gfx::ColorArgb  edgeColour() const { return m_edgeColour; }

    void            fontFamily(std::string family)     { m_fontFamily = family; }
    void            fontStyle(FontStyleType style)        { m_fontStyle = style; }
    void            textColour(gfx::ColorArgb txt_colour) { m_textColour = txt_colour; }
    void            bgColour(gfx::ColorArgb bgColour)    { m_backgroundColour = bgColour; }
    void            edgeColour(gfx::ColorArgb edgeColour)    { m_edgeColour = edgeColour; }

    std::string     getStyleName() const;

    /**
     * @brief Returns the font name for use with FcMatch
     *
     * @return std::string
     */
    std::string getFontStyle() const
    {
        std::string fontFamily {};
        if (m_fontStyle != FontStyleType::kDefault)
            fontFamily = getStyleName();
        
        return fontFamily;
    }

private:
    std::string             m_fontFamily;
    FontStyleType           m_fontStyle;
    gfx::ColorArgb          m_textColour;
    gfx::ColorArgb          m_backgroundColour;
    gfx::ColorArgb          m_edgeColour;
};

using StylePtr = std::shared_ptr<Style>;

}
}
