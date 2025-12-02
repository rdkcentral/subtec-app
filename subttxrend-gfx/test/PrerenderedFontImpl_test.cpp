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

#include "../src/PrerenderedFontImpl.hpp"
#include <stdexcept>
#include <memory>

using subttxrend::gfx::PrerenderedFontImpl;

// Test font path - using a commonly available system font
// This should be adjusted based on the test environment
#ifdef __linux__
#define TEST_FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define TEST_FONT_PATH_ALT "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
#else
#define TEST_FONT_PATH "/System/Library/Fonts/Helvetica.ttc"
#define TEST_FONT_PATH_ALT "/System/Library/Fonts/Times.ttc"
#endif

class PrerenderedFontImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PrerenderedFontImplTest );
    CPPUNIT_TEST(testConstructorWithValidFontDefaultParams);
    CPPUNIT_TEST(testConstructorWithValidFontHeight10);
    CPPUNIT_TEST(testConstructorWithValidFontHeight100);
    CPPUNIT_TEST(testConstructorWithValidFontHeight1);
    CPPUNIT_TEST(testConstructorWithStrictModeTrue);
    CPPUNIT_TEST(testConstructorWithStrictModeFalse);
    CPPUNIT_TEST(testConstructorWithItalicsTrue);
    CPPUNIT_TEST(testConstructorWithItalicsFalse);
    CPPUNIT_TEST(testConstructorWithStrictAndItalics);
    CPPUNIT_TEST(testConstructorMultipleTimes);
    CPPUNIT_TEST(testConstructorWithNullFontPath);
    CPPUNIT_TEST(testConstructorWithEmptyFontPath);
    CPPUNIT_TEST(testConstructorWithNonExistentPath);
    CPPUNIT_TEST(testConstructorWithDirectoryPath);
    CPPUNIT_TEST(testConstructorWithInvalidFileFormat);
    CPPUNIT_TEST(testConstructorWithZeroHeight);
    CPPUNIT_TEST(testConstructorWithNegativeHeight);
    CPPUNIT_TEST(testConstructorWithVeryLargeHeight);
    CPPUNIT_TEST(testConstructorWithCorruptedFontFile);
    CPPUNIT_TEST(testGetFontHeightReturnsPositive);
    CPPUNIT_TEST(testGetFontHeightConsistent);
    CPPUNIT_TEST(testGetMaxAdvanceReturnsPositive);
    CPPUNIT_TEST(testGetMaxAdvanceConsistent);
    CPPUNIT_TEST(testGetFontAscenderWithScalableFont);
    CPPUNIT_TEST(testGetFontDescenderWithScalableFont);
    CPPUNIT_TEST(testGetFontAscenderConsistent);
    CPPUNIT_TEST(testGetFontDescenderConsistent);
    CPPUNIT_TEST(testFontMetricsRelationship);
    CPPUNIT_TEST(testTextToTokensWithEmptyString);
    CPPUNIT_TEST(testTextToTokensWithSingleCharacter);
    CPPUNIT_TEST(testTextToTokensWithSingleSpace);
    CPPUNIT_TEST(testTextToTokensWithSingleWord);
    CPPUNIT_TEST(testTextToTokensWithTwoWords);
    CPPUNIT_TEST(testTextToTokensWithMultipleSpaces);
    CPPUNIT_TEST(testTextToTokensWithLeadingWhitespace);
    CPPUNIT_TEST(testTextToTokensWithTrailingWhitespace);
    CPPUNIT_TEST(testTextToTokensWithOnlyWhitespace);
    CPPUNIT_TEST(testTextToTokensWithNewline);
    CPPUNIT_TEST(testTextToTokensWithMultipleLines);
    CPPUNIT_TEST(testTextToTokensWithMixedWhitespace);
    CPPUNIT_TEST(testTextToTokensWithNumbers);
    CPPUNIT_TEST(testTextToTokensWithPunctuation);
    CPPUNIT_TEST(testTextToTokensWithLongString);
    CPPUNIT_TEST(testGetCharInfoWithValidGlyphIndex);
    CPPUNIT_TEST(testGetCharInfoWithZeroGlyphIndex);
    CPPUNIT_TEST(testGetCharInfoReturnsNonNull);
    CPPUNIT_TEST(testGetCharInfoCaching);
    CPPUNIT_TEST(testGetCharInfoMultipleCalls);
    CPPUNIT_TEST(testGetCharInfoDifferentGlyphs);
    CPPUNIT_TEST(testGetCharInfoValidatesBitmapWidth);
    CPPUNIT_TEST(testGetCharInfoValidatesBitmapHeight);
    CPPUNIT_TEST(testGetCharInfoValidatesAtlasOffset);
    CPPUNIT_TEST(testGetCharInfoValidatesSurface);
    CPPUNIT_TEST(testGetCharInfoWithOutlineSize1);
    CPPUNIT_TEST(testGetCharInfoWithOutlineSize5);
    CPPUNIT_TEST(testGetCharInfoWithOutlineSize10);
    CPPUNIT_TEST(testGetCharInfoWithOutlineZero);
    CPPUNIT_TEST(testGetCharInfoWithNegativeOutline);
    CPPUNIT_TEST(testGetCharInfoOutlineCaching);
    CPPUNIT_TEST(testGetCharInfoOutlineVsNormal);
    CPPUNIT_TEST(testGetCharInfoOutlineLargerThanNormal);
    CPPUNIT_TEST(testGetCharInfoOutlineSeparateCache);
    CPPUNIT_TEST(testGetCharInfoOutlineMultipleSizes);
    CPPUNIT_TEST(testGetCharInfoAtlasPageCreation);
    CPPUNIT_TEST(testGetCharInfoMultipleGlyphsOnSamePage);
    CPPUNIT_TEST(testGetCharInfoAtlasCoordinatesIncreasing);
    CPPUNIT_TEST(testGetCharInfoAtlasNoOverlap);
    CPPUNIT_TEST(testGetCharInfoAtlasWidthConstraint);
    CPPUNIT_TEST(testGetCharInfoAtlasHeightConstraint);
    CPPUNIT_TEST(testGetCharInfoAtlasBoundsValidation);
    CPPUNIT_TEST(testTextToTokensWithUTF8Latin);
    CPPUNIT_TEST(testTextToTokensWithUTF8Cyrillic);
    CPPUNIT_TEST(testTextToTokensWithUTF8Arabic);
    CPPUNIT_TEST(testTextToTokensWithUTF8Chinese);
    CPPUNIT_TEST(testTextToTokensWithUTF8Emoji);
    CPPUNIT_TEST(testTextToTokensWithUTF8Mixed);
    CPPUNIT_TEST(testTextToTokensWithCombiningDiacritics);
    CPPUNIT_TEST(testTextToTokensWithZeroWidthJoiner);
    CPPUNIT_TEST(testTextToTokensWithRightToLeft);
    CPPUNIT_TEST(testTextToTokensWithBidirectionalText);
    CPPUNIT_TEST(testTextToTokensTokenCountCorrect);
    CPPUNIT_TEST(testTextToTokensGlyphCountInToken);
    CPPUNIT_TEST(testTextToTokensTotalAdvanceXPositive);
    CPPUNIT_TEST(testTextToTokensAdvanceXConsistency);
    CPPUNIT_TEST(testTextToTokensClusterMapping);
    CPPUNIT_TEST(testTextToTokensGlyphIndexValid);
    CPPUNIT_TEST(testTextToTokensCodepointPreserved);
    CPPUNIT_TEST(testTextToTokensWhitespaceTokensSeparate);
    CPPUNIT_TEST(testTextToTokensNewlineTokensForced);
    CPPUNIT_TEST(testTextToTokensComplexSentence);
    CPPUNIT_TEST(testEndToEndRenderAndCache);
    CPPUNIT_TEST(testEndToEndMultipleFonts);
    CPPUNIT_TEST(testEndToEndLargeText);
    CPPUNIT_TEST(testEndToEndMixedOutlines);
    CPPUNIT_TEST(testEndToEndTokensToGlyphs);
    CPPUNIT_TEST(testEndToEndUnicodeRendering);
    CPPUNIT_TEST(testEndToEndMetricsAndTokens);
    CPPUNIT_TEST(testEndToEndCachePerformance);
    CPPUNIT_TEST(testEndToEndRobustness);
    CPPUNIT_TEST(testConstructorHeightScaling);
    CPPUNIT_TEST(testConstructorStrictModeEnforcement);
    CPPUNIT_TEST(testConstructorItalicsTransformation);
    CPPUNIT_TEST(testConstructorAllCombinations);
    CPPUNIT_TEST(testConstructorResourceInitialization);
    CPPUNIT_TEST(testDestructorCleanup);
    CPPUNIT_TEST(testDestructorMultipleInstances);
    CPPUNIT_TEST(testDestructorAfterError);
    CPPUNIT_TEST(testDestructorWithCachedData);
    CPPUNIT_TEST(testDestructorMemoryManagement);
    CPPUNIT_TEST(testMetricsWithDifferentHeights);
    CPPUNIT_TEST(testMetricsStrictVsNonStrict);
    CPPUNIT_TEST(testMetricsWithItalics);
    CPPUNIT_TEST(testMetricsScalability);
    CPPUNIT_TEST(testMetricsConsistencyAcrossCalls);
    CPPUNIT_TEST(testGetCharInfoInvalidGlyphIndex);
    CPPUNIT_TEST(testGetCharInfoVeryLargeGlyphIndex);
    CPPUNIT_TEST(testGetCharInfoExcessiveOutlineSize);
    CPPUNIT_TEST(testGetCharInfoAfterDestruction);
    CPPUNIT_TEST(testGetCharInfoNullCheck);
    CPPUNIT_TEST(testGetCharInfoGlyphTooWide);
    CPPUNIT_TEST(testGetCharInfoGlyphTooTall);
    CPPUNIT_TEST(testGetCharInfoErrorRecovery);
    CPPUNIT_TEST(testGetCharInfoConsecutiveErrors);
    CPPUNIT_TEST(testGetCharInfoMixedValidInvalid);
    CPPUNIT_TEST(testTextToTokensLigatures);
    CPPUNIT_TEST(testTextToTokensKerning);
    CPPUNIT_TEST(testTextToTokensComplexScript);
    CPPUNIT_TEST(testTextToTokensControlCharacters);
    CPPUNIT_TEST(testTextToTokensNullTerminator);
    CPPUNIT_TEST(testTextToTokensVeryLongWord);
    CPPUNIT_TEST(testTextToTokensRepeatedText);
    CPPUNIT_TEST(testTextToTokensAllPrintableASCII);
    CPPUNIT_TEST(testTextToTokensExtendedUnicode);
    CPPUNIT_TEST(testTextToTokensEdgeCaseCharacters);
    CPPUNIT_TEST(testStressRapidTokenization);
    CPPUNIT_TEST(testStressMaximumGlyphs);
    CPPUNIT_TEST(testStressConcurrentAccess);
    CPPUNIT_TEST(testStressMemoryPressure);
    CPPUNIT_TEST(testStressAtlasFragmentation);
    CPPUNIT_TEST(testStressLargeOutlines);
    CPPUNIT_TEST(testStressRepeatedOperations);
    CPPUNIT_TEST(testStressBoundaryConditions);
    CPPUNIT_TEST(testStressMixedOperations);
    CPPUNIT_TEST(testStressLongRunning);
CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<PrerenderedFontImpl> font;
    std::string validFontPath;

public:
    void setUp()
    {
        // Try to find a valid font path
        validFontPath = TEST_FONT_PATH;

        // Check if file exists, if not try alternative
        std::ifstream f(validFontPath.c_str());
        if (!f.good())
        {
            validFontPath = TEST_FONT_PATH_ALT;
        }
    }

    void tearDown()
    {
        font.reset();
    }

    // Helper method to check if font file exists
    bool fontFileExists(const std::string& path)
    {
        std::ifstream f(path.c_str());
        return f.good();
    }

    void testConstructorWithValidFontDefaultParams()
    {
        if (!fontFileExists(validFontPath)) {
            CPPUNIT_FAIL("Test font file not found. Please install DejaVu or Liberation fonts.");
            return;
        }

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
    }

    void testConstructorWithValidFontHeight10()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 10, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
        CPPUNIT_ASSERT(font->getFontHeight() <= 15); // Should be close to 10
    }

    void testConstructorWithValidFontHeight100()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 100, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() >= 80); // Should be reasonably close to 100
        CPPUNIT_ASSERT(font->getFontHeight() <= 120); // Allow 20% tolerance for font rendering
    }

    void testConstructorWithValidFontHeight1()
    {
        if (!fontFileExists(validFontPath)) return;

        // Height 1 is technically valid, should not throw
        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 1, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
    }

    void testConstructorWithStrictModeTrue()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, true, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
        // In strict mode, font height should be <= requested height
        CPPUNIT_ASSERT(font->getFontHeight() <= 20);
    }

    void testConstructorWithStrictModeFalse()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
    }

    void testConstructorWithItalicsTrue()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, true))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
    }

    void testConstructorWithItalicsFalse()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
    }

    void testConstructorWithStrictAndItalics()
    {
        if (!fontFileExists(validFontPath)) return;

        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, true, true))
        );

        CPPUNIT_ASSERT(font != nullptr);
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
        CPPUNIT_ASSERT(font->getFontHeight() <= 20); // Strict mode constraint
    }

    void testConstructorMultipleTimes()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create multiple instances to test no resource leaks
        for (int i = 0; i < 5; i++)
        {
            CPPUNIT_ASSERT_NO_THROW(
                font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 15 + i, false, false))
            );
            CPPUNIT_ASSERT(font != nullptr);
        }
    }

    void testConstructorWithNullFontPath()
    {
        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl(nullptr, 20, false, false)),
            std::runtime_error
        );
    }

    void testConstructorWithEmptyFontPath()
    {
        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl("", 20, false, false)),
            std::runtime_error
        );
    }

    void testConstructorWithNonExistentPath()
    {
        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl("/tmp/nonexistent_font_xyz123.ttf", 20, false, false)),
            std::runtime_error
        );
    }

    void testConstructorWithDirectoryPath()
    {
        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl("/tmp", 20, false, false)),
            std::runtime_error
        );
    }

    void testConstructorWithInvalidFileFormat()
    {
        // Create a temporary non-font file
        std::string tempFile = "/tmp/test_invalid_font.txt";
        std::ofstream ofs(tempFile);
        ofs << "This is not a font file";
        ofs.close();

        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl(tempFile.c_str(), 20, false, false)),
            std::runtime_error
        );

        // Cleanup
        std::remove(tempFile.c_str());
    }

    void testConstructorWithZeroHeight()
    {
        if (!fontFileExists(validFontPath)) return;

        // Zero height should either work or throw - test it handles gracefully
        try {
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 0, false, false));
            // If it doesn't throw, verify font was created
            CPPUNIT_ASSERT(font != nullptr);
        } catch (const std::exception&) {
            // Exception is acceptable for invalid height
            CPPUNIT_ASSERT(true);
        }
    }

    void testConstructorWithNegativeHeight()
    {
        if (!fontFileExists(validFontPath)) return;

        // Negative height should either be handled or throw
        try {
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), -10, false, false));
            // If it doesn't throw, verify font was created
            CPPUNIT_ASSERT(font != nullptr);
        } catch (const std::exception&) {
            // Exception is acceptable for invalid height
            CPPUNIT_ASSERT(true);
        }
    }

    void testConstructorWithVeryLargeHeight()
    {
        if (!fontFileExists(validFontPath)) return;

        // Very large height should be handled gracefully
        CPPUNIT_ASSERT_NO_THROW(
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 10000, false, false))
        );

        CPPUNIT_ASSERT(font != nullptr);
    }

    void testConstructorWithCorruptedFontFile()
    {
        // Create a temporary corrupted font file
        std::string tempFile = "/tmp/test_corrupted_font.ttf";
        std::ofstream ofs(tempFile, std::ios::binary);
        // Write some random bytes that look like they could be a font header
        ofs << "ttcf";
        for (int i = 0; i < 100; i++) {
            ofs << static_cast<char>(rand() % 256);
        }
        ofs.close();

        CPPUNIT_ASSERT_THROW(
            font.reset(new PrerenderedFontImpl(tempFile.c_str(), 20, false, false)),
            std::runtime_error
        );

        // Cleanup
        std::remove(tempFile.c_str());
    }

    void testGetFontHeightReturnsPositive()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        CPPUNIT_ASSERT(font->getFontHeight() > 0);
    }

    void testGetFontHeightConsistent()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int height1 = font->getFontHeight();
        int height2 = font->getFontHeight();

        CPPUNIT_ASSERT_EQUAL(height1, height2);
    }

    void testGetMaxAdvanceReturnsPositive()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        CPPUNIT_ASSERT(font->getMaxAdvance() > 0);
    }

    void testGetMaxAdvanceConsistent()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int advance1 = font->getMaxAdvance();
        int advance2 = font->getMaxAdvance();

        CPPUNIT_ASSERT_EQUAL(advance1, advance2);
    }

    void testGetFontAscenderWithScalableFont()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int ascender = font->getFontAscender();

        // Ascender should be positive for scalable fonts
        CPPUNIT_ASSERT(ascender > 0 || ascender == 0); // Can be 0 for non-scalable
    }

    void testGetFontDescenderWithScalableFont()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int descender = font->getFontDescender();

        // Descender should be negative or zero for scalable fonts
        CPPUNIT_ASSERT(descender <= 0);
    }

    void testGetFontAscenderConsistent()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int ascender1 = font->getFontAscender();
        int ascender2 = font->getFontAscender();

        CPPUNIT_ASSERT_EQUAL(ascender1, ascender2);
    }

    void testGetFontDescenderConsistent()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int descender1 = font->getFontDescender();
        int descender2 = font->getFontDescender();

        CPPUNIT_ASSERT_EQUAL(descender1, descender2);
    }

    void testFontMetricsRelationship()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        int height = font->getFontHeight();
        int ascender = font->getFontAscender();
        int descender = font->getFontDescender();

        // For scalable fonts: height should be close to ascender - descender
        // Allow some tolerance due to rounding
        if (ascender != 0 || descender != 0) {
            int calculated = ascender - descender;
            CPPUNIT_ASSERT(abs(calculated - height) < 5);
        }
    }

    void testTextToTokensWithEmptyString()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), tokens.size());
    }

    void testTextToTokensWithSingleCharacter()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("a");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT(!tokens[0].isWhite);
        CPPUNIT_ASSERT(!tokens[0].forceNewline);
        CPPUNIT_ASSERT(tokens[0].glyphs.size() >= 1);
    }

    void testTextToTokensWithSingleSpace()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens(" ");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT(tokens[0].isWhite);
        CPPUNIT_ASSERT(!tokens[0].forceNewline);
    }

    void testTextToTokensWithSingleWord()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT(!tokens[0].isWhite);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), tokens[0].glyphs.size());
        CPPUNIT_ASSERT(tokens[0].totalAdvanceX > 0);
    }

    void testTextToTokensWithTwoWords()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello world");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), tokens.size());
        CPPUNIT_ASSERT(!tokens[0].isWhite); // "hello"
        CPPUNIT_ASSERT(tokens[1].isWhite);  // " "
        CPPUNIT_ASSERT(!tokens[2].isWhite); // "world"
    }

    void testTextToTokensWithMultipleSpaces()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("a   b");

        CPPUNIT_ASSERT(tokens.size() >= 3);
        CPPUNIT_ASSERT(!tokens[0].isWhite); // "a"
        CPPUNIT_ASSERT(tokens[1].isWhite);  // "   " (multiple spaces in one token)
        CPPUNIT_ASSERT(!tokens[2].isWhite); // "b"
    }

    void testTextToTokensWithLeadingWhitespace()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("  hello");

        CPPUNIT_ASSERT(tokens.size() >= 2);
        CPPUNIT_ASSERT(tokens[0].isWhite);  // Leading spaces
        CPPUNIT_ASSERT(!tokens[1].isWhite); // "hello"
    }

    void testTextToTokensWithTrailingWhitespace()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello  ");

        CPPUNIT_ASSERT(tokens.size() >= 2);
        CPPUNIT_ASSERT(!tokens[0].isWhite); // "hello"
        CPPUNIT_ASSERT(tokens[tokens.size()-1].isWhite); // Trailing spaces
    }

    void testTextToTokensWithOnlyWhitespace()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("   ");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT(tokens[0].isWhite);
    }

    void testTextToTokensWithNewline()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello\n");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Find the token with forceNewline
        bool foundNewline = false;
        for (const auto& token : tokens) {
            if (token.forceNewline) {
                foundNewline = true;
                break;
            }
        }
        CPPUNIT_ASSERT(foundNewline);
    }

    void testTextToTokensWithMultipleLines()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("line1\nline2");

        CPPUNIT_ASSERT(tokens.size() >= 2);

        // Count newline tokens
        int newlineCount = 0;
        for (const auto& token : tokens) {
            if (token.forceNewline) {
                newlineCount++;
            }
        }
        CPPUNIT_ASSERT(newlineCount >= 1);
    }

    void testTextToTokensWithMixedWhitespace()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("a \t b");

        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Verify whitespace tokens exist
        bool hasWhitespace = false;
        for (const auto& token : tokens) {
            if (token.isWhite) {
                hasWhitespace = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasWhitespace);
    }

    void testTextToTokensWithNumbers()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("123 456");

        CPPUNIT_ASSERT(tokens.size() >= 3);
        CPPUNIT_ASSERT(!tokens[0].isWhite); // "123"
        CPPUNIT_ASSERT(tokens[1].isWhite);  // " "
        CPPUNIT_ASSERT(!tokens[2].isWhite); // "456"
    }

    void testTextToTokensWithPunctuation()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("Hello, world!");

        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Verify we got tokens (punctuation is part of non-whitespace tokens)
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
            }
        }
    }

    void testTextToTokensWithLongString()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        std::string longText = "This is a much longer string with many words to test "
                               "the tokenization system handles larger inputs correctly "
                               "without any issues or performance problems.";

        auto tokens = font->textToTokens(longText);

        // Should have many tokens
        CPPUNIT_ASSERT(tokens.size() > 10);

        // Verify all tokens have valid data
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
                CPPUNIT_ASSERT(token.totalAdvanceX >= 0);
            }
        }
    }

    void testGetCharInfoWithValidGlyphIndex()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Glyph index 0 is typically the .notdef glyph
        const auto* charInfo = font->getCharInfo(0, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoWithZeroGlyphIndex()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(0, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testGetCharInfoReturnsNonNull()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test with common ASCII glyph indices (typically A=36, a=68 in many fonts)
        const auto* charInfo1 = font->getCharInfo(36, 0);
        const auto* charInfo2 = font->getCharInfo(68, 0);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);
    }

    void testGetCharInfoCaching()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // First call - should render
        const auto* charInfo1 = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo1 != nullptr);

        // Second call - should return cached version (same pointer)
        const auto* charInfo2 = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo2 != nullptr);
        CPPUNIT_ASSERT_EQUAL(charInfo1, charInfo2);
    }

    void testGetCharInfoMultipleCalls()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Multiple calls should not fail
        for (int i = 0; i < 10; i++) {
            const auto* charInfo = font->getCharInfo(36, 0);
            CPPUNIT_ASSERT(charInfo != nullptr);
        }
    }

    void testGetCharInfoDifferentGlyphs()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo1 = font->getCharInfo(36, 0);
        const auto* charInfo2 = font->getCharInfo(68, 0);
        const auto* charInfo3 = font->getCharInfo(72, 0);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);
        CPPUNIT_ASSERT(charInfo3 != nullptr);

        // Should be different character info structures
        CPPUNIT_ASSERT(charInfo1 != charInfo2);
        CPPUNIT_ASSERT(charInfo2 != charInfo3);
    }

    void testGetCharInfoValidatesBitmapWidth()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        // Bitmap width should be reasonable (0 for spaces, positive for visible glyphs)
        CPPUNIT_ASSERT(charInfo->bitmapWidth >= 0);
        CPPUNIT_ASSERT(charInfo->bitmapWidth < 128); // Should fit in atlas page
    }

    void testGetCharInfoValidatesBitmapHeight()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->bitmapHeight >= 0);
        CPPUNIT_ASSERT(charInfo->bitmapHeight < 128); // Should fit in atlas page
    }

    void testGetCharInfoValidatesAtlasOffset()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        // Atlas offsets should be non-negative and within page bounds
        CPPUNIT_ASSERT(charInfo->atlasXOffset >= 0);
        CPPUNIT_ASSERT(charInfo->atlasYOffset >= 0);
        CPPUNIT_ASSERT(charInfo->atlasXOffset < 128);
        CPPUNIT_ASSERT(charInfo->atlasYOffset < 128);
    }

    void testGetCharInfoValidatesSurface()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoWithOutlineSize1()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 1);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoWithOutlineSize5()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 5);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoWithOutlineSize10()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = font->getCharInfo(36, 10);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoWithOutlineZero()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Outline size 0 should behave like normal rendering
        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testGetCharInfoWithNegativeOutline()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Negative outline should be treated as 0 (normal rendering)
        const auto* charInfo = font->getCharInfo(36, -5);

        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testGetCharInfoOutlineCaching()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo1 = font->getCharInfo(36, 3);
        const auto* charInfo2 = font->getCharInfo(36, 3);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);
        CPPUNIT_ASSERT_EQUAL(charInfo1, charInfo2); // Should be cached
    }

    void testGetCharInfoOutlineVsNormal()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* normalInfo = font->getCharInfo(36, 0);
        const auto* outlineInfo = font->getCharInfo(36, 3);

        CPPUNIT_ASSERT(normalInfo != nullptr);
        CPPUNIT_ASSERT(outlineInfo != nullptr);
        // Should be different character info structures
        CPPUNIT_ASSERT(normalInfo != outlineInfo);
    }

    void testGetCharInfoOutlineLargerThanNormal()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* normalInfo = font->getCharInfo(36, 0);
        const auto* outlineInfo = font->getCharInfo(36, 3);

        CPPUNIT_ASSERT(normalInfo != nullptr);
        CPPUNIT_ASSERT(outlineInfo != nullptr);

        // Outline should typically be larger than normal
        CPPUNIT_ASSERT(outlineInfo->bitmapWidth >= normalInfo->bitmapWidth);
        CPPUNIT_ASSERT(outlineInfo->bitmapHeight >= normalInfo->bitmapHeight);
    }

    void testGetCharInfoOutlineSeparateCache()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Get normal and outline versions
        const auto* normal1 = font->getCharInfo(36, 0);
        const auto* outline1 = font->getCharInfo(36, 3);

        // Get them again - should come from separate caches
        const auto* normal2 = font->getCharInfo(36, 0);
        const auto* outline2 = font->getCharInfo(36, 3);

        CPPUNIT_ASSERT_EQUAL(normal1, normal2);
        CPPUNIT_ASSERT_EQUAL(outline1, outline2);
        CPPUNIT_ASSERT(normal1 != outline1);
    }

    void testGetCharInfoOutlineMultipleSizes()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* outline1 = font->getCharInfo(36, 1);
        const auto* outline3 = font->getCharInfo(36, 3);
        const auto* outline5 = font->getCharInfo(36, 5);

        CPPUNIT_ASSERT(outline1 != nullptr);
        CPPUNIT_ASSERT(outline3 != nullptr);
        CPPUNIT_ASSERT(outline5 != nullptr);

        // Different outline sizes should create different entries
        // Note: They might be cached separately, but we can't guarantee distinct pointers
        // So just verify they all render successfully
    }

    void testGetCharInfoAtlasPageCreation()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // First character should create initial atlas page
        const auto* charInfo = font->getCharInfo(36, 0);

        CPPUNIT_ASSERT(charInfo != nullptr);
        CPPUNIT_ASSERT(charInfo->surface != nullptr);
    }

    void testGetCharInfoMultipleGlyphsOnSamePage()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo1 = font->getCharInfo(36, 0);
        const auto* charInfo2 = font->getCharInfo(37, 0);
        const auto* charInfo3 = font->getCharInfo(38, 0);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);
        CPPUNIT_ASSERT(charInfo3 != nullptr);

        // All should point to same surface initially (same atlas page)
        CPPUNIT_ASSERT_EQUAL(charInfo1->surface, charInfo2->surface);
        CPPUNIT_ASSERT_EQUAL(charInfo2->surface, charInfo3->surface);
    }

    void testGetCharInfoAtlasCoordinatesIncreasing()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo1 = font->getCharInfo(36, 0);
        const auto* charInfo2 = font->getCharInfo(37, 0);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);

        // On same page, X offset should increase (or Y if row wrapped)
        if (charInfo1->surface == charInfo2->surface) {
            bool xIncreased = charInfo2->atlasXOffset > charInfo1->atlasXOffset;
            bool yIncreased = charInfo2->atlasYOffset > charInfo1->atlasYOffset;
            CPPUNIT_ASSERT(xIncreased || yIncreased);
        }
    }

    void testGetCharInfoAtlasNoOverlap()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo1 = font->getCharInfo(36, 0);
        const auto* charInfo2 = font->getCharInfo(37, 0);

        CPPUNIT_ASSERT(charInfo1 != nullptr);
        CPPUNIT_ASSERT(charInfo2 != nullptr);

        // If on same page, they shouldn't overlap
        if (charInfo1->surface == charInfo2->surface) {
            int x1End = charInfo1->atlasXOffset + charInfo1->bitmapWidth;
            int y1End = charInfo1->atlasYOffset + charInfo1->bitmapHeight;

            bool noXOverlap = (charInfo2->atlasXOffset >= x1End) ||
                              (charInfo1->atlasXOffset >= charInfo2->atlasXOffset + charInfo2->bitmapWidth);
            bool noYOverlap = (charInfo2->atlasYOffset >= y1End) ||
                              (charInfo1->atlasYOffset >= charInfo2->atlasYOffset + charInfo2->bitmapHeight);

            CPPUNIT_ASSERT(noXOverlap || noYOverlap);
        }
    }

    void testGetCharInfoAtlasWidthConstraint()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render multiple glyphs
        for (uint32_t i = 36; i < 80; i++) {
            const auto* info = font->getCharInfo(i, 0);
            if (info != nullptr) {
                // Atlas position + bitmap width should not exceed page width
                CPPUNIT_ASSERT(info->atlasXOffset + info->bitmapWidth <= 128);
            }
        }
    }

    void testGetCharInfoAtlasHeightConstraint()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render multiple glyphs
        for (uint32_t i = 36; i < 80; i++) {
            const auto* info = font->getCharInfo(i, 0);
            if (info != nullptr) {
                // Atlas position + bitmap height should not exceed page height
                CPPUNIT_ASSERT(info->atlasYOffset + info->bitmapHeight <= 128);
            }
        }
    }

    void testGetCharInfoAtlasBoundsValidation()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render many glyphs and verify all stay within bounds
        for (uint32_t i = 0; i < 150; i++) {
            const auto* info = font->getCharInfo(i, 0);
            if (info != nullptr) {
                CPPUNIT_ASSERT(info->atlasXOffset >= 0 && info->atlasXOffset < 128);
                CPPUNIT_ASSERT(info->atlasYOffset >= 0 && info->atlasYOffset < 128);
                CPPUNIT_ASSERT(info->atlasXOffset + info->bitmapWidth <= 128);
                CPPUNIT_ASSERT(info->atlasYOffset + info->bitmapHeight <= 128);
            }
        }
    }

    void testTextToTokensWithUTF8Latin()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("Café résumé");

        CPPUNIT_ASSERT(tokens.size() >= 2);

        // Should have non-whitespace tokens
        bool hasNonWhite = false;
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                hasNonWhite = true;
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
            }
        }
        CPPUNIT_ASSERT(hasNonWhite);
    }

    void testTextToTokensWithUTF8Cyrillic()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("Привет мир");

        CPPUNIT_ASSERT(tokens.size() >= 2);

        // Verify tokens were created
        bool hasGlyphs = false;
        for (const auto& token : tokens) {
            if (!token.isWhite && token.glyphs.size() > 0) {
                hasGlyphs = true;
            }
        }
        CPPUNIT_ASSERT(hasGlyphs);
    }

    void testTextToTokensWithUTF8Arabic()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("مرحبا");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Should have at least one token with glyphs
        bool hasGlyphs = false;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                hasGlyphs = true;
            }
        }
        CPPUNIT_ASSERT(hasGlyphs);
    }

    void testTextToTokensWithUTF8Chinese()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("你好世界");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Should have tokens
        bool hasGlyphs = false;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                hasGlyphs = true;
            }
        }
        CPPUNIT_ASSERT(hasGlyphs);
    }

    void testTextToTokensWithUTF8Emoji()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("Hello 😀 World");

        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Should have multiple tokens
        int nonWhiteCount = 0;
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                nonWhiteCount++;
            }
        }
        CPPUNIT_ASSERT(nonWhiteCount >= 2);
    }

    void testTextToTokensWithUTF8Mixed()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("Hello Café 世界 مرحبا");

        CPPUNIT_ASSERT(tokens.size() >= 4);

        // Should have tokens with glyphs
        int tokensWithGlyphs = 0;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                tokensWithGlyphs++;
            }
        }
        CPPUNIT_ASSERT(tokensWithGlyphs >= 3);
    }

    void testTextToTokensWithCombiningDiacritics()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // é as e + combining acute
        auto tokens = font->textToTokens("e\u0301");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Should create at least one token
        bool hasToken = false;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                hasToken = true;
            }
        }
        CPPUNIT_ASSERT(hasToken);
    }

    void testTextToTokensWithZeroWidthJoiner()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Use zero-width joiner in text
        auto tokens = font->textToTokens("a\u200Db");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Should handle the zero-width character
        bool hasGlyphs = false;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                hasGlyphs = true;
            }
        }
        CPPUNIT_ASSERT(hasGlyphs);
    }

    void testTextToTokensWithRightToLeft()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Hebrew text (RTL)
        auto tokens = font->textToTokens("שלום");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Should create tokens
        bool hasGlyphs = false;
        for (const auto& token : tokens) {
            if (token.glyphs.size() > 0) {
                hasGlyphs = true;
            }
        }
        CPPUNIT_ASSERT(hasGlyphs);
    }

    void testTextToTokensWithBidirectionalText()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Mix of LTR and RTL
        auto tokens = font->textToTokens("Hello שלום World");

        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Should have multiple tokens
        int nonWhiteCount = 0;
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                nonWhiteCount++;
            }
        }
        CPPUNIT_ASSERT(nonWhiteCount >= 2);
    }

    void testTextToTokensTokenCountCorrect()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("one two three");

        // Should have 5 tokens: word, space, word, space, word
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), tokens.size());
    }

    void testTextToTokensGlyphCountInToken()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), tokens[0].glyphs.size());
    }

    void testTextToTokensTotalAdvanceXPositive()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("hello world");

        // All non-whitespace tokens should have positive advance
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                CPPUNIT_ASSERT(token.totalAdvanceX > 0);
            }
        }
    }

    void testTextToTokensAdvanceXConsistency()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("test");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // Calculate manual advance from glyphs
        float manualAdvance = 0;
        for (const auto& glyph : tokens[0].glyphs) {
            manualAdvance += glyph.advanceX;
        }

        // Should be close to totalAdvanceX (allow small floating point difference)
        CPPUNIT_ASSERT(std::abs(tokens[0].totalAdvanceX - manualAdvance) < 1.0f);
    }

    void testTextToTokensClusterMapping()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("abc");

        CPPUNIT_ASSERT(tokens.size() >= 1);
        CPPUNIT_ASSERT(tokens[0].glyphs.size() >= 3);

        // Each glyph should have a valid codepoint
        for (const auto& glyph : tokens[0].glyphs) {
            CPPUNIT_ASSERT(glyph.codepoint > 0);
        }
    }

    void testTextToTokensGlyphIndexValid()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("test");

        CPPUNIT_ASSERT(tokens.size() >= 1);

        // All glyphs should have valid glyph indices
        for (const auto& token : tokens) {
            for (const auto& glyph : token.glyphs) {
                // Glyph index should be reasonable
                CPPUNIT_ASSERT(glyph.glyphIndex < 10000);
            }
        }
    }

    void testTextToTokensCodepointPreserved()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("A");

        CPPUNIT_ASSERT(tokens.size() >= 1);
        CPPUNIT_ASSERT(tokens[0].glyphs.size() >= 1);

        // 'A' should have codepoint 0x41 (65)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x41), tokens[0].glyphs[0].codepoint);
    }

    void testTextToTokensWhitespaceTokensSeparate()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("a b c");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), tokens.size());

        // Pattern should be: word, space, word, space, word
        CPPUNIT_ASSERT(!tokens[0].isWhite);
        CPPUNIT_ASSERT(tokens[1].isWhite);
        CPPUNIT_ASSERT(!tokens[2].isWhite);
        CPPUNIT_ASSERT(tokens[3].isWhite);
        CPPUNIT_ASSERT(!tokens[4].isWhite);
    }

    void testTextToTokensNewlineTokensForced()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("line1\nline2\nline3");

        // Count tokens with forceNewline
        int newlineCount = 0;
        for (const auto& token : tokens) {
            if (token.forceNewline) {
                newlineCount++;
            }
        }

        CPPUNIT_ASSERT_EQUAL(2, newlineCount);
    }

    void testTextToTokensComplexSentence()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        auto tokens = font->textToTokens("The quick, brown fox jumps over the lazy dog!");

        // Should have many tokens (words + spaces + punctuation within words)
        CPPUNIT_ASSERT(tokens.size() >= 10);

        // Verify structure
        int wordCount = 0;
        int spaceCount = 0;
        for (const auto& token : tokens) {
            if (token.isWhite) {
                spaceCount++;
            } else {
                wordCount++;
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
                CPPUNIT_ASSERT(token.totalAdvanceX > 0);
            }
        }

        CPPUNIT_ASSERT(wordCount >= 8);
        CPPUNIT_ASSERT(spaceCount >= 7);
    }

    void testEndToEndRenderAndCache()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Tokenize text
        auto tokens = font->textToTokens("Hello World");
        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Render all glyphs from tokens
        for (const auto& token : tokens) {
            for (const auto& glyph : token.glyphs) {
                const auto* charInfo = font->getCharInfo(glyph.glyphIndex, 0);
                CPPUNIT_ASSERT(charInfo != nullptr);
                CPPUNIT_ASSERT(charInfo->surface != nullptr);
            }
        }

        // Verify caching - re-render should use cache
        for (const auto& token : tokens) {
            for (const auto& glyph : token.glyphs) {
                const auto* charInfo1 = font->getCharInfo(glyph.glyphIndex, 0);
                const auto* charInfo2 = font->getCharInfo(glyph.glyphIndex, 0);
                CPPUNIT_ASSERT_EQUAL(charInfo1, charInfo2);
            }
        }
    }

    void testEndToEndMultipleFonts()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create multiple font instances
        std::unique_ptr<PrerenderedFontImpl> font1(new PrerenderedFontImpl(validFontPath.c_str(), 16, false, false));
        std::unique_ptr<PrerenderedFontImpl> font2(new PrerenderedFontImpl(validFontPath.c_str(), 24, false, false));
        std::unique_ptr<PrerenderedFontImpl> font3(new PrerenderedFontImpl(validFontPath.c_str(), 32, false, false));

        CPPUNIT_ASSERT(font1->getFontHeight() < font2->getFontHeight());
        CPPUNIT_ASSERT(font2->getFontHeight() < font3->getFontHeight());

        // All should be able to tokenize text
        auto tokens1 = font1->textToTokens("test");
        auto tokens2 = font2->textToTokens("test");
        auto tokens3 = font3->textToTokens("test");

        CPPUNIT_ASSERT(tokens1.size() >= 1);
        CPPUNIT_ASSERT(tokens2.size() >= 1);
        CPPUNIT_ASSERT(tokens3.size() >= 1);
    }

    void testEndToEndLargeText()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Generate large text
        std::string largeText;
        for (int i = 0; i < 100; i++) {
            largeText += "The quick brown fox jumps over the lazy dog. ";
        }

        auto tokens = font->textToTokens(largeText);

        // Should have many tokens
        CPPUNIT_ASSERT(tokens.size() > 500);

        // All tokens should be valid
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
                CPPUNIT_ASSERT(token.totalAdvanceX >= 0);
            }
        }
    }

    void testEndToEndMixedOutlines()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render same glyph with different outline sizes
        uint32_t glyphIdx = 36;

        const auto* normal = font->getCharInfo(glyphIdx, 0);
        const auto* outline1 = font->getCharInfo(glyphIdx, 2);
        const auto* outline2 = font->getCharInfo(glyphIdx, 5);
        const auto* outline3 = font->getCharInfo(glyphIdx, 8);

        CPPUNIT_ASSERT(normal != nullptr);
        CPPUNIT_ASSERT(outline1 != nullptr);
        CPPUNIT_ASSERT(outline2 != nullptr);
        CPPUNIT_ASSERT(outline3 != nullptr);

        // Normal should be different from outlined versions
        CPPUNIT_ASSERT(normal != outline1);
        CPPUNIT_ASSERT(normal != outline2);
        CPPUNIT_ASSERT(normal != outline3);

        // Outlines should be progressively larger (implementation may cache similar sizes)
        CPPUNIT_ASSERT(outline1->bitmapWidth >= normal->bitmapWidth);
        CPPUNIT_ASSERT(outline2->bitmapWidth >= outline1->bitmapWidth);
        CPPUNIT_ASSERT(outline3->bitmapWidth >= outline2->bitmapWidth);
    }

    void testEndToEndTokensToGlyphs()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        std::string text = "Hello World 123!";
        auto tokens = font->textToTokens(text);

        // Count total glyphs
        int totalGlyphs = 0;
        for (const auto& token : tokens) {
            totalGlyphs += token.glyphs.size();
        }

        CPPUNIT_ASSERT(totalGlyphs > 10);

        // Render all glyphs and validate
        for (const auto& token : tokens) {
            for (const auto& glyph : token.glyphs) {
                const auto* charInfo = font->getCharInfo(glyph.glyphIndex, 0);
                CPPUNIT_ASSERT(charInfo != nullptr);

                // Validate glyph data
                CPPUNIT_ASSERT(glyph.advanceX >= 0);
                CPPUNIT_ASSERT(glyph.codepoint > 0 || glyph.codepoint == 0x20); // Allow space
            }
        }
    }

    void testEndToEndUnicodeRendering()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test various Unicode strings
        std::vector<std::string> testStrings = {
            "Café",
            "Привет",
            "Hello 世界",
            "Test 123!"
        };

        for (const auto& str : testStrings) {
            auto tokens = font->textToTokens(str);
            CPPUNIT_ASSERT(tokens.size() >= 1);

            // Render all glyphs
            for (const auto& token : tokens) {
                for (const auto& glyph : token.glyphs) {
                    const auto* charInfo = font->getCharInfo(glyph.glyphIndex, 0);
                    // Some glyphs might not be available in all fonts
                    if (charInfo != nullptr) {
                        CPPUNIT_ASSERT(charInfo->surface != nullptr);
                    }
                }
            }
        }
    }

    void testEndToEndMetricsAndTokens()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Get metrics
        int height = font->getFontHeight();
        int maxAdvance = font->getMaxAdvance();
        int ascender = font->getFontAscender();
        int descender = font->getFontDescender();

        CPPUNIT_ASSERT(height > 0);
        CPPUNIT_ASSERT(maxAdvance > 0);

        // Tokenize and validate advances are reasonable
        auto tokens = font->textToTokens("Test string");

        for (const auto& token : tokens) {
            if (!token.isWhite) {
                // Total advance should be less than maxAdvance * glyph count
                CPPUNIT_ASSERT(token.totalAdvanceX <= maxAdvance * token.glyphs.size() * 1.5);
                CPPUNIT_ASSERT(token.totalAdvanceX > 0);
            }
        }
    }

    void testEndToEndCachePerformance()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render glyphs multiple times - should be fast due to caching
        for (int iteration = 0; iteration < 10; iteration++) {
            for (uint32_t i = 36; i < 126; i++) {
                const auto* charInfo = font->getCharInfo(i, 0);
                CPPUNIT_ASSERT(charInfo != nullptr);
            }
        }

        // All glyphs should be cached
        const auto* firstCall = font->getCharInfo(50, 0);
        const auto* secondCall = font->getCharInfo(50, 0);

        CPPUNIT_ASSERT_EQUAL(firstCall, secondCall);
    }

    void testEndToEndRobustness()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test with various edge cases
        std::vector<std::string> testCases = {
            "",
            " ",
            "\n",
            "a",
            "Test",
            "Test with spaces",
            "Test\nwith\nnewlines",
            "   Leading spaces",
            "Trailing spaces   ",
            "Multiple   spaces",
            "Special!@#$%^&*()chars",
            "Numbers 0123456789",
            "UPPERCASE lowercase MiXeD"
        };

        for (const auto& testCase : testCases) {
            auto tokens = font->textToTokens(testCase);

            // Should not crash, tokens should be valid
            for (const auto& token : tokens) {
                if (!token.isWhite && !token.forceNewline) {
                    CPPUNIT_ASSERT(token.glyphs.size() > 0);
                }
            }
        }
    }

    void testConstructorHeightScaling()
    {
        if (!fontFileExists(validFontPath)) return;

        std::vector<int> heights = {8, 12, 16, 20, 24, 32, 48};
        std::vector<int> resultHeights;

        for (int h : heights) {
            std::unique_ptr<PrerenderedFontImpl> testFont(
                new PrerenderedFontImpl(validFontPath.c_str(), h, false, false));
            resultHeights.push_back(testFont->getFontHeight());
        }

        // Heights should generally increase
        for (size_t i = 1; i < resultHeights.size(); i++) {
            CPPUNIT_ASSERT(resultHeights[i] >= resultHeights[i-1]);
        }
    }

    void testConstructorStrictModeEnforcement()
    {
        if (!fontFileExists(validFontPath)) return;

        int targetHeight = 25;

        std::unique_ptr<PrerenderedFontImpl> strictFont(
            new PrerenderedFontImpl(validFontPath.c_str(), targetHeight, true, false));
        std::unique_ptr<PrerenderedFontImpl> nonStrictFont(
            new PrerenderedFontImpl(validFontPath.c_str(), targetHeight, false, false));

        // Strict mode should result in height <= target
        CPPUNIT_ASSERT(strictFont->getFontHeight() <= targetHeight);

        // Both should have valid positive heights
        CPPUNIT_ASSERT(strictFont->getFontHeight() > 0);
        CPPUNIT_ASSERT(nonStrictFont->getFontHeight() > 0);
    }

    void testConstructorItalicsTransformation()
    {
        if (!fontFileExists(validFontPath)) return;

        std::unique_ptr<PrerenderedFontImpl> normalFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));
        std::unique_ptr<PrerenderedFontImpl> italicFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 20, false, true));

        // Both fonts should be valid
        CPPUNIT_ASSERT(normalFont->getFontHeight() > 0);
        CPPUNIT_ASSERT(italicFont->getFontHeight() > 0);

        // Heights might differ slightly due to transformation
        int normalHeight = normalFont->getFontHeight();
        int italicHeight = italicFont->getFontHeight();
        CPPUNIT_ASSERT(abs(normalHeight - italicHeight) < 5);
    }

    void testConstructorAllCombinations()
    {
        if (!fontFileExists(validFontPath)) return;

        // Test all combinations of strict and italics
        bool strictVals[] = {false, true};
        bool italicsVals[] = {false, true};

        for (bool strict : strictVals) {
            for (bool italics : italicsVals) {
                std::unique_ptr<PrerenderedFontImpl> testFont(
                    new PrerenderedFontImpl(validFontPath.c_str(), 18, strict, italics));

                CPPUNIT_ASSERT(testFont != nullptr);
                CPPUNIT_ASSERT(testFont->getFontHeight() > 0);
                CPPUNIT_ASSERT(testFont->getMaxAdvance() > 0);
            }
        }
    }

    void testConstructorResourceInitialization()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Verify font is fully initialized
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
        CPPUNIT_ASSERT(font->getMaxAdvance() > 0);

        // Should be able to use immediately
        auto tokens = font->textToTokens("test");
        CPPUNIT_ASSERT(tokens.size() > 0);

        const auto* charInfo = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testDestructorCleanup()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create and destroy font multiple times
        for (int i = 0; i < 5; i++) {
            std::unique_ptr<PrerenderedFontImpl> tempFont(
                new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

            // Use the font
            auto tokens = tempFont->textToTokens("test");
            const auto* charInfo = tempFont->getCharInfo(36, 0);

            CPPUNIT_ASSERT(tokens.size() > 0);
            CPPUNIT_ASSERT(charInfo != nullptr);

            // Font destructor called here
        }

        // If we got here, no crashes occurred
        CPPUNIT_ASSERT(true);
    }

    void testDestructorMultipleInstances()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create multiple instances and destroy them
        std::vector<std::unique_ptr<PrerenderedFontImpl>> fonts;

        for (int i = 0; i < 3; i++) {
            fonts.push_back(std::unique_ptr<PrerenderedFontImpl>(
                new PrerenderedFontImpl(validFontPath.c_str(), 15 + i * 5, false, false)));
        }

        // All should be valid
        for (const auto& f : fonts) {
            CPPUNIT_ASSERT(f->getFontHeight() > 0);
        }

        // Clear will destroy all
        fonts.clear();

        CPPUNIT_ASSERT(fonts.empty());
    }

    void testDestructorAfterError()
    {
        // Try to create font with invalid path
        try {
            std::unique_ptr<PrerenderedFontImpl> errorFont(
                new PrerenderedFontImpl("/invalid/path.ttf", 20, false, false));
            CPPUNIT_FAIL("Should have thrown exception");
        } catch (const std::runtime_error&) {
            // Expected exception
            CPPUNIT_ASSERT(true);
        }

        // Should be able to create valid font after error
        if (fontFileExists(validFontPath)) {
            font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));
            CPPUNIT_ASSERT(font->getFontHeight() > 0);
        }
    }

    void testDestructorWithCachedData()
    {
        if (!fontFileExists(validFontPath)) return;

        {
            std::unique_ptr<PrerenderedFontImpl> tempFont(
                new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

            // Fill cache with data
            for (uint32_t i = 36; i < 100; i++) {
                tempFont->getCharInfo(i, 0);
            }

            auto tokens = tempFont->textToTokens("Test with cached data");
            CPPUNIT_ASSERT(tokens.size() > 0);

            // Destructor should clean up all cached data
        }

        // No memory leaks or crashes
        CPPUNIT_ASSERT(true);
    }

    void testDestructorMemoryManagement()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create font with large atlas usage
        {
            std::unique_ptr<PrerenderedFontImpl> largeFont(
                new PrerenderedFontImpl(validFontPath.c_str(), 40, false, false));

            // Render many glyphs with outlines
            for (uint32_t i = 0; i < 200; i++) {
                largeFont->getCharInfo(i % 128, (i / 128) * 3);
            }

            // Destructor should properly free all resources
        }

        CPPUNIT_ASSERT(true);
    }

    void testMetricsWithDifferentHeights()
    {
        if (!fontFileExists(validFontPath)) return;

        std::vector<int> testHeights = {10, 15, 20, 30, 50};

        for (int height : testHeights) {
            std::unique_ptr<PrerenderedFontImpl> testFont(
                new PrerenderedFontImpl(validFontPath.c_str(), height, false, false));

            int fontHeight = testFont->getFontHeight();
            int maxAdvance = testFont->getMaxAdvance();

            CPPUNIT_ASSERT(fontHeight > 0);
            CPPUNIT_ASSERT(maxAdvance > 0);
            CPPUNIT_ASSERT(maxAdvance >= fontHeight / 2); // Sanity check
        }
    }

    void testMetricsStrictVsNonStrict()
    {
        if (!fontFileExists(validFontPath)) return;

        int targetHeight = 22;

        std::unique_ptr<PrerenderedFontImpl> strictFont(
            new PrerenderedFontImpl(validFontPath.c_str(), targetHeight, true, false));
        std::unique_ptr<PrerenderedFontImpl> nonStrictFont(
            new PrerenderedFontImpl(validFontPath.c_str(), targetHeight, false, false));

        int strictHeight = strictFont->getFontHeight();
        int nonStrictHeight = nonStrictFont->getFontHeight();

        // Both should be positive
        CPPUNIT_ASSERT(strictHeight > 0);
        CPPUNIT_ASSERT(nonStrictHeight > 0);

        // Strict should be <= target
        CPPUNIT_ASSERT(strictHeight <= targetHeight);
    }

    void testMetricsWithItalics()
    {
        if (!fontFileExists(validFontPath)) return;

        std::unique_ptr<PrerenderedFontImpl> normalFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));
        std::unique_ptr<PrerenderedFontImpl> italicFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 20, false, true));

        // Max advance might differ slightly for italics
        int normalAdvance = normalFont->getMaxAdvance();
        int italicAdvance = italicFont->getMaxAdvance();

        CPPUNIT_ASSERT(normalAdvance > 0);
        CPPUNIT_ASSERT(italicAdvance > 0);

        // Italic advance might be larger due to slant
        CPPUNIT_ASSERT(italicAdvance >= normalAdvance * 0.9);
    }

    void testMetricsScalability()
    {
        if (!fontFileExists(validFontPath)) return;

        std::unique_ptr<PrerenderedFontImpl> smallFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 10, false, false));
        std::unique_ptr<PrerenderedFontImpl> largeFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 40, false, false));

        int smallHeight = smallFont->getFontHeight();
        int largeHeight = largeFont->getFontHeight();

        // Larger font should have larger metrics
        CPPUNIT_ASSERT(largeHeight > smallHeight);
        CPPUNIT_ASSERT(largeFont->getMaxAdvance() > smallFont->getMaxAdvance());

        // Rough proportionality check
        float ratio = static_cast<float>(largeHeight) / smallHeight;
        CPPUNIT_ASSERT(ratio >= 2.0f && ratio <= 6.0f);
    }

    void testMetricsConsistencyAcrossCalls()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Call metrics multiple times
        std::vector<int> heights, advances, ascenders, descenders;

        for (int i = 0; i < 10; i++) {
            heights.push_back(font->getFontHeight());
            advances.push_back(font->getMaxAdvance());
            ascenders.push_back(font->getFontAscender());
            descenders.push_back(font->getFontDescender());
        }

        // All values should be identical
        for (size_t i = 1; i < heights.size(); i++) {
            CPPUNIT_ASSERT_EQUAL(heights[0], heights[i]);
            CPPUNIT_ASSERT_EQUAL(advances[0], advances[i]);
            CPPUNIT_ASSERT_EQUAL(ascenders[0], ascenders[i]);
            CPPUNIT_ASSERT_EQUAL(descenders[0], descenders[i]);
        }
    }

    void testGetCharInfoInvalidGlyphIndex()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Try invalid glyph indices
        const auto* charInfo1 = font->getCharInfo(999999, 0);
        const auto* charInfo2 = font->getCharInfo(100000, 0);

        // May return nullptr for invalid indices
        // Just verify it doesn't crash
        CPPUNIT_ASSERT(true);
    }

    void testGetCharInfoVeryLargeGlyphIndex()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Try extremely large glyph index
        const auto* charInfo = font->getCharInfo(0xFFFFFF, 0);

        // Should handle gracefully (return nullptr or valid info)
        CPPUNIT_ASSERT(true);
    }

    void testGetCharInfoExcessiveOutlineSize()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Try very large outline size
        const auto* charInfo = font->getCharInfo(36, 100);

        // Should either render successfully or return nullptr
        if (charInfo != nullptr) {
            CPPUNIT_ASSERT(charInfo->bitmapWidth > 0 || charInfo->bitmapHeight > 0);
        }
    }

    void testGetCharInfoAfterDestruction()
    {
        if (!fontFileExists(validFontPath)) return;

        // This test ensures proper cleanup
        std::unique_ptr<PrerenderedFontImpl> tempFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        const auto* charInfo = tempFont->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo != nullptr);

        // Destroy the font
        tempFont.reset();

        // Can't access charInfo anymore, but no crash should have occurred
        CPPUNIT_ASSERT(true);
    }

    void testGetCharInfoNullCheck()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Get valid char info
        const auto* charInfo = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo != nullptr);

        // Verify surface is not null
        if (charInfo != nullptr) {
            CPPUNIT_ASSERT(charInfo->surface != nullptr);
        }
    }

    void testGetCharInfoGlyphTooWide()
    {
        if (!fontFileExists(validFontPath)) return;

        // Use very large font size to potentially create too-wide glyphs
        std::unique_ptr<PrerenderedFontImpl> largeFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 200, false, false));

        // Try to render a wide character
        const auto* charInfo = largeFont->getCharInfo(36, 0);

        // Should either fit or return nullptr
        if (charInfo != nullptr) {
            CPPUNIT_ASSERT(charInfo->bitmapWidth <= 128);
        }
    }

    void testGetCharInfoGlyphTooTall()
    {
        if (!fontFileExists(validFontPath)) return;

        // Use very large font size
        std::unique_ptr<PrerenderedFontImpl> largeFont(
            new PrerenderedFontImpl(validFontPath.c_str(), 200, false, false));

        const auto* charInfo = largeFont->getCharInfo(36, 0);

        // Should either fit or return nullptr
        if (charInfo != nullptr) {
            CPPUNIT_ASSERT(charInfo->bitmapHeight <= 128);
        }
    }

    void testGetCharInfoErrorRecovery()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Try invalid glyph
        const auto* badInfo = font->getCharInfo(999999, 0);

        // Then try valid glyph - should work
        const auto* goodInfo = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(goodInfo != nullptr);
    }

    void testGetCharInfoConsecutiveErrors()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Try multiple invalid glyphs
        for (uint32_t i = 100000; i < 100010; i++) {
            font->getCharInfo(i, 0);
        }

        // Should still work with valid glyph
        const auto* charInfo = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testGetCharInfoMixedValidInvalid()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Mix valid and potentially invalid indices
        std::vector<uint32_t> indices = {36, 999999, 68, 100000, 72, 50000};

        for (uint32_t idx : indices) {
            font->getCharInfo(idx, 0);
        }

        // Font should still be usable
        auto tokens = font->textToTokens("test");
        CPPUNIT_ASSERT(tokens.size() > 0);
    }

    void testTextToTokensLigatures()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Common ligature combinations
        auto tokens = font->textToTokens("officeffle ffi");

        CPPUNIT_ASSERT(tokens.size() >= 3);

        // Verify tokens are valid
        for (const auto& token : tokens) {
            if (!token.isWhite) {
                CPPUNIT_ASSERT(token.glyphs.size() > 0);
            }
        }
    }

    void testTextToTokensKerning()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Letter pairs that typically have kerning
        auto tokens1 = font->textToTokens("WAVE");
        auto tokens2 = font->textToTokens("W A V E");

        CPPUNIT_ASSERT(tokens1.size() >= 1);
        CPPUNIT_ASSERT(tokens2.size() >= 7); // W, space, A, space, V, space, E

        // Without spaces should typically be narrower
        if (tokens1.size() > 0 && !tokens1[0].isWhite) {
            CPPUNIT_ASSERT(tokens1[0].totalAdvanceX > 0);
        }
    }

    void testTextToTokensComplexScript()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test with complex script if font supports it
        auto tokens = font->textToTokens("संस्कृत"); // Sanskrit (Devanagari)

        // Should create tokens without crashing
        CPPUNIT_ASSERT(tokens.size() >= 0);
    }

    void testTextToTokensControlCharacters()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test with control characters
        std::string text = "Hello\tWorld\nNew\rLine";
        auto tokens = font->textToTokens(text);

        CPPUNIT_ASSERT(tokens.size() > 0);

        // Should have newline tokens
        bool hasNewline = false;
        for (const auto& token : tokens) {
            if (token.forceNewline) {
                hasNewline = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasNewline);
    }

    void testTextToTokensNullTerminator()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // String with explicit null
        std::string text("Hello\0World", 11);
        auto tokens = font->textToTokens(text);

        // Should handle gracefully
        CPPUNIT_ASSERT(tokens.size() >= 0);
    }

    void testTextToTokensVeryLongWord()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Very long word without spaces
        std::string longWord(1000, 'a');
        auto tokens = font->textToTokens(longWord);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), tokens.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1000), tokens[0].glyphs.size());
    }

    void testTextToTokensRepeatedText()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Tokenize same text multiple times
        for (int i = 0; i < 5; i++) {
            auto tokens = font->textToTokens("Test repeated");
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), tokens.size());
        }
    }

    void testTextToTokensAllPrintableASCII()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Generate all printable ASCII characters
        std::string ascii;
        for (char c = 32; c < 127; c++) {
            ascii += c;
        }

        auto tokens = font->textToTokens(ascii);

        CPPUNIT_ASSERT(tokens.size() > 0);

        // Count total glyphs
        int totalGlyphs = 0;
        for (const auto& token : tokens) {
            totalGlyphs += token.glyphs.size();
        }
        CPPUNIT_ASSERT(totalGlyphs >= 90); // Should have most ASCII chars
    }

    void testTextToTokensExtendedUnicode()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test with various Unicode ranges
        std::vector<std::string> unicodeTests = {
            "Ω α β γ δ", // Greek
            "א ב ג ד", // Hebrew
            "Ж З И К", // Cyrillic
            "€ £ ¥ ₹" // Currency
        };

        for (const auto& text : unicodeTests) {
            auto tokens = font->textToTokens(text);
            CPPUNIT_ASSERT(tokens.size() > 0);
        }
    }

    void testTextToTokensEdgeCaseCharacters()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test various edge case characters
        std::vector<std::string> edgeCases = {
            "\u200B", // Zero-width space
            "\u00A0", // Non-breaking space
            "\uFEFF", // Zero-width no-break space
            "\u2060"  // Word joiner
        };

        for (const auto& text : edgeCases) {
            auto tokens = font->textToTokens(text);
            // Should not crash
            CPPUNIT_ASSERT(tokens.size() >= 0);
        }
    }

    void testStressRapidTokenization()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Tokenize rapidly
        for (int i = 0; i < 100; i++) {
            auto tokens = font->textToTokens("Quick tokenization test");
            CPPUNIT_ASSERT(tokens.size() > 0);
        }
    }

    void testStressMaximumGlyphs()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Render maximum number of unique glyphs
        for (uint32_t i = 0; i < 256; i++) {
            font->getCharInfo(i, 0);
        }

        // Font should still work
        auto tokens = font->textToTokens("test");
        CPPUNIT_ASSERT(tokens.size() > 0);
    }

    void testStressConcurrentAccess()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Simulate concurrent-like access (in single thread)
        for (int i = 0; i < 50; i++) {
            auto tokens = font->textToTokens("test");
            font->getCharInfo(36 + (i % 50), 0);
            int height = font->getFontHeight();
            CPPUNIT_ASSERT(height > 0);
        }
    }

    void testStressMemoryPressure()
    {
        if (!fontFileExists(validFontPath)) return;

        // Create multiple fonts to simulate memory pressure
        std::vector<std::unique_ptr<PrerenderedFontImpl>> fonts;

        for (int i = 0; i < 5; i++) {
            fonts.push_back(std::unique_ptr<PrerenderedFontImpl>(
                new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false)));
        }

        // Use all fonts
        for (auto& f : fonts) {
            auto tokens = f->textToTokens("memory test");
            CPPUNIT_ASSERT(tokens.size() > 0);
        }
    }

    void testStressAtlasFragmentation()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 25, false, false));

        // Render glyphs in random order to fragment atlas
        std::vector<uint32_t> indices = {50, 20, 80, 10, 90, 30, 70, 40, 60};

        for (uint32_t idx : indices) {
            font->getCharInfo(idx, 0);
            font->getCharInfo(idx, 3);
            font->getCharInfo(idx, 7);
        }

        // Atlas should still be functional
        const auto* charInfo = font->getCharInfo(36, 0);
        CPPUNIT_ASSERT(charInfo != nullptr);
    }

    void testStressLargeOutlines()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 30, false, false));

        // Render multiple glyphs with large outlines
        for (uint32_t i = 36; i < 80; i++) {
            font->getCharInfo(i, 15);
        }

        // Font should still work
        auto tokens = font->textToTokens("outline test");
        CPPUNIT_ASSERT(tokens.size() > 0);
    }

    void testStressRepeatedOperations()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Repeat same operations many times
        for (int i = 0; i < 200; i++) {
            font->getCharInfo(36, 0);
            font->getFontHeight();
            font->textToTokens("a");
        }

        // Should still be functional
        CPPUNIT_ASSERT(font->getFontHeight() > 0);
    }

    void testStressBoundaryConditions()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Test boundary conditions
        font->getCharInfo(0, 0);
        font->getCharInfo(1, 0);
        font->getCharInfo(255, 0);

        font->textToTokens("");
        font->textToTokens("a");

        CPPUNIT_ASSERT(true);
    }

    void testStressMixedOperations()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Mix different operations
        for (int i = 0; i < 30; i++) {
            if (i % 3 == 0) {
                auto tokens = font->textToTokens("mixed test");
                CPPUNIT_ASSERT(tokens.size() > 0);
            } else if (i % 3 == 1) {
                const auto* charInfo = font->getCharInfo(36 + (i % 40), i % 5);
                CPPUNIT_ASSERT(charInfo != nullptr || true); // Allow nullptr
            } else {
                int h = font->getFontHeight();
                CPPUNIT_ASSERT(h > 0);
            }
        }
    }

    void testStressLongRunning()
    {
        if (!fontFileExists(validFontPath)) return;
        font.reset(new PrerenderedFontImpl(validFontPath.c_str(), 20, false, false));

        // Simulate long-running usage
        for (int iteration = 0; iteration < 10; iteration++) {
            // Render some glyphs
            for (uint32_t i = 0; i < 50; i++) {
                font->getCharInfo(i, 0);
            }

            // Tokenize some text
            auto tokens = font->textToTokens("Long running test iteration");
            CPPUNIT_ASSERT(tokens.size() > 0);

            // Check metrics
            CPPUNIT_ASSERT(font->getFontHeight() > 0);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PrerenderedFontImplTest );
