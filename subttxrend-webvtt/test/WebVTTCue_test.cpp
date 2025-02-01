/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <WebVTTCue.hpp>
#include <WebVTTExceptions.hpp>

#include <iostream>

using namespace subttxrend::webvttengine;
using subttxrend::webvttengine::WebVTTCue;

#define INT32(a) static_cast<std::int32_t>(a)
class WebVTTCueTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTCueTest);
CPPUNIT_TEST(testDefaultCueValues);
CPPUNIT_TEST(testSetCueProperty);
CPPUNIT_TEST(testCueOrdering);
CPPUNIT_TEST(testComputedValues);
CPPUNIT_TEST_SUITE_END();
public:
    class CueTest : public WebVTTCue
    {
    public:
        explicit CueTest(Timing m_timing) : WebVTTCue(m_timing) {}
    };

    void setUp()
    {

    }

    void tearDown()
    {

    }

    static Timing buildTp(const std::uint64_t start, const std::uint64_t end)
    {
        return Timing(TimePoint(start), TimePoint(end));
    }

    void testCueOrdering()
    {
        std::list<WebVTTCue> cuelist;

        WebVTTCue cue1 {buildTp(1000, 3000)};
        cue1.identifier("1");
        WebVTTCue cue2 {buildTp(3000, 5000)};
        cue2.identifier("2");

        cuelist.push_back(cue2);
        cuelist.push_back(cue1);
        CPPUNIT_ASSERT(cuelist.front().identifier() == "2");
        cuelist.sort();
        CPPUNIT_ASSERT(cuelist.front().identifier() == "1");

        std::list<WebVTTCue> cuelist2;
        WebVTTCue cue3 {buildTp(2000, 4000)};
        cue3.identifier("3");
        WebVTTCue cue4 {buildTp(4000, 6000)};
        cue4.identifier("4");

        cuelist2.push_back(cue3);
        cuelist2.push_back(cue4);
        cuelist.merge(cuelist2);
        CPPUNIT_ASSERT(cuelist.front().identifier() == "1");
        cuelist.pop_front();
        CPPUNIT_ASSERT(cuelist.front().identifier() == "3");
        cuelist.pop_front();
        CPPUNIT_ASSERT(cuelist.front().identifier() == "2");
        cuelist.pop_front();
        CPPUNIT_ASSERT(cuelist.front().identifier() == "4");
        cuelist.pop_front();
        CPPUNIT_ASSERT(cuelist.empty());
    }

    void testDefaultCueValues()
    {
        // |-------------------------|-------------------------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto defaultCue = WebVTTCue(buildTp(1000, 3000));
        WebVTTCue::CueBox cuePosition = defaultCue.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(5000), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(10000), cuePosition.computedSizeVwH);
    }

    static int GetSize(WebVTTCue cue)
    {
        WebVTTCue::CueBox cueBox = cue.cueBox();
        return (cueBox.computedSizeVwH);
    }

    static int GetLinePercentage(WebVTTCue cue)
    {
        WebVTTCue::CueBox cueBox = cue.cueBox();
        return (cueBox.lineVhH);
    }

    void testSetCueProperty()
    {
        WebVTTCue cue(buildTp(1000, 3000));
        SettingsMap settings;

        //Check good values
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27%,line-left"}}));
        cue = WebVTTCue(buildTp(1000, 3000));
        //Check no throw on auto
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27%,auto"}}));
        cue = WebVTTCue(buildTp(1000, 3000));
        //Check some good percentages
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"size", "5.34%"}}));
        CPPUNIT_ASSERT_EQUAL(534, GetSize(cue));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"size", "5.347%"}}));
        CPPUNIT_ASSERT_EQUAL(535, GetSize(cue));
        cue = WebVTTCue(buildTp(1000, 3000));
        
        //Check some bad percentages
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "blah"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "123%"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "-52%"}}));
        //Still the default
        CPPUNIT_ASSERT_EQUAL(5000, cue.cueBox().computedPositionVwH);
        
        cue = WebVTTCue(buildTp(1000, 3000));
        //Check some bad values - should always be a %age
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27,line-left"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27,lineleft"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"position", "27%line-left"}}));
        //Still the default
        CPPUNIT_ASSERT_EQUAL(5000, cue.cueBox().computedPositionVwH);

        cue = WebVTTCue(buildTp(1000, 3000));
        //Line values - can be number (+ or -) or percentage
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "53%,start"}}));
        CPPUNIT_ASSERT_EQUAL(5300, GetLinePercentage(cue));

        cue = WebVTTCue(buildTp(1000, 3000));
        //All valid
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "-1,start"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "8,end"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "auto,center"}}));
        cue = WebVTTCue(buildTp(1000, 3000));
        //Bad line-align type and params the wrong way around
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "27,line-left"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"line", "start,53%"}}));
        //Still the default
        CPPUNIT_ASSERT_EQUAL(constants::kCueAutoSetting, cue.cueBox().lineVhH);

        cue = WebVTTCue(buildTp(1000, 3000));
        //Size must be valid percentage
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"size", "120%"}}));
        //Still the default
        CPPUNIT_ASSERT_EQUAL(10000, cue.cueBox().computedSizeVwH);

        cue = WebVTTCue(buildTp(1000, 3000));
        //Chaeck for valid aligns
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"align", "right"}}));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"align", "middle"}}));
        CPPUNIT_ASSERT(cue.cueBox().cueTextAlign == WebVTTCue::AlignType::kRight);
        
        cue = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"align", "center"}}));
        
        cue = WebVTTCue(buildTp(1000, 3000));
        //Check region is parsed OK
        CPPUNIT_ASSERT_NO_THROW(cue.addCueSettings({{"region", "r1"}}));
        // CPPUNIT_ASSERT_EQUAL(4000, cue.region().width_vw_h);
        // CPPUNIT_ASSERT_EQUAL(7000, cue.cueBox().computedPositionVwH);
        // CPPUNIT_ASSERT_EQUAL(4000, cue.cueBox().computedSizeVwH);
        // //A bit of a hack - this will change in response to different default line heights
        // //but is hardcoded to the anchor and lines settings above
        // CPPUNIT_ASSERT_EQUAL(9000 - (constants::kDefaultLineHeight) * 3, cue.cueBox().lineVhH);
    }

    void testComputedValues()
    {
        WebVTTCue::CueBox cuePosition;
        //          pos 20     size 40
        // |----------|----------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto cue1 = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue1.addCueSettings({{"position", "20%"}}));

        cuePosition = cue1.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(2000), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(4000), cuePosition.computedSizeVwH);

        //     10                    50                  90
        //     |---------------------|--------------------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto cue2 = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue2.addCueSettings({{"size", "80%"}}));

        cuePosition = cue2.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(5000), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(8000), cuePosition.computedSizeVwH);

        //                                         size 80
        // |---------------------|---------------------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto cue3 = WebVTTCue(buildTp(100, 300));
        //Position align:auto uses the text align
        CPPUNIT_ASSERT_NO_THROW(cue3.addCueSettings({{"size", "80%"}, {"align", "left"}}));

        cuePosition = cue3.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(0), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(8000), cuePosition.computedSizeVwH);

        //           20                                       100
        //            |----------------------------------------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto cue4 = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue4.addCueSettings({{"size", "80%"}, {"align", "right"}}));

        cuePosition = cue4.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(10000), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(8000), cuePosition.computedSizeVwH);

        //                               60          80       100
        //                                |----------|---------|
        // |-------------------------|-------------------------|
        // 0                         50                       100
        auto cue5 = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue5.addCueSettings({{"position", "80%,center"}, {"align", "right"}}));

        cuePosition = cue5.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(8000), cuePosition.computedPositionVwH);
        CPPUNIT_ASSERT_EQUAL(INT32(constants::kCueAutoSetting), cuePosition.lineVhH);
        CPPUNIT_ASSERT_EQUAL(INT32(4000), cuePosition.computedSizeVwH);

        // 50% line == 50% of the screen height
        auto cue6 = WebVTTCue(buildTp(1000, 3000));
        CPPUNIT_ASSERT_NO_THROW(cue6.addCueSettings({{"line", "50%"}}));
        cuePosition = cue6.cueBox();
        CPPUNIT_ASSERT_EQUAL(INT32(5000), cuePosition.lineVhH);

        // // -4 line == 4 lines up from the bottom
        // auto cue7 = WebVTTCue(buildTp(1000, 3000));
        // CPPUNIT_ASSERT_NO_THROW(cue7.addCueSettings({{"line", "-4"}}));
        // cuePosition = cue7.cueBox();
        // CPPUNIT_ASSERT_EQUAL(INT32(10000 - (4 * constants::kDefaultLineHeight)), cuePosition.lineVhH);

        // // 3 line == 3 lines down from the top
        // auto cue8 = WebVTTCue(buildTp(1000, 3000));
        // CPPUNIT_ASSERT_NO_THROW(cue8.addCueSettings({{"line", "3"}}));
        // cuePosition = cue8.cueBox();
        // CPPUNIT_ASSERT_EQUAL(INT32(3 * constants::kDefaultLineHeight), cuePosition.lineVhH);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTCueTest);
