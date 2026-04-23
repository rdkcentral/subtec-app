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
    CPPUNIT_TEST(testStatusManagement);
    CPPUNIT_TEST(testDisplayManagement);
    CPPUNIT_TEST(testPageManagement);
    CPPUNIT_TEST(testCommitPage);
    CPPUNIT_TEST(testSpecificationVersions);
    CPPUNIT_TEST(testRegionDepthVariants);
    CPPUNIT_TEST(testBoundaryValues);
    CPPUNIT_TEST(testMemoryAllocationEdgeCases);
    CPPUNIT_TEST(testComplexObjectManagement);
    CPPUNIT_TEST(testStateConsistency);
    CPPUNIT_TEST(testErrorRecovery);
    CPPUNIT_TEST(testMaxCapacityStress);
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

    void testStatusManagement()
    {
        Database& database = *m_database;


        auto& status = database.getStatus();
        CPPUNIT_ASSERT(database.getRegionCount() == 0);

        // Test that status persists across operations
        database.epochReset();
        auto& statusAfterReset = database.getStatus();
        CPPUNIT_ASSERT(&status == &statusAfterReset); // Same object reference

        // Test status with different specification version
        tearDown();
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(500 * 1024);
        m_pixmapAllocator.reset(new PixmapAllocator(Specification::VERSION_1_2_1, *m_client));
        m_database.reset(new Database(Specification::VERSION_1_2_1, *m_pixmapAllocator));

        auto& status121 = m_database->getStatus();
        // zero regions until parsing starts.
        CPPUNIT_ASSERT(m_database->getRegionCount() == 0);
    }

    void testDisplayManagement()
    {
        Database& database = *m_database;

        // Access display objects (references are inherently non-null).
        auto& currentDisplay = database.getCurrentDisplay();
        auto& parsedDisplay = database.getParsedDisplay();

        // Verify they are different objects
        CPPUNIT_ASSERT(&currentDisplay != &parsedDisplay);

        // Test display persistence across epoch reset
        database.epochReset();
        auto& currentAfterReset = database.getCurrentDisplay();
        auto& parsedAfterReset = database.getParsedDisplay();
        
        CPPUNIT_ASSERT(&currentDisplay == &currentAfterReset);
        CPPUNIT_ASSERT(&parsedDisplay == &parsedAfterReset);
    }

    void testPageManagement()
    {
        Database& database = *m_database;

        // Page reference acquisition (non-null by design)
        auto& page = database.getPage();

        // Test page state management
        database.epochReset();
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::INVALID);

        // Test page parsing lifecycle
        page.startParsing(0x10, StcTime(), 5);
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::INCOMPLETE);

        page.finishParsing();
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::COMPLETE);

        // Test page persistence
        auto& pageAfterOps = database.getPage();
        CPPUNIT_ASSERT(&page == &pageAfterOps);
    }

    void testCommitPage()
    {
        Database& database = *m_database;

        // Setup for page commit testing
        database.epochReset();
        database.getPage().startParsing(0x15, StcTime(), 3);
        
        // Test that regions can be added before commit
        CPPUNIT_ASSERT(database.canAddRegion());
        
        bool result = database.addRegionAndClut(1, 20, 30,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        CPPUNIT_ASSERT(result);
        
        // Commit the page
        database.commitPage();
        
        // Test that regions cannot be added after commit (no longer epoch start)
        CPPUNIT_ASSERT(!database.canAddRegion());
        
        result = database.addRegionAndClut(2, 20, 30,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 2);
        CPPUNIT_ASSERT(!result);

        // Test that existing regions remain accessible
        auto region = database.getRegionById(1);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(database.getRegionCount() == 1);
    }

    void testSpecificationVersions()
    {
        // Test VERSION_1_2_1
        tearDown();
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(500 * 1024);
        m_pixmapAllocator.reset(new PixmapAllocator(Specification::VERSION_1_2_1, *m_client));
        m_database.reset(new Database(Specification::VERSION_1_2_1, *m_pixmapAllocator));

        Database& database121 = *m_database;
        
        // Test basic functionality with VERSION_1_2_1
        database121.epochReset();
        database121.getPage().startParsing(0x20, StcTime(), 1);
        
        bool result = database121.addRegionAndClut(10, 15, 25,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, 10);
        CPPUNIT_ASSERT(result);
        
        auto region = database121.getRegionById(10);
        CPPUNIT_ASSERT(region);
        
        // Test rendering states with different spec version
        auto& currentState = database121.getCurrentRenderingState();
        auto& previousState = database121.getPreviousRenderingState();
        CPPUNIT_ASSERT(&currentState != &previousState);
        
        database121.swapRenderingStates();
        auto& swappedCurrent = database121.getCurrentRenderingState();
        CPPUNIT_ASSERT(&swappedCurrent == &previousState);

        // Test VERSION_1_3_1 (restore original)
        tearDown();
        setUp();
        Database& database131 = *m_database;
        
        database131.epochReset();
        database131.getPage().startParsing(0x25, StcTime(), 2);
        
        result = database131.addRegionAndClut(20, 40, 50,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 20);
        CPPUNIT_ASSERT(result);
        
        auto region131 = database131.getRegionById(20);
        CPPUNIT_ASSERT(region131);
    }

    void testRegionDepthVariants()
    {
        Database& database = *m_database;
        
        database.epochReset();
        database.getPage().startParsing(0x30, StcTime(), 4);

        // Test DEPTH_2BIT
        bool result = database.addRegionAndClut(1, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, 1);
        CPPUNIT_ASSERT(result);

        // Test DEPTH_4BIT
        result = database.addRegionAndClut(2, 15, 15,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, 2);
        CPPUNIT_ASSERT(result);

        // Test DEPTH_8BIT
        result = database.addRegionAndClut(3, 20, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 3);
        CPPUNIT_ASSERT(result);

        // Test mixed depths sharing same CLUT
        result = database.addRegionAndClut(4, 12, 12,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, 1); // Same CLUT as first
        CPPUNIT_ASSERT(result);

        // Verify all regions exist
        CPPUNIT_ASSERT(database.getRegionById(1));
        CPPUNIT_ASSERT(database.getRegionById(2));
        CPPUNIT_ASSERT(database.getRegionById(3));
        CPPUNIT_ASSERT(database.getRegionById(4));
        CPPUNIT_ASSERT(database.getRegionCount() == 4);

        // Test that regions with different depths can share CLUTs
        auto clut1 = database.getClutById(1);
        auto region1 = database.getRegionById(1);
        auto region4 = database.getRegionById(4);
        CPPUNIT_ASSERT(clut1);
        CPPUNIT_ASSERT(region1->getClut() == clut1);
        CPPUNIT_ASSERT(region4->getClut() == clut1);
    }

    void testBoundaryValues()
    {
        Database& database = *m_database;
        
        database.epochReset();
        database.getPage().startParsing(0x40, StcTime(), 6);

        // Test region ID boundaries (uint8_t: 0-255)
        bool result = database.addRegionAndClut(0, 5, 5,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        CPPUNIT_ASSERT(result);

        result = database.addRegionAndClut(255, 5, 5,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 255);
        CPPUNIT_ASSERT(result);

        // Test CLUT ID boundaries (uint8_t: 0-255)
        auto clut0 = database.getClutById(0);
        auto clut255 = database.getClutById(255);
        CPPUNIT_ASSERT(clut0);
        CPPUNIT_ASSERT(clut255);

        // Test dimension boundaries (positive values only)
        result = database.addRegionAndClut(1, 1, 1,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        CPPUNIT_ASSERT(result);

        // Test large dimensions (within memory limits)
        result = database.addRegionAndClut(2, 100, 100,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 2);
        CPPUNIT_ASSERT(result);

        // Test object position boundaries
        auto region = database.getRegionById(1);
        CPPUNIT_ASSERT(region);

        // Test with boundary object positions
        CPPUNIT_ASSERT(database.addRegionObject(region, 0, -2147483648, -2147483648)); // INT32_MIN
        CPPUNIT_ASSERT(database.addRegionObject(region, 65535, 2147483647, 2147483647)); // UINT16_MAX, INT32_MAX
        CPPUNIT_ASSERT(database.addRegionObject(region, 32768, 0, 0)); // Mid-range values

        // Validate stored extreme coordinate/object ID values via safe iteration.
        {
            const auto* obj = region->getFirstObject();
            bool foundMin = false, foundMax = false, foundMid = false;
            int safety = 0;
            while (obj && safety < 10)
            {
                if (obj->m_objectId == 0)
                {
                    CPPUNIT_ASSERT(obj->m_positionX == -2147483648);
                    CPPUNIT_ASSERT(obj->m_positionY == -2147483648);
                    foundMin = true;
                }
                else if (obj->m_objectId == 65535)
                {
                    CPPUNIT_ASSERT(obj->m_positionX == 2147483647);
                    CPPUNIT_ASSERT(obj->m_positionY == 2147483647);
                    foundMax = true;
                }
                else if (obj->m_objectId == 32768)
                {
                    CPPUNIT_ASSERT(obj->m_positionX == 0);
                    CPPUNIT_ASSERT(obj->m_positionY == 0);
                    foundMid = true;
                }
                obj = region->getNextObject(obj);
                ++safety;
            }
            CPPUNIT_ASSERT(foundMin && foundMax && foundMid);
        }
    }

    void testMemoryAllocationEdgeCases()
    {
        // Test CLUT limit exhaustion
        tearDown();
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(500 * 1024);
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));

        Database& database = *m_database;
        database.epochReset();
        database.getPage().startParsing(0x50, StcTime(), 7);

        // Fill up CLUT pool (MAX_SUPPORTED_CLUTS = 16)
        const auto MAX_CLUTS = dvbsubdecoder::MAX_SUPPORTED_CLUTS;
        for (int i = 0; i < MAX_CLUTS; ++i)
        {
            bool result = database.addRegionAndClut(i, 5, 5,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, i);
            CPPUNIT_ASSERT(result);
        }

        // Try to add one more with a new CLUT - expected to fail because CLUT pool is full (new CLUT id cannot be allocated).
        bool result = database.addRegionAndClut(MAX_CLUTS, 5, 5,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, MAX_CLUTS);
        CPPUNIT_ASSERT(!result);

        // Attempt reusing an existing CLUT after CLUT exhaustion. Test currently expects failure (implementation coupling: either region addition disallowed after prior allocation pattern or other internal limit hit).
        // If a future change allows this to succeed, revisit this assertion and separate CLUT vs region limit conditions explicitly.
        result = database.addRegionAndClut(MAX_CLUTS, 5, 5,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0); // Reuse CLUT 0
        CPPUNIT_ASSERT(!result); // Should fail because region limit reached

        // Test partial memory allocation failure - use a more robust approach
        tearDown();
        m_client.reset(new DecoderClientMock());
        
        // First test - establish that allocation works with generous limit
        m_client->setAllocLimit(100 * 1024); // 100KB - should definitely work
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));

        Database& testDatabase = *m_database;
        testDatabase.epochReset();
        testDatabase.getPage().startParsing(0x55, StcTime(), 8);

        // Verify that small region works with generous memory
        result = testDatabase.addRegionAndClut(1, 1, 1,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        CPPUNIT_ASSERT(result); // This should always succeed
        
        // Now test failure case with very restrictive limit
        tearDown();
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(10); // Very small limit - should definitely fail
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));

        Database& limitedDatabase = *m_database;
        limitedDatabase.epochReset();
        limitedDatabase.getPage().startParsing(0x56, StcTime(), 9);

        // Region creation should fail with very limited memory (10 bytes)
        result = limitedDatabase.addRegionAndClut(1, 1, 1,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        CPPUNIT_ASSERT(!result); // This should fail due to insufficient memory
    }

    void testComplexObjectManagement()
    {
        Database& database = *m_database;
        
        database.epochReset();
        database.getPage().startParsing(0x60, StcTime(), 9);

        // Create regions for complex object testing
        bool result = database.addRegionAndClut(1, 20, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        CPPUNIT_ASSERT(result);

        result = database.addRegionAndClut(2, 15, 15,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, 2);
        CPPUNIT_ASSERT(result);

        auto region1 = database.getRegionById(1);
        auto region2 = database.getRegionById(2);
        CPPUNIT_ASSERT(region1);
        CPPUNIT_ASSERT(region2);

        // Test object positioning edge cases
        CPPUNIT_ASSERT(database.addRegionObject(region1, 100, -1000, -2000)); // Negative positions
        CPPUNIT_ASSERT(database.addRegionObject(region1, 200, 0, 0)); // Zero positions
        CPPUNIT_ASSERT(database.addRegionObject(region1, 300, 1000000, 2000000)); // Large positions

        // Test object ID boundaries
        CPPUNIT_ASSERT(database.addRegionObject(region2, 0, 10, 20)); // Min object ID
        CPPUNIT_ASSERT(database.addRegionObject(region2, 65535, 30, 40)); // Max object ID (uint16_t)

        // Test cross-region object isolation
        auto obj1 = region1->getFirstObject();
        auto obj2 = region2->getFirstObject();
        CPPUNIT_ASSERT(obj1);
        CPPUNIT_ASSERT(obj2);
        CPPUNIT_ASSERT(obj1 != obj2); // Different objects in different regions

        // Test object removal and pool management
        database.removeRegionObjects(region1);
        CPPUNIT_ASSERT(region1->getFirstObject() == nullptr);
        CPPUNIT_ASSERT(region2->getFirstObject() != nullptr); // Other region unaffected

        // Test that removed objects can be reallocated
        CPPUNIT_ASSERT(database.addRegionObject(region1, 400, 50, 60));
        
        // Test object pool exhaustion and recovery
        int objectCount = 0;
        while (database.addRegionObject(region1, objectCount, objectCount, objectCount))
        {
            objectCount++;
            if (objectCount > 300) break; // Safety limit
        }
        CPPUNIT_ASSERT(objectCount > 0); // Some objects were added
        
        // Test that removing objects makes pool available again
        database.removeRegionObjects(region1);
        CPPUNIT_ASSERT(database.addRegionObject(region1, 500, 70, 80));
    }

    void testStateConsistency()
    {
        Database& database = *m_database;

        // Test multiple epoch resets
        for (int i = 0; i < 5; ++i)
        {
            database.epochReset();
            CPPUNIT_ASSERT(database.getRegionCount() == 0);
            CPPUNIT_ASSERT(!database.canAddRegion());
            
            database.getPage().startParsing(0x70 + i, StcTime(), i);
            CPPUNIT_ASSERT(database.canAddRegion());
            
            bool result = database.addRegionAndClut(i, 10, 10,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, i);
            CPPUNIT_ASSERT(result);
            CPPUNIT_ASSERT(database.getRegionCount() == 1);
        }

        // Test rendering state consistency across operations
        auto& state1 = database.getCurrentRenderingState();
        auto& state2 = database.getPreviousRenderingState();
        
        // Multiple swaps should maintain consistency
        for (int i = 0; i < 10; ++i)
        {
            database.swapRenderingStates();
            auto& currentAfterSwap = database.getCurrentRenderingState();
            auto& previousAfterSwap = database.getPreviousRenderingState();
            
            // States should remain distinct
            CPPUNIT_ASSERT(&currentAfterSwap != &previousAfterSwap);
            
            // References should be consistent
            if ((i + 1) % 2 == 1) // After odd number of swaps
            {
                CPPUNIT_ASSERT(&currentAfterSwap == &state2);
                CPPUNIT_ASSERT(&previousAfterSwap == &state1);
            }
            else // After even number of swaps (back to original)
            {
                CPPUNIT_ASSERT(&currentAfterSwap == &state1);
                CPPUNIT_ASSERT(&previousAfterSwap == &state2);
            }
        }

        // Test page state persistence
        database.epochReset();
        auto& page = database.getPage();
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::INVALID);
        
        page.startParsing(0x80, StcTime(), 10);
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::INCOMPLETE);
        
        // Add some regions
        database.addRegionAndClut(1, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 1);
        
        // Page state should remain consistent
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::INCOMPLETE);
        
        page.finishParsing();
        CPPUNIT_ASSERT(page.getState() == dvbsubdecoder::Page::State::COMPLETE);
    }

    void testErrorRecovery()
    {
        Database& database = *m_database;
        
        database.epochReset();
        database.getPage().startParsing(0x90, StcTime(), 11);

        // Test duplicate region ID error recovery
        bool result = database.addRegionAndClut(5, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 5);
        CPPUNIT_ASSERT(result);

        // Attempt to add duplicate - should fail gracefully
        result = database.addRegionAndClut(5, 20, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 6);
        CPPUNIT_ASSERT(!result);

        // Database should remain functional
        CPPUNIT_ASSERT(database.getRegionCount() == 1);
        CPPUNIT_ASSERT(database.getRegionById(5) != nullptr);
        CPPUNIT_ASSERT(database.canAddRegion());

        // Should be able to add different region
        result = database.addRegionAndClut(6, 15, 15,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 6);
        CPPUNIT_ASSERT(result);

        // Test invalid dimension error recovery
        result = database.addRegionAndClut(7, 0, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 7);
        CPPUNIT_ASSERT(!result);

        // If parameter types are unsigned, this relies on internal range checks detecting overflowed large values after implicit conversion.
        result = database.addRegionAndClut(7, -5, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 7);
        CPPUNIT_ASSERT(!result);

        // Database should still be functional after errors
        CPPUNIT_ASSERT(database.getRegionCount() == 2);
        CPPUNIT_ASSERT(database.canAddRegion());

        // Test operation sequencing robustness
        database.commitPage();
        CPPUNIT_ASSERT(!database.canAddRegion());

        // Attempt operations in wrong state - should fail gracefully
        result = database.addRegionAndClut(8, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 8);
        CPPUNIT_ASSERT(!result);

        // Database should remain consistent
        CPPUNIT_ASSERT(database.getRegionCount() == 2);
        auto existingRegion = database.getRegionById(5);
        CPPUNIT_ASSERT(existingRegion != nullptr);
    }

    void testMaxCapacityStress()
    {
        Database& database = *m_database;
        
        database.epochReset();
        database.getPage().startParsing(0xA0, StcTime(), 12);

        const auto MAX_REGIONS = dvbsubdecoder::MAX_SUPPORTED_REGIONS;
        const auto MAX_OBJECTS = dvbsubdecoder::MAX_SUPPORTED_OBJECTS;

        // Fill to maximum region capacity
        for (std::size_t i = 0; i < MAX_REGIONS; ++i)
        {
            bool result = database.addRegionAndClut(static_cast<std::uint8_t>(i), 5, 5,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 
                    static_cast<std::uint8_t>(i % 8)); // Reuse some CLUTs
            CPPUNIT_ASSERT(result);
        }

        CPPUNIT_ASSERT(database.getRegionCount() == MAX_REGIONS);
        CPPUNIT_ASSERT(!database.canAddRegion());

        // Verify all regions are accessible
        for (std::size_t i = 0; i < MAX_REGIONS; ++i)
        {
            auto region = database.getRegionById(static_cast<std::uint8_t>(i));
            CPPUNIT_ASSERT(region != nullptr);
            auto regionByIndex = database.getRegionByIndex(i);
            CPPUNIT_ASSERT(regionByIndex == region);
        }

        // Test object capacity across multiple regions
        auto firstRegion = database.getRegionById(0);
        CPPUNIT_ASSERT(firstRegion);

        // Fill object pool to capacity
        std::size_t objectsAdded = 0;
        for (std::uint16_t objId = 0; objectsAdded < MAX_OBJECTS; ++objId)
        {
            if (database.addRegionObject(firstRegion, objId, objId % 1000, objId % 2000))
            {
                objectsAdded++;
            }
            else
            {
                break;
            }
        }

        CPPUNIT_ASSERT(objectsAdded == MAX_OBJECTS);

        // Verify no more objects can be added to any region
        auto secondRegion = database.getRegionById(1);
        CPPUNIT_ASSERT(!database.addRegionObject(secondRegion, 65534, 0, 0));

        // Test that removing objects from one region makes them available to others
        database.removeRegionObjects(firstRegion);
        CPPUNIT_ASSERT(database.addRegionObject(secondRegion, 65533, 100, 200));

        // Test maximum capacity with different object distributions
        database.removeRegionObjects(secondRegion);
        
        // Distribute objects across multiple regions
        std::size_t objectsPerRegion = MAX_OBJECTS / MAX_REGIONS;
        std::size_t totalObjectsAdded = 0;
        
        for (std::size_t regionIdx = 0; regionIdx < MAX_REGIONS && totalObjectsAdded < MAX_OBJECTS; ++regionIdx)
        {
            auto region = database.getRegionByIndex(regionIdx);
            CPPUNIT_ASSERT(region);
            
            for (std::size_t objIdx = 0; objIdx < objectsPerRegion && totalObjectsAdded < MAX_OBJECTS; ++objIdx)
            {
                std::uint16_t objId = static_cast<std::uint16_t>(totalObjectsAdded);
                if (database.addRegionObject(region, objId, objId % 500, objId % 300))
                {
                    totalObjectsAdded++;
                }
                else
                {
                    break;
                }
            }
        }

        CPPUNIT_ASSERT(totalObjectsAdded == MAX_OBJECTS);

        // Verify that the system handles maximum capacity gracefully
        auto lastRegion = database.getRegionById(static_cast<std::uint8_t>(MAX_REGIONS - 1));
        CPPUNIT_ASSERT(!database.addRegionObject(lastRegion, 65532, 0, 0));

        // Test epoch reset clears everything properly at maximum capacity
        database.epochReset();
        CPPUNIT_ASSERT(database.getRegionCount() == 0);
        CPPUNIT_ASSERT(!database.canAddRegion());

        // Verify system is fully functional after capacity stress test
        database.getPage().startParsing(0xB0, StcTime(), 13);
        CPPUNIT_ASSERT(database.canAddRegion());
        
        bool result = database.addRegionAndClut(100, 25, 35,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 100);
        CPPUNIT_ASSERT(result);
        
        auto testRegion = database.getRegionById(100);
        CPPUNIT_ASSERT(testRegion);
        CPPUNIT_ASSERT(database.addRegionObject(testRegion, 12345, 50, 75));
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTest);
