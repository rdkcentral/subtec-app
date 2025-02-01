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
#include "Parser/Timing.hpp"

using namespace subttxrend::ttmlengine;

class TtmlTimingTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( TtmlTimingTest );
    CPPUNIT_TEST(timing);
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

    void timing()
    {
        using namespace std::chrono;

        TimePoint tp1, tp2;

        constexpr auto HOUR_MS = 60 * 60 * 1000;
        constexpr auto MINUTE_MS = 60 * 1000;

        TimePoint hourTp{1h, 0min, 0s, 0ms};
        TimePoint min30Tp{0h, 30min, 0s, 0ms};

        CPPUNIT_ASSERT(tp1 == tp1);
        tp1.applyOffset(HOUR_MS);
        CPPUNIT_ASSERT(tp1 > tp2);
        CPPUNIT_ASSERT(tp1 == hourTp);

        tp2.applyOffset(30 * MINUTE_MS);
        CPPUNIT_ASSERT(min30Tp == tp2);

        CPPUNIT_ASSERT(hourTp > min30Tp);
        min30Tp.applyOffset(30 * MINUTE_MS);
        CPPUNIT_ASSERT(hourTp == min30Tp);
        min30Tp.applyOffset(30 * MINUTE_MS);
        CPPUNIT_ASSERT(hourTp < min30Tp);

        min30Tp.applyOffset(-30 * MINUTE_MS);
        CPPUNIT_ASSERT(hourTp == min30Tp);

        hourTp.applyOffset(-2* HOUR_MS);
        auto hourMinus = TimePoint{-1h, 0min, 0s, 0ms};
        CPPUNIT_ASSERT(hourTp == hourMinus);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TtmlTimingTest );

