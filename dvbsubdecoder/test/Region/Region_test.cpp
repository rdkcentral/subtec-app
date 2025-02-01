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

#include "Region.hpp"
#include "Consts.hpp"
#include "Clut.hpp"

using dvbsubdecoder::Region;
using dvbsubdecoder::Clut;
using dvbsubdecoder::ObjectInstance;

class RegionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( RegionTest );
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testClutArray);
    CPPUNIT_TEST(testObjects);CPPUNIT_TEST_SUITE_END()
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

    void testBasic()
    {
        const std::uint8_t ID = 14;
        const std::uint8_t VERSION = 3;
        const std::int32_t WIDTH = 20;
        const std::int32_t HEIGHT = 10;
        const std::uint8_t DEPTH = dvbsubdecoder::RegionDepthBits::DEPTH_4BIT;
        const std::uint8_t COMPATIBILITY_LEVEL =
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT;
        const std::uint8_t CLUT_ID = 6;
        const std::uint8_t CLUT_VERSION = 9;
        std::array<std::uint8_t, WIDTH * HEIGHT> pixmapMemory;
        Clut clut;
        const std::uint8_t BACKGROUND_INDEX = 2;

        clut.setId(CLUT_ID);
        clut.setVersion(CLUT_VERSION);

        Region region;

        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT_THROW(region.getClutId(), std::logic_error);
        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);
        CPPUNIT_ASSERT(region.getPixmap().getBuffer() == nullptr);

        region.setId(ID);
        region.setVersion(VERSION);
        region.init(WIDTH, HEIGHT, pixmapMemory.data(), COMPATIBILITY_LEVEL,
                DEPTH, &clut);
        region.setBackgroundIndex(BACKGROUND_INDEX);

        CPPUNIT_ASSERT(region.getId() == ID);
        CPPUNIT_ASSERT(region.getVersion() == VERSION);
        CPPUNIT_ASSERT(region.getClut() == &clut);
        CPPUNIT_ASSERT(region.getClutId() == CLUT_ID);
        CPPUNIT_ASSERT(region.getClutArray() != nullptr);
        CPPUNIT_ASSERT(region.getWidth() == WIDTH);
        CPPUNIT_ASSERT(region.getHeight() == HEIGHT);
        CPPUNIT_ASSERT(region.getDepth() == DEPTH);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == BACKGROUND_INDEX);
        CPPUNIT_ASSERT(region.getCompatibilityLevel() == COMPATIBILITY_LEVEL);
        CPPUNIT_ASSERT(region.getPixmap().getWidth() == WIDTH);
        CPPUNIT_ASSERT(region.getPixmap().getHeight() == HEIGHT);
        CPPUNIT_ASSERT(region.getPixmap().getBuffer() == pixmapMemory.data());

        region.reset();

        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT_THROW(region.getClutId(), std::logic_error);
        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);
        CPPUNIT_ASSERT(region.getPixmap().getBuffer() == nullptr);
    }

    void testClutArray()
    {
        const std::uint8_t VERSION = 3;
        const std::int32_t WIDTH = 20;
        const std::int32_t HEIGHT = 10;
        const std::uint8_t COMPATIBILITY =
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT;
        const std::uint8_t CLUT_ID = 6;
        const std::uint8_t CLUT_VERSION = 9;
        std::array<std::uint8_t, WIDTH * HEIGHT> pixmapMemory;
        Clut clut;

        clut.setId(CLUT_ID);
        clut.setVersion(CLUT_VERSION);

        Region region;

        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);

        region.reset();
        auto DEPTH2 = dvbsubdecoder::RegionDepthBits::DEPTH_2BIT;
        region.init(WIDTH, HEIGHT, pixmapMemory.data(), COMPATIBILITY, DEPTH2,
                &clut);

        CPPUNIT_ASSERT(region.getClutArray() == clut.getArray2bit());

        region.reset();
        auto DEPTH4 = dvbsubdecoder::RegionDepthBits::DEPTH_4BIT;
        region.init(WIDTH, HEIGHT, pixmapMemory.data(), COMPATIBILITY, DEPTH4,
                &clut);

        CPPUNIT_ASSERT(region.getClutArray() == clut.getArray4bit());

        region.reset();
        auto DEPTH8 = dvbsubdecoder::RegionDepthBits::DEPTH_8BIT;
        region.init(WIDTH, HEIGHT, pixmapMemory.data(), COMPATIBILITY, DEPTH8,
                &clut);

        CPPUNIT_ASSERT(region.getClutArray() == clut.getArray8bit());

        region.reset();
        auto DEPTH_BAD = 0xF1;
        region.init(WIDTH, HEIGHT, pixmapMemory.data(), COMPATIBILITY,
                DEPTH_BAD, &clut);

        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);

        region.reset();

        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);
    }

    void testObjects()
    {
        const int OBJECT_COUNT = 16;

        Region region;
        std::array<ObjectInstance, OBJECT_COUNT> objectPool;

        for (int i = 0; i < OBJECT_COUNT; ++i)
        {
            region.addObject(&objectPool[i]);
        }

        CPPUNIT_ASSERT_THROW(region.reset(), std::logic_error);

        int count = 0;
        for (auto item = region.getFirstObject(); item;
                item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == OBJECT_COUNT);

        while (region.removeFirstObject())
        {
            // noop
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(RegionTest);
