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


#include <cppunit/extensions/HelperMacros.h>

#include <subttxrend/common/Logger.hpp>

#include "ObjectParser.hpp"
#include "Pixmap.hpp"
#include "PixelWriter.hpp"
#include "PesPacketReader.hpp"
#include "ParserException.hpp"

#include "BitStreamWriter.hpp"
#include "PixelStringWriter.hpp"

using dvbsubdecoder::ObjectParser;
using dvbsubdecoder::Pixmap;
using dvbsubdecoder::PixelWriter;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::ParserException;

namespace
{

subttxrend::common::Logger g_logger("Test", "PixelWriterTest");

}

class PixelWriterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PixelWriterTest );
    CPPUNIT_TEST(testBitStreamWriter);
    CPPUNIT_TEST(testBadStream);
    CPPUNIT_TEST(testWrongDepth);
    CPPUNIT_TEST(testMap2to4);
    CPPUNIT_TEST(testMap2to8);
    CPPUNIT_TEST(testMap4to8);
    CPPUNIT_TEST(testSingleString2);
    CPPUNIT_TEST(testSingleString4);
    CPPUNIT_TEST(testSingleString8);
    CPPUNIT_TEST(testDoubleString2);
    CPPUNIT_TEST(testDoubleString4);
    CPPUNIT_TEST(testDoubleString8);
    CPPUNIT_TEST(testMultiline);
    CPPUNIT_TEST(testConstructorValidation);
    CPPUNIT_TEST(testEmptyDataStream);
    CPPUNIT_TEST(testInvalidDataType);
    CPPUNIT_TEST(testBoundaryDataTypes);
    CPPUNIT_TEST(testIncompletePixelString);
    CPPUNIT_TEST(testCorruptedMapTable);
    CPPUNIT_TEST(testMaxRunLengthBoundaries);
    CPPUNIT_TEST(testZeroRunLength);
    CPPUNIT_TEST(testDepthMismatchValidation);
    CPPUNIT_TEST(testMixedDataTypes);
    CPPUNIT_TEST(testConsecutiveEndOfLine);
    CPPUNIT_TEST(testLargePixelCount);
    CPPUNIT_TEST(testMapTableOverflow);
    CPPUNIT_TEST(testParseMultipleObjects);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_pixmap.init(WIDTH, HEIGHT, m_pixmapBuffer.data());
        m_pixmap.clear(0);
    }

    void tearDown()
    {
        m_pixmap.reset();
    }

    void testBitStreamWriter()
    {
        // simple test to verify bit stream writer implementation

        BitStreamWriter writer;

        writer.write(0x0F, 4);
        writer.write(0x00, 1);
        writer.write(0x02, 3);

        writer.write(0x7E, 8);

        writer.write(0x00, 7);
        writer.write(0x03, 2);
        writer.write(0x00, 7);

        writer.write(0x04, 4);

        CPPUNIT_ASSERT_EQUAL(0xF2, static_cast<int>(writer.data()[0]));
        CPPUNIT_ASSERT_EQUAL(0x7E, static_cast<int>(writer.data()[1]));
        CPPUNIT_ASSERT_EQUAL(0x01, static_cast<int>(writer.data()[2]));
        CPPUNIT_ASSERT_EQUAL(0x80, static_cast<int>(writer.data()[3]));
        CPPUNIT_ASSERT_EQUAL(0x40, static_cast<int>(writer.data()[4]));
    }

    void testBadStream()
    {
        {
            PixelStringWriter dataStream2bitWriter;
            dataStream2bitWriter.start2bitPixelCodeString();
            dataStream2bitWriter.write2bitPixels(1, 3);
            // no end

            PesPacketReader reader(dataStream2bitWriter.data(),
                    dataStream2bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }

        {
            BitStreamWriter brokenStreamWriter;
            brokenStreamWriter.write(0x10, 8); // 2 bit string
            brokenStreamWriter.write(0x00, 2);
            brokenStreamWriter.write(0x01, 2); // one pixel in colour 0
            brokenStreamWriter.write(0x00, 2);
            brokenStreamWriter.write(0x00, 2); // two pixels in colour 0 (last bits missing)

            // -2 bytes to make stream incomplete
            PesPacketReader reader(brokenStreamWriter.data(),
                    brokenStreamWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }

        {
            BitStreamWriter badStreamWriter;
            badStreamWriter.write(0xFF, 8);

            PesPacketReader reader(badStreamWriter.data(),
                    badStreamWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }
    }

    void testWrongDepth()
    {
        PixelStringWriter dataStream2bitWriter;
        dataStream2bitWriter.start2bitPixelCodeString();
        dataStream2bitWriter.write2bitPixels(1, 1);
        dataStream2bitWriter.end2bitPixelCodeString();

        PixelStringWriter dataStream4bitWriter;
        dataStream4bitWriter.start4bitPixelCodeString();
        dataStream4bitWriter.write4bitPixels(1, 1);
        dataStream4bitWriter.end4bitPixelCodeString();

        PixelStringWriter dataStream8bitWriter;
        dataStream8bitWriter.start8bitPixelCodeString();
        dataStream8bitWriter.write8bitPixels(1, 1);
        dataStream8bitWriter.end8bitPixelCodeString();

        // check 4 bit on 2 bit
        {
            // parse
            PesPacketReader reader(dataStream4bitWriter.data(),
                    dataStream4bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }

        // check 8 bit on 2 bit
        {
            // parse
            PesPacketReader reader(dataStream8bitWriter.data(),
                    dataStream8bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }

        // check 8 bit on 4 bit
        {
            // parse
            PesPacketReader reader(dataStream8bitWriter.data(),
                    dataStream8bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 4, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }

        // check 2 bit on 3 bit
        {
            // parse
            PesPacketReader reader(dataStream2bitWriter.data(),
                    dataStream2bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 3, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }

        // check 4 bit on 3 bit
        {
            // parse
            PesPacketReader reader(dataStream4bitWriter.data(),
                    dataStream4bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 3, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }

        // check 8 bit on 3 bit
        {
            // parse
            PesPacketReader reader(dataStream8bitWriter.data(),
                    dataStream8bitWriter.size(), nullptr, 0);
            PixelWriter writer(false, 3, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }
    }

    void testMap2to4()
    {
        PixelStringWriter dataStream2bitWriter;

        // one pixel for each color
        dataStream2bitWriter.start2bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2bitPixels(colorIndex, 1);
        }
        dataStream2bitWriter.end2bitPixelCodeString();

        // map 2->4
        dataStream2bitWriter.start2to4bitMap();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2to4bitMapEntry(0x0F - colorIndex);
        }
        dataStream2bitWriter.end2to4bitMap();

        // one pixel for each color
        dataStream2bitWriter.start2bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2bitPixels(colorIndex, 1);
        }
        dataStream2bitWriter.end2bitPixelCodeString();

        // prepare pixmap
        m_pixmap.clear(0xFF);

        // parse
        PesPacketReader reader(dataStream2bitWriter.data(),
                dataStream2bitWriter.size(), nullptr, 0);
        PixelWriter writer(false, 4, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());

        // verify
        auto line = m_pixmap.getLine(0);
        CPPUNIT_ASSERT_EQUAL(0x00, static_cast<int>(line[0]));
        CPPUNIT_ASSERT_EQUAL(0x07, static_cast<int>(line[1]));
        CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[2]));
        CPPUNIT_ASSERT_EQUAL(0x0F, static_cast<int>(line[3]));

        CPPUNIT_ASSERT_EQUAL(0x0F, static_cast<int>(line[4]));
        CPPUNIT_ASSERT_EQUAL(0x0E, static_cast<int>(line[5]));
        CPPUNIT_ASSERT_EQUAL(0x0D, static_cast<int>(line[6]));
        CPPUNIT_ASSERT_EQUAL(0x0C, static_cast<int>(line[7]));

        CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[8]));
    }

    void testMap2to8()
    {
        PixelStringWriter dataStream2bitWriter;

        // one pixel for each color
        dataStream2bitWriter.start2bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2bitPixels(colorIndex, 1);
        }
        dataStream2bitWriter.end2bitPixelCodeString();

        // map 2->4
        dataStream2bitWriter.start2to8bitMap();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2to8bitMapEntry(0x80 + colorIndex);
        }
        dataStream2bitWriter.end2to8bitMap();

        // one pixel for each color
        dataStream2bitWriter.start2bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            dataStream2bitWriter.write2bitPixels(colorIndex, 1);
        }
        dataStream2bitWriter.end2bitPixelCodeString();

        // prepare pixmap
        m_pixmap.clear(0xFF);

        // parse
        PesPacketReader reader(dataStream2bitWriter.data(),
                dataStream2bitWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());

        // verify
        auto line = m_pixmap.getLine(0);
        CPPUNIT_ASSERT_EQUAL(0x00, static_cast<int>(line[0]));
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(line[1]));
        CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(line[2]));
        CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[3]));

        CPPUNIT_ASSERT_EQUAL(0x80, static_cast<int>(line[4]));
        CPPUNIT_ASSERT_EQUAL(0x81, static_cast<int>(line[5]));
        CPPUNIT_ASSERT_EQUAL(0x82, static_cast<int>(line[6]));
        CPPUNIT_ASSERT_EQUAL(0x83, static_cast<int>(line[7]));

        CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[8]));
    }

    void testMap4to8()
    {
        PixelStringWriter dataStream4bitWriter;

        // one pixel for each color
        dataStream4bitWriter.start4bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            dataStream4bitWriter.write4bitPixels(colorIndex, 1);
        }
        dataStream4bitWriter.end4bitPixelCodeString();

        // map 2->4
        dataStream4bitWriter.start4to8bitMap();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            dataStream4bitWriter.write4to8bitMapEntry(0x80 + colorIndex);
        }
        dataStream4bitWriter.end4to8bitMap();

        // one pixel for each color
        dataStream4bitWriter.start4bitPixelCodeString();
        for (std::uint8_t colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            dataStream4bitWriter.write4bitPixels(colorIndex, 1);
        }
        dataStream4bitWriter.end4bitPixelCodeString();

        // prepare pixmap
        m_pixmap.clear(0xFF);

        // parse
        PesPacketReader reader(dataStream4bitWriter.data(),
                dataStream4bitWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());

        // verify
        auto line = m_pixmap.getLine(0);
        for (auto i = 0; i < (1 << 4); ++i)
        {
            auto colorIndex = (i << 4) | i;
            CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
        }
        for (auto i = 0; i < (1 << 4); ++i)
        {
            auto colorIndex = 0x80 + i;
            CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[16 + i]));
        }

        CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[32]));
    }

    void testSingleString2()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        for (auto colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            for (auto runLength = 0; runLength <= 284; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream2bitWriter;

                dataStream2bitWriter.start2bitPixelCodeString();
                dataStream2bitWriter.write2bitPixels(colorIndex, runLength);
                dataStream2bitWriter.end2bitPixelCodeString();

                // prepare pixmap
                m_pixmap.clear(0xFF);

                // parse
                PesPacketReader reader(dataStream2bitWriter.data(),
                        dataStream2bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 2, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testSingleString4()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        for (auto colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            for (auto runLength = 0; runLength <= 280; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream4bitWriter;

                dataStream4bitWriter.start4bitPixelCodeString();
                dataStream4bitWriter.write4bitPixels(colorIndex, runLength);
                dataStream4bitWriter.end4bitPixelCodeString();

                // prepare pixmap
                m_pixmap.clear(0xFF);

                // parse
                PesPacketReader reader(dataStream4bitWriter.data(),
                        dataStream4bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 4, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testSingleString8()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        for (auto colorIndex = 0; colorIndex < (1 << 8); colorIndex += 15)
        {
            for (auto runLength = 0; runLength <= 127; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream8bitWriter;

                dataStream8bitWriter.start8bitPixelCodeString();
                dataStream8bitWriter.write8bitPixels(colorIndex, runLength);
                dataStream8bitWriter.end8bitPixelCodeString();

                // prepare pixmap
                auto clearColor = (colorIndex != 0xFF) ? 0xFF : 0x7F;
                m_pixmap.clear(clearColor);

                // parse
                PesPacketReader reader(dataStream8bitWriter.data(),
                        dataStream8bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 8, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(clearColor, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(clearColor, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testDoubleString2()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        auto colorMask = (1 << 2) - 1;
        for (auto colorIndex = 0; colorIndex < (1 << 2); ++colorIndex)
        {
            auto nextColor = (colorIndex + 1) & colorMask;
            for (auto runLength = 0; runLength <= 284; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream2bitWriter;

                dataStream2bitWriter.start2bitPixelCodeString();
                dataStream2bitWriter.write2bitPixels(colorIndex, runLength);
                dataStream2bitWriter.write2bitPixels(nextColor, runLength / 2);
                dataStream2bitWriter.end2bitPixelCodeString();

                // prepare pixmap
                m_pixmap.clear(0xFF);

                // parse
                PesPacketReader reader(dataStream2bitWriter.data(),
                        dataStream2bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 2, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < runLength + (runLength / 2); ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(nextColor, static_cast<int>(line[i]));
                }
                for (auto i = runLength + (runLength / 2); i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testDoubleString4()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        auto colorMask = (1 << 4) - 1;
        for (auto colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            auto nextColor = (colorIndex + 1) & colorMask;
            for (auto runLength = 0; runLength <= 280; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream4bitWriter;

                dataStream4bitWriter.start4bitPixelCodeString();
                dataStream4bitWriter.write4bitPixels(colorIndex, runLength);
                dataStream4bitWriter.write4bitPixels(nextColor, runLength / 2);
                dataStream4bitWriter.end4bitPixelCodeString();

                // prepare pixmap
                m_pixmap.clear(0xFF);

                // parse
                PesPacketReader reader(dataStream4bitWriter.data(),
                        dataStream4bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 4, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < runLength + (runLength / 2); ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(nextColor, static_cast<int>(line[i]));
                }
                for (auto i = runLength + (runLength / 2); i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testDoubleString8()
    {
        // limit the pixmap
        m_pixmap.init(WIDTH, 2, m_pixmapBuffer.data());

        auto colorMask = (1 << 4) - 1;
        for (auto colorIndex = 0; colorIndex < (1 << 4); ++colorIndex)
        {
            auto nextColor = (colorIndex + 1) & colorMask;
            auto clearColor = (nextColor + 1) & colorMask;
            for (auto runLength = 0; runLength <= 127; ++runLength)
            {
                g_logger.trace("%s - color=%d len=%d", __func__, colorIndex,
                        runLength);

                PixelStringWriter dataStream8bitWriter;

                dataStream8bitWriter.start8bitPixelCodeString();
                dataStream8bitWriter.write8bitPixels(colorIndex, runLength);
                dataStream8bitWriter.write8bitPixels(nextColor, runLength / 2);
                dataStream8bitWriter.end8bitPixelCodeString();

                // prepare pixmap
                m_pixmap.clear(clearColor);

                // parse
                PesPacketReader reader(dataStream8bitWriter.data(),
                        dataStream8bitWriter.size(), nullptr, 0);
                PixelWriter writer(false, 8, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);

                CPPUNIT_ASSERT_NO_THROW(parser.parse());

                // verify
                for (auto i = 0; i < runLength; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(colorIndex, static_cast<int>(line[i]));
                }
                for (auto i = runLength; i < runLength + (runLength / 2); ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(nextColor, static_cast<int>(line[i]));
                }
                for (auto i = runLength + (runLength / 2); i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(0);
                    CPPUNIT_ASSERT_EQUAL(clearColor, static_cast<int>(line[i]));
                }
                for (auto i = 0; i < WIDTH; ++i)
                {
                    auto line = m_pixmap.getLine(1);
                    CPPUNIT_ASSERT_EQUAL(clearColor, static_cast<int>(line[i]));
                }
            }
        }
    }

    void testMultiline()
    {
        PixelStringWriter dataStreamWriter;

        dataStreamWriter.start2bitPixelCodeString();
        dataStreamWriter.write2bitPixels(2, 2);
        dataStreamWriter.end2bitPixelCodeString();

        dataStreamWriter.writeEndOfLine();

        dataStreamWriter.start4bitPixelCodeString();
        dataStreamWriter.write4bitPixels(4, 4);
        dataStreamWriter.end4bitPixelCodeString();

        dataStreamWriter.writeEndOfLine();

        dataStreamWriter.start8bitPixelCodeString();
        dataStreamWriter.write8bitPixels(8, 8);
        dataStreamWriter.end8bitPixelCodeString();

        dataStreamWriter.writeEndOfLine();

        // clear pixmap
        m_pixmap.clear(0x99);

        // parse
        PesPacketReader reader(dataStreamWriter.data(),
                               dataStreamWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());

        // note: maps are used!
        {
            auto line = m_pixmap.getLine(0);
            CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(line[0]));
            CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(line[1]));
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[2]));
        }
        {
            auto line = m_pixmap.getLine(1);
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[0]));
        }
        {
            auto line = m_pixmap.getLine(2);
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(line[0]));
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(line[1]));
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(line[2]));
            CPPUNIT_ASSERT_EQUAL(0x44, static_cast<int>(line[3]));
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[4]));
        }
        {
            auto line = m_pixmap.getLine(3);
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[0]));
        }
        {
            auto line = m_pixmap.getLine(4);
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[0]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[1]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[2]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[3]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[4]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[5]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[6]));
            CPPUNIT_ASSERT_EQUAL(0x08, static_cast<int>(line[7]));
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[8]));
        }
        {
            auto line = m_pixmap.getLine(3);
            CPPUNIT_ASSERT_EQUAL(0x99, static_cast<int>(line[0]));
        }
    }

    void testConstructorValidation()
    {
        // Test constructor with valid parameters
        PixelStringWriter dataStreamWriter;
        dataStreamWriter.start2bitPixelCodeString();
        dataStreamWriter.write2bitPixels(1, 1);
        dataStreamWriter.end2bitPixelCodeString();

        PesPacketReader reader(dataStreamWriter.data(), dataStreamWriter.size(), nullptr, 0);
        PixelWriter writer(false, 2, m_pixmap, 0, 0);
        
        // Constructor should not throw with valid references
        CPPUNIT_ASSERT_NO_THROW(ObjectParser parser(reader, writer));
    }

    void testEmptyDataStream()
    {
        // Test parser with empty data stream
        std::vector<std::uint8_t> emptyData;
        PesPacketReader reader(emptyData.data(), 0, nullptr, 0);
        PixelWriter writer(false, 2, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        // Should handle empty stream gracefully
        CPPUNIT_ASSERT_NO_THROW(parser.parse());
    }

    void testInvalidDataType()
    {
        // Test with various invalid data types
        std::vector<std::uint8_t> invalidDataTypes = {0x00, 0x01, 0x13, 0x1F, 0x23, 0x30, 0xEF, 0xF1, 0xFF};
        
        for (auto dataType : invalidDataTypes)
        {
            BitStreamWriter invalidStreamWriter;
            invalidStreamWriter.write(dataType, 8);

            PesPacketReader reader(invalidStreamWriter.data(), invalidStreamWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }
    }

    void testBoundaryDataTypes()
    {
        // Test all valid data type boundaries
        std::vector<std::uint8_t> validDataTypes = {0x10, 0x11, 0x12, 0x20, 0x21, 0x22, 0xF0};
        
        for (auto dataType : validDataTypes)
        {
            BitStreamWriter streamWriter;
            streamWriter.write(dataType, 8);
            
            if (dataType == 0xF0) // End of line - should not throw
            {
                PesPacketReader reader(streamWriter.data(), streamWriter.size(), nullptr, 0);
                PixelWriter writer(false, 2, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);
                CPPUNIT_ASSERT_NO_THROW(parser.parse());
            }
            else if (dataType >= 0x20 && dataType <= 0x22) // Map tables - need proper data
            {
                if (dataType == 0x20) // 2to4 map
                {
                    streamWriter.write(0x00, 4); // entry 0
                    streamWriter.write(0x01, 4); // entry 1  
                    streamWriter.write(0x02, 4); // entry 2
                    streamWriter.write(0x03, 4); // entry 3
                }
                else if (dataType == 0x21) // 2to8 map
                {
                    for (int i = 0; i < 4; ++i)
                        streamWriter.write(i, 8);
                }
                else // 4to8 map
                {
                    for (int i = 0; i < 16; ++i)
                        streamWriter.write(i, 8);
                }
                
                PesPacketReader reader(streamWriter.data(), streamWriter.size(), nullptr, 0);
                PixelWriter writer(false, 8, m_pixmap, 0, 0);
                ObjectParser parser(reader, writer);
                CPPUNIT_ASSERT_NO_THROW(parser.parse());
            }
        }
    }

    void testIncompletePixelString()
    {
        // Test incomplete 2-bit pixel string (missing data for run length)
        {
            BitStreamWriter incompleteWriter;
            incompleteWriter.write(0x10, 8); // 2-bit pixel string
            incompleteWriter.write(0x00, 2); // 00 - start escape sequence  
            incompleteWriter.write(0x00, 1); // 0 - continue to next level
            incompleteWriter.write(0x00, 1); // 0 - continue to next level
            incompleteWriter.write(0x03, 2); // 11 - signals long run (needs 8 more bits + 2 bits for color)
            // Missing the required 8 bits for run length and 2 bits for color

            PesPacketReader reader(incompleteWriter.data(), incompleteWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }

        // Test incomplete 4-bit pixel string (missing data for run length)
        {
            BitStreamWriter incompleteWriter;
            incompleteWriter.write(0x11, 8); // 4-bit pixel string
            incompleteWriter.write(0x00, 4); // 0000 - start escape sequence
            incompleteWriter.write(0x01, 1); // 1 - continue to next level
            incompleteWriter.write(0x01, 1); // 1 - continue to next level  
            incompleteWriter.write(0x03, 2); // 11 - signals long run (needs 8 more bits + 4 bits for color)
            // Missing the required 8 bits for run length and 4 bits for color

            PesPacketReader reader(incompleteWriter.data(), incompleteWriter.size(), nullptr, 0);
            PixelWriter writer(false, 4, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }
    }

    void testCorruptedMapTable()
    {
        // Test corrupted 2to4 map table (insufficient data)
        {
            BitStreamWriter corruptWriter;
            corruptWriter.write(0x20, 8); // 2to4 map
            corruptWriter.write(0x00, 4); // only one entry instead of 4

            PesPacketReader reader(corruptWriter.data(), corruptWriter.size(), nullptr, 0);
            PixelWriter writer(false, 4, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }

        // Test corrupted 4to8 map table (insufficient data)
        {
            BitStreamWriter corruptWriter;
            corruptWriter.write(0x22, 8); // 4to8 map
            for (int i = 0; i < 8; ++i) // only 8 entries instead of 16
                corruptWriter.write(i, 8);

            PesPacketReader reader(corruptWriter.data(), corruptWriter.size(), nullptr, 0);
            PixelWriter writer(false, 8, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), PesPacketReader::Exception);
        }
    }

    void testMaxRunLengthBoundaries()
    {
        // Test maximum run length for 2-bit pixels (284)
        {
            PixelStringWriter maxRunWriter;
            maxRunWriter.start2bitPixelCodeString();
            maxRunWriter.write2bitPixels(1, 284); // Maximum run length
            maxRunWriter.end2bitPixelCodeString();

            m_pixmap.clear(0xFF);
            PesPacketReader reader(maxRunWriter.data(), maxRunWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_NO_THROW(parser.parse());
        }

        // Test maximum run length for 4-bit pixels (280)
        {
            PixelStringWriter maxRunWriter;
            maxRunWriter.start4bitPixelCodeString();
            maxRunWriter.write4bitPixels(1, 280); // Maximum run length
            maxRunWriter.end4bitPixelCodeString();

            m_pixmap.clear(0xFF);
            PesPacketReader reader(maxRunWriter.data(), maxRunWriter.size(), nullptr, 0);
            PixelWriter writer(false, 4, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_NO_THROW(parser.parse());
        }

        // Test maximum run length for 8-bit pixels (127)
        {
            PixelStringWriter maxRunWriter;
            maxRunWriter.start8bitPixelCodeString();
            maxRunWriter.write8bitPixels(1, 127); // Maximum run length
            maxRunWriter.end8bitPixelCodeString();

            m_pixmap.clear(0xFF);
            PesPacketReader reader(maxRunWriter.data(), maxRunWriter.size(), nullptr, 0);
            PixelWriter writer(false, 8, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_NO_THROW(parser.parse());
        }
    }

    void testZeroRunLength()
    {
        // Test zero run length for different bit depths
        {
            PixelStringWriter zeroRunWriter;
            zeroRunWriter.start2bitPixelCodeString();
            zeroRunWriter.write2bitPixels(1, 0); // Zero run length
            zeroRunWriter.end2bitPixelCodeString();

            m_pixmap.clear(0xFF);
            PesPacketReader reader(zeroRunWriter.data(), zeroRunWriter.size(), nullptr, 0);
            PixelWriter writer(false, 2, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_NO_THROW(parser.parse());
            
            // Verify no pixels were written
            auto line = m_pixmap.getLine(0);
            CPPUNIT_ASSERT_EQUAL(0xFF, static_cast<int>(line[0]));
        }
    }

    void testDepthMismatchValidation()
    {
        // Test all depth mismatches comprehensively
        std::vector<std::pair<int, int>> depthMismatches = {
            {3, 2}, {3, 4}, {3, 8}, // Invalid depth 3
            {5, 2}, {5, 4}, {5, 8}  // Invalid depth 5
        };

        for (auto mismatch : depthMismatches)
        {
            PixelStringWriter dataWriter;
            dataWriter.start2bitPixelCodeString();
            dataWriter.write2bitPixels(1, 1);
            dataWriter.end2bitPixelCodeString();

            PesPacketReader reader(dataWriter.data(), dataWriter.size(), nullptr, 0);
            PixelWriter writer(false, mismatch.first, m_pixmap, 0, 0);
            ObjectParser parser(reader, writer);

            CPPUNIT_ASSERT_THROW(parser.parse(), ParserException);
        }
    }

    void testMixedDataTypes()
    {
        // Test mixing different data types in single parse
        PixelStringWriter mixedWriter;
        
        // 2-bit string
        mixedWriter.start2bitPixelCodeString();
        mixedWriter.write2bitPixels(1, 2);
        mixedWriter.end2bitPixelCodeString();
        
        // End of line
        mixedWriter.writeEndOfLine();
        
        // Map table
        mixedWriter.start2to8bitMap();
        for (int i = 0; i < 4; ++i)
            mixedWriter.write2to8bitMapEntry(i * 50);
        mixedWriter.end2to8bitMap();
        
        // Another pixel string
        mixedWriter.start2bitPixelCodeString();
        mixedWriter.write2bitPixels(2, 3);
        mixedWriter.end2bitPixelCodeString();

        m_pixmap.clear(0xFF);
        PesPacketReader reader(mixedWriter.data(), mixedWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());
    }

    void testConsecutiveEndOfLine()
    {
        // Test multiple consecutive end-of-line markers
        BitStreamWriter eolWriter;
        eolWriter.write(0xF0, 8); // End of line
        eolWriter.write(0xF0, 8); // Another end of line
        eolWriter.write(0xF0, 8); // Third end of line

        PesPacketReader reader(eolWriter.data(), eolWriter.size(), nullptr, 0);
        PixelWriter writer(false, 2, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());
    }

    void testLargePixelCount()
    {
        // Test with large pixel counts to stress the system
        m_pixmap.init(1000, 10, m_pixmapBuffer.data()); // Larger pixmap
        
        PixelStringWriter largeWriter;
        largeWriter.start8bitPixelCodeString();
        largeWriter.write8bitPixels(100, 127); // Large run
        largeWriter.write8bitPixels(200, 100); // Another large run
        largeWriter.end8bitPixelCodeString();

        m_pixmap.clear(0xFF);
        PesPacketReader reader(largeWriter.data(), largeWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());
        
        // Verify pixel values
        auto line = m_pixmap.getLine(0);
        for (int i = 0; i < 127; ++i)
            CPPUNIT_ASSERT_EQUAL(100, static_cast<int>(line[i]));
        for (int i = 127; i < 227; ++i)
            CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(line[i]));
    }

    void testMapTableOverflow()
    {
        // Test accessing map table with boundary indices
        PixelStringWriter overflowWriter;
        
        // Create 2to4 map
        overflowWriter.start2to4bitMap();
        overflowWriter.write2to4bitMapEntry(15); // Max 4-bit value
        overflowWriter.write2to4bitMapEntry(14);
        overflowWriter.write2to4bitMapEntry(13);
        overflowWriter.write2to4bitMapEntry(12);
        overflowWriter.end2to4bitMap();
        
        // Use all map indices
        overflowWriter.start2bitPixelCodeString();
        overflowWriter.write2bitPixels(0, 1);
        overflowWriter.write2bitPixels(1, 1);
        overflowWriter.write2bitPixels(2, 1);
        overflowWriter.write2bitPixels(3, 1); // Index 3 (max for 2-bit)
        overflowWriter.end2bitPixelCodeString();

        m_pixmap.clear(0xFF);
        PesPacketReader reader(overflowWriter.data(), overflowWriter.size(), nullptr, 0);
        PixelWriter writer(false, 4, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());
        
        // Verify mapped values
        auto line = m_pixmap.getLine(0);
        CPPUNIT_ASSERT_EQUAL(15, static_cast<int>(line[0]));
        CPPUNIT_ASSERT_EQUAL(14, static_cast<int>(line[1]));
        CPPUNIT_ASSERT_EQUAL(13, static_cast<int>(line[2]));
        CPPUNIT_ASSERT_EQUAL(12, static_cast<int>(line[3]));
    }

    void testParseMultipleObjects()
    {
        // Test parsing multiple complete objects in sequence
        PixelStringWriter multiWriter;
        
        // First object - 2-bit
        multiWriter.start2bitPixelCodeString();
        multiWriter.write2bitPixels(1, 5);
        multiWriter.end2bitPixelCodeString();
        multiWriter.writeEndOfLine();
        
        // Second object - 4-bit  
        multiWriter.start4bitPixelCodeString();
        multiWriter.write4bitPixels(8, 3);
        multiWriter.end4bitPixelCodeString();
        multiWriter.writeEndOfLine();
        
        // Third object - 8-bit
        multiWriter.start8bitPixelCodeString();
        multiWriter.write8bitPixels(255, 2);
        multiWriter.end8bitPixelCodeString();

        m_pixmap.clear(0x00);
        PesPacketReader reader(multiWriter.data(), multiWriter.size(), nullptr, 0);
        PixelWriter writer(false, 8, m_pixmap, 0, 0);
        ObjectParser parser(reader, writer);

        CPPUNIT_ASSERT_NO_THROW(parser.parse());
        
        // Verify all objects were processed correctly
        auto line0 = m_pixmap.getLine(0);
        auto line2 = m_pixmap.getLine(2);
        auto line4 = m_pixmap.getLine(4);
        
        // First object (mapped from 2-bit)
        CPPUNIT_ASSERT_EQUAL(0x77, static_cast<int>(line0[0])); // Default 2to8 mapping for index 1
        
        // Second object (mapped from 4-bit) 
        CPPUNIT_ASSERT_EQUAL(0x88, static_cast<int>(line2[0])); // Default 4to8 mapping for index 8
        
        // Third object (direct 8-bit)
        CPPUNIT_ASSERT_EQUAL(255, static_cast<int>(line4[0]));
    }

private:
    static const std::int32_t WIDTH = 1000;
    static const std::int32_t HEIGHT = 100;
    static const std::int32_t PIXMAP_SIZE = WIDTH * HEIGHT;

    std::array<std::uint8_t, PIXMAP_SIZE> m_pixmapBuffer;
    Pixmap m_pixmap;
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixelWriterTest);
