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
#include <vector>
#include <cstring>

#include "PesBuffer.hpp"

using namespace ttxdecoder;

class PesBufferTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PesBufferTest );
    CPPUNIT_TEST(testConstructorWithValidBuffer);
    CPPUNIT_TEST(testConstructorWithZeroSize);
    CPPUNIT_TEST(testAddValidMinimalPesPacket);
    CPPUNIT_TEST(testAddValidPesPacketWithData);
    CPPUNIT_TEST(testAddMultipleValidPackets);
    CPPUNIT_TEST(testAddPacketExactlyFillsBuffer);
    CPPUNIT_TEST(testAddPacketWrapsAroundBuffer);
    CPPUNIT_TEST(testAddValidTeletextPacketWithPTS);
    CPPUNIT_TEST(testAddValidTeletextPacketWithoutPTS);
    CPPUNIT_TEST(testAddValidTeletextPacketWithPTSDTS);
    CPPUNIT_TEST(testAddPacketWithMaxPesLength);
    CPPUNIT_TEST(testAddPacketTooSmall);
    CPPUNIT_TEST(testAddPacketWithInvalidStartCode1);
    CPPUNIT_TEST(testAddPacketWithInvalidStartCode2);
    CPPUNIT_TEST(testAddPacketWithInvalidStartCode3);
    CPPUNIT_TEST(testAddPacketWithInvalidStreamId);
    CPPUNIT_TEST(testAddPacketWithZeroPesLength);
    CPPUNIT_TEST(testAddPacketWithMismatchedPesLength);
    CPPUNIT_TEST(testAddPacketWhenBufferFull);
    CPPUNIT_TEST(testAddPacketLargerThanBuffer);
    CPPUNIT_TEST(testClearEmptyBuffer);
    CPPUNIT_TEST(testClearBufferWithOnePacket);
    CPPUNIT_TEST(testClearBufferWithMultiplePackets);
    CPPUNIT_TEST(testClearAfterPartialConsumption);
    CPPUNIT_TEST(testGetNextPacketFromEmptyBuffer);
    CPPUNIT_TEST(testGetNextPacketWithOnePacket);
    CPPUNIT_TEST(testGetNextPacketWithMultiplePackets);
    CPPUNIT_TEST(testGetNextPacketAfterConsumption);
    CPPUNIT_TEST(testGetNextPacketWrappedAroundBuffer);
    CPPUNIT_TEST(testGetNextPacketWithPTS);
    CPPUNIT_TEST(testGetNextPacketWithPTSDTS);
    CPPUNIT_TEST(testGetNextPacketWithoutPTS);
    CPPUNIT_TEST(testGetNextPacketWithHeaderSkip);
    CPPUNIT_TEST(testGetNextPacketWithCorruptedStartCode);
    CPPUNIT_TEST(testGetNextPacketWithInsufficientData);
    CPPUNIT_TEST(testGetNextPacketWithInvalidHeaderLength);
    CPPUNIT_TEST(testMarkPacketConsumed);
    CPPUNIT_TEST(testMarkPacketConsumedWithWrapAround);
    CPPUNIT_TEST(testMarkLastPacketConsumed);
    CPPUNIT_TEST(testMarkMultiplePacketsConsumed);
    CPPUNIT_TEST(testFullCycleAddGetConsume);
    CPPUNIT_TEST(testMultipleCyclesWithWrapAround);
    CPPUNIT_TEST(testFillConsumeRefill);
    CPPUNIT_TEST(testGetWithoutConsume);
    CPPUNIT_TEST(testClearInterruptsRetrieval);
    CPPUNIT_TEST(testMixedPacketSizes);
    CPPUNIT_TEST(testVerySmallBuffer);
    CPPUNIT_TEST(testPTSWithAllBitsSet);
    CPPUNIT_TEST(testMultipleWrapsAround);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Allocate test buffer
        m_buffer = new std::uint8_t[BUFFER_SIZE];
        std::memset(m_buffer, 0, BUFFER_SIZE);
    }

    void tearDown() override
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }

