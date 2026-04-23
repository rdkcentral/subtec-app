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

#include "ParserDDS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "ParserException.hpp"

#include "DecoderClientMock.hpp"
#include "BitStreamWriter.hpp"
#include "Misc.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserDDS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::ParserException;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserDDSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserDDSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testNoWindow);
    CPPUNIT_TEST(testNotEnoughData);
    CPPUNIT_TEST(testSameVersion);
    CPPUNIT_TEST(testBadSize);
    CPPUNIT_TEST(testBadWindow);
    CPPUNIT_TEST(testMaxSizeValues);
    CPPUNIT_TEST(testMinSizeValues);
    CPPUNIT_TEST(testVersionRollover);
    CPPUNIT_TEST(testSizeAdjustmentSuccess);
    CPPUNIT_TEST(testSizeAdjustmentFailure);
    CPPUNIT_TEST(testWindowAtDisplayBoundary);
    CPPUNIT_TEST(testWindowZeroCoordinates);
    CPPUNIT_TEST(testWindowEqualCoordinates);
    CPPUNIT_TEST(testVersionSkipping);
    CPPUNIT_TEST(testDisplayBoundsCalculation);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_client.reset(new DecoderClientMock());
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));
    }

    void tearDown()
    {
        m_database.reset();
        m_pixmapAllocator.reset();
        m_client.reset();
    }

    void testSimple()
    {
        BitStreamWriter writer;
        writer.write((0xC << 4) | (1 << 3), 8); // version and flags
        writer.write(700 - 1, 16); // width
        writer.write(400 - 1, 16); // height
        writer.write(100, 16); // window x1
        writer.write(500 - 1, 16); // window y1
        writer.write(80, 16); // window x2
        writer.write(300 - 1, 16); // window y2

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xC);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 700);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 400);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 100);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 80);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 500);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 300);
    }

    void testNoWindow()
    {
        BitStreamWriter writer;
        writer.write(0xC << 4, 8); // version and flags
        writer.write(700 - 1, 16); // width
        writer.write(400 - 1, 16); // height

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xC);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 700);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 400);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 700);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 400);
    }

    void testNotEnoughData()
    {
        ParserDDS parser;

        // empty reader
        {
            PesPacketReader reader;
            CPPUNIT_ASSERT_THROW(
                    parser.parseDisplayDefinitionSegment(*m_database, reader),
                    PesPacketReader::Exception);
        }

        // header only
        {
            BitStreamWriter writer;
            writer.write(0xC0, 8); // version and flags
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);

            CPPUNIT_ASSERT_THROW(
                    parser.parseDisplayDefinitionSegment(*m_database, reader),
                    PesPacketReader::Exception);
        }

        // broken display size
        {
            BitStreamWriter writer;
            writer.write(0xC0, 8); // version and flags
            writer.write(0x0123, 16); // width - full
            writer.write(0x0123, 8); // height - first byte only
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);

            CPPUNIT_ASSERT_THROW(
                    parser.parseDisplayDefinitionSegment(*m_database, reader),
                    PesPacketReader::Exception);
        }

        // half of window
        {
            BitStreamWriter writer;
            writer.write((0xC << 4) | (1 << 3), 8); // version and flags
            writer.write(0x0123, 16); // width
            writer.write(0x0123, 16); // height
            writer.write(0, 16); // window x1
            writer.write(0, 16); // window y1
            // missing rest of window
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);

            CPPUNIT_ASSERT_THROW(
                    parser.parseDisplayDefinitionSegment(*m_database, reader),
                    PesPacketReader::Exception);
        }
    }

    void testSameVersion()
    {
        BitStreamWriter writer;
        writer.write(0xC << 4, 8); // version and flags
        writer.write(700 - 1, 16); // width
        writer.write(400 - 1, 16); // height

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xC);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 700);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 400);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 700);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 400);

        display.set(display.getVersion(), display.getDisplayBounds(),
        { 0, 0, 1, 1 });

        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 1);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 1);

        PesPacketReader reader2(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader2);

        // not parsed again, so not modified
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 1);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 1);
    }

    void testBadSize()
    {
        {
            BitStreamWriter writer;
            writer.write(0xC << 4, 8); // version and flags
            writer.write(0xFFFF, 16); // width
            writer.write(0xFFFF, 16); // height

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

        {
            BitStreamWriter writer;
            writer.write(0xC << 4, 8); // version and flags
            writer.write(100, 16); // width
            writer.write(0xFFFF, 16); // height

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

        {
            BitStreamWriter writer;
            writer.write(0xC << 4, 8); // version and flags
            writer.write(0xFFFF, 16); // width
            writer.write(100, 16); // height

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

    }

    void testBadWindow()
    {
        {
            BitStreamWriter writer;
            writer.write((0xC << 4) | (1 << 3), 8); // version and flags
            writer.write(700 - 1, 16); // width
            writer.write(400 - 1, 16); // height
            writer.write(0, 16);
            writer.write(700, 16);
            writer.write(0, 16);
            writer.write(100, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

        {
            BitStreamWriter writer;
            writer.write((0xC << 4) | (1 << 3), 8); // version and flags
            writer.write(700 - 1, 16); // width
            writer.write(400 - 1, 16); // height
            writer.write(0, 16);
            writer.write(600, 16);
            writer.write(0, 16);
            writer.write(400, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

        {
            BitStreamWriter writer;
            writer.write((0xC << 4) | (1 << 3), 8); // version and flags
            writer.write(700 - 1, 16); // width
            writer.write(400 - 1, 16); // height
            writer.write(101, 16);
            writer.write(100, 16);
            writer.write(100, 16);
            writer.write(100, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }

        {
            BitStreamWriter writer;
            writer.write((0xC << 4) | (1 << 3), 8); // version and flags
            writer.write(700 - 1, 16); // width
            writer.write(400 - 1, 16); // height
            writer.write(100, 16);
            writer.write(100, 16);
            writer.write(101, 16);
            writer.write(100, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            CPPUNIT_ASSERT_THROW(
                    ParserDDS().parseDisplayDefinitionSegment(*m_database,
                            reader), ParserException);
        }
    }

    void testMaxSizeValues()
    {
        BitStreamWriter writer;
        writer.write(0xA << 4, 8); // version and flags
        writer.write(1919 - 1, 16); // max width
        writer.write(1079 - 1, 16); // max height

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xA);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 1919);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 1079);
    }

    void testMinSizeValues()
    {
        BitStreamWriter writer;
        writer.write(0x1 << 4, 8); // version and flags
        writer.write(1 - 1, 16); // min width (1 pixel)
        writer.write(1 - 1, 16); // min height (1 pixel)

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0x1);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 1);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 1);
    }

    void testVersionRollover()
    {
        // Start with version 15
        {
            BitStreamWriter writer;
            writer.write(0xF << 4, 8);
            writer.write(100 - 1, 16);
            writer.write(100 - 1, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

            const auto& display = m_database->getParsedDisplay();
            CPPUNIT_ASSERT(display.getVersion() == 0xF);
        }

        // Rollover to version 0
        {
            BitStreamWriter writer;
            writer.write(0x0 << 4, 8);
            writer.write(200 - 1, 16);
            writer.write(200 - 1, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

            const auto& display = m_database->getParsedDisplay();
            CPPUNIT_ASSERT(display.getVersion() == 0x0);
            CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 200);
        }
    }

    void testSizeAdjustmentSuccess()
    {
        // Oversized raw display, but window defines a valid in-range sub-area so adjustment succeeds.
        BitStreamWriter writer;
        writer.write((0x7 << 4) | (1 << 3), 8); // version and window flag
        writer.write(3000 - 1, 16); // raw width exceeds limit
        writer.write(2500 - 1, 16); // raw height exceeds limit
        // Window spans exactly the maximum permitted dimensions (0..1919, 0..1079)
        writer.write(0, 16);          // window min x
        writer.write(1919 - 1, 16);   // window max x (1918 raw -> span 1919)
        writer.write(0, 16);          // window min y
        writer.write(1079 - 1, 16);   // window max y (1078 raw -> span 1079)

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0x7);
        // After adjustment, display width/height become (windowMax - windowMin) + 1
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 1919);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 1079);
    }

    void testSizeAdjustmentFailure()
    {
        BitStreamWriter writer;
        writer.write((0x8 << 4) | (1 << 3), 8); // version and window flag
        writer.write(3000 - 1, 16); // exceeds max width
        writer.write(2000 - 1, 16); // exceeds max height
        writer.write(0, 16); // window min x
        writer.write(3000 - 1, 16); // window max x (still too large)
        writer.write(0, 16); // window min y
        writer.write(2000 - 1, 16); // window max y (still too large)

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(
            ParserDDS().parseDisplayDefinitionSegment(*m_database, reader),
            ParserException);
    }

    // Window Edge Cases
    void testWindowAtDisplayBoundary()
    {
        BitStreamWriter writer;
        writer.write((0xB << 4) | (1 << 3), 8); // version and window flag
        writer.write(800 - 1, 16); // width
        writer.write(600 - 1, 16); // height
        writer.write(0, 16); // window min x at boundary
        writer.write(800 - 1, 16); // window max x at boundary
        writer.write(0, 16); // window min y at boundary
        writer.write(600 - 1, 16); // window max y at boundary

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xB);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 800);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 600);
    }

    void testWindowZeroCoordinates()
    {
        BitStreamWriter writer;
        writer.write((0xD << 4) | (1 << 3), 8); // version and window flag
        writer.write(400 - 1, 16); // width
        writer.write(300 - 1, 16); // height
        writer.write(0, 16); // window min x
        writer.write(0, 16); // window max x
        writer.write(0, 16); // window min y
        writer.write(0, 16); // window max y

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xD);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 1); // 0 + 1
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 1); // 0 + 1
    }

    void testWindowEqualCoordinates()
    {
        BitStreamWriter writer;
        writer.write((0xE << 4) | (1 << 3), 8); // version and window flag
        writer.write(500 - 1, 16); // width
        writer.write(400 - 1, 16); // height
        writer.write(100, 16); // window min x
        writer.write(100, 16); // window max x (equal to min)
        writer.write(150, 16); // window min y
        writer.write(150, 16); // window max y (equal to min)

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0xE);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 100);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 101); // 100 + 1
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 150);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 151); // 150 + 1
    }

    // Version Handling
    void testVersionSkipping()
    {
        // Start with version 2
        {
            BitStreamWriter writer;
            writer.write(0x2 << 4, 8);
            writer.write(300 - 1, 16);
            writer.write(200 - 1, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

            const auto& display = m_database->getParsedDisplay();
            CPPUNIT_ASSERT(display.getVersion() == 0x2);
        }

        // Skip to version 7
        {
            BitStreamWriter writer;
            writer.write(0x7 << 4, 8);
            writer.write(600 - 1, 16);
            writer.write(400 - 1, 16);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

            const auto& display = m_database->getParsedDisplay();
            CPPUNIT_ASSERT(display.getVersion() == 0x7);
            CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 600);
            CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 400);
        }
    }

    // Data Flow
    void testDisplayBoundsCalculation()
    {
        // Test that display bounds are correctly calculated with +1 offset
        BitStreamWriter writer;
        writer.write(0x3 << 4, 8);
        writer.write(719, 16); // 720 - 1
        writer.write(575, 16); // 576 - 1

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserDDS().parseDisplayDefinitionSegment(*m_database, reader);

        const auto& display = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(display.getVersion() == 0x3);
        
        // Verify the +1 calculation for bounds
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getDisplayBounds().m_x2 == 720); // 719 + 1
        CPPUNIT_ASSERT(display.getDisplayBounds().m_y2 == 576); // 575 + 1
        
        // Window should match display when no window flag
        CPPUNIT_ASSERT(display.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(display.getWindowBounds().m_x2 == 720);
        CPPUNIT_ASSERT(display.getWindowBounds().m_y2 == 576);
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserDDSTest);
