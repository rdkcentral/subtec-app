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
#include "CcCaptionChannelPacket.hpp"
#include "CcExceptions.hpp"

using namespace subttxrend::cc;

class CcCaptionChannelPacketTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcCaptionChannelPacketTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testParameterizedConstructor);
    CPPUNIT_TEST(testAddCcDataValidStart);
    CPPUNIT_TEST(testAddCcDataValidData);
    CPPUNIT_TEST(testGetCcpData);
    CPPUNIT_TEST(testGetSize);
    CPPUNIT_TEST(testIsFull);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testAddInvalidCcData);
    CPPUNIT_TEST(testAddDataToNotStartedPacket);
    CPPUNIT_TEST(testAddStartToAlreadyStartedPacket);
    CPPUNIT_TEST(testAddMoreDataThanAllowed);
    CPPUNIT_TEST(testZeroSizePacket);
    CPPUNIT_TEST(testMaxSizePacket);
    CPPUNIT_TEST(testSingleBytePacket);
    CPPUNIT_TEST(testSequenceNumbers);
    CPPUNIT_TEST(testMultipleResets);
    CPPUNIT_TEST(testConstructorWithInvalidData);
    CPPUNIT_TEST(testIsFullAfterReset);
    CPPUNIT_TEST(testGettersAfterReset);
    CPPUNIT_TEST(testCompletePacketFlow);
    CPPUNIT_TEST(testMultipleDataPackets);
    CPPUNIT_TEST(testPacketReusability);
    CPPUNIT_TEST(testDifferentSequenceNumbers);
    CPPUNIT_TEST(testMixedValidInvalidOperations);
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

    // Helper methods to create test CcData objects
    CcData createValidStartData(uint8_t seqNo = 0, uint8_t size = 4)
    {
        CcData ccData;
        ccData.ccValid = true;
        ccData.ccType = CcData::CcType::DTVCC_CCP_START;
        ccData.data1 = (seqNo << 6) | size;  // seqNo in upper 2 bits, size in lower 6 bits
        ccData.data2 = 0x01;  // Some data byte
        return ccData;
    }

    CcData createValidDataData(uint8_t data1 = 0x42, uint8_t data2 = 0x43)
    {
        CcData ccData;
        ccData.ccValid = true;
        ccData.ccType = CcData::CcType::DTVCC_CCP_DATA;
        ccData.data1 = data1;
        ccData.data2 = data2;
        return ccData;
    }

    CcData createInvalidData()
    {
        CcData ccData;
        ccData.ccValid = false;
        ccData.ccType = CcData::CcType::DTVCC_CCP_START;
        ccData.data1 = 0x04;
        ccData.data2 = 0x01;
        return ccData;
    }

    void testDefaultConstructor()
    {
        CaptionChannelPacket packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testParameterizedConstructor()
    {
        CcData startData = createValidStartData(1, 6);  // seqNo=1, size=6

        CaptionChannelPacket packet(startData);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());  // size=6 means 6*2=12 bytes
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), packet.getCcpData()[0]);
    }

    void testAddCcDataValidStart()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(2, 4);  // seqNo=2, size=4

        packet.addCcData(startData);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getSize());  // size=4 means 4*2=8 bytes
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testAddCcDataValidData()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 4);
        CcData dataPacket = createValidDataData(0x42, 0x43);

        packet.addCcData(startData);
        packet.addCcData(dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), packet.getCcpData()[0]);  // data2 from start
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x42), packet.getCcpData()[1]);  // data1 from data
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x43), packet.getCcpData()[2]);  // data2 from data
    }

    void testGetCcpData()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 3);

        packet.addCcData(startData);
        const std::vector<std::uint8_t>& data = packet.getCcpData();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), data[0]);
    }

    void testGetSize()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 10);

        packet.addCcData(startData);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(20), packet.getSize());  // 10 * 2 = 20
    }

    void testIsFull()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 2);  // Size = 2*2 = 4 bytes total

        packet.addCcData(startData);
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());  // Has 1 byte, needs 4 total (3 more)

        CcData dataPacket = createValidDataData(0x42, 0x43);
        packet.addCcData(dataPacket);
        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());   // Has 3 bytes, size is 4, so 3+1=4 (full)
    }

    void testReset()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(1, 5);

        packet.addCcData(startData);
        packet.reset();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testAddInvalidCcData()
    {
        CaptionChannelPacket packet;
        CcData invalidData = createInvalidData();

        CPPUNIT_ASSERT_THROW(packet.addCcData(invalidData), InvalidOperation);
    }

    void testAddDataToNotStartedPacket()
    {
        CaptionChannelPacket packet;
        CcData dataPacket = createValidDataData();

        CPPUNIT_ASSERT_THROW(packet.addCcData(dataPacket), InvalidOperation);
    }

    void testAddStartToAlreadyStartedPacket()
    {
        CaptionChannelPacket packet;
        CcData startData1 = createValidStartData(0, 4);
        CcData startData2 = createValidStartData(1, 6);

        packet.addCcData(startData1);
        CPPUNIT_ASSERT_THROW(packet.addCcData(startData2), InvalidOperation);
    }

    void testAddMoreDataThanAllowed()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 2);  // Size = 2*2 = 4 bytes total

        packet.addCcData(startData);  // Adds 1 byte (data2)

        CcData dataPacket1 = createValidDataData();  // Will add 2 more bytes (total 3)
        packet.addCcData(dataPacket1);  // This should work, total = 3, size = 4

        CcData dataPacket2 = createValidDataData();  // Would add 2 more bytes (total 5 > 4)
        CPPUNIT_ASSERT_THROW(packet.addCcData(dataPacket2), InvalidOperation);
    }

    void testZeroSizePacket()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 0);  // Size = 0 should map to 128 (CCP_SIZE_MAX)

        packet.addCcData(startData);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(128), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testMaxSizePacket()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 63);  // Max size field value (63*2 = 126)

        packet.addCcData(startData);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(126), packet.getSize());
    }

    void testSingleBytePacket()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 1);  // Size = 1*2 = 2 bytes total

        packet.addCcData(startData);  // Adds 1 byte

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());  // 1 byte data + 1 header = 2 total
    }

    void testSequenceNumbers()
    {
        // Test all possible sequence numbers (0-3)
        for (uint8_t seqNo = 0; seqNo < 4; ++seqNo)
        {
            CaptionChannelPacket packet;
            CcData startData = createValidStartData(seqNo, 2);

            packet.addCcData(startData);

            // We can't directly test seqNo as it's private, but we can verify the packet works
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        }
    }

    void testMultipleResets()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(1, 3);

        packet.addCcData(startData);
        packet.reset();
        packet.reset();  // Multiple resets should be safe

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testConstructorWithInvalidData()
    {
        CcData invalidData = createInvalidData();

        CPPUNIT_ASSERT_THROW(CaptionChannelPacket packet(invalidData), InvalidOperation);
    }

    void testIsFullAfterReset()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 1);

        packet.addCcData(startData);
        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());

        packet.reset();
        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
    }

    void testGettersAfterReset()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(2, 5);

        packet.addCcData(startData);
        CPPUNIT_ASSERT(packet.getSize() > 0);
        CPPUNIT_ASSERT(packet.getCcpData().size() > 0);

        packet.reset();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getCcpData().size());
    }

    void testCompletePacketFlow()
    {
        CaptionChannelPacket packet;

        // Start a packet
        CcData startData = createValidStartData(1, 4);  // 8 bytes total
        packet.addCcData(startData);

        // Add data until full
        CcData dataPacket1 = createValidDataData(0x41, 0x42);
        packet.addCcData(dataPacket1);

        CcData dataPacket2 = createValidDataData(0x43, 0x44);
        packet.addCcData(dataPacket2);

        CcData dataPacket3 = createValidDataData(0x45, 0x46);
        packet.addCcData(dataPacket3);

        // Verify full packet
        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), packet.getCcpData().size());  // 7 data bytes (8-1 header)

        // Verify data integrity
        const std::vector<std::uint8_t>& data = packet.getCcpData();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), data[0]);  // start data2
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x41), data[1]);  // data1 from first data packet
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x42), data[2]);  // data2 from first data packet
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x43), data[3]);  // data1 from second data packet
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x44), data[4]);  // data2 from second data packet
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x45), data[5]);  // data1 from third data packet
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x46), data[6]);  // data2 from third data packet
    }

    void testMultipleDataPackets()
    {
        CaptionChannelPacket packet;
        CcData startData = createValidStartData(0, 10);  // 20 bytes total

        packet.addCcData(startData);

        // Add multiple data packets
        for (int i = 0; i < 8; ++i)  // 8 data packets = 16 bytes + 1 start = 17 bytes (not full, can add 1 more)
        {
            CcData dataPacket = createValidDataData(0x30 + i, 0x40 + i);
            packet.addCcData(dataPacket);
        }

        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(17), packet.getCcpData().size());  // 17 data bytes

        // Verify data integrity for first and last packets
        const std::vector<std::uint8_t>& data = packet.getCcpData();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), data[0]);   // start data2
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x30), data[1]);   // first data packet data1
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x37), data[15]);  // last data packet data1 (0x30 + 7)
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x47), data[16]);  // last data packet data2 (0x40 + 7)
    }

    void testPacketReusability()
    {
        CaptionChannelPacket packet;

        // First use
        CcData startData1 = createValidStartData(1, 2);
        packet.addCcData(startData1);
        CcData dataPacket1 = createValidDataData(0xAA, 0xBB);
        packet.addCcData(dataPacket1);

        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());

        // Reset and reuse
        packet.reset();

        // Second use with different data
        CcData startData2 = createValidStartData(2, 3);
        packet.addCcData(startData2);
        CcData dataPacket2 = createValidDataData(0xCC, 0xDD);
        packet.addCcData(dataPacket2);

        CPPUNIT_ASSERT_EQUAL(false, packet.isFull());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), packet.getSize());

        // Verify new data
        const std::vector<std::uint8_t>& data = packet.getCcpData();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), data.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x01), data[0]);   // start data2
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xCC), data[1]);   // new data1
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xDD), data[2]);   // new data2
    }

    void testDifferentSequenceNumbers()
    {
        // Test creating packets with different sequence numbers
        for (uint8_t seqNo = 0; seqNo < 4; ++seqNo)
        {
            CaptionChannelPacket packet;
            CcData startData = createValidStartData(seqNo, 2);

            packet.addCcData(startData);

            CcData dataPacket = createValidDataData(0x50 + seqNo, 0x60 + seqNo);
            packet.addCcData(dataPacket);

            CPPUNIT_ASSERT_EQUAL(true, packet.isFull());
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());

            // Verify sequence-specific data
            const std::vector<std::uint8_t>& data = packet.getCcpData();
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x50 + seqNo), data[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x60 + seqNo), data[2]);
        }
    }

    void testMixedValidInvalidOperations()
    {
        CaptionChannelPacket packet;

        // Valid start
        CcData startData = createValidStartData(0, 3);
        packet.addCcData(startData);

        // Valid data addition
        CcData dataPacket = createValidDataData(0x11, 0x22);
        packet.addCcData(dataPacket);

        // Try invalid operations
        CcData invalidData = createInvalidData();
        CPPUNIT_ASSERT_THROW(packet.addCcData(invalidData), InvalidOperation);

        CcData anotherStart = createValidStartData(1, 4);
        CPPUNIT_ASSERT_THROW(packet.addCcData(anotherStart), InvalidOperation);

        // Verify packet is still valid after failed operations
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet.getCcpData().size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), packet.getSize());

        // Should still be able to add valid data
        CcData finalData = createValidDataData(0x33, 0x44);
        packet.addCcData(finalData);

        CPPUNIT_ASSERT_EQUAL(true, packet.isFull());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), packet.getCcpData().size());
    }
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(CcCaptionChannelPacketTest);