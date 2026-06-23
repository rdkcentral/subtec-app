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

#include "Collector.hpp"
#include "CollectorListener.hpp"
#include "PacketHeader.hpp"
#include "PacketRaw.hpp"
#include "PesPacketReader.hpp"
#include <ttxdecoder/ControlInfo.hpp>

using namespace ttxdecoder;

namespace
{

const std::uint8_t kEncodedHamming84Nibbles[16] = {
    0x28, 0x00, 0x12, 0x3A,
    0x06, 0x4E, 0x0C, 0x74,
    0x03, 0x63, 0x11, 0x59,
    0x05, 0x2D, 0x3F, 0x17
};

std::uint8_t reverseBits(std::uint8_t value)
{
    value = static_cast<std::uint8_t>(((value & 0xF0) >> 4) | ((value & 0x0F) << 4));
    value = static_cast<std::uint8_t>(((value & 0xCC) >> 2) | ((value & 0x33) << 2));
    value = static_cast<std::uint8_t>(((value & 0xAA) >> 1) | ((value & 0x55) << 1));
    return value;
}

std::uint8_t encodeParityByte(char value)
{
    std::uint8_t data = static_cast<std::uint8_t>(value) & 0x7F;
    std::uint8_t parity = 0;

    for (int bit = 0; bit < 7; ++bit)
    {
        parity ^= static_cast<std::uint8_t>((data >> bit) & 0x01);
    }

    if (parity == 0)
    {
        data |= 0x80;
    }

    return reverseBits(data);
}

} // namespace

// Mock CollectorListener for testing
class MockCollectorListener : public CollectorListener
{
public:
    MockCollectorListener()
        : m_callCount(0)
        , m_lastMagazineNumber(0xFF)
        , m_lastPacketAddress(0xFF)
        , m_lastDesignationCode(0)
    {
    }

    virtual ~MockCollectorListener() = default;

    virtual void onPacketReady(CollectorPacketContext& context) override
    {
        m_callCount++;
        m_lastMagazineNumber = context.getMagazineNumber();
        m_lastPacketAddress = context.getPacketAddress();
        m_lastDesignationCode = context.getDesignationCode();

        // Store context for further verification if needed
        m_contexts.push_back({
            context.getMagazineNumber(),
            context.getPacketAddress(),
            context.getDesignationCode()
        });
    }

    void reset()
    {
        m_callCount = 0;
        m_lastMagazineNumber = 0xFF;
        m_lastPacketAddress = 0xFF;
        m_lastDesignationCode = 0;
        m_contexts.clear();
    }

    int getCallCount() const { return m_callCount; }
    std::uint8_t getLastMagazineNumber() const { return m_lastMagazineNumber; }
    std::uint8_t getLastPacketAddress() const { return m_lastPacketAddress; }
    std::int8_t getLastDesignationCode() const { return m_lastDesignationCode; }

    struct ContextInfo {
        std::uint8_t magazineNumber;
        std::uint8_t packetAddress;
        std::int8_t designationCode;
    };

    const std::vector<ContextInfo>& getContexts() const { return m_contexts; }

private:
    int m_callCount;
    std::uint8_t m_lastMagazineNumber;
    std::uint8_t m_lastPacketAddress;
    std::int8_t m_lastDesignationCode;
    std::vector<ContextInfo> m_contexts;
};

class RawConsumeListener : public CollectorListener
{
public:
    RawConsumeListener()
        : m_callCount(0)
        , m_consumed(false)
    {
    }

    virtual void onPacketReady(CollectorPacketContext& context) override
    {
        m_callCount++;
        m_consumed = context.consume(m_packet);
        m_buffer.assign(m_packet.getBuffer(), m_packet.getBuffer() + m_packet.getBufferLength());
    }

    int getCallCount() const { return m_callCount; }
    bool isConsumed() const { return m_consumed; }
    const PacketRaw& getPacket() const { return m_packet; }
    const std::vector<std::uint8_t>& getBuffer() const { return m_buffer; }

private:
    int m_callCount;
    bool m_consumed;
    PacketRaw m_packet;
    std::vector<std::uint8_t> m_buffer;
};

