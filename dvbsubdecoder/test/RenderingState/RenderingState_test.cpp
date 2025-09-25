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
    CPPUNIT_TEST(testBadIndex);CPPUNIT_TEST_SUITE_END()
    ;

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
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(RenderingStateTest);
