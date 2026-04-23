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
#include <stdexcept>
#include <cstring>
#include <limits>

#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class DataPacketTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DataPacketTest );
    CPPUNIT_TEST(testConstructorWithZeroCapacity);
    CPPUNIT_TEST(testConstructorWithCapacityOne);
    CPPUNIT_TEST(testConstructorWithSmallCapacity);
    CPPUNIT_TEST(testConstructorWithLargeCapacity);
    CPPUNIT_TEST(testConstructorInitializesSizeToZero);
    CPPUNIT_TEST(testGetBufferReturnsNonNull);
    CPPUNIT_TEST(testGetBufferConstVariantReturnsSamePointer);
    CPPUNIT_TEST(testGetBufferIsWritable);
    CPPUNIT_TEST(testGetBufferAfterResetReturnsSamePointer);
    CPPUNIT_TEST(testGetCapacityReturnsZeroForZeroCapacity);
    CPPUNIT_TEST(testGetCapacityReturnsExactConstructorValue);
    CPPUNIT_TEST(testGetCapacityIsImmutable);
    CPPUNIT_TEST(testGetSizeInitiallyZero);
    CPPUNIT_TEST(testGetSizeAfterSetSize);
    CPPUNIT_TEST(testGetSizeAfterAppendLeUint32);
    CPPUNIT_TEST(testGetSizeAfterAppendLeUint64);
    CPPUNIT_TEST(testGetSizeAfterAppendZeroes);
    CPPUNIT_TEST(testGetSizeAfterReset);
    CPPUNIT_TEST(testSetSizeToZeroOnEmptyPacket);
    CPPUNIT_TEST(testSetSizeToZeroOnPacketWithData);
    CPPUNIT_TEST(testSetSizeLessThanCapacity);
    CPPUNIT_TEST(testSetSizeExactlyToCapacity);
    CPPUNIT_TEST(testSetSizeMultipleTimes);
    CPPUNIT_TEST(testSetSizeGreaterThanCapacityThrows);
    CPPUNIT_TEST(testSetSizeToMaxSizeTThrows);
    CPPUNIT_TEST(testSetSizeToCapacityTimesTwoThrows);
    CPPUNIT_TEST(testResetOnEmptyPacket);
    CPPUNIT_TEST(testResetOnPacketWithData);
    CPPUNIT_TEST(testResetAfterPartialAppend);
    CPPUNIT_TEST(testMultipleConsecutiveResets);
    CPPUNIT_TEST(testResetDoesNotChangeCapacity);
    CPPUNIT_TEST(testAppendLeUint32Zero);
    CPPUNIT_TEST(testAppendLeUint32LittleEndianByteOrder);
    CPPUNIT_TEST(testAppendLeUint32MaxValue);
    CPPUNIT_TEST(testAppendLeUint32One);
    CPPUNIT_TEST(testMultipleAppendLeUint32);
    CPPUNIT_TEST(testAppendLeUint32ExactFit);
    CPPUNIT_TEST(testAppendLeUint32WithThreeBytesRemainingThrows);
    CPPUNIT_TEST(testAppendLeUint32OnFullBufferThrows);
    CPPUNIT_TEST(testAppendLeUint32OnZeroCapacityThrows);
    CPPUNIT_TEST(testAppendLeUint64Zero);
    CPPUNIT_TEST(testAppendLeUint64LittleEndianByteOrder);
    CPPUNIT_TEST(testAppendLeUint64MaxValue);
    CPPUNIT_TEST(testAppendLeUint64ExactFit);
    CPPUNIT_TEST(testAppendLeUint64MultipleTimes);
    CPPUNIT_TEST(testAppendLeUint64WithSevenBytesRemainingThrows);
    CPPUNIT_TEST(testAppendLeUint64OnFullBufferThrows);
    CPPUNIT_TEST(testAppendZeroesWithCountZero);
    CPPUNIT_TEST(testAppendZeroesWithCountOne);
    CPPUNIT_TEST(testAppendZeroesMultipleBytes);
    CPPUNIT_TEST(testAppendZeroesExactFit);
    CPPUNIT_TEST(testAppendZeroesAfterOtherAppends);
    CPPUNIT_TEST(testAppendZeroesVerifyBytesAreZero);
    CPPUNIT_TEST(testAppendZeroesExceedingCapacityThrows);
    CPPUNIT_TEST(testAppendZeroesOnFullBufferThrows);
    CPPUNIT_TEST(testMixedAppendsUint32Uint64Zeros);
    CPPUNIT_TEST(testFillBufferCompletelyWithMixedOperations);
    CPPUNIT_TEST(testAppendResetReAppendCycle);
    CPPUNIT_TEST(testSetSizeBetweenAppends);
    CPPUNIT_TEST(testComplexLifecycle);
    CPPUNIT_TEST(testSequentialAppendsVerifyCorrectPositioning);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup code here if needed
    }

    void tearDown()
    {
        // Cleanup code here if needed
    }

