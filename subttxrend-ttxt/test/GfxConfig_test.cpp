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
    CPPUNIT_TEST(testGetFontInfoG0G2);
    CPPUNIT_TEST(testInitWithCustomWindowDimensions);
    CPPUNIT_TEST(testInitWithCustomGridCellDimensions);
    CPPUNIT_TEST(testInitWithCustomFontName);
    CPPUNIT_TEST(testInitWithCustomFlashPeriod);
    CPPUNIT_TEST(testInitWithCustomBackgroundAlpha);
    CPPUNIT_TEST(testFlashPeriodBelowMinimumThreshold);
    CPPUNIT_TEST(testFlashPeriodAtMinimumThreshold);
    CPPUNIT_TEST(testFlashPeriodAboveMinimumThreshold);
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
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testFlashPeriodJustAboveMinimum);
    CPPUNIT_TEST(testPartialConfigurationWindow);
    CPPUNIT_TEST(testPartialConfigurationGridCell);
    CPPUNIT_TEST(testPartialConfigurationFont);
    CPPUNIT_TEST(testInvalidWindowFallback);
    CPPUNIT_TEST(testInvalidGridFallback);
    CPPUNIT_TEST(testInvalidFontSizeFallback);
    CPPUNIT_TEST(testInvalidFlashFallback);
    CPPUNIT_TEST(testInvalidAlphaFallback);
    CPPUNIT_TEST(testAllCustomValuesFullConfig);
    CPPUNIT_TEST(testAlphaOverflow);
    CPPUNIT_TEST(testMultipleInitWithPartialUpdates);
    CPPUNIT_TEST(testDefaultsWhenProvidedKeysMissing);
    CPPUNIT_TEST(testFontNameWithNumbers);
    CPPUNIT_TEST(testFontNameWithSpaces);
    CPPUNIT_TEST(testBackgroundAlphaNegativeValue);
    CPPUNIT_TEST(testGridAndFontGlyphDependence);
    CPPUNIT_TEST(testConfigWithAllBoundaryValues);
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
        // Negative width should be preserved as parsed.
        m_provider.setValue("WINDOW.W", "-640");
        m_provider.setValue("WINDOW.H", "480");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), -640, 480);
    }

    void testWindowDimensionNegativeHeight()
    {
        // Negative height should be preserved as parsed.
        m_provider.setValue("WINDOW.W", "1280");
        m_provider.setValue("WINDOW.H", "-480");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1280, -480);
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
        // Explicit empty string should be preserved, not replaced by the default font.
        m_provider.setValue("FONT_G0_G2.NAME", "");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string(""), config.getFontInfoG0G2().m_name);
    }

    void testFontNameWithSpecialCharacters()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Mono*Bold-Italic@2.0");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Mono*Bold-Italic@2.0"),
                            config.getFontInfoG0G2().m_name);
    }

    void testFontNameVeryLong()
    {
        std::string longName(500, 'A');
        m_provider.setValue("FONT_G0_G2.NAME", longName);

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(longName, config.getFontInfoG0G2().m_name);
    }

    void testConstructorInitialization()
    {
        GfxConfig config;

        verifySize(config.getWindowSizePixels(), 0, 0);
        verifySize(config.getGridCellSizePixels(), 0, 0);
        verifyFontInfo(config.getFontInfoG0G2(), "", 0, 0, 0, 0);
        CPPUNIT_ASSERT_EQUAL(0, config.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
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

    void testInvalidWindowFallback()
    {
        m_provider.setValue("WINDOW.W", "12px");
        m_provider.setValue("WINDOW.H", "   ");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getWindowSizePixels(), 1280, 720);
    }

    void testInvalidGridFallback()
    {
        m_provider.setValue("GRID_CELL.W", "abc");
        m_provider.setValue("GRID_CELL.H", "99999999999999999999");

        GfxConfig config;
        config.init(&m_provider);

        verifySize(config.getGridCellSizePixels(), 22, 26);
    }

    void testInvalidFontSizeFallback()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Teletext");
        m_provider.setValue("FONT_G0_G2.W", "+");
        m_provider.setValue("FONT_G0_G2.H", "-99999999999999999999");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Teletext"), config.getFontInfoG0G2().m_name);
        CPPUNIT_ASSERT_EQUAL(34, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_w));
        CPPUNIT_ASSERT_EQUAL(24, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_h));
    }

    void testInvalidFlashFallback()
    {
        // Trailing characters make the value invalid, so the default should be used.
        m_provider.setValue("FLASH_PERIOD_MS", "200ms");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(1000, config.getFlashPeriodMs());
    }

    void testInvalidAlphaFallback()
    {
        m_provider.setValue("BG_ALPHA", "99999999999999999999");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF),
                            config.getDefaultBackgroundAlpha());
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
        // Alpha is read as int and stored in uint8_t, so overflow wraps modulo 256.
        m_provider.setValue("BG_ALPHA", "256");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            config.getDefaultBackgroundAlpha());
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

    void testFontNameWithNumbers()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Font123Medium");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Font123Medium"),
                            config.getFontInfoG0G2().m_name);
    }

    void testFontNameWithSpaces()
    {
        m_provider.setValue("FONT_G0_G2.NAME", "Monospace Bold Italic");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(std::string("Monospace Bold Italic"),
                            config.getFontInfoG0G2().m_name);
    }

    void testBackgroundAlphaNegativeValue()
    {
        // Signed values stored in uint8_t wrap modulo 256.
        m_provider.setValue("BG_ALPHA", "-1");

        GfxConfig config;
        config.init(&m_provider);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255),
                            config.getDefaultBackgroundAlpha());
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
        CPPUNIT_ASSERT_EQUAL(100, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_h));
    }

    void testConfigWithAllBoundaryValues()
    {
        // Test a representative minimum-valid configuration across all fields.
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
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(config.getFontInfoG0G2().m_charSize.m_h));
        CPPUNIT_ASSERT_EQUAL(200, config.getFlashPeriodMs());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            config.getDefaultBackgroundAlpha());
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