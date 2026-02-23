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
#include "GfxTtxClut.hpp"
#include <cstddef>
#include <climits>

using namespace subttxrend::ttxt;

class GfxTtxClutTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxTtxClutTest);
    CPPUNIT_TEST(testConstructor_InitializesAllColorsToZero);
    CPPUNIT_TEST(testConstructor_InitializesCorrectArraySize);
    CPPUNIT_TEST(testResetColors_SetsAllThirtyTwoColors);
    CPPUNIT_TEST(testResetColors_IsIdempotent);
    CPPUNIT_TEST(testResetColors_OverwritesCustomColors);
    CPPUNIT_TEST(testSetColor_ValidIndex_MinimumBoundary);
    CPPUNIT_TEST(testSetColor_ValidIndex_MaximumBoundary);
    CPPUNIT_TEST(testSetColor_ValidIndex_MiddleRange);
    CPPUNIT_TEST(testSetColor_UpdatesTargetIndexOnly);
    CPPUNIT_TEST(testSetColor_WithFullOpacity);
    CPPUNIT_TEST(testSetColor_WithFullTransparency);
    CPPUNIT_TEST(testSetColor_WithPartialTransparency);
    CPPUNIT_TEST(testSetColor_WithMaximumColorValue);
    CPPUNIT_TEST(testSetColor_WithMinimumColorValue);
    CPPUNIT_TEST(testSetColor_ReturnsTrueWhenColorChanges);
    CPPUNIT_TEST(testSetColor_ReturnsFalseWhenColorUnchanged);
    CPPUNIT_TEST(testSetColor_MultipleSequentialUpdates);
    CPPUNIT_TEST(testSetColor_InvalidIndex_EqualToSize);
    CPPUNIT_TEST(testSetColor_InvalidIndex_LargeValue);
    CPPUNIT_TEST(testSetColor_InvalidIndex_VeryLargeValue);
    CPPUNIT_TEST(testSetColor_InvalidIndex_DoesNotModifyArray);
    CPPUNIT_TEST(testGetArray_ReturnsNonNullPointer);
    CPPUNIT_TEST(testGetArray_ReturnsCorrectDataAfterConstruction);
    CPPUNIT_TEST(testGetArray_ReturnsCorrectDataAfterResetColors);
    CPPUNIT_TEST(testGetArray_ReturnsCorrectDataAfterSetColor);
    CPPUNIT_TEST(testGetArray_ReturnsConsistentPointer);
    CPPUNIT_TEST(testGetArray_ReflectsMultipleColorChanges);
    CPPUNIT_TEST(testGetSize_ReturnsThirtyTwo);
    CPPUNIT_TEST(testGetSize_UnchangedAfterResetColors);
    CPPUNIT_TEST(testGetSize_UnchangedAfterSetColor);
    CPPUNIT_TEST(testPublicConstant_ColorIndexBlack);
    CPPUNIT_TEST(testPublicConstant_ColorIndexRed);
    CPPUNIT_TEST(testPublicConstant_ColorIndexGreen);
    CPPUNIT_TEST(testPublicConstant_ColorIndexYellow);
    CPPUNIT_TEST(testPublicConstant_ColorIndexCyan);
    CPPUNIT_TEST(testPublicConstant_ColorIndexWhite);
    CPPUNIT_TEST(testPublicConstant_ColorIndexTransparent);
    CPPUNIT_TEST(testPublicConstant_UsableWithSetColor);
    CPPUNIT_TEST(testIntegration_FullLifecycle);
    CPPUNIT_TEST(testIntegration_MultipleInstances);
    CPPUNIT_TEST(testIntegration_StateConsistency);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override {}
    void tearDown() override {}

