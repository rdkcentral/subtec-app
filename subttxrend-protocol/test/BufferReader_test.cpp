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
#include "BufferReader.hpp"
#include "Buffer.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::protocol;

class BufferReaderTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(BufferReaderTest);
    CPPUNIT_TEST(testConstructorWithValidBuffer);
    CPPUNIT_TEST(testConstructorWithEmptyBuffer);
    CPPUNIT_TEST(testGetBufferReturnsCorrectReference);
    CPPUNIT_TEST(testGetSizeReturnsCorrectSize);
    CPPUNIT_TEST(testGetSizeWithEmptyBuffer);
    CPPUNIT_TEST(testGetOffsetReturnsInitialZero);
    CPPUNIT_TEST(testExtractLeUint32WithValidData);
    CPPUNIT_TEST(testExtractLeUint32InsufficientBytes);
    CPPUNIT_TEST(testExtractLeUint32OffsetIncrementsCorrectly);
    CPPUNIT_TEST(testExtractLeUint32AtBufferBoundary);
    CPPUNIT_TEST(testExtractLeUint32BeyondBufferBoundary);
    CPPUNIT_TEST(testExtractLeUint32WithAllZeros);
    CPPUNIT_TEST(testExtractLeUint32WithAllOnes);
    CPPUNIT_TEST(testExtractLeUint32LittleEndianOrder);
    CPPUNIT_TEST(testExtractLeUint32FailurePreservesValue);
    CPPUNIT_TEST(testExtractLeUint32FailurePreservesOffset);
    CPPUNIT_TEST(testExtractLeUint64WithValidData);
    CPPUNIT_TEST(testExtractLeUint64InsufficientBytes);
    CPPUNIT_TEST(testExtractLeUint64OffsetIncrementsCorrectly);
    CPPUNIT_TEST(testExtractLeUint64AtBufferBoundary);
    CPPUNIT_TEST(testExtractLeUint64BeyondBufferBoundary);
    CPPUNIT_TEST(testExtractLeUint64WithMaxValue);
    CPPUNIT_TEST(testExtractLeUint64LittleEndianOrder);
    CPPUNIT_TEST(testExtractLeUint64FailurePreservesValue);
    CPPUNIT_TEST(testExtractLeUint64FailurePreservesOffset);
    CPPUNIT_TEST(testExtractLeInt64WithValidPositiveValue);
    CPPUNIT_TEST(testExtractLeInt64WithValidNegativeValue);
    CPPUNIT_TEST(testExtractLeInt64InsufficientBytes);
    CPPUNIT_TEST(testExtractLeInt64WithMaxPositive);
    CPPUNIT_TEST(testExtractLeInt64WithMinNegative);
    CPPUNIT_TEST(testExtractLeInt64OffsetIncrementsCorrectly);
    CPPUNIT_TEST(testExtractLeInt64FailurePreservesValue);
    CPPUNIT_TEST(testExtractLeInt64FailurePreservesOffset);
    CPPUNIT_TEST(testExtractBufferWithValidSize);
    CPPUNIT_TEST(testExtractBufferWithZeroSize);
    CPPUNIT_TEST(testExtractBufferSizeExceedsRemaining);
    CPPUNIT_TEST(testExtractBufferOffsetIncrementsCorrectly);
    CPPUNIT_TEST(testExtractBufferAtEndOfBuffer);
    CPPUNIT_TEST(testExtractBufferPreservesDataIntegrity);
    CPPUNIT_TEST(testExtractBufferFailurePreservesOffset);
    CPPUNIT_TEST(testSequentialExtractLeUint32Calls);
    CPPUNIT_TEST(testSequentialExtractLeUint64Calls);
    CPPUNIT_TEST(testMixedTypeExtractions);
    CPPUNIT_TEST(testExtractLeUint32ThenExtractBuffer);
    CPPUNIT_TEST(testExtractBufferThenExtractLeUint32);
    CPPUNIT_TEST(testExtractLeUint64ThenExtractBuffer);
    CPPUNIT_TEST(testExtractLeInt64ThenExtractBuffer);
    CPPUNIT_TEST(testMultipleFailedExtractionsPreserveOffset);
    CPPUNIT_TEST(testFailedExtractionDoesNotCorruptSubsequentSuccess);
    CPPUNIT_TEST(testGetOffsetAfterMultipleOperations);
    CPPUNIT_TEST(testExtractOperationsAfterReachingEndOfBuffer);
    CPPUNIT_TEST(testOffsetTrackingAcrossBoundaryFailures);
    CPPUNIT_TEST(testLargeBufferMultipleVariedExtractions);
    CPPUNIT_TEST(testRealWorldPacketParsingSimulation);
    CPPUNIT_TEST(testAlternatingExtractionsAcrossBuffer);
    CPPUNIT_TEST(testSequentialBufferExtractions);
    CPPUNIT_TEST(testExtractAllTypesFromSameBuffer);

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

    void testConstructorWithValidBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testConstructorWithEmptyBuffer()
    {
        char data[] = {0x00};
        Buffer buffer(data, 0);
        BufferReader reader(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testGetBufferReturnsCorrectReference()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);

        const Buffer& returnedBuffer = reader.getBuffer();

        CPPUNIT_ASSERT_EQUAL(buffer.getSize(), returnedBuffer.getSize());
    }

    void testGetSizeReturnsCorrectSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getSize());
    }

    void testGetSizeWithEmptyBuffer()
    {
        char data[] = {0x00};
        Buffer buffer(data, 0);
        BufferReader reader(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getSize());
    }

    void testGetOffsetReturnsInitialZero()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testExtractLeUint32WithValidData()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value);
    }

    void testExtractLeUint32InsufficientBytes()
    {
        char data[] = {0x01, 0x02};
        Buffer buffer(data, 2);
        BufferReader reader(buffer);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32OffsetIncrementsCorrectly()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());
    }

    void testExtractLeUint32AtBufferBoundary()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());
    }

    void testExtractLeUint32BeyondBufferBoundary()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::uint32_t value = 0xDEADBEEF;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), value);
    }

    void testExtractLeUint32WithAllZeros()
    {
        char data[] = {0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t value = 0xFFFFFFFF;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000000), value);
    }

    void testExtractLeUint32WithAllOnes()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), value);
    }

    void testExtractLeUint32LittleEndianOrder()
    {
        char data[] = {0x01, 0x00, 0x00, 0x00};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000001), value);
    }

    void testExtractLeUint32FailurePreservesValue()
    {
        char data[] = {0x01, 0x02};
        Buffer buffer(data, 2);
        BufferReader reader(buffer);
        std::uint32_t value = 0x12345678;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), value);
    }

    void testExtractLeUint32FailurePreservesOffset()
    {
        char data[] = {0x01, 0x02};
        Buffer buffer(data, 2);
        BufferReader reader(buffer);
        std::uint32_t value = 0;

        bool result = reader.extractLeUint32(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testExtractLeUint64WithValidData()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value);
    }

    void testExtractLeUint64InsufficientBytes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint64_t value = 0xFFFFFFFFFFFFFFFFULL;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64OffsetIncrementsCorrectly()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
        Buffer buffer(data, 10);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testExtractLeUint64AtBufferBoundary()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testExtractLeUint64BeyondBufferBoundary()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        Buffer buffer(data, 5);
        BufferReader reader(buffer);
        std::uint64_t value = 0xCAFEBABEDEADBEEFULL;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xCAFEBABEDEADBEEFULL), value);
    }

    void testExtractLeUint64WithMaxValue()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), value);
    }

    void testExtractLeUint64LittleEndianOrder()
    {
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000000000001ULL), value);
    }

    void testExtractLeUint64FailurePreservesValue()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::uint64_t value = 0x123456789ABCDEF0ULL;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x123456789ABCDEF0ULL), value);
    }

    void testExtractLeUint64FailurePreservesOffset()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::uint64_t value = 0;

        bool result = reader.extractLeUint64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testExtractLeInt64WithValidPositiveValue()
    {
        char data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x7F00000000000001LL), value);
        CPPUNIT_ASSERT(value > 0);
    }

    void testExtractLeInt64WithValidNegativeValue()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF)};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }

    void testExtractLeInt64InsufficientBytes()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::int64_t value = -999;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-999), value);
    }

    void testExtractLeInt64WithMaxPositive()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), 0x7F};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x7FFFFFFFFFFFFFFFLL), value);
    }

    void testExtractLeInt64WithMinNegative()
    {
        char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<char>(0x80)};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x8000000000000000LL), value);
    }

    void testExtractLeInt64OffsetIncrementsCorrectly()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
        Buffer buffer(data, 10);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testExtractLeInt64FailurePreservesValue()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::int64_t value = 0x1122334455667788LL;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0x1122334455667788LL), value);
    }

    void testExtractLeInt64FailurePreservesOffset()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::int64_t value = 0;

        bool result = reader.extractLeInt64(value);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testExtractBufferWithValidSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;

        bool result = reader.extractBuffer(4, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), outBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), outBuffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), outBuffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), outBuffer[3]);
    }

    void testExtractBufferWithZeroSize()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = reader.extractBuffer(0, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testExtractBufferSizeExceedsRemaining()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;
        outBuffer.push_back(0xFF);

        bool result = reader.extractBuffer(5, outBuffer);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), outBuffer[0]);
    }

    void testExtractBufferOffsetIncrementsCorrectly()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());

        bool result = reader.extractBuffer(3, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), reader.getOffset());
    }

    void testExtractBufferAtEndOfBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        Buffer buffer(data, 6);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;

        bool result = reader.extractBuffer(6, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), outBuffer.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), reader.getOffset());
    }

    void testExtractBufferPreservesDataIntegrity()
    {
        char data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        Buffer buffer(data, 5);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;

        bool result = reader.extractBuffer(5, outBuffer);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), outBuffer.size());
        for (size_t i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(data[i], outBuffer[i]);
        }
    }

    void testExtractBufferFailurePreservesOffset()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::vector<char> outBuffer;

        bool result = reader.extractBuffer(5, outBuffer);

        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
    }

    void testSequentialExtractLeUint32Calls()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
        Buffer buffer(data, 12);
        BufferReader reader(buffer);
        std::uint32_t value1 = 0, value2 = 0, value3 = 0;

        bool result1 = reader.extractLeUint32(value1);
        bool result2 = reader.extractLeUint32(value2);
        bool result3 = reader.extractLeUint32(value3);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(true, result3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), value2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x0C0B0A09), value3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), reader.getOffset());
    }

    void testSequentialExtractLeUint64Calls()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};
        Buffer buffer(data, 16);
        BufferReader reader(buffer);
        std::uint64_t value1 = 0, value2 = 0;

        bool result1 = reader.extractLeUint64(value1);
        bool result2 = reader.extractLeUint64(value2);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), value1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x1817161514131211ULL), value2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), reader.getOffset());
    }

    void testMixedTypeExtractions()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x11, 0x12, 0x13, 0x14};
        Buffer buffer(data, 12);
        BufferReader reader(buffer);
        std::uint32_t value32 = 0;
        std::uint64_t value64 = 0;

        bool result1 = reader.extractLeUint32(value32);
        bool result2 = reader.extractLeUint64(value64);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x1413121108070605ULL), value64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), reader.getOffset());
    }

    void testExtractLeUint32ThenExtractBuffer()
    {
        char data[] = {0x04, 0x00, 0x00, 0x00, static_cast<char>(0xAA), static_cast<char>(0xBB), static_cast<char>(0xCC), static_cast<char>(0xDD)};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint32_t size = 0;
        std::vector<char> extractedData;

        bool result1 = reader.extractLeUint32(size);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), size);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        bool result2 = reader.extractBuffer(size, extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), extractedData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xBB), extractedData[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xCC), extractedData[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xDD), extractedData[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testExtractBufferThenExtractLeUint32()
    {
        char data[] = {static_cast<char>(0xAA), static_cast<char>(0xBB), 0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 6);
        BufferReader reader(buffer);
        std::vector<char> header;
        std::uint32_t value32 = 0;

        bool result1 = reader.extractBuffer(2, header);
        bool result2 = reader.extractLeUint32(value32);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), header.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), header[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), value32);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), reader.getOffset());
    }

    void testExtractLeUint64ThenExtractBuffer()
    {
        char data[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
        Buffer buffer(data, 11);
        BufferReader reader(buffer);
        std::uint64_t size = 0;
        std::vector<char> extractedData;

        bool result1 = reader.extractLeUint64(size);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3), size);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());

        bool result2 = reader.extractBuffer(static_cast<size_t>(size), extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), extractedData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), extractedData[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x33), extractedData[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(11), reader.getOffset());
    }

    void testExtractLeInt64ThenExtractBuffer()
    {
        char data[] = {static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       static_cast<char>(0xFF), static_cast<char>(0xFF),
                       0x44, 0x55, 0x66};
        Buffer buffer(data, 11);
        BufferReader reader(buffer);
        std::int64_t signedValue = 0;
        std::vector<char> extractedData;

        bool result1 = reader.extractLeInt64(signedValue);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(-1), signedValue);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());

        bool result2 = reader.extractBuffer(3, extractedData);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), extractedData.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x44), extractedData[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(11), reader.getOffset());
    }

    void testMultipleFailedExtractionsPreserveOffset()
    {
        char data[] = {0x01, 0x02, 0x03};
        Buffer buffer(data, 3);
        BufferReader reader(buffer);
        std::uint32_t val32 = 0xAAAAAAAA;
        std::uint64_t val64 = 0xBBBBBBBBBBBBBBBBULL;
        std::int64_t vali64 = 0xCCCCCCCCCCCCCCCCLL;
        std::vector<char> buf;
        buf.push_back(0xDD);

        bool r1 = reader.extractLeUint32(val32);
        bool r2 = reader.extractLeUint64(val64);
        bool r3 = reader.extractLeInt64(vali64);
        bool r4 = reader.extractBuffer(10, buf);

        CPPUNIT_ASSERT_EQUAL(false, r1);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(false, r3);
        CPPUNIT_ASSERT_EQUAL(false, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAAAAAAAA), val32);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xBBBBBBBBBBBBBBBBULL), val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int64_t>(0xCCCCCCCCCCCCCCCCLL), vali64);
    }

    void testFailedExtractionDoesNotCorruptSubsequentSuccess()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::uint64_t failVal = 0xFFFFFFFFFFFFFFFFULL;
        std::uint32_t successVal = 0;

        // Try to extract uint64 at offset 0, but first extract a uint32
        bool r1 = reader.extractLeUint32(successVal);
        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), successVal);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        // Now try to extract uint64 with only 4 bytes remaining - should fail
        bool r2 = reader.extractLeUint64(failVal);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), failVal);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        // Extract another uint32 - should succeed
        std::uint32_t successVal2 = 0;
        bool r3 = reader.extractLeUint32(successVal2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), successVal2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testGetOffsetAfterMultipleOperations()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
        Buffer buffer(data, 14);
        BufferReader reader(buffer);
        std::uint32_t val32 = 0;
        std::uint64_t val64 = 0;
        std::vector<char> buf;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), reader.getOffset());

        reader.extractLeUint32(val32);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        reader.extractLeUint64(val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), reader.getOffset());

        reader.extractBuffer(2, buf);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), reader.getOffset());
    }

    void testExtractOperationsAfterReachingEndOfBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04};
        Buffer buffer(data, 4);
        BufferReader reader(buffer);
        std::uint32_t val1 = 0, val2 = 0xFFFFFFFF;
        std::vector<char> buf;

        // Extract all data
        bool r1 = reader.extractLeUint32(val1);
        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        // Try to extract more - should fail
        bool r2 = reader.extractLeUint32(val2);
        CPPUNIT_ASSERT_EQUAL(false, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), val2);

        bool r3 = reader.extractBuffer(1, buf);
        CPPUNIT_ASSERT_EQUAL(false, r3);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());
    }

    void testOffsetTrackingAcrossBoundaryFailures()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
        Buffer buffer(data, 9);
        BufferReader reader(buffer);
        std::uint32_t val1 = 0, val2 = 0xFFFFFFFF, val3 = 0;
        std::vector<char> buf1, buf2;

        bool r1 = reader.extractLeUint32(val1);
        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        // Try to extract uint32 with only 5 bytes remaining - should fail
        bool r2 = reader.extractLeUint32(val2);
        bool r3 = reader.extractBuffer(6, buf1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(false, r3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());

        // Extract remaining byte
        bool r4 = reader.extractBuffer(1, buf2);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(9), reader.getOffset());
    }

    void testLargeBufferMultipleVariedExtractions()
    {
        char data[256];
        for (int i = 0; i < 256; i++)
        {
            data[i] = static_cast<char>(i);
        }
        Buffer buffer(data, 256);
        BufferReader reader(buffer);

        std::uint32_t val32_1 = 0, val32_2 = 0;
        std::uint64_t val64 = 0;
        std::vector<char> chunk;

        bool r1 = reader.extractLeUint32(val32_1);
        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x03020100), val32_1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        bool r2 = reader.extractBuffer(96, chunk);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(96), chunk.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100), reader.getOffset());

        bool r3 = reader.extractLeUint64(val64);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x6B6A696867666564ULL), val64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(108), reader.getOffset());

        bool r4 = reader.extractLeUint32(val32_2);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(112), reader.getOffset());
    }

    void testRealWorldPacketParsingSimulation()
    {
        char packetData[] = {
            0x10, 0x00, 0x00, 0x00,                          // total size
            0x01, 0x00, 0x00, 0x00,                          // packet type
            0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, // timestamp
            0x48, 0x65, 0x6C, 0x6C, 0x6F                     // payload "Hello"
        };
        Buffer buffer(packetData, sizeof(packetData));
        BufferReader reader(buffer);

        std::uint32_t totalSize = 0;
        std::uint32_t packetType = 0;
        std::uint64_t timestamp = 0;
        std::vector<char> payload;

        bool r1 = reader.extractLeUint32(totalSize);
        bool r2 = reader.extractLeUint32(packetType);
        bool r3 = reader.extractLeUint64(timestamp);
        bool r4 = reader.extractBuffer(5, payload);

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
        CPPUNIT_ASSERT_EQUAL(static_cast<char>('l'), payload[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>('o'), payload[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(21), reader.getOffset());
    }

    void testAlternatingExtractionsAcrossBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, static_cast<char>(0xAA), static_cast<char>(0xBB), 0x05, 0x06, 0x07, 0x08, static_cast<char>(0xCC), static_cast<char>(0xDD)};
        Buffer buffer(data, 12);
        BufferReader reader(buffer);
        std::uint32_t val1 = 0, val2 = 0;
        std::vector<char> buf1, buf2;

        bool r1 = reader.extractLeUint32(val1);
        bool r2 = reader.extractBuffer(2, buf1);
        bool r3 = reader.extractLeUint32(val2);
        bool r4 = reader.extractBuffer(2, buf2);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), val1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), val2);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xAA), buf1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xBB), buf1[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xCC), buf2[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xDD), buf2[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), reader.getOffset());
    }

    void testSequentialBufferExtractions()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        Buffer buffer(data, 8);
        BufferReader reader(buffer);
        std::vector<char> chunk1, chunk2, chunk3, chunk4;

        bool r1 = reader.extractBuffer(2, chunk1);
        bool r2 = reader.extractBuffer(2, chunk2);
        bool r3 = reader.extractBuffer(2, chunk3);
        bool r4 = reader.extractBuffer(2, chunk4);

        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(true, r4);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), chunk1[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), chunk1[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), chunk2[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), chunk2[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x05), chunk3[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x06), chunk3[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x07), chunk4[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x08), chunk4[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), reader.getOffset());
    }

    void testExtractAllTypesFromSameBuffer()
    {
        char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                       0x11, 0x12};
        Buffer buffer(data, 18);
        BufferReader reader(buffer);
        std::uint32_t u32 = 0;
        std::uint64_t u64 = 0;
        std::int64_t i64 = 0;
        std::vector<char> buf;

        bool r1 = reader.extractLeUint32(u32);
        CPPUNIT_ASSERT_EQUAL(true, r1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), u32);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), reader.getOffset());

        bool r2 = reader.extractLeUint64(u64);
        CPPUNIT_ASSERT_EQUAL(true, r2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0C0B0A0908070605ULL), u64);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), reader.getOffset());

        bool r3 = reader.extractBuffer(6, buf);
        CPPUNIT_ASSERT_EQUAL(true, r3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), buf.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0D), buf[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0E), buf[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(18), reader.getOffset());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BufferReaderTest);
