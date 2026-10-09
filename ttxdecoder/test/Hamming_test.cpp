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
#include <array>
#include <cstddef>
#include <cstdint>

#include "Hamming.hpp"

namespace
{

std::uint8_t reverseBits(std::uint8_t value)
{
    std::uint8_t reversed = 0;

    for (int bitIndex = 0; bitIndex < 8; ++bitIndex)
    {
        reversed = static_cast<std::uint8_t>(reversed << 1);
        reversed = static_cast<std::uint8_t>(reversed | (value & 0x01));
        value = static_cast<std::uint8_t>(value >> 1);
    }

    return reversed;
}

bool hasOddParity(std::uint8_t value)
{
    std::uint8_t bitCount = 0;

    while (value != 0)
    {
        bitCount = static_cast<std::uint8_t>(bitCount + (value & 0x01));
        value = static_cast<std::uint8_t>(value >> 1);
    }

    return (bitCount % 2) == 1;
}

std::uint8_t encode84(std::uint8_t nibble)
{
    const std::uint8_t data1 = static_cast<std::uint8_t>((nibble >> 0) & 0x01);
    const std::uint8_t data2 = static_cast<std::uint8_t>((nibble >> 1) & 0x01);
    const std::uint8_t data3 = static_cast<std::uint8_t>((nibble >> 2) & 0x01);
    const std::uint8_t data4 = static_cast<std::uint8_t>((nibble >> 3) & 0x01);

    const std::uint8_t parity1 = static_cast<std::uint8_t>(1 ^ data1 ^ data3 ^ data4);
    const std::uint8_t parity2 = static_cast<std::uint8_t>(1 ^ data1 ^ data2 ^ data4);
    const std::uint8_t parity3 = static_cast<std::uint8_t>(1 ^ data1 ^ data2 ^ data3);
    const std::uint8_t parity4 = static_cast<std::uint8_t>(1 ^ parity1 ^ data1 ^ parity2 ^ data2 ^ parity3 ^ data3 ^ data4);

    return static_cast<std::uint8_t>((parity1 << 7) | (data1 << 6) | (parity2 << 5)
        | (data2 << 4) | (parity3 << 3) | (data3 << 2) | (parity4 << 1)
        | (data4 << 0));
}

std::uint8_t encodeParityProtectedByte(std::uint8_t payload)
{
    std::uint8_t dataByte = static_cast<std::uint8_t>(payload & 0x7F);

    if (!hasOddParity(dataByte))
    {
        dataByte = static_cast<std::uint8_t>(dataByte | 0x80);
    }

    return reverseBits(dataByte);
}

std::uint8_t encodeParityProtectedByteWithInvalidParity(std::uint8_t payload)
{
    std::uint8_t dataByte = static_cast<std::uint8_t>(payload & 0x7F);

    if (!hasOddParity(dataByte))
    {
        dataByte = static_cast<std::uint8_t>(dataByte | 0x80);
    }
    else
    {
        dataByte = static_cast<std::uint8_t>(dataByte & 0x7F);
    }

    dataByte = static_cast<std::uint8_t>(dataByte ^ 0x80);

    return reverseBits(dataByte);
}

std::array<std::uint8_t, 3> encode2418(std::uint32_t value)
{
    auto dataBit = [value](int bitNumber) -> std::uint8_t
    {
        return static_cast<std::uint8_t>((value >> (bitNumber - 1)) & 0x01);
    };

    const std::uint8_t p1 = static_cast<std::uint8_t>(1 ^ dataBit(1) ^ dataBit(2) ^ dataBit(4)
        ^ dataBit(5) ^ dataBit(7) ^ dataBit(9) ^ dataBit(11) ^ dataBit(12)
        ^ dataBit(14) ^ dataBit(16) ^ dataBit(18));
    const std::uint8_t p2 = static_cast<std::uint8_t>(1 ^ dataBit(1) ^ dataBit(3) ^ dataBit(4)
        ^ dataBit(6) ^ dataBit(7) ^ dataBit(10) ^ dataBit(11) ^ dataBit(13)
        ^ dataBit(14) ^ dataBit(17) ^ dataBit(18));
    const std::uint8_t p3 = static_cast<std::uint8_t>(1 ^ dataBit(2) ^ dataBit(3) ^ dataBit(4)
        ^ dataBit(8) ^ dataBit(9) ^ dataBit(10) ^ dataBit(11) ^ dataBit(15)
        ^ dataBit(16) ^ dataBit(17) ^ dataBit(18));
    const std::uint8_t p4 = static_cast<std::uint8_t>(1 ^ dataBit(5) ^ dataBit(6) ^ dataBit(7)
        ^ dataBit(8) ^ dataBit(9) ^ dataBit(10) ^ dataBit(11));
    const std::uint8_t p5 = static_cast<std::uint8_t>(1 ^ dataBit(12) ^ dataBit(13) ^ dataBit(14)
        ^ dataBit(15) ^ dataBit(16) ^ dataBit(17) ^ dataBit(18));
    const std::uint8_t p6 = static_cast<std::uint8_t>(1 ^ p1 ^ p2 ^ dataBit(1) ^ p3 ^ dataBit(2)
        ^ dataBit(3) ^ dataBit(4) ^ p4 ^ dataBit(5) ^ dataBit(6) ^ dataBit(7)
        ^ dataBit(8) ^ dataBit(9) ^ dataBit(10) ^ dataBit(11) ^ p5 ^ dataBit(12)
        ^ dataBit(13) ^ dataBit(14) ^ dataBit(15) ^ dataBit(16) ^ dataBit(17)
        ^ dataBit(18));

    const std::uint8_t logicalByte1 = static_cast<std::uint8_t>((p1 << 0) | (p2 << 1)
        | (dataBit(1) << 2) | (p3 << 3) | (dataBit(2) << 4) | (dataBit(3) << 5)
        | (dataBit(4) << 6) | (p4 << 7));
    const std::uint8_t logicalByte2 = static_cast<std::uint8_t>((dataBit(5) << 0)
        | (dataBit(6) << 1) | (dataBit(7) << 2) | (dataBit(8) << 3)
        | (dataBit(9) << 4) | (dataBit(10) << 5) | (dataBit(11) << 6)
        | (p5 << 7));
    const std::uint8_t logicalByte3 = static_cast<std::uint8_t>((dataBit(12) << 0)
        | (dataBit(13) << 1) | (dataBit(14) << 2) | (dataBit(15) << 3)
        | (dataBit(16) << 4) | (dataBit(17) << 5) | (dataBit(18) << 6)
        | (p6 << 7));

    return {reverseBits(logicalByte1), reverseBits(logicalByte2), reverseBits(logicalByte3)};
}

} // namespace

class HammingTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( HammingTest );
    CPPUNIT_TEST( test_decode2418_round_trips_known_values );
    CPPUNIT_TEST( test_decode2418_corrects_single_bit_errors );
    CPPUNIT_TEST( test_decode2418_rejects_double_bit_errors );
    CPPUNIT_TEST( test_decode84_decodes_canonical_codewords );
    CPPUNIT_TEST( test_decode84_corrects_all_single_bit_errors );
    CPPUNIT_TEST( test_decode84_rejects_invalid_codewords );
    CPPUNIT_TEST( test_decodeParity_round_trips_full_payload_range );
    CPPUNIT_TEST( test_decodeParity_rejects_invalid_parity );
    CPPUNIT_TEST( test_decodeParity_masks_parity_bit );

CPPUNIT_TEST_SUITE_END();

private:
    ttxdecoder::Hamming m_hamming;

public:
    void test_decode2418_round_trips_known_values()
    {
        const std::uint32_t values[] = {0x00000, 0x00001, 0x12345, 0x2AAAA, 0x3FFFF};

        for (std::uint32_t value : values)
        {
            const std::array<std::uint8_t, 3> encoded = encode2418(value);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(value),
                m_hamming.decode2418(encoded[0], encoded[1], encoded[2]));
        }
    }

    void test_decode2418_corrects_single_bit_errors()
    {
        const std::uint32_t value = 0x12345;
        const std::array<std::uint8_t, 3> encoded = encode2418(value);

        for (std::size_t byteIndex = 0; byteIndex < encoded.size(); ++byteIndex)
        {
            for (std::uint8_t bitMask = 0x01; bitMask != 0; bitMask = static_cast<std::uint8_t>(bitMask << 1))
            {
                std::array<std::uint8_t, 3> corrupted = encoded;
                corrupted[byteIndex] = static_cast<std::uint8_t>(corrupted[byteIndex] ^ bitMask);

                CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(value),
                    m_hamming.decode2418(corrupted[0], corrupted[1], corrupted[2]));
            }
        }
    }

    void test_decode2418_rejects_double_bit_errors()
    {
        const std::uint32_t values[] = {0x00000, 0x12345, 0x3FFFF};

        for (std::uint32_t value : values)
        {
            const std::array<std::uint8_t, 3> encoded = encode2418(value);

            for (std::size_t firstBit = 0; firstBit < 24; ++firstBit)
            {
                for (std::size_t secondBit = firstBit + 1; secondBit < 24; ++secondBit)
                {
                    std::array<std::uint8_t, 3> corrupted = encoded;
                    corrupted[firstBit / 8] = static_cast<std::uint8_t>(
                        corrupted[firstBit / 8] ^ (0x01u << (firstBit % 8)));
                    corrupted[secondBit / 8] = static_cast<std::uint8_t>(
                        corrupted[secondBit / 8] ^ (0x01u << (secondBit % 8)));

                    CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1),
                        m_hamming.decode2418(corrupted[0], corrupted[1], corrupted[2]));
                }
            }
        }
    }

    void test_decode84_decodes_canonical_codewords()
    {
        for (std::uint8_t nibble = 0; nibble < 16; ++nibble)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(nibble), m_hamming.decode84(encode84(nibble)));
        }
    }

    void test_decode84_corrects_all_single_bit_errors()
    {
        for (std::uint8_t nibble = 0; nibble < 16; ++nibble)
        {
            const std::uint8_t encoded = encode84(nibble);

            for (std::uint8_t bitMask = 0x01; bitMask != 0; bitMask = static_cast<std::uint8_t>(bitMask << 1))
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(nibble),
                    m_hamming.decode84(static_cast<std::uint8_t>(encoded ^ bitMask)));
            }
        }
    }

    void test_decode84_rejects_invalid_codewords()
    {
        std::array<bool, 256> correctable{};

        for (std::uint8_t nibble = 0; nibble < 16; ++nibble)
        {
            const std::uint8_t encoded = encode84(nibble);
            correctable[encoded] = true;

            for (std::uint8_t bitMask = 0x01; bitMask != 0; bitMask = static_cast<std::uint8_t>(bitMask << 1))
            {
                correctable[static_cast<std::uint8_t>(encoded ^ bitMask)] = true;
            }
        }

        for (std::size_t encoded = 0; encoded < correctable.size(); ++encoded)
        {
            if (!correctable[encoded])
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(-1),
                    m_hamming.decode84(static_cast<std::uint8_t>(encoded)));
            }
        }
    }

    void test_decodeParity_round_trips_full_payload_range()
    {
        for (std::uint16_t payload = 0; payload <= 0x7F; ++payload)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(payload),
                m_hamming.decodeParity(encodeParityProtectedByte(static_cast<std::uint8_t>(payload))));
        }
    }

    void test_decodeParity_rejects_invalid_parity()
    {
        const std::uint8_t payloads[] = {0x00, 0x01, 0x2A, 0x55, 0x7F};

        for (std::uint8_t payload : payloads)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(-1),
                m_hamming.decodeParity(encodeParityProtectedByteWithInvalidParity(payload)));
        }
    }

    void test_decodeParity_masks_parity_bit()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(0x00), m_hamming.decodeParity(encodeParityProtectedByte(0x00)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(0x55), m_hamming.decodeParity(encodeParityProtectedByte(0x55)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(0x7F), m_hamming.decodeParity(encodeParityProtectedByte(0x7F)));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION( HammingTest );
