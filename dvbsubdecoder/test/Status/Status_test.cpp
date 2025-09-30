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
#include <limits>

#include "Misc.hpp"

#include "Status.hpp"

using dvbsubdecoder::Status;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;
using dvbsubdecoder::Specification;

class StatusTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StatusTest );
    CPPUNIT_TEST(testDefault);
    CPPUNIT_TEST(testStc);
    CPPUNIT_TEST(testPageIds);
    CPPUNIT_TEST(testConstructorWithVersion121);
    CPPUNIT_TEST(testConstructorWithVersion131);
    CPPUNIT_TEST(testInitialStateIsValid);
    CPPUNIT_TEST(testPageIdBoundaryValuesMin);
    CPPUNIT_TEST(testPageIdBoundaryValuesMax);
    CPPUNIT_TEST(testPageIdBoundaryValuesMid);
    CPPUNIT_TEST(testZeroPageIds);
    CPPUNIT_TEST(testMaxPageIds);
    CPPUNIT_TEST(testPageIdEdgeCases);
    CPPUNIT_TEST(testStcTimeBoundaryValues);
    CPPUNIT_TEST(testStcTimeTypes);
    CPPUNIT_TEST(testStcTimeZeroValues);
    CPPUNIT_TEST(testStcTimeMaxValues);
    CPPUNIT_TEST(testStcTimeEquality);
    CPPUNIT_TEST(testMultiplePageIdUpdates);
    CPPUNIT_TEST(testPageIdStateTransitions);
    CPPUNIT_TEST(testStcTimeSequence);
    CPPUNIT_TEST(testMixedOperations);
    CPPUNIT_TEST(testPageQueryConsistency);
    CPPUNIT_TEST(testSpecVersionConsistency);
    CPPUNIT_TEST(testStateReset);
    CPPUNIT_TEST(testComplexScenario);
    CPPUNIT_TEST(testAllPageIdCombinations);
    CPPUNIT_TEST(testStcTimeComplexScenarios);
    CPPUNIT_TEST(testIntegrationScenario);
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

    void testDefault()
    {
        Status status121(Specification::VERSION_1_2_1);
        CPPUNIT_ASSERT(
                status121.getSpecVersion() == Specification::VERSION_1_2_1);

        Status status131(Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(
                status131.getSpecVersion() == Specification::VERSION_1_3_1);

        Status status(Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(!status.isAncillaryPage(0));
        CPPUNIT_ASSERT(!status.isCompositionPage(0));
        CPPUNIT_ASSERT(!status.isSelectedPage(0));

        CPPUNIT_ASSERT(status.getLastPts() == StcTime());
    }

    void testStc()
    {
        Status status(Specification::VERSION_1_3_1);

        StcTime timeHigh(StcTimeType::HIGH_32, 1);
        StcTime timeLow(StcTimeType::LOW_32, 2);

        status.setLastPts(timeHigh);
        CPPUNIT_ASSERT(status.getLastPts() == timeHigh);

        status.setLastPts(timeLow);
        CPPUNIT_ASSERT(status.getLastPts() == timeLow);
    }

    void testPageIds()
    {
        Status statusDiff(Specification::VERSION_1_3_1);

        statusDiff.setPageIds(1, 2);

        CPPUNIT_ASSERT(!statusDiff.isSelectedPage(0));
        CPPUNIT_ASSERT(statusDiff.isSelectedPage(1));
        CPPUNIT_ASSERT(statusDiff.isSelectedPage(2));
        CPPUNIT_ASSERT(!statusDiff.isSelectedPage(3));

        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(0));
        CPPUNIT_ASSERT(statusDiff.isCompositionPage(1));
        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(2));
        CPPUNIT_ASSERT(!statusDiff.isCompositionPage(3));

        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(0));
        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(1));
        CPPUNIT_ASSERT(statusDiff.isAncillaryPage(2));
        CPPUNIT_ASSERT(!statusDiff.isAncillaryPage(3));

        Status statusSame(Specification::VERSION_1_3_1);

        statusSame.setPageIds(1, 1);

        CPPUNIT_ASSERT(!statusSame.isSelectedPage(0));
        CPPUNIT_ASSERT(statusSame.isSelectedPage(1));
        CPPUNIT_ASSERT(!statusSame.isSelectedPage(2));
        CPPUNIT_ASSERT(!statusSame.isSelectedPage(3));

        CPPUNIT_ASSERT(!statusSame.isCompositionPage(0));
        CPPUNIT_ASSERT(statusSame.isCompositionPage(1));
        CPPUNIT_ASSERT(!statusSame.isCompositionPage(2));
        CPPUNIT_ASSERT(!statusSame.isCompositionPage(3));

        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(0));
        CPPUNIT_ASSERT(statusSame.isAncillaryPage(1));
        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(2));
        CPPUNIT_ASSERT(!statusSame.isAncillaryPage(3));
    }

    // Constructor validation
    void testConstructorWithVersion121()
    {
        Status status(Specification::VERSION_1_2_1);
        CPPUNIT_ASSERT(status.getSpecVersion() == Specification::VERSION_1_2_1);
        CPPUNIT_ASSERT(status.getLastPts() == StcTime());
        CPPUNIT_ASSERT(!status.isSelectedPage(0));
        CPPUNIT_ASSERT(!status.isCompositionPage(0));
        CPPUNIT_ASSERT(!status.isAncillaryPage(0));
    }

    void testConstructorWithVersion131()
    {
        Status status(Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(status.getSpecVersion() == Specification::VERSION_1_3_1);
        CPPUNIT_ASSERT(status.getLastPts() == StcTime());
        CPPUNIT_ASSERT(!status.isSelectedPage(0));
        CPPUNIT_ASSERT(!status.isCompositionPage(0));
        CPPUNIT_ASSERT(!status.isAncillaryPage(0));
    }

    void testInitialStateIsValid()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Verify initial state for various page IDs
        for (std::uint16_t pageId = 0; pageId < 10; ++pageId)
        {
            CPPUNIT_ASSERT(!status.isSelectedPage(pageId));
            CPPUNIT_ASSERT(!status.isCompositionPage(pageId));
            CPPUNIT_ASSERT(!status.isAncillaryPage(pageId));
        }
        
        // Verify default PTS
        CPPUNIT_ASSERT(status.getLastPts() == StcTime());
    }

    // Boundary value testing for uint16_t page IDs
    void testPageIdBoundaryValuesMin()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test minimum values
        status.setPageIds(0, 0);
        CPPUNIT_ASSERT(status.isSelectedPage(0));
        CPPUNIT_ASSERT(status.isCompositionPage(0));
        CPPUNIT_ASSERT(status.isAncillaryPage(0));
        CPPUNIT_ASSERT(!status.isSelectedPage(1));
    }

    void testPageIdBoundaryValuesMax()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test maximum values
        const std::uint16_t maxVal = std::numeric_limits<std::uint16_t>::max();
        status.setPageIds(maxVal, maxVal);
        CPPUNIT_ASSERT(status.isSelectedPage(maxVal));
        CPPUNIT_ASSERT(status.isCompositionPage(maxVal));
        CPPUNIT_ASSERT(status.isAncillaryPage(maxVal));
        CPPUNIT_ASSERT(!status.isSelectedPage(maxVal - 1));
    }

    void testPageIdBoundaryValuesMid()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test mid-range values
        const std::uint16_t midVal = std::numeric_limits<std::uint16_t>::max() / 2;
        status.setPageIds(midVal, midVal + 1);
        CPPUNIT_ASSERT(status.isSelectedPage(midVal));
        CPPUNIT_ASSERT(status.isSelectedPage(midVal + 1));
        CPPUNIT_ASSERT(status.isCompositionPage(midVal));
        CPPUNIT_ASSERT(status.isAncillaryPage(midVal + 1));
        CPPUNIT_ASSERT(!status.isSelectedPage(midVal - 1));
        CPPUNIT_ASSERT(!status.isSelectedPage(midVal + 2));
    }

    void testZeroPageIds()
    {
        Status status(Specification::VERSION_1_3_1);
        
        status.setPageIds(0, 1);
        CPPUNIT_ASSERT(status.isSelectedPage(0));
        CPPUNIT_ASSERT(status.isSelectedPage(1));
        CPPUNIT_ASSERT(status.isCompositionPage(0));
        CPPUNIT_ASSERT(status.isAncillaryPage(1));
        CPPUNIT_ASSERT(!status.isCompositionPage(1));
        CPPUNIT_ASSERT(!status.isAncillaryPage(0));
    }

    void testMaxPageIds()
    {
        Status status(Specification::VERSION_1_3_1);
        
        const std::uint16_t maxVal = std::numeric_limits<std::uint16_t>::max();
        status.setPageIds(maxVal - 1, maxVal);
        CPPUNIT_ASSERT(status.isSelectedPage(maxVal - 1));
        CPPUNIT_ASSERT(status.isSelectedPage(maxVal));
        CPPUNIT_ASSERT(status.isCompositionPage(maxVal - 1));
        CPPUNIT_ASSERT(status.isAncillaryPage(maxVal));
        CPPUNIT_ASSERT(!status.isSelectedPage(maxVal - 2));
    }

    void testPageIdEdgeCases()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test edge cases around boundaries
        status.setPageIds(1, 65534); // Near min and max
        CPPUNIT_ASSERT(status.isSelectedPage(1));
        CPPUNIT_ASSERT(status.isSelectedPage(65534));
        CPPUNIT_ASSERT(!status.isSelectedPage(0));
        CPPUNIT_ASSERT(!status.isSelectedPage(2));
        CPPUNIT_ASSERT(!status.isSelectedPage(65533));
        CPPUNIT_ASSERT(!status.isSelectedPage(65535));
    }

    // StcTime boundary testing
    void testStcTimeBoundaryValues()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test minimum uint32_t value
        StcTime minTime(StcTimeType::HIGH_32, 0);
        status.setLastPts(minTime);
        CPPUNIT_ASSERT(status.getLastPts() == minTime);
        
        // Test maximum uint32_t value
        StcTime maxTime(StcTimeType::HIGH_32, std::numeric_limits<std::uint32_t>::max());
        status.setLastPts(maxTime);
        CPPUNIT_ASSERT(status.getLastPts() == maxTime);
    }

    void testStcTimeTypes()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test both StcTimeType values
        StcTime highTime(StcTimeType::HIGH_32, 12345);
        StcTime lowTime(StcTimeType::LOW_32, 12345);
        
        status.setLastPts(highTime);
        CPPUNIT_ASSERT(status.getLastPts() == highTime);
        
        status.setLastPts(lowTime);
        CPPUNIT_ASSERT(status.getLastPts() == lowTime);
    }

    void testStcTimeZeroValues()
    {
        Status status(Specification::VERSION_1_3_1);
        
        StcTime zeroHigh(StcTimeType::HIGH_32, 0);
        StcTime zeroLow(StcTimeType::LOW_32, 0);
        
        status.setLastPts(zeroHigh);
        CPPUNIT_ASSERT(status.getLastPts() == zeroHigh);
        
        status.setLastPts(zeroLow);
        CPPUNIT_ASSERT(status.getLastPts() == zeroLow);
    }

    void testStcTimeMaxValues()
    {
        Status status(Specification::VERSION_1_3_1);
        
        const std::uint32_t maxVal = std::numeric_limits<std::uint32_t>::max();
        StcTime maxHigh(StcTimeType::HIGH_32, maxVal);
        StcTime maxLow(StcTimeType::LOW_32, maxVal);
        
        status.setLastPts(maxHigh);
        CPPUNIT_ASSERT(status.getLastPts() == maxHigh);
        
        status.setLastPts(maxLow);
        CPPUNIT_ASSERT(status.getLastPts() == maxLow);
    }

    void testStcTimeEquality()
    {
        Status status(Specification::VERSION_1_3_1);
        
        StcTime time1(StcTimeType::HIGH_32, 1000);
        StcTime time2(StcTimeType::HIGH_32, 1000);
        StcTime time3(StcTimeType::LOW_32, 1000);
        
        status.setLastPts(time1);
        CPPUNIT_ASSERT(status.getLastPts() == time1);
        CPPUNIT_ASSERT(status.getLastPts() == time2); // Same type and value
        
        status.setLastPts(time3);
        CPPUNIT_ASSERT(status.getLastPts() == time3);
        CPPUNIT_ASSERT(!(status.getLastPts() == time1)); // Different type
    }

    // State management
    void testMultiplePageIdUpdates()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Initial setting
        status.setPageIds(100, 200);
        CPPUNIT_ASSERT(status.isSelectedPage(100));
        CPPUNIT_ASSERT(status.isSelectedPage(200));
        CPPUNIT_ASSERT(status.isCompositionPage(100));
        CPPUNIT_ASSERT(status.isAncillaryPage(200));
        
        // Update page IDs
        status.setPageIds(300, 400);
        CPPUNIT_ASSERT(!status.isSelectedPage(100));
        CPPUNIT_ASSERT(!status.isSelectedPage(200));
        CPPUNIT_ASSERT(status.isSelectedPage(300));
        CPPUNIT_ASSERT(status.isSelectedPage(400));
        CPPUNIT_ASSERT(status.isCompositionPage(300));
        CPPUNIT_ASSERT(status.isAncillaryPage(400));
    }

    void testPageIdStateTransitions()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Transition from unset to set
        CPPUNIT_ASSERT(!status.isSelectedPage(1));
        status.setPageIds(1, 2);
        CPPUNIT_ASSERT(status.isSelectedPage(1));
        
        // Transition from different pages to same page
        status.setPageIds(5, 5);
        CPPUNIT_ASSERT(!status.isSelectedPage(1));
        CPPUNIT_ASSERT(!status.isSelectedPage(2));
        CPPUNIT_ASSERT(status.isSelectedPage(5));
        CPPUNIT_ASSERT(status.isCompositionPage(5));
        CPPUNIT_ASSERT(status.isAncillaryPage(5));
    }

    void testStcTimeSequence()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test sequence of different PTS values
        StcTime time1(StcTimeType::HIGH_32, 1000);
        StcTime time2(StcTimeType::LOW_32, 2000);
        StcTime time3(StcTimeType::HIGH_32, 3000);
        
        status.setLastPts(time1);
        CPPUNIT_ASSERT(status.getLastPts() == time1);
        
        status.setLastPts(time2);
        CPPUNIT_ASSERT(status.getLastPts() == time2);
        
        status.setLastPts(time3);
        CPPUNIT_ASSERT(status.getLastPts() == time3);
    }

    void testMixedOperations()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Mix page ID and PTS operations
        StcTime time1(StcTimeType::HIGH_32, 5000);
        status.setLastPts(time1);
        status.setPageIds(10, 20);
        
        CPPUNIT_ASSERT(status.getLastPts() == time1);
        CPPUNIT_ASSERT(status.isSelectedPage(10));
        CPPUNIT_ASSERT(status.isSelectedPage(20));
        
        StcTime time2(StcTimeType::LOW_32, 6000);
        status.setLastPts(time2);
        
        CPPUNIT_ASSERT(status.getLastPts() == time2);
        CPPUNIT_ASSERT(status.isSelectedPage(10)); // Page IDs should remain
        CPPUNIT_ASSERT(status.isSelectedPage(20));
    }

    void testPageQueryConsistency()
    {
        Status status(Specification::VERSION_1_3_1);
        
        status.setPageIds(100, 200);
        
        // Test consistency across all query methods
        CPPUNIT_ASSERT(status.isSelectedPage(100));
        CPPUNIT_ASSERT(status.isCompositionPage(100));
        CPPUNIT_ASSERT(!status.isAncillaryPage(100));
        
        CPPUNIT_ASSERT(status.isSelectedPage(200));
        CPPUNIT_ASSERT(!status.isCompositionPage(200));
        CPPUNIT_ASSERT(status.isAncillaryPage(200));
        
        CPPUNIT_ASSERT(!status.isSelectedPage(150));
        CPPUNIT_ASSERT(!status.isCompositionPage(150));
        CPPUNIT_ASSERT(!status.isAncillaryPage(150));
    }

    void testSpecVersionConsistency()
    {
        Status status121(Specification::VERSION_1_2_1);
        Status status131(Specification::VERSION_1_3_1);
        
        // Verify spec versions remain constant throughout operations
        status121.setPageIds(1, 2);
        StcTime time(StcTimeType::HIGH_32, 1000);
        status121.setLastPts(time);
        CPPUNIT_ASSERT(status121.getSpecVersion() == Specification::VERSION_1_2_1);
        
        status131.setPageIds(3, 4);
        status131.setLastPts(time);
        CPPUNIT_ASSERT(status131.getSpecVersion() == Specification::VERSION_1_3_1);
    }

    void testStateReset()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Set initial state
        status.setPageIds(100, 200);
        StcTime time(StcTimeType::HIGH_32, 5000);
        status.setLastPts(time);
        
        // Reset to new values
        status.setPageIds(0, 0);
        StcTime newTime(StcTimeType::LOW_32, 0);
        status.setLastPts(newTime);
        
        CPPUNIT_ASSERT(!status.isSelectedPage(100));
        CPPUNIT_ASSERT(!status.isSelectedPage(200));
        CPPUNIT_ASSERT(status.isSelectedPage(0));
        CPPUNIT_ASSERT(status.getLastPts() == newTime);
    }

    // Complex scenarios
    void testComplexScenario()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Complex workflow simulation
        status.setPageIds(1, 2);
        StcTime pts1(StcTimeType::HIGH_32, 1000);
        status.setLastPts(pts1);
        
        // Verify intermediate state
        CPPUNIT_ASSERT(status.isSelectedPage(1));
        CPPUNIT_ASSERT(status.isSelectedPage(2));
        CPPUNIT_ASSERT(status.getLastPts() == pts1);
        
        // Update PTS only
        StcTime pts2(StcTimeType::LOW_32, 2000);
        status.setLastPts(pts2);
        CPPUNIT_ASSERT(status.isSelectedPage(1)); // Pages unchanged
        CPPUNIT_ASSERT(status.isSelectedPage(2));
        CPPUNIT_ASSERT(status.getLastPts() == pts2);
        
        // Update pages only
        status.setPageIds(10, 20);
        CPPUNIT_ASSERT(!status.isSelectedPage(1));
        CPPUNIT_ASSERT(!status.isSelectedPage(2));
        CPPUNIT_ASSERT(status.isSelectedPage(10));
        CPPUNIT_ASSERT(status.isSelectedPage(20));
        CPPUNIT_ASSERT(status.getLastPts() == pts2); // PTS unchanged
    }

    void testAllPageIdCombinations()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test various page ID combinations
        const std::uint16_t testValues[] = {0, 1, 100, 32767, 65534, 65535};
        
        for (auto comp : testValues)
        {
            for (auto anc : testValues)
            {
                status.setPageIds(comp, anc);
                
                CPPUNIT_ASSERT(status.isSelectedPage(comp));
                CPPUNIT_ASSERT(status.isSelectedPage(anc));
                CPPUNIT_ASSERT(status.isCompositionPage(comp));
                CPPUNIT_ASSERT(status.isAncillaryPage(anc));
                
                if (comp != anc)
                {
                    CPPUNIT_ASSERT(!status.isAncillaryPage(comp));
                    CPPUNIT_ASSERT(!status.isCompositionPage(anc));
                }
                else
                {
                    CPPUNIT_ASSERT(status.isAncillaryPage(comp));
                    CPPUNIT_ASSERT(status.isCompositionPage(anc));
                }
            }
        }
    }

    void testStcTimeComplexScenarios()
    {
        Status status(Specification::VERSION_1_3_1);
        
        // Test alternating types and values
        const std::uint32_t testValues[] = {0, 1, 1000, 2147483647u, 4294967294u, 4294967295u};
        
        for (auto val : testValues)
        {
            StcTime highTime(StcTimeType::HIGH_32, val);
            StcTime lowTime(StcTimeType::LOW_32, val);
            
            status.setLastPts(highTime);
            CPPUNIT_ASSERT(status.getLastPts() == highTime);
            
            status.setLastPts(lowTime);
            CPPUNIT_ASSERT(status.getLastPts() == lowTime);
        }
    }

    void testIntegrationScenario()
    {
        // Integration test with both specification versions
        Status status121(Specification::VERSION_1_2_1);
        Status status131(Specification::VERSION_1_3_1);
        
        // Setup both with same data
        status121.setPageIds(42, 43);
        status131.setPageIds(42, 43);
        
        StcTime pts(StcTimeType::HIGH_32, 12345);
        status121.setLastPts(pts);
        status131.setLastPts(pts);
        
        // Verify both behave identically for page operations
        CPPUNIT_ASSERT(status121.isSelectedPage(42) == status131.isSelectedPage(42));
        CPPUNIT_ASSERT(status121.isSelectedPage(43) == status131.isSelectedPage(43));
        CPPUNIT_ASSERT(status121.isCompositionPage(42) == status131.isCompositionPage(42));
        CPPUNIT_ASSERT(status121.isAncillaryPage(43) == status131.isAncillaryPage(43));
        CPPUNIT_ASSERT(status121.getLastPts() == status131.getLastPts());
        
        // But differ in specification version
        CPPUNIT_ASSERT(status121.getSpecVersion() != status131.getSpecVersion());
        CPPUNIT_ASSERT(status121.getSpecVersion() == Specification::VERSION_1_2_1);
        CPPUNIT_ASSERT(status131.getSpecVersion() == Specification::VERSION_1_3_1);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(StatusTest);
