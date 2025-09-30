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

#include "ColorCalculator.hpp"

using dvbsubdecoder::ColorCalculator;
using dvbsubdecoder::ColorYCrCbT;
using dvbsubdecoder::ColorARGB;

class ColorCalculatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ColorCalculatorTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testToUint32);
    CPPUNIT_TEST(testBoundaryValues);
    CPPUNIT_TEST(testYFullyTransparent);
    CPPUNIT_TEST(testMinMaxYValues);
    CPPUNIT_TEST(testMinMaxCrCbValues);
    CPPUNIT_TEST(testTransparencyValues);
    CPPUNIT_TEST(testClamping);
    CPPUNIT_TEST(testColorSpaceExtremes);
    CPPUNIT_TEST(testMathematicalAccuracy);
    CPPUNIT_TEST(testRoundingBehavior);
    CPPUNIT_TEST(testInvalidInputs);
CPPUNIT_TEST_SUITE_END()
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

    void testSimple()
    {
        struct TestCase
        {
            ColorYCrCbT m_ycrcbt;
            ColorARGB m_argb;
        };

        TestCase entries[] =
        {
        {
        { 0x00, 0x00, 0x00, 0xFF },
        { 0x00, 0x00, 0x9A, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x00 },
        { 0xFF, 0x00, 0x00, 0x00 } },

        {
        { 0x2B, 0x79, 0x82, 0x00 },
        { 0xFF, 0x14, 0x24, 0x23 }, },

        {
        { 0x46, 0x72, 0x85, 0x00 },
        { 0xFF, 0x28, 0x48, 0x48 }, },

        {
        { 0x62, 0x6B, 0x87, 0x00 },
        { 0xFF, 0x3D, 0x6D, 0x6D } },

        {
        { 0x7D, 0x64, 0x89, 0x00 },
        { 0xFF, 0x52, 0x92, 0x91 } },

        {
        { 0x98, 0x5E, 0x8C, 0x00 },
        { 0xFF, 0x67, 0xB5, 0xB6 } },

        {
        { 0xB4, 0x56, 0x8E, 0x00 },
        { 0xFF, 0x7B, 0xDB, 0xDB } },

        {
        { 0xCF, 0x50, 0x90, 0x00 },
        { 0xFF, 0x91, 0xFF, 0xFE }, },

        {
        { 0x10, 0x80, 0x80, 0x11 },
        { 0xEE, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x22 },
        { 0xDD, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x33 },
        { 0xCC, 0x00, 0x00, 0x00 } },

        {
        { 0x10, 0x80, 0x80, 0x3C },
        { 0xC3, 0x00, 0x00, 0x00 } },

        };

        for (auto iter = std::begin(entries); iter != std::end(entries); ++iter)
        {
            auto result = ColorCalculator().toARGB(iter->m_ycrcbt);

            std::cout << "Inp:" << ' ' << static_cast<int>(iter->m_ycrcbt.m_y)
                    << ' ' << static_cast<int>(iter->m_ycrcbt.m_cr) << ' '
                    << static_cast<int>(iter->m_ycrcbt.m_cb) << ' '
                    << static_cast<int>(iter->m_ycrcbt.m_t) << std::endl;

            std::cout << "Exp:" << ' ' << static_cast<int>(iter->m_argb.m_a)
                    << ' ' << static_cast<int>(iter->m_argb.m_r) << ' '
                    << static_cast<int>(iter->m_argb.m_g) << ' '
                    << static_cast<int>(iter->m_argb.m_b) << std::endl;

            std::cout << "Res:" << ' ' << static_cast<int>(result.m_a) << ' '
                    << static_cast<int>(result.m_r) << ' '
                    << static_cast<int>(result.m_g) << ' '
                    << static_cast<int>(result.m_b) << std::endl;

            auto diffA = std::abs(
                    static_cast<int>(iter->m_argb.m_a)
                            - static_cast<int>(result.m_a));
            auto diffR = std::abs(
                    static_cast<int>(iter->m_argb.m_r)
                            - static_cast<int>(result.m_r));
            auto diffG = std::abs(
                    static_cast<int>(iter->m_argb.m_g)
                            - static_cast<int>(result.m_g));
            auto diffB = std::abs(
                    static_cast<int>(iter->m_argb.m_b)
                            - static_cast<int>(result.m_b));

            // allow minimal difference due to roundings

            CPPUNIT_ASSERT(diffA <= 1);
            if (iter->m_argb.m_a > 0)
            {
                // only makes sense for non-fully-transparent colors
                CPPUNIT_ASSERT(diffR <= 1);
                CPPUNIT_ASSERT(diffG <= 1);
                CPPUNIT_ASSERT(diffB <= 1);
            }
        }
    }

    void testToUint32()
    {
        // Test the toUint32() method that was completely untested
        ColorARGB argb1{0xFF, 0x80, 0x40, 0x20};
        std::uint32_t expected1 = (0xFF << 24) | (0x80 << 16) | (0x40 << 8) | 0x20;
        CPPUNIT_ASSERT_EQUAL(expected1, argb1.toUint32());

        // Test boundary values
        ColorARGB argb2{0x00, 0x00, 0x00, 0x00};
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), argb2.toUint32());

        ColorARGB argb3{0xFF, 0xFF, 0xFF, 0xFF};
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), argb3.toUint32());

        // Test specific pattern
        ColorARGB argb4{0xAA, 0xBB, 0xCC, 0xDD};
        std::uint32_t expected4 = 0xAABBCCDD;
        CPPUNIT_ASSERT_EQUAL(expected4, argb4.toUint32());
    }

    void testBoundaryValues()
    {
        ColorCalculator calculator;

        // Test Y boundary values (16-235 range)
        ColorYCrCbT input1{16, 128, 128, 0};
        ColorARGB result1 = calculator.toARGB(input1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), result1.m_a); // Should be opaque

        ColorYCrCbT input2{235, 128, 128, 0};
        ColorARGB result2 = calculator.toARGB(input2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), result2.m_a); // Should be opaque

        // Test Cr/Cb boundary values (16-240 range)
        ColorYCrCbT input3{128, 16, 16, 0};
        ColorARGB result3 = calculator.toARGB(input3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), result3.m_a);

        ColorYCrCbT input4{128, 240, 240, 0};
        ColorARGB result4 = calculator.toARGB(input4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), result4.m_a);
    }

    void testYFullyTransparent()
    {
        ColorCalculator calculator;

        // Y=0 should result in fully transparent white
        ColorYCrCbT input{0, 128, 128, 0};
        ColorARGB result = calculator.toARGB(input);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), result.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result.m_b);

        // Y=0 with different Cr/Cb values should still be transparent white
        ColorYCrCbT input2{0, 50, 200, 100};
        ColorARGB result2 = calculator.toARGB(input2);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), result2.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result2.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result2.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), result2.m_b);
    }

    void testMinMaxYValues()
    {
        ColorCalculator calculator;

        // Test minimum Y value (should be clamped to 16)
        ColorYCrCbT input1{1, 128, 128, 0};
        ColorARGB result1 = calculator.toARGB(input1);
        
        ColorYCrCbT input2{16, 128, 128, 0};
        ColorARGB result2 = calculator.toARGB(input2);
        
        // Both should produce same result due to clamping
        CPPUNIT_ASSERT_EQUAL(result2.m_r, result1.m_r);
        CPPUNIT_ASSERT_EQUAL(result2.m_g, result1.m_g);
        CPPUNIT_ASSERT_EQUAL(result2.m_b, result1.m_b);

        // Test maximum Y value (should be clamped to 235)
        ColorYCrCbT input3{255, 128, 128, 0};
        ColorARGB result3 = calculator.toARGB(input3);
        
        ColorYCrCbT input4{235, 128, 128, 0};
        ColorARGB result4 = calculator.toARGB(input4);
        
        // Both should produce same result due to clamping
        CPPUNIT_ASSERT_EQUAL(result4.m_r, result3.m_r);
        CPPUNIT_ASSERT_EQUAL(result4.m_g, result3.m_g);
        CPPUNIT_ASSERT_EQUAL(result4.m_b, result3.m_b);
    }

    void testMinMaxCrCbValues()
    {
        ColorCalculator calculator;

        // Test minimum Cr/Cb values (should be clamped to 16)
        ColorYCrCbT input1{128, 0, 0, 0};
        ColorARGB result1 = calculator.toARGB(input1);
        
        ColorYCrCbT input2{128, 16, 16, 0};
        ColorARGB result2 = calculator.toARGB(input2);
        
        // Both should produce same result due to clamping
        CPPUNIT_ASSERT_EQUAL(result2.m_r, result1.m_r);
        CPPUNIT_ASSERT_EQUAL(result2.m_g, result1.m_g);
        CPPUNIT_ASSERT_EQUAL(result2.m_b, result1.m_b);

        // Test maximum Cr/Cb values (should be clamped to 240)
        ColorYCrCbT input3{128, 255, 255, 0};
        ColorARGB result3 = calculator.toARGB(input3);
        
        ColorYCrCbT input4{128, 240, 240, 0};
        ColorARGB result4 = calculator.toARGB(input4);
        
        // Both should produce same result due to clamping
        CPPUNIT_ASSERT_EQUAL(result4.m_r, result3.m_r);
        CPPUNIT_ASSERT_EQUAL(result4.m_g, result3.m_g);
        CPPUNIT_ASSERT_EQUAL(result4.m_b, result3.m_b);
    }

    void testTransparencyValues()
    {
        ColorCalculator calculator;

        // Test transparency calculations (A = 255 - T)
        ColorYCrCbT input1{128, 128, 128, 0};
        ColorARGB result1 = calculator.toARGB(input1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(255), result1.m_a);

        ColorYCrCbT input2{128, 128, 128, 128};
        ColorARGB result2 = calculator.toARGB(input2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(127), result2.m_a);

        ColorYCrCbT input3{128, 128, 128, 255};
        ColorARGB result3 = calculator.toARGB(input3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), result3.m_a);

        // Test transparency boundary
        ColorYCrCbT input4{128, 128, 128, 1};
        ColorARGB result4 = calculator.toARGB(input4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(254), result4.m_a);
    }

    void testClamping()
    {
        ColorCalculator calculator;

        // Test RGB clamping to 0-255 range
        // Use extreme Cr/Cb values that might cause overflow
        ColorYCrCbT input1{235, 240, 16, 0}; // High Y, high Cr, low Cb
        ColorARGB result1 = calculator.toARGB(input1);
        
        // NOTE: result components are uint8_t (always 0..255)
        CPPUNIT_ASSERT(result1.m_r <= 255);
        CPPUNIT_ASSERT(result1.m_g <= 255);
        CPPUNIT_ASSERT(result1.m_b <= 255);

        ColorYCrCbT input2{235, 16, 240, 0}; // High Y, low Cr, high Cb
        ColorARGB result2 = calculator.toARGB(input2);
        
        CPPUNIT_ASSERT(result2.m_r <= 255);
        CPPUNIT_ASSERT(result2.m_g <= 255);
        CPPUNIT_ASSERT(result2.m_b <= 255);
    }

    void testColorSpaceExtremes()
    {
        ColorCalculator calculator;

        // Test pure red scenario
        ColorYCrCbT redInput{81, 240, 128, 0}; // Approximation for red
        ColorARGB redResult = calculator.toARGB(redInput);
        // Red should be higher than green and blue
        CPPUNIT_ASSERT(redResult.m_r >= redResult.m_g);
        CPPUNIT_ASSERT(redResult.m_r >= redResult.m_b);

        // Test pure blue scenario
        ColorYCrCbT blueInput{41, 128, 240, 0}; // Approximation for blue
        ColorARGB blueResult = calculator.toARGB(blueInput);
        // Blue should be higher than red and green
        CPPUNIT_ASSERT(blueResult.m_b >= blueResult.m_r);
        CPPUNIT_ASSERT(blueResult.m_b >= blueResult.m_g);

        // Test grayscale (Cr=Cb=128)
        ColorYCrCbT grayInput{128, 128, 128, 0};
        ColorARGB grayResult = calculator.toARGB(grayInput);
        // RGB values should be approximately equal for grayscale
        int diffRG = std::abs(static_cast<int>(grayResult.m_r) - static_cast<int>(grayResult.m_g));
        int diffRB = std::abs(static_cast<int>(grayResult.m_r) - static_cast<int>(grayResult.m_b));
        int diffGB = std::abs(static_cast<int>(grayResult.m_g) - static_cast<int>(grayResult.m_b));
        CPPUNIT_ASSERT(diffRG <= 2); // Allow small rounding differences
        CPPUNIT_ASSERT(diffRB <= 2);
        CPPUNIT_ASSERT(diffGB <= 2);
    }

    void testMathematicalAccuracy()
    {
        ColorCalculator calculator;

        // Test known color conversion values
        // Black: Y=16, Cr=Cb=128 should give RGB close to (0,0,0)
        ColorYCrCbT blackInput{16, 128, 128, 0};
        ColorARGB blackResult = calculator.toARGB(blackInput);
        CPPUNIT_ASSERT(blackResult.m_r <= 5); // Should be very dark
        CPPUNIT_ASSERT(blackResult.m_g <= 5);
        CPPUNIT_ASSERT(blackResult.m_b <= 5);

        // White: Y=235, Cr=Cb=128 should give RGB close to (255,255,255)
        ColorYCrCbT whiteInput{235, 128, 128, 0};
        ColorARGB whiteResult = calculator.toARGB(whiteInput);
        CPPUNIT_ASSERT(whiteResult.m_r >= 250); // Should be very bright
        CPPUNIT_ASSERT(whiteResult.m_g >= 250);
        CPPUNIT_ASSERT(whiteResult.m_b >= 250);

        // Test mathematical consistency
        ColorYCrCbT testInput{100, 150, 110, 50};
        ColorARGB result1 = calculator.toARGB(testInput);
        ColorARGB result2 = calculator.toARGB(testInput);
        
        // Same input should always give same output
        CPPUNIT_ASSERT_EQUAL(result1.m_a, result2.m_a);
        CPPUNIT_ASSERT_EQUAL(result1.m_r, result2.m_r);
        CPPUNIT_ASSERT_EQUAL(result1.m_g, result2.m_g);
        CPPUNIT_ASSERT_EQUAL(result1.m_b, result2.m_b);
    }

    void testRoundingBehavior()
    {
        ColorCalculator calculator;

        // Test values that might cause rounding issues
        ColorYCrCbT input1{17, 129, 127, 1}; // Slightly off center values
        ColorARGB result1 = calculator.toARGB(input1);
        
        ColorYCrCbT input2{18, 127, 129, 2}; // Slightly different
        ColorARGB result2 = calculator.toARGB(input2);
        
        // Results should be different but close
        CPPUNIT_ASSERT(result1.m_r != result2.m_r || result1.m_g != result2.m_g || result1.m_b != result2.m_b);
        
        // Test that small changes produce small differences
        int diffR = std::abs(static_cast<int>(result1.m_r) - static_cast<int>(result2.m_r));
        int diffG = std::abs(static_cast<int>(result1.m_g) - static_cast<int>(result2.m_g));
        int diffB = std::abs(static_cast<int>(result1.m_b) - static_cast<int>(result2.m_b));
        
        CPPUNIT_ASSERT(diffR <= 10); // Changes should be gradual
        CPPUNIT_ASSERT(diffG <= 10);
        CPPUNIT_ASSERT(diffB <= 10);
    }

    void testInvalidInputs()
    {
        ColorCalculator calculator;

        // Test that extreme invalid inputs don't crash and produce clamped results
        ColorYCrCbT extremeInput{255, 255, 255, 255};
        ColorARGB extremeResult = calculator.toARGB(extremeInput);
        
        // Should not crash; components are uint8_t so only upper bound is meaningful.
        CPPUNIT_ASSERT(extremeResult.m_a <= 255);
        CPPUNIT_ASSERT(extremeResult.m_r <= 255);
        CPPUNIT_ASSERT(extremeResult.m_g <= 255);
        CPPUNIT_ASSERT(extremeResult.m_b <= 255);

        // Test zero values
        ColorYCrCbT zeroInput{0, 0, 0, 0};
        ColorARGB zeroResult = calculator.toARGB(zeroInput);
        
        // Y=0 case: should be transparent white
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), zeroResult.m_a);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), zeroResult.m_r);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), zeroResult.m_g);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xFF), zeroResult.m_b);

        // Test mixed extreme values
        ColorYCrCbT mixedInput{1, 255, 0, 128};
        ColorARGB mixedResult = calculator.toARGB(mixedInput);
        
        CPPUNIT_ASSERT(mixedResult.m_a <= 255);
        CPPUNIT_ASSERT(mixedResult.m_r <= 255);
        CPPUNIT_ASSERT(mixedResult.m_g <= 255);
        CPPUNIT_ASSERT(mixedResult.m_b <= 255);
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ColorCalculatorTest);
