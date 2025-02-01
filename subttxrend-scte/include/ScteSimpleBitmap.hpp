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

#pragma once


#include <cstdint>
#include <cstring>

#include "ScteRawBitmap.hpp"


namespace subttxrend
{
namespace scte
{

enum class BackgroundStyle
{
    TRANSPARENT,
    FRAMED
};

enum class OutlineStyle
{
    NONE,
    OUTLINE,
    DROP_SHADOW,
    RESERVED
};

enum PaletteColor
{
    COLOR_TRANSPARENT,
    COLOR_CHARACTER,
    COLOR_FRAME,
    COLOR_OUTLINE,
    COLOR_SHADOW,
    COLOR_LAST,
};

struct Color
{
    uint8_t y;
    bool    opaqueEnabled;
    uint8_t cr;
    uint8_t cb;
};

struct Coords
{
    uint16_t x;
    uint16_t y;
};

class SimpleBitmap
{
public:
    SimpleBitmap() = default;
    SimpleBitmap(uint8_t* data, std::size_t size);
    SimpleBitmap(const RawBitmap& rawBitmap);
    ~SimpleBitmap() = default;

    BackgroundStyle getBackgroundStyle() const;
    OutlineStyle getOutlineStyle() const;
    Color getCharacterColor() const;
    Coords getCharacterTop() const;
    Coords getCharacterBottom() const;

    Coords getFrameTop() const;
    Coords getFrameBottom() const;
    Color getFrameColor() const;

    uint8_t getOutlineThickness() const;
    Color getOutlineColor() const;

    uint8_t getShadowRight() const;
    uint8_t getShadowBottom() const;
    Color getShadowColor() const;

    void decompress();
    uint16_t width() const;
    uint16_t height() const;

    const RawBitmap& getBitmap() const;
    void setBitmap(const uint8_t *data, std::size_t size, bool compressed = true);
    void setBitmap(const RawBitmap& bitStream);

private:
    void fillFields(const uint8_t *data, std::size_t size);

    BackgroundStyle     backgroundStyle;
    OutlineStyle        outlineStyle;
    Color               charColor;
    Coords              charTop;
    Coords              charBottom;

    // Valid if background style is "framed"
    Coords              frameTop;
    Coords              frameBottom;
    Color               frameColor;

    // Valid if outline style is "outline"
    uint8_t             outlineThickness;
    Color               outlineColor;

    // Valid if outline style is "drop shadow"
    uint8_t             shadowRight;
    uint8_t             shadowBottom;
    Color               shadowColor;

    RawBitmap           bitmap;
};

} // namespace scte
} // namespace subttxrend
