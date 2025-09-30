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

#include "ParserPCS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"

#include "BitStreamWriter.hpp"
#include "Misc.hpp"
#include "DecoderClientMock.hpp"
#include "ParserException.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserException;
using dvbsubdecoder::ParserPCS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserPCSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserPCSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testRegionsLimit);
    CPPUNIT_TEST(testSameVersion);
    CPPUNIT_TEST(testInvalidState);
    CPPUNIT_TEST(testNotEnoughData);
    CPPUNIT_TEST(testStartSetNeeded);
    CPPUNIT_TEST(testAcquisitionPointSkipped);
    CPPUNIT_TEST(testVersionNumberBoundaries);
    CPPUNIT_TEST(testTimeoutBoundaryValues);
    CPPUNIT_TEST(testRegionPositionBoundaries);
    CPPUNIT_TEST(testVersionNumberWraparound);
    CPPUNIT_TEST(testDuplicateRegionIds);
    CPPUNIT_TEST(testReservedBitsIgnored);
    CPPUNIT_TEST(testZeroLengthSegment);
    CPPUNIT_TEST(testPtsTimeHandling);
    CPPUNIT_TEST(testPartialRegionData);
    CPPUNIT_TEST(testMaxRegionCountExceeded);
    CPPUNIT_TEST(testNormalCaseWithInvalidPage);
    CPPUNIT_TEST(testNormalCaseWithIncompletePage);
    CPPUNIT_TEST(testMixedPageStateTransitions);
    CPPUNIT_TEST(testEmptyDatabaseScenario);
    CPPUNIT_TEST(testRegionAddressOverflow);
    CPPUNIT_TEST(testConcurrentVersionProcessing);
    CPPUNIT_TEST(testPageStateValidation);
    CPPUNIT_TEST(testAcquisitionPointWithIncompleteState);
    CPPUNIT_TEST(testAcquisitionPointWithInvalidState);
    CPPUNIT_TEST(testModeChangeEpochReset);
    CPPUNIT_TEST(testRegionDataIntegrity);
    CPPUNIT_TEST(testMultiplePcsSequence);
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
        BitStreamWriter bitStreamWriter;

        const std::uint8_t TIMEOUT = 0x74;
        const std::uint8_t PAGE_VERSION = 0x06;
        const std::uint8_t PAGE_STATE =
                dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t REGION_COUNT = 5;
        const StcTime STC_TIME(StcTimeType::LOW_32, 165);

        bitStreamWriter.write(TIMEOUT, 8);
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved
        for (std::uint8_t i = 0; i < REGION_COUNT; ++i)
        {
            bitStreamWriter.write(i, 8); // regionId
            bitStreamWriter.write(0, 8); // reserved
            bitStreamWriter.write(i * 2, 16); // horizontal address
            bitStreamWriter.write(i * 2 + 1, 16); // vertical address
        }

        m_database->getStatus().setLastPts(STC_TIME);

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(),
                nullptr, 0);

        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();

        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getTimeout() == TIMEOUT);
        CPPUNIT_ASSERT(page.getPts() == STC_TIME);
        CPPUNIT_ASSERT(page.getRegionCount() == REGION_COUNT);
        for (std::uint8_t i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(page.getRegion(i).m_regionId == i);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionX == i * 2);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionY == i * 2 + 1);
        }
    }

    void testRegionsLimit()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE =
                dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t REGION_LIMIT = dvbsubdecoder::MAX_SUPPORTED_REGIONS;
        const std::uint8_t REGION_COUNT = REGION_LIMIT + 1;

        bitStreamWriter.write(0, 8);
        bitStreamWriter.write(0, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved
        for (std::uint8_t i = 0; i < REGION_COUNT; ++i)
        {
            bitStreamWriter.write(i, 8); // regionId
            bitStreamWriter.write(0, 8); // reserved
            bitStreamWriter.write(i * 2, 16); // horizontal address
            bitStreamWriter.write(i * 2 + 1, 16); // vertical address
        }

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(),
                nullptr, 0);

        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();

        CPPUNIT_ASSERT(page.getRegionCount() == REGION_LIMIT);
        for (std::uint8_t i = 0; i < REGION_LIMIT; ++i)
        {
            CPPUNIT_ASSERT(page.getRegion(i).m_regionId == i);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionX == i * 2);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionY == i * 2 + 1);
        }
    }

    void testSameVersion()
    {
        const std::uint8_t PAGE_STATE =
                dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t TIMEOUT1 = 0x74;
        const std::uint8_t TIMEOUT2 = 0x47;
        const std::uint8_t PAGE_VERSION = 0x06;

        BitStreamWriter bitStreamWriter1;
        bitStreamWriter1.write(TIMEOUT1, 8);
        bitStreamWriter1.write(PAGE_VERSION, 4);
        bitStreamWriter1.write(PAGE_STATE, 2);
        bitStreamWriter1.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter1.data(),
                bitStreamWriter1.size(), nullptr, 0);

        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(TIMEOUT2, 8);
        bitStreamWriter2.write(PAGE_VERSION, 4);
        bitStreamWriter2.write(PAGE_STATE, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(),
                bitStreamWriter2.size(), nullptr, 0);

        ParserPCS parser;

        parser.parsePageCompositionSegment(*m_database, reader1);
        const auto& page1 = m_database->getPage();
        CPPUNIT_ASSERT(page1.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page1.getTimeout() == TIMEOUT1);

        parser.parsePageCompositionSegment(*m_database, reader2);
        const auto& page2 = m_database->getPage();
        CPPUNIT_ASSERT(page2.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page2.getTimeout() == TIMEOUT1);
    }

    void testInvalidState()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE =
                dvbsubdecoder::PageStateBits::RESERVED;

        bitStreamWriter.write(0, 8);
        bitStreamWriter.write(0, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(),
                nullptr, 0);

        ParserPCS parser;

        CPPUNIT_ASSERT_THROW(
                parser.parsePageCompositionSegment(*m_database, reader),
                ParserException);
    }

    void testNotEnoughData()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE =
                dvbsubdecoder::PageStateBits::MODE_CHANGE;

        bitStreamWriter.write(0, 8);
        bitStreamWriter.write(0, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved
        bitStreamWriter.write(1, 8); // region id. no position

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(),
                nullptr, 0);

        ParserPCS parser;

        CPPUNIT_ASSERT_THROW(
                parser.parsePageCompositionSegment(*m_database, reader),
                PesPacketReader::Exception);
    }

    void testStartSetNeeded()
    {
        const std::uint8_t PAGE_STATE1 =
                dvbsubdecoder::PageStateBits::NORMAL_CASE;
        const std::uint8_t PAGE_STATE2 =
                dvbsubdecoder::PageStateBits::ACQUISITION_POINT;

        BitStreamWriter bitStreamWriter1;

        bitStreamWriter1.write(0, 8);
        bitStreamWriter1.write(0, 4);
        bitStreamWriter1.write(PAGE_STATE1, 2);
        bitStreamWriter1.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter1.data(),
                bitStreamWriter1.size(), nullptr, 0);

        BitStreamWriter bitStreamWriter2;

        bitStreamWriter2.write(0, 8);
        bitStreamWriter2.write(1, 4);
        bitStreamWriter2.write(PAGE_STATE2, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(),
                bitStreamWriter2.size(), nullptr, 0);

        ParserPCS parser;
        const auto& page = m_database->getPage();

        parser.parsePageCompositionSegment(*m_database, reader1);

        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);

        parser.parsePageCompositionSegment(*m_database, reader2);

        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
    }

    void testAcquisitionPointSkipped()
    {
        // simulate previous page retrieval was completed
        m_database->commitPage();

        BitStreamWriter bitStreamWriter1;

        bitStreamWriter1.write(0, 8);
        bitStreamWriter1.write(0, 4);
        bitStreamWriter1.write(dvbsubdecoder::PageStateBits::ACQUISITION_POINT, 2);
        bitStreamWriter1.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter1.data(),
                bitStreamWriter1.size(), nullptr, 0);

        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        // NOTE: normally this assertion should be false, but there is a workaround
        // to treat acquisition points as mode change page composition segments.
        CPPUNIT_ASSERT(m_database->canAddRegion());

    }

    void testVersionNumberBoundaries()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        
        // Test minimum version (0)
        bitStreamWriter.write(0x50, 8); // timeout
        bitStreamWriter.write(0x00, 4); // version = 0
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        const auto& page1 = m_database->getPage();
        CPPUNIT_ASSERT(page1.getVersion() == 0);
        CPPUNIT_ASSERT(page1.getState() == Page::State::INCOMPLETE);

        // Test maximum version (15)
        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(0x60, 8); // timeout
        bitStreamWriter2.write(0x0F, 4); // version = 15
        bitStreamWriter2.write(PAGE_STATE, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(), bitStreamWriter2.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader2);

        const auto& page2 = m_database->getPage();
        CPPUNIT_ASSERT(page2.getVersion() == 15);
        CPPUNIT_ASSERT(page2.getState() == Page::State::INCOMPLETE);
    }

    void testTimeoutBoundaryValues()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        // Test minimum timeout (0)
        bitStreamWriter.write(0, 8); // timeout = 0
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        const auto& page1 = m_database->getPage();
        CPPUNIT_ASSERT(page1.getTimeout() == 0);

        // Test maximum timeout (255)
        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(255, 8); // timeout = 255
        bitStreamWriter2.write(PAGE_VERSION + 1, 4);
        bitStreamWriter2.write(PAGE_STATE, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(), bitStreamWriter2.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader2);

        const auto& page2 = m_database->getPage();
        CPPUNIT_ASSERT(page2.getTimeout() == 255);
    }

    void testRegionPositionBoundaries()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Region at (0,0)
        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(0, 16); // x = 0
        bitStreamWriter.write(0, 16); // y = 0

        // Region at maximum coordinates (65535, 65535)
        bitStreamWriter.write(2, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(65535, 16); // x = 65535
        bitStreamWriter.write(65535, 16); // y = 65535

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 0);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionY == 0);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionX == 65535);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionY == 65535);
    }

    void testVersionNumberWraparound()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;

        // First parse with version 15
        bitStreamWriter.write(0x50, 8); // timeout
        bitStreamWriter.write(0x0F, 4); // version = 15
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        const auto& page1 = m_database->getPage();
        CPPUNIT_ASSERT(page1.getVersion() == 15);

        // Then parse with version 0 (wraparound)
        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(0x60, 8); // timeout
        bitStreamWriter2.write(0x00, 4); // version = 0
        bitStreamWriter2.write(PAGE_STATE, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(), bitStreamWriter2.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader2);

        const auto& page2 = m_database->getPage();
        CPPUNIT_ASSERT(page2.getVersion() == 0);
        CPPUNIT_ASSERT(page2.getTimeout() == 0x60);
    }

    void testDuplicateRegionIds()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // First region with ID 5
        bitStreamWriter.write(5, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(100, 16); // x = 100
        bitStreamWriter.write(200, 16); // y = 200

        // Second region with same ID 5 but different position
        bitStreamWriter.write(5, 8); // regionId (duplicate)
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(300, 16); // x = 300
        bitStreamWriter.write(400, 16); // y = 400

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        CPPUNIT_ASSERT(page.getRegion(0).m_regionId == 5);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 100);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionY == 200);
        CPPUNIT_ASSERT(page.getRegion(1).m_regionId == 5);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionX == 300);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionY == 400);
    }

    void testReservedBitsIgnored()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0x03, 2); // reserved bits set to non-zero

        // Region with reserved byte set to non-zero
        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0xFF, 8); // reserved byte set to 0xFF
        bitStreamWriter.write(150, 16); // x = 150
        bitStreamWriter.write(250, 16); // y = 250

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 150);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionY == 250);
    }

    void testZeroLengthSegment()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        // Only header, no region data
        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getTimeout() == 0x30);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
    }

    void testPtsTimeHandling()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;
        const StcTime TEST_PTS(StcTimeType::LOW_32, 12345);

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        m_database->getStatus().setLastPts(TEST_PTS);

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getPts() == TEST_PTS);
    }

    void testPartialRegionData()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Partial region data (missing Y coordinate)
        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(100, 16); // x = 100
        // Missing Y coordinate

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;

        CPPUNIT_ASSERT_THROW(
                parser.parsePageCompositionSegment(*m_database, reader),
                PesPacketReader::Exception);
    }

    void testMaxRegionCountExceeded()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;
        const std::uint8_t REGION_LIMIT = dvbsubdecoder::MAX_SUPPORTED_REGIONS;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Add exactly the maximum number of regions + 1 extra
        for (std::uint8_t i = 0; i <= REGION_LIMIT; ++i)
        {
            bitStreamWriter.write(i, 8); // regionId
            bitStreamWriter.write(0, 8); // reserved
            bitStreamWriter.write(i * 10, 16); // x
            bitStreamWriter.write(i * 10 + 5, 16); // y
        }

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        // Should only store up to the maximum
        CPPUNIT_ASSERT(page.getRegionCount() == REGION_LIMIT);
        
        // Verify the stored regions are the first ones
        for (std::uint8_t i = 0; i < REGION_LIMIT; ++i)
        {
            CPPUNIT_ASSERT(page.getRegion(i).m_regionId == i);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionX == i * 10);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionY == i * 10 + 5);
        }
    }

    void testNormalCaseWithInvalidPage()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::NORMAL_CASE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Database starts with INVALID page state
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        // Should remain INVALID since NORMAL_CASE requires COMPLETE or TIMEDOUT state
        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
    }

    void testNormalCaseWithIncompletePage()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE1 = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_STATE2 = dvbsubdecoder::PageStateBits::NORMAL_CASE;
        const std::uint8_t PAGE_VERSION1 = 0x05;
        const std::uint8_t PAGE_VERSION2 = 0x06;

        // First, create an INCOMPLETE page
        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION1, 4);
        bitStreamWriter.write(PAGE_STATE1, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);

        // Now try NORMAL_CASE with INCOMPLETE page
        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(0x40, 8); // timeout
        bitStreamWriter2.write(PAGE_VERSION2, 4);
        bitStreamWriter2.write(PAGE_STATE2, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(), bitStreamWriter2.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader2);

        // Should remain INCOMPLETE since NORMAL_CASE requires COMPLETE or TIMEDOUT state
        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION1); // Should not have changed
    }

    void testMixedPageStateTransitions()
    {
        ParserPCS parser;

        // 1. Start with MODE_CHANGE
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x30, 8); // timeout
            bitStreamWriter.write(0x01, 4); // version = 1
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::MODE_CHANGE, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

        // 2. Complete the page
        m_database->getPage().finishParsing();
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // 3. Use NORMAL_CASE (should work with COMPLETE state)
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x40, 8); // timeout
            bitStreamWriter.write(0x02, 4); // version = 2
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::NORMAL_CASE, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 2);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

        // 4. Use ACQUISITION_POINT
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x50, 8); // timeout
            bitStreamWriter.write(0x03, 4); // version = 3
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::ACQUISITION_POINT, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 3);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }
    }

    void testEmptyDatabaseScenario()
    {
        // Create a fresh database
        auto client = std::make_unique<DecoderClientMock>();
        auto pixmapAllocator = std::make_unique<PixmapAllocator>(SPEC_VERSION, *client);
        auto database = std::make_unique<Database>(SPEC_VERSION, *pixmapAllocator);

        BitStreamWriter bitStreamWriter;
        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(100, 16); // x = 100
        bitStreamWriter.write(200, 16); // y = 200

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*database, reader);

        const auto& page = database->getPage();
        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        CPPUNIT_ASSERT(page.getRegion(0).m_regionId == 1);
    }

    void testRegionAddressOverflow()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Test with values that could cause overflow if not handled properly
        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(0xFFFF, 16); // x = maximum uint16
        bitStreamWriter.write(0xFFFF, 16); // y = maximum uint16

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 0xFFFF);
        CPPUNIT_ASSERT(page.getRegion(0).m_positionY == 0xFFFF);
    }

    void testConcurrentVersionProcessing()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        // First PCS with version 5
        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader1(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader1);

        CPPUNIT_ASSERT(m_database->getPage().getVersion() == PAGE_VERSION);

        // Second PCS with same version 5 (should be skipped)
        BitStreamWriter bitStreamWriter2;
        bitStreamWriter2.write(0x40, 8); // different timeout
        bitStreamWriter2.write(PAGE_VERSION, 4); // same version
        bitStreamWriter2.write(PAGE_STATE, 2);
        bitStreamWriter2.write(0, 2); // reserved

        PesPacketReader reader2(bitStreamWriter2.data(), bitStreamWriter2.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader2);

        // Should still have the original timeout
        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getTimeout() == 0x30);

        // Third PCS with different version 6 (should be processed)
        BitStreamWriter bitStreamWriter3;
        bitStreamWriter3.write(0x50, 8); // timeout
        bitStreamWriter3.write(PAGE_VERSION + 1, 4); // version = 6
        bitStreamWriter3.write(PAGE_STATE, 2);
        bitStreamWriter3.write(0, 2); // reserved

        PesPacketReader reader3(bitStreamWriter3.data(), bitStreamWriter3.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader3);

        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION + 1);
        CPPUNIT_ASSERT(page.getTimeout() == 0x50);
    }

    void testPageStateValidation()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        bitStreamWriter.write(1, 8); // regionId
        bitStreamWriter.write(0, 8); // reserved
        bitStreamWriter.write(100, 16); // x = 100
        bitStreamWriter.write(200, 16); // y = 200

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        // Verify page state is correctly set to INCOMPLETE after startParsing
        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getVersion() == PAGE_VERSION);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
    }

    void testAcquisitionPointWithIncompleteState()
    {
        ParserPCS parser;

        // First, create an INCOMPLETE page
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x30, 8); // timeout
            bitStreamWriter.write(0x01, 4); // version = 1
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::MODE_CHANGE, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

        // Now send ACQUISITION_POINT with INCOMPLETE state
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x40, 8); // timeout
            bitStreamWriter.write(0x02, 4); // version = 2
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::ACQUISITION_POINT, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            // Should reset and start new page
            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 2);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }
    }

    void testAcquisitionPointWithInvalidState()
    {
        ParserPCS parser;

        // Page starts in INVALID state
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        // Send ACQUISITION_POINT with INVALID state
        BitStreamWriter bitStreamWriter;
        bitStreamWriter.write(0x40, 8); // timeout
        bitStreamWriter.write(0x02, 4); // version = 2
        bitStreamWriter.write(dvbsubdecoder::PageStateBits::ACQUISITION_POINT, 2);
        bitStreamWriter.write(0, 2); // reserved

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        parser.parsePageCompositionSegment(*m_database, reader);

        // Should reset and start new page
        CPPUNIT_ASSERT(m_database->getPage().getVersion() == 2);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
    }

    void testModeChangeEpochReset()
    {
        ParserPCS parser;

        // First, create and complete a page
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x30, 8); // timeout
            bitStreamWriter.write(0x01, 4); // version = 1
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::MODE_CHANGE, 2);
            bitStreamWriter.write(0, 2); // reserved

            bitStreamWriter.write(1, 8); // regionId
            bitStreamWriter.write(0, 8); // reserved
            bitStreamWriter.write(100, 16); // x = 100
            bitStreamWriter.write(200, 16); // y = 200

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            m_database->getPage().finishParsing();
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);
        }

        // Now send MODE_CHANGE which should reset everything
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x50, 8); // timeout
            bitStreamWriter.write(0x02, 4); // version = 2
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::MODE_CHANGE, 2);
            bitStreamWriter.write(0, 2); // reserved

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);

            // Should have reset and started new page
            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 2);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
            CPPUNIT_ASSERT(m_database->getPage().getRegionCount() == 0);
        }
    }

    void testRegionDataIntegrity()
    {
        BitStreamWriter bitStreamWriter;

        const std::uint8_t PAGE_STATE = dvbsubdecoder::PageStateBits::MODE_CHANGE;
        const std::uint8_t PAGE_VERSION = 0x05;

        bitStreamWriter.write(0x30, 8); // timeout
        bitStreamWriter.write(PAGE_VERSION, 4);
        bitStreamWriter.write(PAGE_STATE, 2);
        bitStreamWriter.write(0, 2); // reserved

        // Add multiple regions with specific data patterns
        const std::uint8_t REGION_COUNT = 3;
        for (std::uint8_t i = 0; i < REGION_COUNT; ++i)
        {
            bitStreamWriter.write(10 + i, 8); // regionId (10, 11, 12)
            bitStreamWriter.write(0, 8); // reserved
            bitStreamWriter.write(i * 1000, 16); // x coordinates
            bitStreamWriter.write(i * 2000 + 500, 16); // y coordinates
        }

        PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
        ParserPCS parser;
        parser.parsePageCompositionSegment(*m_database, reader);

        const auto& page = m_database->getPage();
        CPPUNIT_ASSERT(page.getRegionCount() == REGION_COUNT);

        // Verify each region's data integrity
        for (std::uint8_t i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(page.getRegion(i).m_regionId == 10 + i);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionX == i * 1000);
            CPPUNIT_ASSERT(page.getRegion(i).m_positionY == i * 2000 + 500);
        }
    }

    void testMultiplePcsSequence()
    {
        ParserPCS parser;

        // Sequence: MODE_CHANGE -> complete -> NORMAL_CASE -> ACQUISITION_POINT
        
        // 1. MODE_CHANGE
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x30, 8);
            bitStreamWriter.write(0x01, 4);
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::MODE_CHANGE, 2);
            bitStreamWriter.write(0, 2);

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

        // 2. Complete the page
        m_database->getPage().finishParsing();
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // 3. NORMAL_CASE
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x40, 8);
            bitStreamWriter.write(0x02, 4);
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::NORMAL_CASE, 2);
            bitStreamWriter.write(0, 2);

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 2);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

        // 4. Complete again
        m_database->getPage().finishParsing();
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // 5. ACQUISITION_POINT
        {
            BitStreamWriter bitStreamWriter;
            bitStreamWriter.write(0x50, 8);
            bitStreamWriter.write(0x03, 4);
            bitStreamWriter.write(dvbsubdecoder::PageStateBits::ACQUISITION_POINT, 2);
            bitStreamWriter.write(0, 2);

            PesPacketReader reader(bitStreamWriter.data(), bitStreamWriter.size(), nullptr, 0);
            parser.parsePageCompositionSegment(*m_database, reader);
            CPPUNIT_ASSERT(m_database->getPage().getVersion() == 3);
            CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        }

    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserPCSTest);