protected:
    void testConstructor_InitializesAllColorsToZero()
    {
        GfxTtxClut clut;
        const std::uint32_t* array = clut.getArray();

        for (std::size_t i = 0; i < clut.getSize(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index should be initialized to 0",
                                         static_cast<std::uint32_t>(0), array[i]);
        }
    }

    void testConstructor_InitializesCorrectArraySize()
    {
        GfxTtxClut clut;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("CLUT size should be 32",
                                     static_cast<std::size_t>(32), clut.getSize());
    }

    void testResetColors_SetsAllThirtyTwoColors()
    {
        GfxTtxClut clut;
        clut.resetColors();
        const std::uint32_t* array = clut.getArray();

        // Verify all 32 colors are set (some to specific known values)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF000000), array[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF0000), array[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF00FF00), array[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFF00), array[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF0000FF), array[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF00FF), array[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF00FFFF), array[6]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFF0FF), array[7]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), array[8]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF770000), array[9]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF007700), array[10]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF777700), array[11]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF000077), array[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF770077), array[13]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF007777), array[14]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF777777), array[15]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF0055), array[16]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF7700), array[17]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF00FF77), array[18]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFBB), array[19]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF00CCAA), array[20]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF550000), array[21]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF665522), array[22]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFCC7777), array[23]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF333333), array[24]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF7777), array[25]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF77FF77), array[26]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFF77), array[27]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF7777FF), array[28]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF77FF), array[29]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF77FFFF), array[30]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFDDDDDD), array[31]);
    }

    void testResetColors_IsIdempotent()
    {
        GfxTtxClut clut;
        clut.resetColors();
        const std::uint32_t* array1 = clut.getArray();
        std::uint32_t firstColor = array1[0];

        clut.resetColors();
        const std::uint32_t* array2 = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Multiple resetColors calls should produce same result",
                                     firstColor, array2[0]);

        // Verify all colors remain the same
        for (std::size_t i = 0; i < clut.getSize(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(array1[i], array2[i]);
        }
    }

    void testResetColors_OverwritesCustomColors()
    {
        GfxTtxClut clut;
        clut.setColor(0, 0x12345678);
        clut.setColor(15, 0xAABBCCDD);

        clut.resetColors();
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom color at index 0 should be overwritten",
                                     static_cast<std::uint32_t>(0xFF000000), array[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom color at index 15 should be overwritten",
                                     static_cast<std::uint32_t>(0xFF777777), array[15]);
    }

    void testSetColor_ValidIndex_MinimumBoundary()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(0, 0xAABBCCDD);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_MESSAGE("setColor should return true for valid index 0", result);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 0 should be updated",
                                     static_cast<std::uint32_t>(0xAABBCCDD), array[0]);
    }

    void testSetColor_ValidIndex_MaximumBoundary()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(31, 0x11223344);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_MESSAGE("setColor should return true for valid index 31", result);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 31 should be updated",
                                     static_cast<std::uint32_t>(0x11223344), array[31]);
    }

    void testSetColor_ValidIndex_MiddleRange()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(15, 0xDEADBEEF);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_MESSAGE("setColor should return true for valid index 15", result);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 15 should be updated",
                                     static_cast<std::uint32_t>(0xDEADBEEF), array[15]);
    }

    void testSetColor_UpdatesTargetIndexOnly()
    {
        GfxTtxClut clut;
        clut.resetColors();
        const std::uint32_t* array = clut.getArray();

        std::uint32_t colorBefore14 = array[14];
        std::uint32_t colorBefore16 = array[16];

        clut.setColor(15, 0x99887766);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 14 should not change",
                                     colorBefore14, array[14]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 15 should be updated",
                                     static_cast<std::uint32_t>(0x99887766), array[15]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color at index 16 should not change",
                                     colorBefore16, array[16]);
    }

    void testSetColor_WithFullOpacity()
    {
        GfxTtxClut clut;
        clut.setColor(10, 0xFFFF0000);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Fully opaque color should be stored correctly",
                                     static_cast<std::uint32_t>(0xFFFF0000), array[10]);
    }

    void testSetColor_WithFullTransparency()
    {
        GfxTtxClut clut;
        clut.setColor(10, 0x00FFFFFF);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Fully transparent color should be stored correctly",
                                     static_cast<std::uint32_t>(0x00FFFFFF), array[10]);
    }

    void testSetColor_WithPartialTransparency()
    {
        GfxTtxClut clut;
        clut.setColor(10, 0x80FF00FF);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Semi-transparent color should be stored correctly",
                                     static_cast<std::uint32_t>(0x80FF00FF), array[10]);
    }

    void testSetColor_WithMaximumColorValue()
    {
        GfxTtxClut clut;
        clut.setColor(20, 0xFFFFFFFF);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Maximum color value should be stored correctly",
                                     static_cast<std::uint32_t>(0xFFFFFFFF), array[20]);
    }

    void testSetColor_WithMinimumColorValue()
    {
        GfxTtxClut clut;
        clut.setColor(20, 0x00000000);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Minimum color value should be stored correctly",
                                     static_cast<std::uint32_t>(0x00000000), array[20]);
    }

    void testSetColor_ReturnsTrueWhenColorChanges()
    {
        GfxTtxClut clut;
        clut.setColor(5, 0x11111111);

        bool result = clut.setColor(5, 0x22222222);

        CPPUNIT_ASSERT_MESSAGE("setColor should return true when color changes", result);
    }

    void testSetColor_ReturnsFalseWhenColorUnchanged()
    {
        GfxTtxClut clut;
        clut.setColor(5, 0x11111111);

        bool result = clut.setColor(5, 0x11111111);

        CPPUNIT_ASSERT_MESSAGE("setColor should return false when color is unchanged", !result);
    }

    void testSetColor_MultipleSequentialUpdates()
    {
        GfxTtxClut clut;
        const std::uint32_t* array = clut.getArray();

        clut.setColor(0, 0x11111111);
        clut.setColor(1, 0x22222222);
        clut.setColor(2, 0x33333333);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11111111), array[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x22222222), array[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x33333333), array[2]);
    }

    void testSetColor_InvalidIndex_EqualToSize()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(32, 0xAABBCCDD);

        CPPUNIT_ASSERT_MESSAGE("setColor should return false for index equal to size", !result);
    }

    void testSetColor_InvalidIndex_LargeValue()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(100, 0xAABBCCDD);

        CPPUNIT_ASSERT_MESSAGE("setColor should return false for large invalid index", !result);
    }

    void testSetColor_InvalidIndex_VeryLargeValue()
    {
        GfxTtxClut clut;
        bool result = clut.setColor(SIZE_MAX, 0xAABBCCDD);

        CPPUNIT_ASSERT_MESSAGE("setColor should return false for SIZE_MAX", !result);
    }

    void testSetColor_InvalidIndex_DoesNotModifyArray()
    {
        GfxTtxClut clut;
        clut.resetColors();
        const std::uint32_t* array = clut.getArray();

        // Store initial state
        std::uint32_t initialColors[32];
        for (std::size_t i = 0; i < 32; ++i)
        {
            initialColors[i] = array[i];
        }

        // Attempt invalid updates
        clut.setColor(32, 0xFFFFFFFF);
        clut.setColor(100, 0xFFFFFFFF);

        // Verify no changes
        for (std::size_t i = 0; i < 32; ++i)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Array should not be modified by invalid index",
                                         initialColors[i], array[i]);
        }
    }

    void testGetArray_ReturnsNonNullPointer()
    {
        GfxTtxClut clut;
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_MESSAGE("getArray should return non-null pointer", array != nullptr);
    }

    void testGetArray_ReturnsCorrectDataAfterConstruction()
    {
        GfxTtxClut clut;
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("First element should be 0 after construction",
                                     static_cast<std::uint32_t>(0), array[0]);
    }

    void testGetArray_ReturnsCorrectDataAfterResetColors()
    {
        GfxTtxClut clut;
        clut.resetColors();
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("First element should be BLACK after reset",
                                     static_cast<std::uint32_t>(0xFF000000), array[0]);
    }

    void testGetArray_ReturnsCorrectDataAfterSetColor()
    {
        GfxTtxClut clut;
        clut.setColor(10, 0xDEADBEEF);
        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Element 10 should reflect setColor change",
                                     static_cast<std::uint32_t>(0xDEADBEEF), array[10]);
    }

    void testGetArray_ReturnsConsistentPointer()
    {
        GfxTtxClut clut;
        const std::uint32_t* array1 = clut.getArray();
        const std::uint32_t* array2 = clut.getArray();

        CPPUNIT_ASSERT_MESSAGE("getArray should return same pointer", array1 == array2);
    }

    void testGetArray_ReflectsMultipleColorChanges()
    {
        GfxTtxClut clut;
        clut.setColor(0, 0x11111111);
        clut.setColor(15, 0x22222222);
        clut.setColor(31, 0x33333333);

        const std::uint32_t* array = clut.getArray();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11111111), array[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x22222222), array[15]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x33333333), array[31]);
    }

    void testGetSize_ReturnsThirtyTwo()
    {
        GfxTtxClut clut;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getSize should return 32",
                                     static_cast<std::size_t>(32), clut.getSize());
    }

    void testGetSize_UnchangedAfterResetColors()
    {
        GfxTtxClut clut;
        clut.resetColors();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("getSize should still return 32 after reset",
                                     static_cast<std::size_t>(32), clut.getSize());
    }

    void testGetSize_UnchangedAfterSetColor()
    {
        GfxTtxClut clut;
        clut.setColor(10, 0xFFFFFFFF);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("getSize should still return 32 after setColor",
                                     static_cast<std::size_t>(32), clut.getSize());
    }

    void testPublicConstant_ColorIndexBlack()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_BLACK;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_BLACK should be 0",
                                     static_cast<std::uint8_t>(0), value);
    }

    void testPublicConstant_ColorIndexRed()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_RED;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_RED should be 1",
                                     static_cast<std::uint8_t>(1), value);
    }

    void testPublicConstant_ColorIndexGreen()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_GREEN;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_GREEN should be 2",
                                     static_cast<std::uint8_t>(2), value);
    }

    void testPublicConstant_ColorIndexYellow()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_YELLOW;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_YELLOW should be 3",
                                     static_cast<std::uint8_t>(3), value);
    }

    void testPublicConstant_ColorIndexCyan()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_CYAN;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_CYAN should be 5",
                                     static_cast<std::uint8_t>(5), value);
    }

    void testPublicConstant_ColorIndexWhite()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_WHITE;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_WHITE should be 7",
                                     static_cast<std::uint8_t>(7), value);
    }

    void testPublicConstant_ColorIndexTransparent()
    {
        std::uint8_t value = GfxTtxClut::COLOR_INDEX_TRANSPARENT;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("COLOR_INDEX_TRANSPARENT should be 8",
                                     static_cast<std::uint8_t>(8), value);
    }

    void testPublicConstant_UsableWithSetColor()
    {
        GfxTtxClut clut;

        bool result = clut.setColor(1, 0xFFAABBCC);

        CPPUNIT_ASSERT_MESSAGE("Public constants should work with setColor", result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFAABBCC),
                             clut.getArray()[1]);
    }

    void testIntegration_FullLifecycle()
    {
        GfxTtxClut clut;

        // Initial state
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(32), clut.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), clut.getArray()[0]);

        // Reset to standard colors
        clut.resetColors();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF000000), clut.getArray()[0]);

        // Customize specific colors
        clut.setColor(0, 0x12345678);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), clut.getArray()[0]);

        // Reset again
        clut.resetColors();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF000000), clut.getArray()[0]);
    }

    void testIntegration_MultipleInstances()
    {
        GfxTtxClut clut1;
        GfxTtxClut clut2;

        clut1.setColor(0, 0x11111111);
        clut2.setColor(0, 0x22222222);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Instance 1 should have its own color",
                                     static_cast<std::uint32_t>(0x11111111), clut1.getArray()[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Instance 2 should have its own color",
                                     static_cast<std::uint32_t>(0x22222222), clut2.getArray()[0]);
    }

    void testIntegration_StateConsistency()
    {
        GfxTtxClut clut;
        clut.resetColors();

        // Make multiple changes
        clut.setColor(5, 0xAABBCCDD);
        clut.setColor(10, 0x11223344);
        clut.setColor(20, 0xFFEEDDCC);

        const std::uint32_t* array = clut.getArray();

        // Verify specific changes
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), array[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), array[10]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFEEDDCC), array[20]);

        // Verify unchanged colors still have standard values
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF000000), array[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF0000), array[1]);
    }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxTtxClutTest);