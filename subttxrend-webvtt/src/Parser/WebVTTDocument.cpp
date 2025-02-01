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
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>
#include <regex>

#include <WebVTTDocument.hpp>
#include <WebVTTExceptions.hpp>

namespace subttxrend {
namespace webvttengine {

namespace {
common::Logger g_logger("WebvttEngine", "WebVTTDocument");

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

/**
 * @brief Parse the Position data member from the string representation
 * 
 * @param position_string 
 * @return WebVTTCue::Position 
 */
static Region::Position parsePosition(std::string position_string) {
    std::string token;
    std::istringstream iss(position_string);
    Region::Position position;
    int x, y;
    
    if (std::getline(iss, token, ',')) {
        x = parsePercentageHundredths(token);
        if (std::getline(iss, token, ',')) {
            y = parsePercentageHundredths(token);
            position = {x, y};
        }
    }
    
    return position;
}

/**
 * @brief Regex-based time parser to get time in HHH:MM:SS:MS format
 *
 * @param hhmmss
 * @throws ParserException if time is invalid
 * @return std::uint64_t
 */
Time parseHHMMSS(std::string hhmmss) {
    Time timeInHHMMSS = {0,0,0,0};
    try {
        bool matched = false;
        //Regex still works with no hours and/or no ms. Mins and secs are required
        std::regex re("(?:([0-9]*):)?([0-9][0-9]):([0-9][0-9])(?:\\.([0-9][0-9][0-9]))?");
        std::smatch match;
        matched = std::regex_match(hhmmss, match, re);

        if (matched) {

            if (!match.str(1).empty()) timeInHHMMSS.hours = std::stoi(match.str(1));
            if (!match.str(2).empty()) timeInHHMMSS.minutes = std::stoi(match.str(2));
            if (!match.str(3).empty()) timeInHHMMSS.seconds = std::stoi(match.str(3));
            if (!match.str(4).empty()) timeInHHMMSS.milliseconds = std::stoi(match.str(4));
        }
        else {
            throw ParserException("Failed to parse time: " + hhmmss);
        }
    }
    catch (std::regex_error& e) {
        throw ParserException("Regex error " + std::to_string(e.code()) + " from time " + hhmmss);
    }

    return timeInHHMMSS;
}

uint64_t calculateRelativeTime(Time actualTime, Time localTime)
{
    Time relativeTime = {0,0,0,0};
    uint64_t totalMs = 0;
    if(localTime.milliseconds > actualTime.milliseconds)
    {
        --actualTime.seconds;
        actualTime.milliseconds += 1000;
    }
    relativeTime.milliseconds = actualTime.milliseconds - localTime.milliseconds;
    if(localTime.seconds > actualTime.seconds)
    {
        --actualTime.minutes;
        actualTime.seconds += 60;
    }
    relativeTime.seconds = actualTime.seconds - localTime.seconds;
    if(localTime.minutes > actualTime.minutes)
    {
        --actualTime.hours;
        actualTime.minutes += 60;
    }
    relativeTime.minutes = actualTime.minutes-localTime.minutes;
    relativeTime.hours = actualTime.hours-localTime.hours;

    totalMs = relativeTime.milliseconds + (1000 * (relativeTime.seconds + (60 * (relativeTime.minutes + (60 * relativeTime.hours)))));

    return totalMs;
}

/**
 * @brief Takes a colon-separated string pair in the format {property}:{value} and parses
 *        into a std::pair
 *
 * @param input
 * @param propValPair
 * @return true
 * @return false
 */
bool parsePropertyValuePair(const std::string& input, std::pair<std::string, std::string>& propValPair) {
    std::istringstream iss(input);
    std::string property, value;
    bool valid = false;

    if (std::getline(iss, property, ':')) {
        //Gets rest of line (needed because timestamps use ':')
        std::getline(iss, value);
        if (!value.empty()) {
            propValPair = { property, value };
            valid = true;
        }
    }

    return valid;
}

std::vector<std::string> getTokenVector(std::string line, char separator) {
    std::vector<std::string> tokens;
    if (line.find(separator) != std::string::npos) {
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, separator)) {
            tokens.emplace_back(token);
        }
    }
    else {
        tokens.emplace_back(line);
    }
    return tokens;
}

/**
 * @brief Peek ahead in the stream to check for term in next line, then
 * restore the istream to the previous location
 * 
 * @param ifile 
 * @param searchString 
 * @return true 
 * @return false 
 */
bool findInNextLine(std::istream &ifile, std::string searchString) {
    std::string line;
    bool found = false;
    
    //Get current position, seek to next line, search, then seek back to original position
    std::streampos sp = ifile.tellg();
    if (std::getline(ifile, line)) {
        if (line.find(searchString) != std::string::npos)
            found = true;
    }
    ifile.seekg(sp);
    
    return found;
}

/**
 * @brief Move the stream to the next not empty lne (duh)
 * Passes out the position of the previous line so you can go back if needed
 * 
 * @param is 
 * @param lastLinePos 
 * @return std::string 
 */
std::string getNextNotEmptyLine(std::istream& is, std::streampos &lastLinePos) {
    std::string line;
    
    lastLinePos = is.tellg();
    while (std::getline(is, line)) {
        if (!line.empty()) {
            return line;
        } else {
            lastLinePos = is.tellg();
        }
    }
    throw EndOfFileException("End of file reached");
}

/**
 * @brief NOTES are comments in the WebVTT file
 *
 * @param line
 * @return true
 * @return false
 */
bool isNote(std::string line) {
    return (line.substr(0, 4) == "NOTE" ? true : false);
}

/**
 * @brief Move the istream forwards to the next empty line
 * 
 * @param is 
 */
void nextEmptyLine(std::istream& is) {
    std::string line;
    while (std::getline(is, line)) {
        if (line.empty()) {
            break;
        }
    }
}


}  //namespace

/**
 * @brief Checks for an optional UTF-8 BOM followed by WEBVTT,
 *        and an optional comment
 *        Needs to be present for a valid WebVTT file
 *
 * @param ifile
 */
void WebVTTDocument::checkValidWebVTTHeader(std::istream& ifile) {
    std::string line;

    std::getline(ifile, line);
    if (!line.empty() && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
        line.erase(0, 3);
    }

    if (line.substr(0, 6) != "WEBVTT" ||
        line.length() > 6 && !std::isspace(line[6])) {
        throw InvalidCueException("Bad WEBVTT header");
    }
}

SettingsMap WebVTTDocument::parseRegionSettings(std::istream &is) {
    std::string line;
    std::string id;
    SettingsMap regionSettings;
    
    while (std::getline(is, line) && !line.empty()) {
        SettingsItem item;
        if (parsePropertyValuePair(line, item)) {
            regionSettings.insert(item);
        }
    }
    
    return regionSettings;
}

/**
 * @brief Parses an HLS X-TIMESTAMP-MAP header.  This will contain an optional
 *        PTS offset and should match the AV content
 *
 * @param line
 * @param ptsOffset
 * @return true
 * @return false
 */
std::uint64_t WebVTTDocument::parseXTimestampMap(std::string line) {
    std::istringstream iss(line);
    std::string token;
    std::uint64_t ptsOffset = 0;

    if (std::getline(iss, token, '=') && token == "X-TIMESTAMP-MAP") {
        std::getline(iss, token, '=');
        SettingsMap settingsMap;
        std::uint64_t mpegts = 0;
        Time localtime = {0,0,0,0};

        //Settings are in the format SETTING:VALUE,SETTING:VALUE
        //So split on ',' first, then use the standard methods
        for (const auto& setting : getTokenVector(token, ',')) {
            SettingsItem propValPair;
            if (parsePropertyValuePair(setting, propValPair)) {
                settingsMap.insert(propValPair);
            }
        }

        for (const auto& setting : settingsMap) {
            if ("MPEGTS" == setting.first) {
                try {
                    mpegts = static_cast<std::uint64_t>(std::stoll(setting.second));
                }
                catch (...) {    //I know...
                    mpegts = 0;
                }
            }
            else if ("LOCAL" == setting.first) {
                try {
                    localtime = parseHHMMSS(setting.second);
                }
                catch (const ParserException& e) {
                    localtime = {0,0,0,0};
                }
            }
        }
        ptsOffset = (mpegts / 90);
        m_localTime = localtime;
    }

    return ptsOffset;
}

/**
 * @brief Gets all the Cue Header information.  This consists of a mandatory timestamp
 *        that must be valid, followed by an optional space-separated list of inline
 *        positioning variables
 *        In the absence of the extra positioning data, auto positioning will be used
 *        (center aligned, auto-positioned at the bottom of the screen, full screen width)
 *
 * @param headerLine the header string - will be one line
 * @param timing Reference for the m_timing data to pass back to the caller
 * @param settings Optional std::map of settings to pass back
 * @return true
 * @return false
 */
bool WebVTTDocument::parseWebVTTCueHeader(const std::string& headerLine, Timing& timing, SettingsMap& settings) {
    std::istringstream iss(headerLine);
    bool ret = true;

    try {
        timing = parseWebVTTCueTime(iss);
        settings = parseWebVTTCueSettings(iss);
    }
    catch (const InvalidCueException& e) {
        g_logger.warning("%s", e.what());
        ret = false;
    }

    return ret;
}

/**
 * @brief Will throw an InvalidCueException on any error - must be correct for
 *        the cue to be valid
 *
 * @param iss
 * @throws InvalidCueException
 * @return Timing
 */
Timing WebVTTDocument::parseWebVTTCueTime(std::istringstream& iss) {
    bool ret = false;
    TimePoint start, end;
    std::string token;

    try {
        if (iss.str().find("-->") != std::string::npos) {
            // Get start time
            iss >> token;
            try {
                std::uint64_t startTimeMs;
                Time startTime = parseHHMMSS(token);
                startTimeMs = calculateRelativeTime(startTime, m_localTime);
                start = TimePoint(startTimeMs);
            }
            catch (const ParserException&) {
                throw ParserException("Bad start time " + token);
            }

            // Check for -->
            iss >> token;
            if (token != "-->") {
                throw ParserException("Bad --> delimiter");
            }

            // Get end time
            iss >> token;
            try {
                std::uint64_t endTimeMs;
                Time endTime = parseHHMMSS(token);
                endTimeMs = calculateRelativeTime(endTime, m_localTime);
                end = TimePoint(endTimeMs);
            }
            catch (const ParserException&) {
                throw ParserException("Bad end time " + token);
            }
        }
        else {
            throw ParserException("Missing --> delimiter");
        }
    }
    catch (const ParserException& e) {
        throw InvalidCueException(e.what());
    }

    return Timing(start, end);
}

/**
 * @brief Parses the optional settings into a std::map
 *
 * @param iss
 * @return SettingsMap
 */
SettingsMap WebVTTDocument::parseWebVTTCueSettings(std::istringstream& iss) {
    SettingsMap settingsMap;
    std::string token;

    while (iss) {
        iss >> token;
        if (!token.empty()) {
            try {
                SettingsItem propValPair;
                if (parsePropertyValuePair(token, propValPair)) {
                    settingsMap.insert(propValPair);
                }
            }
            catch (const ParserException& e) {
                g_logger.warning("Error %s - clear settings map and return", e.what());
                settingsMap.clear();
                break;
            }
        }
    };

    return settingsMap;
}


/**
 * @brief Returns the next WebVTTCue object in the stream
 *        Will parse header, settings and all cue text into a WebVTTCue
 *        object and return
 *
 * @param is Input stream
 * @param cue WebVTTCue object
 * @throws ParserException
 * @throws InvalidCueException
 * @return true
 * @return false
 */
bool WebVTTDocument::nextCue(std::istream& is, CuePtr& cue) {
    std::string line, id;
    Timing cueTime;
    SettingsMap settings;
    bool ret = false;

    if (std::getline(is, line)) {
        if (isNote(line)) {
            g_logger.info("NOTE: found in WebVTT file");
            nextEmptyLine(is);
            if (!std::getline(is, line)) {
                return false;
            }
            g_logger.debug("Line after NOTE: %s", line.c_str());
        }

        //If first line of cue is not the timeline, it's the ID
        if (line.find("-->") == std::string::npos) {
            id = line;
            if (!std::getline(is, line)) {
                return false;
            }
        }

        if (parseWebVTTCueHeader(line, cueTime, settings)) {
            cueTime.applyOffset(m_timeOffset);
            cue = std::make_unique<WebVTTCue>(cueTime);
            cue->addCueSettings(settings);

            while (std::getline(is, line)) {
                if (!line.empty()) {
                    g_logger.debug("Adding text \"%s\" length %d last char 0x%X", line.c_str(), (int)line.length(), line.back());
                    cue->addTextLine(line);
                }
                else {
                    // Delimited by empty line so break
                    g_logger.debug("End of cue");
                    break;
                }
            }
            ret = true;
        }
        else
            throw InvalidCueException("Skip to next cue");
    }

    return ret;
}

/**
 * @brief Parse a REGION from the file header into a Region object
 * 
 * @param regionSettings 
 * @return Region 
 */
Region ParseRegion(const SettingsMap& regionSettings) {
    Region region;
    for (const auto &pair : regionSettings) {
        std::string property = pair.first;
        std::string value = pair.second;
        
        if (!property.compare("id")) {
            region.id = value;
        } else if (!property.compare("width")) {
            try {
                region.width_vw_h = parsePercentageHundredths(value);
            } catch (const ParserException &e) {
                g_logger.osinfo(__LOGGER_FUNC__, e.what());
                g_logger.osinfo(__LOGGER_FUNC__, " - failed to parse width ", value);
            }
        } else if (!property.compare("lines")) {
            try {
                region.lines = std::stoi(value);
            } catch (const std::logic_error &e) {
                g_logger.osinfo(__LOGGER_FUNC__, e.what());
                g_logger.osinfo(__LOGGER_FUNC__, " - failed to parse lines ", value);
            }
        } else if (!property.compare("regionanchor")) {
            try {
                region.region_anchor = parsePosition(value);
            } catch (const ParserException &e) {
                g_logger.osinfo(__LOGGER_FUNC__, e.what());
                g_logger.osinfo(__LOGGER_FUNC__, " - failed to parse region anchor ", value);
            }
        } else if (!property.compare("viewportanchor")) {
            try {
                region.viewport_anchor = parsePosition(value);
            } catch (const ParserException &e) {
                g_logger.osinfo(__LOGGER_FUNC__, e.what());
                g_logger.osinfo(__LOGGER_FUNC__, " - failed to parse viewport anchor ", value);
            }
        } else if (!property.compare("scroll")) {
            if (value == "up") {
                region.scroll = Region::Scroll::kUp;
            }
        } else {
            g_logger.osinfo(__LOGGER_FUNC__, " - Bad prop/value pair in region settings: ", 
                                            property, "/", value);
        }
    }
    
    return region;
}

/**
 * @brief Parses the whole stream into a std::list of WebVTTCue objects and
 * a map of Region objects
 *
 * @param ifile
 * @return CueList
 */
std::tuple<CueList, RegionMap> WebVTTDocument::parseCueList(std::istream& ifile, 
                                                            std::uint64_t ptsOffsetMs) {
    std::string line;
    uint64_t offset = 0;
    CuePtr cue;
    CueList list;
    RegionMap regionMap;

    checkValidWebVTTHeader(ifile);
    
    do {
        try {
            std::streampos prevLinePosition = ifile.tellg();
            line = getNextNotEmptyLine(ifile, prevLinePosition);
            if (line.find("X-TIMESTAMP-MAP") != std::string::npos) {
                offset = parseXTimestampMap(line);
                m_timeOffset = offset - ptsOffsetMs;
                g_logger.osinfo(__LOGGER_FUNC__, " - offset from index file:", offset, 
                                                " offset from data packet:", ptsOffsetMs,
                                                " Overall offset to apply: ", m_timeOffset);
            } else if (line.find("REGION") != std::string::npos) {
                SettingsMap regionSettings = parseRegionSettings(ifile);
                Region region = ParseRegion(regionSettings);
                regionMap.emplace(region.id, region);
            } else if (line.find("STYLE") != std::string::npos) {
                //Not implemented - just skip
                g_logger.osinfo(__LOGGER_FUNC__, " - Found STYLE block but not feature not yet implemented");
                nextEmptyLine(ifile);
            } else if (line.find("NOTE") != std::string::npos) {
                g_logger.osinfo(__LOGGER_FUNC__, " - Found NOTE block - skipping");
                nextEmptyLine(ifile);
            } else if (line.find("-->") != std::string::npos || findInNextLine(ifile, "-->")) {
                //Finished parsing the various headers
                //Go back a line and break so we can parse the cues
                ifile.seekg(prevLinePosition);
                break;
            }
        } catch (const EndOfFileException &e) {
            g_logger.osinfo(__LOGGER_FUNC__, " - ", e.what());
            break;
        }
    } while (ifile);
    
    
    //Now have a list of Regions
    
    
    while (ifile) {
        try {
            if (nextCue(ifile, cue)) {
                list.emplace_back(std::move(cue));
            }
        } catch (const InvalidCueException& e) {
            g_logger.info("%s", e.what());
            nextEmptyLine(ifile);
        }
    }

    return std::make_tuple(std::move(list), regionMap);
}

}
}
