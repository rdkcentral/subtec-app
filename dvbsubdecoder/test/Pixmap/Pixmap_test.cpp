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

#include "Pixmap.hpp"

#include <algorithm>

using dvbsubdecoder::Pixmap;

class PixmapTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PixmapTest );
    CPPUNIT_TEST(testInitReset);
    CPPUNIT_TEST(testBadInit);
    CPPUNIT_TEST(testGetLine);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST(testZeroDimensions);
    CPPUNIT_TEST(testBoundaryDimensions);
    CPPUNIT_TEST(testGetLineBoundaryValues);
    CPPUNIT_TEST(testClearBoundaryColors);
    CPPUNIT_TEST(testMultipleInitCalls);
    CPPUNIT_TEST(testInitResetCycle);
    CPPUNIT_TEST(testGetLinePointerArithmetic);
    CPPUNIT_TEST(testClearStatePreservation);
    CPPUNIT_TEST(testLargeBufferClear);
    CPPUNIT_TEST(testConstCorrectness);
    CPPUNIT_TEST(testMinimalDimensions);
    CPPUNIT_TEST(testClearAfterReset);
    CPPUNIT_TEST(testGetLineZeroHeight);
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

    void testInitReset()
    {
        Pixmap testPixmap;

        // test initial state
        CPPUNIT_ASSERT(testPixmap.getBuffer() == nullptr);
        CPPUNIT_ASSERT(testPixmap.getWidth() == 0);
        CPPUNIT_ASSERT(testPixmap.getHeight() == 0);

        // test state after init
        const std::int32_t width = 100;
        const std::int32_t height = 50;
        std::uint8_t buffer[width * height] =
        { 0 };

        testPixmap.init(width, height, buffer);

        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);
        CPPUNIT_ASSERT(testPixmap.getWidth() == width);
        CPPUNIT_ASSERT(testPixmap.getHeight() == height);

        // test state after reset
        testPixmap.reset();

        CPPUNIT_ASSERT(testPixmap.getBuffer() == nullptr);
        CPPUNIT_ASSERT(testPixmap.getWidth() == 0);
        CPPUNIT_ASSERT(testPixmap.getHeight() == 0);
    }

    void testBadInit()
    {
        Pixmap testPixmap;

        const std::int32_t width = 100;
        const std::int32_t height = 50;
        std::uint8_t buffer[width * height] =
        { 0 };

        CPPUNIT_ASSERT_THROW(testPixmap.init(-1, -1, nullptr), std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(width, -1, nullptr),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(-1, height, nullptr),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(width, height, nullptr),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(-1, -1, buffer), std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(width, -1, buffer),
                std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.init(-1, height, buffer),
                std::logic_error);
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(width, height, buffer));
    }

    void testGetLine()
    {
        Pixmap testPixmap;

        // test throw after constructed
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(0), std::logic_error);

        // test state after init
        const std::int32_t width = 100;
        const std::int32_t height = 50;
        std::uint8_t buffer[width * height] =
        { 0 };
        testPixmap.init(width, height, buffer);

        // test not throw after init on valid line
        CPPUNIT_ASSERT_NO_THROW(testPixmap.getLine(0));

        // test throw after init on invalid line
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(-1), std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(height), std::logic_error);

        // test works
        CPPUNIT_ASSERT(testPixmap.getLine(0) == buffer);
        CPPUNIT_ASSERT(testPixmap.getLine(1) - testPixmap.getLine(0) == width);
        CPPUNIT_ASSERT(
                testPixmap.getLine(10) - testPixmap.getLine(0) == (width * 10));

        // test throw after rest
        testPixmap.reset();
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(0), std::logic_error);
    }

    void testClear()
    {
        Pixmap testPixmap;

        CPPUNIT_ASSERT_THROW(testPixmap.clear(0), std::logic_error);

        const std::int32_t width = 100;
        const std::int32_t height = 50;
        std::uint8_t buffer[width * height] =
        { 0 };

        testPixmap.init(width, height, buffer);

        std::fill(std::begin(buffer), std::end(buffer), 0xFF);

        std::uint8_t colorIndex = 1;
        testPixmap.clear(colorIndex);

        CPPUNIT_ASSERT(
                std::end(buffer)
                        == std::find_if(std::begin(buffer), std::end(buffer),
                                [colorIndex](std::uint8_t value) -> bool
                                {   return value != colorIndex;}));

        colorIndex = 5;
        testPixmap.clear(colorIndex);

        CPPUNIT_ASSERT(
                std::end(buffer)
                        == std::find_if(std::begin(buffer), std::end(buffer),
                                [colorIndex](std::uint8_t value) -> bool
                                {   return value != colorIndex;}));
    }

    void testZeroDimensions()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[1] = { 0 }; // Minimal buffer for zero dimension cases

        // Test zero width with valid height and buffer
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(0, 10, buffer));
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);

        testPixmap.reset();

        // Test zero height with valid width and buffer
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(10, 0, buffer));
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);

        testPixmap.reset();

        // Test both dimensions zero
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(0, 0, buffer));
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);
    }

    void testBoundaryDimensions()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[4] = { 0 }; // Small buffer for testing

        // Test dimension value 1 (minimum positive)
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(1, 1, buffer));
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getHeight());

        testPixmap.reset();

        // Test asymmetric small dimensions
        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(1, 4, buffer));
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(4, testPixmap.getHeight());

        testPixmap.reset();

        CPPUNIT_ASSERT_NO_THROW(testPixmap.init(4, 1, buffer));
        CPPUNIT_ASSERT_EQUAL(4, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getHeight());
    }

    void testGetLineBoundaryValues()
    {
        Pixmap testPixmap;
        const std::int32_t width = 10;
        const std::int32_t height = 5;
        std::uint8_t buffer[width * height] = { 0 };

        testPixmap.init(width, height, buffer);

        // Test boundary line access
        CPPUNIT_ASSERT_NO_THROW(testPixmap.getLine(0)); // First line
        CPPUNIT_ASSERT_NO_THROW(testPixmap.getLine(height - 1)); // Last line
        CPPUNIT_ASSERT(testPixmap.getLine(0) == buffer);
        CPPUNIT_ASSERT(testPixmap.getLine(height - 1) == buffer + (height - 1) * width);

        // Test out of bounds
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(-1), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(height), std::invalid_argument);
    }

    void testClearBoundaryColors()
    {
        Pixmap testPixmap;
        const std::int32_t width = 4;
        const std::int32_t height = 4;
        std::uint8_t buffer[width * height] = { 0 };

        testPixmap.init(width, height, buffer);

        // Test boundary color values
        CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(0)); // Minimum uint8_t value
        for (int i = 0; i < width * height; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), buffer[i]);
        }

        CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(255)); // Maximum uint8_t value
        for (int i = 0; i < width * height; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), buffer[i]);
        }

        // Test mid-range values
        CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(127));
        for (int i = 0; i < width * height; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(127), buffer[i]);
        }
    }

    void testMultipleInitCalls()
    {
        Pixmap testPixmap;
        std::uint8_t buffer1[100] = { 0 };
        std::uint8_t buffer2[200] = { 0 };

        // First init
        testPixmap.init(10, 10, buffer1);
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer1);

        // Second init should overwrite previous state
        testPixmap.init(20, 10, buffer2);
        CPPUNIT_ASSERT_EQUAL(20, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer2);

        // Third init with different dimensions
        testPixmap.init(5, 40, buffer1);
        CPPUNIT_ASSERT_EQUAL(5, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(40, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer1);
    }

    void testInitResetCycle()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[50] = { 0 };

        // Init -> Reset -> Init cycle
        testPixmap.init(5, 10, buffer);
        CPPUNIT_ASSERT_EQUAL(5, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);

        testPixmap.reset();
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == nullptr);

        // Should be able to init again after reset
        testPixmap.init(10, 5, buffer);
        CPPUNIT_ASSERT_EQUAL(10, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(5, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == buffer);

        // Multiple reset cycles
        testPixmap.reset();
        testPixmap.reset(); // Should be safe to call multiple times
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(0, testPixmap.getHeight());
        CPPUNIT_ASSERT(testPixmap.getBuffer() == nullptr);
    }

    void testGetLinePointerArithmetic()
    {
        Pixmap testPixmap;
        const std::int32_t width = 7;
        const std::int32_t height = 6;
        std::uint8_t buffer[width * height] = { 0 };

        testPixmap.init(width, height, buffer);

        // Verify all lines return correctly spaced pointers
        for (int line = 0; line < height; ++line)
        {
            std::uint8_t* linePtr = testPixmap.getLine(line);
            std::uint8_t* expectedPtr = buffer + line * width;
            CPPUNIT_ASSERT_EQUAL(expectedPtr, linePtr);
        }

        // Verify spacing between consecutive lines
        for (int line = 0; line < height - 1; ++line)
        {
            std::uint8_t* currentLine = testPixmap.getLine(line);
            std::uint8_t* nextLine = testPixmap.getLine(line + 1);
            CPPUNIT_ASSERT_EQUAL(width, static_cast<int>(nextLine - currentLine));
        }
    }

    void testClearStatePreservation()
    {
        Pixmap testPixmap;
        const std::int32_t width = 8;
        const std::int32_t height = 4;
        std::uint8_t buffer[width * height] = { 0 };

        testPixmap.init(width, height, buffer);

        // Store original state
        std::int32_t originalWidth = testPixmap.getWidth();
        std::int32_t originalHeight = testPixmap.getHeight();
        std::uint8_t* originalBuffer = testPixmap.getBuffer();

        // Clear should not change pixmap metadata
        testPixmap.clear(42);
        CPPUNIT_ASSERT_EQUAL(originalWidth, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(originalHeight, testPixmap.getHeight());
        CPPUNIT_ASSERT_EQUAL(originalBuffer, testPixmap.getBuffer());

        // Multiple clears should preserve state
        testPixmap.clear(0);
        testPixmap.clear(255);
        testPixmap.clear(128);
        CPPUNIT_ASSERT_EQUAL(originalWidth, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(originalHeight, testPixmap.getHeight());
        CPPUNIT_ASSERT_EQUAL(originalBuffer, testPixmap.getBuffer());
    }

    void testLargeBufferClear()
    {
        Pixmap testPixmap;
        const std::int32_t width = 100;
        const std::int32_t height = 100;
        const std::int32_t size = width * height;
        std::uint8_t* buffer = new std::uint8_t[size];

        try
        {
            testPixmap.init(width, height, buffer);

            // Test large buffer clear operation
            CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(85));

            // Verify clear worked correctly
            for (int i = 0; i < size; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(85), buffer[i]);
            }

            // Test another clear
            CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(170));
            for (int i = 0; i < size; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(170), buffer[i]);
            }
        }
        catch (...)
        {
            delete[] buffer;
            throw;
        }

        delete[] buffer;
    }

    void testConstCorrectness()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[20] = { 0 };

        testPixmap.init(4, 5, buffer);

        // Test const methods on const reference
        const Pixmap& constPixmap = testPixmap;
        CPPUNIT_ASSERT_EQUAL(4, constPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(5, constPixmap.getHeight());
        CPPUNIT_ASSERT(constPixmap.getBuffer() == buffer);

        // Test that const methods don't modify state
        std::int32_t width1 = constPixmap.getWidth();
        std::int32_t width2 = constPixmap.getWidth();
        CPPUNIT_ASSERT_EQUAL(width1, width2);

        std::int32_t height1 = constPixmap.getHeight();
        std::int32_t height2 = constPixmap.getHeight();
        CPPUNIT_ASSERT_EQUAL(height1, height2);

        std::uint8_t* buffer1 = constPixmap.getBuffer();
        std::uint8_t* buffer2 = constPixmap.getBuffer();
        CPPUNIT_ASSERT_EQUAL(buffer1, buffer2);
    }

    void testMinimalDimensions()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[1] = { 0 };

        // Test single pixel pixmap (1x1)
        testPixmap.init(1, 1, buffer);
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getHeight());
        CPPUNIT_ASSERT_NO_THROW(testPixmap.getLine(0));
        CPPUNIT_ASSERT(testPixmap.getLine(0) == buffer);
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(1), std::invalid_argument);

        // Test clear on single pixel
        testPixmap.clear(123);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(123), buffer[0]);

        testPixmap.reset();

        // Test single column pixmap (1xN)
        std::uint8_t columnBuffer[5] = { 0 };
        testPixmap.init(1, 5, columnBuffer);
        CPPUNIT_ASSERT_EQUAL(1, testPixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(5, testPixmap.getHeight());

        for (int line = 0; line < 5; ++line)
        {
            CPPUNIT_ASSERT_NO_THROW(testPixmap.getLine(line));
            CPPUNIT_ASSERT(testPixmap.getLine(line) == columnBuffer + line);
        }
    }

    void testClearAfterReset()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[10] = { 0 };

        // Init, then reset
        testPixmap.init(2, 5, buffer);
        testPixmap.reset();

        // Clear after reset should throw
        CPPUNIT_ASSERT_THROW(testPixmap.clear(50), std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.clear(0), std::logic_error);
        CPPUNIT_ASSERT_THROW(testPixmap.clear(255), std::logic_error);
    }

    void testGetLineZeroHeight()
    {
        Pixmap testPixmap;
        std::uint8_t buffer[10] = { 0 };

        // Init with zero height
        testPixmap.init(10, 0, buffer);

        // Any line access should throw since height is 0
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(0), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(-1), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(testPixmap.getLine(1), std::invalid_argument);

        // Clear should still work (empty operation)
        CPPUNIT_ASSERT_NO_THROW(testPixmap.clear(100));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixmapTest);