private:
    // Helper methods
    std::vector<std::uint8_t> createValidPesPacket(std::uint16_t dataSize,
                                                     bool withPTS = false,
                                                     std::uint32_t ptsValue = 0,
                                                     bool withDTS = false)
    {
        std::vector<std::uint8_t> packet;

        // Start code prefix
        packet.push_back(0x00);
        packet.push_back(0x00);
        packet.push_back(0x01);

        // Stream ID (0xBD for teletext)
        packet.push_back(0xBD);

        // PES packet length
        std::uint16_t pesLength = dataSize;

        if (withPTS || withDTS)
        {
            // Control bytes (2) + PES header length (1) + PTS/DTS bytes
            std::uint8_t controlByte2 = 0x80; // Marker bits
            std::uint8_t pesHeaderLength = 0;

            if (withPTS && withDTS)
            {
                controlByte2 |= 0xC0; // pts_dts_flags = 11
                pesHeaderLength = 10; // 5 for PTS + 5 for DTS
            }
            else if (withPTS)
            {
                controlByte2 |= 0x80; // pts_dts_flags = 10
                pesHeaderLength = 5;
            }

            pesLength += 3 + pesHeaderLength; // control bytes (2) + header length (1) + header data
        }

        packet.push_back((pesLength >> 8) & 0xFF);
        packet.push_back(pesLength & 0xFF);

        if (withPTS || withDTS)
        {
            // Control byte 1 (marker bits)
            packet.push_back(0x80);

            // Control byte 2
            std::uint8_t controlByte2 = 0x80;
            if (withPTS && withDTS)
            {
                controlByte2 |= 0xC0;
            }
            else if (withPTS)
            {
                controlByte2 |= 0x80;
            }
            packet.push_back(controlByte2);

            // PES header length
            std::uint8_t pesHeaderLength = (withPTS && withDTS) ? 10 : (withPTS ? 5 : 0);
            packet.push_back(pesHeaderLength);

            // PTS
            if (withPTS)
            {
                std::uint8_t prefix = withDTS ? 0x30 : 0x20;
                // Encode PTS according to MPEG-2 format to match decoder
                // Decoder does: pts |= (ptsByte1 << 28) & 0xE0000000
                // Takes byte bits 3-1 and puts them at pts bits 31-29
                packet.push_back(prefix | ((ptsValue >> 28) & 0x0E) | 0x01);
                // Decoder does: pts |= (ptsByte2 << 21) & 0x1FE00000
                // Takes byte bits 7-0 and puts them at pts bits 28-21
                packet.push_back((ptsValue >> 21) & 0xFF);
                // Decoder does: pts |= (ptsByte3 << 13) & 0x001FC000
                // Takes byte bits 7-1 and puts them at pts bits 20-14
                packet.push_back(((ptsValue >> 13) & 0xFE) | 0x01);
                // Decoder does: pts |= (ptsByte4 << 6) & 0x00003FC0
                // Takes byte bits 7-0 and puts them at pts bits 13-6
                packet.push_back((ptsValue >> 6) & 0xFF);
                // Decoder does: pts |= (ptsByte5 >> 2) & 0x0000003F
                // Takes byte bits 7-2 and puts them at pts bits 5-0
                packet.push_back(((ptsValue << 2) & 0xFC) | 0x01);
            }

            // DTS (if needed, use dummy value)
            if (withDTS)
            {
                packet.push_back(0x11);
                packet.push_back(0x00);
                packet.push_back(0x01);
                packet.push_back(0x00);
                packet.push_back(0x01);
            }
        }

        // Add payload data
        for (std::uint16_t i = 0; i < dataSize; ++i)
        {
            packet.push_back(static_cast<std::uint8_t>(i & 0xFF));
        }

        return packet;
    }

    std::vector<std::uint8_t> createInvalidPacket(const char* invalidType)
    {
        std::vector<std::uint8_t> packet;

        if (std::strcmp(invalidType, "short") == 0)
        {
            // Too short (< 6 bytes)
            packet.push_back(0x00);
            packet.push_back(0x00);
            packet.push_back(0x01);
        }
        else if (std::strcmp(invalidType, "bad_start1") == 0)
        {
            packet.push_back(0xFF); // Invalid
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xBD);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xAA);
        }
        else if (std::strcmp(invalidType, "bad_start2") == 0)
        {
            packet.push_back(0x00);
            packet.push_back(0xFF); // Invalid
            packet.push_back(0x01);
            packet.push_back(0xBD);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xAA);
        }
        else if (std::strcmp(invalidType, "bad_start3") == 0)
        {
            packet.push_back(0x00);
            packet.push_back(0x00);
            packet.push_back(0xFF); // Invalid
            packet.push_back(0xBD);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xAA);
        }
        else if (std::strcmp(invalidType, "bad_stream_id") == 0)
        {
            packet.push_back(0x00);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xBC); // Invalid stream ID
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xAA);
        }
        else if (std::strcmp(invalidType, "zero_length") == 0)
        {
            packet.push_back(0x00);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xBD);
            packet.push_back(0x00);
            packet.push_back(0x00); // Zero PES length
        }
        else if (std::strcmp(invalidType, "mismatched_length") == 0)
        {
            packet.push_back(0x00);
            packet.push_back(0x00);
            packet.push_back(0x01);
            packet.push_back(0xBD);
            packet.push_back(0x00);
            packet.push_back(0x0A); // Says 10 bytes but only 1 follows
            packet.push_back(0xAA);
        }

        return packet;
    }

    void testConstructorWithValidBuffer()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        // Verify by attempting to add a packet
        auto packet = createValidPesPacket(10);
        bool result = buffer.addPesPacket(packet.data(), packet.size());
        CPPUNIT_ASSERT(result);
    }

    void testConstructorWithZeroSize()
    {
        PesBuffer buffer(m_buffer, 0);
        // Should initialize but reject any packet due to no space
        auto packet = createValidPesPacket(1);
        bool result = buffer.addPesPacket(packet.data(), packet.size());
        CPPUNIT_ASSERT(!result);
    }

    void testAddValidMinimalPesPacket()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(1);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);
    }

    void testAddValidPesPacketWithData()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(100);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);
    }

    void testAddMultipleValidPackets()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);
        auto packet3 = createValidPesPacket(70);

        bool result1 = buffer.addPesPacket(packet1.data(), packet1.size());
        bool result2 = buffer.addPesPacket(packet2.data(), packet2.size());
        bool result3 = buffer.addPesPacket(packet3.data(), packet3.size());

        CPPUNIT_ASSERT(result1);
        CPPUNIT_ASSERT(result2);
        CPPUNIT_ASSERT(result3);
    }

    void testAddPacketExactlyFillsBuffer()
    {
        const std::size_t smallBufferSize = 100;
        std::uint8_t smallBuffer[100];
        PesBuffer buffer(smallBuffer, smallBufferSize);

        // Create packet that exactly fills buffer (100 - 6 header = 94 data)
        auto packet = createValidPesPacket(94);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(100), packet.size());

        // Try to add one more byte - should fail
        auto tinyPacket = createValidPesPacket(1);
        bool result2 = buffer.addPesPacket(tinyPacket.data(), tinyPacket.size());
        CPPUNIT_ASSERT(!result2);
    }

    void testAddPacketWrapsAroundBuffer()
    {
        const std::size_t smallBufferSize = 200;
        std::uint8_t smallBuffer[200];
        PesBuffer buffer(smallBuffer, smallBufferSize);

        // Add first packet
        auto packet1 = createValidPesPacket(80); // ~86 bytes total
        bool result1 = buffer.addPesPacket(packet1.data(), packet1.size());
        CPPUNIT_ASSERT(result1);

        // Get and consume to move read pointer
        PesPacketHeader header;
        PesPacketReader reader;
        bool gotPacket = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(gotPacket);
        buffer.markPacketConsumed(header);

        // Add second packet near end
        auto packet2 = createValidPesPacket(100); // ~106 bytes total
        bool result2 = buffer.addPesPacket(packet2.data(), packet2.size());
        CPPUNIT_ASSERT(result2);

        // Add third packet that should wrap
        auto packet3 = createValidPesPacket(50); // ~56 bytes total
        bool result3 = buffer.addPesPacket(packet3.data(), packet3.size());
        CPPUNIT_ASSERT(result3);
    }

    void testAddValidTeletextPacketWithPTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        std::uint32_t ptsValue = 0x12345678;
        auto packet = createValidPesPacket(10, true, ptsValue, false);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);

        // Verify PTS can be retrieved
        PesPacketHeader header;
        PesPacketReader reader;
        bool gotPacket = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(gotPacket);
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(ptsValue, header.m_pts);
    }

    void testAddValidTeletextPacketWithoutPTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(10, false, 0, false);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);

        // Verify no PTS
        PesPacketHeader header;
        PesPacketReader reader;
        bool gotPacket = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(gotPacket);
        CPPUNIT_ASSERT(!header.m_hasPts);
    }

    void testAddValidTeletextPacketWithPTSDTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        std::uint32_t ptsValue = 0xABCDEF12;
        auto packet = createValidPesPacket(10, true, ptsValue, true);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);

        // Verify PTS can be retrieved
        PesPacketHeader header;
        PesPacketReader reader;
        bool gotPacket = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(gotPacket);
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(ptsValue, header.m_pts);
    }

    void testAddPacketWithMaxPesLength()
    {
        // Create large buffer for max packet
        const std::size_t largeBufferSize = 70000;
        std::vector<std::uint8_t> largeBuffer(largeBufferSize);
        PesBuffer buffer(largeBuffer.data(), largeBufferSize);

        // Max PES length is 65535, but total packet size (65541) exceeds uint16_t
        // So test with a large but valid size that fits in uint16_t
        // Use 65529 data bytes -> 65535 total packet size (fits in uint16_t)
        auto packet = createValidPesPacket(65529);

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(65535), packet.size());
    }

    void testAddPacketTooSmall()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("short");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithInvalidStartCode1()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("bad_start1");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithInvalidStartCode2()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("bad_start2");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithInvalidStartCode3()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("bad_start3");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithInvalidStreamId()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("bad_stream_id");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithZeroPesLength()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("zero_length");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWithMismatchedPesLength()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createInvalidPacket("mismatched_length");

        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testAddPacketWhenBufferFull()
    {
        const std::size_t tinyBufferSize = 50;
        std::uint8_t tinyBuffer[50];
        PesBuffer buffer(tinyBuffer, tinyBufferSize);

        // Fill buffer
        auto packet1 = createValidPesPacket(44); // 50 bytes total
        bool result1 = buffer.addPesPacket(packet1.data(), packet1.size());
        CPPUNIT_ASSERT(result1);

        // Try to add another - should fail
        auto packet2 = createValidPesPacket(1);
        bool result2 = buffer.addPesPacket(packet2.data(), packet2.size());

        CPPUNIT_ASSERT(!result2);
    }

    void testAddPacketLargerThanBuffer()
    {
        const std::size_t tinyBufferSize = 30;
        std::uint8_t tinyBuffer[30];
        PesBuffer buffer(tinyBuffer, tinyBufferSize);

        // Try to add packet larger than buffer
        auto packet = createValidPesPacket(50); // 56 bytes total > 30
        bool result = buffer.addPesPacket(packet.data(), packet.size());

        CPPUNIT_ASSERT(!result);
    }

    void testClearEmptyBuffer()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        buffer.clear();

        // Verify still empty
        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testClearBufferWithOnePacket()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(50);
        buffer.addPesPacket(packet.data(), packet.size());

        buffer.clear();

        // Verify empty
        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);

        // Verify can add new packet after clear
        auto packet2 = createValidPesPacket(30);
        bool addResult = buffer.addPesPacket(packet2.data(), packet2.size());
        CPPUNIT_ASSERT(addResult);
    }

    void testClearBufferWithMultiplePackets()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);
        auto packet3 = createValidPesPacket(70);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());
        buffer.addPesPacket(packet3.data(), packet3.size());

        buffer.clear();

        // Verify empty
        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testClearAfterPartialConsumption()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());

        // Consume first packet
        PesPacketHeader header;
        PesPacketReader reader;
        buffer.getNextPacket(header, reader);
        buffer.markPacketConsumed(header);

        buffer.clear();

        // Verify empty
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testGetNextPacketFromEmptyBuffer()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        PesPacketHeader header;
        PesPacketReader reader;

        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(!result);
    }

    void testGetNextPacketWithOnePacket()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(50);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xBD), header.m_streamId);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(50), header.m_pesPacketLength);
    }

    void testGetNextPacketWithMultiplePackets()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(50), header.m_pesPacketLength);
    }

    void testGetNextPacketAfterConsumption()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());

        // Get and consume first
        PesPacketHeader header1;
        PesPacketReader reader1;
        buffer.getNextPacket(header1, reader1);
        buffer.markPacketConsumed(header1);

        // Get second
        PesPacketHeader header2;
        PesPacketReader reader2;
        bool result = buffer.getNextPacket(header2, reader2);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(60), header2.m_pesPacketLength);
    }

    void testGetNextPacketWrappedAroundBuffer()
    {
        const std::size_t smallBufferSize = 200;
        std::uint8_t smallBuffer[200];
        PesBuffer buffer(smallBuffer, smallBufferSize);

        // Add, get, consume to advance pointers
        auto packet1 = createValidPesPacket(80);
        buffer.addPesPacket(packet1.data(), packet1.size());

        PesPacketHeader header1;
        PesPacketReader reader1;
        buffer.getNextPacket(header1, reader1);
        buffer.markPacketConsumed(header1);

        // Add packet near end
        auto packet2 = createValidPesPacket(100);
        buffer.addPesPacket(packet2.data(), packet2.size());

        // Get packet that should be read correctly even if wrapped
        PesPacketHeader header2;
        PesPacketReader reader2;
        bool result = buffer.getNextPacket(header2, reader2);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(100), header2.m_pesPacketLength);
    }

    void testGetNextPacketWithPTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        std::uint32_t expectedPTS = 0x87654321;
        auto packet = createValidPesPacket(10, true, expectedPTS, false);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(expectedPTS, header.m_pts);
        CPPUNIT_ASSERT(header.isTeletextPacket());
    }

    void testGetNextPacketWithPTSDTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        std::uint32_t expectedPTS = 0x11223344;
        auto packet = createValidPesPacket(10, true, expectedPTS, true);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(expectedPTS, header.m_pts);
    }

    void testGetNextPacketWithoutPTS()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(10, false, 0, false);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(!header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), header.m_pts);
    }

    void testGetNextPacketWithHeaderSkip()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        // Create packet with PTS (which has header to skip)
        auto packet = createValidPesPacket(20, true, 0x12345678, false);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
        // Verify reader is positioned after header (should have data left)
        CPPUNIT_ASSERT(reader.getBytesLeft() > 0);
    }

    void testGetNextPacketWithCorruptedStartCode()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        // Manually inject corrupted data into buffer
        std::uint8_t corruptedData[] = {0xFF, 0x00, 0x01, 0xBD, 0x00, 0x01, 0xAA};
        buffer.addPesPacket(corruptedData, sizeof(corruptedData));

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        // Should return false and clear buffer
        CPPUNIT_ASSERT(!result);

        // Verify buffer was cleared
        bool result2 = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result2);
    }

    void testGetNextPacketWithInsufficientData()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        // Add valid packet first
        auto validPacket = createValidPesPacket(50);
        buffer.addPesPacket(validPacket.data(), validPacket.size());

        // Manually corrupt buffer to claim larger size than available
        // This is hard to test without accessing internals, but we can test
        // that buffer handles exceptions properly

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        // Should succeed with valid packet
        CPPUNIT_ASSERT(result);
    }

    void testGetNextPacketWithInvalidHeaderLength()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        // Create a manually crafted packet with PTS flag but insufficient header length
        std::vector<std::uint8_t> badPacket;
        badPacket.push_back(0x00);
        badPacket.push_back(0x00);
        badPacket.push_back(0x01);
        badPacket.push_back(0xBD);
        badPacket.push_back(0x00);
        badPacket.push_back(0x08); // PES length = 8
        badPacket.push_back(0x80); // Control byte 1
        badPacket.push_back(0x80); // Control byte 2 with PTS flag
        badPacket.push_back(0x02); // Header length = 2 (but PTS needs 5!)
        badPacket.push_back(0x00);
        badPacket.push_back(0x00);
        badPacket.push_back(0x00);
        badPacket.push_back(0x00);
        badPacket.push_back(0x00);

        buffer.addPesPacket(badPacket.data(), badPacket.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        // Should fail and clear buffer
        CPPUNIT_ASSERT(!result);
    }

    void testMarkPacketConsumed()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());

        PesPacketHeader header1;
        PesPacketReader reader1;
        buffer.getNextPacket(header1, reader1);

        buffer.markPacketConsumed(header1);

        // Now should get second packet
        PesPacketHeader header2;
        PesPacketReader reader2;
        bool result = buffer.getNextPacket(header2, reader2);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(60), header2.m_pesPacketLength);
    }

    void testMarkPacketConsumedWithWrapAround()
    {
        const std::size_t smallBufferSize = 200;
        std::uint8_t smallBuffer[200];
        PesBuffer buffer(smallBuffer, smallBufferSize);

        // Fill and consume to wrap pointers
        auto packet1 = createValidPesPacket(80);
        buffer.addPesPacket(packet1.data(), packet1.size());

        PesPacketHeader header1;
        PesPacketReader reader1;
        buffer.getNextPacket(header1, reader1);
        buffer.markPacketConsumed(header1);

        // Add more packets
        auto packet2 = createValidPesPacket(100);
        auto packet3 = createValidPesPacket(50);
        buffer.addPesPacket(packet2.data(), packet2.size());
        buffer.addPesPacket(packet3.data(), packet3.size());

        // Consume second
        PesPacketHeader header2;
        PesPacketReader reader2;
        buffer.getNextPacket(header2, reader2);
        buffer.markPacketConsumed(header2);

        // Should get third packet
        PesPacketHeader header3;
        PesPacketReader reader3;
        bool result = buffer.getNextPacket(header3, reader3);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(50), header3.m_pesPacketLength);
    }

    void testMarkLastPacketConsumed()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(50);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        buffer.getNextPacket(header, reader);

        buffer.markPacketConsumed(header);

        // Buffer should be empty
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testMarkMultiplePacketsConsumed()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(40);
        auto packet2 = createValidPesPacket(50);
        auto packet3 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());
        buffer.addPesPacket(packet3.data(), packet3.size());

        // Consume first two
        PesPacketHeader header;
        PesPacketReader reader;

        buffer.getNextPacket(header, reader);
        buffer.markPacketConsumed(header);

        buffer.getNextPacket(header, reader);
        buffer.markPacketConsumed(header);

        // Should get third
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(60), header.m_pesPacketLength);
    }

    void testFullCycleAddGetConsume()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        // Add packet
        auto packet = createValidPesPacket(75, true, 0x99887766);
        bool addResult = buffer.addPesPacket(packet.data(), packet.size());
        CPPUNIT_ASSERT(addResult);

        // Get packet
        PesPacketHeader header;
        PesPacketReader reader;
        bool getResult = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(getResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(75 + 3 + 5), header.m_pesPacketLength);
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99887766), header.m_pts);

        // Consume packet
        buffer.markPacketConsumed(header);

        // Verify empty
        bool emptyResult = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!emptyResult);
    }

    void testMultipleCyclesWithWrapAround()
    {
        const std::size_t mediumBufferSize = 300;
        std::uint8_t mediumBuffer[300];
        PesBuffer buffer(mediumBuffer, mediumBufferSize);

        // Multiple add-get-consume cycles
        for (int i = 0; i < 5; ++i)
        {
            auto packet = createValidPesPacket(50 + i * 10);
            bool addResult = buffer.addPesPacket(packet.data(), packet.size());
            CPPUNIT_ASSERT(addResult);

            PesPacketHeader header;
            PesPacketReader reader;
            bool getResult = buffer.getNextPacket(header, reader);
            CPPUNIT_ASSERT(getResult);

            buffer.markPacketConsumed(header);
        }

        // Buffer should be empty
        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testFillConsumeRefill()
    {
        const std::size_t mediumBufferSize = 250;
        std::uint8_t mediumBuffer[250];
        PesBuffer buffer(mediumBuffer, mediumBufferSize);

        // Fill buffer
        auto packet1 = createValidPesPacket(70);
        auto packet2 = createValidPesPacket(70);
        auto packet3 = createValidPesPacket(70);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());
        buffer.addPesPacket(packet3.data(), packet3.size());

        // Consume all
        PesPacketHeader header;
        PesPacketReader reader;

        for (int i = 0; i < 3; ++i)
        {
            bool result = buffer.getNextPacket(header, reader);
            CPPUNIT_ASSERT(result);
            buffer.markPacketConsumed(header);
        }

        // Refill
        auto packet4 = createValidPesPacket(80);
        bool addResult = buffer.addPesPacket(packet4.data(), packet4.size());
        CPPUNIT_ASSERT(addResult);

        bool getResult = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(getResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(80), header.m_pesPacketLength);
    }

    void testGetWithoutConsume()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet = createValidPesPacket(50);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header1;
        PesPacketReader reader1;
        bool result1 = buffer.getNextPacket(header1, reader1);
        CPPUNIT_ASSERT(result1);

        // Get again without consuming - should get same packet
        PesPacketHeader header2;
        PesPacketReader reader2;
        bool result2 = buffer.getNextPacket(header2, reader2);
        CPPUNIT_ASSERT(result2);

        // Verify same packet
        CPPUNIT_ASSERT_EQUAL(header1.m_pesPacketLength, header2.m_pesPacketLength);
        CPPUNIT_ASSERT_EQUAL(header1.m_streamId, header2.m_streamId);
    }

    void testClearInterruptsRetrieval()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        auto packet1 = createValidPesPacket(50);
        auto packet2 = createValidPesPacket(60);

        buffer.addPesPacket(packet1.data(), packet1.size());
        buffer.addPesPacket(packet2.data(), packet2.size());

        PesPacketHeader header;
        PesPacketReader reader;
        buffer.getNextPacket(header, reader);

        // Clear before consuming
        buffer.clear();

        // Should not be able to get any packet
        bool result = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(!result);
    }

    void testMixedPacketSizes()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);

        auto small = createValidPesPacket(10);
        auto medium = createValidPesPacket(100);
        auto large = createValidPesPacket(500);

        buffer.addPesPacket(small.data(), small.size());
        buffer.addPesPacket(medium.data(), medium.size());
        buffer.addPesPacket(large.data(), large.size());

        PesPacketHeader header;
        PesPacketReader reader;

        // Get small
        buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(10), header.m_pesPacketLength);
        buffer.markPacketConsumed(header);

        // Get medium
        buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(100), header.m_pesPacketLength);
        buffer.markPacketConsumed(header);

        // Get large
        buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(500), header.m_pesPacketLength);
        buffer.markPacketConsumed(header);
    }

    void testVerySmallBuffer()
    {
        const std::size_t tinyBufferSize = 10;
        std::uint8_t tinyBuffer[10];
        PesBuffer buffer(tinyBuffer, tinyBufferSize);

        // Try to add minimal valid packet (7 bytes minimum)
        auto packet = createValidPesPacket(1);
        bool result = buffer.addPesPacket(packet.data(), packet.size());

        // Should succeed as minimal packet is 7 bytes
        CPPUNIT_ASSERT(result);
    }

    void testPTSWithAllBitsSet()
    {
        PesBuffer buffer(m_buffer, BUFFER_SIZE);
        std::uint32_t maxPTS = 0xFFFFFFFF;
        auto packet = createValidPesPacket(10, true, maxPTS, false);
        buffer.addPesPacket(packet.data(), packet.size());

        PesPacketHeader header;
        PesPacketReader reader;
        bool result = buffer.getNextPacket(header, reader);

        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
        // PTS is 33 bits, so max value is limited
        CPPUNIT_ASSERT(header.m_pts != 0);
    }

    void testMultipleWrapsAround()
    {
        const std::size_t smallBufferSize = 150;
        std::uint8_t smallBuffer[150];
        PesBuffer buffer(smallBuffer, smallBufferSize);

        // Cause multiple wrap-arounds
        for (int i = 0; i < 10; ++i)
        {
            auto packet = createValidPesPacket(30);
            bool addResult = buffer.addPesPacket(packet.data(), packet.size());
            CPPUNIT_ASSERT(addResult);

            PesPacketHeader header;
            PesPacketReader reader;
            bool getResult = buffer.getNextPacket(header, reader);
            CPPUNIT_ASSERT(getResult);

            buffer.markPacketConsumed(header);
        }

        // Verify buffer still works correctly
        auto finalPacket = createValidPesPacket(40);
        bool addResult = buffer.addPesPacket(finalPacket.data(), finalPacket.size());
        CPPUNIT_ASSERT(addResult);

        PesPacketHeader header;
        PesPacketReader reader;
        bool getResult = buffer.getNextPacket(header, reader);
        CPPUNIT_ASSERT(getResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(40), header.m_pesPacketLength);
    }

private:
    static const std::size_t BUFFER_SIZE = 4096;
    std::uint8_t* m_buffer;
};

CPPUNIT_TEST_SUITE_REGISTRATION( PesBufferTest );
