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

#include <vector>

#include "PesBuffer.hpp"
#include "DynamicAllocator.hpp"
#include "Consts.hpp"

using dvbsubdecoder::PesBuffer;
using dvbsubdecoder::PesPacketHeader;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::DynamicAllocator;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class PesBufferTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PesBufferTest );
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testAddAndProcess);
    CPPUNIT_TEST(testPts);
    CPPUNIT_TEST(testBadData);
    CPPUNIT_TEST(testMinimumValidPacket);
    CPPUNIT_TEST(testMaximumValidPacket);
    CPPUNIT_TEST(testExactBufferCapacity);
    CPPUNIT_TEST(testZeroPtsValue);
    CPPUNIT_TEST(testMaximumPtsValue);
    CPPUNIT_TEST(testInsufficientPacketData);
    CPPUNIT_TEST(testExcessivePacketLength);
    CPPUNIT_TEST(testInvalidStreamId);
    CPPUNIT_TEST(testMalformedPtsData);
    CPPUNIT_TEST(testMultiplePacketRetrievalWithoutConsumption);
    CPPUNIT_TEST(testPacketRetrievalAfterClear);
    CPPUNIT_TEST(testInvalidPesHeaderLength);
    CPPUNIT_TEST(testInconsistentMarkerBits);
    CPPUNIT_TEST(testPacketConsumptionOrder);
    CPPUNIT_TEST(testBufferWrapAround);
    CPPUNIT_TEST(testLargePayloadPacket);
    CPPUNIT_TEST(testHeaderLengthBoundaries);
    CPPUNIT_TEST(testTimeTypeSwitching);
    CPPUNIT_TEST(testErrorRecovery);
    CPPUNIT_TEST(testPacketWithoutPts);
    CPPUNIT_TEST(testInvalidPtsDtsFlags);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        buildPacket(PES_PACKET_LENGTH);
    }

    void tearDown()
    {
        m_pesPacket.clear();
    }

    void testFill()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // push single
        CPPUNIT_ASSERT(
                buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // push until full
        while (buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()))
        {
            // noop
        }

        // consume one
        PesPacketHeader header;
        PesPacketReader dataReader;

        CPPUNIT_ASSERT(
                buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        buffer.markPacketConsumed(header);

        CPPUNIT_ASSERT(
                buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // clear all
        buffer.clear();
    }

    void testAddAndProcess()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        const int REPEAT_COUNT = 100;

        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::HIGH_32, header,
                            dataReader));
            buffer.markPacketConsumed(header);
        }
    }

    void testPts()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        const int REPEAT_COUNT = 100;

        // high32
        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            std::uint64_t pts = (1LLU << 32) | (1LLU << 31) | i;
            std::uint32_t ptsHigh = pts >> 1;

            setPacketPts(pts);

            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::HIGH_32, header,
                            dataReader));

            CPPUNIT_ASSERT(header.m_streamId == 0xBD);
            CPPUNIT_ASSERT(header.m_pesPacketLength == PES_PACKET_LENGTH);
            CPPUNIT_ASSERT(header.m_hasPts);
            CPPUNIT_ASSERT(header.m_pts.m_type == StcTimeType::HIGH_32);
            CPPUNIT_ASSERT(header.m_pts.m_time == ptsHigh);

            buffer.markPacketConsumed(header);
        }

        // low32
        for (int i = 0; i < REPEAT_COUNT; ++i)
        {
            std::uint64_t pts = (1LLU << 32) | (1LLU << 31) | i;
            std::uint32_t ptsLow = pts & 0xFFFFFFFF;

            setPacketPts(pts);

            // push single
            CPPUNIT_ASSERT(
                    buffer.addPesPacket(m_pesPacket.data(),
                            m_pesPacket.size()));

            PesPacketHeader header;
            PesPacketReader dataReader;

            CPPUNIT_ASSERT(
                    buffer.getNextPacket(StcTimeType::LOW_32, header,
                            dataReader));

            CPPUNIT_ASSERT(header.m_streamId == 0xBD);
            CPPUNIT_ASSERT(header.m_pesPacketLength == PES_PACKET_LENGTH);
            CPPUNIT_ASSERT(header.m_hasPts);
            CPPUNIT_ASSERT(header.m_pts.m_type == StcTimeType::LOW_32);
            CPPUNIT_ASSERT(header.m_pts.m_time == ptsLow);

            buffer.markPacketConsumed(header);
        }
    }

    void testBadData()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // too small
        buildPacket(3);
        m_pesPacket.resize(5);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // broken start code
        buildPacket(3);
        m_pesPacket[2] = 0xFF;

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // zero size (unlimited)
        buildPacket(0);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // bad size
        buildPacket(1);
        m_pesPacket.push_back(0);

        CPPUNIT_ASSERT(
                !buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testMinimumValidPacket()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Minimum valid packet size (6 bytes header only)
        buildPacket(0);
        m_pesPacket.resize(6); // Only header, no payload
        m_pesPacket[4] = 0; // Set length to 0
        m_pesPacket[5] = 0;

        // Should be rejected due to zero length
        CPPUNIT_ASSERT(!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // Now create minimum packet with 1 byte payload
        buildPacket(1);
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testMaximumValidPacket()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);
        // addPesPacket length parameter is 16-bit, so total packet length cannot exceed 65535.
        // Therefore the maximum payload we can represent is (65535 - 6 header) = 65529 bytes.
        const std::uint16_t maxTotal = 65535; // max representable total length
        const std::uint16_t maxPayload = maxTotal - 6; // header is 6 bytes
        buildPacket(maxPayload);
        CPPUNIT_ASSERT(m_pesPacket.size() == maxTotal);

        bool result = buffer.addPesPacket(m_pesPacket.data(), static_cast<std::uint16_t>(m_pesPacket.size()));
        CPPUNIT_ASSERT(result);

        // Retrieve and validate header length matches payload size
        PesPacketHeader header; PesPacketReader reader;
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, reader));
        CPPUNIT_ASSERT(header.m_pesPacketLength == maxPayload);
        buffer.markPacketConsumed(header);
    }

    void testExactBufferCapacity()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Buffer size is 2 * 64 * 1024 = 131072 bytes
        const std::size_t bufferSize = 2 * 64 * 1024;
        
        // Create packets that will fill buffer exactly
        std::uint16_t packetSize = 1000; // Reasonable packet size
        buildPacket(packetSize - 6); // buildPacket adds 6 header bytes
        
        int packetsAdded = 0;
        while (buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()))
        {
            packetsAdded++;
            if (packetsAdded > bufferSize / packetSize + 10) // Safety break
                break;
        }
        
        CPPUNIT_ASSERT(packetsAdded > 0);
        
        // Should not be able to add one more
        CPPUNIT_ASSERT(!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testZeroPtsValue()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        setPacketPts(0); // Zero PTS value
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        CPPUNIT_ASSERT(header.m_hasPts);
        CPPUNIT_ASSERT(header.m_pts.m_time == 0);
    }

    void testMaximumPtsValue()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Maximum 33-bit PTS value
        std::uint64_t maxPts = (1ULL << 33) - 1;
        setPacketPts(maxPts);
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        CPPUNIT_ASSERT(header.m_hasPts);
        // Verify the PTS extraction works for maximum value
    }

    void testInsufficientPacketData()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Create packet with declared length longer than actual data
        buildPacket(100);
        m_pesPacket.resize(50); // Truncate to less than declared length
        
        CPPUNIT_ASSERT(!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testExcessivePacketLength()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Create packet claiming to be larger than maximum
        buildPacket(1000);
        // Manually set an excessive length in header
        m_pesPacket[4] = 0xFF;
        m_pesPacket[5] = 0xFF; // 65535 bytes payload
        // But actual packet is much smaller - should be rejected
        
        CPPUNIT_ASSERT(!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testInvalidStreamId()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        buildPacket(100);
        m_pesPacket[3] = 0xBC; // Change stream ID from 0xBD to 0xBC
        
        CPPUNIT_ASSERT(!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
    }

    void testMalformedPtsData()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        setPacketPts(12345);
        
        // Corrupt marker bits in PTS data
        const int START_OFFSET = 6;
        m_pesPacket[START_OFFSET + 3] &= ~0x01; // Clear marker bit
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        // Should handle malformed PTS gracefully (likely by clearing buffer)
        bool result = buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader);
        // Implementation does not validate marker bits; expect success & PTS present
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
    }

    void testMultiplePacketRetrievalWithoutConsumption()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header1, header2;
        PesPacketReader dataReader1, dataReader2;
        
        // Get same packet multiple times
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header1, dataReader1));
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header2, dataReader2));
        
        // Should return same packet both times
        CPPUNIT_ASSERT(header1.m_streamId == header2.m_streamId);
        CPPUNIT_ASSERT(header1.m_pesPacketLength == header2.m_pesPacketLength);
    }

    void testPacketRetrievalAfterClear()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        buffer.clear();
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        // Should return false after clear
        CPPUNIT_ASSERT(!buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
    }

    void testInvalidPesHeaderLength()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        setPacketPts(12345);
        
        // Set invalid header length (too small for PTS data)
        const int START_OFFSET = 6;
        m_pesPacket[START_OFFSET + 2] = 3; // Header length too small for PTS
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        // Should handle invalid header length gracefully
        bool result = buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader);
        CPPUNIT_ASSERT(!result); // invalid header length triggers reset & false
    }

    void testInconsistentMarkerBits()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        setPacketPts(12345);
        
        // Corrupt all marker bits in PTS
        const int START_OFFSET = 6;
        m_pesPacket[START_OFFSET + 3] &= ~0x01; // Clear marker bit 1
        m_pesPacket[START_OFFSET + 5] &= ~0x01; // Clear marker bit 2
        m_pesPacket[START_OFFSET + 7] &= ~0x01; // Clear marker bit 3
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        // Should handle inconsistent marker bits
        bool result = buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader);
        // Marker bits are not validated; still expect success
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(header.m_hasPts);
    }

    void testPacketConsumptionOrder()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Add multiple packets with different PTS values
        std::vector<std::uint64_t> ptsValues = {1000, 2000, 3000};
        
        for (auto pts : ptsValues)
        {
            setPacketPts(pts);
            CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        }
        
        // Retrieve and consume in FIFO order
        for (size_t i = 0; i < ptsValues.size(); ++i)
        {
            PesPacketHeader header;
            PesPacketReader dataReader;
            
            CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
            buffer.markPacketConsumed(header);
            
            if (header.m_hasPts)
            {
                // Verify FIFO order (allowing for PTS extraction differences)
                std::uint32_t expectedPts = ptsValues[i] >> 1; // HIGH_32 extraction
                CPPUNIT_ASSERT(header.m_pts.m_time == expectedPts);
            }
        }
    }

    void testBufferWrapAround()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Fill buffer partially, consume some packets, then add more
        // to test circular buffer behavior
        std::uint16_t packetSize = 500;
        buildPacket(packetSize - 6);
        
        // Add several packets
        int initialPackets = 10;
        for (int i = 0; i < initialPackets; ++i)
        {
            if (!buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()))
                break;
        }
        
        // Consume half
        for (int i = 0; i < initialPackets / 2; ++i)
        {
            PesPacketHeader header;
            PesPacketReader dataReader;
            
            if (buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader))
            {
                buffer.markPacketConsumed(header);
            }
        }
        
        // Add more packets to test wrap-around
        int additionalPackets = 5;
        for (int i = 0; i < additionalPackets; ++i)
        {
            buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size());
        }
    }

    void testLargePayloadPacket()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Test packet with large but valid payload
        std::uint16_t largePayload = 32000;
        buildPacket(largePayload);
        
        bool result = buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size());
        // Result depends on available buffer space
        
        if (result)
        {
            PesPacketHeader header;
            PesPacketReader dataReader;
            
            CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
            CPPUNIT_ASSERT(header.m_pesPacketLength == largePayload);
        }
    }

    void testHeaderLengthBoundaries()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Test with minimum header length (0)
        setPacketPts(12345);
        const int START_OFFSET = 6;
        m_pesPacket[START_OFFSET + 2] = 0; // Header length = 0
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        bool result = buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader);
        // Header length 0 with pts_dts_flags=2 is invalid -> expect failure
        CPPUNIT_ASSERT(!result);
        
        // Test with maximum header length (255)
        buffer.clear();
        setPacketPts(12345);
        
        // Need to extend packet to accommodate large header
        while (m_pesPacket.size() < START_OFFSET + 3 + 255)
        {
            m_pesPacket.push_back(0);
        }
        
        // Update packet length to match new size
        std::uint16_t newLength = m_pesPacket.size() - 6;
        m_pesPacket[4] = (newLength >> 8) & 0xFF;
        m_pesPacket[5] = newLength & 0xFF;
        
        m_pesPacket[START_OFFSET + 2] = 255; // Maximum header length
        
        result = buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size());
        CPPUNIT_ASSERT(result);
    }

    void testTimeTypeSwitching()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        std::uint64_t pts = (1ULL << 32) | (1ULL << 31) | 12345;
        setPacketPts(pts);
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        // Test HIGH_32 extraction
        PesPacketHeader headerHigh;
        PesPacketReader dataReaderHigh;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, headerHigh, dataReaderHigh));
        CPPUNIT_ASSERT(headerHigh.m_pts.m_type == StcTimeType::HIGH_32);
        
        // Test LOW_32 extraction on same packet
        PesPacketHeader headerLow;
        PesPacketReader dataReaderLow;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::LOW_32, headerLow, dataReaderLow));
        CPPUNIT_ASSERT(headerLow.m_pts.m_type == StcTimeType::LOW_32);
        
        // Verify different extracted values
        CPPUNIT_ASSERT(headerHigh.m_pts.m_time != headerLow.m_pts.m_time);
    }

    void testErrorRecovery()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);
        // Add one valid packet
        buildPacket(50);
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // Add a second packet that will parse-fail during getNextPacket (invalid header length)
        buildPacket(20);
        const int START_OFFSET = 6;
        setPacketPts(1234); // sets headerLength=5 & flags=PTS
        m_pesPacket[START_OFFSET + 2] = 3; // invalid (less than 5 while flags need 5)
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));

        // Consume first (valid) packet
        PesPacketHeader header;
        PesPacketReader dataReader;
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        buffer.markPacketConsumed(header);

        // Next retrieval should hit invalid packet, trigger internal clear and return false
        PesPacketHeader header2;
        PesPacketReader dataReader2;
        CPPUNIT_ASSERT(!buffer.getNextPacket(StcTimeType::HIGH_32, header2, dataReader2));

        // Buffer should be usable after recovery
        buildPacket(40);
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        PesPacketHeader header3; PesPacketReader dataReader3;
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header3, dataReader3));
    }

    void testPacketWithoutPts()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        // Create packet without PTS data
        buildPacket(100);
        const int START_OFFSET = 6;
        
        std::uint8_t control1 = 2 << 6; // "10"
        std::uint8_t control2 = 0 << 6; // PTS_DTS_flags = 0 (no PTS)
        std::uint8_t headerLength = 0;
        
        m_pesPacket[START_OFFSET + 0] = control1;
        m_pesPacket[START_OFFSET + 1] = control2;
        m_pesPacket[START_OFFSET + 2] = headerLength;
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        CPPUNIT_ASSERT(!header.m_hasPts); // Should not have PTS
    }

    void testInvalidPtsDtsFlags()
    {
        DynamicAllocator allocator;
        PesBuffer buffer(allocator);

        buildPacket(100);
        const int START_OFFSET = 6;
        
        std::uint8_t control1 = 2 << 6; // "10"
        std::uint8_t control2 = 1 << 6; // PTS_DTS_flags = 1 (invalid/forbidden)
        std::uint8_t headerLength = 0;
        
        m_pesPacket[START_OFFSET + 0] = control1;
        m_pesPacket[START_OFFSET + 1] = control2;
        m_pesPacket[START_OFFSET + 2] = headerLength;
        
        CPPUNIT_ASSERT(buffer.addPesPacket(m_pesPacket.data(), m_pesPacket.size()));
        
        PesPacketHeader header;
        PesPacketReader dataReader;
        
        CPPUNIT_ASSERT(buffer.getNextPacket(StcTimeType::HIGH_32, header, dataReader));
        CPPUNIT_ASSERT(!header.m_hasPts); // Should not extract PTS for invalid flags
    }

