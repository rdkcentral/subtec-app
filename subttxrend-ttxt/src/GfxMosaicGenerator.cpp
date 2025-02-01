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


#include "GfxMosaicGenerator.hpp"

#include <array>

namespace subttxrend
{
namespace ttxt
{

namespace
{

class StdMosaicChar
{
public:
    static const std::size_t WIDTH = 12;
    static const std::size_t HEIGHT = 10;

    StdMosaicChar() = default;

    const std::uint8_t* getData() const
    {
        return m_pixels.data();
    }

    std::size_t getSize() const
    {
        return m_pixels.size();
    }

    void clear()
    {
        m_pixels.fill(0);
    }

    void print()
    {
        for (std::size_t y = 0; y < HEIGHT; ++y)
        {
            char lineChars[WIDTH + 1];

            for (std::size_t x = 0; x < WIDTH; ++x)
            {
                lineChars[x] = m_pixels[y * WIDTH + x] > 0 ? '#' : '.';
            }
            lineChars[WIDTH] = 0;

            printf("%s\n", lineChars);
            printf("%s\n", lineChars);
        }

        printf("\n");
    }

    void fillSegment(std::uint8_t segmentIndex,
                     bool separated)
    {
        if (separated)
        {
            fillSegmentSeparated(segmentIndex);
        }
        else
        {
            fillSegmentBlock(segmentIndex);
        }
    }

private:
    void fillSegmentBlock(std::uint8_t segmentIndex)
    {
        static const std::size_t SEGMENT_START_L = 0;
        static const std::size_t SEGMENT_START_R = 6;
        static const std::size_t SEGMENT_WIDTH = 6;

        static const std::size_t SEGMENT_START_TOP = 0;
        static const std::size_t SEGMENT_START_MID = 3;
        static const std::size_t SEGMENT_START_BOT = 7;
        static const std::size_t SEGMENT_HEIGHT_TOP = 3;
        static const std::size_t SEGMENT_HEIGHT_MID = 4;
        static const std::size_t SEGMENT_HEIGHT_BOT = 3;

        switch (segmentIndex)
        {
        case 0:
            fillRect(SEGMENT_START_L, SEGMENT_START_TOP, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_TOP);
            break;
        case 1:
            fillRect(SEGMENT_START_R, SEGMENT_START_TOP, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_TOP);
            break;
        case 2:
            fillRect(SEGMENT_START_L, SEGMENT_START_MID, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_MID);
            break;
        case 3:
            fillRect(SEGMENT_START_R, SEGMENT_START_MID, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_MID);
            break;
        case 4:
            fillRect(SEGMENT_START_L, SEGMENT_START_BOT, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_BOT);
            break;
        case 5:
            fillRect(SEGMENT_START_R, SEGMENT_START_BOT, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_BOT);
            break;
        default:
            break;
        }
    }

    void fillSegmentSeparated(std::uint8_t segmentIndex)
    {
        static const std::size_t SEGMENT_START_L = 1;
        static const std::size_t SEGMENT_START_R = 7;
        static const std::size_t SEGMENT_WIDTH = 4;

        static const std::size_t SEGMENT_START_TOP = 1;
        static const std::size_t SEGMENT_START_MID = 4;
        static const std::size_t SEGMENT_START_BOT = 7;
        static const std::size_t SEGMENT_HEIGHT_TOP = 2;
        static const std::size_t SEGMENT_HEIGHT_MID = 2;
        static const std::size_t SEGMENT_HEIGHT_BOT = 2;

        switch (segmentIndex)
        {
        case 0:
            fillRect(SEGMENT_START_L, SEGMENT_START_TOP, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_TOP);
            break;
        case 1:
            fillRect(SEGMENT_START_R, SEGMENT_START_TOP, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_TOP);
            break;
        case 2:
            fillRect(SEGMENT_START_L, SEGMENT_START_MID, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_MID);
            break;
        case 3:
            fillRect(SEGMENT_START_R, SEGMENT_START_MID, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_MID);
            break;
        case 4:
            fillRect(SEGMENT_START_L, SEGMENT_START_BOT, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_BOT);
            break;
        case 5:
            fillRect(SEGMENT_START_R, SEGMENT_START_BOT, SEGMENT_WIDTH,
                    SEGMENT_HEIGHT_BOT);
            break;
        default:
            break;
        }
    }

