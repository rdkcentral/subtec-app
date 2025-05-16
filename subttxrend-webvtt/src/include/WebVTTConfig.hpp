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
#include <string>
#include <cmath>

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/common/Logger.hpp>

#include <Constants.hpp>

namespace subttxrend {
namespace webvttengine {
extern common::Logger g_logger_config;

inline double getConfigFractional(const common::ConfigProvider* configProvider, const std::string &key, int defaultValue, int fraction) {
    const std::string defString = std::to_string((double)defaultValue / (double)fraction);
    const std::string confString = configProvider->get(key, defString);

    try {
        double confDouble = std::stod(confString);
        int converted = std::round(fraction * confDouble);
        g_logger_config.osdebug(__LOGGER_FUNC__, " ", key, "=", confString, " :", converted);

        return converted;
    } catch (const std::exception &e) {
        g_logger_config.oswarning(__LOGGER_FUNC__, " - Can't convert ", confString, " to double");
    }

    return defaultValue;
}
    
struct WebVTTConfig {
    void init(const common::ConfigProvider* configProvider) {
        fontHeightH =           getConfigFractional(configProvider, "FONT.HEIGHT_VH", constants::kDefaultFontHeight, 100);
        lineHeightH =           getConfigFractional(configProvider, "LINE.HEIGHT_VH", constants::kDefaultLineHeight, 100);
        screenPadding =         getConfigFractional(configProvider, "SCREEN.PADDING", constants::kScreenPaddingVmH, 100);
        horizontalPaddingEm =   getConfigFractional(configProvider, "FONT.HPAD_EM", constants::kDefaultHorizPaddingEmTenths, 10);
        verticalPaddingEm =     getConfigFractional(configProvider, "FONT.VPAD_EM", constants::kDefaultVerticalPaddingEmTenths, 10);
        fontFamily =            configProvider->get("FONT.FAMILY", "Cinecav Sans");
        textColour =            configProvider->get("FONT.COLOUR", "WHITE");
        bgColour =              configProvider->get("BG.COLOUR", "BLACK");
    }

    int fontHeightH {constants::kDefaultFontHeight};
    int lineHeightH {constants::kDefaultLineHeight};
    int screenPadding {constants::kScreenPaddingVmH};
    float horizontalPaddingEm {constants::kDefaultHorizPaddingEmTenths};
    float verticalPaddingEm {constants::kDefaultVerticalPaddingEmTenths};
    std::string fontFamily {"Cinecav Sans"};
    std::string textColour {"WHITE"};
    std::string bgColour {"BLACK"};
};

}
}
