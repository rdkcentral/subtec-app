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
    CPPUNIT_TEST(testTimePointComparisonOperators);
    CPPUNIT_TEST(testTimePointToStrAndToMilliseconds);
    CPPUNIT_TEST(testTimePointInvalidInput);
    CPPUNIT_TEST(testTimingConstructionAndGetters);
    CPPUNIT_TEST(testTimingIsOverlappingAndIsContinous);
    CPPUNIT_TEST(testTimingMerge);
    CPPUNIT_TEST(testTimingOperators);
    CPPUNIT_TEST(testTimingApplyOffset);
    CPPUNIT_TEST(testTimingToStr);
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

    void testTimePointComparisonOperators()
    {
        using namespace subttxrend::ttmlengine;
        TimePoint t1(1h, 2min, 3s, 4ms);
        TimePoint t2(1h, 2min, 3s, 4ms);
        TimePoint t3(1h, 2min, 3s, 5ms);
        CPPUNIT_ASSERT(t1 == t2);
        CPPUNIT_ASSERT(t1 != t3);
        CPPUNIT_ASSERT(t1 < t3);
        CPPUNIT_ASSERT(t3 > t2);
        CPPUNIT_ASSERT(t1 <= t2);
        CPPUNIT_ASSERT(t1 <= t3);
        CPPUNIT_ASSERT(t3 >= t2);
    }

    void testTimePointToStrAndToMilliseconds()
    {
        using namespace subttxrend::ttmlengine;
        TimePoint t(1h, 2min, 3s, 4ms);
        std::string str = t.toStr();
        CPPUNIT_ASSERT(str.find("01:02:03.004") != std::string::npos);
        CPPUNIT_ASSERT(t.toMilliseconds().count() == ((1*3600+2*60+3)*1000+4));
    }

    void testTimePointInvalidInput()
    {
        using namespace subttxrend::ttmlengine;
        TimePoint t1(1h, 61min, 0s); // invalid minutes
        TimePoint t2(1h, 2min, 61s); // invalid seconds
        // Should default to 0
        CPPUNIT_ASSERT(t1 == TimePoint());
        CPPUNIT_ASSERT(t2 == TimePoint());
    }

    void testTimingConstructionAndGetters()
    {
        using namespace subttxrend::ttmlengine;
        TimePoint begin(1h, 2min, 3s, 4ms);
        TimePoint end(1h, 3min, 3s, 4ms);
        Timing timing(begin, end);
        CPPUNIT_ASSERT(timing.getStartTimeRef() == begin);
        CPPUNIT_ASSERT(timing.getEndTimeRef() == end);
        const Timing& ctiming = timing;
        CPPUNIT_ASSERT(ctiming.getStartTimeRef() == begin);
        CPPUNIT_ASSERT(ctiming.getEndTimeRef() == end);
    }

    void testTimingIsOverlappingAndIsContinous()
    {
        using namespace subttxrend::ttmlengine;
        Timing t1(TimePoint(0h,0min,0s,0ms), TimePoint(0h,0min,1s,0ms));
        Timing t2(TimePoint(0h,0min,0s,500ms), TimePoint(0h,0min,1s,500ms));
        Timing t3(TimePoint(0h,0min,1s,0ms), TimePoint(0h,0min,2s,0ms));
        Timing t4(TimePoint(0h,0min,2s,0ms), TimePoint(0h,0min,3s,0ms));
        CPPUNIT_ASSERT(t1.isOverlapping(t2));
        CPPUNIT_ASSERT(!t1.isOverlapping(t4));
        CPPUNIT_ASSERT(t1.isContinous(t3));
        CPPUNIT_ASSERT(!t1.isContinous(t4));
    }

    void testTimingMerge()
    {
        using namespace subttxrend::ttmlengine;
        Timing t1(TimePoint(0h,0min,0s,0ms), TimePoint(0h,0min,1s,0ms));
        Timing t2(TimePoint(0h,0min,0s,500ms), TimePoint(0h,0min,1s,500ms));
        t1.merge(t2);
        CPPUNIT_ASSERT(t1.getStartTimeRef() == TimePoint(0h,0min,0s,0ms));
        CPPUNIT_ASSERT(t1.getEndTimeRef() == TimePoint(0h,0min,1s,500ms));
    }

    void testTimingOperators()
    {
        using namespace subttxrend::ttmlengine;
        Timing t1(TimePoint(0h,0min,0s,0ms), TimePoint(0h,0min,1s,0ms));
        Timing t2(TimePoint(0h,0min,0s,0ms), TimePoint(0h,0min,1s,0ms));
        Timing t3(TimePoint(0h,0min,0s,0ms), TimePoint(0h,0min,2s,0ms));
        CPPUNIT_ASSERT(t1 == t2);
        CPPUNIT_ASSERT(!(t1 == t3));
        CPPUNIT_ASSERT(t1 < t3);
    }

    void testTimingApplyOffset()
    {
        using namespace subttxrend::ttmlengine;
        Timing t(TimePoint(0h,0min,1s,0ms), TimePoint(0h,0min,2s,0ms));
        t.applyOffset(500);
        CPPUNIT_ASSERT(t.getStartTimeRef() == TimePoint(0h,0min,1s,500ms));
        CPPUNIT_ASSERT(t.getEndTimeRef() == TimePoint(0h,0min,2s,500ms));
    }

    void testTimingToStr()
    {
        using namespace subttxrend::ttmlengine;
        Timing t(TimePoint(1h, 2min, 3s, 4ms), TimePoint(1h, 3min, 4s, 5ms));
        std::string str = t.toStr();
        CPPUNIT_ASSERT(str.find("01:02:03.004-01:03:04.005") != std::string::npos);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TtmlTimingTest );

