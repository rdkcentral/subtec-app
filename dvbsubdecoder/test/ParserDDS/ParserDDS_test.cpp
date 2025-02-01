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
    CPPUNIT_TEST(testBadWindow);CPPUNIT_TEST_SUITE_END()
    ;

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

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserDDSTest);
