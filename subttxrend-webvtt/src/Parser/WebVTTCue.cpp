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
#include <string>
#include <map>
#include <cmath>

#include <WebVTTCue.hpp>
#include <WebVTTExceptions.hpp>
#include <Constants.hpp>

namespace subttxrend {
namespace webvttengine {

namespace {
common::Logger g_logger("WebvttEngine", "WebVTTCue");

template<typename T>
T convertStringFromMap(const std::map<std::string, T>& map, const std::string& str) {
    try {
        return map.at(str);
    }
    catch (const std::out_of_range& e) {
        throw ParserException("String not in map: " + str);
    }
}

WebVTTCue::AlignType convertStringToAlign(const std::string& align) {
    const std::map<std::string, WebVTTCue::AlignType> alignMap = {
        { "start", WebVTTCue::AlignType::kStart },
        { "center", WebVTTCue::AlignType::kCenter },
        { "end", WebVTTCue::AlignType::kEnd },
        { "left", WebVTTCue::AlignType::kLeft },
        { "right", WebVTTCue::AlignType::kRight }
    };
    return convertStringFromMap<WebVTTCue::AlignType>(alignMap, align);
}

WebVTTCue::LineAlignType convertStringToLineAlign(const std::string& strAlign) {
    const std::map<std::string, WebVTTCue::LineAlignType> alignMap = {
        { "start", WebVTTCue::LineAlignType::kStart },
        { "center", WebVTTCue::LineAlignType::kCenter },
        { "end", WebVTTCue::LineAlignType::kEnd }
    };

    return convertStringFromMap<WebVTTCue::LineAlignType>(alignMap, strAlign);
}

WebVTTCue::PositionAlignType convertStringToPositionAlign(const std::string& strAlign) {
    const std::map<std::string, WebVTTCue::PositionAlignType> alignMap = {
        { "line-left", WebVTTCue::PositionAlignType::kLineLeft },
        { "center", WebVTTCue::PositionAlignType::kCenter },
        { "line-right", WebVTTCue::PositionAlignType::kLineRight },
        { "auto", WebVTTCue::PositionAlignType::kAutoAlign }
    };

    return convertStringFromMap<WebVTTCue::PositionAlignType>(alignMap, strAlign);
}

/**
 * @brief Parse percentage data from text and multiply by 100
 *
 * @param value
 * @throws ParserException - values must be valid %age 0 <= n <= 100
 * @return int
 */
int parsePercentageHundredths(std::string value) {
    int percent;

    try {
        if (value.back() == '%') {
            float fpercent = std::stof(value);
            fpercent *= 100.0;
            percent = std::floor(fpercent + 0.5);
            if (percent < 0 || percent > 10000) {
                throw ParserException("Percentage out of range :" + value);
            }
        }
        else {
            throw ParserException("Not a percentage string");
        }
    }
    catch (const std::invalid_argument& e) {
        throw ParserException("Invalid percentage conversion of :" + value);
    }

    return percent;
}

} // namespace

/**
 * @brief Adds a cue text line string to the Cue's vector
 *
 * @param textLine
 */
void WebVTTCue::addTextLine(const std::string& textLine) {
    m_lines.push_back(textLine);
}

/**
 * @brief Take any settings parsed from the file and add to the cue
 *        This also includes the necessary calculations if values are not default
 *        If no settings it will use the hardcoded defaults
 *
 * @param settings
 */
void WebVTTCue::addCueSettings(const SettingsMap& settings) {
    if (!settings.empty()) {
        for (const auto& setting : settings) {
            auto& property = setting.first;
            auto& value = setting.second;
            try {
                if (!property.compare("line")) {
                    line(value);
                }
                else if (!property.compare("size")) {
                    size(value);
                }
                else if (!property.compare("position")) {
                    position(value);
                }
                else if (!property.compare("align")) {
                    align(value);
                }
                else if (!property.compare("region")) {
                    m_regionId = value;
                }
            } catch (const ParserException &e) {
                g_logger.osinfo(__LOGGER_FUNC__, e.what());
                g_logger.osinfo(__LOGGER_FUNC__, " - failed to parse p:", property, " v:", value,
                                    " - continuing...");
            }
        }
        
        m_cuePosition.computedSizeVwH = computedSize();
        m_cuePosition.computedPositionVwH = computedPosition();
        m_cuePosition.lineVhH = computedLine();
        m_cuePosition.cueTextAlign = m_align;
        m_cuePosition.cueLineAlign = m_lineAlign;
        m_cuePosition.snapToLines = m_snapToLines;
    }
}

/**
 * @brief Used by CueDrawer to get dimensions of the cue's drawing area
 *
 * @return const WebVTTCue::CueBox&
 */
const WebVTTCue::CueBox& WebVTTCue::cueBox() {
    return m_cuePosition;
}

/**
 * @brief Edit line settings based on REGION
 * 
 * @return std::int32_t 
 */
std::int32_t WebVTTCue::computedLine() {
    return m_line;
}

/**
 * @brief Get WebVTT computed size from settings
 *
 * @return std::int32_t computed size
 */
std::int32_t WebVTTCue::computedSize() const {
    std::int32_t maxSize = computedMaxSize();

    g_logger.osdebug(__LOGGER_FUNC__, " - m_size:", m_size);
                                        
    return (m_size < maxSize ? m_size : maxSize);
}

/**
 *
 * @brief Get a max size based on the reported size, position and position align
 * @return std::int32_t
 */
std::int32_t WebVTTCue::computedMaxSize() const {
    std::int32_t max_size;
    std::int32_t computed_pos = computedPosition();

        switch (computedPositionAlign()) {
            case PositionAlignType::kLineLeft:
                max_size = 10000 - computed_pos;
                break;
            case PositionAlignType::kLineRight:
                max_size = computed_pos;
                break;
            case PositionAlignType::kCenter:
                if (computed_pos <= 5000) {
                    max_size = computed_pos * 2;
                }
                else {
                    max_size = (10000 - computed_pos) * 2;
                }
                break;
            default:
                max_size = 10000;
        }
    g_logger.osdebug(__LOGGER_FUNC__, " - computedPosition:", computed_pos,
                                        " max_size:", max_size);

    return max_size;
}

/**
 * @brief Computed position based on text alignment
 *        NOTE no BiDirectional (BIDI) Unicode support
 *
 * @return std::int32_t
 */
std::int32_t WebVTTCue::computedPosition() const {
    std::int32_t position;

    if (m_position >= 0 && m_position <= 10000)
        position = m_position;
    //Unicode BiDi not supported, so just default to l-r
    else if (m_align == AlignType::kLeft or m_align == AlignType::kStart)
        position = 0;
    else if (m_align == AlignType::kRight or m_align == AlignType::kEnd)
        position = 10000;
    else
        position = 5000;

    return position;
}

/**
 * @brief Based on text align, unless set explicitly
 *
 * @return WebVTTCue::PositionAlignType
 */
WebVTTCue::PositionAlignType WebVTTCue::computedPositionAlign() const {
    PositionAlignType posAlign;

    if (m_positionAlign != PositionAlignType::kAutoAlign)
        posAlign = m_positionAlign;
    else if (m_align == AlignType::kLeft)
        posAlign = PositionAlignType::kLineLeft;
    else if (m_align == AlignType::kRight)
        posAlign = PositionAlignType::kLineRight;
    else if (m_align == AlignType::kStart)
        if (m_textDirection == TextDirection::kLeftToRight)
            posAlign = PositionAlignType::kLineLeft;
        else
            posAlign = PositionAlignType::kLineRight;
    else if (m_align == AlignType::kEnd)
        if (m_textDirection == TextDirection::kLeftToRight)
            posAlign = PositionAlignType::kLineRight;
        else
            posAlign = PositionAlignType::kLineLeft;
    else
        posAlign = PositionAlignType::kCenter;

    return posAlign;
}

/**
 * @brief Parses line value from input string.  Can be a %age, a line number or AUTO
 *        Calculates the line value in %age hundredths of screen height
 *
 * @param value
 */
void WebVTTCue::line(const std::string& value) {
    int lineValue;
    std::istringstream iss(value);
    std::string token;

    //Get value
    std::getline(iss, token, ',');

    if (!token.compare("auto")) {
        m_line = constants::kCueAutoSetting;
        m_snapToLines = true;
    }
    else if (token.back() == '%') {
        try {
            m_line = parsePercentageHundredths(token);
            m_snapToLines = false;
        }
        catch (...) {
            m_line = constants::kCueAutoSetting;
            m_snapToLines = true;
        }
    }
    else  //line number
    {
        try {
            m_line = std::stoi(token);
            m_snapToLines = true;
        }
        catch (const std::logic_error &e) {
            g_logger.osdebug(__LOGGER_FUNC__, " - ", e.what());
            m_line = constants::kCueAutoSetting;
            m_snapToLines = true;
        }
    }

    //Get align if present
    if (std::getline(iss, token, ',')) {
        m_lineAlign = convertStringToLineAlign(token);
    }
}

/**
 * @brief Must be a %age
 *
 * @param value
 */
void WebVTTCue::size(const std::string& value) {
    m_size = parsePercentageHundredths(value);
}

/**
 * @brief Must be a %age or AUTO, can also include a position align
 *
 * @param value
 */
void WebVTTCue::position(const std::string& value) {
    std::istringstream iss(value);
    std::string token;

    std::getline(iss, token, ',');
    if (!token.compare("auto")) {
        m_position = constants::kCueAutoSetting;
    }
    else if (token.back() == '%') {
        m_position = parsePercentageHundredths(token);
    }
    else {
        throw ParserException("First position arg has to be \%age" + value);
    }

    //Get align if present
    if (std::getline(iss, token, ',')) {
        m_positionAlign = convertStringToPositionAlign(token);
    }
}

/**
 * @brief Cue text align
 *
 * @param value
 */
void WebVTTCue::align(const std::string& value) {
    m_align = convertStringToAlign(value);
}

}
}