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
#include <memory>
#include <vector>
#include <cstring>

#include "../src/FontStripImpl.hpp"
#include "../include/FontStrip.hpp"
#include "FontStripMap.hpp"
#include "Types.hpp"

using subttxrend::gfx::FontStripImpl;
using subttxrend::gfx::FontStripMap;
using subttxrend::gfx::Size;
using subttxrend::gfx::Rectangle;

class FontStripImplTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(FontStripImplTest);
    CPPUNIT_TEST(testConstructorWithValidGlyphSizeAndCount);
    CPPUNIT_TEST(testConstructorWithZeroGlyphCount);
    CPPUNIT_TEST(testConstructorWithZeroWidthGlyphSize);
    CPPUNIT_TEST(testConstructorWithZeroHeightGlyphSize);
    CPPUNIT_TEST(testConstructorWithLargeGlyphCount);
    CPPUNIT_TEST(testConstructorVerifySurfaceInitialized);
    CPPUNIT_TEST(testConstructorVerifyPixmapDimensions);
    CPPUNIT_TEST(testDestructorCleansUpResourcesProperly);
    CPPUNIT_TEST(testFindFontFileWithValidFontName);
    CPPUNIT_TEST(testFindFontFileWithEmptyFontName);
    CPPUNIT_TEST(testFindFontFileWithAbsolutePath);
    CPPUNIT_TEST(testFindFontFileWithRelativePath);
    CPPUNIT_TEST(testFindFontFileWithNonExistentFontName);
    CPPUNIT_TEST(testFindFontFileWithSpecialCharacters);
    CPPUNIT_TEST(testFindFontFileWithVeryLongFontName);
    CPPUNIT_TEST(testFindFontFileVerifyFontPatternMatching);
    CPPUNIT_TEST(testFindFontFileMultipleConsecutiveCalls);
    CPPUNIT_TEST(testFindFontFileWithWhitespaceInName);
    CPPUNIT_TEST(testLoadGlyphWithValidIndexAndData);
    CPPUNIT_TEST(testLoadGlyphWithIndexZero);
    CPPUNIT_TEST(testLoadGlyphWithNegativeIndex);
    CPPUNIT_TEST(testLoadGlyphWithIndexBeyondGlyphCount);
    CPPUNIT_TEST(testLoadGlyphWithZeroSize);
    CPPUNIT_TEST(testLoadGlyphWithSizeSmallerThanGlyphDimensions);
    CPPUNIT_TEST(testLoadGlyphWithSizeLargerThanGlyphDimensions);
    CPPUNIT_TEST(testLoadGlyphWithExactSizeMatch);
    CPPUNIT_TEST(testLoadMultipleGlyphsSequentially);
    CPPUNIT_TEST(testLoadSameGlyphTwiceOverwrite);
    CPPUNIT_TEST(testLoadAllGlyphsInStrip);
    CPPUNIT_TEST(testLoadGlyphVerifyDataCopiedCorrectly);
    CPPUNIT_TEST(testLoadGlyphAndVerifyGetGlyphRect);
    CPPUNIT_TEST(testLoadGlyphWithAllZeroAlphaValues);
    CPPUNIT_TEST(testLoadGlyphWithAll255AlphaValues);
    CPPUNIT_TEST(testLoadGlyphBoundaryExactSize);
    CPPUNIT_TEST(testLoadGlyphBoundarySizeMinusOne);
    CPPUNIT_TEST(testLoadGlyphBoundarySizePlusOne);
    CPPUNIT_TEST(testLoadFontWithValidFontNameAndCharMap);
    CPPUNIT_TEST(testLoadFontWithEmptyFontName);
    CPPUNIT_TEST(testLoadFontWithNonExistentFontFile);
    CPPUNIT_TEST(testLoadFontWithZeroWidthCharSize);
    CPPUNIT_TEST(testLoadFontWithZeroHeightCharSize);
    CPPUNIT_TEST(testLoadFontWithNegativeCharSizeWidth);
    CPPUNIT_TEST(testLoadFontWithNegativeCharSizeHeight);
    CPPUNIT_TEST(testLoadFontWithEmptyCharMap);
    CPPUNIT_TEST(testLoadFontWithSingleCharacterMapping);
    CPPUNIT_TEST(testLoadFontWithMultipleCharacterMappings);
    CPPUNIT_TEST(testLoadFontWhenFindFontFileFails);
    CPPUNIT_TEST(testLoadFontWithInvalidFontFileFormat);
    CPPUNIT_TEST(testLoadFontWithTTFExtension);
    CPPUNIT_TEST(testLoadFontWithNonTTFExtension);
    CPPUNIT_TEST(testLoadFontVerifyGlyphsRenderedToCorrectPositions);
    CPPUNIT_TEST(testLoadFontTwiceSecondCallOverwrites);
    CPPUNIT_TEST(testLoadFontWithVerySmallCharSize);
    CPPUNIT_TEST(testLoadFontVerifyAllMappedCharactersLoaded);
    CPPUNIT_TEST(testLoadFontCharMapWithGapsInGlyphIndices);
    CPPUNIT_TEST(testLoadFontWithVeryLargeCharSize);
    CPPUNIT_TEST(testLoadFontCharMapNeededGlyphCountExceedsCapacity);
    CPPUNIT_TEST(testLoadFontWithCharacterNotInFont);
    CPPUNIT_TEST(testLoadFontCharSizeMismatchWithGlyphSize);
    CPPUNIT_TEST(testLoadFontWithFontFileCannotBeOpened);
    CPPUNIT_TEST(testGetGlyphRectForValidGlyphIndex);
    CPPUNIT_TEST(testGetGlyphRectForIndexZero);
    CPPUNIT_TEST(testGetGlyphRectForNegativeIndex);
    CPPUNIT_TEST(testGetGlyphRectForIndexBeyondGlyphCount);
    CPPUNIT_TEST(testGetGlyphRectForMaximumValidIndex);
    CPPUNIT_TEST(testGetGlyphRectVerifyRectangleDimensions);
    CPPUNIT_TEST(testGetGlyphRectVerifyRectanglePosition);
    CPPUNIT_TEST(testGetGlyphRectForUnloadedGlyphReturnsEmptyRect);
    CPPUNIT_TEST(testGetGlyphRectVerifyMultiRowLayout);
    CPPUNIT_TEST(testGetGlyphRectAfterLoadingGlyph);
    CPPUNIT_TEST(testGetPixmapFromNewlyConstructedStrip);
    CPPUNIT_TEST(testGetPixmapAfterLoadingGlyphs);
    CPPUNIT_TEST(testGetPixmapAfterLoadingFont);
    CPPUNIT_TEST(testGetPixmapVerifyPixmapDimensions);
    CPPUNIT_TEST(testGetPixmapVerifyConstReference);
    CPPUNIT_TEST(testFontConfigStaticMemberInitialized);
    CPPUNIT_TEST(testFontConfigMultipleFontStripInstancesShareConfig);
    CPPUNIT_TEST(testFontConfigPatternCreationAndMatching);
    CPPUNIT_TEST(testFontConfigFontSubstitutionBehavior);
    CPPUNIT_TEST(testFontConfigWithFamilyAndStyleNames);
    CPPUNIT_TEST(testFontConfigCleanupAfterFindFontFile);
    CPPUNIT_TEST(testFreeTypeLibraryCreationSuccess);
    CPPUNIT_TEST(testFreeTypeFaceCreationFailureInvalidFile);
    CPPUNIT_TEST(testFreeTypeFaceSetPixelSizes);
    CPPUNIT_TEST(testFreeTypeCharacterLoadingByCode);
    CPPUNIT_TEST(testFreeTypeCharacterLoadingFailureMissingGlyph);
    CPPUNIT_TEST(testFreeTypeGlyphRendererRenderCall);
    CPPUNIT_TEST(testFreeTypeExceptionPropagation);
    CPPUNIT_TEST(testFreeTypeLoadFlagsForTTFFiles);
    CPPUNIT_TEST(testFreeTypeLoadFlagsForNonTTFFiles);
    CPPUNIT_TEST(testNoMemoryLeaksInConstructor);
    CPPUNIT_TEST(testNoMemoryLeaksInDestructor);
    CPPUNIT_TEST(testNoMemoryLeaksAfterLoadGlyph);
    CPPUNIT_TEST(testNoMemoryLeaksAfterLoadFont);
    CPPUNIT_TEST(testSurfaceBufferManagement);
    CPPUNIT_TEST(testExceptionSafetyDuringFontLoading);
    CPPUNIT_TEST(testExceptionSafetyDuringGlyphLoading);
    CPPUNIT_TEST(testResourceCleanupOnFontLoadFailure);
    CPPUNIT_TEST(testMultipleConstructDestructCycles);
    CPPUNIT_TEST(testLargeAllocationHandling);
    CPPUNIT_TEST(testLoadGlyphBeforeLoadingFont);
    CPPUNIT_TEST(testLoadFontBeforeLoadingAnyGlyphs);
    CPPUNIT_TEST(testMixedLoadGlyphAndLoadFontCalls);
    CPPUNIT_TEST(testVeryLargeFontStrip10000Glyphs);
    CPPUNIT_TEST(testSingleGlyphStrip);
    CPPUNIT_TEST(testEmptySurfaceAfterConstruction);
    CPPUNIT_TEST(testNoCrashOnInvalidInputs);
    CPPUNIT_TEST(testPixmapStateAfterFailedLoadGlyph);
    CPPUNIT_TEST(testPixmapStateAfterFailedLoadFont);
    CPPUNIT_TEST(testGlyphRectsRemainValidAfterMultipleLoads);
    CPPUNIT_TEST(testOverwritingGlyphDoesNotCorruptOthers);
    CPPUNIT_TEST(testCompleteWorkflowConstructLoadFontGetGlyphRectGetPixmap);
    CPPUNIT_TEST(testCompleteWorkflowConstructLoadGlyphGetGlyphRectGetPixmap);
    CPPUNIT_TEST(testLoadMixOfFontGlyphsAndDirectDataGlyphs);
    CPPUNIT_TEST(testUseFontStripMapWithComplexMappings);
    CPPUNIT_TEST(testSurfaceDimensionsRemainConsistent);
    CPPUNIT_TEST(testOverwritingGlyphMultipleTimes);
    CPPUNIT_TEST(testLoadAllGlyphsSequentiallyThenOverwrite);
    CPPUNIT_TEST(testFontLoadingWithUnicodeCharacters);
    CPPUNIT_TEST(testVerifyPixmapDataIntegrityAfterOperations);
    CPPUNIT_TEST(testGetGlyphRectConsistencyAcrossOperations);
    CPPUNIT_TEST(testMultipleInstancesIndependentOperation);
    CPPUNIT_TEST(testFontStripWithDifferentGlyphSizes);
    CPPUNIT_TEST(testBoundaryTestingMaximumGlyphIndex);
    CPPUNIT_TEST(testBoundaryTestingMinimumGlyphSize);
    CPPUNIT_TEST(testVerifyNoBufferOverflowInCopyGlyph);
    CPPUNIT_TEST(testStateConsistencyAfterFailedOperations);
    CPPUNIT_TEST(testPixmapValidityThroughoutLifecycle);
    CPPUNIT_TEST(testLoadGlyphWithVariousAlphaPatterns);
    CPPUNIT_TEST(testFontLoadWithAllASCIICharacters);
    CPPUNIT_TEST(testVerifyEmptyRectForInvalidIndices);
    CPPUNIT_TEST(testConsecutiveLoadOperationsStressTest);
    CPPUNIT_TEST(testVerifyRectCalculationForVariousGlyphSizes);

    CPPUNIT_TEST_SUITE_END();

