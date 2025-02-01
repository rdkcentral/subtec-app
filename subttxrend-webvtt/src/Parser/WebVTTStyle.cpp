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

#include <subttxrend/common/Logger.hpp>

#include <WebVTTStyle.hpp>


namespace subttxrend {
namespace webvttengine {
namespace { common::Logger g_logger("WebvttEngine", "WebVTTStyle"); }

const gfx::ColorArgb        Style::kWhite ( 255, 255, 255, 255 );
const gfx::ColorArgb        Style::kLime ( 255, 0, 255, 0 );
const gfx::ColorArgb        Style::kCyan ( 255, 0, 255, 255 );
const gfx::ColorArgb        Style::kRed ( 255, 255, 0, 0 );
const gfx::ColorArgb        Style::kYellow ( 255, 255, 255, 0 );
const gfx::ColorArgb        Style::kMagenta ( 255, 255, 0, 255 );
const gfx::ColorArgb        Style::kBlue ( 255, 0, 0, 255 );
const gfx::ColorArgb        Style::kBlack ( 255, 0, 0, 0 );
const gfx::ColorArgb        Style::kMostlyBlack ( 204, 0, 0, 0 );
const gfx::ColorArgb        Style::kTransparent ( 0, 0, 0, 0 );

const gfx::ColorArgb        Style::kDefaultTextColour(kWhite);
const gfx::ColorArgb        Style::kDefaultBgColour(kBlack);

const std::string           Style::kDefaultFontFamilyString {"cinecavD sans"};

const Style::FontStyleType     Style::kDefaultFontStyle {FontStyleType::kNormal};

/**
 * @brief Get a style name string to pass to FC-Match
 * 
 * @return std::string 
 */
std::string Style::getStyleName() const
{
    static std::map<FontStyleType, std::string> typeMap {
        {FontStyleType::kBold, "bold"},
        {FontStyleType::kUnderline, "underline"},
        {FontStyleType::kItalic, "italic"}
    };
    std::string type_string;

    if (typeMap.count(m_fontStyle)) {
        type_string = typeMap.at(m_fontStyle);
    } else {
        g_logger.info("Style %s not supported - returning empty string", type_string.c_str());
    }

    return type_string;
}


}
}
