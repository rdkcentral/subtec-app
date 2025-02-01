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

#include <list>
#include <vector>
#include <map>
#include <memory>

#include <Timing.hpp>
#include <Constants.hpp>

#include <subttxrend/common/Logger.hpp>

namespace subttxrend {
namespace webvttengine {
using SettingsMap = std::map<std::string, std::string>;
using SettingsItem = std::pair<std::string, std::string>;
using LineVector = std::vector<std::string>;

/**
 * @brief Main class for holding WebVTT cue information (cue timing, text and positioning data).
 * Calculates the *computed* size and position for a cue, which is based on the size, position and
 * align values and will be used by the LineBuilder to calculate the final display postition
 * See https://w3c.github.io/webvtt/#cues
 */
class WebVTTCue {
public:

    enum class LineAlignType : int {
        kStart,
        kCenter,
        kEnd,
        kDefault = kStart
    };

    enum class PositionAlignType : int {
        kLineLeft,
        kCenter,
        kLineRight,
        kAutoAlign,
        kDefault = kAutoAlign
    };

    enum class TextDirection : int {
        kLeftToRight,
        kRightToLeft,
        kDefault = kLeftToRight
    };

    enum class AlignType : int {
        kStart,
        kCenter,
        kEnd,
        kLeft,
        kRight,
        kDefault = kCenter
    };

    struct CueBox {
        int             computedPositionVwH;
        int             lineVhH;
        int             computedSizeVwH;
        AlignType       cueTextAlign;
        LineAlignType   cueLineAlign;
        bool            snapToLines;

        friend std::ostream& operator<<(std::ostream &os, CueBox &cb) {
            std::string alignString;
            switch(cb.cueTextAlign) {
                case AlignType::kCenter:
                    alignString = "CENTER";
                    break;
                case AlignType::kLeft:
                    alignString = "LEFT";
                    break;
                case AlignType::kRight:
                    alignString = "RIGHT";
                    break;
                case AlignType::kStart:
                    alignString = "START";
                    break;
                case AlignType::kEnd:
                    alignString = "END";
                    break;
                default:
                    alignString = "INVALID";
            }
            os << "{p:" << cb.computedPositionVwH << 
                " l:" << cb.lineVhH << 
                " s:" << cb.computedSizeVwH << 
                " a:" << alignString << 
                "}";
            return os;
        }
    };
    
    explicit WebVTTCue(Timing timing) :
        m_timing(timing),
        m_logger("WebvttEngine", "WebVTTCue", this) {}

    WebVTTCue(Timing timing, std::uint64_t timeOffset) : WebVTTCue(timing) 
    {
        m_timeOffset = timeOffset;
    }

    void addTextLine(const std::string& line);
    void addCueSettings(const SettingsMap& settings);
    
    void identifier(const std::string& id) { m_identifier = id; }

    //Public Getters
    inline TimePoint      startTime() const { return m_timing.getStartTimeRef(); }
    inline TimePoint      endTime() const { return m_timing.getEndTimeRef(); }
    inline LineVector     lines() const { return m_lines; }
    inline std::string    identifier() const { return m_identifier; }
    inline std::string    regionId() const { return m_regionId; }

    const CueBox&         cueBox();

    friend bool operator<(const WebVTTCue& lhs, const WebVTTCue& rhs) {
        return lhs.m_timing < rhs.m_timing;
    }
    
    friend std::ostream& operator<<(std::ostream &os, WebVTTCue &cue) {
        os << "[" << cue.m_timing << "]: ";

        for (const auto& line : cue.lines()) {
            os << line;
        }
        return os;
    }
    
    friend bool operator==(const WebVTTCue &lhs, const WebVTTCue &rhs) {
        //Doesn't include offset because it's passed in from the data
        //packet and could be different for identical cues
        return (
            lhs.m_timing == rhs.m_timing &&
            lhs.m_identifier == rhs.m_identifier &&
            lhs.m_line == rhs.m_line &&
            lhs.m_size == rhs.m_size &&
            lhs.m_position == rhs.m_position &&
            lhs.m_align == rhs.m_align &&
            lhs.m_lineAlign == rhs.m_lineAlign &&
            lhs.m_positionAlign == rhs.m_positionAlign &&
            lhs.m_lines == rhs.m_lines &&
            lhs.m_textDirection == rhs.m_textDirection
        );
    }

private:
    /** Logger object. */
    mutable subttxrend::common::Logger m_logger;

    std::int32_t                computedSize() const;
    std::int32_t                computedPosition() const;
    std::int32_t                computedLine();
    PositionAlignType           computedPositionAlign() const;
    std::int32_t                computedMaxSize() const;
            
    void                        line(const std::string& line);
    void                        size(const std::string& size);
    void                        position(const std::string& position);
    void                        align(const std::string& align);
    
    static constexpr int        kDefaultMaxSize {10000};
    static constexpr int        kDefaultComputedPosition {5000};
                                                    
    Timing                      m_timing {};
    std::uint64_t               m_timeOffset {0};
    std::string                 m_identifier {};
    CueBox                      m_cuePosition = {kDefaultComputedPosition, constants::kCueAutoSetting,
                                            kDefaultMaxSize, AlignType::kDefault,
                                            LineAlignType::kDefault, true};
    std::string                 m_regionId {};
    int                         m_line {constants::kCueAutoSetting};
    int                         m_size {kDefaultMaxSize};
    int                         m_position {constants::kCueAutoSetting};
    bool                        m_snapToLines {true};
    AlignType                   m_align {AlignType::kDefault};
    LineAlignType               m_lineAlign {LineAlignType::kDefault};
    PositionAlignType           m_positionAlign {PositionAlignType::kDefault};
    std::vector<std::string>    m_lines {};
    TextDirection               m_textDirection {TextDirection::kLeftToRight};
};

using CuePtr = std::unique_ptr<WebVTTCue>;
using CueSharedPtr = std::shared_ptr<WebVTTCue>;
using CueList = std::list<CuePtr>;
using CueSharedList = std::list<CueSharedPtr>;

}
}
