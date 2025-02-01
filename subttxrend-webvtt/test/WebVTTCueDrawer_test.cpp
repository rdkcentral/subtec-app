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

#include <subttxrend/gfx/DrawContext.hpp>

#include <WebVTTDraw.hpp>
#include <WebVTTCue.hpp>

using namespace subttxrend::webvttengine;

#define INT32(a) static_cast<std::int32_t>(a)

class WebVTTCueDrawerTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTCueDrawerTest);
CPPUNIT_TEST(testCreateDrawer);
CPPUNIT_TEST(testSetUpRegion);
CPPUNIT_TEST_SUITE_END();

public:
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

    void testCreateDrawer()
    {
        std::unique_ptr<WebVTTDraw> drawer;
        CPPUNIT_ASSERT_NO_THROW(drawer = std::make_unique<WebVTTDraw>(1200, 1080));
    }

    void testSetUpRegion()
    {
        auto cue = std::make_shared<WebVTTCue>(buildTp(3000, 5000));
        const std::int32_t Width = 1920, Height = 1080;
        std::unique_ptr<WebVTTDraw> drawer;

        CPPUNIT_ASSERT_NO_THROW(cue->addCueSettings({{"position", "20%"}}, {}));
        CPPUNIT_ASSERT_NO_THROW(drawer = std::make_unique<WebVTTDraw>(Width, Height));
        //CPPUNIT_ASSERT_NO_THROW(drawer->SetUpRegionFromCue(cue, regionptr));
        //CPPUNIT_ASSERT_EQUAL(std::int32_t(384), regionptr->penX());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTCueDrawerTest);
