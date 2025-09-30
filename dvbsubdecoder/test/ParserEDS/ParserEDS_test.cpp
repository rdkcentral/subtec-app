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

#include "ParserEDS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "Misc.hpp"      // Provides Rectangle operator== implementation

#include "DecoderClientMock.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserEDS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserEDSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserEDSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testDisplayTransferVerification);
    CPPUNIT_TEST(testParsedDisplayResetVerification);
    CPPUNIT_TEST(testPesPacketReaderNonUsage);
    CPPUNIT_TEST(testNullEmptyDisplayStateTransfer);
    CPPUNIT_TEST(testMultipleIncompleteCompleteCycles);
    CPPUNIT_TEST(testVersionRolloverHandling);
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
        // no bytes are read from the reader
        PesPacketReader reader;

        ParserEDS parser;

        // verify page state
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        // invalid state, do nothing
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        // valid state, finish parsing
        m_database->getPage().startParsing(0, StcTime(), 0);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // complete state, do nothing
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // timed out state, do nothing
        m_database->getPage().setTimedOut();
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::TIMEDOUT);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::TIMEDOUT);
    }

    void testDisplayTransferVerification()
    {
        PesPacketReader reader;
        ParserEDS parser;

        // Set up parsed display with specific values
        auto& parsedDisplay = m_database->getParsedDisplay();
        parsedDisplay.set(5, {10, 20, 640, 480}, {50, 60, 590, 420});

        // Verify initial state of current display (should be default)
        auto& currentDisplay = m_database->getCurrentDisplay();
        CPPUNIT_ASSERT(currentDisplay.getVersion() == dvbsubdecoder::INVALID_VERSION);

        // Start parsing and trigger EDS
        m_database->getPage().startParsing(0, StcTime(), 0);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);

        // Verify that current display now matches what was in parsed display
        CPPUNIT_ASSERT(currentDisplay.getVersion() == 5);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_x1 == 10);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_y1 == 20);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_x2 == 640);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_y2 == 480);
        CPPUNIT_ASSERT(currentDisplay.getWindowBounds().m_x1 == 50);
        CPPUNIT_ASSERT(currentDisplay.getWindowBounds().m_y1 == 60);
        CPPUNIT_ASSERT(currentDisplay.getWindowBounds().m_x2 == 590);
        CPPUNIT_ASSERT(currentDisplay.getWindowBounds().m_y2 == 420);
    }

    void testParsedDisplayResetVerification()
    {
        PesPacketReader reader;
        ParserEDS parser;

        // Set up parsed display with specific values
        auto& parsedDisplay = m_database->getParsedDisplay();
        parsedDisplay.set(8, {100, 200, 800, 600}, {150, 250, 750, 550});

        // Verify parsed display has values before EDS
        CPPUNIT_ASSERT(parsedDisplay.getVersion() == 8);
        CPPUNIT_ASSERT(parsedDisplay.getDisplayBounds().m_x1 == 100);

        // Start parsing and trigger EDS
        m_database->getPage().startParsing(0, StcTime(), 0);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);

        // Verify that parsed display has been reset to defaults
        CPPUNIT_ASSERT(parsedDisplay.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(parsedDisplay.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(parsedDisplay.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(parsedDisplay.getDisplayBounds().m_x2 == 720); // DEFAULT_SD_WIDTH
        CPPUNIT_ASSERT(parsedDisplay.getDisplayBounds().m_y2 == 576); // DEFAULT_SD_HEIGHT
    }

    void testPesPacketReaderNonUsage()
    {
        ParserEDS parser;

        // Create reader with some data
        std::vector<std::uint8_t> data = {0x01, 0x02, 0x03, 0x04};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0);

        // Verify reader has data available initially
        CPPUNIT_ASSERT(reader.getBytesLeft() == 4);

        // Start parsing and trigger EDS
        m_database->getPage().startParsing(0, StcTime(), 0);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);

        // Verify that EDS parser did not consume any bytes from reader
        CPPUNIT_ASSERT(reader.getBytesLeft() == 4);
    }

    // State Management
    void testNullEmptyDisplayStateTransfer()
    {
        PesPacketReader reader;
        ParserEDS parser;

        // Parsed display starts in default/reset state (no DDS was parsed)
        auto& parsedDisplay = m_database->getParsedDisplay();
        CPPUNIT_ASSERT(parsedDisplay.getVersion() == dvbsubdecoder::INVALID_VERSION);

        m_database->getPage().startParsing(0, StcTime(), 0);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);

        // Verify that default state was transferred to current display
        auto& currentDisplay = m_database->getCurrentDisplay();
        CPPUNIT_ASSERT(currentDisplay.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_x2 == 720); // DEFAULT_SD_WIDTH
        CPPUNIT_ASSERT(currentDisplay.getDisplayBounds().m_y2 == 576); // DEFAULT_SD_HEIGHT
        CPPUNIT_ASSERT(currentDisplay.getWindowBounds() == currentDisplay.getDisplayBounds());
    }

    void testMultipleIncompleteCompleteCycles()
    {
        PesPacketReader reader;
        ParserEDS parser;

        // First cycle: set display, parse, verify
        {
            auto& parsedDisplay = m_database->getParsedDisplay();
            parsedDisplay.set(1, {0, 0, 100, 100}, {10, 10, 90, 90});

            m_database->getPage().startParsing(0, StcTime(), 0);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
            
            parser.parseEndOfDisplaySetSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getVersion() == 1);
        }

        // Second cycle: new display, parse, verify
        {
            auto& parsedDisplay = m_database->getParsedDisplay();
            parsedDisplay.set(2, {0, 0, 200, 200}, {20, 20, 180, 180});

            m_database->getPage().startParsing(1, StcTime(), 0);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
            
            parser.parseEndOfDisplaySetSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getVersion() == 2);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getDisplayBounds().m_x2 == 200);
        }

        // Third cycle: different display, parse, verify
        {
            auto& parsedDisplay = m_database->getParsedDisplay();
            parsedDisplay.set(3, {0, 0, 300, 300}, {30, 30, 270, 270});

            m_database->getPage().startParsing(2, StcTime(), 0);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
            
            parser.parseEndOfDisplaySetSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getVersion() == 3);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getDisplayBounds().m_x2 == 300);
        }
    }

    void testVersionRolloverHandling()
    {
        PesPacketReader reader;
        ParserEDS parser;

        // Test version rollover scenario: 15 -> 0
        {
            auto& parsedDisplay = m_database->getParsedDisplay();
            parsedDisplay.set(15, {0, 0, 1000, 1000}, {100, 100, 900, 900});

            m_database->getPage().startParsing(0, StcTime(), 0);
            parser.parseEndOfDisplaySetSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getVersion() == 15);
        }

        // Next page with version 0 (rollover)
        {
            auto& parsedDisplay = m_database->getParsedDisplay();
            parsedDisplay.set(0, {0, 0, 1100, 1100}, {110, 110, 990, 990});

            m_database->getPage().startParsing(1, StcTime(), 0);
            parser.parseEndOfDisplaySetSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getVersion() == 0);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getDisplayBounds().m_x2 == 1100);
            CPPUNIT_ASSERT(m_database->getCurrentDisplay().getWindowBounds().m_x1 == 110);
        }
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserEDSTest);
