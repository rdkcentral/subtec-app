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
#include "PesFinder.hpp"
#include <vector>
#include <cstring>

using namespace subttxrend::ctrl;

class PesFinderTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(PesFinderTest);
    CPPUNIT_TEST(testConstructorWithValidBuffer);
    CPPUNIT_TEST(testConstructorWithZeroSizeBuffer);
    CPPUNIT_TEST(testFindSingleValidPesAtStart);
    CPPUNIT_TEST(testFindSingleValidPesNotAtStart);
    CPPUNIT_TEST(testFindMultipleConsecutivePesPackets);
    CPPUNIT_TEST(testFindPesWithMinimumLength);
    CPPUNIT_TEST(testFindPesWithMaximumLength);
    CPPUNIT_TEST(testFindPesExactlyFittingBuffer);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testBufferWithLessThan6Bytes);
    CPPUNIT_TEST(testBufferWith5BytesStartingWithValidStartCode);
    CPPUNIT_TEST(testNoPesInBuffer);
    CPPUNIT_TEST(testMultipleCallsAfterBufferExhaustion);
    CPPUNIT_TEST(testPesWithZeroLength);
    CPPUNIT_TEST(testPesHeaderPresentButInsufficientDataBytes);
    CPPUNIT_TEST(testInvalidStartCodeFirstByte);
    CPPUNIT_TEST(testInvalidStartCodeSecondByte);
    CPPUNIT_TEST(testInvalidStartCodeThirdByte);
    CPPUNIT_TEST(testInvalidStreamId);
    CPPUNIT_TEST(testPartialStartCodeAtBufferEnd);
    CPPUNIT_TEST(testSkipOneByteWhenFirstByteWrong);
    CPPUNIT_TEST(testSkipTwoBytesWhenSecondByteWrong);
    CPPUNIT_TEST(testSkipThreeBytesWhenThirdByteWrong);
    CPPUNIT_TEST(testSkipThreeBytesWhenStreamIdWrong);
    CPPUNIT_TEST(testAdjacentPesPacketsWithNoGap);
    CPPUNIT_TEST(testPesPacketAfterGarbageData);
    CPPUNIT_TEST(testMultiplePesWithDifferentLengths);
    CPPUNIT_TEST(testBufferPositionAfterFindingPes);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Setup code here
    }

    void tearDown() override
    {
        // Cleanup code here
    }

