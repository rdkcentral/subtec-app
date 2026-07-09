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
#include "ScteBitStream.hpp"
#include "ScteExceptions.hpp"

using namespace subttxrend::scte;

class ScteBitStreamTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteBitStreamTest );
    CPPUNIT_TEST(testConstructorWithEmptyVector);
    CPPUNIT_TEST(testConstructorWithSingleByte);
    CPPUNIT_TEST(testConstructorWithMultipleBytes);
    CPPUNIT_TEST(testConstructorWithLargeVector);
    CPPUNIT_TEST(testShiftPositiveWithinBounds);
    CPPUNIT_TEST(testShiftNegativeWithinBounds);
    CPPUNIT_TEST(testShiftZero);
    CPPUNIT_TEST(testShiftBeyondEnd);
    CPPUNIT_TEST(testShiftBeforeStart);
    CPPUNIT_TEST(testShiftExactlyToEnd);
    CPPUNIT_TEST(testShiftToZeroFromMiddle);
    CPPUNIT_TEST(testMultipleConsecutiveShifts);
    CPPUNIT_TEST(testDataReferenceReturnsModifiable);
    CPPUNIT_TEST(testDataReferenceOnEmpty);
    CPPUNIT_TEST(testDataReferenceExternalModification);
    CPPUNIT_TEST(testDataSizeOneBit);
    CPPUNIT_TEST(testDataSizeOneByte);
    CPPUNIT_TEST(testDataSizeThirtyTwoBits);
    CPPUNIT_TEST(testDataSizeSixteenBits);
    CPPUNIT_TEST(testDataSizeGreaterThanThirtyTwo);
    CPPUNIT_TEST(testDataSizeZero);
    CPPUNIT_TEST(testDataSizeGreaterThanRemaining);
    CPPUNIT_TEST(testDataSizeOnEmptyStream);
    CPPUNIT_TEST(testDataSizeAfterShift);
    CPPUNIT_TEST(testDataSizeAcrossByteBoundaries);
    CPPUNIT_TEST(testDataSizeAllBitsSet);
    CPPUNIT_TEST(testDataSizeAllBitsClear);
    CPPUNIT_TEST(testDataSizeAlternatingPattern);
    CPPUNIT_TEST(testDataSizeSequentialCallsWithoutShift);
    CPPUNIT_TEST(testDataSizeReadLastBits);
    CPPUNIT_TEST(testZeroAdjustedDataNonZeroValue);
    CPPUNIT_TEST(testZeroAdjustedDataZeroValue);
    CPPUNIT_TEST(testZeroAdjustedDataOneBitZero);
    CPPUNIT_TEST(testZeroAdjustedDataEightBitsZero);
    CPPUNIT_TEST(testZeroAdjustedDataInvalidSizeThrows);
    CPPUNIT_TEST(testSizeOnEmpty);
    CPPUNIT_TEST(testSizeOnSingleByte);
    CPPUNIT_TEST(testSizeOnMultipleBytes);
    CPPUNIT_TEST(testSizeAfterSetData);
    CPPUNIT_TEST(testLengthOnEmpty);
    CPPUNIT_TEST(testLengthOnSingleByte);
    CPPUNIT_TEST(testLengthOnMultipleBytes);
    CPPUNIT_TEST(testRemainingBitsAtStart);
    CPPUNIT_TEST(testRemainingBitsAfterShift);
    CPPUNIT_TEST(testRemainingBitsAtEnd);
    CPPUNIT_TEST(testRemainingBitsInMiddle);
    CPPUNIT_TEST(testResetAfterShifts);
    CPPUNIT_TEST(testResetWhenAlreadyAtStart);
    CPPUNIT_TEST(testResetThenRead);
    CPPUNIT_TEST(testSetOffsetToValidPosition);
    CPPUNIT_TEST(testSetOffsetToZero);
    CPPUNIT_TEST(testSetOffsetToEnd);
    CPPUNIT_TEST(testSetOffsetBeyondEnd);
    CPPUNIT_TEST(testSetOffsetByteAligned);
    CPPUNIT_TEST(testSetOffsetNonByteAligned);
    CPPUNIT_TEST(testSetDataWithNewVector);
    CPPUNIT_TEST(testSetDataWithEmptyVector);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup code here
    }

    void tearDown()
    {
        // Cleanup code here
    }

