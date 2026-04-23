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
#include "ScteOutliner.hpp"
#include "ScteSimpleBitmap.hpp"
#include <vector>
#include <cstring>
#include <algorithm>

using namespace subttxrend::scte;

class ScteOutlinerTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteOutlinerTest );
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithMinimalDimensions);
    CPPUNIT_TEST(testConstructorWithLargeDimensions);
    CPPUNIT_TEST(testSetRangeWithFullRange);
    CPPUNIT_TEST(testSetRangeWithSingleRow);
    CPPUNIT_TEST(testOutlineWithThicknessZero);
    CPPUNIT_TEST(testOutlineWithThicknessOne);
    CPPUNIT_TEST(testOutlineWithThicknessFive);
    CPPUNIT_TEST(testOutlineOnEmptyBytemap);
    CPPUNIT_TEST(testOutlineOnSingleCharacterPixel);
    CPPUNIT_TEST(testOutlineOnMultipleCharacterPixels);
    CPPUNIT_TEST(testOutlineOnlyOverwritesTransparentAndFrame);
    CPPUNIT_TEST(testOutlinePreservesCharacterPixels);
    CPPUNIT_TEST(testOutlinePreservesExistingOutline);
    CPPUNIT_TEST(testOutlineAtTopLeftCorner);
    CPPUNIT_TEST(testOutlineAtBottomRightCorner);
    CPPUNIT_TEST(testOutlineAtTopEdge);
    CPPUNIT_TEST(testOutlineAtBottomEdge);
    CPPUNIT_TEST(testOutlineAtLeftEdge);
    CPPUNIT_TEST(testOutlineAtRightEdge);
    CPPUNIT_TEST(testOutlineWithRestrictedRange);
    CPPUNIT_TEST(testOutlineWithRangeNotCoveringCharacterPixels);
    CPPUNIT_TEST(testMultipleOutlineCallsOnSameBytemap);
    CPPUNIT_TEST(testOutlineCreatesCircularPattern);
    CPPUNIT_TEST(testOutlineWithDifferentColorValues);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Outliner::initCircle() initializes its internal circle mask only once
        // (static local flag). Test execution order can otherwise lock the mask
        // to a smaller thickness and make later thickness>1 tests fail.
        // Initialize it deterministically with the maximum thickness used here.
        std::vector<uint8_t> bytemap = createBytemap(1, 1);
        setBytemapPixel(bytemap, 1, 0, 0, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 1, 1);
        outliner.outline(5, COLOR_OUTLINE);
    }

    void tearDown()
    {
        // Cleanup code here
    }

