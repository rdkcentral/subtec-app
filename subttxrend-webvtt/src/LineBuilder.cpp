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
#include <algorithm>
#include <unordered_map>
#include <map>

#include <subttxrend/gfx/Types.hpp>

#include <WebVTTStyle.hpp>
#include <LineBuilder.hpp>
#include <WebVTTExceptions.hpp>


namespace subttxrend {
namespace webvttengine {

namespace {
common::Logger g_logger("WebvttEngine", "LineBuilder"); 

using namespace linebuilder;

LineList splitLinesByScreenWidth(const Line& tokens, int maxWidthPx) {
    LineList lines;
    Line currentLine;
    auto lineWidth = tokens.lineWidth;

    int numberOfLines = ((lineWidth - 1) / maxWidthPx) + 1;
    int lineDivide = lineWidth / numberOfLines;
    int totalAdvance = 0;
    
    g_logger.ostrace(__LOGGER_FUNC__, " - lineDivide:", lineDivide, " lineWidth:", lineWidth);

    for (const auto& token : tokens.tokenVector) {
        totalAdvance = totalAdvance + token.width;
        g_logger.ostrace(__LOGGER_FUNC__, " - totalAdvance:", totalAdvance);
        if (totalAdvance > lineDivide) {
            //If the last token in the line is whitespace, skip it
            if (!token.token->isWhite) {
                currentLine.tokenVector.push_back(token);
                currentLine.lineWidth = totalAdvance;
            } else {
                currentLine.lineWidth = totalAdvance - token.width;
            }

            // If the token causes the line to overflow, move it to the next
            // line, unless it is the only token on the line (line width equals
            // token width).
            if ((currentLine.lineWidth > maxWidthPx) && (currentLine.lineWidth > token.width)) {
                //Pop the last token and add it to the start of the next line
                auto last_token = currentLine.tokenVector.back();
                currentLine.tokenVector.pop_back();
                currentLine.lineWidth -= token.width;
                
                lines.push_back(currentLine);
                g_logger.ostrace(__LOGGER_FUNC__, " - push_back after pop line l_w:", currentLine.lineWidth);
                
                currentLine.tokenVector.clear();
                currentLine.tokenVector.push_back(last_token);
                currentLine.lineWidth = token.width;
                totalAdvance = token.width;
            } else {
                lines.push_back(currentLine);
                g_logger.ostrace(__LOGGER_FUNC__, " - push_back line l_w:", currentLine.lineWidth);
                currentLine.tokenVector.clear();
                totalAdvance = 0;
            }
        } else {
            currentLine.tokenVector.push_back(token);
        }
    }
    if (totalAdvance > 0) {
        currentLine.lineWidth = totalAdvance;
        g_logger.ostrace(__LOGGER_FUNC__, " - final push_back line l_w:", currentLine.lineWidth);
        lines.push_back(currentLine);
    }

    return lines;
}

gfx::ColorArgb getTextColourFromMap(std::string textColour) {
    static const std::map<std::string, gfx::ColorArgb> textmap {
        { "white", Style::kWhite },
        { "lime", Style::kLime },
        { "cyan", Style::kCyan },
        { "red", Style::kRed },
        { "yellow", Style::kYellow },
        { "magenta", Style::kMagenta },
        { "blue", Style::kBlue },
        { "black", Style::kBlack },
    };

    if (textmap.count(textColour)) {
        return textmap.at(textColour);
    } else {
        g_logger.info("Style %s not found - returning WHITE", textColour.c_str());
        return Style::kWhite;
    }
}

gfx::ColorArgb getBgColourFromMap(std::string bgColour) {
    static const std::map<std::string, gfx::ColorArgb> textmap {
        { "bg_white", Style::kWhite },
        { "bg_lime", Style::kLime },
        { "bg_cyan", Style::kCyan },
        { "bg_red", Style::kRed },
        { "bg_yellow", Style::kYellow },
        { "bg_magenta", Style::kMagenta },
        { "bg_blue", Style::kBlue },
        { "bg_black", Style::kBlack },
    };

    if (textmap.count(bgColour)) {
        return textmap.at(bgColour);
    } else {
        g_logger.info("Style %s not found - returning BLACK", bgColour.c_str());
        return Style::kBlack;
    }
}

/**
 * @brief Colour tags can be eg: <c.lime.cyan.bg_white.bg_yellow> and last in the list
 *        takes precedence, so the above would be cyan on a yellow background
 *
 * @param styleString
 * @param style
 * @return Style
 */
Style addColourToStyle(const std::string& styleString, Style style) {
    std::istringstream iss(styleString);
    std::string colourClass;
    gfx::ColorArgb colour;

    while (std::getline(iss, colourClass, '.')) {
        if (colourClass == "c")
            continue;

        if (colourClass.find("bg_") != std::string::npos)
            style.bgColour(getBgColourFromMap(colourClass));
        else
            style.textColour(getTextColourFromMap(colourClass));
    }

    return style;
}

Style getStyleFromMap(std::string styleString) {
    static const std::map<std::string, Style> styleMap {
        { "i", { Style::kDefaultFontFamilyString, Style::FontStyleType::kItalic, Style::kDefaultTextColour, Style::kDefaultBgColour } },
        { "b", { Style::kDefaultFontFamilyString, Style::FontStyleType::kBold, Style::kDefaultTextColour, Style::kDefaultBgColour } },
        { "u", { Style::kDefaultFontFamilyString, Style::FontStyleType::kUnderline, Style::kDefaultTextColour, Style::kDefaultBgColour } },
    };

    if (styleMap.count(styleString)) {
        return styleMap.at(styleString);
    } else {
        g_logger.trace("Style %s not found - returning default", styleString.c_str());
        return Style {};
    }
}

/**
 * @brief Add Style to and existing Style object
 *
 * @param styleString Style to add
 * @param style reference
 */
void addStyle(const std::string& styleString, Style &style) {
    g_logger.ostrace(__LOGGER_FUNC__, " - stylestring ", styleString);
    if (styleString.empty()) {
        return;
    }

    switch (styleString[0]) {
    case 'i':
        style.fontStyle(Style::FontStyleType::kItalic);
        break;
    case 'b':
        style.fontStyle(Style::FontStyleType::kBold);
        break;
    case 'u':
        style.fontStyle(Style::FontStyleType::kUnderline);
        break;
    case 'c':
        style = addColourToStyle(styleString, style);
        break;
    default:
        g_logger.info("Unsupported style class");
        break;
    }
}

/**
 * @brief Regex-ish method to parse a tag and move the index to one after the end tag
 *
 * @param search
 * @param tag
 * @return std::size_t One after the end tag
 */
std::size_t getTagAndRemainder(std::string search, std::string& tag) {
    std::istringstream iss(search);
    std::string remainder;
    std::size_t index = search.find('>');

    if (index != std::string::npos) {
        tag = search.substr(0, index);
        remainder = search.substr(index + 1);
        g_logger.ostrace(__LOGGER_FUNC__, " - tag:", tag, " remainder:", remainder);
        return index + 1;
    }
    return 0;
}

static void findAndReplace(std::string &in_string, const std::string &findString, const std::string &replaceString) {
    std::size_t pos = 0;
    
    while ((pos = in_string.find(findString, pos)) != std::string::npos) {
        in_string.replace(pos, (int)findString.length(), replaceString);
        pos += replaceString.length();
    }
}

/**
 * @brief Replace escaped unicode chars <, >, &, ", ' in the string
 * 
 * @param editString 
 */
static void replaceEscapedCharacters(std::string &editString) {
    std::map<std::string, std::string> findReplace {
        {"&lt;", "<"}, {"&gt;", ">"}, {"&amp;", "&"}, {"&quot;", "\""}, {"&apos;", "\'"}
    };

    std::for_each(findReplace.begin(), findReplace.end(),
                    [&editString](const std::pair<std::string, std::string> item) {
                        findAndReplace(editString, item.first, item.second);
                    });
}

/**
 * @brief Recursive method to parse an HTML-formatted string (ie one<b>two<i>three</i>four</b>)
 * Handles simple classes only - i, b, u, c.textColour
 * As it unwinds, it creates Result objects with a text string and a Style object.  Also replaces
 * escaped characters in the text string (&lt;, &gt; etc)
 * 
 * @param search 
 * @param results 
 * @param style_list 
 * @return std::string 
 */
std::string searchTag(const std::string& search, std::vector<Result>& results, std::list<std::string>& styleList) {
    std::string prefix, suffix, tag;
    std::list<std::string> localList;
    Result result;
    Style style;
    std::size_t index = search.find('<');
    prefix = search.substr(0, index);

    try {
        if (index != std::string::npos) {
            index++;
            bool close_tag = (search[index] == '/');
            if (close_tag) index++;

            index += getTagAndRemainder(search.substr(index), tag);
            suffix = search.substr(index);

            if (!close_tag)
                styleList.push_back(tag);
            else
                styleList.remove(tag);

            g_logger.osdebug(__LOGGER_FUNC__, " - prefix:", prefix,
                                              " suffix:", suffix,
                                              " tag:", tag,
                                              " close_tag:", close_tag,
                                              " local_list.size:", localList.size(),
                                              " styleList.size:", styleList.size());

            localList = styleList;
            result.text = searchTag(suffix, results, styleList);

            for (const auto& styleString : localList)
                addStyle(styleString, style);

            result.style = style;
            if (!result.text.empty()) {
                g_logger.osdebug(__LOGGER_FUNC__, " - push_back t:", result.text, " s:", result.style.getStyleName());
                results.push_back(result);
            }
        }
        // I know - I want to ensure that the caller has the opportunity to
        //always return something
    }
    catch (...) {
        throw ParserException("Error in recursive function for " + search);
    }

    replaceEscapedCharacters(prefix);

    return prefix;
}

/**
 * @brief Just strip the tags from str without attempting to parse them
 * 
 * @param str 
 */
void stripHtmlTags(std::string& str) {
    size_t startpos = std::string::npos, endpos = std::string::npos;

    do {
        startpos = str.find_first_of('<');
        if (startpos != std::string::npos) {
            endpos = str.find_first_of('>');
            if (endpos != std::string::npos) {
                str.erase(startpos, endpos - startpos + 1);
                continue;
            }
        }
        break;
    } while (true);
}

/**
 * @brief Calls the searchTag recursive method to parse all the styles from
 * a formatted string.  If there are errors it will just return a simple string
 * with the tags stripped.
 * 
 * @param line 
 * @return std::vector<Result> 
 */
std::vector<Result> parseStyles(const std::string &line) {
    std::string search, res;
    std::vector<Result> results;
    Result result;

    try {
        std::list<std::string> styleList{};
        result.style = Style{};
        result.text = searchTag(line, results, styleList);
        if (!result.text.empty()) {
            if (styleList.size() > 0) {
                for (const auto& styleString : styleList)
                    addStyle(styleString, result.style);
            }
            results.push_back(result);
        }

        std::reverse(results.begin(), results.end());

        for(const auto &res : results) {
            g_logger.ostrace(__LOGGER_FUNC__, " - text:", res.text, " style:", res.style.getFontStyle());
        }
    }
    catch (const ParserException& e) {
        results.clear();
        std::string stripped = line;
        stripHtmlTags(stripped);
        results.emplace_back(Result{stripped, Style{}});
        g_logger.info("Badly formed tags in %s", line.c_str());
    }
    
    return results;
}

/**
 * @brief Special case - all boxes are off the screen so we need to switch the adjustment direction
 * 
 * @param boxes 
 * @param step 
 * @param viewportHeight 
 * @return true 
 * @return false 
 */
bool switchPosition(const std::list<Line> &boxes, const int step, const int viewportHeight) {
    if (step < 0) {
        if (boxes.front().lineRectangle.m_y < 0)
            return true;
    } else {
        if (boxes.back().lineRectangle.m_y > viewportHeight)
            return true;
    }
    return false;
}

/**
 * @brief Top of first box or the bottom of the last box is off the screen
 * 
 * @param boxes 
 * @param viewportHeight 
 * @return true 
 * @return false 
 */
bool needsAdjustment(const std::list<Line> &boxes, const int viewportHeight) {
    if (boxes.empty()) {
        return false;
    } else if (boxes.front().lineRectangle.m_y < 0 ||
        (boxes.back().lineRectangle.m_y + boxes.back().lineRectangle.m_h) > viewportHeight) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Groups cues by region. This helps with auto positioning in an area when there
 * is more than one cue to display.
 * 
 * @param webvttCueList 
 * @param regionMap 
 * @return std::unordered_map<std::string, CueSharedList> 
 */
std::unordered_map<std::string, CueSharedList> sortCuesByRegion(const CueSharedList& webvttCueList) {
    std::unordered_map<std::string, CueSharedList> cuesByRegion;
    
    for (const auto &cue : webvttCueList) {
        auto regionId = cue->regionId();
        if (regionId.empty()) regionId = "m_Empty";
        cuesByRegion[regionId].push_back(cue);
    }
    
    return cuesByRegion;
}

gfx::ColorArgb setOpacity(gfx::ColorArgb color, WebVTTAttributes::Opacity opacity)
{
    uint8_t alpha = 255;
    switch (opacity)
    {
        case WebVTTAttributes::Opacity::SOLID:
            alpha = 255;
            break;
        case WebVTTAttributes::Opacity::TRANSLUCENT:
            alpha = 100;
            break;
        case WebVTTAttributes::Opacity::TRANSPARENT:
            alpha = 0;
            break;
    }
    return gfx::ColorArgb(alpha,color.m_r,color.m_g,color.m_b);
}


}  // namespace

/**
 * @brief Takes a single line from a WebVTTCue and adds style metadata for use by the render loop
 * 
 * @param line 
 * @return WebVTTDraw::Line 
 */
Line LineBuilder::buildTokensForLine(const std::vector<Result> &lineSegments) {
    Line currentLineTokens;
    auto lineWidthPx = 0;
    
    for (const auto &lineSegment : lineSegments) {
        std::vector<Token> chunkTokens;
        auto style = lineSegment.style;
        getUserDefinedColorAttributes(style);
        auto fontname = m_fontFamily + " " + style.getFontStyle();
        auto fontSize = m_converter.fontSizePixels();
        auto font = getFont(fontname, fontSize);
        for (const auto &token : font->textToTokens(lineSegment.text)) {
            auto tokenWidth = static_cast<int>(token.totalAdvanceX);
            g_logger.osdebug(__LOGGER_FUNC__, " - token width:", tokenWidth);
            chunkTokens.emplace_back(Token {
                tokenWidth,
                Style(style),
                std::make_shared<gfx::TextTokenData>(token),
                FontPtr(font) });
            lineWidthPx += tokenWidth;
        }
        
        g_logger.osdebug(__LOGGER_FUNC__, " - Adding ", chunkTokens.size(), " tokens - w:", lineWidthPx);

        currentLineTokens.tokenVector.insert(currentLineTokens.tokenVector.end(), 
                                                    chunkTokens.begin(), 
                                                    chunkTokens.end());
    }
    
    g_logger.osdebug(__LOGGER_FUNC__, " - Total line width:", lineWidthPx);

    currentLineTokens.lineWidth = lineWidthPx;
    return currentLineTokens;
}

/**
 * @brief Takes the raw line strings and converts them into Tokens that the renderer can
 * display.  Splits lines if necessary.
 * 
 * @param line_strings 
 * @param maximum_size 
 * @return std::list<Line> 
 */
std::list<Line> LineBuilder::buildLines(const std::vector<std::string> line_strings, int maxLineSize) {
    std::list<Line> lines;
    for (const auto &cue_line : line_strings) {
        auto lineSegments = parseStyles(cue_line);
        auto fullLine = buildTokensForLine(lineSegments);
        
        g_logger.osdebug(__LOGGER_FUNC__, " lineWidth:", fullLine.lineWidth, " max:", maxLineSize);

        if (fullLine.lineWidth > maxLineSize) {
            auto split_lines = splitLinesByScreenWidth(fullLine, maxLineSize);
            g_logger.osdebug(__LOGGER_FUNC__, " split line into ", split_lines.size());
            lines.insert(lines.end(), split_lines.begin(), split_lines.end());
        } else {
            lines.push_back(fullLine);
        }
    }
    return lines;
}

/**
 * @brief Gfx module uses FcMatch to get the font
 *
 * @param fontFamily
 * @param size
 * @return std::shared_ptr<gfx::PrerenderedFont>
 */
std::shared_ptr<gfx::PrerenderedFont> LineBuilder::getFont(std::string fontFamily, const std::uint32_t size) {
    std::shared_ptr<gfx::PrerenderedFont> font;
    
    g_logger.osdebug(__LOGGER_FUNC__, " - get fontFamily:", fontFamily, " size:", size);
    
    try {
        font = m_fontCache->getFont(fontFamily, size, true);
    }
    catch (const std::exception& e) {
        g_logger.warning("getFont failed - trying fallback \"sans\"");
        //Will throw on fail - this means we can't display anything!
        font = m_fontCache->getFont("sans", size);
    }
    return font;
}


void LineBuilder::linePositionsBeforeAdjustment(std::list<Line> &boxes, WebVTTCue::AlignType align, 
                                                int startingY, int positionPx, bool setY) {
    auto y = startingY, x = 0;
    for (auto &cueLine : boxes) {
        auto lineWidth = cueLine.lineWidth + (m_converter.horizontalPadding() * 2);
        x = m_converter.getXForTextBox(lineWidth, align, positionPx) + m_converter.screenPaddingWidthPixels();
        cueLine.lineRectangle = gfx::Rectangle(x, y, lineWidth, m_converter.lineHeightPixels());
        cueLine.linePaddingX = m_converter.horizontalPadding();
        if (setY) y += m_converter.lineHeightPixels();
    }
}

/**
 * @brief Calculation for rendering Region lines
 * Returns a list of lines with embedded display data
 * See https://w3c.github.io/webvtt/#processing-model
 * 
 * @param cueList 
 * @param region 
 * @return std::list<Line> 
 */
std::list<Line> LineBuilder::getRegionLines(const CueSharedList &cueList, const Region &region) {
    std::list<Line> regionLines;
    
    try {
        const auto regionWidthVwH = region.width_vw_h;
        const auto linesVhH = m_converter.lineHeightVh() * region.lines;
        const auto linesPx = m_converter.vhToHeightPixels(linesVhH);
        const auto left = region.viewport_anchor.x - (region.region_anchor.x * (regionWidthVwH / 10000.0));
        const auto leftPx = m_converter.vwToWidthPixels(left);
        const auto top = region.viewport_anchor.y - (region.region_anchor.y * (linesVhH / 10000.0));
        const auto topPx = m_converter.vhToHeightPixels(top);
        const auto scroll = region.scroll;
                
        auto lineHeightPx = m_converter.lineHeightPixels();
        auto y = topPx;
        
        auto yAdjust = [lineHeightPx, &y](Line &line) {
            line.lineRectangle.m_y = y;
            y += lineHeightPx;
        };

        bool reachedMaxLinesInRegion = false;
        auto doTokens = [this, &y, &regionLines, region, leftPx, scroll, &reachedMaxLinesInRegion](const auto &cue) {
            if (reachedMaxLinesInRegion) return;
            const auto regionWidthVwH = region.width_vw_h;
            const auto regionWidthPx = m_converter.vwToWidthPixels(regionWidthVwH);
            
            auto lines = this->buildLines(cue->lines(), regionWidthPx);
            
            auto pop_front_n = [](auto &items, int number) 
                            { for (int i = 0; i < number; i++) items.pop_front(); };
            auto pop_back_n = [](auto &items, int number) 
                            { for (int i = 0; i < number; i++) items.pop_back(); };
            
            //Does this push us over the region's line limit?
            if ((int)(regionLines.size() + lines.size()) > region.lines) {
                reachedMaxLinesInRegion = true;
                if (scroll == Region::Scroll::kUp)
                    pop_front_n(regionLines, lines.size());
                else if (scroll == Region::Scroll::kNone)
                    return;
            }
            
            const auto cueBox = cue->cueBox();
            const auto align = cueBox.cueTextAlign;
            const auto offset = cueBox.computedPositionVwH * regionWidthVwH / 10000.0;
            const auto offsetPx = m_converter.vwToWidthPixels(offset);

            //Calculate the line positions
            this->linePositionsBeforeAdjustment(lines, align, 0, offsetPx + leftPx, false);
            regionLines.insert(regionLines.end(), lines.begin(), lines.end());
        };
        
        if (region.scroll == Region::Scroll::kUp) {
            std::for_each(cueList.begin(), cueList.end(), doTokens);
        } else {
            std::for_each(cueList.rbegin(), cueList.rend(), doTokens);
        }
        
        std::for_each(regionLines.begin(), regionLines.end(), yAdjust);
        
        //Shuffle lines down to the bottom of the area
        auto display_lines_height = regionLines.size() * m_converter.lineHeightPixels();
        auto delta = linesPx - display_lines_height;
        if (delta > 0) {
            for (auto &line : regionLines) {
                line.lineRectangle.m_y += delta;
            }
        }
    } catch (const std::out_of_range &e) {
        g_logger.oswarning(__LOGGER_FUNC__, e.what(),  " - can't find region - continue with defaults");
        throw ParserException(e.what());
    }
    
    return regionLines;
}

/**
 * @brief Calculate positions for cues not in a region
 * See https://w3c.github.io/webvtt/#processing-model
 * 
 * @param cueList 
 * @return std::list<Line> 
 */
std::list<Line> LineBuilder::getOutputLines(const CueSharedList &cueList) {
    std::list<Line> outputLines;
    auto autoLine = 0;
    const auto startingY = m_converter.screenPaddingHeightPixels();

    for (const auto &cue : cueList) {
        const auto &cueBox = cue->cueBox();
        const auto computedPosition = cueBox.computedPositionVwH;
        const auto ComputedPosition_px = m_converter.vwToWidthPixels(computedPosition);
        const auto computedLine = cueBox.lineVhH;
        const auto snapToLines = cueBox.snapToLines;
        const auto align = cueBox.cueTextAlign;
        const auto lineAlign = cueBox.cueLineAlign;
        const auto viewportHeight = m_converter.height();
        const auto region_width_px = m_converter.vwToWidthPixels(cueBox.computedSizeVwH);
        
        auto boxes = buildLines(cue->lines(), region_width_px);
        
        //The line setting is either a line number (+ve or -ve) or auto
        if (snapToLines) {
            linePositionsBeforeAdjustment(boxes, align, 0, ComputedPosition_px, true);
            auto step = m_converter.lineHeightPixels();
            auto line = computedLine;
            if (line == constants::kCueAutoSetting) {
                line = --autoLine;
            }
            auto position = step * line;
            if (line < 0) {
                position += viewportHeight;
                step *= -1;
            }
            //Adjust boxes
            for (auto &cueLine : boxes) {
                auto &lineRectangle = cueLine.lineRectangle;
                lineRectangle.m_y += position ;
                g_logger.osinfo(__LOGGER_FUNC__, " - line added rect:", cueLine.lineRectangle,
                                                " position:", position);
            }

            // If auto line numbering and the top positioning attribute is set,
            // move the top of the cue boxes.
            if ((!boxes.empty()) &&
                (computedLine == constants::kCueAutoSetting) &&
                m_converter.isTopPositioningSet())
            {
                int delta = m_converter.topPositioningPixels() - boxes.front().lineRectangle.m_y;
                for (auto &cueLine : boxes) {
                    auto &lineRectangle = cueLine.lineRectangle;
                    lineRectangle.m_y += delta;
                    g_logger.osinfo(__LOGGER_FUNC__, " - top positioning rect:", cueLine.lineRectangle);
                }
            }

            //Check display position
            bool switched = false;
            //TODO Check if any overlap with output
            while (needsAdjustment(boxes, viewportHeight)) {
                if (switchPosition(boxes, step, viewportHeight)) {
                    if (switched) {
                        //No chance - clear all the boxes and give up
                        boxes.clear();
                        break;
                    }
                    step *= -1;
                    switched = true;
                }
                for (auto &cueLine : boxes) {
                    cueLine.lineRectangle.m_y += step;
                }
            }
            outputLines.insert(outputLines.end(), boxes.begin(), boxes.end());
        } 
        //The line setting is a percentage of the drawing area
        else {
            const auto ComputedLine_px = m_converter.vhToHeightPixels(computedLine);
            auto total_boxes_height = boxes.size() * m_converter.lineHeightPixels();
            linePositionsBeforeAdjustment(boxes, align, ComputedLine_px, 
                                            ComputedPosition_px, true);
            switch(lineAlign) {
                case WebVTTCue::LineAlignType::kStart:
                    break;
                case WebVTTCue::LineAlignType::kCenter:
                    for (auto &cueLine : boxes) {
                        cueLine.lineRectangle.m_y -= total_boxes_height / 2;
                    }
                    break;
                case WebVTTCue::LineAlignType::kEnd:
                    for (auto &cueLine : boxes) {
                        cueLine.lineRectangle.m_y -= total_boxes_height;
                    }
                    break;
            }
            
            //Check if subs need adjustment
            while (needsAdjustment(boxes, viewportHeight)) {
                auto top = boxes.front().lineRectangle.m_y;
                auto bottom = boxes.back().lineRectangle.m_y + m_converter.lineHeightPixels();
                if (top < 0) {
                    for (auto &cueLine : boxes) {
                        cueLine.lineRectangle.m_y += m_converter.lineHeightPixels();
                    }
                } else if (bottom > viewportHeight) {
                    for (auto &cueLine : boxes) {
                        cueLine.lineRectangle.m_y -= m_converter.lineHeightPixels();
                    }
                }
            }
            outputLines.insert(outputLines.end(), boxes.begin(), boxes.end());
        }
    }
    
    for (auto &cueLine : outputLines) {
        auto &lineRectangle = cueLine.lineRectangle;
        lineRectangle.m_y += startingY;
        g_logger.osinfo(__LOGGER_FUNC__, " - line added rect:", cueLine.lineRectangle,
                                        " startingY:", startingY);
    }

    return outputLines;
}

/**
 * @brief Sort the cues by region (or no region) then calculate the metadata needed
 * for rendering:
 *  - size and position data
 *  - font tokens
 *  - font styles
 *  - calculate relative positions
 *  - region data
 * 
 * @param webvttCueList 
 * @param regionMap 
 * @return std::list<Line> 
 */
std::list<Line> LineBuilder::buildOutputLines(const CueSharedList& webvttCueList, 
                                                const RegionMap &regionMap) {
    std::list<Line> output;
    auto cuesByRegion = sortCuesByRegion(webvttCueList);
    
    for (const auto &obj : cuesByRegion) {
        const auto regionId = obj.first;
        const auto &cueList = obj.second;
        if (regionId != "m_Empty") {
            try {
                //Add region cues to output
                Region region = regionMap.at(regionId);
                auto lines = getRegionLines(cueList, region);
                output.insert(output.end(), lines.begin(), lines.end());
            } catch (const ParserException &e) {
                //Region wasn't found - display the cues anyway
                //Auto settings should make a reasonable go at positioning
                //in a sane manner
                auto lines = getOutputLines(cueList);
                output.insert(output.end(), lines.begin(), lines.end());
            } catch (const std::out_of_range &e) {
                g_logger.oswarning(__LOGGER_FUNC__, "Region ID ", regionId, " not found in map");
            }
        } else {
            //Add regular cues to output
            auto lines = getOutputLines(cueList);
            output.insert(output.end(), lines.begin(), lines.end());
        }
    }
    
    return output;
}

void LineBuilder::cleanUpState() {
    m_fontCache->clear();
}

std::string LineBuilder::getFontFamily(WebVTTConfig config)
{
    std::string fontName = config.fontFamily;
    if (m_attributes.isSet(WebVTTAttributes::AttributeType::FONT_STYLE))
    {
        WebVTTAttributes::FontStyle fontStyleAttribute = static_cast<WebVTTAttributes::FontStyle>(m_attributes.getInteger(WebVTTAttributes::AttributeType::FONT_STYLE));

        switch(fontStyleAttribute)
        {
            case WebVTTAttributes::FontStyle::CASUAL_FONT_TYPE:
                fontName = "Cinecav Casual";
                break;
            case WebVTTAttributes::FontStyle::CURSIVE_FONT_TYPE:
                fontName = "Cinecav Script";
                break;
            case WebVTTAttributes::FontStyle::MONOSPACED_WITHOUT_SERIFS:
                fontName = "Cinecav Mono";
                break;
            case WebVTTAttributes::FontStyle::MONOSPACED_WITH_SERIFS:
                fontName = "Cinecav Type";
                break;
            case WebVTTAttributes::FontStyle::PROPORTIONALLY_SPACED_WITHOUT_SERIFS:
                fontName = "Cinecav Sans";
                break;
            case WebVTTAttributes::FontStyle::PROPORTIONALLY_SPACED_WITH_SERIFS:
                fontName = "Cinecav Serif";
                break;
            case WebVTTAttributes::FontStyle::SMALL_CAPITALS:
                fontName = "Cinecav Smallcaps";
                break;
            case WebVTTAttributes::FontStyle::DEFAULT_OR_UNDEFINED:
                fontName = "Cinecav Mono";
                break;
        }
    }
    return fontName;
}

void LineBuilder::getUserDefinedColorAttributes(Style &style)
{
    gfx::ColorArgb colorArgb = style.textColour();
    WebVTTAttributes::Opacity opacity = WebVTTAttributes::Opacity::SOLID;

    if (m_attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR))
    {
        uint32_t fontColor = m_attributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR);

        colorArgb = gfx::ColorArgb(fontColor);
    }

    if (m_attributes.isSet(WebVTTAttributes::AttributeType::FONT_OPACITY))
    {
        opacity = static_cast<WebVTTAttributes::Opacity>(m_attributes.getInteger(WebVTTAttributes::AttributeType::FONT_OPACITY));
    }

    style.textColour(setOpacity(colorArgb, opacity));

    colorArgb = style.bgColour();

    if (m_attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_COLOR))
    {
        uint32_t backgroundColor = m_attributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR);

        colorArgb = gfx::ColorArgb(backgroundColor);
    }

    if (m_attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY))
    {
        opacity = static_cast<WebVTTAttributes::Opacity>(m_attributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY));
    }

    style.bgColour(setOpacity(colorArgb, opacity));

    colorArgb = style.edgeColour();

    if (m_attributes.isSet(WebVTTAttributes::AttributeType::EDGE_COLOR))
    {
        uint32_t edgeColor = m_attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_COLOR);

        colorArgb = gfx::ColorArgb(edgeColor);
    }

    opacity = WebVTTAttributes::Opacity::SOLID;

    style.edgeColour(setOpacity(colorArgb, opacity));
}

}
}