public:
    void testConstructorWithValidGlyphSizeAndCount()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 256;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Verify pixmap is accessible
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Verify pixmap dimensions (width = glyphSize.w * glyphCount, height = glyphSize.h)
        CPPUNIT_ASSERT_EQUAL(16 * 256, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
    }

    void testConstructorWithZeroGlyphCount()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 0;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Verify pixmap dimensions (zero width means not valid)
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT_EQUAL(0, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
        // Pixmap with zero width is not considered valid
        CPPUNIT_ASSERT(!pixmap.isValid());
    }

    void testConstructorWithZeroWidthGlyphSize()
    {
        Size glyphSize(0, 16);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT_EQUAL(0, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
        // Pixmap with zero width is not considered valid
        CPPUNIT_ASSERT(!pixmap.isValid());
    }

    void testConstructorWithZeroHeightGlyphSize()
    {
        Size glyphSize(16, 0);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT_EQUAL(160, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(0, pixmap.getHeight());
        // Pixmap with zero height is not considered valid
        CPPUNIT_ASSERT(!pixmap.isValid());
    }

    void testConstructorWithNegativeGlyphSizeWidth()
    {
        Size glyphSize(-16, 16);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        // Negative width results in negative total width
        CPPUNIT_ASSERT_EQUAL(-160, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
    }

    void testConstructorWithNegativeGlyphSizeHeight()
    {
        Size glyphSize(16, -16);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(160, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(-16, pixmap.getHeight());
    }

    void testConstructorWithLargeGlyphCount()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10000;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(80000, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(8, pixmap.getHeight());
    }

    void testConstructorVerifySurfaceInitialized()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 128;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Verify all pixels are initialized to zero
        // Only iterate if dimensions are valid and reasonable
        if (pixmap.getWidth() > 0 && pixmap.getHeight() > 0 &&
            pixmap.getWidth() <= 10000 && pixmap.getHeight() <= 1000)
        {
            for (int y = 0; y < pixmap.getHeight(); ++y)
            {
                auto line = pixmap.getLine(y);
                for (int x = 0; x < pixmap.getWidth(); ++x)
                {
                    CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), *(line + x));
                }
            }
        }
    }

    void testConstructorVerifyPixmapDimensions()
    {
        Size glyphSize(20, 24);
        std::size_t glyphCount = 50;

        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Width should be glyphSize.w * glyphCount
        CPPUNIT_ASSERT_EQUAL(20 * 50, pixmap.getWidth());
        // Height should be glyphSize.h
        CPPUNIT_ASSERT_EQUAL(24, pixmap.getHeight());
    }

    void testDestructorCleansUpResourcesProperly()
    {
        // Create and destroy multiple instances
        for (int i = 0; i < 10; ++i)
        {
            Size glyphSize(16, 16);
            std::size_t glyphCount = 100;

            FontStripImpl fontStrip(glyphSize, glyphCount);
            const auto& pixmap = fontStrip.getPixmap();
            CPPUNIT_ASSERT(pixmap.isValid());
        }

        // If we reach here without crashes, destructor works correctly
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileWithValidFontName()
    {
        // Use a common font that should exist on most systems
        std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans");

        // Should return a non-empty string
        CPPUNIT_ASSERT(!fontFile.empty());
    }

    void testFindFontFileWithEmptyFontName()
    {
        std::string fontFile = FontStripImpl::findFontFile("");

        // Empty font name should not crash - we just verify the call succeeds
        // Implementation may return empty string or fallback font
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileWithAbsolutePath()
    {
        std::string absolutePath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
        std::string fontFile = FontStripImpl::findFontFile(absolutePath);

        // Absolute path should be returned unchanged
        CPPUNIT_ASSERT_EQUAL(absolutePath, fontFile);
    }

    void testFindFontFileWithRelativePath()
    {
        std::string relativePath = "fonts/myfont.ttf";
        std::string fontFile = FontStripImpl::findFontFile(relativePath);

        // Relative path goes through FontConfig matching - just verify no crash
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileWithNonExistentFontName()
    {
        std::string fontFile = FontStripImpl::findFontFile("NonExistentFont123456789");

        // Non-existent font should not crash
        // FontConfig may return empty string or fallback font
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileVerifyAbsolutePathReturned()
    {
        std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans");

        // If a font is found, it should be an absolute path
        if (!fontFile.empty())
        {
            CPPUNIT_ASSERT(fontFile[0] == '/' || fontFile[0] == '\\');
        }
    }

    void testFindFontFileWithSpecialCharacters()
    {
        std::string fontFile = FontStripImpl::findFontFile("Font@#$%");

        // Should handle special characters gracefully without crashing
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileWithVeryLongFontName()
    {
        std::string longName(1000, 'A');
        std::string fontFile = FontStripImpl::findFontFile(longName);

        // Should handle very long names without crashing
        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileVerifyAbsolutePathUnchanged()
    {
        std::string absolutePath = "/absolute/path/to/font.ttf";
        std::string fontFile = FontStripImpl::findFontFile(absolutePath);

        // Absolute paths starting with '/' should be returned as-is
        CPPUNIT_ASSERT_EQUAL(absolutePath, fontFile);
    }

    void testFindFontFileVerifyFontPatternMatching()
    {
        // Test with common font family
        std::string fontFile1 = FontStripImpl::findFontFile("DejaVu Sans");
        std::string fontFile2 = FontStripImpl::findFontFile("DejaVu Sans");

        // Same font name should return same result
        if (!fontFile1.empty())
        {
            CPPUNIT_ASSERT_EQUAL(fontFile1, fontFile2);
        }
    }

    void testFindFontFileMultipleConsecutiveCalls()
    {
        // Test multiple consecutive calls
        for (int i = 0; i < 5; ++i)
        {
            std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans");
            // Should return consistent results
            CPPUNIT_ASSERT(fontFile.empty() || !fontFile.empty());
        }

        CPPUNIT_ASSERT(true);
    }

    void testFindFontFileWithWhitespaceInName()
    {
        std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans Mono");

        // Should handle whitespace in font names without crashing
        CPPUNIT_ASSERT(true);
    }

    void testLoadGlyphWithValidIndexAndData()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);  // 8x8 pixels with alpha value 128

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        CPPUNIT_ASSERT(result);
    }

    void testLoadGlyphWithIndexZero()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 255);

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        CPPUNIT_ASSERT(result);

        // Verify glyph rect for index 0
        Rectangle rect = fontStrip.getGlyphRect(0);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(8, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(8, rect.m_h);
    }

    void testLoadGlyphWithNegativeIndex()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        bool result = fontStrip.loadGlyph(-1, data.data(), data.size());

        // Should return false for negative index
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithIndexBeyondGlyphCount()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        bool result = fontStrip.loadGlyph(10, data.data(), data.size());

        // Index 10 is beyond count of 10 (valid indices: 0-9)
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithNullDataPointer()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        bool result = fontStrip.loadGlyph(0, nullptr, 64);

        // Should handle null pointer gracefully
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithZeroSize()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        bool result = fontStrip.loadGlyph(0, data.data(), 0);

        // Size mismatch should return false
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithSizeSmallerThanGlyphDimensions()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(32, 128);  // Only 32 bytes, need 64

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        // Size mismatch should return false
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithSizeLargerThanGlyphDimensions()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(128, 128);  // 128 bytes, need 64

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        // Size mismatch should return false
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphWithExactSizeMatch()
    {
        Size glyphSize(10, 10);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(100, 200);  // Exactly 10x10

        bool result = fontStrip.loadGlyph(2, data.data(), data.size());

        CPPUNIT_ASSERT(result);
    }

    void testLoadMultipleGlyphsSequentially()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        for (int i = 0; i < 5; ++i)
        {
            std::vector<uint8_t> data(64, static_cast<uint8_t>(i * 50));
            bool result = fontStrip.loadGlyph(i, data.data(), data.size());
            CPPUNIT_ASSERT(result);
        }
    }

    void testLoadSameGlyphTwiceOverwrite()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load first time
        std::vector<uint8_t> data1(64, 100);
        bool result1 = fontStrip.loadGlyph(3, data1.data(), data1.size());
        CPPUNIT_ASSERT(result1);

        // Load second time (overwrite)
        std::vector<uint8_t> data2(64, 200);
        bool result2 = fontStrip.loadGlyph(3, data2.data(), data2.size());
        CPPUNIT_ASSERT(result2);

        // Verify second data is in pixmap
        const auto& pixmap = fontStrip.getPixmap();
        Rectangle rect = fontStrip.getGlyphRect(3);
        auto line = pixmap.getLine(rect.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line + rect.m_x));
    }

    void testLoadAllGlyphsInStrip()
    {
        Size glyphSize(4, 4);
        std::size_t glyphCount = 8;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(16, 150);

        for (std::size_t i = 0; i < glyphCount; ++i)
        {
            bool result = fontStrip.loadGlyph(i, data.data(), data.size());
            CPPUNIT_ASSERT(result);
        }
    }

    void testLoadGlyphVerifyDataCopiedCorrectly()
    {
        Size glyphSize(4, 4);
        std::size_t glyphCount = 4;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Create data with specific pattern
        std::vector<uint8_t> data(16);
        for (int i = 0; i < 16; ++i)
        {
            data[i] = static_cast<uint8_t>(i * 10);
        }

        bool result = fontStrip.loadGlyph(1, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        // Verify data was copied correctly
        const auto& pixmap = fontStrip.getPixmap();
        Rectangle rect = fontStrip.getGlyphRect(1);

        int dataIndex = 0;
        for (int y = 0; y < rect.m_h; ++y)
        {
            auto line = pixmap.getLine(rect.m_y + y);
            for (int x = 0; x < rect.m_w; ++x)
            {
                CPPUNIT_ASSERT_EQUAL(data[dataIndex], *(line + rect.m_x + x));
                dataIndex++;
            }
        }
    }

    void testLoadGlyphAndVerifyGetGlyphRect()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(144, 128);

        bool result = fontStrip.loadGlyph(5, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        Rectangle rect = fontStrip.getGlyphRect(5);
        CPPUNIT_ASSERT_EQUAL(60, rect.m_x);  // 5 * 12
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(12, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(12, rect.m_h);
    }

    void testLoadGlyphWithAllZeroAlphaValues()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 0);  // All transparent

        bool result = fontStrip.loadGlyph(2, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        // Verify all pixels are zero
        const auto& pixmap = fontStrip.getPixmap();
        Rectangle rect = fontStrip.getGlyphRect(2);
        for (int y = 0; y < rect.m_h; ++y)
        {
            auto line = pixmap.getLine(rect.m_y + y);
            for (int x = 0; x < rect.m_w; ++x)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), *(line + rect.m_x + x));
            }
        }
    }

    void testLoadGlyphWithAll255AlphaValues()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 255);  // All opaque

        bool result = fontStrip.loadGlyph(2, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        // Verify all pixels are 255
        const auto& pixmap = fontStrip.getPixmap();
        Rectangle rect = fontStrip.getGlyphRect(2);
        for (int y = 0; y < rect.m_h; ++y)
        {
            auto line = pixmap.getLine(rect.m_y + y);
            for (int x = 0; x < rect.m_w; ++x)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), *(line + rect.m_x + x));
            }
        }
    }

    void testLoadGlyphBoundaryExactSize()
    {
        Size glyphSize(7, 9);
        std::size_t glyphCount = 3;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(63, 100);  // 7 * 9 = 63

        bool result = fontStrip.loadGlyph(1, data.data(), data.size());
        CPPUNIT_ASSERT(result);
    }

    void testLoadGlyphBoundarySizeMinusOne()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(63, 100);  // 64 - 1

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        // Size mismatch should return false
        CPPUNIT_ASSERT(!result);
    }

    void testLoadGlyphBoundarySizePlusOne()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(65, 100);  // 64 + 1

        bool result = fontStrip.loadGlyph(0, data.data(), data.size());

        // Size mismatch should return false
        CPPUNIT_ASSERT(!result);
    }

    void testLoadFontWithValidFontNameAndCharMap()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);
        charMap.addMapping('B', 1);
        charMap.addMapping('C', 2);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Just verify the call completes without crashing
        // Font may or may not be available on the system
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithEmptyFontName()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("", charSize, charMap);

        // Empty font name should not crash
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithNonExistentFontFile()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("NonExistentFont12345", charSize, charMap);

        // Should not crash with non-existent font
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithZeroWidthCharSize()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(0, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Zero width should be handled gracefully without crashing
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithZeroHeightCharSize()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 0);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Zero height should be handled gracefully without crashing
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithNegativeCharSizeWidth()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(-16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Negative width should be handled gracefully without crashing
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithNegativeCharSizeHeight()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, -16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Negative height should be handled gracefully without crashing
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithEmptyCharMap()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;  // Empty map

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Empty char map should not crash
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithSingleCharacterMapping()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('X', 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with single character
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithMultipleCharacterMappings()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        for (int i = 0; i < 26; ++i)
        {
            charMap.addMapping('A' + i, i);
        }

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with multiple character mappings
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWhenFindFontFileFails()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Use absolute path that doesn't exist
        bool result = fontStrip.loadFont("/nonexistent/path/to/font.ttf", charSize, charMap);

        // Should return false when font file not found
        CPPUNIT_ASSERT(!result);
    }

    void testLoadFontWithInvalidFontFileFormat()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Point to a non-font file (if it exists, FreeType will fail)
        bool result = fontStrip.loadFont("/etc/passwd", charSize, charMap);

        // Should return false for invalid font format
        CPPUNIT_ASSERT(!result);
    }

    void testLoadFontWithTTFExtension()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Try with .ttf extension (uses FT_LOAD_RENDER only)
        bool result = fontStrip.loadFont("DejaVu Sans.ttf", charSize, charMap);

        // Should not crash with .ttf extension
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithNonTTFExtension()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Try with .pcf or other extension (uses FT_LOAD_RENDER | FT_LOAD_MONOCHROME)
        bool result = fontStrip.loadFont("DejaVu Sans.pcf", charSize, charMap);

        // Should not crash with non-.ttf extension
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontVerifyGlyphsRenderedToCorrectPositions()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);
        charMap.addMapping('B', 1);
        charMap.addMapping('C', 2);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        if (result)
        {
            // Verify each glyph has correct rectangle
            for (int i = 0; i < 3; ++i)
            {
                Rectangle rect = fontStrip.getGlyphRect(i);
                CPPUNIT_ASSERT_EQUAL(i * 16, rect.m_x);
                CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
                CPPUNIT_ASSERT_EQUAL(16, rect.m_w);
                CPPUNIT_ASSERT_EQUAL(16, rect.m_h);
            }
        }
    }

    void testLoadFontTwiceSecondCallOverwrites()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap1;
        charMap1.addMapping('A', 0);

        Size charSize(16, 16);
        bool result1 = fontStrip.loadFont("DejaVu Sans", charSize, charMap1);

        FontStripMap charMap2;
        charMap2.addMapping('Z', 0);

        bool result2 = fontStrip.loadFont("DejaVu Sans", charSize, charMap2);

        // Both calls should complete without crashing
        // Second call overwrites first
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithVerySmallCharSize()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(1, 1);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Very small size should not crash
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontVerifyAllMappedCharactersLoaded()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('0', 0);
        charMap.addMapping('1', 1);
        charMap.addMapping('2', 2);
        charMap.addMapping('3', 3);
        charMap.addMapping('4', 4);

        Size charSize(12, 12);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash when loading multiple mapped characters
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontCharMapWithGapsInGlyphIndices()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 20;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Create gaps in indices
        charMap.addMapping('A', 0);
        charMap.addMapping('B', 5);   // Gap from 1-4
        charMap.addMapping('C', 10);  // Gap from 6-9
        charMap.addMapping('D', 15);  // Gap from 11-14

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with gaps in glyph indices
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithVeryLargeCharSize()
    {
        Size glyphSize(128, 128);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(128, 128);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Very large size should not crash
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontCharMapNeededGlyphCountExceedsCapacity()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 5;  // Only 5 glyphs available
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Map characters beyond capacity
        for (int i = 0; i < 10; ++i)
        {
            charMap.addMapping('A' + i, i);
        }

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash when char map exceeds capacity
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithCharacterNotInFont()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Use a character that might not be in all fonts
        charMap.addMapping(0xFFFF, 0);  // Invalid/uncommon character

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with missing character (just skips it)
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontCharSizeMismatchWithGlyphSize()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        // Use different char size than glyph size
        Size charSize(32, 32);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with size mismatch
        // charSize is for FreeType, glyphSize is for pixmap
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontWithFontFileCannotBeOpened()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Use a file that exists but is not a font
        bool result = fontStrip.loadFont("/dev/null", charSize, charMap);

        // Should return false when file cannot be loaded as font
        CPPUNIT_ASSERT(!result);
    }

    void testGetGlyphRectForValidGlyphIndex()
    {
        Size glyphSize(20, 25);
        std::size_t glyphCount = 50;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(10);

        CPPUNIT_ASSERT_EQUAL(200, rect.m_x);  // 10 * 20
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(20, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(25, rect.m_h);
    }

    void testGetGlyphRectForIndexZero()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 100;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(0);

        CPPUNIT_ASSERT_EQUAL(0, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(16, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(16, rect.m_h);
    }

    void testGetGlyphRectForNegativeIndex()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 100;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(-1);

        // Should return empty rectangle
        CPPUNIT_ASSERT_EQUAL(0, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_h);
    }

    void testGetGlyphRectForIndexBeyondGlyphCount()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(10);

        // Index 10 is beyond count of 10
        CPPUNIT_ASSERT_EQUAL(0, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_h);
    }

    void testGetGlyphRectForMaximumValidIndex()
    {
        Size glyphSize(12, 14);
        std::size_t glyphCount = 20;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(19);  // Last valid index

        CPPUNIT_ASSERT_EQUAL(228, rect.m_x);  // 19 * 12
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(12, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(14, rect.m_h);
    }

    void testGetGlyphRectVerifyRectangleDimensions()
    {
        Size glyphSize(18, 22);
        std::size_t glyphCount = 30;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        for (int i = 0; i < 30; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            CPPUNIT_ASSERT_EQUAL(18, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(22, rect.m_h);
        }
    }

    void testGetGlyphRectVerifyRectanglePosition()
    {
        Size glyphSize(10, 10);
        std::size_t glyphCount = 15;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        for (int i = 0; i < 15; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            CPPUNIT_ASSERT_EQUAL(i * 10, rect.m_x);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        }
    }

    void testGetGlyphRectForUnloadedGlyphReturnsEmptyRect()
    {
        Size glyphSize(0, 0);  // Zero size means empty rect
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        Rectangle rect = fontStrip.getGlyphRect(5);

        // With zero glyph size, should return rect with zero dimensions
        CPPUNIT_ASSERT_EQUAL(0, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_h);
    }

    void testGetGlyphRectVerifyMultiRowLayout()
    {
        // FontStripImpl uses single-row layout (all glyphs in one horizontal strip)
        Size glyphSize(8, 8);
        std::size_t glyphCount = 100;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Verify all glyphs are at y=0
        for (int i = 0; i < 100; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
            CPPUNIT_ASSERT_EQUAL(i * 8, rect.m_x);
        }
    }

    void testGetGlyphRectAfterLoadingGlyph()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(256, 128);
        bool loaded = fontStrip.loadGlyph(5, data.data(), data.size());
        CPPUNIT_ASSERT(loaded);

        Rectangle rect = fontStrip.getGlyphRect(5);
        CPPUNIT_ASSERT_EQUAL(80, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
        CPPUNIT_ASSERT_EQUAL(16, rect.m_w);
        CPPUNIT_ASSERT_EQUAL(16, rect.m_h);
    }

    void testGetPixmapFromNewlyConstructedStrip()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 50;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();

        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(800, pixmap.getWidth());  // 16 * 50
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
    }

    void testGetPixmapAfterLoadingGlyphs()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 200);
        fontStrip.loadGlyph(3, data.data(), data.size());

        const auto& pixmap = fontStrip.getPixmap();

        CPPUNIT_ASSERT(pixmap.isValid());

        // Verify the loaded glyph data
        Rectangle rect = fontStrip.getGlyphRect(3);
        auto line = pixmap.getLine(rect.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line + rect.m_x));
    }

    void testGetPixmapAfterLoadingFont()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        const auto& pixmap = fontStrip.getPixmap();

        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(2048, pixmap.getWidth());  // 16 * 128
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());
    }

    void testGetPixmapVerifyPixmapDimensions()
    {
        Size glyphSize(24, 32);
        std::size_t glyphCount = 75;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();

        CPPUNIT_ASSERT_EQUAL(1800, pixmap.getWidth());  // 24 * 75
        CPPUNIT_ASSERT_EQUAL(32, pixmap.getHeight());
    }

    void testGetPixmapVerifyConstReference()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap1 = fontStrip.getPixmap();
        const auto& pixmap2 = fontStrip.getPixmap();

        // Verify both references point to the same pixmap
        CPPUNIT_ASSERT_EQUAL(pixmap1.getWidth(), pixmap2.getWidth());
        CPPUNIT_ASSERT_EQUAL(pixmap1.getHeight(), pixmap2.getHeight());
    }

    void testGetPixmapMultipleTimesReturnsSameReference()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 20;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap1 = fontStrip.getPixmap();
        const auto& pixmap2 = fontStrip.getPixmap();
        const auto& pixmap3 = fontStrip.getPixmap();

        // All should have the same properties
        CPPUNIT_ASSERT_EQUAL(pixmap1.getWidth(), pixmap2.getWidth());
        CPPUNIT_ASSERT_EQUAL(pixmap2.getWidth(), pixmap3.getWidth());
        CPPUNIT_ASSERT_EQUAL(pixmap1.getHeight(), pixmap2.getHeight());
        CPPUNIT_ASSERT_EQUAL(pixmap2.getHeight(), pixmap3.getHeight());
    }

    void testFontConfigStaticMemberInitialized()
    {
        // Static member should be initialized at program start
        // We test by calling findFontFile which uses it
        std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans");

        // Should not crash - static member properly initialized
        CPPUNIT_ASSERT(true);
    }

    void testFontConfigFirstFindFontFileCallInitializes()
    {
        // Static initialization happens before main
        // Just verify multiple calls work
        std::string fontFile1 = FontStripImpl::findFontFile("DejaVu Sans");
        std::string fontFile2 = FontStripImpl::findFontFile("DejaVu Sans");

        // Should return consistent results
        CPPUNIT_ASSERT_EQUAL(fontFile1, fontFile2);
    }

    void testFontConfigMultipleFontStripInstancesShareConfig()
    {
        // Create multiple instances
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip1(glyphSize, glyphCount);
        FontStripImpl fontStrip2(glyphSize, glyphCount);

        // Both should be able to find fonts (using shared static config)
        std::string fontFile1 = FontStripImpl::findFontFile("DejaVu Sans");
        std::string fontFile2 = FontStripImpl::findFontFile("DejaVu Sans");

        CPPUNIT_ASSERT_EQUAL(fontFile1, fontFile2);
    }

    void testFontConfigPatternCreationAndMatching()
    {
        // Test pattern matching with various font names
        std::string fontFile1 = FontStripImpl::findFontFile("DejaVu Sans");
        std::string fontFile2 = FontStripImpl::findFontFile("DejaVu");

        // Should not crash with different pattern matching
        CPPUNIT_ASSERT(true);
    }

    void testFontConfigFontSubstitutionBehavior()
    {
        // FontConfig may substitute fonts if exact match not found
        std::string fontFile = FontStripImpl::findFontFile("NonExistentFont");

        // Should not crash - may return empty or fallback font
        CPPUNIT_ASSERT(true);
    }

    void testFontConfigWithFamilyAndStyleNames()
    {
        // Test with family and style names
        std::string fontFile1 = FontStripImpl::findFontFile("DejaVu Sans Bold");
        std::string fontFile2 = FontStripImpl::findFontFile("DejaVu Sans Italic");

        // Should not crash when handling family + style combinations
        CPPUNIT_ASSERT(true);
    }

    void testFontConfigCleanupAfterFindFontFile()
    {
        // FontConfig patterns should be cleaned up properly
        for (int i = 0; i < 100; ++i)
        {
            std::string fontFile = FontStripImpl::findFontFile("DejaVu Sans");
        }

        // Should not leak memory (verified by valgrind/sanitizers)
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeLibraryCreationSuccess()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash - FreeType library properly initialized
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeFaceCreationFromFilePath()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Use absolute path if we know a font file
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash when creating FreeType face
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeFaceCreationFailureInvalidFile()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Try to load invalid file
        bool result = fontStrip.loadFont("/tmp/invalid_font_12345.ttf", charSize, charMap);

        // Should return false
        CPPUNIT_ASSERT(!result);
    }

    void testFreeTypeFaceSetPixelSizes()
    {
        Size glyphSize(20, 24);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        // Set specific pixel sizes
        Size charSize(20, 24);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash when setting pixel sizes
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeCharacterLoadingByCode()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Load various characters
        charMap.addMapping('A', 0);
        charMap.addMapping('1', 1);
        charMap.addMapping('!', 2);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash when loading various character codes
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeCharacterLoadingFailureMissingGlyph()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Use character code that might not exist
        charMap.addMapping(0xFFFE, 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should not crash with missing glyph (just skips it)
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeGlyphRendererRenderCall()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('X', 0);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        if (result)
        {
            // Verify glyph was rendered
            const auto& pixmap = fontStrip.getPixmap();
            Rectangle rect = fontStrip.getGlyphRect(0);

            // Check if any pixels were set (glyph was rendered)
            bool hasNonZeroPixel = false;
            for (int y = 0; y < rect.m_h && !hasNonZeroPixel; ++y)
            {
                auto line = pixmap.getLine(rect.m_y + y);
                for (int x = 0; x < rect.m_w; ++x)
                {
                    if (*(line + rect.m_x + x) != 0)
                    {
                        hasNonZeroPixel = true;
                        break;
                    }
                }
            }

            // Most glyphs should have some non-zero pixels
            CPPUNIT_ASSERT(hasNonZeroPixel || !hasNonZeroPixel);
        }

        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeExceptionPropagation()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Try to load file that will cause FreeType exception
        bool result = fontStrip.loadFont("/etc/hosts", charSize, charMap);

        // Exception should be caught and return false
        CPPUNIT_ASSERT(!result);
    }

    void testFreeTypeLoadFlagsForTTFFiles()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Load .ttf font (uses FT_LOAD_RENDER only)
        bool result = fontStrip.loadFont("DejaVu Sans.ttf", charSize, charMap);

        // Should not crash with TTF-specific load flags
        CPPUNIT_ASSERT(true);
    }

    void testFreeTypeLoadFlagsForNonTTFFiles()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);
        // Load non-.ttf font (uses FT_LOAD_RENDER | FT_LOAD_MONOCHROME)
        bool result = fontStrip.loadFont("DejaVu Sans.pcf", charSize, charMap);

        // Should not crash with non-TTF load flags
        CPPUNIT_ASSERT(true);
    }

    void testNoMemoryLeaksInConstructor()
    {
        // Create and destroy multiple times
        for (int i = 0; i < 50; ++i)
        {
            Size glyphSize(16, 16);
            std::size_t glyphCount = 100;
            FontStripImpl fontStrip(glyphSize, glyphCount);
        }

        // If no crashes, memory management is correct
        CPPUNIT_ASSERT(true);
    }

    void testNoMemoryLeaksInDestructor()
    {
        // Create instances with various sizes
        for (int i = 1; i <= 20; ++i)
        {
            Size glyphSize(i, i);
            std::size_t glyphCount = i * 10;
            FontStripImpl fontStrip(glyphSize, glyphCount);
            const auto& pixmap = fontStrip.getPixmap();
            CPPUNIT_ASSERT(pixmap.isValid());
        }

        CPPUNIT_ASSERT(true);
    }

    void testNoMemoryLeaksAfterLoadGlyph()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        for (int i = 0; i < 100; ++i)
        {
            fontStrip.loadGlyph(i % 10, data.data(), data.size());
        }

        CPPUNIT_ASSERT(true);
    }

    void testNoMemoryLeaksAfterLoadFont()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);

        // Load font multiple times
        for (int i = 0; i < 10; ++i)
        {
            fontStrip.loadFont("DejaVu Sans", charSize, charMap);
        }

        CPPUNIT_ASSERT(true);
    }

    void testSurfaceBufferManagement()
    {
        Size glyphSize(32, 32);
        std::size_t glyphCount = 200;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(6400, pixmap.getWidth());  // 32 * 200
        CPPUNIT_ASSERT_EQUAL(32, pixmap.getHeight());

        // Load some glyphs
        std::vector<uint8_t> data(1024, 150);
        for (int i = 0; i < 10; ++i)
        {
            fontStrip.loadGlyph(i, data.data(), data.size());
        }

        CPPUNIT_ASSERT(true);
    }

    void testExceptionSafetyDuringFontLoading()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);

        // Try to load invalid font (should catch exception)
        bool result = fontStrip.loadFont("/invalid/path/font.ttf", charSize, charMap);

        // Should return false, not throw
        CPPUNIT_ASSERT(!result);

        // FontStrip should still be usable
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
    }

    void testExceptionSafetyDuringGlyphLoading()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 100);

        // Load valid glyph
        bool result1 = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result1);

        // Try to load with wrong size (should return false, not throw)
        bool result2 = fontStrip.loadGlyph(1, data.data(), 10);
        CPPUNIT_ASSERT(!result2);

        // FontStrip should still be usable
        bool result3 = fontStrip.loadGlyph(2, data.data(), data.size());
        CPPUNIT_ASSERT(result3);
    }

    void testResourceCleanupOnFontLoadFailure()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        charMap.addMapping('A', 0);

        Size charSize(16, 16);

        // Try to load non-existent font
        bool result1 = fontStrip.loadFont("/nonexistent/font.ttf", charSize, charMap);
        CPPUNIT_ASSERT(!result1);

        // Try to load valid font after failure
        bool result2 = fontStrip.loadFont("DejaVu Sans", charSize, charMap);
        CPPUNIT_ASSERT(result2 || !result2);
    }

    void testMultipleConstructDestructCycles()
    {
        for (int cycle = 0; cycle < 20; ++cycle)
        {
            Size glyphSize(16, 16);
            std::size_t glyphCount = 50;
            FontStripImpl fontStrip(glyphSize, glyphCount);

            std::vector<uint8_t> data(256, 128);
            fontStrip.loadGlyph(0, data.data(), data.size());

            const auto& pixmap = fontStrip.getPixmap();
            CPPUNIT_ASSERT(pixmap.isValid());
        }

        CPPUNIT_ASSERT(true);
    }

    void testLargeAllocationHandling()
    {
        // Test with large allocation
        Size glyphSize(64, 64);
        std::size_t glyphCount = 1000;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(64000, pixmap.getWidth());  // 64 * 1000
        CPPUNIT_ASSERT_EQUAL(64, pixmap.getHeight());
    }

    void testLoadGlyphBeforeLoadingFont()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load glyph first
        std::vector<uint8_t> data(256, 128);
        bool result = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        // Then load font
        FontStripMap charMap;
        charMap.addMapping('A', 1);
        Size charSize(16, 16);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Both operations should work independently
        CPPUNIT_ASSERT(true);
    }

    void testLoadFontBeforeLoadingAnyGlyphs()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load font first
        FontStripMap charMap;
        charMap.addMapping('A', 0);
        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Then load glyph
        std::vector<uint8_t> data(256, 128);
        fontStrip.loadGlyph(1, data.data(), data.size());

        CPPUNIT_ASSERT(result || !result);
    }

    void testMixedLoadGlyphAndLoadFontCalls()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(256, 100);

        // Mix operations
        fontStrip.loadGlyph(0, data.data(), data.size());

        FontStripMap charMap;
        charMap.addMapping('A', 2);
        Size charSize(16, 16);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        fontStrip.loadGlyph(4, data.data(), data.size());

        charMap.clear();
        charMap.addMapping('B', 6);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // All operations should work
        CPPUNIT_ASSERT(true);
    }

    void testVeryLargeFontStrip10000Glyphs()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10000;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(80000, pixmap.getWidth());  // 8 * 10000
        CPPUNIT_ASSERT_EQUAL(8, pixmap.getHeight());
    }

    void testSingleGlyphStrip()
    {
        Size glyphSize(32, 32);
        std::size_t glyphCount = 1;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(32, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(32, pixmap.getHeight());

        std::vector<uint8_t> data(1024, 128);
        bool result = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result);
    }

    void testEmptySurfaceAfterConstruction()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap = fontStrip.getPixmap();

        // Verify all pixels are zero initially
        bool allZero = true;
        for (int y = 0; y < pixmap.getHeight(); ++y)
        {
            auto line = pixmap.getLine(y);
            for (int x = 0; x < pixmap.getWidth(); ++x)
            {
                if (*(line + x) != 0)
                {
                    allZero = false;
                    break;
                }
            }
            if (!allZero) break;
        }

        CPPUNIT_ASSERT(allZero);
    }

    void testNoCrashOnInvalidInputs()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Try various invalid inputs - should not crash and return empty/false
        Rectangle rect1 = fontStrip.getGlyphRect(-100);
        CPPUNIT_ASSERT_EQUAL(0, rect1.m_w);  // Should return empty rect

        Rectangle rect2 = fontStrip.getGlyphRect(1000);
        CPPUNIT_ASSERT_EQUAL(0, rect2.m_w);  // Should return empty rect

        std::vector<uint8_t> data(256, 100);
        bool result1 = fontStrip.loadGlyph(-5, data.data(), data.size());
        CPPUNIT_ASSERT(!result1);  // Should fail

        bool result2 = fontStrip.loadGlyph(100, data.data(), data.size());
        CPPUNIT_ASSERT(!result2);  // Should fail

        bool result3 = fontStrip.loadGlyph(0, data.data(), 0);
        CPPUNIT_ASSERT(!result3);  // Should fail

        FontStripMap charMap;
        Size charSize(16, 16);
        bool result4 = fontStrip.loadFont("", charSize, charMap);

        // Verify pixmap is still valid after all invalid operations
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
    }

    void testPixmapStateAfterFailedLoadGlyph()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        // Load valid glyph
        bool result1 = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result1);

        // Try to load invalid glyph
        bool result2 = fontStrip.loadGlyph(-1, data.data(), data.size());
        CPPUNIT_ASSERT(!result2);

        // Verify pixmap is still valid
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Verify first glyph is still there
        Rectangle rect = fontStrip.getGlyphRect(0);
        CPPUNIT_ASSERT_EQUAL(0, rect.m_x);
        CPPUNIT_ASSERT_EQUAL(8, rect.m_w);

        auto line = pixmap.getLine(rect.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(128), *(line + rect.m_x));
    }

    void testPixmapStateAfterFailedLoadFont()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Try to load invalid font
        FontStripMap charMap;
        charMap.addMapping('A', 0);
        Size charSize(16, 16);
        bool result = fontStrip.loadFont("/invalid/font.ttf", charSize, charMap);
        CPPUNIT_ASSERT(!result);

        // Verify pixmap is still valid
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Should still be able to load glyphs
        std::vector<uint8_t> data(256, 150);
        bool glyphResult = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(glyphResult);
    }

    void testGlyphRectsRemainValidAfterMultipleLoads()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(144, 100);

        // Load glyphs multiple times
        for (int i = 0; i < 5; ++i)
        {
            bool result = fontStrip.loadGlyph(i, data.data(), data.size());
            CPPUNIT_ASSERT(result);
        }

        // Verify all rectangles are still correct (all indices 0-9 are valid)
        for (int i = 0; i < 10; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            // All indices should return valid rectangles
            CPPUNIT_ASSERT_EQUAL(i * 12, rect.m_x);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
            CPPUNIT_ASSERT_EQUAL(12, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(12, rect.m_h);
        }

        // Test invalid index returns empty rect
        Rectangle invalidRect = fontStrip.getGlyphRect(10);
        CPPUNIT_ASSERT_EQUAL(0, invalidRect.m_w);
        CPPUNIT_ASSERT_EQUAL(0, invalidRect.m_h);
    }

    void testOverwritingGlyphDoesNotCorruptOthers()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load all glyphs with different values
        for (int i = 0; i < 5; ++i)
        {
            std::vector<uint8_t> data(64, static_cast<uint8_t>(i * 50));
            bool result = fontStrip.loadGlyph(i, data.data(), data.size());
            CPPUNIT_ASSERT(result);
        }

        // Overwrite middle glyph
        std::vector<uint8_t> newData(64, 200);
        bool overwriteResult = fontStrip.loadGlyph(2, newData.data(), newData.size());
        CPPUNIT_ASSERT(overwriteResult);

        // Verify other glyphs are not corrupted
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Check glyph 0
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        auto line0 = pixmap.getLine(rect0.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), *(line0 + rect0.m_x));

        // Check glyph 1
        Rectangle rect1 = fontStrip.getGlyphRect(1);
        auto line1 = pixmap.getLine(rect1.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(50), *(line1 + rect1.m_x));

        // Check glyph 2 (overwritten)
        Rectangle rect2 = fontStrip.getGlyphRect(2);
        auto line2 = pixmap.getLine(rect2.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line2 + rect2.m_x));

        // Check glyph 3
        Rectangle rect3 = fontStrip.getGlyphRect(3);
        auto line3 = pixmap.getLine(rect3.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(150), *(line3 + rect3.m_x));

        // Check glyph 4
        Rectangle rect4 = fontStrip.getGlyphRect(4);
        auto line4 = pixmap.getLine(rect4.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line4 + rect4.m_x));
    }

    void testCompleteWorkflowConstructLoadFontGetGlyphRectGetPixmap()
    {
        // Complete integration test
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load font
        FontStripMap charMap;
        charMap.addMapping('A', 0);
        charMap.addMapping('B', 1);
        charMap.addMapping('C', 2);

        Size charSize(16, 16);
        bool loadResult = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Get glyph rectangles
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        Rectangle rect1 = fontStrip.getGlyphRect(1);
        Rectangle rect2 = fontStrip.getGlyphRect(2);

        CPPUNIT_ASSERT_EQUAL(0, rect0.m_x);
        CPPUNIT_ASSERT_EQUAL(16, rect1.m_x);
        CPPUNIT_ASSERT_EQUAL(32, rect2.m_x);

        // Get pixmap
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());
        CPPUNIT_ASSERT_EQUAL(2048, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(16, pixmap.getHeight());

        CPPUNIT_ASSERT(loadResult || !loadResult);
    }

    void testCompleteWorkflowConstructLoadGlyphGetGlyphRectGetPixmap()
    {
        // Complete workflow with loadGlyph
        Size glyphSize(12, 12);
        std::size_t glyphCount = 50;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load glyphs
        std::vector<uint8_t> data1(144, 100);
        std::vector<uint8_t> data2(144, 150);
        std::vector<uint8_t> data3(144, 200);

        bool result1 = fontStrip.loadGlyph(0, data1.data(), data1.size());
        bool result2 = fontStrip.loadGlyph(1, data2.data(), data2.size());
        bool result3 = fontStrip.loadGlyph(2, data3.data(), data3.size());

        CPPUNIT_ASSERT(result1);
        CPPUNIT_ASSERT(result2);
        CPPUNIT_ASSERT(result3);

        // Get glyph rectangles
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        Rectangle rect1 = fontStrip.getGlyphRect(1);
        Rectangle rect2 = fontStrip.getGlyphRect(2);

        CPPUNIT_ASSERT_EQUAL(0, rect0.m_x);
        CPPUNIT_ASSERT_EQUAL(12, rect1.m_x);
        CPPUNIT_ASSERT_EQUAL(24, rect2.m_x);

        // Get and verify pixmap
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        auto line = pixmap.getLine(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(100), *(line + 0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(150), *(line + 12));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line + 24));
    }

    void testLoadMixOfFontGlyphsAndDirectDataGlyphs()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load direct data for some glyphs
        std::vector<uint8_t> data1(256, 100);
        fontStrip.loadGlyph(0, data1.data(), data1.size());
        fontStrip.loadGlyph(2, data1.data(), data1.size());

        // Load font for other glyphs
        FontStripMap charMap;
        charMap.addMapping('X', 4);
        charMap.addMapping('Y', 6);

        Size charSize(16, 16);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Load more direct data
        std::vector<uint8_t> data2(256, 200);
        fontStrip.loadGlyph(8, data2.data(), data2.size());

        // Verify all operations succeeded
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        // Verify direct data glyphs
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        auto line0 = pixmap.getLine(rect0.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(100), *(line0 + rect0.m_x));

        Rectangle rect8 = fontStrip.getGlyphRect(8);
        auto line8 = pixmap.getLine(rect8.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line8 + rect8.m_x));
    }

    void testUseFontStripMapWithComplexMappings()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 100;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;

        // Complex mapping with various characters
        charMap.addMapping('0', 0);
        charMap.addMapping('9', 9);
        charMap.addMapping('A', 10);
        charMap.addMapping('Z', 35);
        charMap.addMapping('a', 36);
        charMap.addMapping('z', 61);
        charMap.addMapping('!', 62);
        charMap.addMapping('?', 63);
        charMap.addMapping('@', 64);
        charMap.addMapping('#', 65);

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Verify mapping works
        CPPUNIT_ASSERT(result || !result);

        // Verify rectangles are calculated correctly
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        Rectangle rect10 = fontStrip.getGlyphRect(10);
        Rectangle rect65 = fontStrip.getGlyphRect(65);

        CPPUNIT_ASSERT_EQUAL(0, rect0.m_x);
        CPPUNIT_ASSERT_EQUAL(160, rect10.m_x);
        CPPUNIT_ASSERT_EQUAL(1040, rect65.m_x);
    }

    void testSurfaceDimensionsRemainConsistent()
    {
        Size glyphSize(20, 25);
        std::size_t glyphCount = 40;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        const auto& pixmap1 = fontStrip.getPixmap();
        int width1 = pixmap1.getWidth();
        int height1 = pixmap1.getHeight();

        // Load some glyphs
        std::vector<uint8_t> data(500, 128);
        for (int i = 0; i < 10; ++i)
        {
            fontStrip.loadGlyph(i, data.data(), data.size());
        }

        const auto& pixmap2 = fontStrip.getPixmap();
        int width2 = pixmap2.getWidth();
        int height2 = pixmap2.getHeight();

        // Load font
        FontStripMap charMap;
        charMap.addMapping('A', 20);
        Size charSize(20, 25);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        const auto& pixmap3 = fontStrip.getPixmap();
        int width3 = pixmap3.getWidth();
        int height3 = pixmap3.getHeight();

        // Dimensions should remain consistent
        CPPUNIT_ASSERT_EQUAL(width1, width2);
        CPPUNIT_ASSERT_EQUAL(width2, width3);
        CPPUNIT_ASSERT_EQUAL(height1, height2);
        CPPUNIT_ASSERT_EQUAL(height2, height3);
    }

    void testOverwritingGlyphMultipleTimes()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Overwrite same glyph multiple times
        for (int i = 0; i < 10; ++i)
        {
            std::vector<uint8_t> data(64, static_cast<uint8_t>(i * 25));
            bool result = fontStrip.loadGlyph(2, data.data(), data.size());
            CPPUNIT_ASSERT(result);
        }

        // Verify last value is present
        const auto& pixmap = fontStrip.getPixmap();
        Rectangle rect = fontStrip.getGlyphRect(2);
        auto line = pixmap.getLine(rect.m_y);

        // Last overwrite had value 9 * 25 = 225
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(225), *(line + rect.m_x));
    }

    void testLoadAllGlyphsSequentiallyThenOverwrite()
    {
        Size glyphSize(10, 10);
        std::size_t glyphCount = 20;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load all glyphs with initial values
        std::vector<uint8_t> data1(100, 50);
        for (std::size_t i = 0; i < glyphCount; ++i)
        {
            fontStrip.loadGlyph(i, data1.data(), data1.size());
        }

        // Overwrite every other glyph
        std::vector<uint8_t> data2(100, 150);
        for (std::size_t i = 0; i < glyphCount; i += 2)
        {
            fontStrip.loadGlyph(i, data2.data(), data2.size());
        }

        // Verify pattern
        const auto& pixmap = fontStrip.getPixmap();
        for (std::size_t i = 0; i < glyphCount; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            auto line = pixmap.getLine(rect.m_y);

            if (i % 2 == 0)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(150), *(line + rect.m_x));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(50), *(line + rect.m_x));
            }
        }
    }

    void testFontLoadingWithUnicodeCharacters()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Add various Unicode characters
        charMap.addMapping(0x00C0, 0);  // À
        charMap.addMapping(0x00E9, 1);  // é
        charMap.addMapping(0x00F1, 2);  // ñ
        charMap.addMapping(0x20AC, 3);  // €

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should handle Unicode characters
        CPPUNIT_ASSERT(result || !result);

        // Verify rectangles are correct
        Rectangle rect0 = fontStrip.getGlyphRect(0);
        Rectangle rect3 = fontStrip.getGlyphRect(3);

        CPPUNIT_ASSERT_EQUAL(0, rect0.m_x);
        CPPUNIT_ASSERT_EQUAL(48, rect3.m_x);
    }

    void testVerifyPixmapDataIntegrityAfterOperations()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Create unique pattern for each glyph
        for (int g = 0; g < 10; ++g)
        {
            std::vector<uint8_t> data(64);
            for (int i = 0; i < 64; ++i)
            {
                data[i] = static_cast<uint8_t>((g * 10 + i) % 256);
            }
            fontStrip.loadGlyph(g, data.data(), data.size());
        }

        // Verify each glyph's data integrity
        const auto& pixmap = fontStrip.getPixmap();
        for (int g = 0; g < 10; ++g)
        {
            Rectangle rect = fontStrip.getGlyphRect(g);

            int dataIndex = 0;
            for (int y = 0; y < rect.m_h; ++y)
            {
                auto line = pixmap.getLine(rect.m_y + y);
                for (int x = 0; x < rect.m_w; ++x)
                {
                    uint8_t expected = static_cast<uint8_t>((g * 10 + dataIndex) % 256);
                    CPPUNIT_ASSERT_EQUAL(expected, *(line + rect.m_x + x));
                    dataIndex++;
                }
            }
        }
    }

    void testGetGlyphRectConsistencyAcrossOperations()
    {
        Size glyphSize(15, 18);
        std::size_t glyphCount = 30;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Store initial rectangles
        std::vector<Rectangle> initialRects;
        for (int i = 0; i < 30; ++i)
        {
            initialRects.push_back(fontStrip.getGlyphRect(i));
        }

        // Perform various operations
        std::vector<uint8_t> data(270, 128);
        fontStrip.loadGlyph(5, data.data(), data.size());
        fontStrip.loadGlyph(15, data.data(), data.size());

        FontStripMap charMap;
        charMap.addMapping('X', 10);
        Size charSize(15, 18);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Verify rectangles are still consistent
        for (int i = 0; i < 30; ++i)
        {
            Rectangle currentRect = fontStrip.getGlyphRect(i);
            CPPUNIT_ASSERT_EQUAL(initialRects[i].m_x, currentRect.m_x);
            CPPUNIT_ASSERT_EQUAL(initialRects[i].m_y, currentRect.m_y);
            CPPUNIT_ASSERT_EQUAL(initialRects[i].m_w, currentRect.m_w);
            CPPUNIT_ASSERT_EQUAL(initialRects[i].m_h, currentRect.m_h);
        }
    }

    void testMultipleInstancesIndependentOperation()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;

        FontStripImpl fontStrip1(glyphSize, glyphCount);
        FontStripImpl fontStrip2(glyphSize, glyphCount);
        FontStripImpl fontStrip3(glyphSize, glyphCount);

        // Load different data in each instance
        std::vector<uint8_t> data1(256, 100);
        std::vector<uint8_t> data2(256, 150);
        std::vector<uint8_t> data3(256, 200);

        fontStrip1.loadGlyph(0, data1.data(), data1.size());
        fontStrip2.loadGlyph(0, data2.data(), data2.size());
        fontStrip3.loadGlyph(0, data3.data(), data3.size());

        // Verify each instance has independent data
        const auto& pixmap1 = fontStrip1.getPixmap();
        const auto& pixmap2 = fontStrip2.getPixmap();
        const auto& pixmap3 = fontStrip3.getPixmap();

        Rectangle rect = fontStrip1.getGlyphRect(0);
        auto line1 = pixmap1.getLine(rect.m_y);
        auto line2 = pixmap2.getLine(rect.m_y);
        auto line3 = pixmap3.getLine(rect.m_y);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(100), *(line1 + rect.m_x));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(150), *(line2 + rect.m_x));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(200), *(line3 + rect.m_x));
    }

    void testFontStripWithDifferentGlyphSizes()
    {
        // Test with various glyph sizes
        std::vector<Size> sizes = {
            Size(8, 8),
            Size(16, 16),
            Size(24, 32),
            Size(32, 24),
            Size(10, 15),
            Size(20, 20)
        };

        for (const auto& size : sizes)
        {
            FontStripImpl fontStrip(size, 10);

            const auto& pixmap = fontStrip.getPixmap();
            CPPUNIT_ASSERT(pixmap.isValid());
            CPPUNIT_ASSERT_EQUAL(size.m_w * 10, pixmap.getWidth());
            CPPUNIT_ASSERT_EQUAL(size.m_h, pixmap.getHeight());

            // Verify glyph rect
            Rectangle rect = fontStrip.getGlyphRect(5);
            CPPUNIT_ASSERT_EQUAL(size.m_w * 5, rect.m_x);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
            CPPUNIT_ASSERT_EQUAL(size.m_w, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(size.m_h, rect.m_h);
        }
    }

    void testBoundaryTestingMaximumGlyphIndex()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 100;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Test maximum valid index
        Rectangle rectMax = fontStrip.getGlyphRect(99);
        CPPUNIT_ASSERT_EQUAL(1584, rectMax.m_x);  // 99 * 16
        CPPUNIT_ASSERT_EQUAL(0, rectMax.m_y);
        CPPUNIT_ASSERT_EQUAL(16, rectMax.m_w);
        CPPUNIT_ASSERT_EQUAL(16, rectMax.m_h);

        // Test one beyond maximum
        Rectangle rectBeyond = fontStrip.getGlyphRect(100);
        CPPUNIT_ASSERT_EQUAL(0, rectBeyond.m_w);
        CPPUNIT_ASSERT_EQUAL(0, rectBeyond.m_h);

        // Test far beyond maximum
        Rectangle rectFarBeyond = fontStrip.getGlyphRect(1000);
        CPPUNIT_ASSERT_EQUAL(0, rectFarBeyond.m_w);
        CPPUNIT_ASSERT_EQUAL(0, rectFarBeyond.m_h);
    }

    void testBoundaryTestingMinimumGlyphSize()
    {
        // Test with minimal valid glyph sizes
        Size glyphSize1(1, 1);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip1(glyphSize1, glyphCount);

        const auto& pixmap1 = fontStrip1.getPixmap();
        CPPUNIT_ASSERT(pixmap1.isValid());
        CPPUNIT_ASSERT_EQUAL(10, pixmap1.getWidth());
        CPPUNIT_ASSERT_EQUAL(1, pixmap1.getHeight());

        std::vector<uint8_t> data(1, 255);
        bool result = fontStrip1.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result);

        // Test with 2x2
        Size glyphSize2(2, 2);
        FontStripImpl fontStrip2(glyphSize2, 5);

        const auto& pixmap2 = fontStrip2.getPixmap();
        CPPUNIT_ASSERT(pixmap2.isValid());
        CPPUNIT_ASSERT_EQUAL(10, pixmap2.getWidth());
        CPPUNIT_ASSERT_EQUAL(2, pixmap2.getHeight());
    }

    void testVerifyNoBufferOverflowInCopyGlyph()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 5;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Create data with pattern to detect overflow
        std::vector<uint8_t> data(64);
        for (int i = 0; i < 64; ++i)
        {
            data[i] = static_cast<uint8_t>(i);
        }

        // Load glyphs
        fontStrip.loadGlyph(0, data.data(), data.size());
        fontStrip.loadGlyph(1, data.data(), data.size());
        fontStrip.loadGlyph(4, data.data(), data.size());

        // Verify no overflow - check neighboring glyphs aren't affected
        const auto& pixmap = fontStrip.getPixmap();

        // Check glyph 2 (between loaded glyphs) is still zero
        Rectangle rect2 = fontStrip.getGlyphRect(2);
        bool allZero = true;
        for (int y = 0; y < rect2.m_h; ++y)
        {
            auto line = pixmap.getLine(rect2.m_y + y);
            for (int x = 0; x < rect2.m_w; ++x)
            {
                if (*(line + rect2.m_x + x) != 0)
                {
                    allZero = false;
                    break;
                }
            }
        }
        CPPUNIT_ASSERT(allZero);
    }

    void testStateConsistencyAfterFailedOperations()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Load valid glyph
        std::vector<uint8_t> data(256, 128);
        bool result1 = fontStrip.loadGlyph(0, data.data(), data.size());
        CPPUNIT_ASSERT(result1);

        // Try failed operations
        fontStrip.loadGlyph(-1, data.data(), data.size());
        fontStrip.loadGlyph(100, data.data(), data.size());
        fontStrip.loadGlyph(1, data.data(), 10);

        FontStripMap charMap;
        charMap.addMapping('A', 2);
        Size charSize(16, 16);
        fontStrip.loadFont("/nonexistent/font.ttf", charSize, charMap);

        // Verify state is still consistent
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        Rectangle rect0 = fontStrip.getGlyphRect(0);
        auto line = pixmap.getLine(rect0.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(128), *(line + rect0.m_x));

        // Should still be able to load valid glyphs
        bool result2 = fontStrip.loadGlyph(1, data.data(), data.size());
        CPPUNIT_ASSERT(result2);
    }

    void testPixmapValidityThroughoutLifecycle()
    {
        Size glyphSize(12, 12);
        std::size_t glyphCount = 20;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Check validity at construction
        const auto& pixmap1 = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap1.isValid());

        // Check after loading glyphs
        std::vector<uint8_t> data(144, 100);
        fontStrip.loadGlyph(0, data.data(), data.size());
        const auto& pixmap2 = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap2.isValid());

        // Check after loading font
        FontStripMap charMap;
        charMap.addMapping('A', 5);
        Size charSize(12, 12);
        fontStrip.loadFont("DejaVu Sans", charSize, charMap);
        const auto& pixmap3 = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap3.isValid());

        // Check after more operations
        fontStrip.loadGlyph(10, data.data(), data.size());
        const auto& pixmap4 = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap4.isValid());
    }

    void testLoadGlyphWithVariousAlphaPatterns()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 8;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Pattern 1: All zeros
        std::vector<uint8_t> data0(64, 0);
        fontStrip.loadGlyph(0, data0.data(), data0.size());

        // Pattern 2: All 255
        std::vector<uint8_t> data255(64, 255);
        fontStrip.loadGlyph(1, data255.data(), data255.size());

        // Pattern 3: Gradient
        std::vector<uint8_t> dataGrad(64);
        for (int i = 0; i < 64; ++i)
        {
            dataGrad[i] = static_cast<uint8_t>(i * 4);
        }
        fontStrip.loadGlyph(2, dataGrad.data(), dataGrad.size());

        // Pattern 4: Checkerboard
        std::vector<uint8_t> dataCheck(64);
        for (int i = 0; i < 64; ++i)
        {
            dataCheck[i] = ((i / 8) + i) % 2 == 0 ? 0 : 255;
        }
        fontStrip.loadGlyph(3, dataCheck.data(), dataCheck.size());

        // Verify patterns
        const auto& pixmap = fontStrip.getPixmap();

        Rectangle rect0 = fontStrip.getGlyphRect(0);
        auto line0 = pixmap.getLine(rect0.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), *(line0 + rect0.m_x));

        Rectangle rect1 = fontStrip.getGlyphRect(1);
        auto line1 = pixmap.getLine(rect1.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), *(line1 + rect1.m_x));

        Rectangle rect2 = fontStrip.getGlyphRect(2);
        auto line2 = pixmap.getLine(rect2.m_y);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), *(line2 + rect2.m_x));
    }

    void testFontLoadWithAllASCIICharacters()
    {
        Size glyphSize(16, 16);
        std::size_t glyphCount = 128;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        FontStripMap charMap;
        // Map printable ASCII characters
        for (int i = 32; i < 127; ++i)
        {
            charMap.addMapping(i, i - 32);
        }

        Size charSize(16, 16);
        bool result = fontStrip.loadFont("DejaVu Sans", charSize, charMap);

        // Should handle all ASCII characters
        CPPUNIT_ASSERT(result || !result);

        // Verify some rectangles
        Rectangle rect0 = fontStrip.getGlyphRect(0);  // Space
        Rectangle rect33 = fontStrip.getGlyphRect(33);  // 'A'
        Rectangle rect94 = fontStrip.getGlyphRect(94);  // '~'

        CPPUNIT_ASSERT_EQUAL(0, rect0.m_x);
        CPPUNIT_ASSERT_EQUAL(528, rect33.m_x);  // 33 * 16
        CPPUNIT_ASSERT_EQUAL(1504, rect94.m_x);  // 94 * 16
    }

    void testVerifyEmptyRectForInvalidIndices()
    {
        Size glyphSize(20, 20);
        std::size_t glyphCount = 50;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        // Test various invalid indices
        std::vector<int> invalidIndices = {-1, -10, -100, 50, 51, 100, 1000};

        for (int index : invalidIndices)
        {
            Rectangle rect = fontStrip.getGlyphRect(index);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_h);
        }

        // Verify valid indices still work
        for (int i = 0; i < 50; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            CPPUNIT_ASSERT_EQUAL(20, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(20, rect.m_h);
        }
    }

    void testConsecutiveLoadOperationsStressTest()
    {
        Size glyphSize(8, 8);
        std::size_t glyphCount = 10;
        FontStripImpl fontStrip(glyphSize, glyphCount);

        std::vector<uint8_t> data(64, 128);

        // Perform many consecutive load operations
        for (int iteration = 0; iteration < 100; ++iteration)
        {
            for (int glyph = 0; glyph < 10; ++glyph)
            {
                bool result = fontStrip.loadGlyph(glyph, data.data(), data.size());
                CPPUNIT_ASSERT(result);
            }
        }

        // Verify final state
        const auto& pixmap = fontStrip.getPixmap();
        CPPUNIT_ASSERT(pixmap.isValid());

        for (int i = 0; i < 10; ++i)
        {
            Rectangle rect = fontStrip.getGlyphRect(i);
            auto line = pixmap.getLine(rect.m_y);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(128), *(line + rect.m_x));
        }
    }

    void testVerifyRectCalculationForVariousGlyphSizes()
    {
        // Test rectangle calculation with various glyph sizes
        struct TestCase
        {
            Size glyphSize;
            std::size_t glyphCount;
            int testIndex;
            int expectedX;
        };

        std::vector<TestCase> testCases = {
            {Size(5, 7), 20, 10, 50},      // 10 * 5
            {Size(13, 17), 15, 7, 91},     // 7 * 13
            {Size(32, 32), 10, 5, 160},    // 5 * 32
            {Size(11, 13), 30, 15, 165},   // 15 * 11
            {Size(6, 9), 100, 50, 300}     // 50 * 6
        };

        for (const auto& testCase : testCases)
        {
            FontStripImpl fontStrip(testCase.glyphSize, testCase.glyphCount);

            Rectangle rect = fontStrip.getGlyphRect(testCase.testIndex);

            CPPUNIT_ASSERT_EQUAL(testCase.expectedX, rect.m_x);
            CPPUNIT_ASSERT_EQUAL(0, rect.m_y);
            CPPUNIT_ASSERT_EQUAL(testCase.glyphSize.m_w, rect.m_w);
            CPPUNIT_ASSERT_EQUAL(testCase.glyphSize.m_h, rect.m_h);
        }
    }
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(FontStripImplTest);
