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


#include "StyleSet.hpp"
#include "Utils.hpp"

#include <cmath>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <sstream>

namespace subttxrend
{
namespace ttmlengine
{

namespace // anonymous
{

using subttxrend::gfx::ColorArgb;
/** Parsing color result and value if successful. */
struct ColorResult
{
    bool result{false};
    gfx::ColorArgb color{};
};
struct SizeResult
{
    bool result{false};
    DomainValue size{};
};

ColorResult parseColor(const std::string& text)
{
    // rgba color pattern - 'rgba(x,x,x,x)'
    static const std::regex rgbaColorPattern("rgba\\( *([0-9]+) *, *([0-9]+) *, *([0-9]+) *, *([0-9]+) *?\\)");
    // '#' color pattern - '#rrggbb[aa]'
    static const std::regex hashColorPattern("#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})?");

    // defaults to white
    std::uint8_t a = 0xFF;
    std::uint8_t r = 0xFF;
    std::uint8_t g = 0xFF;
    std::uint8_t b = 0xFF;

    ColorResult result{false, {}};
    auto trimmedText = trimWhitespace(text);

    std::smatch match;
    if (std::regex_match(trimmedText, match, rgbaColorPattern)) {
        try {
            r = std::stoi(match[1].str());
            g = std::stoi(match[2].str());
            b = std::stoi(match[3].str());
            if (match[4].matched) {
                a = std::stoi(match[4].str());
            }
            result = ColorResult{true, ColorArgb(a, r, g, b)};
        }
        catch (...) {
            // noop
        }
    } else if (std::regex_match(trimmedText, match, hashColorPattern)) {
        try {
            r = std::stoi(match[1].str(), 0, 16);
            g = std::stoi(match[2].str(), 0, 16);
            b = std::stoi(match[3].str(), 0, 16);
            if (match[4].matched) {
                a = std::stoi(match[4].str(), 0, 16);
            }
            result = ColorResult{true, ColorArgb(a, r, g, b)};
        }
        catch (...) {
            // noop
        }
    } else {
        //Get color by name
        gfx::ColorArgb color;
        if (gfx::ColorArgb::getColorByName(trimmedText, color)) {
            result = ColorResult{true, color};
        }
    }

    return result;
}

SizeResult parseSize(const std::string& text)
{
    static const std::regex pixelSizePattern("([0-9]+)px");
    static const std::regex percentageSizePattern("(([0-9]*[.])?[0-9]+)%");
    static const std::regex cellSizePattern("(([0-9]*[.])?[0-9]+)c");

    SizeResult sizeResult;
    auto trimmedText = trimWhitespace(text);

    try {
        std::smatch match;
        if (std::regex_match(trimmedText, match, pixelSizePattern)) {
            // 'tts:fontSize="32px"'
            sizeResult = {true, {DomainValue::Type::PIXEL, std::stoi(match[1].str())}};
        } else if (std::regex_match(trimmedText, match, percentageSizePattern)) {
            // 'tts:fontSize="67,23%"'
            auto sizePercent = std::stof(match[1].str());
            sizeResult = {true, {DomainValue::Type::PERCENTAGE_HUNDREDTHS, static_cast<int>(std::floor(sizePercent * 100))}};
        } else if (std::regex_match(trimmedText, match, cellSizePattern)) {
            // 'tts:fontSize="0.80c"'
            auto sizeInCell = std::stof(match[1].str());
            sizeResult = {true, {DomainValue::Type::CELL_HUNDREDTHS, static_cast<int>(std::floor(sizeInCell * 100))}};
        }
    }
    catch (...) {
        sizeResult = {};
    }

    return sizeResult;
}

Outline parseOutline(const std::string& text)
{
    Outline outline;

    std::istringstream iss(text);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

    try {
        auto parseResult = parseColor(tokens.at(0));
        if (parseResult.result) { // e.g. "(0,0,0,255) 2px"
            outline.setColor(parseResult.color);
            SizeResult sizeResult = parseSize(tokens.at(1));
            if (sizeResult.result) {
                outline.setThickness(sizeResult.size);
            }
        } else { // e.g. "2px" or "2px 0px" - last parameter for blur not used
            outline.setColor(gfx::ColorArgb::BLACK);
            SizeResult sizeResult = parseSize(tokens.at(0));
            if (sizeResult.result) {
                outline.setThickness(sizeResult.size);
            }
        }
    } catch (...) {
        // noop
    }

    return outline;
}

StyleSet::TextAlign parseTextAlign(const std::string& text)
{
    StyleSet::TextAlign textAlign {StyleSet::TextAlign::LEFT};

    if (text == "center") {
        textAlign = StyleSet::TextAlign::CENTER;
    } else if (text == "right" || text == "end") {
        textAlign = StyleSet::TextAlign::RIGHT;
    }

    return textAlign;
}

StyleSet::DisplayAlign parseDisplayAlign(const std::string& text)
{
    StyleSet::DisplayAlign displayAlign {StyleSet::DisplayAlign::BEFORE};

    if (text == "center") {
        displayAlign = StyleSet::DisplayAlign::CENTER;
    } else if (text == "after") {
        displayAlign = StyleSet::DisplayAlign::AFTER;
    }

    return displayAlign;
}

} // namespace anonymous

const ColorArgb& StyleSet::getColor() const
{
    return m_color;
}

const ColorArgb& StyleSet::getBackgroundColor() const
{
    return m_backgroundColor;
}

const std::string& StyleSet::getFontFamily() const
{
    return m_fontFamily;
}

StyleSet::TextAlign StyleSet::getTextAlign() const
{
    return m_textAlign;
}

StyleSet::DisplayAlign StyleSet::getDisplayAlign() const
{
    return m_displayAlign;
}

DomainValue StyleSet::getFontSize() const
{
    return m_fontSize;
}

DomainValue StyleSet::getLineHeight() const
{
    return m_lineHeight;
}

const Outline& StyleSet::getOutline() const
{
    return m_textOutline;
}

const std::string& StyleSet::getStyleId() const
{
    return m_styleId;
}

void StyleSet::setStyleId(const std::string& styleId)
{
    m_styleId = styleId;
}
void StyleSet::merge(const Attributes& attributes)
{
    for (const auto& attr : attributes) {
        parseAttribute(attr.first, attr.second);
    }
}

bool operator==(const StyleSet& lhs,
                const StyleSet& rhs)
{
    return ((lhs.m_color == rhs.m_color) && (lhs.m_backgroundColor == rhs.m_backgroundColor)
            && (lhs.m_fontFamily == rhs.m_fontFamily) && (lhs.m_fontSize == rhs.m_fontSize)
            && (lhs.m_textAlign == rhs.m_textAlign) && (lhs.m_displayAlign == rhs.m_displayAlign))
            && (lhs.m_lineHeight == rhs.m_lineHeight) && (lhs.m_textOutline == rhs.m_textOutline);
}

void StyleSet::parseAttribute(const std::string& name,
                              const std::string& value)
{
    if (name == "backgroundColor") {
        auto parseResult = parseColor(value);
        if (parseResult.result) {
            if (gfx::ColorArgb::TRANSPARENT == parseResult.color)
            {
                parseResult.color = gfx::ColorArgb::BLACK;
            }
            m_backgroundColor = parseResult.color;
        }
    } else if (name == "color") {
        auto parseResult = parseColor(value);
        if (parseResult.result) {
            m_color = parseResult.color;
        }
    } else if (name == "fontSize") {
        auto sizeResult = parseSize(value);
        if (sizeResult.result) {
            m_fontSize = sizeResult.size;
        }
    } else if (name == "textAlign") {
        m_textAlign = parseTextAlign(value);
    } else if (name == "displayAlign") {
        m_displayAlign = parseDisplayAlign(value);
    } else if (name == "fontFamily") {
        m_fontFamily = value;
    } else if (name == "lineHeight") {
        auto sizeResult = parseSize(value);
        if (sizeResult.result) {
            m_lineHeight = sizeResult.size;
        }
    } else if (name == "textOutline") {
        m_textOutline = parseOutline(value);
    }
}

std::ostream& operator<<(std::ostream& out, const StyleSet::TextAlign textAlign)
{
    switch (textAlign) {
        case StyleSet::TextAlign::LEFT:
            out << "left";
            break;
        case StyleSet::TextAlign::RIGHT:
            out << "right";
            break;
        default:
            out << "center";
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const StyleSet::DisplayAlign displayAlign)
{
    switch (displayAlign) {
        case StyleSet::DisplayAlign::AFTER:
            out << "after";
            break;
        case StyleSet::DisplayAlign::BEFORE:
            out << "before";
            break;
        default:
            out << "center";
            break;
    }

    return out;
}

std::string StyleSet::toStr()
{
    std::ostringstream str;

    str << "[" << m_styleId << "]: " << "font: " << m_color << ", " << m_fontFamily << ", " << m_fontSize
        << ", bg: " << m_backgroundColor  << ", text: " << m_textAlign << ", display: " << m_displayAlign
        << ", outline: " << m_textOutline.getColor() << ", " << m_textOutline.getThickness();
    return str.str();
}

} // namespace subttxrend
} // namespace ttmlengine

