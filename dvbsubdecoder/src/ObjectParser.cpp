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


#include "ObjectParser.hpp"

#include <subttxrend/common/Logger.hpp>

#include "PesPacketReader.hpp"
#include "PixelWriter.hpp"
#include "ParserException.hpp"
#include "BitStream.hpp"

// #define VERBOSE_LOGGING

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ObjectParser");

const std::uint8_t DATA_TYPE_2BIT_PIXEL_CODE_STRING = 0x10;
const std::uint8_t DATA_TYPE_4BIT_PIXEL_CODE_STRING = 0x11;
const std::uint8_t DATA_TYPE_8BIT_PIXEL_CODE_STRING = 0x12;
const std::uint8_t DATA_TYPE_2_TO_4_BIT_MAP_TABLE_DATA = 0x20;
const std::uint8_t DATA_TYPE_2_TO_8_BIT_MAP_TABLE_DATA = 0x21;
const std::uint8_t DATA_TYPE_4_TO_8_BIT_MAP_TABLE_DATA = 0x22;
const std::uint8_t DATA_TYPE_END_OF_OBJECT_LINE_CODE = 0xF0;

} // namespace <anonmymous>

ObjectParser::ObjectParser(PesPacketReader& reader,
                           PixelWriter& writer) :
        m_reader(reader),
        m_writer(writer),
        m_currentMap(nullptr)
{
    m_mapTable2to4[0] = 0x00;
    m_mapTable2to4[1] = 0x07;
    m_mapTable2to4[2] = 0x08;
    m_mapTable2to4[3] = 0x0F;

    m_mapTable2to8[0] = 0x00;
    m_mapTable2to8[1] = 0x77;
    m_mapTable2to8[2] = 0x88;
    m_mapTable2to8[3] = 0xFF;

    for (std::uint8_t i = 0; i < 16; ++i)
    {
        m_mapTable4to8[i] = (i << 4) | i;
    }
}

void ObjectParser::parse()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    while (m_reader.getBytesLeft() > 0)
    {
        const auto dataType = m_reader.readUint8();

#ifdef VERBOSE_LOGGING
        g_logger.trace("%s - type=%02X", __func__, dataType);
#endif

        switch (dataType)
        {
        case DATA_TYPE_2BIT_PIXEL_CODE_STRING:
            parse2bitPixelCodeString();
            break;

        case DATA_TYPE_4BIT_PIXEL_CODE_STRING:
            parse4bitPixelCodeString();
            break;

        case DATA_TYPE_8BIT_PIXEL_CODE_STRING:
            parse8bitPixelCodeString();
            break;

        case DATA_TYPE_2_TO_4_BIT_MAP_TABLE_DATA:
            parseMapTable2to4();
            break;

        case DATA_TYPE_2_TO_8_BIT_MAP_TABLE_DATA:
            parseMapTable2to8();
            break;

        case DATA_TYPE_4_TO_8_BIT_MAP_TABLE_DATA:
            parseMapTable4to8();
            break;

        case DATA_TYPE_END_OF_OBJECT_LINE_CODE:
            m_writer.endLine();
            break;

        default:
            throw ParserException("Invalid data type");
        }
    }

#ifdef VERBOSE_LOGGING
        g_logger.trace("%s - finished", __func__);
#endif
}

void ObjectParser::parse2bitPixelCodeString()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    switch (m_writer.getDepth())
    {
    case 2:
        // nothing to do 2->2
        m_currentMap = nullptr;
        break;
    case 4:
        m_currentMap = m_mapTable2to4.data();
        break;
    case 8:
        m_currentMap = m_mapTable2to8.data();
        break;
    default:
        throw ParserException("Invalid pixel writer depth");
    }

    BitStream bitStream(m_reader);

    for (;;)
    {
        auto nextBits = bitStream.read<2>();
        if (nextBits != 0)
        {
            // 01 - one pixel in colour 1
            // 10 - one pixel in colour 2
            // 11 - one pixel in colour 3
            auto pixelCode = nextBits;
            setPixels(pixelCode, 1);
        }
        else
        {
            // 00 x
            auto switch1 = bitStream.read<1>();

            if (switch1 == 1)
            {
                // 00 1L LL CC - L pixels (3..10) in colour C
                auto runLength3_10 = bitStream.read<3>();
                auto pixelCode = bitStream.read<2>();
                setPixels(pixelCode, 3 + runLength3_10);
            }
            else
            {
                // 00 0x
                auto switch2 = bitStream.read<1>();
                if (switch2 == 0)
                {
                    // 00 00 xx
                    auto switch3 = bitStream.read<2>();
                    if (switch3 == 0)
                    {
                        // 00 00 00 - end of 2-bit/pixel_code_string
                        break;
                    }
                    else if (switch3 == 1)
                    {
                        // 00 00 01 - two pixels in colour 0
                        setPixels(0, 2);
                    }
                    else if (switch3 == 2)
                    {
                        // 00 00 10 LL LL CC - L pixels (12..27) in colour C
                        auto runLength12_27 = bitStream.read<4>();
                        auto pixelCode = bitStream.read<2>();
                        setPixels(pixelCode, runLength12_27 + 12);
                    }
                    else // if (switch3 == 3)
                    {
                        // 00 00 11 LL LL LL LL CC - L pixels (29..284) in colour C
                        auto runLength29_284 = bitStream.read<8>();
                        auto pixelCode = bitStream.read<2>();
                        setPixels(pixelCode, runLength29_284 + 29);
                    }
                }
                else
                {
                    // 00 01 - one pixel in colour 0
                    setPixels(0, 1);
                }
            }
        }
    }
}

void ObjectParser::parse4bitPixelCodeString()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    switch (m_writer.getDepth())
    {
    case 2:
        throw ParserException("Object of depth 4, region of depth 2");
    case 4:
        m_currentMap = nullptr;
        break;
    case 8:
        m_currentMap = m_mapTable4to8.data();
        break;
    default:
        throw ParserException("Invalid pixel writer depth");
    }

    BitStream bitStream(m_reader);

    for (;;)
    {
        auto nextBits1 = bitStream.read<4>();
        if (nextBits1 != 0)
        {
            // 0001 - one pixel in colour 1
            // to to
            // 1111 - one pixel in colour 15
            auto pixelCode = nextBits1;
            setPixels(pixelCode, 1);
        }
        else
        {
            // 0000 x
            auto switch1 = bitStream.read<1>();
            if (switch1 == 0)
            {
                // 0000 0xxx
                auto nextBits2 = bitStream.read<3>();
                if (nextBits2 != 0)
                {
                    // 0000 0LLL - L pixels (3..9) in colour 0 (L>0)
                    auto runLength3_9 = nextBits2;
                    setPixels(0, runLength3_9 + 2);
                }
                else
                {
                    // 0000 0000 - end of 4-bit/pixel_code_string
                    break;
                }
            }
            else
            {
                // 0000 1x
                auto switch2 = bitStream.read<1>();
                if (switch2 == 0)
                {
                    // 0000 10LL CCCC - L pixels (4..7) in colour C
                    auto runLength4_7 = bitStream.read<2>();
                    auto pixelCode = bitStream.read<4>();
                    setPixels(pixelCode, runLength4_7 + 4);
                }
                else
                {
                    // 0000 11xx
                    auto switch3 = bitStream.read<2>();
                    if (switch3 == 0)
                    {
                        // 0000 1100 one pixel in colour 0
                        setPixels(0, 1);
                    }
                    else if (switch3 == 1)
                    {
                        // 0000 1101 two pixels in colour 0
                        setPixels(0, 2);
                    }
                    else if (switch3 == 2)
                    {
                        // 0000 1110 LLLL CCCC - L pixels (9..24) in colour C
                        auto runLength9_24 = bitStream.read<4>();
                        auto pixelCode = bitStream.read<4>();
                        setPixels(pixelCode, runLength9_24 + 9);
                    }
                    else // if (switch3 == 3)
                    {
                        // 0000 1111 LLLL LLLL CCCC - L pixels (25..280) in colour C
                        auto runLength25_280 = bitStream.read<8>();
                        auto pixelCode = bitStream.read<4>();
                        setPixels(pixelCode, runLength25_280 + 25);
                    }
                }
            }
        }
    }
}

void ObjectParser::parse8bitPixelCodeString()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    switch (m_writer.getDepth())
    {
    case 2:
        throw ParserException("Object of depth 8, region of depth 2");
    case 4:
        throw ParserException("Object of depth 8, region of depth 4");
    case 8:
        m_currentMap = nullptr;
        break;
    default:
        throw ParserException("Invalid pixel writer depth");
    }

    BitStream bitStream(m_reader);

    for (;;)
    {
        auto nextBits1 = bitStream.read<8>();
        if (nextBits1 != 0)
        {
            // 00000001 one pixel in colour 1
            // to to
            // 11111111 one pixel in colour 255
            auto pixelCode = nextBits1;
            setPixels(pixelCode, 1);
        }
        else
        {
            // 00000000 x
            auto switch1 = bitStream.read<1>();
            if (switch1 == 0)
            {
                // 00000000 0xxxxxxx
                auto nextBits2 = bitStream.read<7>();
                if (nextBits2 != 0)
                {
                    // 00000000 0LLLLLLL - L pixels (1-127) in colour 0 (L > 0)
                    auto runLength1_127 = nextBits2;
                    setPixels(0, runLength1_127);
                }
                else
                {
                    // 00000000 00000000 end of 8-bit/pixel_code_string
                    break;
                }
            }
            else
            {
                // 00000000 1LLLLLLL CCCCCCCC L pixels (3-127) in colour C (L > 2)
                auto runLength3_127 = bitStream.read<7>();
                auto pixelCode = bitStream.read<8>();
                setPixels(pixelCode, runLength3_127);
            }
        }
    }
}

void ObjectParser::parseMapTable2to4()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    BitStream bitStream(m_reader);

    for (std::size_t i = 0; i < m_mapTable2to4.size(); ++i)
    {
        m_mapTable2to4[i] = bitStream.read<4>();

#ifdef VERBOSE_LOGGING
        g_logger.trace("%s - idx=%02X entry=%u", __func__, static_cast<int>(i),
                m_mapTable2to4[i]);
#endif
    }
}

void ObjectParser::parseMapTable2to8()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    BitStream bitStream(m_reader);

    for (std::size_t i = 0; i < m_mapTable2to8.size(); ++i)
    {
        m_mapTable2to8[i] = bitStream.read<8>();
    }
}

void ObjectParser::parseMapTable4to8()
{
#ifdef VERBOSE_LOGGING
        g_logger.trace("%s", __func__);
#endif

    BitStream bitStream(m_reader);

    for (std::size_t i = 0; i < m_mapTable4to8.size(); ++i)
    {
        m_mapTable4to8[i] = bitStream.read<8>();
    }
}

void ObjectParser::setPixels(std::uint8_t pixelCode,
                             std::uint32_t count)
{
#ifdef VERBOSE_LOGGING
    g_logger.trace("%s - code=%02X count=%u", __func__, pixelCode, count);
#endif

    if (m_currentMap)
    {
        pixelCode = m_currentMap[pixelCode];
    }

    m_writer.setPixels(pixelCode, count);
}

} // namespace dvbsubdecoder