protected:
    // Helper function to create a bytemap filled with specific value
    std::vector<uint8_t> createBytemap(uint32_t width, uint32_t height, uint8_t fillValue = COLOR_TRANSPARENT)
    {
        return std::vector<uint8_t>(width * height, fillValue);
    }

    // Helper function to set a pixel in bytemap
    void setBytemapPixel(std::vector<uint8_t>& bytemap, uint32_t width, uint32_t x, uint32_t y, uint8_t value)
    {
        bytemap[y * width + x] = value;
    }

    // Helper function to get a pixel from bytemap
    uint8_t getBytemapPixel(const std::vector<uint8_t>& bytemap, uint32_t width, uint32_t x, uint32_t y)
    {
        return bytemap[y * width + x];
    }

    // Helper function to count pixels with specific value
    uint32_t countPixels(const std::vector<uint8_t>& bytemap, uint8_t value)
    {
        return std::count(bytemap.begin(), bytemap.end(), value);
    }

    // Helper function to verify pixel is within bounds
    bool isPixelInBounds(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        return x < width && y < height;
    }

    void testConstructorWithValidParameters()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);

        // Constructor should not throw
        CPPUNIT_ASSERT_NO_THROW(
            Outliner outliner(bytemap.data(), 10, 10)
        );
    }

    void testConstructorWithMinimalDimensions()
    {
        std::vector<uint8_t> bytemap = createBytemap(1, 1);

        CPPUNIT_ASSERT_NO_THROW(
            Outliner outliner(bytemap.data(), 1, 1)
        );
    }

    void testConstructorWithLargeDimensions()
    {
        std::vector<uint8_t> bytemap = createBytemap(1024, 1024);

        CPPUNIT_ASSERT_NO_THROW(
            Outliner outliner(bytemap.data(), 1024, 1024)
        );
    }

    void testSetRangeWithFullRange()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);

        Coords tl{0, 0};
        Coords br{9, 9};

        outliner.setRange(tl, br);
        outliner.outline(1, COLOR_OUTLINE);

        // Should create outline around character pixel
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testSetRangeWithSingleRow()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 4, COLOR_CHARACTER);  // In range
        setBytemapPixel(bytemap, 10, 5, 7, COLOR_CHARACTER);  // Out of range

        Outliner outliner(bytemap.data(), 10, 10);

        Coords tl{0, 4};
        Coords br{9, 4};

        outliner.setRange(tl, br);
        outliner.outline(1, COLOR_OUTLINE);

        // setRange restricts which character pixels are scanned (only row 4)
        // Outline should be drawn around character at (5,4) which is in range
        bool hasOutlineNearInRange = (getBytemapPixel(bytemap, 10, 4, 4) == COLOR_OUTLINE ||
                                       getBytemapPixel(bytemap, 10, 6, 4) == COLOR_OUTLINE ||
                                       getBytemapPixel(bytemap, 10, 5, 3) == COLOR_OUTLINE ||
                                       getBytemapPixel(bytemap, 10, 5, 5) == COLOR_OUTLINE);
        CPPUNIT_ASSERT(hasOutlineNearInRange);

        // Character pixel at (5,7) is outside range [4,4], should NOT be processed
        bool hasOutlineNearOutOfRange = (getBytemapPixel(bytemap, 10, 4, 7) == COLOR_OUTLINE ||
                                          getBytemapPixel(bytemap, 10, 6, 7) == COLOR_OUTLINE);
        CPPUNIT_ASSERT(!hasOutlineNearOutOfRange);

        // Both character pixels should remain unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 4));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 7));
    }

    void testOutlineWithThicknessZero()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(0, COLOR_OUTLINE);

        // With thickness 0, no outline should be drawn
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), outlineCount);
    }

    void testOutlineWithThicknessOne()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(1, COLOR_OUTLINE);

        // Should create outline around character pixel
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);

        // Verify immediate neighbors have outline
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 4, 5));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 6, 5));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 5, 4));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 5, 6));
    }

    void testOutlineWithThicknessFive()
    {
        std::vector<uint8_t> bytemap = createBytemap(20, 20);
        setBytemapPixel(bytemap, 20, 10, 10, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 20, 20);
        outliner.outline(5, COLOR_OUTLINE);

        // Should create wider outline
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 20); // More pixels than thickness 1

        // Verify pixels at distance 5 have outline (within circular pattern)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 20, 5, 10));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 20, 15, 10));
    }

    void testOutlineOnEmptyBytemap()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10, COLOR_TRANSPARENT);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(3, COLOR_OUTLINE);

        // No character pixels, so no outline should be drawn
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), outlineCount);
    }

    void testOutlineOnSingleCharacterPixel()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);

        uint32_t characterCountBefore = countPixels(bytemap, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(2, COLOR_OUTLINE);

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        uint32_t characterCountAfter = countPixels(bytemap, COLOR_CHARACTER);

        // Outline should be created
        CPPUNIT_ASSERT(outlineCount > 0);

        // Character pixel should remain
        CPPUNIT_ASSERT_EQUAL(characterCountBefore, characterCountAfter);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 5));
    }

    void testOutlineOnMultipleCharacterPixels()
    {
        std::vector<uint8_t> bytemap = createBytemap(20, 20);
        setBytemapPixel(bytemap, 20, 5, 5, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 20, 15, 15, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 20, 10, 10, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 20, 20);
        outliner.outline(2, COLOR_OUTLINE);

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        uint32_t characterCount = countPixels(bytemap, COLOR_CHARACTER);

        // Outline should be created around all character pixels
        CPPUNIT_ASSERT(outlineCount > 0);

        // All 3 character pixels should remain
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(3), characterCount);
    }

    void testOutlineOnlyOverwritesTransparentAndFrame()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);

        // Set up different pixel types around a character pixel
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 10, 4, 5, COLOR_TRANSPARENT);
        setBytemapPixel(bytemap, 10, 6, 5, COLOR_FRAME);
        setBytemapPixel(bytemap, 10, 5, 4, COLOR_OUTLINE);
        setBytemapPixel(bytemap, 10, 5, 6, COLOR_SHADOW);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(1, COLOR_OUTLINE);

        // TRANSPARENT should be overwritten
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 4, 5));

        // FRAME should be overwritten
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 6, 5));

        // Existing OUTLINE should remain (not overwritten)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 5, 4));

        // SHADOW should NOT be overwritten
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_SHADOW), getBytemapPixel(bytemap, 10, 5, 6));
    }

    void testOutlinePreservesCharacterPixels()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 10, 6, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(3, COLOR_OUTLINE);

        // Character pixels should never be overwritten
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 5));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 6, 5));

        uint32_t characterCount = countPixels(bytemap, COLOR_CHARACTER);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(2), characterCount);
    }

    void testOutlinePreservesExistingOutline()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 10, 4, 5, COLOR_OUTLINE);

        uint32_t outlineCountBefore = countPixels(bytemap, COLOR_OUTLINE);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(1, COLOR_OUTLINE);

        // Existing outline pixel should be preserved
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 10, 4, 5));

        uint32_t outlineCountAfter = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCountAfter >= outlineCountBefore);
    }

    void testOutlineAtTopLeftCorner()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 0, 0, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);

        // Should not crash or access out of bounds
        CPPUNIT_ASSERT_NO_THROW(
            outliner.outline(2, COLOR_OUTLINE)
        );

        // Character pixel should remain
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 0, 0));

        // Some outline should be drawn (only in valid quadrants)
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineAtBottomRightCorner()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 9, 9, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);

        CPPUNIT_ASSERT_NO_THROW(
            outliner.outline(2, COLOR_OUTLINE)
        );

        // Character pixel should remain
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 9, 9));

        // Some outline should be drawn
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineAtTopEdge()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 0, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(2, COLOR_OUTLINE);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 0));

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineAtBottomEdge()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 9, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(2, COLOR_OUTLINE);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 5, 9));

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineAtLeftEdge()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 0, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(2, COLOR_OUTLINE);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 0, 5));

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineAtRightEdge()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 9, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);
        outliner.outline(2, COLOR_OUTLINE);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 10, 9, 5));

        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCount > 0);
    }

    void testOutlineWithRestrictedRange()
    {
        std::vector<uint8_t> bytemap = createBytemap(20, 20);
        setBytemapPixel(bytemap, 20, 10, 5, COLOR_CHARACTER);
        setBytemapPixel(bytemap, 20, 10, 15, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 20, 20);

        Coords tl{0, 3};
        Coords br{19, 10};
        outliner.setRange(tl, br);

        outliner.outline(2, COLOR_OUTLINE);

        // Should have outline around first pixel (y=5 is in range [3,10])
        bool hasOutlineNearFirst = false;
        for (uint32_t y = 3; y <= 7 && !hasOutlineNearFirst; ++y) {
            for (uint32_t x = 8; x <= 12 && !hasOutlineNearFirst; ++x) {
                if (getBytemapPixel(bytemap, 20, x, y) == COLOR_OUTLINE) {
                    hasOutlineNearFirst = true;
                }
            }
        }
        CPPUNIT_ASSERT(hasOutlineNearFirst);

        // Should NOT have outline around second pixel (y=15 is outside range [3,10])
        bool hasOutlineNearSecond = false;
        for (uint32_t y = 13; y <= 17 && !hasOutlineNearSecond; ++y) {
            for (uint32_t x = 8; x <= 12 && !hasOutlineNearSecond; ++x) {
                if (getBytemapPixel(bytemap, 20, x, y) == COLOR_OUTLINE) {
                    hasOutlineNearSecond = true;
                }
            }
        }
        CPPUNIT_ASSERT(!hasOutlineNearSecond);
    }

    void testOutlineWithRangeNotCoveringCharacterPixels()
    {
        std::vector<uint8_t> bytemap = createBytemap(20, 20);
        setBytemapPixel(bytemap, 20, 10, 10, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 20, 20);

        Coords tl{0, 0};
        Coords br{19, 5};
        outliner.setRange(tl, br);

        outliner.outline(2, COLOR_OUTLINE);

        // Character pixel at y=10 is outside range [0,5], no outline should be drawn
        uint32_t outlineCount = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), outlineCount);
    }

    void testMultipleOutlineCallsOnSameBytemap()
    {
        std::vector<uint8_t> bytemap = createBytemap(20, 20);
        setBytemapPixel(bytemap, 20, 10, 10, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 20, 20);

        // First outline call
        outliner.outline(1, COLOR_OUTLINE);
        uint32_t outlineCountFirst = countPixels(bytemap, COLOR_OUTLINE);
        CPPUNIT_ASSERT(outlineCountFirst > 0);

        // Second outline call (existing outline should be preserved)
        outliner.outline(2, COLOR_OUTLINE);
        uint32_t outlineCountSecond = countPixels(bytemap, COLOR_OUTLINE);

        // Second call should not reduce outline count
        CPPUNIT_ASSERT(outlineCountSecond >= outlineCountFirst);

        // Character pixel should still be preserved
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 20, 10, 10));
    }

    void testOutlineCreatesCircularPattern()
    {
        std::vector<uint8_t> bytemap = createBytemap(30, 30);
        setBytemapPixel(bytemap, 30, 15, 15, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 30, 30);
        outliner.outline(5, COLOR_OUTLINE);

        // Verify circular pattern by checking distance from center
        // Pixels at distance <= 5 (within circle) should have outline
        // Pixels at exact distance 5 on axes should have outline
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 10, 15)); // left
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 20, 15)); // right
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 15, 10)); // top
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 15, 20)); // bottom

        // Diagonal pixels at distance ~7 (5*sqrt(2)/2 ≈ 3.5 in each direction)
        // should have outline
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 12, 12)); // top-left
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), getBytemapPixel(bytemap, 30, 18, 18)); // bottom-right

        // Character pixel should be preserved
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), getBytemapPixel(bytemap, 30, 15, 15));
    }

    void testOutlineWithDifferentColorValues()
    {
        std::vector<uint8_t> bytemap = createBytemap(10, 10);
        setBytemapPixel(bytemap, 10, 5, 5, COLOR_CHARACTER);

        Outliner outliner(bytemap.data(), 10, 10);

        // Outline with COLOR_SHADOW value
        outliner.outline(1, COLOR_SHADOW);

        uint32_t shadowCount = countPixels(bytemap, COLOR_SHADOW);
        CPPUNIT_ASSERT(shadowCount > 0);

        // Verify specific pixels have shadow color
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_SHADOW), getBytemapPixel(bytemap, 10, 4, 5));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_SHADOW), getBytemapPixel(bytemap, 10, 6, 5));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteOutlinerTest );
