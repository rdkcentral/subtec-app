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

#include "Display.hpp"

using dvbsubdecoder::Display;
using dvbsubdecoder::Rectangle;

class StatusTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StatusTest );
    CPPUNIT_TEST(testSimple);CPPUNIT_TEST_SUITE_END()
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
        Rectangle DEFAULT_RECTANGLE =
        { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };

        std::uint8_t SPECIFIC_VERSION = 2;

        Rectangle DISPLAY_RECTANGLE =
        { 5, 6, 20, 21 };

        Rectangle WINDOW_RECTANGLE =
        { 10, 11, 15, 17 };

        Display display;

        // check defaults
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == DEFAULT_RECTANGLE);
        CPPUNIT_ASSERT(display.getWindowBounds() == DEFAULT_RECTANGLE);

        // check set
        display.set(SPECIFIC_VERSION, DISPLAY_RECTANGLE, WINDOW_RECTANGLE);

        CPPUNIT_ASSERT(display.getVersion() == SPECIFIC_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == DISPLAY_RECTANGLE);
        CPPUNIT_ASSERT(display.getWindowBounds() == WINDOW_RECTANGLE);

        // check reset
        display.reset();

        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == DEFAULT_RECTANGLE);
        CPPUNIT_ASSERT(display.getWindowBounds() == DEFAULT_RECTANGLE);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(StatusTest);
