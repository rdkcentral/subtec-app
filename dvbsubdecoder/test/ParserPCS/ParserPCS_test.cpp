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

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserPCSTest);