class HeaderConsumeListener : public CollectorListener
{
public:
    HeaderConsumeListener()
        : m_callCount(0)
        , m_consumed(false)
    {
    }

    virtual void onPacketReady(CollectorPacketContext& context) override
    {
        m_callCount++;
        m_consumed = context.consume(m_packet);
        m_buffer.assign(m_packet.getBuffer(), m_packet.getBuffer() + m_packet.getBufferLength());
    }

    int getCallCount() const { return m_callCount; }
    bool isConsumed() const { return m_consumed; }
    const PacketHeader& getPacket() const { return m_packet; }
    const std::vector<std::int8_t>& getBuffer() const { return m_buffer; }

private:
    int m_callCount;
    bool m_consumed;
    PacketHeader m_packet;
    std::vector<std::int8_t> m_buffer;
};

class CollectorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( CollectorTest );
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testProcessEmptyPacketData);
    CPPUNIT_TEST(testProcessPacketDataWithUnsupportedUnitId);
    CPPUNIT_TEST(testProcessPacketDataWithInvalidFramingCode);
    CPPUNIT_TEST(testProcessPacketDataWithInvalidMagazineHamming);
    CPPUNIT_TEST(testProcessPacketDataWithInvalidPacketHamming);
    CPPUNIT_TEST(testProcessPacketDataWithValidTeletextPacket);
    CPPUNIT_TEST(testProcessPacketDataWithValidSubtitlePacket);
    CPPUNIT_TEST(testProcessPacketDataWithStrangeLengthStillProcessesPacket);
    CPPUNIT_TEST(testProcessPacketDataWithMultipleDataUnits);
    CPPUNIT_TEST(testProcessPacketDataMagazineZeroPreservedInContext);
    CPPUNIT_TEST(testProcessPacketDataAllMagazineNumbers);
    CPPUNIT_TEST(testProcessPacketDataWithDesignationCodeForPacket26);
    CPPUNIT_TEST(testProcessPacketDataWithDesignationCodeForPacket31);
    CPPUNIT_TEST(testConsumeRaw);
    CPPUNIT_TEST(testConsumeHeader);
    CPPUNIT_TEST(testMissingDataId);
    CPPUNIT_TEST(testShortUnitHeader);
    CPPUNIT_TEST(testShortPayload);
    CPPUNIT_TEST(testInvalidDesignationPacket26);
    CPPUNIT_TEST(testInvalidDesignationPacket31);
    CPPUNIT_TEST(testProcessPacketDataWithNoDesignationCodeForPacket0);
    CPPUNIT_TEST(testResetAfterProcessing);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_listener = new MockCollectorListener();
        m_collector = new Collector(*m_listener);
    }

    void tearDown() override
    {
        delete m_collector;
        delete m_listener;
    }

    void testConstructor()
    {
        // Constructor should not throw
        MockCollectorListener listener;
        Collector collector(listener);

        // Collector is created successfully - no exceptions
        CPPUNIT_ASSERT(true);
    }

    void testProcessEmptyPacketData()
    {
        // Create an empty reader (just data identifier, no data units)
        std::uint8_t data[] = { 0x10 }; // data identifier only
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should process without errors
        m_collector->processPacketData(reader);

        // Listener should not be called since there are no data units
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getCallCount());
    }

    void testProcessPacketDataWithUnsupportedUnitId()
    {
        // Create packet with unsupported unit ID
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x01,       // unsupported data unit ID (not 0x02 or 0x03)
            0x05,       // data unit length
            0x00, 0x00, 0x00, 0x00, 0x00 // dummy bytes to skip
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should skip the unsupported unit
        m_collector->processPacketData(reader);

        // Listener should not be called
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getCallCount());
    }

    void testProcessPacketDataWithInvalidFramingCode()
    {
        // Create packet with invalid framing code
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x2C,       // TELETEXT_PACKET_LENGTH (44 bytes)
            0xFF,       // data unit control
            0x00,       // invalid framing code (should be 0xE4)
            0x00,       // valid hamming (decodes to 1)
            0x28,       // valid hamming (decodes to 0)
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should reject due to invalid framing code
        m_collector->processPacketData(reader);

        // Listener should not be called
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getCallCount());
    }

    void testProcessPacketDataWithInvalidMagazineHamming()
    {
        // Create packet with invalid hamming encoding for magazine/packet
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x2C,       // TELETEXT_PACKET_LENGTH
            0xFF,       // data unit control
            0xE4,       // valid framing code
            0x01,       // invalid hamming byte 1 (0x01 decodes to 0xFF=-1)
            0x28,       // valid hamming byte 2 (decodes to 0)
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should reject due to invalid hamming
        m_collector->processPacketData(reader);

        // Listener should not be called
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getCallCount());
    }

    void testProcessPacketDataWithInvalidPacketHamming()
    {
        // Create packet with valid magazine but invalid packet address hamming
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x2C,       // TELETEXT_PACKET_LENGTH
            0xFF,       // data unit control
            0xE4,       // valid framing code
            0x00,       // valid hamming byte 1 (decodes to 1)
            0x01,       // invalid hamming byte 2 (0x01 decodes to 0xFF=-1)
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should reject due to invalid hamming
        m_collector->processPacketData(reader);

        // Listener should not be called
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getCallCount());
    }

    void testProcessPacketDataWithValidTeletextPacket()
    {
        // Create valid teletext packet for magazine 1, packet 0
        // Magazine 1: mpByte1 = 0x01 (bits 0-2), hamming encoded as 0x00
        // Packet 0: packet address = 0, hamming encoded as 0x28
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x2C,       // TELETEXT_PACKET_LENGTH (44 bytes)
            0xFF,       // data unit control
            0xE4,       // valid framing code
            0x00,       // hamming 8/4: 0x00 decodes to 0x01 (magazine 1)
            0x28,       // hamming 8/4: 0x28 decodes to 0x00 (packet address bits)
            // Remaining 40 bytes - packet data (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        m_collector->processPacketData(reader);

        // Listener should be called once
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), m_listener->getLastMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastPacketAddress());
    }

    void testProcessPacketDataWithValidSubtitlePacket()
    {
        // Create valid subtitle packet (unit ID 0x03)
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x03,       // SUBTITLES_UNIT_ID
            0x2C,       // TELETEXT_PACKET_LENGTH
            0xFF,       // data unit control
            0xE4,       // valid framing code
            0x00,       // hamming 8/4: 0x00 decodes to 0x01 (magazine 1)
            0x28,       // hamming 8/4: 0x28 decodes to 0x00 (packet address)
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        m_collector->processPacketData(reader);

        // Listener should be called once
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), m_listener->getLastMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastPacketAddress());
    }

    void testProcessPacketDataWithStrangeLengthStillProcessesPacket()
    {
        // Create packet with non-standard length; processing should continue.
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x10,       // Non-standard length (16 decimal instead of 44)
            0xFF,       // data unit control
            0xE4,       // valid framing code
            0x00,       // hamming byte 1 (decodes to 1)
            0x28,       // hamming byte 2 (decodes to 0)
            // Remaining 12 bytes (1+1+1+1+12=16 total for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        // Should process despite strange length
        m_collector->processPacketData(reader);

        // Listener should be called (warning logged, but processing continues)
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
    }

    void testProcessPacketDataWithMultipleDataUnits()
    {
        // Create packet with multiple data units
        std::vector<std::uint8_t> data;
        data.push_back(0x10); // data identifier

        // First data unit - magazine 1, packet 0 (44 bytes content)
        data.push_back(0x02); // TELETEXT_UNIT_ID
        data.push_back(0x2C); // length (44)
        data.push_back(0xFF); // control
        data.push_back(0xE4); // framing
        data.push_back(0x00); // mag 1 (0x00 decodes to 0x01)
        data.push_back(0x28); // packet 0 (0x28 decodes to 0x00)
        for (int i = 0; i < 40; i++) data.push_back(0x00); // 40 more bytes (total 44)

        // Second data unit - magazine 2, packet 1 (44 bytes content)
        data.push_back(0x02); // TELETEXT_UNIT_ID
        data.push_back(0x2C); // length (44)
        data.push_back(0xFF); // control
        data.push_back(0xE4); // framing
        data.push_back(kEncodedHamming84Nibbles[10]); // mag 2 with packet bit 0 set
        data.push_back(kEncodedHamming84Nibbles[0]);  // packet address bits 1-4 cleared
        for (int i = 0; i < 40; i++) data.push_back(0x00); // 40 more bytes (total 44)

        PesPacketReader reader(data.data(), data.size(), nullptr, 0);

        m_collector->processPacketData(reader);

        // Listener should be called twice
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getCallCount());

        // Verify both contexts were processed
        const auto& contexts = m_listener->getContexts();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), contexts.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), contexts[0].magazineNumber);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), contexts[0].packetAddress);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(2), contexts[1].magazineNumber);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), contexts[1].packetAddress);
    }

    void testProcessPacketDataMagazineZeroPreservedInContext()
    {
        // Magazine 0 is forwarded as 0 in the collector context.
        std::uint8_t data[] = {
            0x10,       // data identifier
            0x02,       // TELETEXT_UNIT_ID
            0x2C,       // length
            0xFF,       // control
            0xE4,       // framing
            0x28,       // hamming: 0x28 decodes to 0x00 (bits 0-2 = 000, magazine 0)
            0x28,       // hamming: packet address 0
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        // Magazine 0 is passed as 0 to the listener (mapping happens in header processing)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastMagazineNumber());
    }

    void testProcessPacketDataAllMagazineNumbers()
    {
        // Test all valid magazine numbers (0-7)
        for (std::uint8_t mag = 0; mag < 8; mag++)
        {
            m_listener->reset();

            std::uint8_t data[] = {
                0x10,       // data identifier
                0x02,       // TELETEXT_UNIT_ID
                0x2C,       // length
                0xFF,       // control
                0xE4,       // framing
                kEncodedHamming84Nibbles[mag],
                0x28,       // packet address (0x28 decodes to 0)
                // Remaining 40 bytes (total 44 for data unit)
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            PesPacketReader reader(data, sizeof(data), nullptr, 0);

            m_collector->processPacketData(reader);

            CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
            CPPUNIT_ASSERT_EQUAL(mag, m_listener->getLastMagazineNumber());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastPacketAddress());
        }
    }

    void testProcessPacketDataWithDesignationCodeForPacket26()
    {
        // Packet 26 should extract a designation code from the first payload byte.
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[1],  // magazine 1, packet bit 0 cleared
            kEncodedHamming84Nibbles[13], // packet address bits 1-4 => packet 26
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), m_listener->getLastMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(26), m_listener->getLastPacketAddress());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(1), m_listener->getLastDesignationCode());
    }

    void testProcessPacketDataWithDesignationCodeForPacket31()
    {
        // Packet 31 should also extract a designation code from the first payload byte.
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[9],  // magazine 1, packet bit 0 set
            kEncodedHamming84Nibbles[15], // packet address bits 1-4 => packet 31
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), m_listener->getLastMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(31), m_listener->getLastPacketAddress());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(1), m_listener->getLastDesignationCode());
    }

    void testConsumeRaw()
    {
        RawConsumeListener listener;
        Collector collector(listener);

        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[11],
            kEncodedHamming84Nibbles[2],
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
            0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        collector.processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, listener.getCallCount());
        CPPUNIT_ASSERT(listener.isConsumed());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(3), listener.getPacket().getMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(5), listener.getPacket().getPacketAddress());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(40), listener.getBuffer().size());

        for (std::size_t i = 0; i < listener.getBuffer().size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x10 + i), listener.getBuffer()[i]);
        }
    }

    void testConsumeHeader()
    {
        HeaderConsumeListener listener;
        Collector collector(listener);

        const char readable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
        std::vector<std::uint8_t> data = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[1],
            kEncodedHamming84Nibbles[0],
            kEncodedHamming84Nibbles[3],
            kEncodedHamming84Nibbles[2],
            kEncodedHamming84Nibbles[5],
            kEncodedHamming84Nibbles[8],
            kEncodedHamming84Nibbles[0],
            kEncodedHamming84Nibbles[12],
            kEncodedHamming84Nibbles[0],
            kEncodedHamming84Nibbles[11]
        };

        for (std::size_t i = 0; i < 32; ++i)
        {
            data.push_back(encodeParityByte(readable[i]));
        }

        PesPacketReader reader(data.data(), data.size(), nullptr, 0);
        collector.processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, listener.getCallCount());
        CPPUNIT_ASSERT(listener.isConsumed());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), listener.getPacket().getMagazineNumber());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), listener.getPacket().getPacketAddress());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x123), listener.getPacket().getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0005), listener.getPacket().getPageId().getSubpage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(ControlInfo::ERASE_PAGE | ControlInfo::NEWSFLASH | ControlInfo::SUBTITLE | ControlInfo::MAGAZINE_SERIAL), listener.getPacket().getControlInfo());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(5), listener.getPacket().getNationalOption());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(32), listener.getBuffer().size());

        for (std::size_t i = 0; i < listener.getBuffer().size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(readable[i]), listener.getBuffer()[i]);
        }
    }

    void testMissingDataId()
    {
        PesPacketReader reader(nullptr, 0, nullptr, 0);

        CPPUNIT_ASSERT_THROW(m_collector->processPacketData(reader), PesPacketReader::Exception);
    }

    void testShortUnitHeader()
    {
        std::uint8_t data[] = { 0x10, 0x02 };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        CPPUNIT_ASSERT_THROW(m_collector->processPacketData(reader), PesPacketReader::Exception);
    }

    void testShortPayload()
    {
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C,
            0xFF, 0xE4, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);

        CPPUNIT_ASSERT_THROW(m_collector->processPacketData(reader), PesPacketReader::Exception);
    }

    void testInvalidDesignationPacket26()
    {
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[1],
            kEncodedHamming84Nibbles[13],
            0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(26), m_listener->getLastPacketAddress());
        CPPUNIT_ASSERT(m_listener->getLastDesignationCode() < 0);
    }

    void testInvalidDesignationPacket31()
    {
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4,
            kEncodedHamming84Nibbles[9],
            kEncodedHamming84Nibbles[15],
            0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(31), m_listener->getLastPacketAddress());
        CPPUNIT_ASSERT(m_listener->getLastDesignationCode() < 0);
    }

    void testProcessPacketDataWithNoDesignationCodeForPacket0()
    {
        // Packet 0 should not extract designation code
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4, 0x00, 0x28,
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
        // Designation code should be 0 for non-26-31 packets
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(0), m_listener->getLastDesignationCode());
    }

    void testResetAfterProcessing()
    {
        // Process some data
        std::uint8_t data[] = {
            0x10, 0x02, 0x2C, 0xFF, 0xE4, 0x00, 0x28,
            // Remaining 40 bytes (total 44 for data unit)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        PesPacketReader reader(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());

        // Reset
        m_collector->reset();

        // Reset should not affect listener state (it's external)
        // But collector should be ready for new processing
        m_listener->reset();

        // Process again
        PesPacketReader reader2(data, sizeof(data), nullptr, 0);
        m_collector->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getCallCount());
    }

private:
    Collector* m_collector;
    MockCollectorListener* m_listener;
};

CPPUNIT_TEST_SUITE_REGISTRATION( CollectorTest );
