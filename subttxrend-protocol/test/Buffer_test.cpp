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
#include "Buffer.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::protocol;

class BufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BufferTest);
    CPPUNIT_TEST(testConstructorWithValidData);
    CPPUNIT_TEST(testConstructorWithEmptyBuffer);
    CPPUNIT_TEST(testConstructorWithLargeBuffer);
    CPPUNIT_TEST(testConstructorWithSingleByte);
    CPPUNIT_TEST(testConstructorStoresCorrectSize);
    CPPUNIT_TEST(testGetSizeReturnsCorrectValue);
    CPPUNIT_TEST(testGetSizeWithZeroSize);
    CPPUNIT_TEST(testGetSizeWithLargeBuffer);
    CPPUNIT_TEST(testExtractLeUint32AtOffsetZero);
    CPPUNIT_TEST(testExtractLeUint32AtValidOffset);
    CPPUNIT_TEST(testExtractLeUint32OffsetEqualsSize);
    CPPUNIT_TEST(testExtractLeUint32OffsetExceedsSize);
    CPPUNIT_TEST(testExtractLeUint32InsufficientBytes);
    CPPUNIT_TEST(testExtractLeUint32ExactlyFourBytes);
    CPPUNIT_TEST(testExtractLeUint32AllZeros);
    CPPUNIT_TEST(testExtractLeUint32AllOnes);
    CPPUNIT_TEST(testExtractLeUint32MaxValue);
    CPPUNIT_TEST(testExtractLeUint32LittleEndianOrder);
    CPPUNIT_TEST(testExtractLeUint32AtEndOfBuffer);
    CPPUNIT_TEST(testExtractLeUint32WithOneByteLess);
    CPPUNIT_TEST(testExtractLeUint32WithTwoBytesLess);
    CPPUNIT_TEST(testExtractLeUint32WithThreeBytesLess);
    CPPUNIT_TEST(testExtractLeUint64AtOffsetZero);
    CPPUNIT_TEST(testExtractLeUint64AtValidOffset);
    CPPUNIT_TEST(testExtractLeUint64OffsetEqualsSize);
    CPPUNIT_TEST(testExtractLeUint64OffsetExceedsSize);
    CPPUNIT_TEST(testExtractLeUint64InsufficientBytes);
    CPPUNIT_TEST(testExtractLeUint64ExactlyEightBytes);
    CPPUNIT_TEST(testExtractLeUint64AllZeros);
    CPPUNIT_TEST(testExtractLeUint64AllOnes);
    CPPUNIT_TEST(testExtractLeUint64MaxValue);
    CPPUNIT_TEST(testExtractLeUint64LittleEndianOrder);
    CPPUNIT_TEST(testExtractLeUint64AtEndOfBuffer);
    CPPUNIT_TEST(testExtractLeInt64AtOffsetZero);
    CPPUNIT_TEST(testExtractLeInt64AtValidOffset);
    CPPUNIT_TEST(testExtractLeInt64OffsetEqualsSize);
    CPPUNIT_TEST(testExtractLeInt64OffsetExceedsSize);
    CPPUNIT_TEST(testExtractLeInt64InsufficientBytes);
    CPPUNIT_TEST(testExtractLeInt64PositiveValue);
    CPPUNIT_TEST(testExtractLeInt64NegativeValue);
    CPPUNIT_TEST(testExtractLeInt64MaxPositive);
    CPPUNIT_TEST(testExtractLeInt64MaxNegative);
    CPPUNIT_TEST(testExtractLeInt64LittleEndianOrder);
    CPPUNIT_TEST(testExtractLeInt64SignBitHandling);
    CPPUNIT_TEST(testExtractBufferAtOffsetZero);
    CPPUNIT_TEST(testExtractBufferAtValidOffset);
    CPPUNIT_TEST(testExtractBufferOffsetEqualsSize);
    CPPUNIT_TEST(testExtractBufferOffsetExceedsSize);
    CPPUNIT_TEST(testExtractBufferInsufficientBytes);
    CPPUNIT_TEST(testExtractBufferZeroSize);
    CPPUNIT_TEST(testExtractBufferFullBuffer);
    CPPUNIT_TEST(testExtractBufferSingleByte);
    CPPUNIT_TEST(testExtractBufferMultipleCalls);
    CPPUNIT_TEST(testExtractBufferOverwritesPrevious);
    CPPUNIT_TEST(testExtractBufferPreservesData);
    CPPUNIT_TEST(testExtractBufferAtEndOfBuffer);
    CPPUNIT_TEST(testExtractBufferExactFit);
    CPPUNIT_TEST(testMultipleUint32ExtractionsSequential);
    CPPUNIT_TEST(testMultipleUint64ExtractionsSequential);
    CPPUNIT_TEST(testMixedTypeExtractions);
    CPPUNIT_TEST(testExtractUint32ThenBuffer);
    CPPUNIT_TEST(testExtractUint64ThenBuffer);
    CPPUNIT_TEST(testExtractInt64ThenBuffer);
    CPPUNIT_TEST(testBufferThenMultipleIntegers);
    CPPUNIT_TEST(testAlternatingExtractionsAcrossBuffer);
    CPPUNIT_TEST(testSequentialBufferExtractions);
    CPPUNIT_TEST(testExtractAllTypesFromSameBuffer);
    CPPUNIT_TEST(testPartialFailureDoesNotCorrupt);
    CPPUNIT_TEST(testExtractionsAfterFailure);
    CPPUNIT_TEST(testLargeBufferMultipleExtractions);
    CPPUNIT_TEST(testBoundaryConditionsMixedTypes);
    CPPUNIT_TEST(testGetSizeRemainsConstant);
    CPPUNIT_TEST(testConsecutiveFailedExtractions);
    CPPUNIT_TEST(testInterleavedSuccessFailurePatterns);
    CPPUNIT_TEST(testRealWorldPacketParsing);
    CPPUNIT_TEST(testComplexDataStructureParsing);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup common test data
    }

    void tearDown()
    {
        // Cleanup
    }

    void testConstructorWithValidData()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), buffer.getSize());
    }

    void testConstructorWithEmptyBuffer()
    {
        char data[] = {0x00};
        Buffer buffer(data, 0);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), buffer.getSize());
    }

    void testConstructorWithLargeBuffer()
    {
        char data[1024];
        std::memset(data, 0xAB, 1024);
        Buffer buffer(data, 1024);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1024), buffer.getSize());
    }

    void testConstructorWithSingleByte()
    {
        char data[] = {0x42};
        Buffer buffer(data, 1);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), buffer.getSize());
    }

    void testConstructorStoresCorrectSize()
    {
        char data[100];
        Buffer buffer(data, 100);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100), buffer.getSize());
    }

    void testGetSizeReturnsCorrectValue()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        Buffer buffer(data, 5);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), buffer.getSize());
    }

    void testGetSizeWithZeroSize()
    {
        char data[] = {0x00};
        Buffer buffer(data, 0);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), buffer.getSize());
    }

    void testGetSizeWithLargeBuffer()
    {
        char data[10000];
        Buffer buffer(data, 10000);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10000), buffer.getSize());
    }

    void testExtractLeUint32AtOffsetZero()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value);
    }

    void testExtractLeUint32AtValidOffset()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 6);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(2, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value);
    }

    void testExtractLeUint32OffsetEqualsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(4, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32OffsetExceedsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(5, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32InsufficientBytes()
    {
        char data[] = {0x01, 0x02};
        Buffer buffer(data, 2);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32ExactlyFourBytes()
    {
        char data[] = {0x12, 0x34, 0x56, 0x78};
        Buffer buffer(data, 4);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x78563412), value);
    }

    void testExtractLeUint32AllZeros()
    {
        char data[] = {0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 4);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), value);
    }

    void testExtractLeUint32AllOnes()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 4);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32MaxValue()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 4);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32LittleEndianOrder()
    {
        // Little-endian: byte0=LSB, byte3=MSB
        char data[] = {0x01, 0x00, 0x00, 0x00}; // Should be 0x00000001
        Buffer buffer(data, 4);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000001), value);
    }

    void testExtractLeUint32AtEndOfBuffer()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), static_cast<char>(0xCC), static_cast<char>(0xDD), 0x11, 0x22, 0x33, 0x44};
        Buffer buffer(data, 8);
        std::uint32_t value = 0;

        bool result = buffer.extractLeUint32(4, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x44332211), value);
    }

    void testExtractLeUint32WithOneByteLess()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(0, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testExtractLeUint32WithTwoBytesLess()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        Buffer buffer(data, 5);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(3, value); // Only 2 bytes remain

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testExtractLeUint32WithThreeBytesLess()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = buffer.extractLeUint32(1, value); // Only 3 bytes remain

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testExtractLeUint64AtOffsetZero()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value);
    }

    void testExtractLeUint64AtValidOffset()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 10);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(2, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value);
    }

    void testExtractLeUint64OffsetEqualsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::uint64_t value = 0xFFFFFFFFFFFFFFFFULL;

        bool result = buffer.extractLeUint64(8, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64OffsetExceedsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::uint64_t value = 0xFFFFFFFFFFFFFFFFULL;

        bool result = buffer.extractLeUint64(10, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64InsufficientBytes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::uint64_t value = 0xFFFFFFFFFFFFFFFFULL;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64ExactlyEightBytes()
    {
        char data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88)};
        Buffer buffer(data, 8);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x8877665544332211ULL), value);
    }

    void testExtractLeUint64AllZeros()
    {
        char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 8);
        std::uint64_t value = 0xFFFFFFFFFFFFFFFFULL;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000000000000ULL), value);
    }

    void testExtractLeUint64AllOnes()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 8);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64MaxValue()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 8);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64LittleEndianOrder()
    {
        // Little-endian: byte0=LSB, byte7=MSB
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 8);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000000000001ULL), value);
    }

    void testExtractLeUint64AtEndOfBuffer()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88)};
        Buffer buffer(data, 10);
        std::uint64_t value = 0;

        bool result = buffer.extractLeUint64(2, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x8877665544332211ULL), value);
    }

    void testExtractLeInt64AtOffsetZero()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x0807060504030201LL), value);
    }

    void testExtractLeInt64AtValidOffset()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 10);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(2, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x0807060504030201LL), value);
    }

    void testExtractLeInt64OffsetEqualsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::int64_t value = -1;

        bool result = buffer.extractLeInt64(8, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }

    void testExtractLeInt64OffsetExceedsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::int64_t value = -1;

        bool result = buffer.extractLeInt64(10, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }

    void testExtractLeInt64InsufficientBytes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::int64_t value = -1;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }

    void testExtractLeInt64PositiveValue()
    {
        // Positive value: MSB = 0x7F (sign bit = 0)
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x7F00000000000001LL), value);
        CPPUNIT_ASSERT(value > 0);
    }

    void testExtractLeInt64NegativeValue()
    {
        // Negative value: MSB = 0xFF (sign bit = 1)
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }

    void testExtractLeInt64MaxPositive()
    {
        // Max positive: 0x7FFFFFFFFFFFFFFF
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), 0x7F};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x7FFFFFFFFFFFFFFFLL), value);
    }

    void testExtractLeInt64MaxNegative()
    {
        // Min negative: 0x8000000000000000 = -9223372036854775808
        char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<char>(0x80)};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x8000000000000000LL), value);
    }

    void testExtractLeInt64LittleEndianOrder()
    {
        // Little-endian: byte0=LSB, byte7=MSB
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x0000000000000001LL), value);
    }

    void testExtractLeInt64SignBitHandling()
    {
        // Value with sign bit set: 0x8000000000000001 (negative)
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<char>(0x80)};
        Buffer buffer(data, 8);
        std::int64_t value = 0;

        bool result = buffer.extractLeInt64(0, value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x8000000000000001LL), value);
        CPPUNIT_ASSERT(value < 0);
    }

    void testExtractBufferAtOffsetZero()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(0, 4, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), outBuffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), outBuffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), outBuffer[3]);
    }

    void testExtractBufferAtValidOffset()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 6);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(2, 4, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), outBuffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), outBuffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), outBuffer[3]);
    }

    void testExtractBufferOffsetEqualsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = buffer.extractBuffer(4, 1, outBuffer);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), outBuffer[0]);
    }

    void testExtractBufferOffsetExceedsSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = buffer.extractBuffer(5, 1, outBuffer);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), outBuffer[0]);
    }

    void testExtractBufferInsufficientBytes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = buffer.extractBuffer(2, 5, outBuffer); // Only 2 bytes remain

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), outBuffer[0]);
    }

    void testExtractBufferZeroSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = buffer.extractBuffer(0, 0, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), outBuffer.size());
    }

    void testExtractBufferFullBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        Buffer buffer(data, 5);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(0, 5, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), outBuffer.size());
        for (size_t i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(i + 1), outBuffer[i]);
        }
    }

    void testExtractBufferSingleByte()
    {
        char data[] = {0x42};
        Buffer buffer(data, 1);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(0, 1, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), outBuffer[0]);
    }

    void testExtractBufferMultipleCalls()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        std::vector<char> outBuffer1;
        std::vector<char> outBuffer2;

        bool result1 = buffer.extractBuffer(0, 3, outBuffer1);
        bool result2 = buffer.extractBuffer(3, 3, outBuffer2);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), outBuffer1.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), outBuffer2.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), outBuffer1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), outBuffer2[0]);
    }

    void testExtractBufferOverwritesPrevious()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xAA);
        outBuffer.push_back(0xBB);

        bool result = buffer.extractBuffer(0, 2, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), outBuffer[1]);
    }

    void testExtractBufferPreservesData()
    {
        char data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        Buffer buffer(data, 5);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(1, 3, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x33), outBuffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x44), outBuffer[2]);
    }

    void testExtractBufferAtEndOfBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(4, 2, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x05), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x06), outBuffer[1]);
    }

    void testExtractBufferExactFit()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        Buffer buffer(data, 5);
        std::vector<char> outBuffer;

        bool result = buffer.extractBuffer(3, 2, outBuffer); // Exactly fits

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x05), outBuffer[1]);
    }

    void testMultipleUint32ExtractionsSequential()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
        Buffer buffer(data, 12);
        std::uint32_t value1 = 0, value2 = 0, value3 = 0;

        bool result1 = buffer.extractLeUint32(0, value1);
        bool result2 = buffer.extractLeUint32(4, value2);
        bool result3 = buffer.extractLeUint32(8, value3);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(true, result3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), value2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x0C0B0A09), value3);
    }

    void testMultipleUint64ExtractionsSequential()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};
        Buffer buffer(data, 16);
        std::uint64_t value1 = 0, value2 = 0;

        bool result1 = buffer.extractLeUint64(0, value1);
        bool result2 = buffer.extractLeUint64(8, value2);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x1817161514131211ULL), value2);
    }

    void testMixedTypeExtractions()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x11, 0x12, 0x13, 0x14};
        Buffer buffer(data, 12);
        std::uint32_t value32 = 0;
        std::uint64_t value64 = 0;

        bool result1 = buffer.extractLeUint32(0, value32);
        bool result2 = buffer.extractLeUint64(4, value64);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x1413121108070605ULL), value64);
    }

    void testExtractUint32ThenBuffer()
    {
        char data[] = {0x04, 0x00, 0x00, 0x00, static_cast<char>(0xAA), static_cast<char>(0xBB), static_cast<char>(0xCC), static_cast<char>(0xDD)};
        Buffer buffer(data, 8);
        std::uint32_t size = 0;
        std::vector<char> extractedData;

        bool result1 = buffer.extractLeUint32(0, size);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), size);

        bool result2 = buffer.extractBuffer(4, size, extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), extractedData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xBB), extractedData[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xCC), extractedData[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xDD), extractedData[3]);
    }

    void testExtractUint64ThenBuffer()
    {
        char data[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
        Buffer buffer(data, 11);
        std::uint64_t size = 0;
        std::vector<char> extractedData;

        bool result1 = buffer.extractLeUint64(0, size);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3), size);

        bool result2 = buffer.extractBuffer(8, static_cast<size_t>(size), extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), extractedData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), extractedData[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x33), extractedData[2]);
    }

    void testExtractInt64ThenBuffer()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       0x44, 0x55, 0x66};
        Buffer buffer(data, 11);
        std::int64_t signedValue = 0;
        std::vector<char> extractedData;

        bool result1 = buffer.extractLeInt64(0, signedValue);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), signedValue);

        bool result2 = buffer.extractBuffer(8, 3, extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x44), extractedData[0]);
    }

    void testBufferThenMultipleIntegers()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 10);
        std::vector<char> header;
        std::uint32_t value32 = 0;
        std::uint32_t value32_2 = 0;

        bool result1 = buffer.extractBuffer(0, 2, header);
        bool result2 = buffer.extractLeUint32(2, value32);
        bool result3 = buffer.extractLeUint32(6, value32_2);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(true, result3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), header.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), header[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), value32_2);
    }

    void testAlternatingExtractionsAcrossBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, static_cast<char>(0xAA), static_cast<char>(0xBB), 0x05, 0x06, 0x07, 0x08, static_cast<char>(0xCC), static_cast<char>(0xDD)};
        Buffer buffer(data, 12);
        std::uint32_t val1 = 0, val2 = 0;
        std::vector<char> buf1, buf2;

        bool r1 = buffer.extractLeUint32(0, val1);
        bool r2 = buffer.extractBuffer(4, 2, buf1);
        bool r3 = buffer.extractLeUint32(6, val2);
        bool r4 = buffer.extractBuffer(10, 2, buf2);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), val1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), val2);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), buf1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xCC), buf2[0]);
    }

    void testSequentialBufferExtractions()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::vector<char> chunk1, chunk2, chunk3, chunk4;

        bool r1 = buffer.extractBuffer(0, 2, chunk1);
        bool r2 = buffer.extractBuffer(2, 2, chunk2);
        bool r3 = buffer.extractBuffer(4, 2, chunk3);
        bool r4 = buffer.extractBuffer(6, 2, chunk4);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), chunk1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), chunk2[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x05), chunk3[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x07), chunk4[0]);
    }

    void testExtractAllTypesFromSameBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                       0x11, 0x12};
        Buffer buffer(data, 18);
        std::uint32_t u32 = 0;
        std::uint64_t u64 = 0;
        std::int64_t i64 = 0;
        std::vector<char> buf;

        bool r1 = buffer.extractLeUint32(0, u32);
        bool r2 = buffer.extractLeUint64(4, u64);
        bool r3 = buffer.extractLeInt64(4, i64);
        bool r4 = buffer.extractBuffer(12, 6, buf);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), u32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0C0B0A0908070605ULL), u64);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x0C0B0A0908070605LL), i64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), buf.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0D), buf[0]);
    }

    void testPartialFailureDoesNotCorrupt()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        std::uint32_t val1 = 0xDEADBEEF;
        std::uint32_t val2 = 0xCAFEBABE;

        bool r1 = buffer.extractLeUint32(0, val1);
        bool r2 = buffer.extractLeUint32(4, val2);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), val1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), val2);
    }

    void testExtractionsAfterFailure()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        std::uint64_t failVal = 0xFFFFFFFFFFFFFFFFULL;
        std::uint32_t successVal = 0;

        bool r1 = buffer.extractLeUint64(5, failVal);
        CPPUNIT_ASSERT_EQUAL(false, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), failVal);

        bool r2 = buffer.extractLeUint32(0, successVal);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), successVal);
    }

    void testLargeBufferMultipleExtractions()
    {
        char data[256];
        for (int i = 0; i < 256; i++)
        {
            data[i] = static_cast<char>(i);
        }
        Buffer buffer(data, 256);

        std::uint32_t val32_1 = 0, val32_2 = 0;
        std::uint64_t val64 = 0;
        std::vector<char> chunk;

        bool r1 = buffer.extractLeUint32(0, val32_1);
        bool r2 = buffer.extractLeUint64(100, val64);
        bool r3 = buffer.extractLeUint32(200, val32_2);
        bool r4 = buffer.extractBuffer(250, 6, chunk);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x03020100), val32_1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x6B6A696867666564ULL), val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(250), chunk[0]);
    }

    void testBoundaryConditionsMixedTypes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C};
        Buffer buffer(data, 12);
        std::uint32_t val32 = 0;
        std::uint64_t val64_fail = 0xFFFFFFFFFFFFFFFFULL;
        std::vector<char> buf;

        bool r1 = buffer.extractLeUint32(8, val32);
        bool r2 = buffer.extractLeUint64(5, val64_fail);
        bool r3 = buffer.extractBuffer(0, 12, buf);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x0C0B0A09), val32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), val64_fail);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), buf.size());
    }

    void testGetSizeRemainsConstant()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);

        size_t initialSize = buffer.getSize();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), initialSize);

        std::uint32_t val = 0;
        buffer.extractLeUint32(0, val);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), buffer.getSize());

        std::uint64_t val64 = 0;
        buffer.extractLeUint64(0, val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), buffer.getSize());

        std::vector<char> buf;
        buffer.extractBuffer(0, 4, buf);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), buffer.getSize());
    }

    void testConsecutiveFailedExtractions()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        std::uint32_t val32 = 0xAAAAAAAA;
        std::uint64_t val64 = 0xBBBBBBBBBBBBBBBBULL;
        std::int64_t vali64 = 0xCCCCCCCCCCCCCCCCLL;
        std::vector<char> buf;
        buf.push_back(0xDD);

        bool r1 = buffer.extractLeUint32(0, val32);
        bool r2 = buffer.extractLeUint64(0, val64);
        bool r3 = buffer.extractLeInt64(0, vali64);
        bool r4 = buffer.extractBuffer(0, 10, buf);

        CPPUNIT_ASSERT_EQUAL(false, r1);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(false, r3);
        CPPUNIT_ASSERT_EQUAL(false, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAAAAAAAA), val32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xBBBBBBBBBBBBBBBBULL), val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0xCCCCCCCCCCCCCCCCLL), vali64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), buf.size());
    }

    void testInterleavedSuccessFailurePatterns()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
        Buffer buffer(data, 9);
        std::uint32_t val1 = 0, val2 = 0xFFFFFFFF, val3 = 0;
        std::vector<char> buf1, buf2;

        bool r1 = buffer.extractLeUint32(0, val1);
        bool r2 = buffer.extractLeUint32(6, val2);
        bool r3 = buffer.extractBuffer(1, 3, buf1);
        bool r4 = buffer.extractBuffer(7, 5, buf2);
        bool r5 = buffer.extractLeUint32(4, val3);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(false, r4);
        CPPUNIT_ASSERT_EQUAL(true, r5);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), val1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), val2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), buf1.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), val3);
    }

    void testRealWorldPacketParsing()
    {
        char packetData[] = {
            0x10, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00,
            0x48, 0x65, 0x6C, 0x6C, 0x6F
        };
        Buffer buffer(packetData, sizeof(packetData));

        std::uint32_t totalSize = 0;
        std::uint32_t packetType = 0;
        std::uint64_t timestamp = 0;
        std::vector<char> payload;

        bool r1 = buffer.extractLeUint32(0, totalSize);
        bool r2 = buffer.extractLeUint32(4, packetType);
        bool r3 = buffer.extractLeUint64(8, timestamp);
        bool r4 = buffer.extractBuffer(16, 5, payload);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(16), totalSize);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packetType);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000012345678ULL), timestamp);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), payload.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>('H'), payload[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>('e'), payload[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>('l'), payload[2]);
    }

    void testComplexDataStructureParsing()
    {
        char structData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x0A, 0x00,
            0x0B, 0x00,
            0x0C, 0x00,
            0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            static_cast<char>(0xFF), static_cast<char>(0xFF),
            static_cast<char>(0xFF), static_cast<char>(0xFF),
            static_cast<char>(0xFF), static_cast<char>(0xFF),
            static_cast<char>(0xFF), static_cast<char>(0xFF)
        };
        Buffer buffer(structData, sizeof(structData));

        std::uint32_t arrayCount = 0;
        std::vector<char> item1, item2, item3;
        std::uint64_t bigValue = 0;
        std::int64_t signedValue = 0;

        bool r1 = buffer.extractLeUint32(0, arrayCount);
        bool r2 = buffer.extractBuffer(4, 2, item1);
        bool r3 = buffer.extractBuffer(6, 2, item2);
        bool r4 = buffer.extractBuffer(8, 2, item3);
        bool r5 = buffer.extractLeUint64(10, bigValue);
        bool r6 = buffer.extractLeInt64(18, signedValue);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(true, r5);
        CPPUNIT_ASSERT_EQUAL(true, r6);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), arrayCount);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0A), item1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0B), item2[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0C), item3[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(100), bigValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), signedValue);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BufferTest);
