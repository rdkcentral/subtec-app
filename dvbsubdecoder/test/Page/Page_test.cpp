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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PageTest);