protected:
    void testConstructorWithZeroCapacity()
    {
        DataPacket packet(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCapacity());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
        CPPUNIT_ASSERT(packet.getBuffer() != nullptr);
    }

    void testConstructorWithCapacityOne()
    {
        DataPacket packet(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet.getCapacity());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testConstructorWithSmallCapacity()
    {
        DataPacket packet(10);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getCapacity());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testConstructorWithLargeCapacity()
    {
        const size_t largeCapacity = 1024 * 1024; // 1MB
        DataPacket packet(largeCapacity);
        CPPUNIT_ASSERT_EQUAL(largeCapacity, packet.getCapacity());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testConstructorInitializesSizeToZero()
    {
        DataPacket packet(100);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testGetBufferReturnsNonNull()
    {
        DataPacket packet(10);
        CPPUNIT_ASSERT(packet.getBuffer() != nullptr);
    }

    void testGetBufferConstVariantReturnsSamePointer()
    {
        DataPacket packet(10);
        char* nonConstPtr = packet.getBuffer();
        const DataPacket& constPacket = packet;
        const char* constPtr = constPacket.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<const char*>(nonConstPtr), constPtr);
    }

    void testGetBufferIsWritable()
    {
        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        buffer[0] = 0x42;
        buffer[1] = 0x43;
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x43), buffer[1]);
    }

    void testGetBufferAfterResetReturnsSamePointer()
    {
        DataPacket packet(10);
        char* ptr1 = packet.getBuffer();
        packet.setSize(5);
        packet.reset();
        char* ptr2 = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(ptr1, ptr2);
    }

    void testGetCapacityReturnsZeroForZeroCapacity()
    {
        DataPacket packet(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCapacity());
    }

    void testGetCapacityReturnsExactConstructorValue()
    {
        const size_t capacity = 42;
        DataPacket packet(capacity);
        CPPUNIT_ASSERT_EQUAL(capacity, packet.getCapacity());
    }

    void testGetCapacityIsImmutable()
    {
        DataPacket packet(100);
        size_t cap1 = packet.getCapacity();
        packet.setSize(50);
        size_t cap2 = packet.getCapacity();
        packet.reset();
        size_t cap3 = packet.getCapacity();
        CPPUNIT_ASSERT_EQUAL(cap1, cap2);
        CPPUNIT_ASSERT_EQUAL(cap2, cap3);
    }

    void testGetSizeInitiallyZero()
    {
        DataPacket packet(100);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testGetSizeAfterSetSize()
    {
        DataPacket packet(100);
        packet.setSize(25);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), packet.getSize());
    }

    void testGetSizeAfterAppendLeUint32()
    {
        DataPacket packet(100);
        packet.appendLeUint32(0x12345678);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
    }

    void testGetSizeAfterAppendLeUint64()
    {
        DataPacket packet(100);
        packet.appendLeUint64(0x123456789ABCDEF0ULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
    }

    void testGetSizeAfterAppendZeroes()
    {
        DataPacket packet(100);
        packet.appendZeroes(10);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());
    }

    void testGetSizeAfterReset()
    {
        DataPacket packet(100);
        packet.setSize(50);
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testSetSizeToZeroOnEmptyPacket()
    {
        DataPacket packet(10);
        packet.setSize(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testSetSizeToZeroOnPacketWithData()
    {
        DataPacket packet(10);
        packet.setSize(5);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), packet.getSize());
        packet.setSize(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testSetSizeLessThanCapacity()
    {
        DataPacket packet(100);
        packet.setSize(50);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(50), packet.getSize());
    }

    void testSetSizeExactlyToCapacity()
    {
        DataPacket packet(100);
        packet.setSize(100);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(100), packet.getSize());
    }

    void testSetSizeMultipleTimes()
    {
        DataPacket packet(100);
        packet.setSize(10);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());
        packet.setSize(20);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(20), packet.getSize());
        packet.setSize(5);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), packet.getSize());
    }

    void testSetSizeGreaterThanCapacityThrows()
    {
        DataPacket packet(10);
        CPPUNIT_ASSERT_THROW(packet.setSize(11), std::length_error);
    }

    void testSetSizeToMaxSizeTThrows()
    {
        DataPacket packet(10);
        CPPUNIT_ASSERT_THROW(packet.setSize(std::numeric_limits<size_t>::max()), std::length_error);
    }

    void testSetSizeToCapacityTimesTwoThrows()
    {
        DataPacket packet(50);
        CPPUNIT_ASSERT_THROW(packet.setSize(100), std::length_error);
    }

    void testResetOnEmptyPacket()
    {
        DataPacket packet(10);
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testResetOnPacketWithData()
    {
        DataPacket packet(10);
        packet.setSize(7);
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testResetAfterPartialAppend()
    {
        DataPacket packet(20);
        packet.appendLeUint32(0x12345678);
        packet.appendZeroes(3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), packet.getSize());
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testMultipleConsecutiveResets()
    {
        DataPacket packet(10);
        packet.setSize(5);
        packet.reset();
        packet.reset();
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testResetDoesNotChangeCapacity()
    {
        DataPacket packet(100);
        size_t capacityBefore = packet.getCapacity();
        packet.setSize(50);
        packet.reset();
        size_t capacityAfter = packet.getCapacity();
        CPPUNIT_ASSERT_EQUAL(capacityBefore, capacityAfter);
    }

    void testAppendLeUint32Zero()
    {
        DataPacket packet(10);
        packet.appendLeUint32(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        const char* buffer = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[3]);
    }

    void testAppendLeUint32LittleEndianByteOrder()
    {
        DataPacket packet(10);
        packet.appendLeUint32(0x12345678);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        const char* buffer = packet.getBuffer();
        // Little endian: LSB first
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x78), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x56), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x34), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x12), buffer[3]);
    }

    void testAppendLeUint32MaxValue()
    {
        DataPacket packet(10);
        packet.appendLeUint32(0xFFFFFFFF);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        const char* buffer = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), buffer[3]);
    }

    void testAppendLeUint32One()
    {
        DataPacket packet(10);
        packet.appendLeUint32(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        const char* buffer = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[3]);
    }

    void testMultipleAppendLeUint32()
    {
        DataPacket packet(20);
        packet.appendLeUint32(0x11111111);
        packet.appendLeUint32(0x22222222);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
        const char* buffer = packet.getBuffer();
        // First uint32
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[3]);
        // Second uint32
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[6]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[7]);
    }

    void testAppendLeUint32ExactFit()
    {
        DataPacket packet(4);
        packet.appendLeUint32(0xABCDEF01);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(packet.getCapacity(), packet.getSize());
    }

    void testAppendLeUint32WithThreeBytesRemainingThrows()
    {
        DataPacket packet(7);
        packet.appendZeroes(4); // 3 bytes remaining
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        CPPUNIT_ASSERT_THROW(packet.appendLeUint32(0x12345678), std::length_error);
    }

    void testAppendLeUint32OnFullBufferThrows()
    {
        DataPacket packet(10);
        packet.setSize(10);
        CPPUNIT_ASSERT_THROW(packet.appendLeUint32(0x12345678), std::length_error);
    }

    void testAppendLeUint32OnZeroCapacityThrows()
    {
        DataPacket packet(0);
        CPPUNIT_ASSERT_THROW(packet.appendLeUint32(0x12345678), std::length_error);
    }

    void testAppendLeUint64Zero()
    {
        DataPacket packet(20);
        packet.appendLeUint64(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
        const char* buffer = packet.getBuffer();
        for (int i = 0; i < 8; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[i]);
        }
    }

    void testAppendLeUint64LittleEndianByteOrder()
    {
        DataPacket packet(20);
        packet.appendLeUint64(0x123456789ABCDEF0ULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
        const char* buffer = packet.getBuffer();
        // Little endian: LSB first
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xF0), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xDE), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xBC), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x9A), buffer[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x78), buffer[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x56), buffer[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x34), buffer[6]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x12), buffer[7]);
    }

    void testAppendLeUint64MaxValue()
    {
        DataPacket packet(20);
        packet.appendLeUint64(0xFFFFFFFFFFFFFFFFULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
        const char* buffer = packet.getBuffer();
        for (int i = 0; i < 8; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0xFF), buffer[i]);
        }
    }

    void testAppendLeUint64ExactFit()
    {
        DataPacket packet(8);
        packet.appendLeUint64(0x0102030405060708ULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(packet.getCapacity(), packet.getSize());
    }

    void testAppendLeUint64MultipleTimes()
    {
        DataPacket packet(24);
        packet.appendLeUint64(0x1111111111111111ULL);
        packet.appendLeUint64(0x2222222222222222ULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), packet.getSize());
        const char* buffer = packet.getBuffer();
        // Verify first uint64
        for (int i = 0; i < 8; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[i]);
        }
        // Verify second uint64
        for (int i = 8; i < 16; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[i]);
        }
    }

    void testAppendLeUint64WithSevenBytesRemainingThrows()
    {
        DataPacket packet(10);
        packet.appendZeroes(3); // 7 bytes remaining
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet.getSize());
        CPPUNIT_ASSERT_THROW(packet.appendLeUint64(0x123456789ABCDEF0ULL), std::length_error);
    }

    void testAppendLeUint64OnFullBufferThrows()
    {
        DataPacket packet(10);
        packet.setSize(10);
        CPPUNIT_ASSERT_THROW(packet.appendLeUint64(0x123456789ABCDEF0ULL), std::length_error);
    }

    void testAppendZeroesWithCountZero()
    {
        DataPacket packet(10);
        packet.appendZeroes(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
    }

    void testAppendZeroesWithCountOne()
    {
        DataPacket packet(10);
        packet.appendZeroes(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet.getSize());
        const char* buffer = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[0]);
    }

    void testAppendZeroesMultipleBytes()
    {
        DataPacket packet(20);
        packet.appendZeroes(5);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), packet.getSize());
        const char* buffer = packet.getBuffer();
        for (int i = 0; i < 5; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[i]);
        }
    }

    void testAppendZeroesExactFit()
    {
        DataPacket packet(10);
        packet.appendZeroes(10);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(packet.getCapacity(), packet.getSize());
    }

    void testAppendZeroesAfterOtherAppends()
    {
        DataPacket packet(20);
        packet.appendLeUint32(0xFFFFFFFF);
        size_t sizeAfterUint32 = packet.getSize();
        packet.appendZeroes(3);
        CPPUNIT_ASSERT_EQUAL(sizeAfterUint32 + 3, packet.getSize());
        const char* buffer = packet.getBuffer();
        // Verify zeros are after the uint32
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[6]);
    }

    void testAppendZeroesVerifyBytesAreZero()
    {
        DataPacket packet(20);
        // Write some non-zero data first
        char* buffer = packet.getBuffer();
        buffer[10] = 0xFF;
        buffer[11] = 0xFF;
        buffer[12] = 0xFF;

        // Now append zeros
        packet.appendZeroes(3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet.getSize());

        // Verify the appended bytes are zero
        const char* constBuffer = packet.getBuffer();
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), constBuffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), constBuffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), constBuffer[2]);
    }

    void testAppendZeroesExceedingCapacityThrows()
    {
        DataPacket packet(10);
        CPPUNIT_ASSERT_THROW(packet.appendZeroes(11), std::length_error);
    }

    void testAppendZeroesOnFullBufferThrows()
    {
        DataPacket packet(10);
        packet.setSize(10);
        CPPUNIT_ASSERT_THROW(packet.appendZeroes(1), std::length_error);
    }

    void testMixedAppendsUint32Uint64Zeros()
    {
        DataPacket packet(30);
        packet.appendLeUint32(0x11111111);
        packet.appendLeUint64(0x2222222222222222ULL);
        packet.appendZeroes(3);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(15), packet.getSize());

        const char* buffer = packet.getBuffer();
        // Verify uint32 (bytes 0-3)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x11), buffer[3]);
        // Verify uint64 (bytes 4-11)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[11]);
        // Verify zeros (bytes 12-14)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[13]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[14]);
    }

    void testFillBufferCompletelyWithMixedOperations()
    {
        DataPacket packet(17); // 4 + 8 + 5 = 17
        packet.appendLeUint32(0xAAAAAAAA);
        packet.appendLeUint64(0xBBBBBBBBBBBBBBBBULL);
        packet.appendZeroes(5);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(17), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(packet.getCapacity(), packet.getSize());
    }

    void testAppendResetReAppendCycle()
    {
        DataPacket packet(20);

        // First cycle
        packet.appendLeUint32(0x11111111);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());

        // Reset
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

        // Re-append
        packet.appendLeUint64(0x2222222222222222ULL);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());

        const char* buffer = packet.getBuffer();
        // Verify the new data (uint64)
        for (int i = 0; i < 8; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x22), buffer[i]);
        }
    }

    void testSetSizeBetweenAppends()
    {
        DataPacket packet(30);

        packet.appendLeUint32(0x11111111);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());

        packet.setSize(10); // Manually increase size
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());

        packet.appendLeUint32(0x22222222);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getSize());
    }

    void testComplexLifecycle()
    {
        DataPacket packet(50);

        // Phase 1: Initial appends
        packet.appendLeUint32(0xAAAAAAAA);
        packet.appendZeroes(2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), packet.getSize());

        // Phase 2: Manual size adjustment
        packet.setSize(10);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());

        // Phase 3: Reset
        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

        // Phase 4: Re-use buffer
        packet.appendLeUint64(0xBBBBBBBBBBBBBBBBULL);
        packet.appendLeUint32(0xCCCCCCCC);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());

        // Verify capacity unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(50), packet.getCapacity());
    }

    void testSequentialAppendsVerifyCorrectPositioning()
    {
        DataPacket packet(50);

        packet.appendLeUint32(0x01020304);
        packet.appendLeUint64(0x0102030405060708ULL);
        packet.appendZeroes(2);
        packet.appendLeUint32(0x0A0B0C0D);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(18), packet.getSize());

        const char* buffer = packet.getBuffer();

        // Verify first uint32 at position 0-3 (little endian)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x04), buffer[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), buffer[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), buffer[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), buffer[3]);

        // Verify uint64 at position 4-11 (little endian)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x08), buffer[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x07), buffer[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), buffer[11]);

        // Verify zeros at position 12-13
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x00), buffer[13]);

        // Verify second uint32 at position 14-17 (little endian)
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0D), buffer[14]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0C), buffer[15]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0B), buffer[16]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x0A), buffer[17]);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( DataPacketTest );
