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

#include <iostream>
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <string>

namespace subttxrend
{
namespace gfx
{

static constexpr int FONT_WIDTH_PIXELS = 20;
static constexpr int FONT_HEIGHT_PIXELS = 40;

class GlyphData
{
    GlyphData() { std::cout << "I'm a glyph\n"; }
};

class TextTokenData
{
public:
    TextTokenData(const std::string &str = {}) : mStr(str), isWhite(false), totalAdvanceX(str.length()*FONT_WIDTH_PIXELS), glyphs() {}

    std::string mStr;
    bool isWhite;
    int totalAdvanceX;
    std::vector<GlyphData> glyphs;
};

class PrerenderedFont
{
public:
    PrerenderedFont()
    {
        std::cout << "PrerenderedFont constr\n";
    }

    int getFontHeight() { return FONT_HEIGHT_PIXELS; }

    std::vector<TextTokenData> textToTokens(std::string text)
    {
        std::istringstream iss(text);
        std::vector<TextTokenData> tokens;
        std::string token;
        TextTokenData ttd;

        while (iss >> token)
        {
            ttd = TextTokenData{token};
            tokens.push_back(ttd);
        }

        return tokens;
    }
};

class PrerenderedFontCache
{
public:
    std::shared_ptr<PrerenderedFont> getFont(const std::string & fontFamily, int height, int force = true)
    {
        std::cout << "Returning font " << fontFamily << " height " << height << "\n";
        return std::make_shared<PrerenderedFont>();
    }

    void clear()
    {
        std::cout << "Clearing the font cache\n";
    }
};
}
}