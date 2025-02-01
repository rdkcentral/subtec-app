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
    CPPUNIT_TEST_SUITE_END()
    ;

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

private:
    static const std::int32_t WIDTH = 512;
    static const std::int32_t HEIGHT = 100;
    static const std::int32_t PIXMAP_SIZE = WIDTH * HEIGHT;

    std::array<std::uint8_t, PIXMAP_SIZE> m_pixmapBuffer;
    Pixmap m_pixmap;
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixelWriterTest);
