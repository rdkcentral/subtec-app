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

class DisplayTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DisplayTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testVersionBoundaryValues);
    CPPUNIT_TEST(testRectangleBoundaryValues);
    CPPUNIT_TEST(testInvalidRectangleGeometry);
    CPPUNIT_TEST(testWindowBoundsOutsideDisplay);
    CPPUNIT_TEST(testMultipleSetOperations);
    CPPUNIT_TEST(testMultipleResetOperations);
    CPPUNIT_TEST(testSetResetCycles);
    CPPUNIT_TEST(testRectangleEquality);
    CPPUNIT_TEST(testStateConsistency);
    CPPUNIT_TEST(testVersionPersistence);
    CPPUNIT_TEST(testDisplayConstants);
    CPPUNIT_TEST(testConstReferenceStability);
    CPPUNIT_TEST(testIdempotency);
    CPPUNIT_TEST(testExtremeValues);
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

    void testDefaultConstructor()
    {
        // Test default constructor initialization
        Display display;
        
        // Verify initial version is INVALID_VERSION
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        
        // Verify default bounds are SD resolution
        Rectangle expectedDefault = { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };
        CPPUNIT_ASSERT(display.getDisplayBounds() == expectedDefault);
        CPPUNIT_ASSERT(display.getWindowBounds() == expectedDefault);
        
        // Verify display and window bounds are initially identical
        CPPUNIT_ASSERT(display.getDisplayBounds() == display.getWindowBounds());
    }

    void testVersionBoundaryValues()
    {
        Display display;
        Rectangle testRect = { 10, 10, 100, 100 };
        
        // Test minimum valid version (0)
        display.set(0, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == 0);
        
        // Test maximum valid 4-bit version (15)
        display.set(15, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == 15);
        
        // Test INVALID_VERSION as input parameter
        display.set(dvbsubdecoder::INVALID_VERSION, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        
        // Test maximum uint8_t value
        display.set(255, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == 255);
        
        // Test value just above 4-bit range
        display.set(16, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == 16);
    }

    void testRectangleBoundaryValues()
    {
        Display display;
        std::uint8_t version = 1;
        
        // Test zero coordinates
        Rectangle zeroRect = { 0, 0, 0, 0 };
        display.set(version, zeroRect, zeroRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == zeroRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == zeroRect);
        
        // Test negative coordinates
        Rectangle negativeRect = { -100, -50, 10, 20 };
        display.set(version, negativeRect, negativeRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == negativeRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == negativeRect);
        
        // Test large positive coordinates
        Rectangle largeRect = { 100000, 200000, 300000, 400000 };
        display.set(version, largeRect, largeRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == largeRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == largeRect);
        
        // Test mixed sign coordinates
        Rectangle mixedRect = { -50, 25, 75, -10 };
        display.set(version, mixedRect, mixedRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == mixedRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == mixedRect);
    }

    void testInvalidRectangleGeometry()
    {
        Display display;
        std::uint8_t version = 2;
        
        // NOTE: These geometry cases intentionally do NOT trigger normalization.
        // The Display::set contract (as currently implemented) stores rectangles verbatim.
        // If future behavior changes (e.g. auto-normalization/clamping), these assertions
        // will catch that change.

        // Test inverted rectangle (x2 < x1, y2 < y1) – expect stored as-is
        Rectangle invertedRect = { 100, 100, 50, 50 };
        display.set(version, invertedRect, invertedRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == invertedRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == invertedRect);
        CPPUNIT_ASSERT(invertedRect.m_x1 > invertedRect.m_x2); // reinforce expectation
        CPPUNIT_ASSERT(invertedRect.m_y1 > invertedRect.m_y2);
        
        // Test zero-width rectangle – expect stored as-is
        Rectangle zeroWidthRect = { 10, 10, 10, 20 };
        display.set(version, zeroWidthRect, zeroWidthRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == zeroWidthRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == zeroWidthRect);
        
        // Test zero-height rectangle – expect stored as-is
        Rectangle zeroHeightRect = { 10, 10, 20, 10 };
        display.set(version, zeroHeightRect, zeroHeightRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == zeroHeightRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == zeroHeightRect);
        
        // Test point rectangle (zero width and height) – expect stored as-is
        Rectangle pointRect = { 15, 25, 15, 25 };
        display.set(version, pointRect, pointRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == pointRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == pointRect);
    }

    void testWindowBoundsOutsideDisplay()
    {
        Display display;
        // Display rectangle is small; window rectangle extends beyond in all directions
        Rectangle displayRect = { 10, 10, 50, 50 };
        Rectangle windowRect  = { 0,  0,  100, 100 }; // extends above/left and below/right
        display.set(3, displayRect, windowRect);

        // Expect verbatim storage (no clamping or intersection performed)
        CPPUNIT_ASSERT(display.getDisplayBounds() == displayRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == windowRect);
        CPPUNIT_ASSERT(!(display.getDisplayBounds() == display.getWindowBounds()));

        // Further exaggerate out-of-range window (negative + very large)
        Rectangle windowRect2 = { -500, -400, 1000, 900 };
        display.set(4, displayRect, windowRect2);
        CPPUNIT_ASSERT(display.getDisplayBounds() == displayRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == windowRect2);

        // Ensure version updated and rectangles remain independent
        CPPUNIT_ASSERT(display.getVersion() == 4);
    }

    void testMultipleSetOperations()
    {
        Display display;
        
        // First set operation
        Rectangle rect1 = { 10, 20, 30, 40 };
        Rectangle window1 = { 15, 25, 25, 35 };
        display.set(1, rect1, window1);
        CPPUNIT_ASSERT(display.getVersion() == 1);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect1);
        CPPUNIT_ASSERT(display.getWindowBounds() == window1);
        
        // Second set operation with different values
        Rectangle rect2 = { 50, 60, 70, 80 };
        Rectangle window2 = { 55, 65, 65, 75 };
        display.set(3, rect2, window2);
        CPPUNIT_ASSERT(display.getVersion() == 3);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect2);
        CPPUNIT_ASSERT(display.getWindowBounds() == window2);
        
        // Third set operation
        Rectangle rect3 = { 0, 0, 100, 100 };
        Rectangle window3 = { 10, 10, 90, 90 };
        display.set(7, rect3, window3);
        CPPUNIT_ASSERT(display.getVersion() == 7);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect3);
        CPPUNIT_ASSERT(display.getWindowBounds() == window3);
    }

    void testMultipleResetOperations()
    {
        Display display;
        Rectangle defaultRect = { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };
        
        // Set some values first
        Rectangle testRect = { 10, 20, 30, 40 };
        display.set(5, testRect, testRect);
        CPPUNIT_ASSERT(display.getVersion() == 5);
        
        // First reset
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == defaultRect);
        
        // Second reset (should have no effect)
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == defaultRect);
        
        // Third reset
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == defaultRect);
    }

    void testSetResetCycles()
    {
        Display display;
        Rectangle defaultRect = { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };
        
        // Cycle 1: set -> reset
        Rectangle rect1 = { 1, 2, 3, 4 };
        display.set(1, rect1, rect1);
        CPPUNIT_ASSERT(display.getVersion() == 1);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect1);
        
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        
        // Cycle 2: set -> reset
        Rectangle rect2 = { 10, 20, 30, 40 };
        display.set(8, rect2, rect2);
        CPPUNIT_ASSERT(display.getVersion() == 8);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect2);
        
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        
        // Cycle 3: set -> reset
        Rectangle rect3 = { 100, 200, 300, 400 };
        display.set(15, rect3, rect3);
        CPPUNIT_ASSERT(display.getVersion() == 15);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect3);
        
        display.reset();
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
    }

    void testRectangleEquality()
    {
        Display display;
        
        // Test identical rectangles
        Rectangle rect = { 10, 20, 30, 40 };
        Rectangle identicalRect = { 10, 20, 30, 40 };
        display.set(1, rect, identicalRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == display.getWindowBounds());
        
        // Test different rectangles
        Rectangle displayRect = { 0, 0, 100, 100 };
        Rectangle windowRect = { 10, 10, 90, 90 };
        display.set(2, displayRect, windowRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == displayRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == windowRect);
        CPPUNIT_ASSERT(!(display.getDisplayBounds() == display.getWindowBounds()));
        
        // Test rectangle with single coordinate difference
        Rectangle rect1 = { 5, 5, 15, 15 };
        Rectangle rect2 = { 5, 5, 15, 16 }; // Only y2 different
        display.set(3, rect1, rect2);
        CPPUNIT_ASSERT(display.getDisplayBounds() == rect1);
        CPPUNIT_ASSERT(display.getWindowBounds() == rect2);
        CPPUNIT_ASSERT(!(display.getDisplayBounds() == display.getWindowBounds()));
    }

    void testStateConsistency()
    {
        Display display;
        
        // Test consistency after construction
        std::uint8_t version = display.getVersion();
        Rectangle displayBounds = display.getDisplayBounds();
        Rectangle windowBounds = display.getWindowBounds();
        
        CPPUNIT_ASSERT(version == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(displayBounds == windowBounds);
        
        // Test consistency after set
        Rectangle testDisplay = { 0, 0, 640, 480 };
        Rectangle testWindow = { 50, 50, 590, 430 };
        display.set(4, testDisplay, testWindow);
        
        CPPUNIT_ASSERT(display.getVersion() == 4);
        CPPUNIT_ASSERT(display.getDisplayBounds() == testDisplay);
        CPPUNIT_ASSERT(display.getWindowBounds() == testWindow);
        
        // Test consistency after reset
        display.reset();
        Rectangle defaultRect = { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };
        
        CPPUNIT_ASSERT(display.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(display.getDisplayBounds() == defaultRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == defaultRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == display.getWindowBounds());
    }

    void testVersionPersistence()
    {
        Display display;
        Rectangle testRect = { 1, 1, 10, 10 };
        
        // Test version persistence through multiple getter calls
        display.set(6, testRect, testRect);
        std::uint8_t version1 = display.getVersion();
        std::uint8_t version2 = display.getVersion();
        std::uint8_t version3 = display.getVersion();
        
        CPPUNIT_ASSERT(version1 == 6);
        CPPUNIT_ASSERT(version2 == 6);
        CPPUNIT_ASSERT(version3 == 6);
        CPPUNIT_ASSERT(version1 == version2);
        CPPUNIT_ASSERT(version2 == version3);
        
        // Test version persistence through rectangle getter calls
        Rectangle bounds1 = display.getDisplayBounds();
        Rectangle bounds2 = display.getWindowBounds();
        std::uint8_t versionAfterBounds = display.getVersion();
        
        CPPUNIT_ASSERT(versionAfterBounds == 6);
        CPPUNIT_ASSERT(bounds1 == testRect);
        CPPUNIT_ASSERT(bounds2 == testRect);
    }

    void testDisplayConstants()
    {
        // Test that constants are accessible and have expected values
        CPPUNIT_ASSERT(Display::DEFAULT_SD_WIDTH == 720);
        CPPUNIT_ASSERT(Display::DEFAULT_SD_HEIGHT == 576);
        
        // Test constants in default rectangle creation
        Rectangle defaultRect = { 0, 0, Display::DEFAULT_SD_WIDTH, Display::DEFAULT_SD_HEIGHT };
        CPPUNIT_ASSERT(defaultRect.m_x1 == 0);
        CPPUNIT_ASSERT(defaultRect.m_y1 == 0);
        CPPUNIT_ASSERT(defaultRect.m_x2 == 720);
        CPPUNIT_ASSERT(defaultRect.m_y2 == 576);
        
        // Test constants in Display object
        Display display;
        Rectangle displayBounds = display.getDisplayBounds();
        CPPUNIT_ASSERT(displayBounds.m_x2 == Display::DEFAULT_SD_WIDTH);
        CPPUNIT_ASSERT(displayBounds.m_y2 == Display::DEFAULT_SD_HEIGHT);
    }

    void testConstReferenceStability()
    {
        Display display;
        Rectangle testDisplay = { 10, 20, 30, 40 };
        Rectangle testWindow = { 15, 25, 25, 35 };
        
        display.set(9, testDisplay, testWindow);
        
        // Get const references
        const Rectangle& displayRef1 = display.getDisplayBounds();
        const Rectangle& windowRef1 = display.getWindowBounds();
        
        // Verify references point to correct data
        CPPUNIT_ASSERT(displayRef1 == testDisplay);
        CPPUNIT_ASSERT(windowRef1 == testWindow);
        
        // Get references again and verify they're consistent
        const Rectangle& displayRef2 = display.getDisplayBounds();
        const Rectangle& windowRef2 = display.getWindowBounds();
        
        CPPUNIT_ASSERT(displayRef2 == testDisplay);
        CPPUNIT_ASSERT(windowRef2 == testWindow);
        CPPUNIT_ASSERT(displayRef1 == displayRef2);
        CPPUNIT_ASSERT(windowRef1 == windowRef2);
        
        // Modify display and verify old references reflect new state
        Rectangle newDisplay = { 100, 200, 300, 400 };
        Rectangle newWindow = { 150, 250, 250, 350 };
        display.set(12, newDisplay, newWindow);
        
        CPPUNIT_ASSERT(displayRef1 == newDisplay);
        CPPUNIT_ASSERT(windowRef1 == newWindow);
    }

    void testIdempotency()
    {
        Display display;
        Rectangle testDisplay = { 5, 10, 15, 20 };
        Rectangle testWindow = { 7, 12, 13, 18 };
        std::uint8_t testVersion = 11;
        
        // Set values
        display.set(testVersion, testDisplay, testWindow);
        
        // Capture initial state
        std::uint8_t version1 = display.getVersion();
        Rectangle display1 = display.getDisplayBounds();
        Rectangle window1 = display.getWindowBounds();
        
        // Set identical values again
        display.set(testVersion, testDisplay, testWindow);
        
        // Verify state is identical
        std::uint8_t version2 = display.getVersion();
        Rectangle display2 = display.getDisplayBounds();
        Rectangle window2 = display.getWindowBounds();
        
        CPPUNIT_ASSERT(version1 == version2);
        CPPUNIT_ASSERT(display1 == display2);
        CPPUNIT_ASSERT(window1 == window2);
        
        // Set identical values a third time
        display.set(testVersion, testDisplay, testWindow);
        
        std::uint8_t version3 = display.getVersion();
        Rectangle display3 = display.getDisplayBounds();
        Rectangle window3 = display.getWindowBounds();
        
        CPPUNIT_ASSERT(version2 == version3);
        CPPUNIT_ASSERT(display2 == display3);
        CPPUNIT_ASSERT(window2 == window3);
    }

    void testExtremeValues()
    {
        Display display;
        
        // Test with INT32_MAX coordinates (boundary of int32_t)
        Rectangle maxRect = { INT32_MAX-1, INT32_MAX-1, INT32_MAX, INT32_MAX };
        display.set(255, maxRect, maxRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == maxRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == maxRect);
        CPPUNIT_ASSERT(display.getVersion() == 255);
        
        // Test with INT32_MIN coordinates
        Rectangle minRect = { INT32_MIN, INT32_MIN, INT32_MIN+1, INT32_MIN+1 };
        display.set(0, minRect, minRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == minRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == minRect);
        CPPUNIT_ASSERT(display.getVersion() == 0);
        
        // Test mixed extreme values
        Rectangle mixedExtremeRect = { INT32_MIN, 0, 0, INT32_MAX };
        display.set(127, mixedExtremeRect, mixedExtremeRect);
        CPPUNIT_ASSERT(display.getDisplayBounds() == mixedExtremeRect);
        CPPUNIT_ASSERT(display.getWindowBounds() == mixedExtremeRect);
        CPPUNIT_ASSERT(display.getVersion() == 127);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DisplayTest);
