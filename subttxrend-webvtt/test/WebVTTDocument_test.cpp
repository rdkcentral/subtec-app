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
#include <sstream>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <WebVTTDocument.hpp>
#include <WebVTTExceptions.hpp>

using namespace subttxrend::webvttengine;

class WebVTTDocumentTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTDocumentTest);
CPPUNIT_TEST(testWebVTTHeader);
CPPUNIT_TEST(testXTimestampMap);
CPPUNIT_TEST(testParseWebVTTCueTime);
CPPUNIT_TEST(testParseWebVTTCueSettings);
CPPUNIT_TEST(testTopLevel);
CPPUNIT_TEST(testMerge);
CPPUNIT_TEST(testBadSetting);
CPPUNIT_TEST(testBadTime);
CPPUNIT_TEST(testWhenNoCues);
CPPUNIT_TEST(testTwoSpaces);
CPPUNIT_TEST(testTimeOffset);
CPPUNIT_TEST(testRegion);
CPPUNIT_TEST(testStyle);
CPPUNIT_TEST_SUITE_END();

public:
    static Timing buildTp(const std::uint64_t start, const std::uint64_t end)
    {
        return Timing(TimePoint(start), TimePoint(end));
    }

    void setUp()
    {

    }

    void tearDown()
    {

    }
    
    void testStyle()
    {
std::istringstream style(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

STYLE
::cue {
  background-image: linear-gradient(to bottom, dimgray, lightgray);
  color: papayawhip;
}
/* Style blocks cannot use blank lines nor "dash dash greater than" */

NOTE comment blocks can be used between style blocks.

STYLE
::cue(b) {
  color: peachpuff;
}

00:01:29.000 --> 00:01:31.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
line:75%

)");
        CueList list;
        WebVTTDocument doc;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(style));
        CPPUNIT_ASSERT_EQUAL((std::size_t)1, list.size());
    }
    
    void testTwoSpaces()
    {
std::istringstream twospaces(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00


00:01:29.000 --> 00:01:31.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
line:75%

)");
        CueList list;
        WebVTTDocument doc;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(twospaces));
        CPPUNIT_ASSERT_EQUAL((std::size_t)1, list.size());
    }
    
    void testTimeOffset()
    {
std::istringstream time_offset(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:900000,LOCAL:00:00:05

00:01:29.000 --> 00:01:31.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
line:75%

)");
        CueList list;
        WebVTTDocument doc;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(time_offset));
        CPPUNIT_ASSERT_EQUAL((std::size_t)1, list.size());
        auto list_iter = list.begin();
        //89s + 10s PTS - 5s LOCAL offset
        CPPUNIT_ASSERT(TimePoint(94000) == (*list_iter)->startTime());
        CPPUNIT_ASSERT(TimePoint(96000) == (*list_iter)->endTime());
    }
    
    void testRegion()
    {
std::istringstream region_header(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

REGION
id:fred
width:30%
lines:3
regionanchor:0%,100%
viewportanchor:10%,90%
scroll:up

REGION
id:bill
width:40%
lines:3
regionanchor:100%,100%
viewportanchor:90%,90%
scroll:up

00:01:29.000 --> 00:01:31.000 line:75% region:bill
English subtitle 15 -Forced- (00:01:27.000)
line:75%

)");
        CueList list;
        WebVTTDocument doc;
        RegionMap regionMap;
        Region region;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, regionMap) = doc.parseCueList(region_header));
        CPPUNIT_ASSERT_EQUAL((std::size_t)1, list.size());
        CPPUNIT_ASSERT_NO_THROW(region = regionMap.at("bill"));
        auto list_iter = list.begin();

        CPPUNIT_ASSERT_EQUAL(std::string{"bill"}, region.id);
        CPPUNIT_ASSERT(Region::Scroll::kUp == region.scroll);
        Region::Position viewportanchor{9000, 9000};
        CPPUNIT_ASSERT(viewportanchor == region.viewport_anchor);
        Region::Position regionanchor{10000, 10000};
        CPPUNIT_ASSERT(regionanchor == region.region_anchor);
    }
    
    void testWhenNoCues()
    {
std::istringstream nocue(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

)");
        CueList list;
        WebVTTDocument doc;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(nocue));
        CPPUNIT_ASSERT(list.empty());
    }
    
    void testBadSetting()
    {
        std::istringstream bad_setting{
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

00:01:29.000 --> 00:01:31.000 wrong:75% line:50%
English subtitle 15 -Forced- (00:01:27.000)
one

00:01:31.000 --> 00:01:33.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
two

)"};
        CueList list;
        WebVTTDocument doc;
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(bad_setting));
        CPPUNIT_ASSERT_EQUAL((std::size_t)2, list.size());
        auto list_iter = list.begin();
        //If there's one bad setting, set everything to defaults but don't throw
        CPPUNIT_ASSERT(TimePoint(89000) == (*list_iter)->startTime());
        CPPUNIT_ASSERT_EQUAL(10000, (*list_iter)->cueBox().computedSizeVwH);
        CPPUNIT_ASSERT_EQUAL(std::size_t{2}, (*list_iter)->lines().size());
        list_iter++;
        //Second one should be fine
        CPPUNIT_ASSERT(TimePoint(91000) == (*list_iter)->startTime());
        CPPUNIT_ASSERT_EQUAL(0, (*list_iter)->cueBox().lineVhH);
        CPPUNIT_ASSERT_EQUAL(std::size_t{2}, (*list_iter)->lines().size());
    }

    void testBadTime()
    {
        std::string bad_time{
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

00:01:XX.000 --> 00:01:31.000 wrong:75% line:50%
English subtitle 15 -Forced- (00:01:27.000)
one

00:01:31.000 --> 00:01:33.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
two

)"};
        CueList list;
        WebVTTDocument doc;
        std::istringstream ss(bad_time);
        
        CPPUNIT_ASSERT_NO_THROW(std::tie(list, std::ignore) = doc.parseCueList(ss));
        CPPUNIT_ASSERT_EQUAL((std::size_t)1, list.size());
        auto list_iter = list.begin();
        
        //If there's a bad time, abort that cue and go to the next one
        CPPUNIT_ASSERT(TimePoint(91000) == (*list_iter)->startTime());
        CPPUNIT_ASSERT_EQUAL(10000, (*list_iter)->cueBox().computedSizeVwH);
        CPPUNIT_ASSERT_EQUAL(std::size_t{2}, (*list_iter)->lines().size());
    }

    void testMerge()
    {
        std::string first_two{
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

00:01:29.000 --> 00:01:31.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
one

00:01:31.000 --> 00:01:33.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
two

)"};
        std::string second_two{
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

00:01:33.000 --> 00:01:35.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
three

00:01:35.000 --> 00:01:37.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
four

)"};

        CueList list1, list2;
        WebVTTDocument doc;
        std::istringstream first, second;
        
        first.str(first_two);
        second.str(second_two);

        CPPUNIT_ASSERT_NO_THROW(std::tie(list1, std::ignore) = doc.parseCueList(first));
        CPPUNIT_ASSERT_NO_THROW(std::tie(list2, std::ignore) = doc.parseCueList(second));

        //This is used when new cues come in - see WebvttEngineImpl.cpp
        list1.merge(list2, [](const CuePtr &a, const CuePtr &b) -> bool
                                { return *a < *b; });
        CPPUNIT_ASSERT_EQUAL(size_t{4}, list1.size());

        auto list_iter = list1.begin();
        CPPUNIT_ASSERT((*list_iter++)->startTime() == TimePoint{89000});
        CPPUNIT_ASSERT((*list_iter++)->startTime() == TimePoint{91000});
        CPPUNIT_ASSERT((*list_iter++)->startTime() == TimePoint{93000});
        CPPUNIT_ASSERT((*list_iter++)->startTime() == TimePoint{95000});

        first.str(second_two);
        first.clear();
        second.str(first_two);
        second.clear();

        CPPUNIT_ASSERT_NO_THROW(std::tie(list1, std::ignore) = doc.parseCueList(first));
        CPPUNIT_ASSERT_NO_THROW(std::tie(list2, std::ignore) = doc.parseCueList(second));

        list1.merge(list2, [](const CuePtr &a, const CuePtr &b) -> bool
                                { return *a < *b; });
        CPPUNIT_ASSERT_EQUAL(size_t{4}, list1.size());

        auto list_iter2 = list1.begin();
        CPPUNIT_ASSERT((*list_iter2++)->startTime() == TimePoint{89000});
        CPPUNIT_ASSERT((*list_iter2++)->startTime() == TimePoint{91000});
        CPPUNIT_ASSERT((*list_iter2++)->startTime() == TimePoint{93000});
        CPPUNIT_ASSERT((*list_iter2++)->startTime() == TimePoint{95000});
    }
    
    void testTopLevel()
    {
        WebVTTDocument parser;

        std::istringstream iss(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:0,LOCAL:00:00:00

00:01:29.000 --> 00:01:31.000 line:75%
English subtitle 15 -Forced- (00:01:27.000)
line:75

00:01:31.000 --> 00:01:33.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
align:start line:0

00:01:33.000 --> 00:01:35.000 align:start line:50%
English subtitle 17 -Unforced- (00:01:33.000)
align:start line:50%

00:01:35.000 --> 00:01:37.000 align:start line:100%
English subtitle 18 -Unforced- (00:01:35.000)
align:start line:100%

00:01:37.000 --> 00:01:39.000 align:middle line:100%
English subtitle 19 -Unforced- (00:01:37.000)
align:middle line:100%

00:01:39.000 --> 00:01:41.000 align:end line:100%
English subtitle 20 -Forced- (00:01:39.000)
align:end line:100%

00:01:41.000 --> 00:01:43.000 align:end line:50%
English subtitle 21 -Unforced- (00:01:41.000)
align:end line:50%

00:01:43.000 --> 00:01:45.000 align:end line:0%
English subtitle 22 -Unforced- (00:01:43.000)
align:end line:0%

00:01:45.000 --> 00:01:47.000 align:middle line:0%
English subtitle 23 -Unforced- (00:01:45.000)
align:middle line:0%

00:01:47.000 --> 00:01:49.000 align:middle line:50%
English subtitle 24 -Unforced- (00:01:47.000)
align:middle line:50%

00:01:49.000 --> 00:01:51.000 align:middle line:100%
English subtitle 25 -Forced- (00:01:49.000)
align:middle line:100%

00:01:51.000 --> 00:01:53.000 position:20% size:55%
English subtitle 26 -Unforced- (00:01:51.000)
position:20% size:55%

00:01:53.000 --> 00:01:55.000
English subtitle 27 -Unforced- (00:01:53.000)
stuff

00:01:55.000 --> 00:01:57.000
English subtitle 28 -Unforced- (00:01:55.000)
Multilines
line 3
line 4
line 5
line 6

00:01:57.000 --> 00:01:59.000
English subtitle 29 -Forced- (00:01:57.000)
Special Characters
œ∑´®†¥¨ˆøπ“
åß∂ƒ©˙∆˚¬
Ω≈ç√∫˜µ≤≥÷

)");

        if (iss)
        {
            CueList list;
            CPPUNIT_ASSERT_NO_THROW_MESSAGE("Parsing file", std::tie(list, std::ignore) = parser.parseCueList(iss));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Test 15 cues", (std::size_t)15, list.size());
            CPPUNIT_ASSERT_MESSAGE("Test last end time", TimePoint(119000) == list.back()->endTime());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Test special chars line count", (std::size_t)5, list.back()->lines().size());
        }
        else
        {
            CPPUNIT_FAIL("Test file failed to open");
        }
    }

    // void testGetlineCrlf()
    // {
    //     std::stringstream ss;
    //     std::string token;
    //     WebVTTDocumentTestFixture doc;

    //     ss.str("one\n\ntwo\r\nthree\rfour\r\nfive");

    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string("one"), token);
    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string(""), token);
    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string("two"), token);
    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string("three"), token);
    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string("four"), token);
    //     doc.getline_crlf(ss, token);
    //     CPPUNIT_ASSERT_EQUAL(std::string("five"), token);
    //     CPPUNIT_ASSERT_MESSAGE("getline_crlf returns false", !doc.getline_crlf(ss, token));
    //     CPPUNIT_ASSERT_EQUAL(std::string(""), token);
    // }

    void testWebVTTHeader()
    {
        WebVTTDocumentTestFixture doc;
        std::istringstream iss;
        std::vector<std::string> goodlines {
            "WEBVTT\n",
            "WEBVTT\r\n",
            "\x0EF\x0BB\x0BFWEBVTT\n",
            "\x0EF\x0BB\x0BFWEBVTT\r\n",
            "\x0EF\x0BB\x0BFWEBVTT \n",
            "\x0EF\x0BB\x0BFWEBVTT - header\n",
            "\x0EF\x0BB\x0BFWEBVTT\t- header\n"
        };

        std::vector<std::string> badlines {
            " WEBVTT\r\n",
            "\x0EF\x0BB\x0BF\n WEBVTT\n",
            "\x0EF\x0BB\x0BF\nWOBVTT\n"
        };

        for (const auto& good : goodlines)
        {
            iss.str(good);
            CPPUNIT_ASSERT_NO_THROW_MESSAGE(good, doc.checkValidWebVTTHeader(iss));
        }

        for (const auto& bad : badlines)
        {
            iss.str(bad);
            CPPUNIT_ASSERT_THROW_MESSAGE(bad, doc.checkValidWebVTTHeader(iss), InvalidCueException);
        }
    }

    void testXTimestampMap()
    {
        WebVTTDocumentTestFixture parser;
        std::map<std::string, std::uint64_t> goodlines = {
            {"X-TIMESTAMP-MAP=MPEGTS:900000,LOCAL:02:00:05", (std::uint64_t)10000},
            {"X-TIMESTAMP-MAP=MPEGTS:900000", (std::uint64_t)10000},
            {"X-TIMESTAMP-MAP=LOCAL:02:00:05", (std::uint64_t)0},
            {"X-TIMESTAMP-MAP=LOCAL:02:00:05.123", (std::uint64_t)0}
        };
        std::list<std::string> badlines = {
            "X-TIMESTAMP-MAPMPEGTS:900000LOCAL:02:00:05",
            "X-TIMESTOMP-MAP=LOCAL:02:00:05,MPEGTS:900000"
        };

        for (const auto& good : goodlines)
        {
            std::cout << good.first << "Krithika" << good.second;
            std::uint64_t ptsOffset = 0;
            auto& str = good.first;
            auto& expectedPts = good.second;

            ptsOffset = parser.parseXTimestampMap(str);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(str, (std::uint64_t)expectedPts, ptsOffset);
        }

        for (const auto& bad : badlines)
        {
            std::uint64_t ptsOffset = 0;
            CPPUNIT_ASSERT_NO_THROW_MESSAGE(bad, ptsOffset = parser.parseXTimestampMap(bad));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(bad, (std::uint64_t)0, ptsOffset);
        }
    }

    void testParseWebVTTCueTime()
    {
        WebVTTDocumentTestFixture parser;
        Timing time;
        std::map<std::string, Timing> good_time = {
            {"00:01:39.123 --> 00:01:41.000 align:center line:100%", buildTp(99123, 101000)},
            {"00:01:39.123\t-->\t00:01:41.000\talign:center\tline:100%", buildTp(99123, 101000)},
            {"00:01:39.123 --> 00:01:41.000 align:middle line:100%", buildTp(99123, 101000)},
            {"100:00:03.123 --> 100:00:04.123", buildTp(360003123, 360004123)}
        };
        std::list<std::string> bad_time = {
            "00:01:37.000-->00:01:39.000",
            "00:01:37.000 -> 00:01:39.000",
            "00:01:37.000 --> blah",
            "00:01:37.000 --> 00:112:39.000"
        };

        for (auto& good : good_time)
        {
            std::istringstream iss(good.first);
            CPPUNIT_ASSERT_NO_THROW_MESSAGE(good.first, time = parser.parseWebVTTCueTime(iss));
            CPPUNIT_ASSERT(good.second == time);
        }

        for (auto& bad : bad_time)
        {
            std::istringstream iss(bad);
            CPPUNIT_ASSERT_THROW_MESSAGE(bad, parser.parseWebVTTCueTime(iss), InvalidCueException);
        }
    }

    void testParseWebVTTCueSettings()
    {
        SettingsMap parsedSettings;
        std::string str;
        WebVTTDocumentTestFixture parser;

        std::vector<std::pair<std::string, SettingsMap>> lines {
            {"", {}},
            {"align:center line:100%", {{"line", "100%"}, {"align", "center"}}},
            {"align:center position:27%,line-left", {{"position", "27%,line-left"}, {"align", "center"}}},
            {"align:center\tline:100%", {{"line", "100%"}, {"align", "center"}}},
            {"align:wibble line:100%", {{"align", "wibble"}, {"line", "100%"}}},
            {"align: center line:100%", {{"line", "100%"}}}
        };

        for (auto const& good : lines)
        {
            const std::string strToParse = good.first;
            const SettingsMap expectedSettings = good.second;
            std::istringstream iss(strToParse);
            CPPUNIT_ASSERT_NO_THROW_MESSAGE(strToParse, parsedSettings = parser.parseWebVTTCueSettings(iss));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(strToParse, expectedSettings.size(), parsedSettings.size());
            for (auto const& setting : expectedSettings)
            {
                const std::string &expectedParam {setting.first};
                const std::string &expectedValue {setting.second};
                CPPUNIT_ASSERT_NO_THROW_MESSAGE(expectedParam, str = parsedSettings.at(expectedParam));
                CPPUNIT_ASSERT_EQUAL_MESSAGE(expectedValue, expectedValue, str);
            }
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTDocumentTest);
