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
using dvbsubdecoder::Pixmap;

class RegionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( RegionTest );
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testClutArray);
    CPPUNIT_TEST(testObjects);
    CPPUNIT_TEST(testConstructorDefaults);
    CPPUNIT_TEST(testInitBoundaryDimensions);
    CPPUNIT_TEST(testInitNullPointers);
    CPPUNIT_TEST(testMultipleInitCalls);
    CPPUNIT_TEST(testSetIdBoundaryValues);
    CPPUNIT_TEST(testSetVersionBoundaryValues);
    CPPUNIT_TEST(testSetBackgroundIndexBoundaryValues);
    CPPUNIT_TEST(testErrorHandlingBeforeInit);
    CPPUNIT_TEST(testInvalidDepthHandling);
    CPPUNIT_TEST(testNullClutScenarios);
    CPPUNIT_TEST(testObjectManagementEmpty);
    CPPUNIT_TEST(testObjectManagementNullObjects);
    CPPUNIT_TEST(testObjectManagementMixedOperations);
    CPPUNIT_TEST(testObjectManagementLargeCount);
    CPPUNIT_TEST(testResetCompleteValidation);
    CPPUNIT_TEST(testMultipleResetCalls);
    CPPUNIT_TEST(testStateConsistencyAfterFailures);
    CPPUNIT_TEST(testComplexLifecycle);
    CPPUNIT_TEST(testPixmapIntegration);
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

    // Constructor and initialization tests
    void testConstructorDefaults()
    {
        Region region;

        // Verify constructor initializes to safe defaults
        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);
        CPPUNIT_ASSERT(region.getPixmap().getBuffer() == nullptr);
        CPPUNIT_ASSERT(region.getId() == 0);  // Default ID should be 0
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 0);  // Default background index
        CPPUNIT_ASSERT(region.getCompatibilityLevel() == 0xFF);  // Default compatibility level
        CPPUNIT_ASSERT(region.getDepth() == 0xFF);  // Default depth
        CPPUNIT_ASSERT(region.getFirstObject() == nullptr);  // No objects initially
    }

    void testInitBoundaryDimensions()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Test zero width - should work as pixmap handles this
        CPPUNIT_ASSERT_NO_THROW(region.init(0, 10, pixmapMemory.data(), 
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut));
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 10);

        region.reset();

        // Test zero height - should work as pixmap handles this
        CPPUNIT_ASSERT_NO_THROW(region.init(10, 0, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut));
        CPPUNIT_ASSERT(region.getWidth() == 10);
        CPPUNIT_ASSERT(region.getHeight() == 0);

        region.reset();

        // Test both zero
        CPPUNIT_ASSERT_NO_THROW(region.init(0, 0, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut));
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);

        region.reset();

        // Test maximum reasonable dimensions
        const std::int32_t maxDim = 1000;
        CPPUNIT_ASSERT_NO_THROW(region.init(maxDim, maxDim, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut));
        CPPUNIT_ASSERT(region.getWidth() == maxDim);
        CPPUNIT_ASSERT(region.getHeight() == maxDim);
    }

    void testInitNullPointers()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Test null pixmap memory - Pixmap::init should throw invalid_argument("buffer")
        CPPUNIT_ASSERT_THROW(region.init(10, 10, nullptr,
            dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
            dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut), std::invalid_argument);

        region.reset();

        // Test null CLUT - should be allowed
        CPPUNIT_ASSERT_NO_THROW(region.init(10, 10, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, nullptr));
        CPPUNIT_ASSERT(region.getClut() == nullptr);
    }

    void testMultipleInitCalls()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory1;
        std::array<std::uint8_t, 200> pixmapMemory2;
        Clut clut1, clut2;
        
        clut1.setId(1);
        clut1.setVersion(1);
        clut2.setId(2);
        clut2.setVersion(2);

        // First init
        region.init(10, 10, pixmapMemory1.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut1);
        
        CPPUNIT_ASSERT(region.getWidth() == 10);
        CPPUNIT_ASSERT(region.getHeight() == 10);
        CPPUNIT_ASSERT(region.getClut() == &clut1);
        CPPUNIT_ASSERT(region.getDepth() == dvbsubdecoder::RegionDepthBits::DEPTH_2BIT);

        // Second init - should reinitialize properly
        region.init(20, 15, pixmapMemory2.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, &clut2);

        CPPUNIT_ASSERT(region.getWidth() == 20);
        CPPUNIT_ASSERT(region.getHeight() == 15);
        CPPUNIT_ASSERT(region.getClut() == &clut2);
        CPPUNIT_ASSERT(region.getDepth() == dvbsubdecoder::RegionDepthBits::DEPTH_4BIT);
        CPPUNIT_ASSERT(region.getCompatibilityLevel() == dvbsubdecoder::RegionDepthBits::DEPTH_4BIT);
    }

    // Boundary value tests for setters
    void testSetIdBoundaryValues()
    {
        Region region;

        // Test minimum value
        region.setId(0);
        CPPUNIT_ASSERT(region.getId() == 0);

        // Test maximum value
        region.setId(255);
        CPPUNIT_ASSERT(region.getId() == 255);

        // Test some intermediate values
        region.setId(128);
        CPPUNIT_ASSERT(region.getId() == 128);

        region.setId(1);
        CPPUNIT_ASSERT(region.getId() == 1);

        region.setId(254);
        CPPUNIT_ASSERT(region.getId() == 254);
    }

    void testSetVersionBoundaryValues()
    {
        Region region;

        // Test minimum value
        region.setVersion(0);
        CPPUNIT_ASSERT(region.getVersion() == 0);

        // Test maximum value
        region.setVersion(255);
        CPPUNIT_ASSERT(region.getVersion() == 255);

        // Test INVALID_VERSION specifically
        region.setVersion(dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);

        // Test some intermediate values
        region.setVersion(15);  // Maximum valid version (4-bit field)
        CPPUNIT_ASSERT(region.getVersion() == 15);

        region.setVersion(128);
        CPPUNIT_ASSERT(region.getVersion() == 128);
    }

    void testSetBackgroundIndexBoundaryValues()
    {
        Region region;

        // Test minimum value
        region.setBackgroundIndex(0);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 0);

        // Test maximum value
        region.setBackgroundIndex(255);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 255);

        // Test some intermediate values
        region.setBackgroundIndex(128);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 128);

        region.setBackgroundIndex(1);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 1);

        region.setBackgroundIndex(254);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 254);
    }

    // Error handling and exception tests
    void testErrorHandlingBeforeInit()
    {
        Region region;

        // All these should throw logic_error when region is not initialized
        CPPUNIT_ASSERT_THROW(region.getClutId(), std::logic_error);
        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);

        // These should work even without initialization
        CPPUNIT_ASSERT_NO_THROW(region.getId());
        CPPUNIT_ASSERT_NO_THROW(region.getVersion());
        CPPUNIT_ASSERT_NO_THROW(region.getBackgroundIndex());
        CPPUNIT_ASSERT_NO_THROW(region.getWidth());
        CPPUNIT_ASSERT_NO_THROW(region.getHeight());
        CPPUNIT_ASSERT_NO_THROW(region.getCompatibilityLevel());
        CPPUNIT_ASSERT_NO_THROW(region.getDepth());
        CPPUNIT_ASSERT_NO_THROW(region.getClut());
        CPPUNIT_ASSERT_NO_THROW(region.getPixmap());
    }

    void testInvalidDepthHandling()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Test various invalid depth values
        std::vector<std::uint8_t> invalidDepths = {0x00, 0x04, 0x05, 0xF0, 0xF1, 0xFF};

        for (auto invalidDepth : invalidDepths)
        {
            region.init(10, 10, pixmapMemory.data(),
                    dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                    invalidDepth, &clut);

            // getClutArray should throw for invalid depth
            CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);

            region.reset();
        }
    }

    void testNullClutScenarios()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;

        // Initialize with null CLUT
        region.init(10, 10, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, nullptr);

        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT_THROW(region.getClutId(), std::logic_error);
        CPPUNIT_ASSERT_THROW(region.getClutArray(), std::logic_error);

        // Verify other operations still work
        CPPUNIT_ASSERT_NO_THROW(region.getWidth());
        CPPUNIT_ASSERT_NO_THROW(region.getHeight());
        CPPUNIT_ASSERT_NO_THROW(region.getDepth());
        CPPUNIT_ASSERT_NO_THROW(region.getCompatibilityLevel());
    }

    // Object management tests
    void testObjectManagementEmpty()
    {
        Region region;

        // Test operations on empty object list
        CPPUNIT_ASSERT(region.getFirstObject() == nullptr);
        CPPUNIT_ASSERT(region.removeFirstObject() == nullptr);
        // getNextObject(nullptr) should throw invalid_argument("prev")
        CPPUNIT_ASSERT_THROW(region.getNextObject(nullptr), std::invalid_argument);

        // Reset should work with empty list
        CPPUNIT_ASSERT_NO_THROW(region.reset());
    }

    void testObjectManagementNullObjects()
    {
        Region region;

        // Adding null object should throw invalid_argument("reference")
        CPPUNIT_ASSERT_THROW(region.addObject(nullptr), std::invalid_argument);

        // getNextObject with null parameter should throw invalid_argument
        CPPUNIT_ASSERT_THROW(region.getNextObject(nullptr), std::invalid_argument);
    }

    void testObjectManagementMixedOperations()
    {
        Region region;
        std::array<ObjectInstance, 5> objectPool;

        // Add objects
        for (int i = 0; i < 3; ++i)
        {
            region.addObject(&objectPool[i]);
        }

        // Verify count
        int count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == 3);

        // Remove one
        ObjectInstance* removed = region.removeFirstObject();
        CPPUNIT_ASSERT(removed != nullptr);

        // Verify count decreased
        count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == 2);

        // Add more
        region.addObject(&objectPool[3]);
        region.addObject(&objectPool[4]);

        // Verify final count
        count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == 4);

        // Clean up
        while (region.removeFirstObject())
        {
            // Remove all objects
        }

        CPPUNIT_ASSERT(region.getFirstObject() == nullptr);
    }

    void testObjectManagementLargeCount()
    {
        Region region;
        const int LARGE_COUNT = 100;
        std::array<ObjectInstance, LARGE_COUNT> objectPool;

        // Add large number of objects
        for (int i = 0; i < LARGE_COUNT; ++i)
        {
            region.addObject(&objectPool[i]);
        }

        // Verify count
        int count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == LARGE_COUNT);

        // Test that we can't reset with objects present
        CPPUNIT_ASSERT_THROW(region.reset(), std::logic_error);

        // Remove all objects
        while (region.removeFirstObject())
        {
            // Remove all
        }

        // Now reset should work
        CPPUNIT_ASSERT_NO_THROW(region.reset());
    }

    // State validation and reset tests
    void testResetCompleteValidation()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        Clut clut;
        clut.setId(5);
        clut.setVersion(3);

        // Set up region with all values
        region.setId(10);
        region.setVersion(15);
        region.init(20, 30, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, &clut);
        region.setBackgroundIndex(7);

        // Verify all values are set
        CPPUNIT_ASSERT(region.getId() == 10);
        CPPUNIT_ASSERT(region.getVersion() == 15);
        CPPUNIT_ASSERT(region.getWidth() == 20);
        CPPUNIT_ASSERT(region.getHeight() == 30);
        CPPUNIT_ASSERT(region.getCompatibilityLevel() == dvbsubdecoder::RegionDepthBits::DEPTH_4BIT);
        CPPUNIT_ASSERT(region.getDepth() == dvbsubdecoder::RegionDepthBits::DEPTH_8BIT);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 7);
        CPPUNIT_ASSERT(region.getClut() == &clut);

        // Reset
        region.reset();

        // Verify ALL fields are reset to defaults
        CPPUNIT_ASSERT(region.getId() == 0);
        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);
        CPPUNIT_ASSERT(region.getCompatibilityLevel() == 0xFF);
        CPPUNIT_ASSERT(region.getDepth() == 0xFF);
        CPPUNIT_ASSERT(region.getBackgroundIndex() == 0);
        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT(region.getPixmap().getBuffer() == nullptr);
        CPPUNIT_ASSERT(region.getFirstObject() == nullptr);
    }

    void testMultipleResetCalls()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Initialize region
        region.init(10, 10, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut);

        // Multiple reset calls should be safe
        CPPUNIT_ASSERT_NO_THROW(region.reset());
        CPPUNIT_ASSERT_NO_THROW(region.reset());
        CPPUNIT_ASSERT_NO_THROW(region.reset());

        // State should remain consistent
        CPPUNIT_ASSERT(region.getVersion() == dvbsubdecoder::INVALID_VERSION);
        CPPUNIT_ASSERT(region.getClut() == nullptr);
        CPPUNIT_ASSERT(region.getWidth() == 0);
        CPPUNIT_ASSERT(region.getHeight() == 0);
    }

    void testStateConsistencyAfterFailures()
    {
        Region region;
        std::array<std::uint8_t, 100> pixmapMemory;
        std::array<ObjectInstance, 1> objectPool;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Initialize region
        region.init(10, 10, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut);

        // Add object to prevent reset
        region.addObject(&objectPool[0]);

        // Attempt reset - should fail
        CPPUNIT_ASSERT_THROW(region.reset(), std::logic_error);

        // Verify region state is still consistent
        CPPUNIT_ASSERT(region.getWidth() == 10);
        CPPUNIT_ASSERT(region.getHeight() == 10);
        CPPUNIT_ASSERT(region.getClut() == &clut);
        CPPUNIT_ASSERT(region.getFirstObject() != nullptr);

        // Remove object and then reset should work
        region.removeFirstObject();
        CPPUNIT_ASSERT_NO_THROW(region.reset());
    }

    // Integration tests
    void testComplexLifecycle()
    {
        Region region;
        std::array<std::uint8_t, 1000> pixmapMemory;
        std::array<ObjectInstance, 10> objectPool;
        Clut clut1, clut2;
        
        clut1.setId(1);
        clut1.setVersion(1);
        clut2.setId(2);
        clut2.setVersion(2);

        // Phase 1: Initial setup
        region.setId(100);
        region.setVersion(5);
        region.init(50, 20, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, &clut1);
        region.setBackgroundIndex(3);

        // Add some objects
        for (int i = 0; i < 5; ++i)
        {
            region.addObject(&objectPool[i]);
        }

        // Verify state
        CPPUNIT_ASSERT(region.getId() == 100);
        CPPUNIT_ASSERT(region.getVersion() == 5);
        CPPUNIT_ASSERT(region.getClutArray() == clut1.getArray4bit());

        // Phase 2: Modify objects
        int count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == 5);

        // Remove some objects
        region.removeFirstObject();
        region.removeFirstObject();

        count = 0;
        for (auto item = region.getFirstObject(); item; item = region.getNextObject(item))
        {
            ++count;
        }
        CPPUNIT_ASSERT(count == 3);

        // Phase 3: Reinitialize
        while (region.removeFirstObject())
        {
            // Remove all objects
        }

        region.init(30, 40, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, &clut2);

        CPPUNIT_ASSERT(region.getWidth() == 30);
        CPPUNIT_ASSERT(region.getHeight() == 40);
        CPPUNIT_ASSERT(region.getClutArray() == clut2.getArray8bit());

        // Phase 4: Final cleanup
        CPPUNIT_ASSERT_NO_THROW(region.reset());
    }

    void testPixmapIntegration()
    {
        Region region;
        std::array<std::uint8_t, 200> pixmapMemory;
        Clut clut;
        clut.setId(1);
        clut.setVersion(1);

        // Initialize with specific memory buffer
        region.init(10, 20, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, &clut);

        // Verify pixmap integration
        Pixmap& pixmap = region.getPixmap();
        CPPUNIT_ASSERT(pixmap.getWidth() == 10);
        CPPUNIT_ASSERT(pixmap.getHeight() == 20);
        CPPUNIT_ASSERT(pixmap.getBuffer() == pixmapMemory.data());

        // Verify pixmap is accessible through region
        CPPUNIT_ASSERT(region.getWidth() == pixmap.getWidth());
        CPPUNIT_ASSERT(region.getHeight() == pixmap.getHeight());

        // Test with different dimensions
        region.reset();
        region.init(5, 8, pixmapMemory.data(),
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, &clut);

        CPPUNIT_ASSERT(region.getPixmap().getWidth() == 5);
        CPPUNIT_ASSERT(region.getPixmap().getHeight() == 8);
        CPPUNIT_ASSERT(region.getWidth() == 5);
        CPPUNIT_ASSERT(region.getHeight() == 8);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(RegionTest);
