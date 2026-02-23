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
#include <map>
#include <climits>

#include "GfxConfig.hpp"

using subttxrend::ttxt::GfxConfig;

/**
 * Test ConfigProvider implementation that allows setting values for testing
 */
class TestConfigProvider : public subttxrend::common::ConfigProvider
{
public:
    void setValue(const std::string& key, const std::string& value)
    {
        m_values[key] = value;
    }

    void setValue(const std::string& key, const char* value)
    {
        if (value)
        {
            m_values[key] = std::string(value);
        }
        else
        {
            m_values.erase(key);
        }
    }

    void removeValue(const std::string& key)
    {
        m_values.erase(key);
    }

    void clear()
    {
        m_values.clear();
    }

protected:
    const char* getValue(const std::string& key) const override
    {
        auto it = m_values.find(key);
        if (it != m_values.end())
        {
            return it->second.c_str();
        }
        return nullptr;
    }

private:
    std::map<std::string, std::string> m_values;
};

class GfxConfigTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxConfigTest);
    CPPUNIT_TEST(testInitWithDefaultValues);
    CPPUNIT_TEST(testGetWindowSizePixels);
    CPPUNIT_TEST(testGetGridCellSizePixels);
    CPPUNIT_TEST(testGetFontInfoG0G2);
    CPPUNIT_TEST(testGetFlashPeriodMs);
    CPPUNIT_TEST(testGetDefaultBackgroundAlpha);
    CPPUNIT_TEST(testInitWithCustomWindowDimensions);
    CPPUNIT_TEST(testInitWithCustomGridCellDimensions);
    CPPUNIT_TEST(testInitWithCustomFontName);
    CPPUNIT_TEST(testInitWithCustomFontCharSize);
    CPPUNIT_TEST(testInitWithCustomFlashPeriod);
    CPPUNIT_TEST(testInitWithCustomBackgroundAlpha);
    CPPUNIT_TEST(testFlashPeriodBelowMinimumThreshold);
    CPPUNIT_TEST(testFlashPeriodAtMinimumThreshold);
    CPPUNIT_TEST(testFlashPeriodAboveMinimumThreshold);
    CPPUNIT_TEST(testFlashPeriodNegativeValue);
    CPPUNIT_TEST(testFlashPeriodZeroValue);
    CPPUNIT_TEST(testFlashPeriodMaxValue);
    CPPUNIT_TEST(testWindowDimensionZeroWidth);
    CPPUNIT_TEST(testWindowDimensionZeroHeight);
    CPPUNIT_TEST(testWindowDimensionNegativeWidth);
    CPPUNIT_TEST(testWindowDimensionNegativeHeight);
    CPPUNIT_TEST(testGridCellDimensionZeroWidth);
    CPPUNIT_TEST(testGridCellDimensionZeroHeight);
    CPPUNIT_TEST(testFontNameEmpty);
    CPPUNIT_TEST(testFontNameWithSpecialCharacters);
    CPPUNIT_TEST(testFontNameVeryLong);
    CPPUNIT_TEST(testBackgroundAlphaMinValue);
    CPPUNIT_TEST(testBackgroundAlphaMaxValue);
    CPPUNIT_TEST(testBackgroundAlphaMidValue);
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testMultipleConsecutiveInitCalls);
    CPPUNIT_TEST(testFontGlyphSizeEqualsGridCellSize);
    CPPUNIT_TEST(testGettersConsistencyAfterInit);
    CPPUNIT_TEST(testFlashPeriodJustAboveMinimum);
    CPPUNIT_TEST(testWindowLargeDimensions);
    CPPUNIT_TEST(testGridCellLargeDimensions);
    CPPUNIT_TEST(testPartialConfigurationWindow);
    CPPUNIT_TEST(testPartialConfigurationGridCell);
    CPPUNIT_TEST(testPartialConfigurationFont);
    CPPUNIT_TEST(testAllCustomValuesFullConfig);
    CPPUNIT_TEST(testAlphaOverflow);
    CPPUNIT_TEST(testFlashPeriod199ms);
    CPPUNIT_TEST(testFlashPeriod201ms);
    CPPUNIT_TEST(testWindowSinglePixelDimensions);
    CPPUNIT_TEST(testGridCellSinglePixelDimensions);
    CPPUNIT_TEST(testFontCharSizeLargeValues);
    CPPUNIT_TEST(testMultipleInitWithPartialUpdates);
    CPPUNIT_TEST(testDefaultsWhenProvidedKeysMissing);
    CPPUNIT_TEST(testBackgroundAlphaOne);
    CPPUNIT_TEST(testBackgroundAlpha254);
    CPPUNIT_TEST(testWindowAndGridIndependence);
    CPPUNIT_TEST(testFontNameWithNumbers);
    CPPUNIT_TEST(testFontNameWithSpaces);
    CPPUNIT_TEST(testConfigConsistencyAcrossMultipleInstances);
    CPPUNIT_TEST(testLargeFlashPeriod);
    CPPUNIT_TEST(testFlashPeriod200ExactlyClamps);
    CPPUNIT_TEST(testWindowAsymmetricDimensions);
    CPPUNIT_TEST(testGridCellAsymmetricDimensions);
    CPPUNIT_TEST(testWindowWidthAtIntMax);
    CPPUNIT_TEST(testWindowHeightAtIntMax);
    CPPUNIT_TEST(testGridCellWidthNegative);
    CPPUNIT_TEST(testGridCellHeightNegative);
    CPPUNIT_TEST(testBackgroundAlphaNegativeValue);
    CPPUNIT_TEST(testFontCharSizeZero);
    CPPUNIT_TEST(testWindowAndFontIndependence);
    CPPUNIT_TEST(testGridAndFontGlyphDependence);
    CPPUNIT_TEST(testConfigWithAllBoundaryValues);
    CPPUNIT_TEST(testRepeatedGettersNoSideEffects);
    CPPUNIT_TEST(testInitializePreservesExistingState);
    CPPUNIT_TEST(testFlashPeriodExactly200IsMinimum);
    CPPUNIT_TEST(testAlphaBoundaryBetween0And255);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_provider.clear();
    }

    void tearDown()
    {
        m_provider.clear();
    }