    void fillRect(std::size_t offsetX,
                  std::size_t offsetY,
                  std::size_t sizeX,
                  std::size_t sizeY)
    {
        for (std::size_t y = 0; y < sizeY; ++y)
        {
            for (std::size_t x = 0; x < sizeX; ++x)
            {
                m_pixels[(offsetY + y) * WIDTH + (offsetX + x)] = 0xFF;
            }
        }
    }

    std::array<std::uint8_t, WIDTH * HEIGHT> m_pixels;
};

void drawCharacter(StdMosaicChar& mosaicChar,
                   int index,
                   bool separated)
{
    mosaicChar.clear();

    for (int bit = 0; bit < 6; ++bit)
    {
        bool segmentSet = (index & (1 << bit));
        if (segmentSet)
        {
            mosaicChar.fillSegment(bit, separated);
        }
    }
}

void writeGlyph(const gfx::FontStripPtr& fontStrip,
                std::uint16_t glyphIndex,
                const StdMosaicChar& mosaicChar)
{
    fontStrip->loadGlyph(glyphIndex, mosaicChar.getData(),
            mosaicChar.getSize());
}

}

std::pair<gfx::FontStripPtr, gfx::FontStripMap> GfxMosaicGenerator::generateStripG1(const ttxdecoder::Engine& ttxEngine,
                                                                                    const gfx::EnginePtr& gfxEngine)
{
    const std::size_t SET_CHARACTER_COUNT = 64;
    const std::size_t SETS_COUNT = 2;

    auto fontStrip = gfxEngine->createFontStrip(
            gfx::Size(StdMosaicChar::WIDTH, StdMosaicChar::HEIGHT),
            SETS_COUNT * SET_CHARACTER_COUNT);
    gfx::FontStripMap fontStripMapping;

    auto& blockMapping = ttxEngine.getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC);
    auto& separatedMapping = ttxEngine.getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED);

    // clear the mapping
    fontStripMapping.clear();

    StdMosaicChar mosaicChar;

    // draw block solid mosaic
    std::uint16_t glyphIndex = 0;
    for (std::size_t characterNumber = 0; characterNumber < SET_CHARACTER_COUNT;
            ++characterNumber)
    {
        drawCharacter(mosaicChar, characterNumber, false);

        auto charsetIndex = characterNumber;
        if (charsetIndex < 0x20)
        {
            charsetIndex += 0x00;
        }
        else
        {
            charsetIndex += 0x20;
        }

        auto characterCode = blockMapping[charsetIndex];

        // store data
        fontStripMapping.addMapping(characterCode, glyphIndex);
        writeGlyph(fontStrip, glyphIndex, mosaicChar);

        ++glyphIndex;
    }

    // draw block separated mosaic
    for (std::size_t characterNumber = 0; characterNumber < SET_CHARACTER_COUNT;
            ++characterNumber)
    {
        drawCharacter(mosaicChar, characterNumber, true);

        auto charsetIndex = characterNumber;
        if (charsetIndex < 2 * 16)
        {
            charsetIndex += 0x00;
        }
        else
        {
            charsetIndex += 0x20;
        }

        auto characterCode = separatedMapping[charsetIndex];

        // store data
        fontStripMapping.addMapping(characterCode, glyphIndex);
        writeGlyph(fontStrip, glyphIndex, mosaicChar);

        ++glyphIndex;
    }

    return
    {   fontStrip, fontStripMapping};
}

} // namespace ttxt
} // namespace subttxrend