protected:
    void testConstructorWithEmptyVector()
    {
        std::vector<uint8_t> empty;
        BitStream bs(empty);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.length());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.remainingBits());
    }

    void testConstructorWithSingleByte()
    {
        std::vector<uint8_t> data = {0xAB};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), bs.length());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), bs.remainingBits());
    }

    void testConstructorWithMultipleBytes()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(32), bs.length());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(32), bs.remainingBits());
    }

    void testConstructorWithLargeVector()
    {
        std::vector<uint8_t> data(10240, 0xFF); // 10KB
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(10240), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(81920), bs.length());
    }

    void testShiftPositiveWithinBounds()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.shift(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testShiftNegativeWithinBounds()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.shift(16);
        bs.shift(-8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testShiftZero()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testShiftBeyondEnd()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(100); // Beyond end
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits()); // Should remain unchanged
    }

    void testShiftBeforeStart()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(-10); // Before start
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits()); // Should remain unchanged
    }

    void testShiftExactlyToEnd()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(16);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.remainingBits());
    }

    void testShiftToZeroFromMiddle()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.shift(12);
        bs.shift(-12);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.remainingBits());
    }

    void testMultipleConsecutiveShifts()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
        BitStream bs(data);
        bs.shift(4);
        bs.shift(4);
        bs.shift(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testDataReferenceReturnsModifiable()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        std::vector<uint8_t>& ref = bs.data();
        ref[0] = 0xFF;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xFF), bs.data(8));
    }

    void testDataReferenceOnEmpty()
    {
        std::vector<uint8_t> empty;
        BitStream bs(empty);
        std::vector<uint8_t>& ref = bs.data();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), ref.size());
    }

    void testDataReferenceExternalModification()
    {
        std::vector<uint8_t> data = {0xAA, 0xBB};
        BitStream bs(data);
        bs.data().push_back(0xCC);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.length());
    }

    void testDataSizeOneBit()
    {
        std::vector<uint8_t> data = {0x80}; // 0b10000000
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(1), bs.data(1));
    }

    void testDataSizeOneByte()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xAB), bs.data(8));
    }

    void testDataSizeThirtyTwoBits()
    {
        std::vector<uint8_t> data = {0x12, 0x34, 0x56, 0x78, 0x9A};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x12345678), bs.data(32));
    }

    void testDataSizeSixteenBits()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD, 0xEF};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xABCD), bs.data(16));
    }

    void testDataSizeGreaterThanThirtyTwo()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BitStream bs(data);
        CPPUNIT_ASSERT_THROW(bs.data(33), InvalidArgument);
    }

    void testDataSizeZero()
    {
        std::vector<uint8_t> data = {0xFF};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), bs.data(0));
    }

    void testDataSizeGreaterThanRemaining()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(10);
        CPPUNIT_ASSERT_THROW(bs.data(10), InvalidArgument);
    }

    void testDataSizeOnEmptyStream()
    {
        std::vector<uint8_t> empty;
        BitStream bs(empty);
        CPPUNIT_ASSERT_THROW(bs.data(1), InvalidArgument);
    }

    void testDataSizeAfterShift()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD, 0xEF};
        BitStream bs(data);
        bs.shift(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xCD), bs.data(8));
    }

    void testDataSizeAcrossByteBoundaries()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD}; // 0b10101011 11001101
        BitStream bs(data);
        bs.shift(4); // Start at bit 4
        // Reading 8 bits from position 4: 0b1011 1100 = 0xBC
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xBC), bs.data(8));
    }

    void testDataSizeAllBitsSet()
    {
        std::vector<uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xFFFFFFFF), bs.data(32));
    }

    void testDataSizeAllBitsClear()
    {
        std::vector<uint8_t> data = {0x00, 0x00, 0x00};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x00), bs.data(16));
    }

    void testDataSizeAlternatingPattern()
    {
        std::vector<uint8_t> data = {0xAA, 0x55}; // 0b10101010 01010101
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xAA55), bs.data(16));
    }

    void testDataSizeSequentialCallsWithoutShift()
    {
        std::vector<uint8_t> data = {0x12, 0x34};
        BitStream bs(data);
        uint32_t first = bs.data(8);
        uint32_t second = bs.data(8);
        CPPUNIT_ASSERT_EQUAL(first, second);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x12), first);
    }

    void testDataSizeReadLastBits()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD, 0xEF};
        BitStream bs(data);
        bs.shift(16); // Move to last byte
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xEF), bs.data(8));
    }

    void testZeroAdjustedDataNonZeroValue()
    {
        std::vector<uint8_t> data = {0x12, 0x34};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0x12), bs.zeroAdjustedData(8));
    }

    void testZeroAdjustedDataZeroValue()
    {
        std::vector<uint8_t> data = {0x00, 0xFF};
        BitStream bs(data);
        uint32_t result = bs.zeroAdjustedData(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(256), result); // 1 << 8
    }

    void testZeroAdjustedDataOneBitZero()
    {
        std::vector<uint8_t> data = {0x00};
        BitStream bs(data);
        uint32_t result = bs.zeroAdjustedData(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(2), result); // 1 << 1
    }

    void testZeroAdjustedDataEightBitsZero()
    {
        std::vector<uint8_t> data = {0x00, 0x00};
        BitStream bs(data);
        uint32_t result = bs.zeroAdjustedData(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(256), result); // 1 << 8
    }

    void testZeroAdjustedDataInvalidSizeThrows()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BitStream bs(data);
        CPPUNIT_ASSERT_THROW(bs.zeroAdjustedData(33), InvalidArgument);
    }

    void testSizeOnEmpty()
    {
        std::vector<uint8_t> empty;
        BitStream bs(empty);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.size());
    }

    void testSizeOnSingleByte()
    {
        std::vector<uint8_t> data = {0xAB};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), bs.size());
    }

    void testSizeOnMultipleBytes()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), bs.size());
    }

    void testSizeAfterSetData()
    {
        std::vector<uint8_t> data1 = {0x01, 0x02};
        std::vector<uint8_t> data2 = {0xAA, 0xBB, 0xCC, 0xDD};
        BitStream bs(data1);
        bs.setData(data2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), bs.size());
    }

    void testLengthOnEmpty()
    {
        std::vector<uint8_t> empty;
        BitStream bs(empty);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.length());
    }

    void testLengthOnSingleByte()
    {
        std::vector<uint8_t> data = {0xFF};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), bs.length());
    }

    void testLengthOnMultipleBytes()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.length());
    }

    void testRemainingBitsAtStart()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.remainingBits());
    }

    void testRemainingBitsAfterShift()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.shift(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testRemainingBitsAtEnd()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(16);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.remainingBits());
    }

    void testRemainingBitsInMiddle()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
        BitStream bs(data);
        bs.shift(12);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(20), bs.remainingBits());
    }

    void testResetAfterShifts()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.shift(16);
        bs.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.remainingBits());
    }

    void testResetWhenAlreadyAtStart()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testResetThenRead()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD};
        BitStream bs(data);
        bs.shift(8);
        bs.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xAB), bs.data(8));
    }

    void testSetOffsetToValidPosition()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.setOffset(12);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(12), bs.remainingBits());
    }

    void testSetOffsetToZero()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.shift(8);
        bs.setOffset(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits());
    }

    void testSetOffsetToEnd()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        BitStream bs(data);
        bs.setOffset(24);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.remainingBits());
    }

    void testSetOffsetBeyondEnd()
    {
        std::vector<uint8_t> data = {0x01, 0x02};
        BitStream bs(data);
        bs.setOffset(100);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), bs.remainingBits()); // Should remain unchanged
    }

    void testSetOffsetByteAligned()
    {
        std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC};
        BitStream bs(data);
        bs.setOffset(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xBB), bs.data(8));
    }

    void testSetOffsetNonByteAligned()
    {
        std::vector<uint8_t> data = {0xAB, 0xCD}; // 0b10101011 11001101
        BitStream bs(data);
        bs.setOffset(4);
        // Reading 8 bits from position 4: 0b1011 1100 = 0xBC
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xBC), bs.data(8));
    }

    void testSetDataWithNewVector()
    {
        std::vector<uint8_t> data1 = {0x01, 0x02};
        std::vector<uint8_t> data2 = {0xAA, 0xBB, 0xCC};
        BitStream bs(data1);
        bs.shift(8);
        bs.setData(data2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), bs.remainingBits());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0xAA), bs.data(8));
    }

    void testSetDataWithEmptyVector()
    {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        std::vector<uint8_t> empty;
        BitStream bs(data);
        bs.setData(empty);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bs.length());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteBitStreamTest );
