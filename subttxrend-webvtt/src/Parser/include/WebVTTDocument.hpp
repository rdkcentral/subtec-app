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

#include <string>
#include <fstream>

#include <subttxrend/common/Logger.hpp>

#include <WebVTTCue.hpp>

namespace subttxrend {
namespace webvttengine {


struct Region {
    enum class Scroll {
        kNone,
        kUp,
        kDefault = kNone
    };

    struct Position {
        int x;
        int y;
        friend bool operator==(const Position& lhs,
                            const Position& rhs) {
            return (
                lhs.x == rhs.x &&
                lhs.y == rhs.y
                );
        }
    };

    std::string id {};
    int width_vw_h {10000};
    int lines {3};
    Position region_anchor {0, 10000};
    Position viewport_anchor {0,10000};
    Scroll scroll {Scroll::kDefault};
};

using RegionMap = std::map<std::string, Region>;

/**
 * @brief Class to parse a WebVTT document into a list of WebVTTCue objects
 * Has a single public method - parseCueList - that returns all the
 * cues in the document plus a list of display regions if present
 * See https://w3c.github.io/webvtt/#file-parsing
 */
class WebVTTDocument {
public:
    WebVTTDocument() = default;
    std::tuple<CueList, RegionMap>  parseCueList(std::istream& ifile, std::uint64_t ptsOffsetMs = 0);

//These are protected for unit testing (see test fixture below)
protected:
    std::uint64_t                   parseXTimestampMap(std::string line);
    Timing                          parseWebVTTCueTime(std::istringstream &iss);
    SettingsMap                     parseWebVTTCueSettings(std::istringstream &iss);
    bool                            parseWebVTTCueHeader(const std::string &header_line, Timing &timing, SettingsMap &settings);
    void                            checkValidWebVTTHeader(std::istream& ifile);
    SettingsMap                     parseRegionSettings(std::istream &is);
    bool                            nextCue(std::istream& is, CuePtr& cue);
    
    std::uint64_t                   m_timeOffset {0};
    Time                            m_localTime {0,0,0,0};
};

/**
 * @brief Test fixture to allow unit testing of protected methods
 *
 */
class WebVTTDocumentTestFixture : public WebVTTDocument
{
public:
    using WebVTTDocument::WebVTTDocument;

    SettingsMap parseWebVTTCueSettings(std::istringstream& iss) {
        return WebVTTDocument::parseWebVTTCueSettings(iss);
    }
    std::uint64_t parseXTimestampMap(std::string line) {
        return WebVTTDocument::parseXTimestampMap(line);
    }
    Timing parseWebVTTCueTime(std::istringstream& iss) {
        return WebVTTDocument::parseWebVTTCueTime(iss);
    }
    void checkValidWebVTTHeader(std::istream& is) {
        WebVTTDocument::checkValidWebVTTHeader(is);
    }
};  //DocTest


}
}
