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

#include "Clut.hpp"
#include "Consts.hpp"

using dvbsubdecoder::Clut;
using dvbsubdecoder::INVALID_VERSION;

class ClutTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ClutTest );
    CPPUNIT_TEST(testIdVersion);
    CPPUNIT_TEST(testSetReset);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testIdBoundaryValues);
    CPPUNIT_TEST(testVersionBoundaryValues);
    CPPUNIT_TEST(testSet2bitBoundaryValues);
    CPPUNIT_TEST(testSet4bitBoundaryValues);
    CPPUNIT_TEST(testSet8bitBoundaryValues);
    CPPUNIT_TEST(testARGBValuePreservation);
    CPPUNIT_TEST(testCLUTArraySizes);
    CPPUNIT_TEST(testCLUTIndependence);
    CPPUNIT_TEST(testDefaultCLUTContent);
    CPPUNIT_TEST(testResetBehaviorMixedStates);
    CPPUNIT_TEST(testStatePersistence);
    CPPUNIT_TEST(testMultipleResetCalls);
    CPPUNIT_TEST(testMultipleInstanceIndependence);
    CPPUNIT_TEST(testArrayPointerConsistency);
    CPPUNIT_TEST(testVersionWrapAround);
    CPPUNIT_TEST(testSpecificColorValues);
    CPPUNIT_TEST(testBitPatternCorrectness);
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

    void testIdVersion()
    {
        Clut clut;

        CPPUNIT_ASSERT(clut.getVersion() == INVALID_VERSION);

        clut.setId(10);
        clut.setVersion(20);

        CPPUNIT_ASSERT(clut.getId() == 10);
        CPPUNIT_ASSERT(clut.getVersion() == 20);

        clut.reset();

        CPPUNIT_ASSERT(clut.getId() == 10);
        CPPUNIT_ASSERT(clut.getVersion() == INVALID_VERSION);

        clut.setId(40);
        clut.setVersion(50);

        CPPUNIT_ASSERT(clut.getId() == 40);
        CPPUNIT_ASSERT(clut.getVersion() == 50);
    }

    void testSetReset()
    {
        const Clut defaultClut;
        Clut testClut;

        // check initial state
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto defaultArray = defaultClut.getArray2bit();
            auto testArray = testClut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto defaultArray = defaultClut.getArray4bit();
            auto testArray = testClut.getArray4bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto defaultArray = defaultClut.getArray8bit();
            auto testArray = testClut.getArray8bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }

        // set
        for (int i = 0; i < (1 << 2); ++i)
        {
            std::uint32_t value = (i << 24) | (i << 16) | i;
            testClut.set2bit(i, value);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            std::uint32_t value = (i << 24) | (i << 16) | i;
            testClut.set4bit(i, value);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            std::uint32_t value = (i << 24) | (i << 16) | i;
            testClut.set8bit(i, value);
        }

        // test set
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto testArray = testClut.getArray2bit();
            std::uint32_t value = (i << 24) | (i << 16) | i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto testArray = testClut.getArray4bit();
            std::uint32_t value = (i << 24) | (i << 16) | i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto testArray = testClut.getArray8bit();
            std::uint32_t value = (i << 24) | (i << 16) | i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }

        testClut.reset();

        // check after reset state
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto defaultArray = defaultClut.getArray2bit();
            auto testArray = testClut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto defaultArray = defaultClut.getArray4bit();
            auto testArray = testClut.getArray4bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto defaultArray = defaultClut.getArray8bit();
            auto testArray = testClut.getArray8bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
    }

    // Constructor Testing
    void testConstructorInitialState()
    {
        Clut clut;
        
        // Test initial ID (should be 0 based on header default initialization)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), clut.getId());
        
        // Test initial version (should be INVALID_VERSION)
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion());
        
        // Test that arrays are properly initialized
        const std::uint32_t* array2bit = clut.getArray2bit();
        const std::uint32_t* array4bit = clut.getArray4bit();
        const std::uint32_t* array8bit = clut.getArray8bit();
        
        CPPUNIT_ASSERT(array2bit != nullptr);
        CPPUNIT_ASSERT(array4bit != nullptr);
        CPPUNIT_ASSERT(array8bit != nullptr);
    }

    // ID Boundary Values
    void testIdBoundaryValues()
    {
        Clut clut;
        
        // Test minimum value
        clut.setId(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), clut.getId());
        
        // Test maximum value
        clut.setId(255);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), clut.getId());
        
        // Test intermediate values
        clut.setId(127);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(127), clut.getId());
        
        clut.setId(128);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(128), clut.getId());
    }

    // Version Boundary Values
    void testVersionBoundaryValues()
    {
        Clut clut;
        
        // Test minimum value
        clut.setVersion(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), clut.getVersion());
        
        // Test maximum valid value (254, since 255 is INVALID_VERSION)
        clut.setVersion(254);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(254), clut.getVersion());
        
        // Test INVALID_VERSION value explicitly
        clut.setVersion(INVALID_VERSION);
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion());
        
        // Test intermediate values
        clut.setVersion(127);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(127), clut.getVersion());
    }

    // 2-bit CLUT Boundary Values
    void testSet2bitBoundaryValues()
    {
        Clut clut;
        
        // Test all valid indices (0-3 for 2-bit)
        for (std::uint8_t i = 0; i < 4; ++i)
        {
            // Test with minimum ARGB value
            clut.set2bit(i, 0x00000000);
            const std::uint32_t* array = clut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), array[i]);
            
            // Test with maximum ARGB value
            clut.set2bit(i, 0xFFFFFFFF);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), array[i]);
            
            // Test with pattern value
            std::uint32_t pattern = 0x12345678 + i;
            clut.set2bit(i, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, array[i]);
        }
    }

    // 4-bit CLUT Boundary Values
    void testSet4bitBoundaryValues()
    {
        Clut clut;
        
        // Test boundary indices
        std::uint8_t testIndices[] = {0, 1, 14, 15};
        for (std::uint8_t i : testIndices)
        {
            // Test with minimum ARGB value
            clut.set4bit(i, 0x00000000);
            const std::uint32_t* array = clut.getArray4bit();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), array[i]);
            
            // Test with maximum ARGB value
            clut.set4bit(i, 0xFFFFFFFF);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), array[i]);
            
            // Test with pattern value
            std::uint32_t pattern = 0xABCDEF00 + i;
            clut.set4bit(i, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, array[i]);
        }
    }

    // 8-bit CLUT Boundary Values
    void testSet8bitBoundaryValues()
    {
        Clut clut;
        
        // Test boundary indices
        std::uint8_t testIndices[] = {0, 1, 127, 128, 254, 255};
        for (std::uint8_t i : testIndices)
        {
            // Test with minimum ARGB value
            clut.set8bit(i, 0x00000000);
            const std::uint32_t* array = clut.getArray8bit();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), array[i]);
            
            // Test with maximum ARGB value
            clut.set8bit(i, 0xFFFFFFFF);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), array[i]);
            
            // Test with pattern value
            std::uint32_t pattern = 0x13579BDF + i;
            clut.set8bit(i, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, array[i]);
        }
    }

    // ARGB Value Preservation
    void testARGBValuePreservation()
    {
        Clut clut;
        
        // Test specific ARGB patterns to ensure bit-perfect storage
        std::uint32_t testValues[] = {
            0x00000000, // Fully transparent black
            0xFF000000, // Fully opaque black
            0xFFFFFFFF, // Fully opaque white
            0x80808080, // Semi-transparent gray
            0xFF123456, // Opaque arbitrary color
            0x7FABCDEF, // Semi-transparent arbitrary color
            0x00FFFFFF, // Fully transparent white
            0xFFFF0000, // Opaque red
            0xFF00FF00, // Opaque green
            0xFF0000FF  // Opaque blue
        };
        
        for (std::uint32_t value : testValues)
        {
            // Test 2-bit CLUT
            for (std::uint8_t i = 0; i < 4; ++i)
            {
                clut.set2bit(i, value);
                const std::uint32_t* array2bit = clut.getArray2bit();
                CPPUNIT_ASSERT_EQUAL(value, array2bit[i]);
            }
            
            // Test 4-bit CLUT (sample indices)
            std::uint8_t indices4[] = {0, 7, 15};
            for (std::uint8_t i : indices4)
            {
                clut.set4bit(i, value);
                const std::uint32_t* array4bit = clut.getArray4bit();
                CPPUNIT_ASSERT_EQUAL(value, array4bit[i]);
            }
            
            // Test 8-bit CLUT (sample indices)
            std::uint8_t indices8[] = {0, 127, 255};
            for (std::uint8_t i : indices8)
            {
                clut.set8bit(i, value);
                const std::uint32_t* array8bit = clut.getArray8bit();
                CPPUNIT_ASSERT_EQUAL(value, array8bit[i]);
            }
        }
    }

    // CLUT Array Size Validation
    void testCLUTArraySizes()
    {
        Clut clut;
        
        // We can't directly test array sizes, but we can test that all valid indices work
        // and verify the expected number of entries by setting and reading them
        
        // Test 2-bit CLUT has exactly 4 entries
        const std::uint32_t* array2bit = clut.getArray2bit();
        for (int i = 0; i < 4; ++i)
        {
            clut.set2bit(i, 0x12345600 + i);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345600 + i), array2bit[i]);
        }
        
        // Test 4-bit CLUT has exactly 16 entries
        const std::uint32_t* array4bit = clut.getArray4bit();
        for (int i = 0; i < 16; ++i)
        {
            clut.set4bit(i, 0x23456700 + i);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x23456700 + i), array4bit[i]);
        }
        
        // Test 8-bit CLUT has exactly 256 entries (sample test)
        const std::uint32_t* array8bit = clut.getArray8bit();
        for (int i = 0; i < 256; i += 17) // Test every 17th entry to cover range
        {
            clut.set8bit(i, 0x34567800 + i);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x34567800 + i), array8bit[i]);
        }
    }

    // CLUT Independence
    void testCLUTIndependence()
    {
        Clut clut;
        
        // Set different values in each CLUT type
        clut.set2bit(0, 0xAABBCCDD);
        clut.set4bit(0, 0x11223344);
        clut.set8bit(0, 0x55667788);
        
        // Verify each CLUT maintains its own values
        const std::uint32_t* array2bit = clut.getArray2bit();
        const std::uint32_t* array4bit = clut.getArray4bit();
        const std::uint32_t* array8bit = clut.getArray8bit();
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), array2bit[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), array4bit[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55667788), array8bit[0]);
        
        // Modify one and verify others are unchanged
        clut.set2bit(0, 0xFFFFFFFF);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), array2bit[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), array4bit[0]); // Unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55667788), array8bit[0]); // Unchanged
        
        // Test with different indices to ensure no cross-contamination
        clut.set2bit(1, 0x12345678);
        clut.set4bit(5, 0x9ABCDEF0);
        clut.set8bit(100, 0xFEDCBA98);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), array2bit[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x9ABCDEF0), array4bit[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFEDCBA98), array8bit[100]);
    }

    // Default CLUT Content Validation
    void testDefaultCLUTContent()
    {
        Clut clut;
        
        // Test that default CLUTs are properly initialized
        const std::uint32_t* array2bit = clut.getArray2bit();
        const std::uint32_t* array4bit = clut.getArray4bit();
        const std::uint32_t* array8bit = clut.getArray8bit();
        
        // Verify default values are not all zeros (they should be meaningful colors)
        bool hasNonZero2bit = false;
        for (int i = 0; i < 4; ++i)
        {
            if (array2bit[i] != 0)
            {
                hasNonZero2bit = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasNonZero2bit);
        
        bool hasNonZero4bit = false;
        for (int i = 0; i < 16; ++i)
        {
            if (array4bit[i] != 0)
            {
                hasNonZero4bit = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasNonZero4bit);
        
        bool hasNonZero8bit = false;
        for (int i = 0; i < 256; ++i)
        {
            if (array8bit[i] != 0)
            {
                hasNonZero8bit = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasNonZero8bit);
        
        // Test that two different Clut instances have the same default values
        Clut clut2;
        const std::uint32_t* array2bit2 = clut2.getArray2bit();
        const std::uint32_t* array4bit2 = clut2.getArray4bit();
        const std::uint32_t* array8bit2 = clut2.getArray8bit();
        
        for (int i = 0; i < 4; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(array2bit[i], array2bit2[i]);
        }
        for (int i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(array4bit[i], array4bit2[i]);
        }
        for (int i = 0; i < 256; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(array8bit[i], array8bit2[i]);
        }
    }

    // Reset Behavior with Mixed States
    void testResetBehaviorMixedStates()
    {
        Clut clut;
        
        // Set different values in all CLUT types and modify ID/version
        clut.setId(42);
        clut.setVersion(15);
        
        clut.set2bit(0, 0xAAAAAAAA);
        clut.set2bit(3, 0xBBBBBBBB);
        clut.set4bit(5, 0xCCCCCCCC);
        clut.set4bit(15, 0xDDDDDDDD);
        clut.set8bit(100, 0xEEEEEEEE);
        clut.set8bit(255, 0xFFFFFFFF);
        
        // Capture default state from fresh instance
        Clut defaultClut;
        const std::uint32_t* defaultArray2bit = defaultClut.getArray2bit();
        const std::uint32_t* defaultArray4bit = defaultClut.getArray4bit();
        const std::uint32_t* defaultArray8bit = defaultClut.getArray8bit();
        
        // Reset and verify all CLUTs return to defaults but ID persists
        clut.reset();
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(42), clut.getId()); // ID should persist
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion()); // Version should reset
        
        const std::uint32_t* array2bit = clut.getArray2bit();
        const std::uint32_t* array4bit = clut.getArray4bit();
        const std::uint32_t* array8bit = clut.getArray8bit();
        
        // Verify all CLUT values are restored to defaults
        for (int i = 0; i < 4; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(defaultArray2bit[i], array2bit[i]);
        }
        for (int i = 0; i < 16; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(defaultArray4bit[i], array4bit[i]);
        }
        for (int i = 0; i < 256; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(defaultArray8bit[i], array8bit[i]);
        }
    }

    // State Persistence
    void testStatePersistence()
    {
        Clut clut;
        
        // Test that ID persists through multiple resets
        clut.setId(123);
        clut.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(123), clut.getId());
        
        clut.setVersion(45);
        clut.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(123), clut.getId()); // Still persists
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion()); // Version resets
        
        // Change ID and verify it persists through another reset
        clut.setId(200);
        clut.setVersion(10);
        clut.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(200), clut.getId());
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion());
    }

    // Multiple Reset Calls
    void testMultipleResetCalls()
    {
        Clut clut;
        
        // Set some state
        clut.setId(88);
        clut.setVersion(77);
        clut.set2bit(0, 0x12345678);
        
        // Get initial reset state
        clut.reset();
        std::uint8_t idAfterFirstReset = clut.getId();
        std::uint8_t versionAfterFirstReset = clut.getVersion();
        const std::uint32_t* arrayAfterFirstReset = clut.getArray2bit();
        std::uint32_t valueAfterFirstReset = arrayAfterFirstReset[0];
        
        // Call reset multiple times and verify state doesn't change
        for (int i = 0; i < 5; ++i)
        {
            clut.reset();
            CPPUNIT_ASSERT_EQUAL(idAfterFirstReset, clut.getId());
            CPPUNIT_ASSERT_EQUAL(versionAfterFirstReset, clut.getVersion());
            const std::uint32_t* currentArray = clut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(valueAfterFirstReset, currentArray[0]);
        }
    }

    // Multiple Instance Independence
    void testMultipleInstanceIndependence()
    {
        Clut clut1;
        Clut clut2;
        Clut clut3;
        
        // Set different states for each instance
        clut1.setId(10);
        clut1.setVersion(20);
        clut1.set2bit(0, 0x11111111);
        
        clut2.setId(30);
        clut2.setVersion(40);
        clut2.set2bit(0, 0x22222222);
        
        clut3.setId(50);
        clut3.setVersion(60);
        clut3.set2bit(0, 0x33333333);
        
        // Verify each instance maintains its own state
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(10), clut1.getId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(20), clut1.getVersion());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11111111), clut1.getArray2bit()[0]);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(30), clut2.getId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(40), clut2.getVersion());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x22222222), clut2.getArray2bit()[0]);
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(50), clut3.getId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(60), clut3.getVersion());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x33333333), clut3.getArray2bit()[0]);
        
        // Reset one instance and verify others are unchanged
        clut2.reset();
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(10), clut1.getId()); // Unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(20), clut1.getVersion()); // Unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11111111), clut1.getArray2bit()[0]); // Unchanged
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(30), clut2.getId()); // ID persists
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut2.getVersion()); // Version reset
        
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(50), clut3.getId()); // Unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(60), clut3.getVersion()); // Unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x33333333), clut3.getArray2bit()[0]); // Unchanged
    }

    // Array Pointer Consistency
    void testArrayPointerConsistency()
    {
        Clut clut;
        
        // Get initial pointers
        const std::uint32_t* ptr2bit1 = clut.getArray2bit();
        const std::uint32_t* ptr4bit1 = clut.getArray4bit();
        const std::uint32_t* ptr8bit1 = clut.getArray8bit();
        
        // Modify CLUT data
        clut.set2bit(0, 0x12345678);
        clut.set4bit(5, 0x9ABCDEF0);
        clut.set8bit(100, 0xFEDCBA98);
        
        // Get pointers again and verify they're the same
        const std::uint32_t* ptr2bit2 = clut.getArray2bit();
        const std::uint32_t* ptr4bit2 = clut.getArray4bit();
        const std::uint32_t* ptr8bit2 = clut.getArray8bit();
        
        CPPUNIT_ASSERT_EQUAL(ptr2bit1, ptr2bit2);
        CPPUNIT_ASSERT_EQUAL(ptr4bit1, ptr4bit2);
        CPPUNIT_ASSERT_EQUAL(ptr8bit1, ptr8bit2);
        
        // Reset and verify pointers remain consistent
        clut.reset();
        const std::uint32_t* ptr2bit3 = clut.getArray2bit();
        const std::uint32_t* ptr4bit3 = clut.getArray4bit();
        const std::uint32_t* ptr8bit3 = clut.getArray8bit();
        
        CPPUNIT_ASSERT_EQUAL(ptr2bit1, ptr2bit3);
        CPPUNIT_ASSERT_EQUAL(ptr4bit1, ptr4bit3);
        CPPUNIT_ASSERT_EQUAL(ptr8bit1, ptr8bit3);
    }

    // Version Wrap-Around
    void testVersionWrapAround()
    {
        Clut clut;
        
        // Test version values around the boundary
        clut.setVersion(253);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(253), clut.getVersion());
        
        clut.setVersion(254);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(254), clut.getVersion());
        
        clut.setVersion(255); // INVALID_VERSION
        CPPUNIT_ASSERT_EQUAL(INVALID_VERSION, clut.getVersion());
        
        clut.setVersion(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), clut.getVersion());
        
        clut.setVersion(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), clut.getVersion());
    }

    // Specific Color Values
    void testSpecificColorValues()
    {
        Clut clut;
        
        // Test meaningful ARGB color values
        struct ColorTest {
            std::uint32_t argb;
            const char* description;
        };
        
        ColorTest colors[] = {
            {0x00000000, "Fully transparent black"},
            {0xFF000000, "Fully opaque black"},
            {0xFFFFFFFF, "Fully opaque white"},
            {0x00FFFFFF, "Fully transparent white"},
            {0xFFFF0000, "Opaque red"},
            {0xFF00FF00, "Opaque green"},
            {0xFF0000FF, "Opaque blue"},
            {0x80808080, "Semi-transparent gray"},
            {0xFF808080, "Opaque gray"},
            {0x80FF0000, "Semi-transparent red"}
        };
        
        for (const auto& color : colors)
        {
            // Test with 2-bit CLUT
            clut.set2bit(0, color.argb);
            CPPUNIT_ASSERT_EQUAL(color.argb, clut.getArray2bit()[0]);
            
            // Test with 4-bit CLUT
            clut.set4bit(0, color.argb);
            CPPUNIT_ASSERT_EQUAL(color.argb, clut.getArray4bit()[0]);
            
            // Test with 8-bit CLUT
            clut.set8bit(0, color.argb);
            CPPUNIT_ASSERT_EQUAL(color.argb, clut.getArray8bit()[0]);
        }
    }

    // Bit Pattern Correctness
    void testBitPatternCorrectness()
    {
        Clut clut;
        
        // Test that the corrected bitwise OR operation produces expected patterns
        for (int i = 0; i < 4; ++i)
        {
            std::uint32_t expected = (i << 24) | (i << 16) | i;
            clut.set2bit(i, expected);
            CPPUNIT_ASSERT_EQUAL(expected, clut.getArray2bit()[i]);
        }
        
        // Test specific bit patterns
        std::uint32_t patterns[] = {
            0x01010101, // Repeating pattern
            0x12121212, // Another repeating pattern
            0x03030003, // Mixed pattern
            0x00000000  // Zero pattern
        };
        
        for (std::uint32_t pattern : patterns)
        {
            clut.set2bit(0, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, clut.getArray2bit()[0]);
            
            clut.set4bit(0, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, clut.getArray4bit()[0]);
            
            clut.set8bit(0, pattern);
            CPPUNIT_ASSERT_EQUAL(pattern, clut.getArray8bit()[0]);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ClutTest);