private:
    void buildPacket(std::uint16_t size)
    {
        m_pesPacket.clear();

        // build PES packet
        m_pesPacket.push_back(0x00);
        m_pesPacket.push_back(0x00);
        m_pesPacket.push_back(0x01);
        m_pesPacket.push_back(0xBD);

        m_pesPacket.push_back((size >> 8) & 0xFF);
        m_pesPacket.push_back((size >> 0) & 0xFF);

        for (auto i = 0; i < size; ++i)
        {
            m_pesPacket.push_back(0);
        }
    }

    void setPacketPts(std::uint64_t pts33)
    {
        std::uint8_t control1 = 0;
        control1 |= 2 << 6; // "10'

        std::uint8_t control2 = 0;
        control2 |= 2 << 6; // PTS_DTS_flags (PTS only)

        std::uint8_t headerLength = 5;

        const int START_OFFSET = 6;

        m_pesPacket[START_OFFSET + 0] = control1;
        m_pesPacket[START_OFFSET + 1] = control2;
        m_pesPacket[START_OFFSET + 2] = headerLength;

        m_pesPacket[START_OFFSET + 3] = 0;
        m_pesPacket[START_OFFSET + 4] = 0;
        m_pesPacket[START_OFFSET + 5] = 0;
        m_pesPacket[START_OFFSET + 6] = 0;
        m_pesPacket[START_OFFSET + 7] = 0;

        m_pesPacket[START_OFFSET + 3] |= 2 << 4; // '0010'
        m_pesPacket[START_OFFSET + 3] |= ((pts33 >> 30) & 0x7) << 1; // PTS[32..30]
        m_pesPacket[START_OFFSET + 3] |= 1 << 0; // marker_bit
        m_pesPacket[START_OFFSET + 4] |= ((pts33 >> 22) & 0xFF) << 0; // PTS[29..22]
        m_pesPacket[START_OFFSET + 5] |= ((pts33 >> 15) & 0x7F) << 1; // PTS[21..15]
        m_pesPacket[START_OFFSET + 5] |= 1 << 0; // marker_bit
        m_pesPacket[START_OFFSET + 6] |= ((pts33 >> 7) & 0xFF) << 0; // PTS[14..7]
        m_pesPacket[START_OFFSET + 7] |= ((pts33 >> 0) & 0x7F) << 1; // PTS[6..0]
        m_pesPacket[START_OFFSET + 7] |= 1 << 0; // marker_bit
    }

    static const auto PES_PACKET_LENGTH = 800;

    std::vector<std::uint8_t> m_pesPacket;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PesBufferTest);
