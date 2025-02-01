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

#include "Database.hpp"
#include "PixmapAllocator.hpp"

#include "Misc.hpp"
#include "DecoderClientMock.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;

class DatabaseTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DatabaseTest );
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testSameClut);
    CPPUNIT_TEST(testRenderingStates);
    CPPUNIT_TEST(testBadRegion);
    CPPUNIT_TEST(testNoPixmapMemory);
    CPPUNIT_TEST(testObjects);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(500 * 1024);
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));
    }

    void tearDown()
    {
        m_database.reset();
        m_pixmapAllocator.reset();
        m_client.reset();
    }

    void testBasic()
    {
        const auto REGION_COUNT = dvbsubdecoder::MAX_SUPPORTED_REGIONS;

        Database& database = *m_database;

        // database is empty by default
        CPPUNIT_ASSERT(database.getRegionCount() == 0);
        CPPUNIT_ASSERT_THROW(database.getRegionByIndex(0), std::range_error);
        CPPUNIT_ASSERT(database.getRegionById(0) == nullptr);
        CPPUNIT_ASSERT(database.getClutById(0) == nullptr);

        // nothing could be added as epoch and page is not started
        CPPUNIT_ASSERT(!database.canAddRegion());
        CPPUNIT_ASSERT(!database.addRegionAndClut(REGION_COUNT, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0xFF));

        // start epoch and page
        database.epochReset();
        CPPUNIT_ASSERT(!database.canAddRegion());
        database.getPage().startParsing(0xF, StcTime(), 0);
        CPPUNIT_ASSERT(database.canAddRegion());

        // add regions
        for (int i = 0; i <= REGION_COUNT; ++i)
        {
            if (i < REGION_COUNT)
            {
                CPPUNIT_ASSERT(database.canAddRegion());
            }
            else
            {
                CPPUNIT_ASSERT(!database.canAddRegion());
            }

            bool result = database.addRegionAndClut(i + 1, i + 2, i + 3,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, i + 4);
            if (i < REGION_COUNT)
            {
                CPPUNIT_ASSERT(result);

                auto regionByIndex = database.getRegionByIndex(i);
                auto regionById = database.getRegionById(i + 1);
                CPPUNIT_ASSERT(regionByIndex);
                CPPUNIT_ASSERT(regionById);
                CPPUNIT_ASSERT(regionByIndex == regionById);
            }
            else
            {
                CPPUNIT_ASSERT(!result);
            }
        }
        CPPUNIT_ASSERT(database.getRegionCount() == REGION_COUNT);

        // remove everything
        database.epochReset();

        // check add flag
        CPPUNIT_ASSERT(!database.canAddRegion());
        database.getPage().startParsing(0xF, StcTime(), 0);
        CPPUNIT_ASSERT(database.canAddRegion());
        database.getPage().finishParsing();
        CPPUNIT_ASSERT(!database.canAddRegion());
    }

    void testSameClut()
    {
        const auto REGION_COUNT = dvbsubdecoder::MAX_SUPPORTED_REGIONS;

        Database& database = *m_database;

        // database is empty by default
        CPPUNIT_ASSERT(database.getRegionCount() == 0);
        CPPUNIT_ASSERT_THROW(database.getRegionByIndex(0), std::range_error);
        CPPUNIT_ASSERT(database.getRegionById(0) == nullptr);
        CPPUNIT_ASSERT(database.getClutById(0) == nullptr);

        // nothing could be added as epoch and page is not started
        CPPUNIT_ASSERT(!database.canAddRegion());
        CPPUNIT_ASSERT(!database.addRegionAndClut(REGION_COUNT, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0xFF));

        // start epoch and page
        database.epochReset();
        CPPUNIT_ASSERT(!database.canAddRegion());
        database.getPage().startParsing(0xF, StcTime(), 0);
        CPPUNIT_ASSERT(database.canAddRegion());

        const std::uint8_t CLUT_ID = 0xEC;

        // add regions
        for (int i = 0; i < REGION_COUNT; ++i)
        {
            bool result = database.addRegionAndClut(i + 1, i + 2, i + 3,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);
            CPPUNIT_ASSERT(result);
        }
        CPPUNIT_ASSERT(database.getRegionCount() == REGION_COUNT);

        // get clut
        auto clut = database.getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            auto region = database.getRegionByIndex(i);
            CPPUNIT_ASSERT(region->getClut() == clut);
        }

        database.epochReset();

        CPPUNIT_ASSERT(database.getClutById(CLUT_ID) == nullptr);
    }

    void testBadRegion()
    {
        Database& database = *m_database;

        // start epoch and page
        database.epochReset();
        database.getPage().startParsing(0xF, StcTime(), 0);

        // region 0 is valid
        bool result = database.addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(result);

        // duplicated id
        result = database.addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(!result);

        // zero size
        result = database.addRegionAndClut(1, 0, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(!result);
        result = database.addRegionAndClut(1, 10, 0,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(!result);
        result = database.addRegionAndClut(1, 0, 0,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(!result);
    }

    void testNoPixmapMemory()
    {
        // nothing could be allocated
        tearDown();

        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(0);
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));

        Database& database = *m_database;

        // start epoch and page
        database.epochReset();
        database.getPage().startParsing(0xF, StcTime(), 0);

        bool result = database.addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(!result);
    }

    void testRenderingStates()
    {
        Database& database = *m_database;

        auto rendA1 = &database.getCurrentRenderingState();
        auto rendA2 = &database.getPreviousRenderingState();

        CPPUNIT_ASSERT(rendA1);
        CPPUNIT_ASSERT(rendA2);
        CPPUNIT_ASSERT(rendA1 != rendA2);

        database.swapRenderingStates();

        auto rendB1 = &database.getCurrentRenderingState();
        auto rendB2 = &database.getPreviousRenderingState();

        CPPUNIT_ASSERT(rendA1 == rendB2);
        CPPUNIT_ASSERT(rendA2 == rendB1);

        database.swapRenderingStates();

        auto rendC1 = &database.getCurrentRenderingState();
        auto rendC2 = &database.getPreviousRenderingState();

        CPPUNIT_ASSERT(rendA1 == rendC1);
        CPPUNIT_ASSERT(rendA2 == rendC2);
    }

    void testObjects()
    {
        Database& database = *m_database;

        // start epoch and page
        database.epochReset();
        database.getPage().startParsing(0xF, StcTime(), 0);

        // region 0 is valid
        bool result = database.addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(result);

        auto region0 = database.getRegionById(0);
        CPPUNIT_ASSERT(region0);

        // region 1 is valid
        result = database.addRegionAndClut(1, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(result);

        auto region1 = database.getRegionById(1);
        CPPUNIT_ASSERT(region1);

        // add single
        CPPUNIT_ASSERT(database.addRegionObject(region0, 0, 0, 0));

        // check if added
        auto first = region0->getFirstObject();
        CPPUNIT_ASSERT(first);
        CPPUNIT_ASSERT(region0->getNextObject(first) == nullptr);

        // add until limit
        while (database.addRegionObject(region1, 0, 0, 0))
        {
            // noop
        }

        // remove the single
        database.removeRegionObjects(region0);

        // add the single to other region
        CPPUNIT_ASSERT(database.addRegionObject(region1, 0, 0, 0));

        // no more available
        CPPUNIT_ASSERT(!database.addRegionObject(region0, 0, 0, 0));

        // remove everything on epoch start
        database.epochReset();
        database.getPage().startParsing(0xF, StcTime(), 0);

        // region 2 is valid
        result = database.addRegionAndClut(2, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(result);

        auto region2 = database.getRegionById(2);
        CPPUNIT_ASSERT(region2);

        // try to add again
        while (database.addRegionObject(region2, 0, 0, 0))
        {
            // noop
        }
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTest);
