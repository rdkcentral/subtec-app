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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixmapTest);
