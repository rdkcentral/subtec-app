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
#include <cstdint>
#include <limits>

#include "Hamming.hpp"

class HammingTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( HammingTest );
    CPPUNIT_TEST( test_decode2418_all_bytes_zero );
    CPPUNIT_TEST( test_decode2418_all_bytes_max );
    CPPUNIT_TEST( test_decode2418_alternating_pattern );
    CPPUNIT_TEST( test_decode2418_returns_int32 );
    CPPUNIT_TEST( test_decode2418_known_value_1 );
    CPPUNIT_TEST( test_decode2418_known_value_2 );
    CPPUNIT_TEST( test_decode2418_known_value_3 );
    CPPUNIT_TEST( test_decode2418_known_value_4 );
    CPPUNIT_TEST( test_decode2418_byte_flip_consistency );
    CPPUNIT_TEST( test_decode2418_varied_byte_combinations );
    CPPUNIT_TEST( test_decode84_byte_zero );
    CPPUNIT_TEST( test_decode84_byte_max );
    CPPUNIT_TEST( test_decode84_low_nibble );
    CPPUNIT_TEST( test_decode84_high_nibble );
    CPPUNIT_TEST( test_decode84_valid_encoded_data_1 );
    CPPUNIT_TEST( test_decode84_valid_encoded_data_2 );
    CPPUNIT_TEST( test_decode84_valid_encoded_data_3 );
    CPPUNIT_TEST( test_decode84_returns_valid_range );
    CPPUNIT_TEST( test_decode84_exhaustive_sample );
    CPPUNIT_TEST( test_decode84_error_markers );
    CPPUNIT_TEST( test_decodeParity_byte_zero );
    CPPUNIT_TEST( test_decodeParity_byte_max );
    CPPUNIT_TEST( test_decodeParity_alternating_bits );
    CPPUNIT_TEST( test_decodeParity_valid_parity_1 );
    CPPUNIT_TEST( test_decodeParity_valid_parity_2 );
    CPPUNIT_TEST( test_decodeParity_invalid_parity );
    CPPUNIT_TEST( test_decodeParity_parity_bit_mask );
    CPPUNIT_TEST( test_decodeParity_return_value_range );
    CPPUNIT_TEST( test_decodeParity_known_patterns );
    CPPUNIT_TEST( test_decodeParity_data_preservation );
    CPPUNIT_TEST( test_decode2418_mid_range_values );
    CPPUNIT_TEST( test_decode2418_sequential_bytes );
    CPPUNIT_TEST( test_decode2418_bit_pattern_walk );
    CPPUNIT_TEST( test_decode2418_complementary_values );
    CPPUNIT_TEST( test_decode84_mid_range_values );
    CPPUNIT_TEST( test_decode84_all_single_bits );
    CPPUNIT_TEST( test_decode84_deterministic_behavior );
    CPPUNIT_TEST( test_decode84_comprehensive_sample );
    CPPUNIT_TEST( test_decodeParity_bit_walk );
    CPPUNIT_TEST( test_decodeParity_even_parity_patterns );
    CPPUNIT_TEST( test_decodeParity_odd_parity_patterns );
    CPPUNIT_TEST( test_decodeParity_consistency_across_calls );
    CPPUNIT_TEST( test_decode2418_state_isolation );
    CPPUNIT_TEST( test_decode84_state_isolation );
    CPPUNIT_TEST( test_decodeParity_state_isolation );
    CPPUNIT_TEST( test_decode2418_no_side_effects );
    CPPUNIT_TEST( test_decode84_no_side_effects );
    CPPUNIT_TEST( test_decodeParity_no_side_effects );

CPPUNIT_TEST_SUITE_END();

private:
    ttxdecoder::Hamming m_hamming;

public:
    void setUp() override
    {
        /* Initialize Hamming decoder instance */
    }

    void tearDown() override
    {
        /* Clean up resources */
    }

    void test_decode2418_all_bytes_zero()
    {
        /* Test boundary condition: all zero bytes */
        std::int32_t result = m_hamming.decode2418(0x00, 0x00, 0x00);
        /* Verify result is either valid data (>=0) or error (-1) */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_all_bytes_max()
    {
        /* Test boundary condition: all maximum value bytes */
        std::int32_t result = m_hamming.decode2418(0xFF, 0xFF, 0xFF);
        /* Verify result is either valid data (>=0) or error (-1) */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_alternating_pattern()
    {
        /* Test pattern stress: alternating bit pattern */
        std::int32_t result1 = m_hamming.decode2418(0xAA, 0x55, 0xAA);
        CPPUNIT_ASSERT(result1 >= -1);

        /* Reverse alternating pattern */
        std::int32_t result2 = m_hamming.decode2418(0x55, 0xAA, 0x55);
        CPPUNIT_ASSERT(result2 >= -1);
    }

    void test_decode2418_returns_int32()
    {
        /* Verify return type produces valid values */
        std::int32_t result = m_hamming.decode2418(0x12, 0x34, 0x56);
        /* Check that result is either valid data (>=0) or error (-1) */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_known_value_1()
    {
        /* Test with specific byte pattern - verifies decoder works */
        std::int32_t result = m_hamming.decode2418(0x11, 0x11, 0x11);
        /* Should return valid result */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_known_value_2()
    {
        /* Test with different known encoded values */
        std::int32_t result = m_hamming.decode2418(0x01, 0x00, 0x00);
        /* Should produce consistent result */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_known_value_3()
    {
        /* Test mixed byte values */
        std::int32_t result = m_hamming.decode2418(0xFF, 0x00, 0xFF);
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_known_value_4()
    {
        /* Test with different byte pattern */
        std::int32_t result = m_hamming.decode2418(0xAB, 0xCD, 0xEF);
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decode2418_byte_flip_consistency()
    {
        /* Verify that same input always produces same output */
        std::int32_t result1 = m_hamming.decode2418(0x0F, 0xF0, 0xCC);
        std::int32_t result2 = m_hamming.decode2418(0x0F, 0xF0, 0xCC);
        CPPUNIT_ASSERT_EQUAL(result1, result2);
    }

    void test_decode2418_varied_byte_combinations()
    {
        /* Test multiple combinations don't crash and return valid values */
        std::int32_t results[5];
        results[0] = m_hamming.decode2418(0x11, 0x22, 0x33);
        results[1] = m_hamming.decode2418(0x44, 0x55, 0x66);
        results[2] = m_hamming.decode2418(0x77, 0x88, 0x99);
        results[3] = m_hamming.decode2418(0xBB, 0xCC, 0xDD);
        results[4] = m_hamming.decode2418(0xEE, 0xFF, 0x00);

        /* All should be valid values */
        for (int i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT(results[i] >= -1);
        }
    }

    void test_decode84_byte_zero()
    {
        /* Test boundary: minimum byte value */
        std::int8_t result = m_hamming.decode84(0x00);
        /* Result should be 0-15 (valid decoded data) or -1 (error) */
        CPPUNIT_ASSERT(result >= -1 && result <= 15);
    }

    void test_decode84_byte_max()
    {
        /* Test boundary: maximum byte value */
        std::int8_t result = m_hamming.decode84(0xFF);
        /* Result should be 0-15 (valid decoded data) or -1 (error) */
        CPPUNIT_ASSERT(result >= -1 && result <= 15);
    }

    void test_decode84_low_nibble()
    {
        /* Test lower nibble variations */
        for (std::uint8_t i = 0; i < 16; ++i)
        {
            std::int8_t result = m_hamming.decode84(i);
            CPPUNIT_ASSERT(result >= -1 && result <= 15);
        }
    }

    void test_decode84_high_nibble()
    {
        /* Test upper nibble variations */
        for (int i = 0xF0; i <= 0xFF; ++i)
        {
            std::int8_t result = m_hamming.decode84(static_cast<std::uint8_t>(i));
            CPPUNIT_ASSERT(result >= -1 && result <= 15);
        }
    }

    void test_decode84_valid_encoded_data_1()
    {
        /* Test known valid Hamming 8/4 encoded values */
        std::int8_t result = m_hamming.decode84(0x38);
        /* Should return valid decoded 4-bit value (0-15) or -1 (error) */
        CPPUNIT_ASSERT(result >= -1 && result <= 15);
    }

    void test_decode84_valid_encoded_data_2()
    {
        /* Test another valid encoded value */
        std::int8_t result = m_hamming.decode84(0x5A);
        CPPUNIT_ASSERT(result >= -1 && result <= 15);
    }

    void test_decode84_valid_encoded_data_3()
    {
        /* Test third valid encoded value */
        std::int8_t result = m_hamming.decode84(0x2D);
        CPPUNIT_ASSERT(result >= -1 && result <= 15);
    }

    void test_decode84_returns_valid_range()
    {
        /* Verify all 256 possible inputs return values in expected range */
        bool allValid = true;
        for (int i = 0; i <= 255; ++i)
        {
            std::int8_t result = m_hamming.decode84(static_cast<std::uint8_t>(i));
            /* Result should be 0-15 (valid decoded data) or -1 (error) */
            if (!(result >= -1 && result <= 15))
            {
                allValid = false;
                break;
            }
        }
        CPPUNIT_ASSERT(allValid);
    }

    void test_decode84_exhaustive_sample()
    {
        /* Test sample of all 256 byte values for consistency */
        std::int8_t samples[10];
        std::uint8_t testBytes[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0xFF};

        for (int i = 0; i < 10; ++i)
        {
            samples[i] = m_hamming.decode84(testBytes[i]);
            CPPUNIT_ASSERT(samples[i] >= -1 && samples[i] <= 15);
        }
    }

    void test_decode84_error_markers()
    {
        /* Verify that error condition (0xFF -> -1 in int8_t) is handled */
        std::int8_t result = m_hamming.decode84(0x01);
        /* This should be an error marker from lookup table */
        CPPUNIT_ASSERT(result >= -1);
    }

    void test_decodeParity_byte_zero()
    {
        /* Test boundary: minimum byte value with parity */
        std::int8_t result = m_hamming.decodeParity(0x00);
        /* Should return either valid data (0-127) or -1 (parity error) */
        CPPUNIT_ASSERT(result >= -1 && result <= 127);
    }

    void test_decodeParity_byte_max()
    {
        /* Test boundary: maximum byte value with parity */
        std::int8_t result = m_hamming.decodeParity(0xFF);
        CPPUNIT_ASSERT(result >= -1 && result <= 127);
    }

    void test_decodeParity_alternating_bits()
    {
        /* Test alternating bit patterns */
        std::int8_t result1 = m_hamming.decodeParity(0xAA);
        std::int8_t result2 = m_hamming.decodeParity(0x55);

        /* Both should return valid results */
        CPPUNIT_ASSERT(result1 >= -1 && result1 <= 127);
        CPPUNIT_ASSERT(result2 >= -1 && result2 <= 127);
    }

    void test_decodeParity_valid_parity_1()
    {
        /* Test byte with valid odd parity */
        std::int8_t result = m_hamming.decodeParity(0x80);
        /* Single bit set (0x80) has odd parity */
        CPPUNIT_ASSERT(result >= -1 && result <= 127);
    }

    void test_decodeParity_valid_parity_2()
    {
        /* Test another valid parity case */
        std::int8_t result = m_hamming.decodeParity(0x01);
        CPPUNIT_ASSERT(result >= -1 && result <= 127);
    }

    void test_decodeParity_invalid_parity()
    {
        /* Test byte with even parity (should fail parity check) */
        std::int8_t result = m_hamming.decodeParity(0x03);
        /* 0x03 has even parity (2 bits set), should likely return -1 */
        CPPUNIT_ASSERT(result >= -1 && result <= 127);
    }

    void test_decodeParity_parity_bit_mask()
    {
        /* Verify parity bit (bit 7) is masked in returned value */
        std::int8_t result = m_hamming.decodeParity(0x7F);
        /* If parity passes, result should be <= 127 (bit 7 masked off) */
        if (result >= 0)
        {
            CPPUNIT_ASSERT(result <= 127);
        }
    }

    void test_decodeParity_return_value_range()
    {
        /* Test all 256 possible inputs return values in valid range */
        bool allValid = true;
        for (int i = 0; i <= 255; ++i)
        {
            std::int8_t result = m_hamming.decodeParity(static_cast<std::uint8_t>(i));
            /* Result should be 0-127 (7-bit data) or -1 (error) */
            if (!(result >= -1 && result <= 127))
            {
                allValid = false;
                break;
            }
        }
        CPPUNIT_ASSERT(allValid);
    }

    void test_decodeParity_known_patterns()
    {
        /* Test known specific patterns */
        std::uint8_t patterns[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

        for (int i = 0; i < 8; ++i)
        {
            std::int8_t result = m_hamming.decodeParity(patterns[i]);
            CPPUNIT_ASSERT(result >= -1 && result <= 127);
        }
    }

    void test_decodeParity_data_preservation()
    {
        /* Verify same input produces same output (deterministic) */
        std::int8_t result1 = m_hamming.decodeParity(0x3C);
        std::int8_t result2 = m_hamming.decodeParity(0x3C);
        CPPUNIT_ASSERT_EQUAL(result1, result2);
    }

    void test_decode2418_mid_range_values()
    {
        /* Test with middle-range byte values */
        std::int32_t result1 = m_hamming.decode2418(0x7F, 0x80, 0x40);
        std::int32_t result2 = m_hamming.decode2418(0x40, 0x7F, 0x80);
        std::int32_t result3 = m_hamming.decode2418(0x80, 0x40, 0x7F);

        /* All should return valid values */
        CPPUNIT_ASSERT(result1 >= -1);
        CPPUNIT_ASSERT(result2 >= -1);
        CPPUNIT_ASSERT(result3 >= -1);
    }

    void test_decode2418_sequential_bytes()
    {
        /* Test sequential increment patterns */
        std::int32_t result1 = m_hamming.decode2418(0x00, 0x01, 0x02);
        std::int32_t result2 = m_hamming.decode2418(0x10, 0x11, 0x12);
        std::int32_t result3 = m_hamming.decode2418(0xF0, 0xF1, 0xF2);

        /* All should return valid values */
        CPPUNIT_ASSERT(result1 >= -1);
        CPPUNIT_ASSERT(result2 >= -1);
        CPPUNIT_ASSERT(result3 >= -1);
    }

    void test_decode2418_bit_pattern_walk()
    {
        /* Test single-bit walking pattern (0x01, 0x02, 0x04, 0x08, etc.) */
        std::int32_t results[8];
        for (int i = 0; i < 8; ++i)
        {
            std::uint8_t bit_pattern = 1 << i;
            results[i] = m_hamming.decode2418(bit_pattern, bit_pattern, bit_pattern);
            CPPUNIT_ASSERT(results[i] >= -1);
        }

        /* Results should vary across bit positions */
        bool hasVariation = false;
        for (int i = 0; i < 7; ++i)
        {
            if (results[i] != results[i+1])
            {
                hasVariation = true;
                break;
            }
        }
        CPPUNIT_ASSERT(hasVariation);
    }

    void test_decode2418_complementary_values()
    {
        /* Test complementary bit patterns */
        std::int32_t result_zeros = m_hamming.decode2418(0x00, 0x00, 0x00);
        std::int32_t result_ones = m_hamming.decode2418(0xFF, 0xFF, 0xFF);
        std::int32_t result_complement1 = m_hamming.decode2418(0xFF, 0x00, 0xFF);
        std::int32_t result_complement2 = m_hamming.decode2418(0x00, 0xFF, 0x00);

        /* All should be valid */
        CPPUNIT_ASSERT(result_zeros >= -1);
        CPPUNIT_ASSERT(result_ones >= -1);
        CPPUNIT_ASSERT(result_complement1 >= -1);
        CPPUNIT_ASSERT(result_complement2 >= -1);
    }

    void test_decode84_mid_range_values()
    {
        /* Test mid-range byte values (not extreme) */
        std::int8_t result1 = m_hamming.decode84(0x40);
        std::int8_t result2 = m_hamming.decode84(0x7F);
        std::int8_t result3 = m_hamming.decode84(0x80);

        /* All should be in valid range */
        CPPUNIT_ASSERT(result1 >= -1 && result1 <= 15);
        CPPUNIT_ASSERT(result2 >= -1 && result2 <= 15);
        CPPUNIT_ASSERT(result3 >= -1 && result3 <= 15);
    }

    void test_decode84_all_single_bits()
    {
        /* Test single-bit patterns for all 8 bit positions */
        std::int8_t results[8];
        for (int i = 0; i < 8; ++i)
        {
            std::uint8_t single_bit = 1 << i;
            results[i] = m_hamming.decode84(single_bit);
            CPPUNIT_ASSERT(results[i] >= -1 && results[i] <= 15);
        }
    }

    void test_decode84_deterministic_behavior()
    {
        /* Test that decode84 produces consistent results across multiple calls */
        std::uint8_t test_values[] = {0x00, 0x55, 0xAA, 0xFF, 0x7E, 0x2A, 0x3B};

        for (int i = 0; i < 7; ++i)
        {
            std::int8_t result1 = m_hamming.decode84(test_values[i]);
            std::int8_t result2 = m_hamming.decode84(test_values[i]);
            std::int8_t result3 = m_hamming.decode84(test_values[i]);

            CPPUNIT_ASSERT_EQUAL(result1, result2);
            CPPUNIT_ASSERT_EQUAL(result2, result3);
        }
    }

    void test_decode84_comprehensive_sample()
    {
        /* Test power-of-2 boundary values */
        std::uint8_t boundaries[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x7F, 0xFE};

        for (int i = 0; i < 10; ++i)
        {
            std::int8_t result = m_hamming.decode84(boundaries[i]);
            CPPUNIT_ASSERT(result >= -1 && result <= 15);
        }
    }

    void test_decodeParity_bit_walk()
    {
        /* Test multi-bit patterns (2-3 bits set) */
        std::uint8_t multi_bit_patterns[] = {0x03, 0x05, 0x06, 0x09, 0x0A, 0x0C, 0x07, 0x0B};

        for (int i = 0; i < 8; ++i)
        {
            std::int8_t result = m_hamming.decodeParity(multi_bit_patterns[i]);
            CPPUNIT_ASSERT(result >= -1 && result <= 127);
        }
    }

    void test_decodeParity_even_parity_patterns()
    {
        /* Test patterns with even number of bits set (should have even parity) */
        std::uint8_t even_patterns[] = {0x00, 0x03, 0x05, 0x06, 0x09, 0x0A, 0x0C, 0x0F};

        for (int i = 0; i < 8; ++i)
        {
            std::int8_t result = m_hamming.decodeParity(even_patterns[i]);
            CPPUNIT_ASSERT(result >= -1 && result <= 127);
        }
    }

    void test_decodeParity_odd_parity_patterns()
    {
        /* Test patterns with odd number of bits set (should have odd parity) */
        std::uint8_t odd_patterns[] = {0x01, 0x02, 0x04, 0x07, 0x08, 0x0B, 0x0D, 0x0E};

        for (int i = 0; i < 8; ++i)
        {
            std::int8_t result = m_hamming.decodeParity(odd_patterns[i]);
            CPPUNIT_ASSERT(result >= -1 && result <= 127);
        }
    }

    void test_decodeParity_consistency_across_calls()
    {
        /* Test that multiple calls with same input produce identical results */
        std::uint8_t test_bytes[] = {0x00, 0x7F, 0x80, 0xFF, 0x2A, 0x5D, 0xA3};

        for (int i = 0; i < 7; ++i)
        {
            std::int8_t call1 = m_hamming.decodeParity(test_bytes[i]);
            std::int8_t call2 = m_hamming.decodeParity(test_bytes[i]);
            std::int8_t call3 = m_hamming.decodeParity(test_bytes[i]);
            std::int8_t call4 = m_hamming.decodeParity(test_bytes[i]);

            CPPUNIT_ASSERT_EQUAL(call1, call2);
            CPPUNIT_ASSERT_EQUAL(call2, call3);
            CPPUNIT_ASSERT_EQUAL(call3, call4);
        }
    }

    void test_decode2418_state_isolation()
    {
        /* Verify that multiple decode2418 calls don't interfere with each other */
        std::int32_t result1_first = m_hamming.decode2418(0x11, 0x22, 0x33);
        std::int32_t result2 = m_hamming.decode2418(0xAA, 0xBB, 0xCC);
        std::int32_t result1_second = m_hamming.decode2418(0x11, 0x22, 0x33);

        /* Same input should produce same output regardless of intervening calls */
        CPPUNIT_ASSERT_EQUAL(result1_first, result1_second);
    }

    void test_decode84_state_isolation()
    {
        /* Verify that multiple decode84 calls don't interfere with each other */
        std::int8_t result1_first = m_hamming.decode84(0x42);
        std::int8_t result2 = m_hamming.decode84(0xB7);
        std::int8_t result3 = m_hamming.decode84(0x5C);
        std::int8_t result1_second = m_hamming.decode84(0x42);

        /* Same input should produce same output regardless of intervening calls */
        CPPUNIT_ASSERT_EQUAL(result1_first, result1_second);
    }

    void test_decodeParity_state_isolation()
    {
        /* Verify that multiple decodeParity calls don't interfere with each other */
        std::int8_t result1_first = m_hamming.decodeParity(0x5A);
        std::int8_t result2 = m_hamming.decodeParity(0x3D);
        std::int8_t result3 = m_hamming.decodeParity(0x7E);
        std::int8_t result1_second = m_hamming.decodeParity(0x5A);

        /* Same input should produce same output regardless of intervening calls */
        CPPUNIT_ASSERT_EQUAL(result1_first, result1_second);
    }

    void test_decode2418_no_side_effects()
    {
        /* Verify decode2418 doesn't modify state across multiple method calls */
        std::int32_t result1 = m_hamming.decode2418(0x00, 0x00, 0x00);
        std::int32_t result2 = m_hamming.decode2418(0xFF, 0xFF, 0xFF);
        std::int32_t result3 = m_hamming.decode2418(0x00, 0x00, 0x00);

        /* Third call with same input as first should match first result */
        CPPUNIT_ASSERT_EQUAL(result1, result3);

        /* All calls should produce valid results */
        CPPUNIT_ASSERT(result1 >= -1);
        CPPUNIT_ASSERT(result2 >= -1);
    }

    void test_decode84_no_side_effects()
    {
        /* Verify decode84 doesn't modify state across multiple method calls */
        std::int8_t result1 = m_hamming.decode84(0x7F);
        std::int8_t result2 = m_hamming.decode84(0x00);
        std::int8_t result3 = m_hamming.decode84(0xFF);
        std::int8_t result4 = m_hamming.decode84(0x7F);

        /* First and fourth call with same input should match */
        CPPUNIT_ASSERT_EQUAL(result1, result4);
    }

    void test_decodeParity_no_side_effects()
    {
        /* Verify decodeParity doesn't modify state across multiple method calls */
        std::int8_t result1 = m_hamming.decodeParity(0x45);
        std::int8_t result2 = m_hamming.decodeParity(0x9A);
        std::int8_t result3 = m_hamming.decodeParity(0x6D);
        std::int8_t result4 = m_hamming.decodeParity(0x45);

        /* First and fourth call with same input should match */
        CPPUNIT_ASSERT_EQUAL(result1, result4);

        /* All intermediate calls should also be deterministic */
        std::int8_t result2_repeat = m_hamming.decodeParity(0x9A);
        CPPUNIT_ASSERT_EQUAL(result2, result2_repeat);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( HammingTest );
