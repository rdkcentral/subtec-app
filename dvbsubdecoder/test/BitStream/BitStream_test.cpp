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

#include "BitStream.hpp"

using dvbsubdecoder::BitStream;
using dvbsubdecoder::PesPacketReader;

class BitStreamTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( BitStreamTest );
    CPPUNIT_TEST(testReadSimple);
    CPPUNIT_TEST(testReadRandom);
    CPPUNIT_TEST(testConstructorWithValidReader);
    CPPUNIT_TEST(testConstructorState);
    CPPUNIT_TEST(testReadBoundaryValues);
    CPPUNIT_TEST(testReadSequential);
    CPPUNIT_TEST(testReadCrossByteBoundary);
    CPPUNIT_TEST(testReadExhaustData);
    CPPUNIT_TEST(testReadAfterDataExhausted);
    CPPUNIT_TEST(testReadDifferentSizes);
    CPPUNIT_TEST(testReadMixedSizes);
    CPPUNIT_TEST(testReadMinMaxSizes);
    CPPUNIT_TEST(testReadSingleBitSequence);
    CPPUNIT_TEST(testReadMaxBitSequence);
    CPPUNIT_TEST(testReadZeroPattern);
    CPPUNIT_TEST(testReadOnePattern);
    CPPUNIT_TEST(testReadAlternatingPattern);
    CPPUNIT_TEST(testReadComplexPattern);
    CPPUNIT_TEST(testReadEmptyData);
    CPPUNIT_TEST(testReadWithTwoChunks);
    CPPUNIT_TEST(testReadSubReader);
    CPPUNIT_TEST(testExceptionPropagation);
    CPPUNIT_TEST(testMultipleReads);
    CPPUNIT_TEST(testBitAlignment);
    CPPUNIT_TEST(testLargeDataSet);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    template<std::uint32_t ITEM_SIZE>
    void testReadSimple(std::uint32_t dataValue)
    {
        // build bytes table
        std::vector<std::uint8_t> data;
        data.push_back((dataValue >> 24) & 0xFF);
        data.push_back((dataValue >> 16) & 0xFF);
        data.push_back((dataValue >> 8) & 0xFF);
        data.push_back((dataValue >> 0) & 0xFF);

        // build expected bit chunks table
        std::vector<std::uint8_t> expected;
        const std::uint32_t bitMask = (1 << ITEM_SIZE) - 1;
        for (int bitIndex = 32 - ITEM_SIZE; bitIndex >= 0; bitIndex -=
                ITEM_SIZE)
        {
            expected.push_back((dataValue >> bitIndex) & bitMask);
        }

        // build read bit chunks table
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        std::vector<std::uint8_t> read;

        for (std::size_t i = 0; i < data.size() * 8 / ITEM_SIZE; ++i)
        {
            read.push_back(bitStream.read<ITEM_SIZE>());
        }

        CPPUNIT_ASSERT_EQUAL(expected.size(), read.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expected[i], read[i]);
        }
    }

    void testReadSimple()
    {
        const std::uint32_t values[] =
        { 0x12345678, 0xFE12EF21, 0x11223221, 0xFFFFFFFF, 0x00000000 };
        for (auto iter = std::begin(values); iter != std::end(values); ++iter)
        {
            testReadSimple<1>(*iter);
            testReadSimple<2>(*iter);
            testReadSimple<3>(*iter);
            testReadSimple<4>(*iter);
            testReadSimple<5>(*iter);
            testReadSimple<6>(*iter);
            testReadSimple<7>(*iter);
            testReadSimple<8>(*iter);
        }
    }

    void testReadRandom()
    {
        struct TestItem
        {
            constexpr TestItem(std::uint32_t value,
                               int size) :
                    m_value(value),
                    m_size(size)
            {
                // noop
            }

            std::uint32_t m_value;
            int m_size;
        };

        constexpr TestItem items[] =
        {
        { 1, 1 },
        { 2, 3 },
        { 3, 2 },
        { 0, 1 },
        { 255, 8 },
        { 6, 3 },
        { 14, 5 },
        { 1, 1 },
        { 0, 2 },
        { 1, 1 } };

        std::vector<std::uint8_t> expected;

        std::uint32_t dataValue = 0;
        int totalSize = 0;
        for (auto iter = std::begin(items); iter != std::end(items); ++iter)
        {
            CPPUNIT_ASSERT(iter->m_size >= 1);
            CPPUNIT_ASSERT(iter->m_size <= 8);
            CPPUNIT_ASSERT(iter->m_value < (1 << iter->m_size));

            dataValue <<= iter->m_size;
            dataValue |= iter->m_value;

            expected.push_back(iter->m_value);

            totalSize += iter->m_size;
        }
        CPPUNIT_ASSERT(totalSize <= 32);

        dataValue <<= (32 - totalSize);

        // build bytes table
        std::vector<std::uint8_t> data;
        data.push_back((dataValue >> 24) & 0xFF);
        data.push_back((dataValue >> 16) & 0xFF);
        data.push_back((dataValue >> 8) & 0xFF);
        data.push_back((dataValue >> 0) & 0xFF);

        // build read bit chunks table
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        std::vector<std::uint8_t> read;
        read.push_back(bitStream.read<items[0].m_size>());
        read.push_back(bitStream.read<items[1].m_size>());
        read.push_back(bitStream.read<items[2].m_size>());
        read.push_back(bitStream.read<items[3].m_size>());
        read.push_back(bitStream.read<items[4].m_size>());
        read.push_back(bitStream.read<items[5].m_size>());
        read.push_back(bitStream.read<items[6].m_size>());
        read.push_back(bitStream.read<items[7].m_size>());
        read.push_back(bitStream.read<items[8].m_size>());
        read.push_back(bitStream.read<items[9].m_size>());

        CPPUNIT_ASSERT_EQUAL(expected.size(), read.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL((int )expected[i], (int )read[i]);
        }
    }

    // Constructor Testing
    void testConstructorWithValidReader()
    {
        std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        
        // Constructor should accept valid reader without throwing
        BitStream bitStream(reader);
        
        // Should be able to read data after construction
        std::uint8_t value = bitStream.read<8>();
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, value);
    }

    void testConstructorState()
    {
        std::vector<std::uint8_t> data = {0xFF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Initial state should allow reading from fresh buffer
        std::uint8_t value1 = bitStream.read<4>();
        std::uint8_t value2 = bitStream.read<4>();
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0F, value1);
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0F, value2);
    }

    // Boundary Value Testing
    void testReadBoundaryValues()
    {
        // Test with maximum values for different bit sizes
        // Need enough data: 1+2+3+4+5+6+7+8 = 36 bits = 5 bytes minimum
        std::vector<std::uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>());  // Max 1-bit: 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x03, bitStream.read<2>());  // Max 2-bit: 3
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x07, bitStream.read<3>());  // Max 3-bit: 7
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0F, bitStream.read<4>());  // Max 4-bit: 15
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x1F, bitStream.read<5>());  // Max 5-bit: 31
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x3F, bitStream.read<6>());  // Max 6-bit: 63
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x7F, bitStream.read<7>());  // Max 7-bit: 127
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0xFF, bitStream.read<8>());  // Max 8-bit: 255
    }

    void testReadSequential()
    {
        // Test sequential reading through multiple bytes
        std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x34, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x56, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x78, bitStream.read<8>());
    }

    // Cross-byte Boundary Reads
    void testReadCrossByteBoundary()
    {
        // Test reading across byte boundaries
        std::vector<std::uint8_t> data = {0xAB, 0xCD}; // 10101011 11001101
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 10101011 11001101
        // 101 = 5, 01011 = 11, 110 = 6, 01101 = 13
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x05, bitStream.read<3>()); // 101 = 5
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0B, bitStream.read<5>()); // 01011 = 11
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x06, bitStream.read<3>()); // 110 = 6
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0D, bitStream.read<5>()); // 01101 = 13
    }

    // Data Exhaustion
    void testReadExhaustData()
    {
        std::vector<std::uint8_t> data = {0xFF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Read exactly all available bits
        bitStream.read<4>();
        bitStream.read<4>();
        
        // Next read should trigger exception from PesPacketReader
        try
        {
            bitStream.read<1>();
            CPPUNIT_FAIL("Expected exception when reading beyond available data");
        }
        catch (const PesPacketReader::Exception&)
        {
            // Expected behavior
        }
    }

    void testReadAfterDataExhausted()
    {
        std::vector<std::uint8_t> data = {0x12};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Exhaust all data
        bitStream.read<8>();
        
        // Multiple attempts to read should all throw
        for (int i = 0; i < 3; ++i)
        {
            try
            {
                bitStream.read<1>();
                CPPUNIT_FAIL("Expected exception on repeated reads after data exhaustion");
            }
            catch (const PesPacketReader::Exception&)
            {
                // Expected behavior
            }
        }
    }

    // Different Read Sizes
    void testReadDifferentSizes()
    {
        std::vector<std::uint8_t> data = {0x87, 0x65, 0x43, 0x21}; // 10000111 01100101 01000011 00100001
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        /*
            Data bytes (MSB -> LSB): 0x87 0x65 0x43 0x21
            Binary: 1000 0111 0110 0101 0100 0011 0010 0001

            Read sequence (sizes: 1,2,3,4,5,6,8,3):
               Size | Bit range (MSB=31) | Bits          | Value (hex/dec)
                1   | 31                 | 1             | 0x1  (1)
                2   | 30-29              | 00            | 0x0  (0)
                3   | 28-26              | 001           | 0x1  (1)
                4   | 25-22              | 1101          | 0xD  (13)
                5   | 21-17              | 10010         | 0x12 (18)
                6   | 16-11              | 101000        | 0x28 (40)
                8   | 10-3               | 01100100      | 0x64 (100)
                3   | 2-0                | 001           | 0x1  (1)
        */
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // bit 0: 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<2>()); // bits 1-2: 00
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<3>()); // bits 3-5: 001
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0D, bitStream.read<4>()); // bits 6-9: 1101 = 13
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, bitStream.read<5>()); // bits 10-14: 10010 = 18
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x28, bitStream.read<6>()); // bits 15-20: 101000 = 40
        // Remaining: bits 21-31 = 01100100001 = 11 bits
        // Let's read 8 bits: 01100100 = 100, then 3 bits: 001 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x64, bitStream.read<8>()); // bits 21-28: 01100100 = 100
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<3>()); // bits 29-31: 001 = 1
    }

    void testReadMixedSizes()
    {
        std::vector<std::uint8_t> data = {0xF0, 0xAA}; // 11110000 10101010
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 11110000 10101010
        // 1111 = 15, 0 = 0, 000 = 0, 101010 = 42, 10 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0F, bitStream.read<4>()); // 1111 = 15
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); // 0 = 0
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<3>()); // 000 = 0
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x2A, bitStream.read<6>()); // 101010 = 42
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<2>()); // 10 = 2
    }

    // Min/Max Template Parameters
    void testReadMinMaxSizes()
    {
        std::vector<std::uint8_t> data = {0xFF, 0x00, 0xFF, 0x00};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Test minimum size (1 bit)
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>());
        
        // Test maximum size (8 bits)
        std::uint8_t remaining7Bits = bitStream.read<7>();
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x7F, remaining7Bits);
        
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0xFF, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<8>());
    }

    void testReadSingleBitSequence()
    {
        std::vector<std::uint8_t> data = {0xAA}; // 10101010
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Read all bits one by one
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); // 0
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); // 0
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); // 0
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); // 0
    }

    void testReadMaxBitSequence()
    {
        std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Read all data in maximum chunks
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x34, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x56, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x78, bitStream.read<8>());
    }

    // Pattern Testing
    void testReadZeroPattern()
    {
        // Need enough data: 1+2+3+4+5+6+7+8 = 36 bits = 5 bytes minimum
        std::vector<std::uint8_t> data = {0x00, 0x00, 0x00, 0x00, 0x00};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        for (int size = 1; size <= 8; ++size)
        {
            switch (size)
            {
                case 1: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<1>()); break;
                case 2: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<2>()); break;
                case 3: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<3>()); break;
                case 4: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<4>()); break;
                case 5: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<5>()); break;
                case 6: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<6>()); break;
                case 7: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<7>()); break;
                case 8: CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<8>()); break;
            }
        }
    }

    void testReadOnePattern()
    {
        // Need enough data: 1+2+3+4+5+6+7+8 = 36 bits = 5 bytes minimum
        std::vector<std::uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>());  // Max 1-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x03, bitStream.read<2>());  // Max 2-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x07, bitStream.read<3>());  // Max 3-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0F, bitStream.read<4>());  // Max 4-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x1F, bitStream.read<5>());  // Max 5-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x3F, bitStream.read<6>());  // Max 6-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x7F, bitStream.read<7>());  // Max 7-bit
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0xFF, bitStream.read<8>());  // Max 8-bit
    }

    void testReadAlternatingPattern()
    {
        std::vector<std::uint8_t> data = {0x55, 0xAA}; // 01010101 10101010
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 01010101 10101010
        // 01 = 1, 01 = 1, 01 = 1, 01 = 1, 10 = 2, 10 = 2, 10 = 2, 10 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<2>()); // 01 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<2>()); // 01 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<2>()); // 01 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<2>()); // 01 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<2>()); // 10 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<2>()); // 10 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<2>()); // 10 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<2>()); // 10 = 2
    }

    void testReadComplexPattern()
    {
        std::vector<std::uint8_t> data = {0x9A, 0xBC}; // 10011010 10111100
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 10011010 10111100
        // 100 = 4, 1101 = 13, 010 = 2, 111100 = 60
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x04, bitStream.read<3>()); // 100 = 4
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x0D, bitStream.read<4>()); // 1101 = 13
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<3>()); // 010 = 2
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x3C, bitStream.read<6>()); // 111100 = 60
    }

    // Edge Cases
    void testReadEmptyData()
    {
        // Test with PesPacketReader that has no data
        PesPacketReader reader; // Empty constructor
        BitStream bitStream(reader);
        
        try
        {
            bitStream.read<1>();
            CPPUNIT_FAIL("Expected exception when reading from empty data");
        }
        catch (const PesPacketReader::Exception&)
        {
            // Expected behavior
        }
    }

    void testReadWithTwoChunks()
    {
        std::vector<std::uint8_t> chunk1 = {0x12, 0x34};
        std::vector<std::uint8_t> chunk2 = {0x56, 0x78};
        PesPacketReader reader(chunk1.data(), chunk1.size(), chunk2.data(), chunk2.size());
        BitStream bitStream(reader);
        
        // Read across chunk boundary
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x34, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x56, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x78, bitStream.read<8>());
    }

    void testReadSubReader()
    {
        std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78};
        PesPacketReader mainReader(data.data(), data.size(), nullptr, 0U);
        PesPacketReader subReader(mainReader, 2); // Limit to first 2 bytes
        BitStream bitStream(subReader);
        
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x12, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x34, bitStream.read<8>());
        
        try
        {
            bitStream.read<1>();
            CPPUNIT_FAIL("Expected exception when reading beyond sub-reader limit");
        }
        catch (const PesPacketReader::Exception&)
        {
            // Expected behavior
        }
    }

    // Exception Propagation
    void testExceptionPropagation()
    {
        std::vector<std::uint8_t> data = {0xFF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Consume available data
        bitStream.read<8>();
        
        // Verify exception type and propagation
        try
        {
            bitStream.read<1>();
            CPPUNIT_FAIL("Expected PesPacketReader::Exception");
        }
        catch (const PesPacketReader::Exception& e)
        {
            // Verify it's the correct exception type
            const char* message = e.what();
            CPPUNIT_ASSERT(message != nullptr);
        }
        catch (...)
        {
            CPPUNIT_FAIL("Expected PesPacketReader::Exception, got different exception type");
        }
    }

    // Multiple Operations
    void testMultipleReads()
    {
        std::vector<std::uint8_t> data = {0xAB, 0xCD, 0xEF};
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 10101011 11001101 11101111
        // Perform complex sequence of reads
        std::vector<std::uint8_t> results;
        results.push_back(bitStream.read<3>());  // 101 = 5
        results.push_back(bitStream.read<2>());  // 01 = 1
        results.push_back(bitStream.read<1>());  // 0 = 0
        results.push_back(bitStream.read<4>());  // 1111 = 15
        results.push_back(bitStream.read<6>());  // 001101 = 13
        results.push_back(bitStream.read<3>());  // 111 = 7
        results.push_back(bitStream.read<5>());  // 01111 = 15
        
        std::vector<std::uint8_t> expected = {5, 1, 0, 15, 13, 7, 15};
        CPPUNIT_ASSERT_EQUAL(expected.size(), results.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expected[i], results[i]);
        }
    }

    void testBitAlignment()
    {
        std::vector<std::uint8_t> data = {0x81, 0x42, 0x24, 0x18}; // 10000001 01000010 00100100 00011000
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // 10000001 01000010 00100100 00011000
        // Test various alignment scenarios
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<1>()); // 1 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<7>()); // 0000001 = 1
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x42, bitStream.read<8>()); // 01000010 = 66
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x24, bitStream.read<8>()); // 00100100 = 36
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x18, bitStream.read<8>()); // 00011000 = 24
    }

    void testLargeDataSet()
    {
        // Test with larger dataset to ensure buffer management works correctly
        std::vector<std::uint8_t> data;
        for (int i = 0; i < 100; ++i)
        {
            data.push_back(static_cast<std::uint8_t>(i & 0xFF));
        }
        
        PesPacketReader reader(data.data(), data.size(), nullptr, 0U);
        BitStream bitStream(reader);
        
        // Read first few bytes to verify correct operation
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x00, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x01, bitStream.read<8>());
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x02, bitStream.read<8>());
        
        // Skip to near the end and verify
        for (int i = 3; i < 99; ++i)
        {
            bitStream.read<8>();
        }
        CPPUNIT_ASSERT_EQUAL((std::uint8_t)0x63, bitStream.read<8>()); // 99 & 0xFF = 0x63
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(BitStreamTest);
