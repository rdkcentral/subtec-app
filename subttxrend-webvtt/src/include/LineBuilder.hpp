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

#include <WebVTTConverter.hpp>
#include <WebVTTCue.hpp>
#include <WebVTTStyle.hpp>
#include <WebVTTDocument.hpp>

#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/Types.hpp>

namespace subttxrend {
namespace webvttengine {
namespace linebuilder {

using FontCachePtr = std::unique_ptr<gfx::PrerenderedFontCache>;
using FontPtr = std::shared_ptr<gfx::PrerenderedFont>;
using TokenPtr = std::shared_ptr<gfx::TextTokenData>;

struct Token
{
    /**
     * Constructor.
     */
    Token() :
        width(0)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param aWidth
     *      Token width.
     * @param aStyle
     *      Token style.
     * @param aToken
     *      Token text.
     * @param aFont
     *      Token font.
     */
    Token(int aWidth, const Style &aStyle, const TokenPtr &aToken, const FontPtr &aFont) :
        width(aWidth),
        style(aStyle),
        token(aToken),
        font(aFont)
    {
        // noop
    }

    int         width;
    Style       style;
    TokenPtr    token;
    FontPtr     font;
};

struct Line {
    /**
     * Constructor.
     */
    Line() :
        lineWidth(0),
        linePaddingX(0)
    {
        // noop
    }

    std::vector<Token>  tokenVector;
    int                 lineWidth;
    int                 linePaddingX;
    gfx::Rectangle      lineRectangle;
};

struct Result {
    std::string text;
    Style       style;
};


using LineList = std::list<Line>;

/**
 * @brief WebVTT has fairly complex requirements for the positioning and rendering
 * of text cues.  This class will take a list of cues, a list of regions and a Converter
 * class with current window dimensions and produce a list of Line objects for passing to the renderer.
 * Each Line consists of:
 *  - a vector of Tokens containing font and style data used by the renderer
 *  - a lineRectangle giving the dimensions of the full line plus padding
 *  - linePaddingX and lineWidth - convenience parameters used in the renderer
 */
class LineBuilder {
public:

    LineBuilder(int viewportWidth, int viewportHeight) : 
                m_converter(viewportWidth, viewportHeight) {}

    LineBuilder(int viewportWidth, int viewportHeight, const WebVTTConfig &config, const WebVTTAttributes &attributes) :
                m_converter(viewportWidth, viewportHeight, config, attributes)
    {
        m_attributes.update(attributes);
        m_fontFamily = getFontFamily(config);
    }
    
    std::list<Line> buildOutputLines(const CueSharedList& webvtt_cue_list, const RegionMap &regionMap);
    void            cleanUpState();
    
private:
    LineList        getRegionLines(const CueSharedList &cueList, const Region &region);
    LineList        getOutputLines(const CueSharedList &cueList);
    void            linePositionsBeforeAdjustment(std::list<Line> &boxes, WebVTTCue::AlignType align, 
                                                  int startingY, int positionPx, bool setY);
    Line            buildTokensForLine(const std::vector<Result> &line_segments);
    LineList        buildLines(const std::vector<std::string> lineStrings, int maximumSize);
    FontPtr         getFont(std::string fontFamily, std::uint32_t size);
    std::string     getFontFamily(WebVTTConfig config);
    void            getUserDefinedColorAttributes(Style &style);

    Converter       m_converter {1920, 1080};
    FontCachePtr    m_fontCache {std::make_unique<gfx::PrerenderedFontCache>()};
    
    std::string     m_fontFamily {"cinecavD sans"};
    WebVTTAttributes    m_attributes;
};

}
}
}
