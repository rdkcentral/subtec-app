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

#include "RenderingState.hpp"
#include "Misc.hpp"

using dvbsubdecoder::RenderingState;
using dvbsubdecoder::Rectangle;

class RenderingStateTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( RenderingStateTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testMarking);
    CPPUNIT_TEST(testLimit);
    CPPUNIT_TEST(testBadIndex);
    CPPUNIT_TEST(testConstructorDefaults);
    CPPUNIT_TEST(testResetCompleteValidation);
    CPPUNIT_TEST(testMultipleResetCalls);
    CPPUNIT_TEST(testSetBoundsExtremeCoordinates);
    CPPUNIT_TEST(testSetBoundsInvalidRectangles);
    CPPUNIT_TEST(testSetBoundsIdenticalBounds);
    CPPUNIT_TEST(testSetBoundsNegativeCoordinates);
    CPPUNIT_TEST(testAddRegionBoundaryValues);
    CPPUNIT_TEST(testAddRegionDuplicateIds);
    CPPUNIT_TEST(testAddRegionInvalidRectangles);
    CPPUNIT_TEST(testAddRegionExactCapacity);
    CPPUNIT_TEST(testAddRegionBeyondCapacity);
    CPPUNIT_TEST(testAddRegionDirtyFlagValidation);
    CPPUNIT_TEST(testGetRegionByIndexBoundaries);
    CPPUNIT_TEST(testGetRegionByIndexExtremeValues);
    CPPUNIT_TEST(testUnmarkRegionExtremeIndices);
    CPPUNIT_TEST(testExceptionTypeVerification);
    CPPUNIT_TEST(testMarkAllRegionsEmptyState);
    CPPUNIT_TEST(testRemoveAllRegionsMultipleCalls);
    CPPUNIT_TEST(testDirtyFlagStateVerification);
    CPPUNIT_TEST(testDirtyFlagInitialState);
    CPPUNIT_TEST(testComplexOperationSequences);
    CPPUNIT_TEST(testStatePersistenceAfterBounds);
    CPPUNIT_TEST(testStateConsistencyAfterFailures);
    CPPUNIT_TEST(testMaximumCapacityScenarios);
    CPPUNIT_TEST(testRectangleStorageAccuracy);
    CPPUNIT_TEST(testRegionInfoMemberAccess);
    CPPUNIT_TEST(testConstCorrectnessValidation);
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

    void testSimple()
    {
        RenderingState state;

        const Rectangle EMPTY_RECTANGLE =
        { 0, 0, 0, 0 };
        const Rectangle DISPLAY_BOUNDS =
        { 0, 1, 1000, 1001 };
        const Rectangle WINDOW_BOUNDS =
        { 10, 11, 100, 101 };
        const std::uint8_t REGION_ID = 1;
        const std::uint8_t REGION_VERSION = 2;
        const Rectangle REGION_RECT =
        { 10, 11, 100, 101 };

        CPPUNIT_ASSERT(state.getDisplayBounds() == EMPTY_RECTANGLE);
        CPPUNIT_ASSERT(state.getWindowBounds() == EMPTY_RECTANGLE);
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        state.setBounds(DISPLAY_BOUNDS, WINDOW_BOUNDS);
        CPPUNIT_ASSERT(state.addRegion(REGION_ID, REGION_VERSION, REGION_RECT));

        CPPUNIT_ASSERT(state.getRegionCount() == 1);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == REGION_ID);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_version == REGION_VERSION);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle == REGION_RECT);

        state.removeAllRegions();

        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        CPPUNIT_ASSERT(state.addRegion(REGION_ID, REGION_VERSION, REGION_RECT));

        CPPUNIT_ASSERT(state.getRegionCount() == 1);

        state.reset();

        CPPUNIT_ASSERT(state.getDisplayBounds() == EMPTY_RECTANGLE);
        CPPUNIT_ASSERT(state.getWindowBounds() == EMPTY_RECTANGLE);
        CPPUNIT_ASSERT(state.getRegionCount() == 0);
    }

    void testMarking()
    {
        RenderingState state;

        const int REGION_COUNT = 10;

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(i, i,
            { i, i + 1, i + 100, i + 101 }));
        }

        state.markAllRegionsAsDirty();

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty);
        }

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            if (i & 1)
            {
                state.unmarkRegionAsDirtyByIndex(i);
            }
        }

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
            if (i & 1)
            {
                CPPUNIT_ASSERT(!state.getRegionByIndex(i).m_dirty);
            }
            else
            {
                CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty);
            }
        }

        state.markAllRegionsAsDirty();

        for (int i = 0; i < REGION_COUNT; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty);
        }
    }

    void testLimit()
    {
        std::size_t regionCount = 0;

        RenderingState state;

        for (int i = 0;; ++i)
        {
            if (state.addRegion(i & 0xFF, i & 0x0F,
            { 0, 0, 10, 10 }))
            {
                CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == (i & 0xFF));

                ++regionCount;
                CPPUNIT_ASSERT(state.getRegionCount() == regionCount);
            }
            else
            {
                CPPUNIT_ASSERT(state.getRegionCount() == regionCount);
                break;
            }
        }
    }

    void testBadIndex()
    {
        RenderingState state;

        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(0), std::range_error);
        CPPUNIT_ASSERT_THROW(state.unmarkRegionAsDirtyByIndex(0),
                std::range_error);

        CPPUNIT_ASSERT(state.addRegion(0, 0,
        { }));

        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(2), std::range_error);
        CPPUNIT_ASSERT_THROW(state.unmarkRegionAsDirtyByIndex(2),
                std::range_error);
    }

    // Constructor and initialization tests
    void testConstructorDefaults()
    {
        RenderingState state;
        const Rectangle EMPTY_RECTANGLE = { 0, 0, 0, 0 };

        // Verify constructor initializes to safe defaults
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == 0);
        CPPUNIT_ASSERT(state.getRegionCount() == 0);
    }

    void testResetCompleteValidation()
    {
        RenderingState state;
        const Rectangle DISPLAY_BOUNDS = { 100, 200, 1000, 2000 };
        const Rectangle WINDOW_BOUNDS = { 50, 100, 500, 1000 };
        const Rectangle EMPTY_RECTANGLE = { 0, 0, 0, 0 };

        // Set up state with bounds and regions
        state.setBounds(DISPLAY_BOUNDS, WINDOW_BOUNDS);
        CPPUNIT_ASSERT(state.addRegion(10, 20, { 10, 20, 100, 200 }));
        CPPUNIT_ASSERT(state.addRegion(30, 40, { 30, 40, 300, 400 }));

        // Verify state is set up
        CPPUNIT_ASSERT(state.getRegionCount() == 2);
        CPPUNIT_ASSERT(!(state.getDisplayBounds().m_x1 == 0 && state.getDisplayBounds().m_y1 == 0));

        // Reset and verify ALL fields are reset
        state.reset();

        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == 0);
        CPPUNIT_ASSERT(state.getRegionCount() == 0);
    }

    void testMultipleResetCalls()
    {
        RenderingState state;
        const Rectangle DISPLAY_BOUNDS = { 10, 20, 100, 200 };
        const Rectangle WINDOW_BOUNDS = { 5, 10, 50, 100 };

        // Set up some state
        state.setBounds(DISPLAY_BOUNDS, WINDOW_BOUNDS);
        CPPUNIT_ASSERT(state.addRegion(1, 2, { 1, 2, 10, 20 }));

        // Multiple reset calls should be safe
        CPPUNIT_ASSERT_NO_THROW(state.reset());
        CPPUNIT_ASSERT_NO_THROW(state.reset());
        CPPUNIT_ASSERT_NO_THROW(state.reset());

        // State should remain consistent
        CPPUNIT_ASSERT(state.getRegionCount() == 0);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == 0);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == 0);
    }

    // setBounds boundary and edge case tests
    void testSetBoundsExtremeCoordinates()
    {
        RenderingState state;
        const std::int32_t MAX_INT = std::numeric_limits<std::int32_t>::max();
        const std::int32_t MIN_INT = std::numeric_limits<std::int32_t>::min();

        // Test maximum coordinates
        const Rectangle EXTREME_DISPLAY = { MAX_INT - 1, MAX_INT - 1, MAX_INT, MAX_INT };
        const Rectangle EXTREME_WINDOW = { MIN_INT, MIN_INT, MIN_INT + 1, MIN_INT + 1 };

        CPPUNIT_ASSERT_NO_THROW(state.setBounds(EXTREME_DISPLAY, EXTREME_WINDOW));

        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == MAX_INT - 1);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == MAX_INT - 1);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == MAX_INT);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == MAX_INT);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == MIN_INT);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == MIN_INT);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == MIN_INT + 1);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == MIN_INT + 1);
    }

    void testSetBoundsInvalidRectangles()
    {
        RenderingState state;

        // Test inverted rectangles (x2 < x1, y2 < y1)
        const Rectangle INVERTED_DISPLAY = { 100, 200, 50, 150 };
        const Rectangle INVERTED_WINDOW = { 200, 300, 100, 250 };

        CPPUNIT_ASSERT_NO_THROW(state.setBounds(INVERTED_DISPLAY, INVERTED_WINDOW));

        // Should store exactly what was provided
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == 100);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == 200);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == 50);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == 150);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == 200);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == 300);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 100);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == 250);
    }

    void testSetBoundsIdenticalBounds()
    {
        RenderingState state;
        const Rectangle IDENTICAL_BOUNDS = { 50, 60, 150, 160 };

        // Test setting identical display and window bounds
        CPPUNIT_ASSERT_NO_THROW(state.setBounds(IDENTICAL_BOUNDS, IDENTICAL_BOUNDS));

        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == 50);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == 60);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == 150);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == 160);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == 50);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == 60);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 150);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == 160);
    }

    void testSetBoundsNegativeCoordinates()
    {
        RenderingState state;

        // Test all negative coordinates
        const Rectangle NEGATIVE_DISPLAY = { -1000, -2000, -100, -200 };
        const Rectangle NEGATIVE_WINDOW = { -500, -600, -50, -60 };

        CPPUNIT_ASSERT_NO_THROW(state.setBounds(NEGATIVE_DISPLAY, NEGATIVE_WINDOW));

        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == -1000);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == -2000);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == -100);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y2 == -200);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x1 == -500);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y1 == -600);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == -50);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == -60);
    }

    // addRegion boundary and edge case tests
    void testAddRegionBoundaryValues()
    {
        RenderingState state;

        // Test boundary values for ID and version
        CPPUNIT_ASSERT(state.addRegion(0, 0, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == 0);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_version == 0);

        CPPUNIT_ASSERT(state.addRegion(255, 255, { 10, 10, 20, 20 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_id == 255);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_version == 255);

        // Test intermediate values
        CPPUNIT_ASSERT(state.addRegion(128, 64, { 20, 20, 30, 30 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_id == 128);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_version == 64);
    }

    void testAddRegionDuplicateIds()
    {
        RenderingState state;

        // Add regions with duplicate IDs - should be allowed
        CPPUNIT_ASSERT(state.addRegion(10, 1, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.addRegion(10, 2, { 10, 10, 20, 20 }));
        CPPUNIT_ASSERT(state.addRegion(10, 3, { 20, 20, 30, 30 }));

        CPPUNIT_ASSERT(state.getRegionCount() == 3);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == 10);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_version == 1);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_id == 10);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_version == 2);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_id == 10);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_version == 3);
    }

    void testAddRegionInvalidRectangles()
    {
        RenderingState state;
        const std::int32_t MAX_INT = std::numeric_limits<std::int32_t>::max();
        const std::int32_t MIN_INT = std::numeric_limits<std::int32_t>::min();

        // Test inverted rectangle
        CPPUNIT_ASSERT(state.addRegion(1, 1, { 100, 200, 50, 150 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle.m_x1 == 100);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle.m_y1 == 200);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle.m_x2 == 50);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle.m_y2 == 150);

        // Test extreme coordinates
        CPPUNIT_ASSERT(state.addRegion(2, 2, { MIN_INT, MIN_INT, MAX_INT, MAX_INT }));
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_rectangle.m_x1 == MIN_INT);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_rectangle.m_x2 == MAX_INT);

        // Test all negative coordinates
        CPPUNIT_ASSERT(state.addRegion(3, 3, { -1000, -2000, -100, -200 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_rectangle.m_x1 == -1000);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_rectangle.m_y1 == -2000);
    }

    void testAddRegionExactCapacity()
    {
        RenderingState state;

        // Add exactly MAX_SUPPORTED_REGIONS (16) regions
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 
                                         static_cast<std::uint8_t>(i), 
                                         { static_cast<std::int32_t>(i), 
                                           static_cast<std::int32_t>(i), 
                                           static_cast<std::int32_t>(i + 10), 
                                           static_cast<std::int32_t>(i + 10) }));
            CPPUNIT_ASSERT(state.getRegionCount() == i + 1);
        }

        CPPUNIT_ASSERT(state.getRegionCount() == 16);

        // Verify all regions are stored correctly
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_version == i);
        }
    }

    void testAddRegionBeyondCapacity()
    {
        RenderingState state;

        // Fill to capacity
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 1, { 0, 0, 10, 10 }));
        }

        CPPUNIT_ASSERT(state.getRegionCount() == 16);

        // Attempt to add beyond capacity - should fail
        CPPUNIT_ASSERT(!state.addRegion(100, 100, { 100, 100, 200, 200 }));
        CPPUNIT_ASSERT(state.getRegionCount() == 16);

        // Verify state is unchanged
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
        }
    }

    void testAddRegionDirtyFlagValidation()
    {
        RenderingState state;

        // Add regions and verify they start as dirty
        CPPUNIT_ASSERT(state.addRegion(1, 1, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_dirty == true);

        CPPUNIT_ASSERT(state.addRegion(2, 2, { 10, 10, 20, 20 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_dirty == true);

        // Unmark and verify
        state.unmarkRegionAsDirtyByIndex(0);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_dirty == false);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_dirty == true);

        // Add another region - should be dirty
        CPPUNIT_ASSERT(state.addRegion(3, 3, { 20, 20, 30, 30 }));
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_dirty == true);
    }

    // Index boundary and exception tests
    void testGetRegionByIndexBoundaries()
    {
        RenderingState state;

        // Test with empty state
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(0), std::range_error);

        // Add one region
        CPPUNIT_ASSERT(state.addRegion(10, 20, { 0, 0, 10, 10 }));

        // Test valid boundary indices
        CPPUNIT_ASSERT_NO_THROW(state.getRegionByIndex(0));
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == 10);

        // Test invalid boundary
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(1), std::range_error);

        // Add more regions and test boundaries
        CPPUNIT_ASSERT(state.addRegion(30, 40, { 10, 10, 20, 20 }));
        CPPUNIT_ASSERT_NO_THROW(state.getRegionByIndex(1));
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(2), std::range_error);
    }

    void testGetRegionByIndexExtremeValues()
    {
        RenderingState state;
        CPPUNIT_ASSERT(state.addRegion(1, 1, { 0, 0, 10, 10 }));

        // Test with maximum size_t values
        const std::size_t MAX_SIZE_T = std::numeric_limits<std::size_t>::max();
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(MAX_SIZE_T), std::range_error);
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(MAX_SIZE_T - 1), std::range_error);

        // Test with large values
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(1000), std::range_error);
        CPPUNIT_ASSERT_THROW(state.getRegionByIndex(100), std::range_error);
    }

    void testUnmarkRegionExtremeIndices()
    {
        RenderingState state;
        CPPUNIT_ASSERT(state.addRegion(1, 1, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.addRegion(2, 2, { 10, 10, 20, 20 }));

        // Test valid boundaries
        CPPUNIT_ASSERT_NO_THROW(state.unmarkRegionAsDirtyByIndex(0));
        CPPUNIT_ASSERT_NO_THROW(state.unmarkRegionAsDirtyByIndex(1));

        // Test invalid boundaries
        CPPUNIT_ASSERT_THROW(state.unmarkRegionAsDirtyByIndex(2), std::range_error);

        // Test extreme values
        const std::size_t MAX_SIZE_T = std::numeric_limits<std::size_t>::max();
        CPPUNIT_ASSERT_THROW(state.unmarkRegionAsDirtyByIndex(MAX_SIZE_T), std::range_error);
        CPPUNIT_ASSERT_THROW(state.unmarkRegionAsDirtyByIndex(1000), std::range_error);
    }

    void testExceptionTypeVerification()
    {
        RenderingState state;

        // Verify specific exception types are thrown
        try
        {
            state.getRegionByIndex(0);
            CPPUNIT_ASSERT(false); // Should not reach here
        }
        catch (const std::range_error& e)
        {
            CPPUNIT_ASSERT(true); // Expected exception type
        }
        catch (...)
        {
            CPPUNIT_ASSERT(false); // Wrong exception type
        }

        try
        {
            state.unmarkRegionAsDirtyByIndex(0);
            CPPUNIT_ASSERT(false); // Should not reach here
        }
        catch (const std::range_error& e)
        {
            CPPUNIT_ASSERT(true); // Expected exception type
        }
        catch (...)
        {
            CPPUNIT_ASSERT(false); // Wrong exception type
        }
    }

    // State management and dirty flag tests
    void testMarkAllRegionsEmptyState()
    {
        RenderingState state;

        // markAllRegionsAsDirty on empty state should not crash
        CPPUNIT_ASSERT_NO_THROW(state.markAllRegionsAsDirty());
        CPPUNIT_ASSERT(state.getRegionCount() == 0);
    }

    void testRemoveAllRegionsMultipleCalls()
    {
        RenderingState state;

        // Add some regions
        CPPUNIT_ASSERT(state.addRegion(1, 1, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.addRegion(2, 2, { 10, 10, 20, 20 }));
        CPPUNIT_ASSERT(state.getRegionCount() == 2);

        // Multiple removeAllRegions calls should be safe
        CPPUNIT_ASSERT_NO_THROW(state.removeAllRegions());
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        CPPUNIT_ASSERT_NO_THROW(state.removeAllRegions());
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        CPPUNIT_ASSERT_NO_THROW(state.removeAllRegions());
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        // Should be able to add regions after multiple removes
        CPPUNIT_ASSERT(state.addRegion(10, 20, { 0, 0, 10, 10 }));
        CPPUNIT_ASSERT(state.getRegionCount() == 1);
    }

    void testDirtyFlagStateVerification()
    {
        RenderingState state;

        // Add regions
        for (int i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 
                                         static_cast<std::uint8_t>(i), 
                                         { i, i, i + 10, i + 10 }));
        }

        // Verify all start as dirty
        for (std::size_t i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == true);
        }

        // Unmark some regions
        state.unmarkRegionAsDirtyByIndex(1);
        state.unmarkRegionAsDirtyByIndex(3);

        // Verify selective unmarking
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_dirty == true);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_dirty == false);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_dirty == true);
        CPPUNIT_ASSERT(state.getRegionByIndex(3).m_dirty == false);
        CPPUNIT_ASSERT(state.getRegionByIndex(4).m_dirty == true);

        // Mark all as dirty again
        state.markAllRegionsAsDirty();

        // Verify all are dirty again
        for (std::size_t i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == true);
        }
    }

    void testDirtyFlagInitialState()
    {
        RenderingState state;

        // Every newly added region should start as dirty
        for (int i = 0; i < 10; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 1, { 0, 0, 10, 10 }));
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == true);
        }
    }

    // Integration and complex scenario tests
    void testComplexOperationSequences()
    {
        RenderingState state;
        const Rectangle DISPLAY_BOUNDS = { 0, 0, 1920, 1080 };
        const Rectangle WINDOW_BOUNDS = { 100, 100, 800, 600 };

        // Phase 1: Setup bounds and add regions
        state.setBounds(DISPLAY_BOUNDS, WINDOW_BOUNDS);
        
        for (int i = 0; i < 8; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 
                                         static_cast<std::uint8_t>(i * 2), 
                                         { i * 10, i * 20, i * 10 + 50, i * 20 + 60 }));
        }

        CPPUNIT_ASSERT(state.getRegionCount() == 8);

        // Phase 2: Mark and unmark operations
        state.markAllRegionsAsDirty();
        for (std::size_t i = 0; i < 8; i += 2)
        {
            state.unmarkRegionAsDirtyByIndex(i);
        }

        // Verify selective dirty state
        for (std::size_t i = 0; i < 8; ++i)
        {
            if (i % 2 == 0)
            {
                CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == false);
            }
            else
            {
                CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == true);
            }
        }

        // Phase 3: Remove all and re-add
        state.removeAllRegions();
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        // Bounds should persist
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x2 == 1920);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 800);

        // Re-add regions
        for (int i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i + 100), 
                                         static_cast<std::uint8_t>(i + 200), 
                                         { i, i, i + 20, i + 30 }));
        }

        CPPUNIT_ASSERT(state.getRegionCount() == 5);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == 100);
        CPPUNIT_ASSERT(state.getRegionByIndex(4).m_version == 204);
    }

    void testStatePersistenceAfterBounds()
    {
        RenderingState state;

        // Add regions first
        CPPUNIT_ASSERT(state.addRegion(10, 20, { 10, 20, 100, 200 }));
        CPPUNIT_ASSERT(state.addRegion(30, 40, { 30, 40, 300, 400 }));
        CPPUNIT_ASSERT(state.addRegion(50, 60, { 50, 60, 500, 600 }));

        // Mark some as not dirty
        state.unmarkRegionAsDirtyByIndex(1);

        // Change bounds multiple times
        state.setBounds({ 0, 0, 1000, 1000 }, { 100, 100, 900, 900 });
        state.setBounds({ -100, -200, 2000, 3000 }, { 0, 0, 1920, 1080 });

        // Verify regions are unchanged
        CPPUNIT_ASSERT(state.getRegionCount() == 3);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_id == 10);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_version == 20);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_rectangle.m_x1 == 10);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_dirty == true);

        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_id == 30);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_version == 40);
        CPPUNIT_ASSERT(state.getRegionByIndex(1).m_dirty == false);

        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_id == 50);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_version == 60);
        CPPUNIT_ASSERT(state.getRegionByIndex(2).m_dirty == true);

        // Verify new bounds are set
        CPPUNIT_ASSERT(state.getDisplayBounds().m_x1 == -100);
        CPPUNIT_ASSERT(state.getDisplayBounds().m_y1 == -200);
        CPPUNIT_ASSERT(state.getWindowBounds().m_x2 == 1920);
        CPPUNIT_ASSERT(state.getWindowBounds().m_y2 == 1080);
    }

    void testStateConsistencyAfterFailures()
    {
        RenderingState state;

        // Fill to capacity
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 1, { 0, 0, 10, 10 }));
        }

        // Attempt to add beyond capacity - should fail
        CPPUNIT_ASSERT(!state.addRegion(100, 200, { 100, 200, 300, 400 }));

        // Verify state is unchanged after failure
        CPPUNIT_ASSERT(state.getRegionCount() == 16);
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_id == i);
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_version == 1);
        }

        // Test exception doesn't corrupt state
        try
        {
            state.getRegionByIndex(20);
        }
        catch (const std::range_error&)
        {
            // Expected
        }

        // State should still be valid
        CPPUNIT_ASSERT(state.getRegionCount() == 16);
        CPPUNIT_ASSERT_NO_THROW(state.getRegionByIndex(15));
    }

    void testMaximumCapacityScenarios()
    {
        RenderingState state;

        // Fill exactly to capacity
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 
                                         static_cast<std::uint8_t>(255 - i), 
                                         { static_cast<std::int32_t>(i * 100), 
                                           static_cast<std::int32_t>(i * 200), 
                                           static_cast<std::int32_t>(i * 100 + 50), 
                                           static_cast<std::int32_t>(i * 200 + 60) }));
        }

        // Test all operations at capacity
        CPPUNIT_ASSERT_NO_THROW(state.markAllRegionsAsDirty());
        
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(state.unmarkRegionAsDirtyByIndex(i));
            CPPUNIT_ASSERT(state.getRegionByIndex(i).m_dirty == false);
        }

        CPPUNIT_ASSERT_NO_THROW(state.markAllRegionsAsDirty());

        // Remove all and verify clean state
        state.removeAllRegions();
        CPPUNIT_ASSERT(state.getRegionCount() == 0);

        // Should be able to fill again
        for (std::size_t i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i + 50), 1, { 0, 0, 10, 10 }));
        }

        CPPUNIT_ASSERT(state.getRegionCount() == 16);
    }

    void testRectangleStorageAccuracy()
    {
        RenderingState state;
        const std::int32_t MAX_INT = std::numeric_limits<std::int32_t>::max();
        const std::int32_t MIN_INT = std::numeric_limits<std::int32_t>::min();

        struct TestRect
        {
            Rectangle rect;
            const char* description;
        };

        TestRect testRects[] = {
            { { 0, 0, 0, 0 }, "Zero rectangle" },
            { { -1000, -2000, -100, -200 }, "All negative" },
            { { 1000, 2000, 100, 200 }, "Inverted positive" },
            { { -100, 200, 300, -400 }, "Mixed signs inverted" },
            { { MIN_INT, MIN_INT, MAX_INT, MAX_INT }, "Extreme values" },
            { { 12345, 67890, 23456, 78901 }, "Arbitrary values" }
        };

        for (std::size_t i = 0; i < sizeof(testRects) / sizeof(testRects[0]); ++i)
        {
            CPPUNIT_ASSERT(state.addRegion(static_cast<std::uint8_t>(i), 
                                         static_cast<std::uint8_t>(i), 
                                         testRects[i].rect));

            const Rectangle& stored = state.getRegionByIndex(i).m_rectangle;
            CPPUNIT_ASSERT(stored.m_x1 == testRects[i].rect.m_x1);
            CPPUNIT_ASSERT(stored.m_y1 == testRects[i].rect.m_y1);
            CPPUNIT_ASSERT(stored.m_x2 == testRects[i].rect.m_x2);
            CPPUNIT_ASSERT(stored.m_y2 == testRects[i].rect.m_y2);
        }
    }

    void testRegionInfoMemberAccess()
    {
        RenderingState state;
        const Rectangle TEST_RECT = { 100, 200, 300, 400 };

        CPPUNIT_ASSERT(state.addRegion(42, 84, TEST_RECT));

        const RenderingState::RegionInfo& info = state.getRegionByIndex(0);

        // Verify all members are accessible
        CPPUNIT_ASSERT(info.m_id == 42);
        CPPUNIT_ASSERT(info.m_version == 84);
        CPPUNIT_ASSERT(info.m_rectangle.m_x1 == 100);
        CPPUNIT_ASSERT(info.m_rectangle.m_y1 == 200);
        CPPUNIT_ASSERT(info.m_rectangle.m_x2 == 300);
        CPPUNIT_ASSERT(info.m_rectangle.m_y2 == 400);
        CPPUNIT_ASSERT(info.m_dirty == true);

        // Test dirty flag modification
        state.unmarkRegionAsDirtyByIndex(0);
        CPPUNIT_ASSERT(state.getRegionByIndex(0).m_dirty == false);
    }

    void testConstCorrectnessValidation()
    {
        RenderingState state;
        CPPUNIT_ASSERT(state.addRegion(10, 20, { 10, 20, 100, 200 }));

        // Test const access
        const RenderingState& constState = state;

        CPPUNIT_ASSERT_NO_THROW(constState.getDisplayBounds());
        CPPUNIT_ASSERT_NO_THROW(constState.getWindowBounds());
        CPPUNIT_ASSERT_NO_THROW(constState.getRegionCount());
        CPPUNIT_ASSERT_NO_THROW(constState.getRegionByIndex(0));

        CPPUNIT_ASSERT(constState.getRegionCount() == 1);
        CPPUNIT_ASSERT(constState.getRegionByIndex(0).m_id == 10);
        CPPUNIT_ASSERT(constState.getRegionByIndex(0).m_version == 20);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(RenderingStateTest);
