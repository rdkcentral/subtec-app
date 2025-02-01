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

#include "ScteSimpleBitmap.hpp"
#include "ScteExceptions.hpp"
#include "ScteRawBitmapDecoder.hpp"

namespace subttxrend
{
namespace scte
{

SimpleBitmap::SimpleBitmap(uint8_t* data, std::size_t size):
    shadowRight(0), shadowBottom(0), shadowColor{0, 0, 0, 0}, outlineThickness(0),
    outlineColor{0, 0, 0, 0}, frameTop{0, 0}, frameBottom{0, 0}, frameColor{0, 0, 0, 0}
{
    fillFields(data, size);
}


SimpleBitmap::SimpleBitmap(const RawBitmap& rawBitmap)
{
    setBitmap(rawBitmap);
}


BackgroundStyle SimpleBitmap::getBackgroundStyle() const
{
    return backgroundStyle;
}


OutlineStyle SimpleBitmap::getOutlineStyle() const
{
    return outlineStyle;
}


Color SimpleBitmap::getCharacterColor() const
{
    return charColor;
}


Coords SimpleBitmap::getCharacterTop() const
{
    return charTop;
}


Coords SimpleBitmap::getCharacterBottom() const
{
    return charBottom;
}


Coords SimpleBitmap::getFrameTop() const
{
    return frameTop;
}


Coords SimpleBitmap::getFrameBottom() const
{
    return frameBottom;
}


Color SimpleBitmap::getFrameColor() const
{
    return frameColor;
}


uint8_t SimpleBitmap::getOutlineThickness() const
{
    return outlineThickness;
}


Color SimpleBitmap::getOutlineColor() const
{
    return outlineColor;
}


uint8_t SimpleBitmap::getShadowRight() const
{
    return shadowRight;
}


uint8_t SimpleBitmap::getShadowBottom() const
{
    return shadowBottom;
}


Color SimpleBitmap::getShadowColor() const
{
    return shadowColor;
}

uint16_t SimpleBitmap::width() const
{
    return getCharacterBottom().x - getCharacterTop().x;

}

uint16_t SimpleBitmap::height() const
{
    return getCharacterBottom().y - getCharacterTop().y;
}

void SimpleBitmap::decompress()
{
    if (bitmap.isCompressed())
    {
        RawBitmapDecoder decoder{bitmap, width(), height()};
        decoder.decompress();
    }
}


const RawBitmap &SimpleBitmap::getBitmap() const
{
    return bitmap;
}


void SimpleBitmap::setBitmap(const uint8_t* data, std::size_t size, bool compressed)
{
    fillFields(data, size);
    bitmap.setCompression(compressed);
}


void SimpleBitmap::setBitmap(const RawBitmap& rawBitmap)
{
    bitmap = rawBitmap;
}


void SimpleBitmap::fillFields(const uint8_t* data, std::size_t size)
{
    std::size_t bitmapLength = 0;
    int idx = 0;

    if (!data || !size)
    {
        throw InvalidArgument("invalid data ptr or size");
    }

    backgroundStyle = static_cast<BackgroundStyle>(data[0] & 0x04);
    outlineStyle = static_cast<OutlineStyle>(data[0] & 0x03);

    charColor.y = (data[1] & 0xF8) >> 3;
    charColor.opaqueEnabled = (data[1] & 0x04)? true : false;
    charColor.cr = ((data[1] & 0x03) << 3) | ((data[2] & 0xE0) >> 5);
    charColor.cb = data[2] & 0x1F;

    charTop.x = (data[3] << 4) | ((data[4] & 0xF0) >> 4);
    charTop.y = ((data[4] & 0x0F) << 8) | data[5];

    charBottom.x = (data[6] << 4) | ((data[7] & 0xF0) >> 4);
    charBottom.y = ((data[7] & 0x0F) << 8) | data[8];

    idx += 9;

    if(backgroundStyle == BackgroundStyle::FRAMED)
    {
        frameTop.x = (data[idx] << 4) | ((data[idx+1] & 0xF0) >> 4);
        frameTop.y = ((data[idx+1] & 0x0F) << 8) | data[idx+2];

        frameBottom.x = (data[idx+3] << 4) | ((data[idx+4] & 0xF0) >> 4);
        frameBottom.y = ((data[idx+4] & 0x0F) << 8) | data[idx+5];

        frameColor.y = (data[idx+6] & 0xF8) >> 3;
        frameColor.opaqueEnabled = (data[idx+6] & 0x04)? true : false;
        frameColor.cr = ((data[idx+6] & 0x03) << 3) | ((data[idx+7] & 0xE0) >> 5);
        frameColor.cb = data[idx+7] & 0x1F;

        idx += 8;
    }

    if(outlineStyle == OutlineStyle::OUTLINE)
    {
        outlineThickness = data[idx] & 0x0F;

        outlineColor.y = (data[idx+1] & 0xF8) >> 3;
        outlineColor.opaqueEnabled = (data[idx+1] & 0x04)? true : false;
        outlineColor.cr = ((data[idx+1] & 0x03) << 3) | ((data[idx+2] & 0xE0) >> 5);
        outlineColor.cb = data[idx+2] & 0x1F;

        idx += 3;
    }
    else if(outlineStyle == OutlineStyle::DROP_SHADOW)
    {
        shadowRight = (data[idx] & 0xF0) >> 4;
        shadowBottom = data[idx+1] & 0x0F;

        shadowColor.y = (data[idx+1] & 0xF8) >> 3;
        shadowColor.opaqueEnabled = (data[idx+1] & 0x04)? true : false;
        shadowColor.cr = ((data[idx+1] & 0x03) << 3) | ((data[idx+2] & 0xE0) >> 5);
        shadowColor.cb = data[idx+2] & 0x1F;

        idx += 3;
    }
    else if(outlineStyle == OutlineStyle::RESERVED)
    {
        idx += 3;
    }

    bitmapLength = (data[idx] << 8) | data[idx+1];
    idx += 2;
    if (bitmapLength + idx > size)
    {
        throw ParseError("bitmap size larger than total size");
    }
    bitmap.setRawData(data+idx, bitmapLength);
}

} //namespace scte
} //namespace subttxrend