protected:
    // Helper function to create a valid PES packet
    std::vector<std::uint8_t> createValidPes(std::uint16_t payloadLength)
    {
        std::vector<std::uint8_t> pes;
        pes.push_back(0x00); // Start code byte 1
        pes.push_back(0x00); // Start code byte 2
        pes.push_back(0x01); // Start code byte 3
        pes.push_back(0xBD); // Stream ID (private stream 1)
        pes.push_back((payloadLength >> 8) & 0xFF); // Length high byte
        pes.push_back(payloadLength & 0xFF); // Length low byte

        // Add payload bytes
        for (std::uint16_t i = 0; i < payloadLength; ++i)
        {
            pes.push_back(0xAA); // Dummy payload data
        }

        return pes;
    }

    void testConstructorWithValidBuffer()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0x01, 0x02, 0x03};
        PesFinder finder(buffer.data(), buffer.size());
        // Constructor should not throw, object is created successfully
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithZeroSizeBuffer()
    {
        std::vector<std::uint8_t> buffer = {0x00};
        PesFinder finder(buffer.data(), 0);

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        // Should return false immediately with zero size
        CPPUNIT_ASSERT_EQUAL(false, finder.findNextPes(pesStart, pesSize));
    }

    void testFindSingleValidPesAtStart()
    {
        std::vector<std::uint8_t> pes = createValidPes(10);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(pes.data()), pesStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(16), pesSize); // 6 header + 10 payload
    }

    void testFindSingleValidPesNotAtStart()
    {
        std::vector<std::uint8_t> buffer = {0xFF, 0xFE, 0xFD}; // Garbage data
        std::vector<std::uint8_t> pes = createValidPes(5);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 3), pesStart); // After 3 garbage bytes
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(11), pesSize); // 6 header + 5 payload
    }

    void testFindMultipleConsecutivePesPackets()
    {
        std::vector<std::uint8_t> pes1 = createValidPes(8);
        std::vector<std::uint8_t> pes2 = createValidPes(12);

        std::vector<std::uint8_t> buffer;
        buffer.insert(buffer.end(), pes1.begin(), pes1.end());
        buffer.insert(buffer.end(), pes2.begin(), pes2.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart1 = nullptr;
        std::uint16_t pesSize1 = 0;
        bool result1 = finder.findNextPes(pesStart1, pesSize1);

        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data()), pesStart1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(14), pesSize1); // 6 + 8

        const std::uint8_t* pesStart2 = nullptr;
        std::uint16_t pesSize2 = 0;
        bool result2 = finder.findNextPes(pesStart2, pesSize2);

        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 14), pesStart2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(18), pesSize2); // 6 + 12

        // Third call should return false
        const std::uint8_t* pesStart3 = nullptr;
        std::uint16_t pesSize3 = 0;
        bool result3 = finder.findNextPes(pesStart3, pesSize3);

        CPPUNIT_ASSERT_EQUAL(false, result3);
    }

    void testFindPesWithMinimumLength()
    {
        std::vector<std::uint8_t> pes = createValidPes(1);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(7), pesSize); // 6 header + 1 payload
    }

    void testFindPesWithMaximumLength()
    {
        std::vector<std::uint8_t> pes = createValidPes(0xFFFF);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(6 + 0xFFFF), pesSize);
    }

    void testFindPesExactlyFittingBuffer()
    {
        std::vector<std::uint8_t> pes = createValidPes(20);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(26), pesSize);

        // Next call should return false
        bool result2 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(false, result2);
    }

    void testEmptyBuffer()
    {
        std::vector<std::uint8_t> buffer;
        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testBufferWithLessThan6Bytes()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0x00, 0x01, 0xBD};
        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testBufferWith5BytesStartingWithValidStartCode()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0x00, 0x01, 0xBD, 0x00};
        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testNoPesInBuffer()
    {
        std::vector<std::uint8_t> buffer = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};
        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testMultipleCallsAfterBufferExhaustion()
    {
        std::vector<std::uint8_t> pes = createValidPes(5);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        // First call should find the PES
        bool result1 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result1);

        // Subsequent calls should consistently return false
        bool result2 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(false, result2);

        bool result3 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(false, result3);
    }

    void testPesWithZeroLength()
    {
        std::vector<std::uint8_t> buffer = {
            0x00, 0x00, 0x01, 0xBD,
            0x00, 0x00 // Length = 0
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        // Zero length PES not supported
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testPesHeaderPresentButInsufficientDataBytes()
    {
        std::vector<std::uint8_t> buffer = {
            0x00, 0x00, 0x01, 0xBD,
            0x00, 0x0A  // Length = 10, but only header present
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testInvalidStartCodeFirstByte()
    {
        std::vector<std::uint8_t> buffer = {
            0x01, 0x00, 0x01, 0xBD, 0x00, 0x05
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testInvalidStartCodeSecondByte()
    {
        std::vector<std::uint8_t> buffer = {
            0x00, 0x01, 0x01, 0xBD, 0x00, 0x05
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testInvalidStartCodeThirdByte()
    {
        std::vector<std::uint8_t> buffer = {
            0x00, 0x00, 0x00, 0xBD, 0x00, 0x05
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testInvalidStreamId()
    {
        std::vector<std::uint8_t> buffer = {
            0x00, 0x00, 0x01, 0xE0, // Stream ID is 0xE0 instead of 0xBD
            0x00, 0x05, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testPartialStartCodeAtBufferEnd()
    {
        std::vector<std::uint8_t> buffer = {
            0xFF, 0xFE, 0xFD, 0xFC, 0x00, 0x00
        };

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        // Should not read beyond buffer
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testSkipOneByteWhenFirstByteWrong()
    {
        std::vector<std::uint8_t> buffer = {0xFF}; // Wrong first byte
        std::vector<std::uint8_t> pes = createValidPes(3);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 1), pesStart); // Skipped 1 byte
    }

    void testSkipTwoBytesWhenSecondByteWrong()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0xFF}; // Second byte wrong
        std::vector<std::uint8_t> pes = createValidPes(3);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 2), pesStart); // Skipped 2 bytes
    }

    void testSkipThreeBytesWhenThirdByteWrong()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0x00, 0xFF}; // Third byte wrong
        std::vector<std::uint8_t> pes = createValidPes(3);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 3), pesStart); // Skipped 3 bytes
    }

    void testSkipThreeBytesWhenStreamIdWrong()
    {
        std::vector<std::uint8_t> buffer = {0x00, 0x00, 0x01, 0xE0}; // Stream ID wrong
        std::vector<std::uint8_t> pes = createValidPes(3);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        // Should skip 3 bytes from the wrong start code and find the valid PES
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 4), pesStart);
    }

    void testAdjacentPesPacketsWithNoGap()
    {
        std::vector<std::uint8_t> pes1 = createValidPes(4);
        std::vector<std::uint8_t> pes2 = createValidPes(6);
        std::vector<std::uint8_t> pes3 = createValidPes(8);

        std::vector<std::uint8_t> buffer;
        buffer.insert(buffer.end(), pes1.begin(), pes1.end());
        buffer.insert(buffer.end(), pes2.begin(), pes2.end());
        buffer.insert(buffer.end(), pes3.begin(), pes3.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        // Find all three packets
        bool result1 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(10), pesSize);

        bool result2 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(12), pesSize);

        bool result3 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(14), pesSize);

        bool result4 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(false, result4);
    }

    void testPesPacketAfterGarbageData()
    {
        std::vector<std::uint8_t> buffer = {
            0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8
        };
        std::vector<std::uint8_t> pes = createValidPes(7);
        buffer.insert(buffer.end(), pes.begin(), pes.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(static_cast<const std::uint8_t*>(buffer.data() + 8), pesStart);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(13), pesSize);
    }

    void testMultiplePesWithDifferentLengths()
    {
        std::vector<std::uint8_t> pes1 = createValidPes(1);   // Min length
        std::vector<std::uint8_t> pes2 = createValidPes(255); // Medium length
        std::vector<std::uint8_t> pes3 = createValidPes(10);  // Small length

        std::vector<std::uint8_t> buffer;
        buffer.insert(buffer.end(), pes1.begin(), pes1.end());
        buffer.insert(buffer.end(), pes2.begin(), pes2.end());
        buffer.insert(buffer.end(), pes3.begin(), pes3.end());

        PesFinder finder(buffer.data(), buffer.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result1 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(7), pesSize);

        bool result2 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(261), pesSize);

        bool result3 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(true, result3);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(16), pesSize);
    }

    void testBufferPositionAfterFindingPes()
    {
        std::vector<std::uint8_t> pes = createValidPes(10);
        PesFinder finder(pes.data(), pes.size());

        const std::uint8_t* pesStart = nullptr;
        std::uint16_t pesSize = 0;

        bool result = finder.findNextPes(pesStart, pesSize);

        CPPUNIT_ASSERT_EQUAL(true, result);

        // After finding PES, buffer position should be at the end
        // Next call should return false
        bool result2 = finder.findNextPes(pesStart, pesSize);
        CPPUNIT_ASSERT_EQUAL(false, result2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PesFinderTest);
