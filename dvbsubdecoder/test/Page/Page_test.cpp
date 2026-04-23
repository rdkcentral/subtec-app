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

#include "Misc.hpp"

#include "Page.hpp"
#include "Consts.hpp"

using dvbsubdecoder::Page;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class PageTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PageTest );
    CPPUNIT_TEST(testDefaults);
    CPPUNIT_TEST(testStateMachine);
    CPPUNIT_TEST(testRegions);
    CPPUNIT_TEST(testVersionBoundaryValues);
    CPPUNIT_TEST(testTimeoutBoundaryValues);
    CPPUNIT_TEST(testPtsWithDifferentTimeTypes);
    CPPUNIT_TEST(testMultipleStartParsingCalls);
    CPPUNIT_TEST(testFinishParsingSuccessPath);
    CPPUNIT_TEST(testFinishParsingEmptyPage);
    CPPUNIT_TEST(testStatePersistenceAfterFinishParsing);
    CPPUNIT_TEST(testSetTimedOutTransitionsAndRegionClearing);
    CPPUNIT_TEST(testRegionCapacityBoundaryTesting);
    CPPUNIT_TEST(testRegionCoordinateExtremeValues);
    CPPUNIT_TEST(testRegionIdBoundaryValues);
    CPPUNIT_TEST(testGetRegionBoundaryConditions);
    CPPUNIT_TEST(testDataPersistenceAcrossTransitions);
    CPPUNIT_TEST(testResetFromAllStates);
    CPPUNIT_TEST(testInvalidStateTransitions);
    CPPUNIT_TEST(testAddRegionFromNonIncompleteStates);
    CPPUNIT_TEST(testMultipleRegionsSameId);
    CPPUNIT_TEST(testRegionRetrievalOrder);
    CPPUNIT_TEST(testCapacityReachedBehavior);
    CPPUNIT_TEST(testEmptyPageRegionAccess);
    CPPUNIT_TEST(testFullLifecycleIntegration);
    CPPUNIT_TEST(testMultiStageParsingSimulation);
    CPPUNIT_TEST(testRegionOverflowBehavior);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testDefaults()
    {
        Page page;

        CPPUNIT_ASSERT(page.getPts() == StcTime());
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        CPPUNIT_ASSERT(page.getTimeout() == 0);
        CPPUNIT_ASSERT(page.getVersion() == dvbsubdecoder::INVALID_VERSION);

        const std::uint8_t SPECIFIC_VERSION = 6;
        const StcTime SPECIFIC_PTS(StcTimeType::LOW_32, 16);
        const std::uint8_t SPECIFIC_TIMEOUT = 10;

        page.startParsing(SPECIFIC_VERSION, SPECIFIC_PTS, SPECIFIC_TIMEOUT);
        page.addRegion(0, 0, 0);

        CPPUNIT_ASSERT(page.getPts() == SPECIFIC_PTS);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getTimeout() == SPECIFIC_TIMEOUT);
        CPPUNIT_ASSERT(page.getVersion() == SPECIFIC_VERSION);

        page.reset();

        CPPUNIT_ASSERT(page.getPts() == StcTime());
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        CPPUNIT_ASSERT(page.getTimeout() == 0);
        CPPUNIT_ASSERT(page.getVersion() == dvbsubdecoder::INVALID_VERSION);
    }

    void testStateMachine()
    {
        const std::uint8_t VERSION = 1;
        const StcTime PTS;
        const std::uint8_t TIMEOUT = 1;

        Page page;

        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);

        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);

        page.reset();
        page.startParsing(VERSION, PTS, TIMEOUT);
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);
        page.startParsing(VERSION, PTS, TIMEOUT);

        page.reset();
        page.startParsing(VERSION, PTS, TIMEOUT);
        page.finishParsing();

        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        page.startParsing(VERSION, PTS, TIMEOUT);

        page.reset();
        page.startParsing(VERSION, PTS, TIMEOUT);
        page.finishParsing();
        page.setTimedOut();

        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);
        page.startParsing(VERSION, PTS, TIMEOUT);

        page.reset();
        page.startParsing(VERSION, PTS, TIMEOUT);
    }

    void testRegions()
    {
        Page page;

        CPPUNIT_ASSERT_THROW(page.addRegion(0, 0, 0), std::logic_error);

        page.startParsing(0, StcTime(), 0);

        auto maxRegions = 0;
        for (maxRegions = 0; ; ++maxRegions)
        {
            CPPUNIT_ASSERT(page.getRegionCount() == maxRegions);

            if (!page.addRegion(maxRegions, maxRegions * 2, maxRegions * 3))
            {
                break;
            }
        }
        CPPUNIT_ASSERT(page.getRegionCount() == maxRegions);

        for (auto i = 0; i < maxRegions; ++i)
        {
            const auto& region = page.getRegion(i);
            CPPUNIT_ASSERT(region.m_regionId == i);
            CPPUNIT_ASSERT(region.m_positionX == i * 2);
            CPPUNIT_ASSERT(region.m_positionY == i * 3);
        }

        CPPUNIT_ASSERT_THROW(page.getRegion(maxRegions), std::range_error);

        page.finishParsing();
        page.setTimedOut();

        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        CPPUNIT_ASSERT_THROW(page.getRegion(0), std::range_error);
        CPPUNIT_ASSERT_THROW(page.getRegion(maxRegions), std::range_error);

        page.reset();
        page.startParsing(0,  StcTime(), 0);
        page.addRegion(0, 0, 0);

        CPPUNIT_ASSERT(page.getRegionCount() == 1);

        page.reset();

        CPPUNIT_ASSERT(page.getRegionCount() == 0);
    }

    void testVersionBoundaryValues()
    {
        Page page;
        const StcTime pts;
        const std::uint32_t timeout = 10;
        
        // Test minimum version (0)
        page.startParsing(0, pts, timeout);
        CPPUNIT_ASSERT(page.getVersion() == 0);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        
        page.reset();
        
        // Test maximum version (255)
        page.startParsing(255, pts, timeout);
        CPPUNIT_ASSERT(page.getVersion() == 255);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
    }

    void testTimeoutBoundaryValues()
    {
        Page page;
        const std::uint8_t version = 1;
        const StcTime pts;
        
        // Test minimum timeout (0)
        page.startParsing(version, pts, 0);
        CPPUNIT_ASSERT(page.getTimeout() == 0);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        
        page.reset();
        
        // Test maximum timeout (UINT32_MAX)
        page.startParsing(version, pts, UINT32_MAX);
        CPPUNIT_ASSERT(page.getTimeout() == UINT32_MAX);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
    }

    void testPtsWithDifferentTimeTypes()
    {
        Page page;
        const std::uint8_t version = 1;
        const std::uint32_t timeout = 10;
        
        // Test with LOW_32 time type
        StcTime ptsLow32(StcTimeType::LOW_32, 12345);
        page.startParsing(version, ptsLow32, timeout);
        CPPUNIT_ASSERT(page.getPts() == ptsLow32);
        
        page.reset();
        
        // Test with HIGH_32 time type
        StcTime ptsHigh32(StcTimeType::HIGH_32, 67890);
        page.startParsing(version, ptsHigh32, timeout);
        CPPUNIT_ASSERT(page.getPts() == ptsHigh32);
    }

    void testMultipleStartParsingCalls()
    {
        Page page;
        
        // First startParsing call
        page.startParsing(1, StcTime(StcTimeType::LOW_32, 100), 5);
        page.addRegion(1, 10, 20);
        CPPUNIT_ASSERT(page.getVersion() == 1);
        CPPUNIT_ASSERT(page.getTimeout() == 5);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        
        // Second startParsing call should reset and update all fields
        page.startParsing(2, StcTime(StcTimeType::HIGH_32, 200), 15);
        CPPUNIT_ASSERT(page.getVersion() == 2);
        CPPUNIT_ASSERT(page.getTimeout() == 15);
        CPPUNIT_ASSERT(page.getRegionCount() == 0); // Regions should be cleared
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
    }

    void testFinishParsingSuccessPath()
    {
        Page page;
        const std::uint8_t version = 5;
        const StcTime pts(StcTimeType::LOW_32, 1000);
        const std::uint32_t timeout = 30;
        
        page.startParsing(version, pts, timeout);
        page.addRegion(1, 100, 200);
        
        // Verify state before finishing
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        
        // Finish parsing
        page.finishParsing();
        
        // Verify state transition and data preservation
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
    }

    void testFinishParsingEmptyPage()
    {
        Page page;
        const std::uint8_t version = 42;
        const StcTime pts(StcTimeType::LOW_32, 123);
        const std::uint32_t timeout = 5;

        page.startParsing(version, pts, timeout);
        // Intentionally do not add any regions
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);

        page.finishParsing();

        // Should transition to COMPLETE with zero regions preserved
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
    }

    void testStatePersistenceAfterFinishParsing()
    {
        Page page;
        const std::uint8_t version = 7;
        const StcTime pts(StcTimeType::HIGH_32, 2000);
        const std::uint32_t timeout = 45;
        
        page.startParsing(version, pts, timeout);
        page.addRegion(2, 150, 250);
        page.addRegion(3, 300, 350);
        page.finishParsing();
        
        // All data should remain unchanged after finishParsing
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        
        const auto& region1 = page.getRegion(0);
        CPPUNIT_ASSERT(region1.m_regionId == 2);
        CPPUNIT_ASSERT(region1.m_positionX == 150);
        CPPUNIT_ASSERT(region1.m_positionY == 250);
    }

    void testSetTimedOutTransitionsAndRegionClearing()
    {
        Page page;
        
        page.startParsing(1, StcTime(), 10);
        page.addRegion(1, 10, 20);
        page.addRegion(2, 30, 40);
        page.finishParsing();
        
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        
        // Set timed out should clear regions
        page.setTimedOut();
        
        CPPUNIT_ASSERT(page.getState() == Page::State::TIMEDOUT);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        
        // Version, PTS, and timeout should remain unchanged
        CPPUNIT_ASSERT(page.getVersion() == 1);
        CPPUNIT_ASSERT(page.getTimeout() == 10);
    }

    void testRegionCapacityBoundaryTesting()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Add regions up to capacity
        std::size_t regionsAdded = 0;
        for (std::size_t i = 0; i < 1000; ++i) // Use large number to ensure we hit limit
        {
            if (page.addRegion(static_cast<std::uint8_t>(i), 
                              static_cast<std::uint16_t>(i), 
                              static_cast<std::uint16_t>(i)))
            {
                regionsAdded++;
                CPPUNIT_ASSERT(page.getRegionCount() == regionsAdded);
            }
            else
            {
                // Capacity reached
                break;
            }
        }
        
        // Verify we added some regions and capacity is now full
        CPPUNIT_ASSERT(regionsAdded > 0);
        CPPUNIT_ASSERT(page.getRegionCount() == regionsAdded);
        
        // Next addition should fail
        CPPUNIT_ASSERT(!page.addRegion(255, 1000, 2000));
        CPPUNIT_ASSERT(page.getRegionCount() == regionsAdded); // Count unchanged
    }

    void testRegionCoordinateExtremeValues()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Test maximum uint16_t values for coordinates
        CPPUNIT_ASSERT(page.addRegion(1, UINT16_MAX, UINT16_MAX));
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        
        const auto& region = page.getRegion(0);
        CPPUNIT_ASSERT(region.m_regionId == 1);
        CPPUNIT_ASSERT(region.m_positionX == UINT16_MAX);
        CPPUNIT_ASSERT(region.m_positionY == UINT16_MAX);
    }

    void testRegionIdBoundaryValues()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Test minimum region ID (0)
        CPPUNIT_ASSERT(page.addRegion(0, 100, 200));
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        CPPUNIT_ASSERT(page.getRegion(0).m_regionId == 0);
        
        // Test maximum region ID (255)
        CPPUNIT_ASSERT(page.addRegion(255, 300, 400));
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        CPPUNIT_ASSERT(page.getRegion(1).m_regionId == 255);
    }

    void testGetRegionBoundaryConditions()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Add some regions
        page.addRegion(1, 10, 20);
        page.addRegion(2, 30, 40);
        
        // Valid access
        CPPUNIT_ASSERT_NO_THROW(page.getRegion(0));
        CPPUNIT_ASSERT_NO_THROW(page.getRegion(1));
        
        // Boundary condition: exactly at region count should throw
        CPPUNIT_ASSERT_THROW(page.getRegion(2), std::range_error);
        
        // Large invalid index should throw
        CPPUNIT_ASSERT_THROW(page.getRegion(SIZE_MAX), std::range_error);
    }

    void testDataPersistenceAcrossTransitions()
    {
        Page page;
        const std::uint8_t version = 9;
        const StcTime pts(StcTimeType::LOW_32, 5000);
        const std::uint32_t timeout = 60;
        
        // Start parsing and add data
        page.startParsing(version, pts, timeout);
        page.addRegion(5, 500, 600);
        
        // Data should persist through COMPLETE state
        page.finishParsing();
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        
        // Version, PTS, timeout should persist through TIMEDOUT state
        // (but regions are cleared)
        page.setTimedOut();
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
    }

    void testResetFromAllStates()
    {
        Page page;
        
        // Reset from INVALID state
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        page.reset();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        CPPUNIT_ASSERT(page.getVersion() == dvbsubdecoder::INVALID_VERSION);
        
        // Reset from INCOMPLETE state
        page.startParsing(1, StcTime(), 10);
        page.addRegion(1, 10, 20);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        page.reset();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        
        // Reset from COMPLETE state
        page.startParsing(2, StcTime(), 20);
        page.finishParsing();
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        page.reset();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        
        // Reset from TIMEDOUT state
        page.startParsing(3, StcTime(), 30);
        page.finishParsing();
        page.setTimedOut();
        CPPUNIT_ASSERT(page.getState() == Page::State::TIMEDOUT);
        page.reset();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
    }

    void testInvalidStateTransitions()
    {
        Page page;
        
        // Test finishParsing from INVALID, COMPLETE, TIMEDOUT states
        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        
        page.startParsing(1, StcTime(), 10);
        page.finishParsing();
        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        
        page.setTimedOut();
        CPPUNIT_ASSERT_THROW(page.finishParsing(), std::logic_error);
        
        // Test setTimedOut from INVALID, INCOMPLETE, TIMEDOUT states
        page.reset();
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);
        
        page.startParsing(2, StcTime(), 20);
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);
        
        page.finishParsing();
        page.setTimedOut();
        CPPUNIT_ASSERT_THROW(page.setTimedOut(), std::logic_error);
    }

    void testAddRegionFromNonIncompleteStates()
    {
        Page page;
        
        // Test addRegion from INVALID state
        CPPUNIT_ASSERT_THROW(page.addRegion(1, 10, 20), std::logic_error);
        
        // Test addRegion from COMPLETE state
        page.startParsing(1, StcTime(), 10);
        page.finishParsing();
        CPPUNIT_ASSERT_THROW(page.addRegion(2, 30, 40), std::logic_error);
        
        // Test addRegion from TIMEDOUT state
        page.setTimedOut();
        CPPUNIT_ASSERT_THROW(page.addRegion(3, 50, 60), std::logic_error);
    }

    void testMultipleRegionsSameId()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Add multiple regions with same ID (should be allowed)
        CPPUNIT_ASSERT(page.addRegion(5, 100, 200));
        CPPUNIT_ASSERT(page.addRegion(5, 300, 400));
        CPPUNIT_ASSERT(page.addRegion(5, 500, 600));
        
        CPPUNIT_ASSERT(page.getRegionCount() == 3);
        
        // Verify all regions have same ID but different positions
        CPPUNIT_ASSERT(page.getRegion(0).m_regionId == 5);
        CPPUNIT_ASSERT(page.getRegion(1).m_regionId == 5);
        CPPUNIT_ASSERT(page.getRegion(2).m_regionId == 5);
        
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 100);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionX == 300);
        CPPUNIT_ASSERT(page.getRegion(2).m_positionX == 500);
    }

    void testRegionRetrievalOrder()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Add regions in specific order
        page.addRegion(10, 1000, 2000);
        page.addRegion(20, 3000, 4000);
        page.addRegion(30, 5000, 6000);
        
        // Verify retrieval order matches insertion order
        CPPUNIT_ASSERT(page.getRegion(0).m_regionId == 10);
        CPPUNIT_ASSERT(page.getRegion(1).m_regionId == 20);
        CPPUNIT_ASSERT(page.getRegion(2).m_regionId == 30);
        
        CPPUNIT_ASSERT(page.getRegion(0).m_positionX == 1000);
        CPPUNIT_ASSERT(page.getRegion(1).m_positionX == 3000);
        CPPUNIT_ASSERT(page.getRegion(2).m_positionX == 5000);
    }

    void testCapacityReachedBehavior()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Fill to capacity
        std::size_t capacity = 0;
        for (std::size_t i = 0; i < 1000; ++i)
        {
            if (!page.addRegion(static_cast<std::uint8_t>(i), 
                               static_cast<std::uint16_t>(i), 
                               static_cast<std::uint16_t>(i)))
            {
                capacity = i;
                break;
            }
        }
        
        // Verify state remains consistent when capacity is reached
        const std::size_t initialCount = page.getRegionCount();
        const Page::State initialState = page.getState();
        
        // Attempt to add another region should fail without changing state
        CPPUNIT_ASSERT(!page.addRegion(255, 9999, 9999));
        CPPUNIT_ASSERT(page.getRegionCount() == initialCount);
        CPPUNIT_ASSERT(page.getState() == initialState);
    }

    void testEmptyPageRegionAccess()
    {
        Page page;
        
        // Test access on freshly constructed page
        CPPUNIT_ASSERT_THROW(page.getRegion(0), std::range_error);
        
        // Test access after reset
        page.startParsing(1, StcTime(), 10);
        page.addRegion(1, 10, 20);
        page.reset();
        CPPUNIT_ASSERT_THROW(page.getRegion(0), std::range_error);
    }

    void testFullLifecycleIntegration()
    {
        Page page;
        const std::uint8_t version = 15;
        const StcTime pts(StcTimeType::HIGH_32, 10000);
        const std::uint32_t timeout = 120;
        
        // Complete lifecycle: start → add regions → finish → timeout → reset
        
        // Phase 1: Start parsing
        page.startParsing(version, pts, timeout);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getVersion() == version);
        CPPUNIT_ASSERT(page.getPts() == pts);
        CPPUNIT_ASSERT(page.getTimeout() == timeout);
        
        // Phase 2: Add regions
        page.addRegion(1, 100, 200);
        page.addRegion(2, 300, 400);
        page.addRegion(3, 500, 600);
        CPPUNIT_ASSERT(page.getRegionCount() == 3);
        
        // Phase 3: Finish parsing
        page.finishParsing();
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 3);
        
        // Phase 4: Timeout
        page.setTimedOut();
        CPPUNIT_ASSERT(page.getState() == Page::State::TIMEDOUT);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
        
        // Phase 5: Reset
        page.reset();
        CPPUNIT_ASSERT(page.getState() == Page::State::INVALID);
        CPPUNIT_ASSERT(page.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(page.getTimeout() == 0);
    }

    void testMultiStageParsingSimulation()
    {
        Page page;
        
        // Simulate multiple parsing attempts with different parameters
        
        // First parsing attempt
        page.startParsing(1, StcTime(StcTimeType::LOW_32, 1000), 30);
        page.addRegion(1, 10, 20);
        CPPUNIT_ASSERT(page.getVersion() == 1);
        CPPUNIT_ASSERT(page.getRegionCount() == 1);
        
        // Second parsing attempt (simulating re-parsing)
        page.startParsing(2, StcTime(StcTimeType::HIGH_32, 2000), 60);
        CPPUNIT_ASSERT(page.getVersion() == 2);
        CPPUNIT_ASSERT(page.getRegionCount() == 0); // Should be cleared
        
        page.addRegion(10, 100, 200);
        page.addRegion(20, 300, 400);
        page.finishParsing();
        CPPUNIT_ASSERT(page.getState() == Page::State::COMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 2);
        
        // Third parsing attempt after completion
        page.startParsing(3, StcTime(StcTimeType::LOW_32, 3000), 90);
        CPPUNIT_ASSERT(page.getVersion() == 3);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        CPPUNIT_ASSERT(page.getRegionCount() == 0);
    }

    void testRegionOverflowBehavior()
    {
        Page page;
        page.startParsing(1, StcTime(), 10);
        
        // Fill page to capacity and beyond
        std::size_t successfulAdditions = 0;
        std::size_t totalAttempts = 0;
        
        // Try to add many regions
        for (std::size_t i = 0; i < 1000; ++i)
        {
            totalAttempts++;
            if (page.addRegion(static_cast<std::uint8_t>(i % 256), 
                              static_cast<std::uint16_t>(i), 
                              static_cast<std::uint16_t>(i * 2)))
            {
                successfulAdditions++;
            }
            else
            {
                // Once we fail, all subsequent attempts should also fail
                for (std::size_t j = i + 1; j < 1000; ++j)
                {
                    totalAttempts++;
                    CPPUNIT_ASSERT(!page.addRegion(static_cast<std::uint8_t>(j % 256), 
                                                  static_cast<std::uint16_t>(j), 
                                                  static_cast<std::uint16_t>(j * 2)));
                }
                break;
            }
        }
        
        // Verify invariants are maintained
        CPPUNIT_ASSERT(successfulAdditions > 0);
        CPPUNIT_ASSERT(successfulAdditions < totalAttempts);
        CPPUNIT_ASSERT(page.getRegionCount() == successfulAdditions);
        CPPUNIT_ASSERT(page.getState() == Page::State::INCOMPLETE);
        
        // Verify all successfully added regions are accessible
        for (std::size_t i = 0; i < successfulAdditions; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(page.getRegion(i));
            const auto& region = page.getRegion(i);
            CPPUNIT_ASSERT(region.m_regionId == static_cast<std::uint8_t>(i % 256));
            CPPUNIT_ASSERT(region.m_positionX == static_cast<std::int32_t>(i));
            CPPUNIT_ASSERT(region.m_positionY == static_cast<std::int32_t>(i * 2));
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PageTest);