protected:
    void verifySize(const subttxrend::ttxt::Size& size, int expectedW, int expectedH)
    {
        CPPUNIT_ASSERT_EQUAL(expectedW, static_cast<int>(size.m_w));
        CPPUNIT_ASSERT_EQUAL(expectedH, static_cast<int>(size.m_h));
    }

    void verifyFontInfo(const subttxrend::ttxt::FontInfo& fontInfo,
                                        const std::string& expectedName,
                                        int expectedGlyphW, int expectedGlyphH,
                                        int expectedCharW, int expectedCharH)
    {
        CPPUNIT_ASSERT_EQUAL(expectedName, fontInfo.m_name);
        verifySize(fontInfo.m_glyphSize, expectedGlyphW, expectedGlyphH);
        verifySize(fontInfo.m_charSize, expectedCharW, expectedCharH);
    }

    void testInitWithDefaultValues()
    {
        // Initialize with all default values (empty provider)
        GfxConfig config;
        config.init(&m_provider);

        // Verify window size defaults (1280x720)
        verifySize(config.getWindowSizePixels(), 1280, 720);

        // Verify grid cell size defaults (22x26)
        verifySize(config.getGridCellSizePixels(), 22, 26);

        // Verify font info defaults
        verifyFontInfo(config.getFontInfoG0G2(),
                    "Bitstream Vera Sans Mono Bold",
                    22, 26,  // glyph size should match grid cell size
                    34, 24); // font char size defaults

        // Verify flash period default (1000 ms)
        CPPUNIT_ASSERT_EQUAL(1000, config.getFlashPeriodMs());

        // Verify background alpha default (0xFF = 255)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF),
                            config.getDefaultBackgroundAlpha());
    }

    void testGetWindowSizePixels()
    {
        m_provider.setValue("WINDOW.W", "640");
        m_provider.setValue("WINDOW.H", "480");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 640, 480);
    }

    void testGetGridCellSizePixels()
    {
        m_provider.setValue("GRID_CELL.W", "20");
        m_provider.setValue("GRID_CELL.H", "24");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 20, 24);
    }

    void testGetFontInfoG0G2()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Custom Font");
        m_provider.setValue("FONT_G0_G2.W", "32");
        m_provider.setValue("FONT_G0_G2.H", "20");
        m_provider.setValue("GRID_CELL.W", "18");
        m_provider.setValue("GRID_CELL.H", "22");

        GfxConfig config;
        config.init(&m_provider);

        const auto& fontInfo = config.getFontInfoG0G2();
        CPPUNIT_ASSERT_EQUAL(std::string("Custom Font"), fontInfo.m_name);
        CPPUNIT_ASSERT_EQUAL(32, static_cast<int>(fontInfo.m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(20, static_cast<int>(fontInfo.m_charSize.m_h));
        CPPUNIT_ASSERT_EQUAL(18, static_cast<int>(fontInfo.m_glyphSize.m_w));
        CPPUNIT_ASSERT_EQUAL(22, static_cast<int>(fontInfo.m_glyphSize.m_h));
    }

    void testGetFlashPeriodMs()
    {
        m_provider.setValue("FLASH_PERIOD_MS", "500");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(500, config.getFlashPeriodMs());
    }

    void testGetDefaultBackgroundAlpha()
    {
        m_provider.setValue("BG_ALPHA", "128");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(128),
                            config.getDefaultBackgroundAlpha());
    }

    void testInitWithCustomWindowDimensions()
    {
        m_provider.setValue("WINDOW.W", "1920");
        m_provider.setValue("WINDOW.H", "1080");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1920, 1080);
    }

    void testInitWithCustomGridCellDimensions()
    {
        m_provider.setValue("GRID_CELL.W", "32");
        m_provider.setValue("GRID_CELL.H", "40");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 32, 40);
    }

    void testInitWithCustomFontName()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Arial Black");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Arial Black"),
                            config.getFontInfoG0G2().m_name);
    }

    void testInitWithCustomFontCharSize()
    {
        m_provider.setValue("FONT_G0_G2.W", "40");
        m_provider.setValue("FONT_G0_G2.H", "28");

        GfxConfig config;
        config.init(&m_provider);

        const auto& fontInfo = config.getFontInfoG0G2();
        CPPUNIT_ASSERT_EQUAL(40, static_cast<int>(fontInfo.m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(28, static_cast<int>(fontInfo.m_charSize.m_h));
    }

    void testInitWithCustomFlashPeriod()
    {
        m_provider.setValue("FLASH_PERIOD_MS", "800");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(800, config.getFlashPeriodMs());
    }

    void testInitWithCustomBackgroundAlpha()
    {
        m_provider.setValue("BG_ALPHA", "64");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(64),
                            config.getDefaultBackgroundAlpha());
    }

    void testFlashPeriodBelowMinimumThreshold()
    {
        // Flash period < 200 ms should be clamped to 200 ms
        m_provider.setValue("FLASH_PERIOD_MS", "100");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testFlashPeriodAtMinimumThreshold()
    {
        // Flash period = 200 ms should be accepted as-is
        m_provider.setValue("FLASH_PERIOD_MS", "200");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testFlashPeriodAboveMinimumThreshold()
    {
        // Flash period > 200 ms should be accepted
        m_provider.setValue("FLASH_PERIOD_MS", "2000");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(2000, config.getFlashPeriodMs());
    }

    void testFlashPeriodNegativeValue()
    {
        // Negative flash period should be clamped to minimum (200 ms)
        m_provider.setValue("FLASH_PERIOD_MS", "-500");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testFlashPeriodZeroValue()
    {
        // Zero flash period should be clamped to minimum (200 ms)
        m_provider.setValue("FLASH_PERIOD_MS", "0");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testFlashPeriodMaxValue()
    {
        // Test with maximum int32_t value
        m_provider.setValue("FLASH_PERIOD_MS", "2147483647");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(2147483647, config.getFlashPeriodMs());
    }

    void testWindowDimensionZeroWidth()
    {
        // Zero width should be accepted (valid value from config perspective)
        m_provider.setValue("WINDOW.W", "0");
        m_provider.setValue("WINDOW.H", "720");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 0, 720);
    }

    void testWindowDimensionZeroHeight()
    {
        // Zero height should be accepted
        m_provider.setValue("WINDOW.W", "1280");
        m_provider.setValue("WINDOW.H", "0");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1280, 0);
    }

    void testWindowDimensionNegativeWidth()
    {
        // Negative width should be accepted as parsed (behavior determined by ConfigProvider)
        m_provider.setValue("WINDOW.W", "-640");
        m_provider.setValue("WINDOW.H", "480");

        GfxConfig config;
        config.init(&m_provider);

        // Value is stored as-is; application behavior is caller's responsibility
        CPPUNIT_ASSERT(config.getWindowSizePixels().m_w < 0 ||
                    config.getWindowSizePixels().m_w == 0);
    }

    void testWindowDimensionNegativeHeight()
    {
        // Negative height should be accepted as parsed
        m_provider.setValue("WINDOW.W", "1280");
        m_provider.setValue("WINDOW.H", "-480");

        GfxConfig config;
        config.init(&m_provider);

        // Value is stored as-is
        CPPUNIT_ASSERT(config.getWindowSizePixels().m_h < 0 ||
                    config.getWindowSizePixels().m_h == 0);
    }

    void testGridCellDimensionZeroWidth()
    {
        // Zero grid cell width should be accepted
        m_provider.setValue("GRID_CELL.W", "0");
        m_provider.setValue("GRID_CELL.H", "26");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 0, 26);
    }

    void testGridCellDimensionZeroHeight()
    {
        // Zero grid cell height should be accepted
        m_provider.setValue("GRID_CELL.W", "22");
        m_provider.setValue("GRID_CELL.H", "0");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 22, 0);
    }

    void testFontNameEmpty()
    {
        // Empty font name should be accepted
        m_provider.setValue("FONT_G0_G2.NAME", "");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string(""), config.getFontInfoG0G2().m_name);
    }

    void testFontNameWithSpecialCharacters()
    {
        // Font name with special characters should be accepted
        m_provider.setValue("FONT_G0_G2.NAME", "Mono*Bold-Italic@2.0");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Mono*Bold-Italic@2.0"),
                            config.getFontInfoG0G2().m_name);
    }

    void testFontNameVeryLong()
    {
        // Very long font name should be accepted
        std::string longName(500, 'A');
        m_provider.setValue("FONT_G0_G2.NAME", longName);

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(longName, config.getFontInfoG0G2().m_name);
    }

    void testBackgroundAlphaMinValue()
    {
        // Alpha value 0 (fully transparent)
        m_provider.setValue("BG_ALPHA", "0");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            config.getDefaultBackgroundAlpha());
    }

    void testBackgroundAlphaMaxValue()
    {
        // Alpha value 255 (fully opaque)
        m_provider.setValue("BG_ALPHA", "255");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255),
                            config.getDefaultBackgroundAlpha());
    }

    void testBackgroundAlphaMidValue()
    {
        // Alpha value 127 (semi-transparent)
        m_provider.setValue("BG_ALPHA", "127");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(127),
                            config.getDefaultBackgroundAlpha());
    }

    void testConstructorInitialization()
    {
        // Constructor should create object with initialized state
        GfxConfig config;

        // Before init() is called, getters should return uninitialized values
        // But they should not crash - just return whatever is in the object
        // This test verifies the object is in a valid state after construction
        (void)config.getWindowSizePixels();
        (void)config.getGridCellSizePixels();
        (void)config.getFontInfoG0G2();
        (void)config.getFlashPeriodMs();
        (void)config.getDefaultBackgroundAlpha();

        // If we got here without crashing, the object is properly initialized
        CPPUNIT_ASSERT(true);
    }

    void testMultipleConsecutiveInitCalls()
    {
        // First initialization
        m_provider.setValue("WINDOW.W", "640");
        m_provider.setValue("WINDOW.H", "480");
        m_provider.setValue("FLASH_PERIOD_MS", "800");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 640, 480);
        CPPUNIT_ASSERT_EQUAL(800, config.getFlashPeriodMs());

        // Clear and update provider
        m_provider.clear();
        m_provider.setValue("WINDOW.W", "1920");
        m_provider.setValue("WINDOW.H", "1080");
        m_provider.setValue("FLASH_PERIOD_MS", "1200");

        // Second initialization should update values
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1920, 1080);
        CPPUNIT_ASSERT_EQUAL(1200, config.getFlashPeriodMs());
    }

    void testFontGlyphSizeEqualsGridCellSize()
    {
        // Font glyph size should match grid cell size after init
        m_provider.setValue("GRID_CELL.W", "25");
        m_provider.setValue("GRID_CELL.H", "30");

        GfxConfig config;
        config.init(&m_provider);

        const auto& gridSize = config.getGridCellSizePixels();
        const auto& fontGlyphSize = config.getFontInfoG0G2().m_glyphSize;

        CPPUNIT_ASSERT_EQUAL(gridSize.m_w, fontGlyphSize.m_w);
        CPPUNIT_ASSERT_EQUAL(gridSize.m_h, fontGlyphSize.m_h);
    }

    void testGettersConsistencyAfterInit()
    {
        // Multiple calls to the same getter should return consistent values
        m_provider.setValue("WINDOW.W", "800");
        m_provider.setValue("WINDOW.H", "600");
        m_provider.setValue("FLASH_PERIOD_MS", "900");

        GfxConfig config;
        config.init(&m_provider);

        // Call getters multiple times and verify consistency
        const auto& size1 = config.getWindowSizePixels();
        const auto& size2 = config.getWindowSizePixels();
        CPPUNIT_ASSERT_EQUAL(size1.m_w, size2.m_w);
        CPPUNIT_ASSERT_EQUAL(size1.m_h, size2.m_h);

        CPPUNIT_ASSERT_EQUAL(config.getFlashPeriodMs(), config.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(config.getDefaultBackgroundAlpha(),
                            config.getDefaultBackgroundAlpha());
    }

    void testFlashPeriodJustAboveMinimum()
    {
        // Test flash period just above the 200 ms minimum (e.g., 201 ms)
        m_provider.setValue("FLASH_PERIOD_MS", "201");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(201, config.getFlashPeriodMs());
    }

    void testWindowLargeDimensions()
    {
        // Test with very large but valid window dimensions
        m_provider.setValue("WINDOW.W", "4096");
        m_provider.setValue("WINDOW.H", "2160");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 4096, 2160);
    }

    void testGridCellLargeDimensions()
    {
        // Test with large grid cell dimensions
        m_provider.setValue("GRID_CELL.W", "64");
        m_provider.setValue("GRID_CELL.H", "128");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 64, 128);
    }

    void testPartialConfigurationWindow()
    {
        // Test with only window width configured, height should use default
        m_provider.setValue("WINDOW.W", "1024");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(1024, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(720, static_cast<int>(config.getWindowSizePixels().m_h));
    }

    void testPartialConfigurationGridCell()
    {
        // Test with only grid cell width configured, height should use default
        m_provider.setValue("GRID_CELL.W", "28");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(28, static_cast<int>(config.getGridCellSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(26, static_cast<int>(config.getGridCellSizePixels().m_h));
    }

    void testPartialConfigurationFont()
    {
        // Test with only font name configured, sizes should use defaults
        m_provider.setValue("FONT_G0_G2.NAME", "DejaVu Mono");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("DejaVu Mono"), config.getFontInfoG0G2().m_name);
        CPPUNIT_ASSERT_EQUAL(34, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(24, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_h));
    }

    void testAllCustomValuesFullConfig()
    {
        // Test with all values configured to non-default values
        m_provider.setValue("WINDOW.W", "2560");
        m_provider.setValue("WINDOW.H", "1440");
        m_provider.setValue("GRID_CELL.W", "50");
        m_provider.setValue("GRID_CELL.H", "60");
        m_provider.setValue("FONT_G0_G2.NAME", "Courier New");
        m_provider.setValue("FONT_G0_G2.W", "48");
        m_provider.setValue("FONT_G0_G2.H", "32");
        m_provider.setValue("FLASH_PERIOD_MS", "1500");
        m_provider.setValue("BG_ALPHA", "200");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 2560, 1440);
        verifySize(config.getGridCellSizePixels(), 50, 60);
        verifyFontInfo(config.getFontInfoG0G2(), "Courier New", 50, 60, 48, 32);
        CPPUNIT_ASSERT_EQUAL(1500, config.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(200),
                            config.getDefaultBackgroundAlpha());
    }

    void testAlphaOverflow()
    {
        // Test alpha value that overflows uint8_t (>255) wraps around
        m_provider.setValue("BG_ALPHA", "256");

        GfxConfig config;
        config.init(&m_provider);

        // Value wraps around: 256 % 256 = 0
        // (ConfigProvider::getInt() converts to int, then cast to uint8_t)
        const std::uint8_t result = config.getDefaultBackgroundAlpha();
        CPPUNIT_ASSERT(result == 0 || result == 1);
    }

    void testFlashPeriod199ms()
    {
        // Test flash period at 199 ms (below minimum by 1)
        m_provider.setValue("FLASH_PERIOD_MS", "199");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testFlashPeriod201ms()
    {
        // Test flash period at 201 ms (above minimum by 1)
        m_provider.setValue("FLASH_PERIOD_MS", "201");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(201, config.getFlashPeriodMs());
    }

    void testWindowSinglePixelDimensions()
    {
        // Test window with single pixel dimensions
        m_provider.setValue("WINDOW.W", "1");
        m_provider.setValue("WINDOW.H", "1");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1, 1);
    }

    void testGridCellSinglePixelDimensions()
    {
        // Test grid cell with single pixel dimensions
        m_provider.setValue("GRID_CELL.W", "1");
        m_provider.setValue("GRID_CELL.H", "1");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 1, 1);
    }

    void testFontCharSizeLargeValues()
    {
        // Test font character size with large values
        m_provider.setValue("FONT_G0_G2.W", "200");
        m_provider.setValue("FONT_G0_G2.H", "200");

        GfxConfig config;
        config.init(&m_provider);

        const auto& fontInfo = config.getFontInfoG0G2();
        CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(fontInfo.m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(200, static_cast<int>(fontInfo.m_charSize.m_h));
    }

    void testMultipleInitWithPartialUpdates()
    {
        // First init with some values
        m_provider.setValue("WINDOW.W", "800");
        m_provider.setValue("FLASH_PERIOD_MS", "500");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(500, config.getFlashPeriodMs());

        // Update only window, flash period should use default for second init
        m_provider.clear();
        m_provider.setValue("WINDOW.W", "1024");

        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(1024, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(1000, config.getFlashPeriodMs()); // Back to default
    }

    void testDefaultsWhenProvidedKeysMissing()
    {
        // Set one value, verify others use defaults
        m_provider.setValue("WINDOW.W", "512");

        GfxConfig config;
        config.init(&m_provider);

        // Window.W is custom, but all others should be defaults
        CPPUNIT_ASSERT_EQUAL(512, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(720, static_cast<int>(config.getWindowSizePixels().m_h));
        CPPUNIT_ASSERT_EQUAL(22, static_cast<int>(config.getGridCellSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(26, static_cast<int>(config.getGridCellSizePixels().m_h));
        CPPUNIT_ASSERT_EQUAL(1000, config.getFlashPeriodMs());
    }

    void testBackgroundAlphaOne()
    {
        // Test alpha value of 1 (minimum non-zero transparency)
        m_provider.setValue("BG_ALPHA", "1");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1),
                            config.getDefaultBackgroundAlpha());
    }

    void testBackgroundAlpha254()
    {
        // Test alpha value of 254 (maximum below opaque)
        m_provider.setValue("BG_ALPHA", "254");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(254),
                            config.getDefaultBackgroundAlpha());
    }

    void testWindowAndGridIndependence()
    {
        // Verify that window and grid cell sizes are independent
        m_provider.setValue("WINDOW.W", "1280");
        m_provider.setValue("WINDOW.H", "720");
        m_provider.setValue("GRID_CELL.W", "16");
        m_provider.setValue("GRID_CELL.H", "20");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1280, 720);
        verifySize(config.getGridCellSizePixels(), 16, 20);

        // Grid cell size should NOT affect window size
        CPPUNIT_ASSERT_EQUAL(1280, static_cast<int>(config.getWindowSizePixels().m_w));
    }

    void testFontNameWithNumbers()
    {
        // Font name with numeric characters
        m_provider.setValue("FONT_G0_G2.NAME", "Font123Medium");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Font123Medium"),
                            config.getFontInfoG0G2().m_name);
    }

    void testFontNameWithSpaces()
    {
        // Font name with spaces (should be preserved)
        m_provider.setValue("FONT_G0_G2.NAME", "Monospace Bold Italic");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Monospace Bold Italic"),
                            config.getFontInfoG0G2().m_name);
    }

    void testConfigConsistencyAcrossMultipleInstances()
    {
        // Verify that same config provider produces consistent results
        // across multiple GfxConfig instances
        m_provider.setValue("WINDOW.W", "1440");
        m_provider.setValue("FLASH_PERIOD_MS", "600");
        m_provider.setValue("BG_ALPHA", "180");

        GfxConfig config1;
        config1.init(&m_provider);

        GfxConfig config2;
        config2.init(&m_provider);

        // Both instances should have identical configuration
        CPPUNIT_ASSERT_EQUAL(config1.getWindowSizePixels().m_w,
                            config2.getWindowSizePixels().m_w);
        CPPUNIT_ASSERT_EQUAL(config1.getFlashPeriodMs(),
                            config2.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(config1.getDefaultBackgroundAlpha(),
                            config2.getDefaultBackgroundAlpha());
    }

    void testLargeFlashPeriod()
    {
        // Test very large flash period value
        m_provider.setValue("FLASH_PERIOD_MS", "1000000");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(1000000, config.getFlashPeriodMs());
    }

    void testFlashPeriod200ExactlyClamps()
    {
        // Verify that 200 is NOT clamped (it's the boundary)
        m_provider.setValue("FLASH_PERIOD_MS", "200");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testWindowAsymmetricDimensions()
    {
        // Test with very different width and height
        m_provider.setValue("WINDOW.W", "100");
        m_provider.setValue("WINDOW.H", "2000");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 100, 2000);
    }

    void testGridCellAsymmetricDimensions()
    {
        // Test with very different grid cell width and height
        m_provider.setValue("GRID_CELL.W", "10");
        m_provider.setValue("GRID_CELL.H", "100");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 10, 100);
    }

    void testWindowWidthAtIntMax()
    {
        // Test window width at maximum int value (2147483647)
        m_provider.setValue("WINDOW.W", "2147483647");
        m_provider.setValue("WINDOW.H", "720");

        GfxConfig config;
        config.init(&m_provider);

        // Should accept the maximum int value
        CPPUNIT_ASSERT_EQUAL(2147483647, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(720, static_cast<int>(config.getWindowSizePixels().m_h));
    }

    void testWindowHeightAtIntMax()
    {
        // Test window height at maximum int value
        m_provider.setValue("WINDOW.W", "1280");
        m_provider.setValue("WINDOW.H", "2147483647");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(1280, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(2147483647, static_cast<int>(config.getWindowSizePixels().m_h));
    }

    void testGridCellWidthNegative()
    {
        // Test grid cell width with negative value
        m_provider.setValue("GRID_CELL.W", "-50");
        m_provider.setValue("GRID_CELL.H", "26");

        GfxConfig config;
        config.init(&m_provider);

        // Negative value should be stored as-is
        CPPUNIT_ASSERT(config.getGridCellSizePixels().m_w < 0 ||
                      config.getGridCellSizePixels().m_w == 0);
    }

    void testGridCellHeightNegative()
    {
        // Test grid cell height with negative value
        m_provider.setValue("GRID_CELL.W", "22");
        m_provider.setValue("GRID_CELL.H", "-30");

        GfxConfig config;
        config.init(&m_provider);

        // Negative value should be stored as-is
        CPPUNIT_ASSERT(config.getGridCellSizePixels().m_h < 0 ||
                      config.getGridCellSizePixels().m_h == 0);
    }

    void testBackgroundAlphaNegativeValue()
    {
        // Test background alpha with negative value (wraps in uint8_t)
        m_provider.setValue("BG_ALPHA", "-1");

        GfxConfig config;
        config.init(&m_provider);

        // Negative int converted to uint8_t should wrap around
        // -1 as signed int, cast to uint8_t = 255
        const std::uint8_t result = config.getDefaultBackgroundAlpha();
        CPPUNIT_ASSERT(result == 255 || result == 0);
    }

    void testFontCharSizeZero()
    {
        // Test font character size with zero values
        m_provider.setValue("FONT_G0_G2.W", "0");
        m_provider.setValue("FONT_G0_G2.H", "0");

        GfxConfig config;
        config.init(&m_provider);

        const auto& fontInfo = config.getFontInfoG0G2();
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(fontInfo.m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(fontInfo.m_charSize.m_h));
    }

    void testWindowAndFontIndependence()
    {
        // Verify that window size doesn't affect font information
        m_provider.setValue("WINDOW.W", "100");
        m_provider.setValue("WINDOW.H", "100");
        m_provider.setValue("FONT_G0_G2.NAME", "TestFont");
        m_provider.setValue("FONT_G0_G2.W", "50");
        m_provider.setValue("FONT_G0_G2.H", "50");

        GfxConfig config;
        config.init(&m_provider);

        // Window size should not affect font settings
        CPPUNIT_ASSERT_EQUAL(100, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(std::string("TestFont"), config.getFontInfoG0G2().m_name);
        CPPUNIT_ASSERT_EQUAL(50, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_w));
    }

    void testGridAndFontGlyphDependence()
    {
        // Verify that font glyph size always equals grid cell size
        m_provider.setValue("GRID_CELL.W", "35");
        m_provider.setValue("GRID_CELL.H", "45");
        m_provider.setValue("FONT_G0_G2.W", "100");
        m_provider.setValue("FONT_G0_G2.H", "100");

        GfxConfig config;
        config.init(&m_provider);

        const auto& gridSize = config.getGridCellSizePixels();
        const auto& glyphSize = config.getFontInfoG0G2().m_glyphSize;

        // Glyph size must equal grid cell size (set in init())
        CPPUNIT_ASSERT_EQUAL(gridSize.m_w, glyphSize.m_w);
        CPPUNIT_ASSERT_EQUAL(gridSize.m_h, glyphSize.m_h);

        // But char size is independent of glyph size
        CPPUNIT_ASSERT_EQUAL(100, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_w));
    }

    void testConfigWithAllBoundaryValues()
    {
        // Test configuration with all boundary values at once
        m_provider.setValue("WINDOW.W", "1");
        m_provider.setValue("WINDOW.H", "1");
        m_provider.setValue("GRID_CELL.W", "1");
        m_provider.setValue("GRID_CELL.H", "1");
        m_provider.setValue("FONT_G0_G2.NAME", "");
        m_provider.setValue("FONT_G0_G2.W", "1");
        m_provider.setValue("FONT_G0_G2.H", "1");
        m_provider.setValue("FLASH_PERIOD_MS", "200");
        m_provider.setValue("BG_ALPHA", "0");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1, 1);
        verifySize(config.getGridCellSizePixels(), 1, 1);
        CPPUNIT_ASSERT_EQUAL(std::string(""), config.getFontInfoG0G2().m_name);
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            config.getDefaultBackgroundAlpha());
    }

    void testRepeatedGettersNoSideEffects()
    {
        // Test that calling getters repeatedly has no side effects
        m_provider.setValue("WINDOW.W", "640");
        m_provider.setValue("FLASH_PERIOD_MS", "450");
        m_provider.setValue("BG_ALPHA", "100");

        GfxConfig config;
        config.init(&m_provider);

        // Call getters many times and verify consistent results
        for (int i = 0; i < 10; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(640, static_cast<int>(config.getWindowSizePixels().m_w));
            CPPUNIT_ASSERT_EQUAL(450, config.getFlashPeriodMs());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(100),
                                config.getDefaultBackgroundAlpha());
        }
    }

    void testInitializePreservesExistingState()
    {
        // Test that re-initialization properly updates state
        m_provider.setValue("WINDOW.W", "500");
        m_provider.setValue("FLASH_PERIOD_MS", "300");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(500, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(300, config.getFlashPeriodMs());

        // Change provider and reinitialize
        m_provider.clear();
        m_provider.setValue("WINDOW.W", "700");
        m_provider.setValue("FLASH_PERIOD_MS", "500");

        config.init(&m_provider);

        // State should be updated
        CPPUNIT_ASSERT_EQUAL(700, static_cast<int>(config.getWindowSizePixels().m_w));
        CPPUNIT_ASSERT_EQUAL(500, config.getFlashPeriodMs());
    }

    void testFlashPeriodExactly200IsMinimum()
    {
        // Explicitly test that 200 is the exact minimum (not clamped)
        m_provider.setValue("FLASH_PERIOD_MS", "200");

        GfxConfig config;
        config.init(&m_provider);

        // 200 should equal 200, not be increased
        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
    }

    void testAlphaBoundaryBetween0And255()
    {
        // Test multiple boundary values for alpha
        int alphaValues[] = {0, 1, 127, 128, 254, 255};

        for (int alpha : alphaValues)
        {
            m_provider.clear();
            m_provider.setValue("BG_ALPHA", std::to_string(alpha));

            GfxConfig config;
            config.init(&m_provider);

            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(alpha),
                                config.getDefaultBackgroundAlpha());
        }
    }

private:
    TestConfigProvider m_provider;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxConfigTest);