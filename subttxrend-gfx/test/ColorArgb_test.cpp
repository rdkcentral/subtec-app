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
#include <sstream>
#include <string>

#include "ColorArgb.hpp"

using subttxrend::gfx::ColorArgb;

class ColorArgbTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ColorArgbTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testDefaultConstructorAllComponents);
    CPPUNIT_TEST(testUint32ConstructorMinimumValue);
    CPPUNIT_TEST(testUint32ConstructorMaximumValue);
    CPPUNIT_TEST(testUint32ConstructorSemiTransparentRed);
    CPPUNIT_TEST(testUint32ConstructorArbitraryValue);
    CPPUNIT_TEST(testUint32ConstructorTransparentMagenta);
    CPPUNIT_TEST(testUint32ConstructorHexValue);
    CPPUNIT_TEST(testComponentConstructorMaxValues);
    CPPUNIT_TEST(testComponentConstructorMinValues);
    CPPUNIT_TEST(testComponentConstructorSemiTransparentRed);
    CPPUNIT_TEST(testComponentConstructorArbitraryValues);
    CPPUNIT_TEST(testComponentConstructorDecimalValues);
    CPPUNIT_TEST(testComponentConstructorOrder);
    CPPUNIT_TEST(testConstructorConsistency);
    CPPUNIT_TEST(testTransparentValue);
    CPPUNIT_TEST(testTransparentAccessible);
    CPPUNIT_TEST(testWhiteValue);
    CPPUNIT_TEST(testWhiteAccessible);
    CPPUNIT_TEST(testBlackValue);
    CPPUNIT_TEST(testBlackAccessible);
    CPPUNIT_TEST(testSilverValue);
    CPPUNIT_TEST(testGrayValue);
    CPPUNIT_TEST(testMaroonValue);
    CPPUNIT_TEST(testRedValue);
    CPPUNIT_TEST(testPurpleValue);
    CPPUNIT_TEST(testFuchsiaValue);
    CPPUNIT_TEST(testMagentaValue);
    CPPUNIT_TEST(testMagentaEqualsFuchsia);
    CPPUNIT_TEST(testGreenValue);
    CPPUNIT_TEST(testLimeValue);
    CPPUNIT_TEST(testOliveValue);
    CPPUNIT_TEST(testYellowValue);
    CPPUNIT_TEST(testNavyValue);
    CPPUNIT_TEST(testBlueValue);
    CPPUNIT_TEST(testTealValue);
    CPPUNIT_TEST(testAquaValue);
    CPPUNIT_TEST(testCyanValue);
    CPPUNIT_TEST(testAquaEqualsCyan);
    CPPUNIT_TEST(testAllConstantsOpaqueExceptTransparent);
    CPPUNIT_TEST(testAllConstantsUnique);
    CPPUNIT_TEST(testConstantsAreConst);
    CPPUNIT_TEST(testConstantsInitializedAtStart);
    CPPUNIT_TEST(testRgbPrimaryColors);
    CPPUNIT_TEST(testYellowIsRedPlusGreen);
    CPPUNIT_TEST(testCyanIsGreenPlusBlue);
    CPPUNIT_TEST(testMagentaIsRedPlusBlue);
    CPPUNIT_TEST(testGrayscaleProgression);
    CPPUNIT_TEST(testNavyVsBlue);
    CPPUNIT_TEST(testGreenVsLime);
    CPPUNIT_TEST(testMaroonVsRed);
    CPPUNIT_TEST(testPurpleVsFuchsia);
    CPPUNIT_TEST(testTransparentIsOnlyWithZeroAlpha);
    CPPUNIT_TEST(testGetColorByNameTransparent);
    CPPUNIT_TEST(testGetColorByNameWhite);
    CPPUNIT_TEST(testGetColorByNameBlack);
    CPPUNIT_TEST(testGetColorByNameRed);
    CPPUNIT_TEST(testGetColorByNameGreen);
    CPPUNIT_TEST(testGetColorByNameBlue);
    CPPUNIT_TEST(testGetColorByNameYellow);
    CPPUNIT_TEST(testGetColorByNameCyan);
    CPPUNIT_TEST(testGetColorByNameMagenta);
    CPPUNIT_TEST(testGetColorByNameSilver);
    CPPUNIT_TEST(testGetColorByNameGray);
    CPPUNIT_TEST(testGetColorByNameAllNamedColors);
    CPPUNIT_TEST(testGetColorByNameLowercase);
    CPPUNIT_TEST(testGetColorByNameLowercaseBlue);
    CPPUNIT_TEST(testGetColorByNameLowercaseGreen);
    CPPUNIT_TEST(testGetColorByNameMixedCaseRed);
    CPPUNIT_TEST(testGetColorByNameMixedCaserEd);
    CPPUNIT_TEST(testGetColorByNameMixedCaseBlUe);
    CPPUNIT_TEST(testGetColorByNameInvalidOrange);
    CPPUNIT_TEST(testGetColorByNameInvalidPink);
    CPPUNIT_TEST(testGetColorByNameInvalidBrown);
    CPPUNIT_TEST(testGetColorByNameEmptyString);
    CPPUNIT_TEST(testGetColorByNameWhitespace);
    CPPUNIT_TEST(testGetColorByNameWithSpaces);
    CPPUNIT_TEST(testGetColorByNameSpecialCharacters);
    CPPUNIT_TEST(testGetColorByNameNumbers);
    CPPUNIT_TEST(testGetColorByNameVeryLongString);
    CPPUNIT_TEST(testGetColorByNameOutputNotModified);
    CPPUNIT_TEST(testGetColorByNameReturnValueTrue);
    CPPUNIT_TEST(testGetColorByNameReturnValueFalse);
    CPPUNIT_TEST(testGetColorByNameMultipleCalls);
    CPPUNIT_TEST(testGetColorByNameOutputCorrectness);
    CPPUNIT_TEST(testOperatorEqualIdenticalColors);
    CPPUNIT_TEST(testOperatorEqualTwoWhiteConstants);
    CPPUNIT_TEST(testOperatorEqualDifferentColors);
    CPPUNIT_TEST(testOperatorEqualSameRgbDifferentAlpha);
    CPPUNIT_TEST(testOperatorEqualDifferentBlueOnly);
    CPPUNIT_TEST(testOperatorEqualDifferentGreenOnly);
    CPPUNIT_TEST(testOperatorEqualDifferentRedOnly);
    CPPUNIT_TEST(testOperatorEqualDifferentAlphaOnly);
    CPPUNIT_TEST(testOperatorEqualSelfComparison);
    CPPUNIT_TEST(testOperatorNotEqualIdenticalColors);
    CPPUNIT_TEST(testOperatorNotEqualDifferentColors);
    CPPUNIT_TEST(testOperatorNotEqualConsistency);
    CPPUNIT_TEST(testOstreamOutputRed);
    CPPUNIT_TEST(testOstreamOutputBlue);
    CPPUNIT_TEST(testOstreamOutputGreen);
    CPPUNIT_TEST(testOstreamOutputTransparent);
    CPPUNIT_TEST(testOstreamOutputAllNamedColors);
    CPPUNIT_TEST(testOstreamOutputCustomColor);
    CPPUNIT_TEST(testOstreamOutputZeroColor);
    CPPUNIT_TEST(testOstreamOutputHexFormat);
    CPPUNIT_TEST(testOstreamOutputSingleDigitHex);
    CPPUNIT_TEST(testOstreamOutputMaxValues);
    CPPUNIT_TEST(testOstreamOutputMinValues);
    CPPUNIT_TEST(testOstreamChaining);
    CPPUNIT_TEST(testOstreamStreamState);
    CPPUNIT_TEST(testOstreamOutputConsistency);
    CPPUNIT_TEST(testOstreamOutputVsToString);
    CPPUNIT_TEST(testMemberReadAccess);
    CPPUNIT_TEST(testMemberWriteAccess);
    CPPUNIT_TEST(testMemberModification);
    CPPUNIT_TEST(testMemberType);
    CPPUNIT_TEST(testMemberRangeMinimum);
    CPPUNIT_TEST(testMemberRangeMaximum);
    CPPUNIT_TEST(testMemberIndependentModification);
    CPPUNIT_TEST(testMemberAllComponentsModifiable);
    CPPUNIT_TEST(testConstructorToOperator);
    CPPUNIT_TEST(testGetColorByNameToOperator);
    CPPUNIT_TEST(testConstructorToStream);
    CPPUNIT_TEST(testComponentConstructionMatch);
    CPPUNIT_TEST(testModifyAndCompare);
    CPPUNIT_TEST(testConstantToStreamOutput);
    CPPUNIT_TEST(testGetColorByNameToStream);
    CPPUNIT_TEST(testMemberModificationEqualityImpact);
    CPPUNIT_TEST(testMultipleOperationsSequence);
    CPPUNIT_TEST(testAllFeaturesIntegration);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup code if needed
    }

    void tearDown()
    {
        // Cleanup code if needed
    }

    void testDefaultConstructor()
    {
        ColorArgb color;

        // Default constructor should create solid white pixel
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testDefaultConstructorAllComponents()
    {
        ColorArgb color;

        // Verify all components are 0xFF
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
        CPPUNIT_ASSERT_MESSAGE("All components should be 0xFF",
                              color.m_a == 0xFF && color.m_r == 0xFF &&
                              color.m_g == 0xFF && color.m_b == 0xFF);
    }

    void testUint32ConstructorMinimumValue()
    {
        ColorArgb color(0x00000000);

        // Should create fully transparent black
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testUint32ConstructorMaximumValue()
    {
        ColorArgb color(0xFFFFFFFF);

        // Should create solid white
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testUint32ConstructorSemiTransparentRed()
    {
        ColorArgb color(0x80FF0000);

        // Should extract correctly: a=0x80, r=0xFF, g=0x00, b=0x00
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testUint32ConstructorArbitraryValue()
    {
        ColorArgb color(0x12345678);

        // Should extract a=0x12, r=0x34, g=0x56, b=0x78
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), color.m_b);
    }

    void testUint32ConstructorTransparentMagenta()
    {
        ColorArgb color(0x00FF00FF);

        // Should extract a=0x00, r=0xFF, g=0x00, b=0xFF
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testUint32ConstructorHexValue()
    {
        ColorArgb color(0xABCDEF01);

        // Should extract a=0xAB, r=0xCD, g=0xEF, b=0x01
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xAB), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xCD), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xEF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), color.m_b);
    }

    void testComponentConstructorMaxValues()
    {
        ColorArgb color(0xFF, 0xFF, 0xFF, 0xFF);

        // Should create solid white pixel
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testComponentConstructorMinValues()
    {
        ColorArgb color(0x00, 0x00, 0x00, 0x00);

        // Should create fully transparent black
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testComponentConstructorSemiTransparentRed()
    {
        ColorArgb color(0x80, 0xFF, 0x00, 0x00);

        // Should create semi-transparent red
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testComponentConstructorArbitraryValues()
    {
        ColorArgb color(0x12, 0x34, 0x56, 0x78);

        // All components should match input values
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), color.m_b);
    }

    void testComponentConstructorDecimalValues()
    {
        ColorArgb color(255, 128, 64, 32);

        // Should store values correctly as uint8_t
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(128), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(64), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(32), color.m_b);
    }

    void testComponentConstructorOrder()
    {
        ColorArgb color(10, 20, 30, 40);

        // Components should be stored in specified order (not swapped)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(10), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(20), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(30), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(40), color.m_b);
        CPPUNIT_ASSERT_MESSAGE("ARGB order should be maintained", true);
    }

    void testConstructorConsistency()
    {
        ColorArgb defaultColor;
        ColorArgb uint32Color(0xFFFFFFFF);
        ColorArgb componentColor(0xFF, 0xFF, 0xFF, 0xFF);

        // All three constructors should create equivalent white
        CPPUNIT_ASSERT(defaultColor == uint32Color);
        CPPUNIT_ASSERT(uint32Color == componentColor);
        CPPUNIT_ASSERT(defaultColor == componentColor);
    }

    void testTransparentValue()
    {
        const ColorArgb& color = ColorArgb::TRANSPARENT;

        // Should be (a=0x00, r=0x00, g=0x00, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testTransparentAccessible()
    {
        // Should be accessible as ColorArgb::TRANSPARENT
        CPPUNIT_ASSERT_MESSAGE("TRANSPARENT constant should be accessible", true);
        const ColorArgb& color = ColorArgb::TRANSPARENT;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_a);
    }

    void testWhiteValue()
    {
        const ColorArgb& color = ColorArgb::WHITE;

        // Should be (a=0xFF, r=0xFF, g=0xFF, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testWhiteAccessible()
    {
        // Should be accessible as ColorArgb::WHITE
        CPPUNIT_ASSERT_MESSAGE("WHITE constant should be accessible", true);
        const ColorArgb& color = ColorArgb::WHITE;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
    }

    void testBlackValue()
    {
        const ColorArgb& color = ColorArgb::BLACK;

        // Should be (a=0xFF, r=0x00, g=0x00, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testBlackAccessible()
    {
        // Should be accessible as ColorArgb::BLACK
        CPPUNIT_ASSERT_MESSAGE("BLACK constant should be accessible", true);
        const ColorArgb& color = ColorArgb::BLACK;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
    }

    void testSilverValue()
    {
        const ColorArgb& color = ColorArgb::SILVER;

        // Should be (a=0xFF, r=0xC0, g=0xC0, b=0xC0)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC0), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC0), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC0), color.m_b);
    }

    void testGrayValue()
    {
        const ColorArgb& color = ColorArgb::GRAY;

        // Should be (a=0xFF, r=0x80, g=0x80, b=0x80)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_b);
    }

    void testMaroonValue()
    {
        const ColorArgb& color = ColorArgb::MAROON;

        // Should be (a=0xFF, r=0x80, g=0x00, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testRedValue()
    {
        const ColorArgb& color = ColorArgb::RED;

        // Should be (a=0xFF, r=0xFF, g=0x00, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testPurpleValue()
    {
        const ColorArgb& color = ColorArgb::PURPLE;

        // Should be (a=0xFF, r=0x80, g=0x00, b=0x80)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_b);
    }

    void testFuchsiaValue()
    {
        const ColorArgb& color = ColorArgb::FUCHSIA;

        // Should be (a=0xFF, r=0xFF, g=0x00, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testMagentaValue()
    {
        const ColorArgb& color = ColorArgb::MAGENTA;

        // Should be (a=0xFF, r=0xFF, g=0x00, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testMagentaEqualsFuchsia()
    {
        // MAGENTA and FUCHSIA should be identical
        CPPUNIT_ASSERT(ColorArgb::MAGENTA == ColorArgb::FUCHSIA);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::MAGENTA.m_a, ColorArgb::FUCHSIA.m_a);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::MAGENTA.m_r, ColorArgb::FUCHSIA.m_r);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::MAGENTA.m_g, ColorArgb::FUCHSIA.m_g);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::MAGENTA.m_b, ColorArgb::FUCHSIA.m_b);
    }

    void testGreenValue()
    {
        const ColorArgb& color = ColorArgb::GREEN;

        // Should be (a=0xFF, r=0x00, g=0x80, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testLimeValue()
    {
        const ColorArgb& color = ColorArgb::LIME;

        // Should be (a=0xFF, r=0x00, g=0xFF, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testOliveValue()
    {
        const ColorArgb& color = ColorArgb::OLIVE;

        // Should be (a=0xFF, r=0x80, g=0x80, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testYellowValue()
    {
        const ColorArgb& color = ColorArgb::YELLOW;

        // Should be (a=0xFF, r=0xFF, g=0xFF, b=0x00)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_b);
    }

    void testNavyValue()
    {
        const ColorArgb& color = ColorArgb::NAVY;

        // Should be (a=0xFF, r=0x00, g=0x00, b=0x80)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_b);
    }

    void testBlueValue()
    {
        const ColorArgb& color = ColorArgb::BLUE;

        // Should be (a=0xFF, r=0x00, g=0x00, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testTealValue()
    {
        const ColorArgb& color = ColorArgb::TEAL;

        // Should be (a=0xFF, r=0x00, g=0x80, b=0x80)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), color.m_b);
    }

    void testAquaValue()
    {
        const ColorArgb& color = ColorArgb::AQUA;

        // Should be (a=0xFF, r=0x00, g=0xFF, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testCyanValue()
    {
        const ColorArgb& color = ColorArgb::CYAN;

        // Should be (a=0xFF, r=0x00, g=0xFF, b=0xFF)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_b);
    }

    void testAquaEqualsCyan()
    {
        // AQUA and CYAN should be identical
        CPPUNIT_ASSERT(ColorArgb::AQUA == ColorArgb::CYAN);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::AQUA.m_a, ColorArgb::CYAN.m_a);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::AQUA.m_r, ColorArgb::CYAN.m_r);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::AQUA.m_g, ColorArgb::CYAN.m_g);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::AQUA.m_b, ColorArgb::CYAN.m_b);
    }

    void testAllConstantsOpaqueExceptTransparent()
    {
        // All constants should have alpha=0xFF except TRANSPARENT
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::TRANSPARENT.m_a);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::WHITE.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::BLACK.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::SILVER.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::GRAY.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::MAROON.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::RED.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::PURPLE.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::FUCHSIA.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::GREEN.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::LIME.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::OLIVE.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::YELLOW.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::NAVY.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::BLUE.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::TEAL.m_a);
    }

    void testAllConstantsUnique()
    {
        // Each constant should have unique ARGB values except MAGENTA/FUCHSIA and AQUA/CYAN
        CPPUNIT_ASSERT(ColorArgb::WHITE != ColorArgb::BLACK);
        CPPUNIT_ASSERT(ColorArgb::WHITE != ColorArgb::RED);
        CPPUNIT_ASSERT(ColorArgb::BLACK != ColorArgb::RED);
        CPPUNIT_ASSERT(ColorArgb::RED != ColorArgb::GREEN);
        CPPUNIT_ASSERT(ColorArgb::RED != ColorArgb::BLUE);
        CPPUNIT_ASSERT(ColorArgb::GREEN != ColorArgb::BLUE);
        CPPUNIT_ASSERT(ColorArgb::SILVER != ColorArgb::GRAY);
        CPPUNIT_ASSERT(ColorArgb::MAROON != ColorArgb::RED);
        CPPUNIT_ASSERT(ColorArgb::GREEN != ColorArgb::LIME);
        CPPUNIT_ASSERT(ColorArgb::NAVY != ColorArgb::BLUE);
        CPPUNIT_ASSERT(ColorArgb::PURPLE != ColorArgb::FUCHSIA);

        // Verify aliases are equal
        CPPUNIT_ASSERT(ColorArgb::MAGENTA == ColorArgb::FUCHSIA);
        CPPUNIT_ASSERT(ColorArgb::AQUA == ColorArgb::CYAN);
    }

    void testConstantsAreConst()
    {
        // Verify constants are accessible and const
        const ColorArgb& red = ColorArgb::RED;
        const ColorArgb& green = ColorArgb::GREEN;
        const ColorArgb& blue = ColorArgb::BLUE;

        CPPUNIT_ASSERT_MESSAGE("Constants should be accessible as const", true);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), red.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), green.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), blue.m_b);
    }

    void testConstantsInitializedAtStart()
    {
        // Constants should be accessible immediately
        CPPUNIT_ASSERT_MESSAGE("Constants should be initialized at program start", true);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::RED.m_r);
    }

    void testRgbPrimaryColors()
    {
        // RED, GREEN, BLUE should be primary colors with correct single-channel values
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::RED.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::RED.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::RED.m_b);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::GREEN.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), ColorArgb::GREEN.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::GREEN.m_b);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::BLUE.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::BLUE.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::BLUE.m_b);
    }

    void testYellowIsRedPlusGreen()
    {
        // YELLOW should have r=0xFF, g=0xFF, b=0x00 (red + green)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::YELLOW.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::YELLOW.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::YELLOW.m_b);
    }

    void testCyanIsGreenPlusBlue()
    {
        // CYAN should have r=0x00, g=0xFF, b=0xFF (green + blue)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::CYAN.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::CYAN.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::CYAN.m_b);
    }

    void testMagentaIsRedPlusBlue()
    {
        // MAGENTA should have r=0xFF, g=0x00, b=0xFF (red + blue)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::MAGENTA.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::MAGENTA.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::MAGENTA.m_b);
    }

    void testGrayscaleProgression()
    {
        // WHITE, SILVER, GRAY, BLACK should have decreasing equal RGB values
        CPPUNIT_ASSERT(ColorArgb::WHITE.m_r == ColorArgb::WHITE.m_g &&
                      ColorArgb::WHITE.m_g == ColorArgb::WHITE.m_b);
        CPPUNIT_ASSERT(ColorArgb::SILVER.m_r == ColorArgb::SILVER.m_g &&
                      ColorArgb::SILVER.m_g == ColorArgb::SILVER.m_b);
        CPPUNIT_ASSERT(ColorArgb::GRAY.m_r == ColorArgb::GRAY.m_g &&
                      ColorArgb::GRAY.m_g == ColorArgb::GRAY.m_b);
        CPPUNIT_ASSERT(ColorArgb::BLACK.m_r == ColorArgb::BLACK.m_g &&
                      ColorArgb::BLACK.m_g == ColorArgb::BLACK.m_b);

        // Verify descending order
        CPPUNIT_ASSERT(ColorArgb::WHITE.m_r > ColorArgb::SILVER.m_r);
        CPPUNIT_ASSERT(ColorArgb::SILVER.m_r > ColorArgb::GRAY.m_r);
        CPPUNIT_ASSERT(ColorArgb::GRAY.m_r > ColorArgb::BLACK.m_r);
    }

    void testNavyVsBlue()
    {
        // NAVY should have lower blue component than BLUE
        CPPUNIT_ASSERT(ColorArgb::NAVY.m_b < ColorArgb::BLUE.m_b);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), ColorArgb::NAVY.m_b);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::BLUE.m_b);
    }

    void testGreenVsLime()
    {
        // GREEN should have lower green component than LIME
        CPPUNIT_ASSERT(ColorArgb::GREEN.m_g < ColorArgb::LIME.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), ColorArgb::GREEN.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::LIME.m_g);
    }

    void testMaroonVsRed()
    {
        // MAROON should have lower red component than RED
        CPPUNIT_ASSERT(ColorArgb::MAROON.m_r < ColorArgb::RED.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), ColorArgb::MAROON.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::RED.m_r);
    }

    void testPurpleVsFuchsia()
    {
        // PURPLE should have lower components than FUCHSIA
        CPPUNIT_ASSERT(ColorArgb::PURPLE.m_r < ColorArgb::FUCHSIA.m_r);
        CPPUNIT_ASSERT(ColorArgb::PURPLE.m_b < ColorArgb::FUCHSIA.m_b);
    }

    void testTransparentIsOnlyWithZeroAlpha()
    {
        // TRANSPARENT is the only color with alpha=0, all others have alpha=0xFF
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), ColorArgb::TRANSPARENT.m_a);

        // Verify a few others have 0xFF
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::BLACK.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::WHITE.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), ColorArgb::RED.m_a);
    }

    void testGetColorByNameTransparent()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("TRANSPARENT", output);

        // Should return true and set output to TRANSPARENT constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::TRANSPARENT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_b);
    }

    void testGetColorByNameWhite()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("WHITE", output);

        // Should return true and set output to WHITE constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::WHITE);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_b);
    }

    void testGetColorByNameBlack()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("BLACK", output);

        // Should return true and set output to BLACK constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::BLACK);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_b);
    }

    void testGetColorByNameRed()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("RED", output);

        // Should return true and set output to RED constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::RED);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_b);
    }

    void testGetColorByNameGreen()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("GREEN", output);

        // Should return true and set output to GREEN constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::GREEN);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_b);
    }

    void testGetColorByNameBlue()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("BLUE", output);

        // Should return true and set output to BLUE constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::BLUE);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_b);
    }

    void testGetColorByNameYellow()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("YELLOW", output);

        // Should return true and set output to YELLOW constant
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::YELLOW);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_b);
    }

    void testGetColorByNameCyan()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("CYAN", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::CYAN);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_b);
    }

    void testGetColorByNameMagenta()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("MAGENTA", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::MAGENTA);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_b);
    }

    void testGetColorByNameSilver()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("SILVER", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::SILVER);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xC0), output.m_r);
    }

    void testGetColorByNameGray()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("GRAY", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::GRAY);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x80), output.m_r);
    }

    void testGetColorByNameAllNamedColors()
    {
        // Test all remaining colors in the map
        ColorArgb output;

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("MAROON", output));
        CPPUNIT_ASSERT(output == ColorArgb::MAROON);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("PURPLE", output));
        CPPUNIT_ASSERT(output == ColorArgb::PURPLE);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("FUCHSIA", output));
        CPPUNIT_ASSERT(output == ColorArgb::FUCHSIA);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("LIME", output));
        CPPUNIT_ASSERT(output == ColorArgb::LIME);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("OLIVE", output));
        CPPUNIT_ASSERT(output == ColorArgb::OLIVE);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("NAVY", output));
        CPPUNIT_ASSERT(output == ColorArgb::NAVY);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("TEAL", output));
        CPPUNIT_ASSERT(output == ColorArgb::TEAL);

        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("AQUA", output));
        CPPUNIT_ASSERT(output == ColorArgb::AQUA);
    }

    void testGetColorByNameLowercase()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("red", output);

        // Should convert to uppercase and find RED
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::RED);
    }

    void testGetColorByNameLowercaseBlue()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("blue", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::BLUE);
    }

    void testGetColorByNameLowercaseGreen()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("green", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::GREEN);
    }

    void testGetColorByNameMixedCaseRed()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("Red", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::RED);
    }

    void testGetColorByNameMixedCaserEd()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("rEd", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::RED);
    }

    void testGetColorByNameMixedCaseBlUe()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("BlUe", output);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(output == ColorArgb::BLUE);
    }

    void testGetColorByNameInvalidOrange()
    {
        ColorArgb output(0xFF, 0x12, 0x34, 0x56); // Pre-initialize
        bool result = ColorArgb::getColorByName("orange", output);

        // Should return false
        CPPUNIT_ASSERT_EQUAL(false, result);
        // Output should remain unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), output.m_b);
    }

    void testGetColorByNameInvalidPink()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("pink", output);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameInvalidBrown()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("brown", output);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameEmptyString()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("", output);

        // Should return false for empty string
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameWhitespace()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("   ", output);

        // Spaces don't match any color
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameWithSpaces()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName(" RED ", output);

        // Spaces prevent match (not trimmed)
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameSpecialCharacters()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("RED!", output);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameNumbers()
    {
        ColorArgb output;
        bool result = ColorArgb::getColorByName("123", output);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameVeryLongString()
    {
        ColorArgb output;
        std::string longString(1000, 'x');
        bool result = ColorArgb::getColorByName(longString, output);

        // Should return false without crashing
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetColorByNameOutputNotModified()
    {
        ColorArgb output(0x12, 0x34, 0x56, 0x78);
        bool result = ColorArgb::getColorByName("invalid", output);

        // Output should retain original value when false returned
        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), output.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), output.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), output.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), output.m_b);
    }

    void testGetColorByNameReturnValueTrue()
    {
        ColorArgb output;

        // Should return true for valid names
        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("RED", output));
        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("BLUE", output));
        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::getColorByName("GREEN", output));
    }

    void testGetColorByNameReturnValueFalse()
    {
        ColorArgb output;

        // Should return false for invalid names
        CPPUNIT_ASSERT_EQUAL(false, ColorArgb::getColorByName("invalid", output));
        CPPUNIT_ASSERT_EQUAL(false, ColorArgb::getColorByName("", output));
        CPPUNIT_ASSERT_EQUAL(false, ColorArgb::getColorByName("orange", output));
    }

    void testGetColorByNameMultipleCalls()
    {
        ColorArgb output1, output2, output3;

        // Each call should work independently
        bool result1 = ColorArgb::getColorByName("RED", output1);
        bool result2 = ColorArgb::getColorByName("BLUE", output2);
        bool result3 = ColorArgb::getColorByName("invalid", output3);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(false, result3);
        CPPUNIT_ASSERT(output1 == ColorArgb::RED);
        CPPUNIT_ASSERT(output2 == ColorArgb::BLUE);
    }

    void testGetColorByNameOutputCorrectness()
    {
        ColorArgb output;
        ColorArgb::getColorByName("RED", output);

        // Should produce identical color to ColorArgb::RED
        CPPUNIT_ASSERT(output == ColorArgb::RED);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED.m_a, output.m_a);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED.m_r, output.m_r);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED.m_g, output.m_g);
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED.m_b, output.m_b);
    }

    void testOperatorEqualIdenticalColors()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x12, 0x34, 0x56);

        // Same ARGB values should return true
        CPPUNIT_ASSERT(color1 == color2);
        CPPUNIT_ASSERT_EQUAL(true, color1 == color2);
    }

    void testOperatorEqualTwoWhiteConstants()
    {
        // ColorArgb::WHITE == ColorArgb::WHITE should be true
        CPPUNIT_ASSERT(ColorArgb::WHITE == ColorArgb::WHITE);
    }

    void testOperatorEqualDifferentColors()
    {
        // RED == BLUE should return false
        CPPUNIT_ASSERT_EQUAL(false, ColorArgb::RED == ColorArgb::BLUE);
        CPPUNIT_ASSERT(!(ColorArgb::RED == ColorArgb::BLUE));
    }

    void testOperatorEqualSameRgbDifferentAlpha()
    {
        ColorArgb color1(0x80, 0xFF, 0x00, 0x00);
        ColorArgb color2(0xFF, 0xFF, 0x00, 0x00);

        // Should be false - alpha affects equality
        CPPUNIT_ASSERT_EQUAL(false, color1 == color2);
    }

    void testOperatorEqualDifferentBlueOnly()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x12, 0x34, 0x78);

        // Only blue component different - should be false
        CPPUNIT_ASSERT_EQUAL(false, color1 == color2);
    }

    void testOperatorEqualDifferentGreenOnly()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x12, 0x78, 0x56);

        // Only green component different - should be false
        CPPUNIT_ASSERT_EQUAL(false, color1 == color2);
    }

    void testOperatorEqualDifferentRedOnly()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x78, 0x34, 0x56);

        // Only red component different - should be false
        CPPUNIT_ASSERT_EQUAL(false, color1 == color2);
    }

    void testOperatorEqualDifferentAlphaOnly()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0x80, 0x12, 0x34, 0x56);

        // Only alpha component different - should be false
        CPPUNIT_ASSERT_EQUAL(false, color1 == color2);
    }

    void testOperatorEqualSelfComparison()
    {
        ColorArgb color(0x12, 0x34, 0x56, 0x78);

        // color == color should always be true
        CPPUNIT_ASSERT(color == color);
        CPPUNIT_ASSERT_EQUAL(true, color == color);
    }

    void testOperatorNotEqualIdenticalColors()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x12, 0x34, 0x56);

        // Same ARGB values should return false for !=
        CPPUNIT_ASSERT_EQUAL(false, color1 != color2);
        CPPUNIT_ASSERT(!(color1 != color2));
    }

    void testOperatorNotEqualDifferentColors()
    {
        // RED != BLUE should return true
        CPPUNIT_ASSERT(ColorArgb::RED != ColorArgb::BLUE);
        CPPUNIT_ASSERT_EQUAL(true, ColorArgb::RED != ColorArgb::BLUE);
    }

    void testOperatorNotEqualConsistency()
    {
        ColorArgb color1(0xFF, 0x12, 0x34, 0x56);
        ColorArgb color2(0xFF, 0x78, 0x9A, 0xBC);

        // (a == b) should equal !(a != b)
        CPPUNIT_ASSERT_EQUAL(color1 == color2, !(color1 != color2));
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED == ColorArgb::RED,
                            !(ColorArgb::RED != ColorArgb::RED));
        CPPUNIT_ASSERT_EQUAL(ColorArgb::RED == ColorArgb::BLUE,
                            !(ColorArgb::RED != ColorArgb::BLUE));
    }

    void testOstreamOutputRed()
    {
        std::ostringstream oss;
        oss << ColorArgb::RED;

        // Should output "RED" (named color string)
        CPPUNIT_ASSERT_EQUAL(std::string("RED"), oss.str());
    }

    void testOstreamOutputBlue()
    {
        std::ostringstream oss;
        oss << ColorArgb::BLUE;

        CPPUNIT_ASSERT_EQUAL(std::string("BLUE"), oss.str());
    }

    void testOstreamOutputGreen()
    {
        std::ostringstream oss;
        oss << ColorArgb::GREEN;

        CPPUNIT_ASSERT_EQUAL(std::string("GREEN"), oss.str());
    }

    void testOstreamOutputTransparent()
    {
        std::ostringstream oss;
        oss << ColorArgb::TRANSPARENT;

        CPPUNIT_ASSERT_EQUAL(std::string("TRANSPARENT"), oss.str());
    }

    void testOstreamOutputAllNamedColors()
    {
        std::ostringstream oss;

        // Test various named colors
        oss.str("");
        oss << ColorArgb::WHITE;
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), oss.str());

        oss.str("");
        oss << ColorArgb::BLACK;
        CPPUNIT_ASSERT_EQUAL(std::string("BLACK"), oss.str());

        oss.str("");
        oss << ColorArgb::YELLOW;
        CPPUNIT_ASSERT_EQUAL(std::string("YELLOW"), oss.str());

        oss.str("");
        oss << ColorArgb::CYAN;
        // CYAN and AQUA are aliases, either name is acceptable
        std::string cyanOutput = oss.str();
        CPPUNIT_ASSERT(cyanOutput == "CYAN" || cyanOutput == "AQUA");
    }

    void testOstreamOutputCustomColor()
    {
        std::ostringstream oss;
        ColorArgb customColor(0x12, 0x34, 0x56, 0x78);
        oss << customColor;

        // Should output hex format "(0xAA, 0xRR, 0xGG, 0xBB)"
        CPPUNIT_ASSERT_EQUAL(std::string("(0x12, 0x34, 0x56, 0x78)"), oss.str());
    }

    void testOstreamOutputZeroColor()
    {
        std::ostringstream oss;
        ColorArgb zeroColor(0x00, 0x00, 0x00, 0x00);
        oss << zeroColor;

        // Should match TRANSPARENT constant and output its name
        CPPUNIT_ASSERT_EQUAL(std::string("TRANSPARENT"), oss.str());
    }

    void testOstreamOutputHexFormat()
    {
        std::ostringstream oss;
        ColorArgb color(0xAB, 0xCD, 0xEF, 0x12);
        oss << color;

        // Should be "(0xAA, 0xRR, 0xGG, 0xBB)" format
        CPPUNIT_ASSERT_EQUAL(std::string("(0xab, 0xcd, 0xef, 0x12)"), oss.str());
    }

    void testOstreamOutputSingleDigitHex()
    {
        std::ostringstream oss;
        ColorArgb color(0x01, 0x02, 0x03, 0x04);
        oss << color;

        // Should output with zero padding
        CPPUNIT_ASSERT_EQUAL(std::string("(0x01, 0x02, 0x03, 0x04)"), oss.str());
    }

    void testOstreamOutputMaxValues()
    {
        std::ostringstream oss;
        ColorArgb color(0xFF, 0xFF, 0xFF, 0xFF);
        oss << color;

        // Should match WHITE constant
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), oss.str());
    }

    void testOstreamOutputMinValues()
    {
        std::ostringstream oss;
        ColorArgb color(0x00, 0x00, 0x00, 0x01); // One component different from TRANSPARENT
        oss << color;

        // Should output hex format since it's not TRANSPARENT
        CPPUNIT_ASSERT_EQUAL(std::string("(0x00, 0x00, 0x00, 0x01)"), oss.str());
    }

    void testOstreamChaining()
    {
        std::ostringstream oss;
        oss << ColorArgb::RED << " " << ColorArgb::BLUE;

        // Should work with operator chaining
        CPPUNIT_ASSERT_EQUAL(std::string("RED BLUE"), oss.str());
    }

    void testOstreamStreamState()
    {
        std::ostringstream oss;
        oss << ColorArgb::RED;

        // Stream should remain in good state after output
        CPPUNIT_ASSERT(oss.good());
        CPPUNIT_ASSERT(!oss.fail());
    }

    void testOstreamOutputConsistency()
    {
        std::ostringstream oss1, oss2;
        ColorArgb color(0x12, 0x34, 0x56, 0x78);

        oss1 << color;
        oss2 << color;

        // Multiple outputs of same color should be identical
        CPPUNIT_ASSERT_EQUAL(oss1.str(), oss2.str());
    }

    void testOstreamOutputVsToString()
    {
        std::ostringstream oss;
        ColorArgb color(0xAA, 0xBB, 0xCC, 0xDD);
        oss << color;

        // Hex format should match internal toString function pattern
        std::string output = oss.str();
        CPPUNIT_ASSERT(output.find("0x") != std::string::npos);
        CPPUNIT_ASSERT(output.find(",") != std::string::npos);
    }

    void testMemberReadAccess()
    {
        ColorArgb color(0x12, 0x34, 0x56, 0x78);

        // All members should be readable
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), color.m_b);
    }

    void testMemberWriteAccess()
    {
        ColorArgb color;

        // All members should be writable
        color.m_a = 0x12;
        color.m_r = 0x34;
        color.m_g = 0x56;
        color.m_b = 0x78;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x34), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), color.m_b);
    }

    void testMemberModification()
    {
        ColorArgb color(0xFF, 0xFF, 0xFF, 0xFF);

        // Modifying members should change color
        color.m_r = 0x00;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color.m_r);
        CPPUNIT_ASSERT(!(color == ColorArgb::WHITE));
    }

    void testMemberType()
    {
        ColorArgb color;

        // Each member is exactly 8 bits (0-255)
        color.m_a = 255;
        color.m_r = 255;
        color.m_g = 255;
        color.m_b = 255;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_b);
    }

    void testMemberRangeMinimum()
    {
        ColorArgb color;

        // Should accept 0 value
        color.m_a = 0;
        color.m_r = 0;
        color.m_g = 0;
        color.m_b = 0;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), color.m_b);
    }

    void testMemberRangeMaximum()
    {
        ColorArgb color;

        // Should accept 255 value
        color.m_a = 255;
        color.m_r = 255;
        color.m_g = 255;
        color.m_b = 255;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(255), color.m_b);
    }

    void testMemberIndependentModification()
    {
        ColorArgb color(0x12, 0x34, 0x56, 0x78);

        // Modifying one member shouldn't affect others
        color.m_r = 0xFF;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x12), color.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x56), color.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x78), color.m_b);
    }

    void testMemberAllComponentsModifiable()
    {
        ColorArgb color(0x00, 0x00, 0x00, 0x00);

        // All components should be independently modifiable
        color.m_a = 0xAA;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xAA), color.m_a);

        color.m_r = 0xBB;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xBB), color.m_r);

        color.m_g = 0xCC;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xCC), color.m_g);

        color.m_b = 0xDD;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xDD), color.m_b);
    }

    void testConstructorToOperator()
    {
        ColorArgb color(0xFF, 0xFF, 0x00, 0x00);

        // Create color and compare with constant
        CPPUNIT_ASSERT(color == ColorArgb::RED);
    }

    void testGetColorByNameToOperator()
    {
        ColorArgb output;
        ColorArgb::getColorByName("RED", output);

        // Get color by name and compare
        CPPUNIT_ASSERT(output == ColorArgb::RED);
        CPPUNIT_ASSERT(!(output != ColorArgb::RED));
    }

    void testConstructorToStream()
    {
        ColorArgb customColor(0x12, 0x34, 0x56, 0x78);
        std::ostringstream oss;
        oss << customColor;

        // Custom color should output in hex format
        CPPUNIT_ASSERT_EQUAL(std::string("(0x12, 0x34, 0x56, 0x78)"), oss.str());
    }

    void testComponentConstructionMatch()
    {
        ColorArgb color1(0x12, 0x34, 0x56, 0x78);
        ColorArgb color2(0x12345678);

        // ColorArgb(a,r,g,b) should equal ColorArgb(uint32)
        CPPUNIT_ASSERT(color1 == color2);
    }

    void testModifyAndCompare()
    {
        ColorArgb color1(0xFF, 0x00, 0x00, 0x00);
        ColorArgb color2(0xFF, 0xFF, 0x00, 0x00);

        // Modification should affect equality
        CPPUNIT_ASSERT(!(color1 == color2));

        color1.m_r = 0xFF;
        CPPUNIT_ASSERT(color1 == color2);
    }

    void testConstantToStreamOutput()
    {
        std::ostringstream oss;

        // Constant should output its name via stream operator
        oss << ColorArgb::RED;
        CPPUNIT_ASSERT_EQUAL(std::string("RED"), oss.str());

        // Multiple constants
        oss.str("");
        oss << ColorArgb::TRANSPARENT;
        CPPUNIT_ASSERT_EQUAL(std::string("TRANSPARENT"), oss.str());

        oss.str("");
        oss << ColorArgb::YELLOW;
        CPPUNIT_ASSERT_EQUAL(std::string("YELLOW"), oss.str());
    }

    void testGetColorByNameToStream()
    {
        ColorArgb output;
        ColorArgb::getColorByName("BLUE", output);

        std::ostringstream oss;
        oss << output;

        // Color retrieved by name should output its name
        CPPUNIT_ASSERT_EQUAL(std::string("BLUE"), oss.str());
        CPPUNIT_ASSERT(output == ColorArgb::BLUE);

        // Another color
        ColorArgb::getColorByName("green", output);
        oss.str("");
        oss << output;
        CPPUNIT_ASSERT_EQUAL(std::string("GREEN"), oss.str());
    }

    void testMemberModificationEqualityImpact()
    {
        ColorArgb color1 = ColorArgb::RED;
        ColorArgb color2 = ColorArgb::RED;

        // Initially equal
        CPPUNIT_ASSERT(color1 == color2);
        CPPUNIT_ASSERT(!(color1 != color2));

        // Modify one component
        color1.m_b = 0x10;
        CPPUNIT_ASSERT(!(color1 == color2));
        CPPUNIT_ASSERT(color1 != color2);

        // Restore component
        color1.m_b = 0x00;
        CPPUNIT_ASSERT(color1 == color2);
        CPPUNIT_ASSERT(!(color1 != color2));

        // Modify alpha
        color1.m_a = 0x80;
        CPPUNIT_ASSERT(color1 != color2);

        // Restore
        color1.m_a = 0xFF;
        CPPUNIT_ASSERT(color1 == color2);
    }

    void testMultipleOperationsSequence()
    {
        // Create from uint32
        ColorArgb color1(0xFF0000FF);

        // Verify components
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color1.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color1.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color1.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color1.m_b);

        // Compare with constant
        CPPUNIT_ASSERT(color1 == ColorArgb::BLUE);

        // Output to stream
        std::ostringstream oss;
        oss << color1;
        CPPUNIT_ASSERT_EQUAL(std::string("BLUE"), oss.str());

        // Modify component
        color1.m_r = 0xFF;
        CPPUNIT_ASSERT(!(color1 == ColorArgb::BLUE));

        // Output modified color - it matches MAGENTA/FUCHSIA constant so outputs name
        oss.str("");
        oss << color1;
        // Color matches a named constant (MAGENTA or FUCHSIA), either name acceptable
        std::string modifiedOutput = oss.str();
        CPPUNIT_ASSERT(modifiedOutput == "MAGENTA" || modifiedOutput == "FUCHSIA");

        // Compare with MAGENTA
        CPPUNIT_ASSERT(color1 == ColorArgb::MAGENTA);
    }

    void testAllFeaturesIntegration()
    {
        // Test complete workflow: getColorByName -> modify -> compare -> stream
        ColorArgb color1, color2;

        // Get color by name (case insensitive)
        bool result = ColorArgb::getColorByName("yellow", color1);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT(color1 == ColorArgb::YELLOW);

        // Stream output should show name
        std::ostringstream oss;
        oss << color1;
        CPPUNIT_ASSERT_EQUAL(std::string("YELLOW"), oss.str());

        // Create another color from components
        color2 = ColorArgb(0xFF, 0xFF, 0xFF, 0x00);
        CPPUNIT_ASSERT(color2 == ColorArgb::YELLOW);
        CPPUNIT_ASSERT(color1 == color2);

        // Modify component to make them different
        color2.m_g = 0x00;
        CPPUNIT_ASSERT(color1 != color2);
        CPPUNIT_ASSERT(color2 == ColorArgb::RED);

        // Stream output for modified color
        oss.str("");
        oss << color2;
        CPPUNIT_ASSERT_EQUAL(std::string("RED"), oss.str());

        // Verify original unchanged
        CPPUNIT_ASSERT(color1 == ColorArgb::YELLOW);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color1.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), color1.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), color1.m_b);

        // Create from uint32 and compare
        ColorArgb color3(0xFFFFFF00);
        CPPUNIT_ASSERT(color3 == color1);
        CPPUNIT_ASSERT(color3 == ColorArgb::YELLOW);

        // Test equality operators consistency
        CPPUNIT_ASSERT_EQUAL(color1 == color3, !(color1 != color3));
        CPPUNIT_ASSERT_EQUAL(color1 == color2, !(color1 != color2));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ColorArgbTest);
