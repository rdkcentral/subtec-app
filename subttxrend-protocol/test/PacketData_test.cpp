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

#include "Packet.hpp"
#include "PacketData.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketData;

class PacketDataTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketDataTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testTooShort);
    CPPUNIT_TEST(testTooLong);
    CPPUNIT_TEST(testWEBVTTDataPacketGood);
    CPPUNIT_TEST(testConstructorDifferentTypes);
    CPPUNIT_TEST(testGetChannelTypeBasic);
    CPPUNIT_TEST(testGetDisplayOffsetBasic);
    CPPUNIT_TEST(testGetDataBasic);
    CPPUNIT_TEST(testGetDataSizeBasic);
    CPPUNIT_TEST(testCCDataPacketTooShort);
    CPPUNIT_TEST(testCCDataPacketMissingPTSFields);
    CPPUNIT_TEST(testTTMLDataPacketTooShort);
    CPPUNIT_TEST(testWEBVTTDataPacketTooShort);
    CPPUNIT_TEST(testDisplayOffsetBoundaryValues);
    CPPUNIT_TEST(testChannelBoundaryValues);
    CPPUNIT_TEST(testMinimalDataSection);
    CPPUNIT_TEST(testZeroChannelValues);
    CPPUNIT_TEST(testMaxChannelValues);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testInitialStateBeforeParsing);
    CPPUNIT_TEST(testStateAfterFailedParsing);
    CPPUNIT_TEST(testLargeDataPayload);
    CPPUNIT_TEST(testMinimumValidPacketSizes);
    CPPUNIT_TEST(testNegativeDisplayOffset);
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

    void testGood()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x01, 0x02, 0x03        // PES data
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 8 + 3);
        CPPUNIT_ASSERT(packet.getChannelId() == 5);
        CPPUNIT_ASSERT(packet.getChannelType() == 1);
        CPPUNIT_ASSERT(packet.getDataSize() == 3);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(packet.getData()[0]) == 0x01);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(packet.getData()[1]) == 0x02);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(packet.getData()[2]) == 0x03);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
            0xF1, 0x00, 0x00, 0x00, // type (invalid)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x01, 0x02, 0x03        // PES data
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00,       // (not enough bytes)
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x01, 0x02, 0x03,       // PES data
            0x00, 0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test WEBVTT_DATA packet with display offset
    void testWEBVTTDataPacketGood()
    {
        std::uint8_t packetData[] =
        {
            0x10, 0x00, 0x00, 0x00, // type (WEBVTT_DATA = 16)
            0x01, 0x23, 0x45, 0x67, // counter
            0x11, 0x00, 0x00, 0x00, // size (17 bytes: 4 channel_id + 8 display_offset + 5 data)
            0x07, 0x00, 0x00, 0x00, // channel id
            0x80, 0x84, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, // display offset (2000000 ms)
            0x57, 0x45, 0x42, 0x56, 0x54 // WEBVTT data
        };

        PacketData packet{Packet::Type::WEBVTT_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 17);
        CPPUNIT_ASSERT(packet.getChannelId() == 7);
        CPPUNIT_ASSERT(packet.getDataSize() == 5);
        CPPUNIT_ASSERT(packet.getData()[0] == 0x57);
        CPPUNIT_ASSERT(packet.getData()[1] == 0x45);
        CPPUNIT_ASSERT(packet.getData()[2] == 0x42);
        CPPUNIT_ASSERT(packet.getData()[3] == 0x56);
        CPPUNIT_ASSERT(packet.getData()[4] == 0x54);
        CPPUNIT_ASSERT(packet.getDisplayOffset() == 2000000);
    }

    // Test constructor with different packet types
    void testConstructorDifferentTypes()
    {
        PacketData pesPacket{Packet::Type::PES_DATA};
        CPPUNIT_ASSERT(pesPacket.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(!pesPacket.isValid()); // Should be invalid before parsing

        PacketData ccPacket{Packet::Type::CC_DATA};
        CPPUNIT_ASSERT(ccPacket.getType() == Packet::Type::CC_DATA);
        CPPUNIT_ASSERT(!ccPacket.isValid()); // Should be invalid before parsing

        PacketData ttmlPacket{Packet::Type::TTML_DATA};
        CPPUNIT_ASSERT(ttmlPacket.getType() == Packet::Type::TTML_DATA);
        CPPUNIT_ASSERT(!ttmlPacket.isValid()); // Should be invalid before parsing

        PacketData webvttPacket{Packet::Type::WEBVTT_DATA};
        CPPUNIT_ASSERT(webvttPacket.getType() == Packet::Type::WEBVTT_DATA);
        CPPUNIT_ASSERT(!webvttPacket.isValid()); // Should be invalid before parsing
    }

    // Test getChannelType basic functionality
    void testGetChannelTypeBasic()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type (PES_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x99, 0x88, 0x77, 0x66, // channel type
            0x01, 0x02, 0x03        // data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelType() == 0x66778899); // Little endian
    }

    // Test getDisplayOffset basic functionality
    void testGetDisplayOffsetBasic()
    {
        std::uint8_t packetData[] =
        {
            0x08, 0x00, 0x00, 0x00, // type (TTML_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size (12 bytes: 4 channel_id + 8 display_offset)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x39, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // display offset (12345 ms)
        };

        PacketData packet{Packet::Type::TTML_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getDisplayOffset() == 12345);
    }

    // Test getData basic functionality
    void testGetDataBasic()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0D, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0xDE, 0xAD, 0xBE, 0xEF, 0xCA // test data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        const char* data = packet.getData();
        CPPUNIT_ASSERT(data != nullptr);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[0]) == 0xDE);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[1]) == 0xAD);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[2]) == 0xBE);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[3]) == 0xEF);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[4]) == 0xCA);
    }

    // Test getDataSize basic functionality
    void testGetDataSizeBasic()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0F, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 // 7 bytes of data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getDataSize() == 7);
    }

    // Test CC_DATA packet that's too short (missing PTS fields)
    void testCCDataPacketTooShort()
    {
        std::uint8_t packetData[] =
        {
            0x0A, 0x00, 0x00, 0x00, // type (CC_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size (12 bytes - missing PTS fields)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00  // channel type (incomplete - missing PTS fields)
        };

        PacketData packet{Packet::Type::CC_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test CC_DATA packet missing full PTS fields
    void testCCDataPacketMissingPTSFields()
    {
        std::uint8_t packetData[] =
        {
            0x0A, 0x00, 0x00, 0x00, // type (CC_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size (16 bytes - still missing one PTS field)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x00, 0x00, 0x00, 0x00  // PTS presence type (missing PTS type field)
        };

        PacketData packet{Packet::Type::CC_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test TTML_DATA packet that's too short (missing display offset)
    void testTTMLDataPacketTooShort()
    {
        std::uint8_t packetData[] =
        {
            0x08, 0x00, 0x00, 0x00, // type (TTML_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x40, 0x42, 0x0F, 0x00  // display offset (incomplete - only 4 bytes)
        };

        PacketData packet{Packet::Type::TTML_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test WEBVTT_DATA packet that's too short (missing display offset)
    void testWEBVTTDataPacketTooShort()
    {
        std::uint8_t packetData[] =
        {
            0x10, 0x00, 0x00, 0x00, // type (WEBVTT_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0E, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x40, 0x42, 0x0F, 0x00, 0x00, 0x00  // display offset (incomplete - only 6 bytes)
        };

        PacketData packet{Packet::Type::WEBVTT_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test display offset boundary values
    void testDisplayOffsetBoundaryValues()
    {
        // Test maximum positive value
        std::uint8_t maxPacketData[] =
        {
            0x08, 0x00, 0x00, 0x00, // type (TTML_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size (12 bytes: 4 channel_id + 8 display_offset)
            0x05, 0x00, 0x00, 0x00, // channel id
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, // max int64_t
        };

        PacketData maxPacket{Packet::Type::TTML_DATA};
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxPacketData), std::end(maxPacketData));

        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.getDisplayOffset() == 9223372036854775807LL);

        // Test minimum negative value
        std::uint8_t minPacketData[] =
        {
            0x10, 0x00, 0x00, 0x00, // type (WEBVTT_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size (12 bytes: 4 channel_id + 8 display_offset)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, // min int64_t
        };

        PacketData minPacket{Packet::Type::WEBVTT_DATA};
        DataBufferPtr minBuffer = std::make_unique<DataBuffer>(std::begin(minPacketData), std::end(minPacketData));

        CPPUNIT_ASSERT(minPacket.parse(std::move(minBuffer)));
        CPPUNIT_ASSERT(minPacket.getDisplayOffset() == std::numeric_limits<std::int64_t>::min());
    }

    // Test channel boundary values
    void testChannelBoundaryValues()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // channel id (max uint32_t)
            0xFF, 0xFF, 0xFF, 0xFF, // channel type (max uint32_t)
            0x01, 0x02, 0x03        // data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0xFFFFFFFF);
        CPPUNIT_ASSERT(packet.getChannelType() == 0xFFFFFFFF);
    }

    // Test minimal data section (single byte)
    void testMinimalDataSection()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x09, 0x00, 0x00, 0x00, // size (9 bytes - channel_id + channel_type + 1 data byte)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x42                    // Single data byte
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getChannelId() == 5);
        CPPUNIT_ASSERT(packet.getChannelType() == 1);
        CPPUNIT_ASSERT(packet.getDataSize() == 1);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(packet.getData()[0]) == 0x42);
    }

    // Test zero channel values
    void testZeroChannelValues()
    {
        std::uint8_t packetData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0A, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, // channel id (0)
            0x00, 0x00, 0x00, 0x00, // channel type (0)
            0x42, 0x43            // data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getChannelType() == 0);
        CPPUNIT_ASSERT(packet.getDataSize() == 2);
    }

    // Test maximum channel values
    void testMaxChannelValues()
    {
        std::uint8_t packetData[] =
        {
            0x0A, 0x00, 0x00, 0x00, // type (CC_DATA)
            0xFF, 0xFF, 0xFF, 0xFF, // counter (max)
            0x13, 0x00, 0x00, 0x00, // size (19 bytes: 4 channel_id + 4 channel_type + 4 PTS_presence + 4 PTS_type + 3 data)
            0xFF, 0xFF, 0xFF, 0xFF, // channel id (max)
            0xFF, 0xFF, 0xFF, 0xFF, // channel type (max)
            0x00, 0x00, 0x00, 0x00, // PTS presence type
            0x00, 0x00, 0x00, 0x00, // PTS type
            0xAA, 0xBB, 0xCC        // data
        };

        PacketData packet{Packet::Type::CC_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0xFFFFFFFF);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xFFFFFFFF);
        CPPUNIT_ASSERT(packet.getChannelType() == 0xFFFFFFFF);
    }

    // Test counter boundary values
    void testCounterBoundaryValues()
    {
        // Test zero counter
        std::uint8_t zeroPacketData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter (0)
            0x09, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x42                    // data
        };

        PacketData zeroPacket{Packet::Type::PES_DATA};
        DataBufferPtr zeroBuffer = std::make_unique<DataBuffer>(std::begin(zeroPacketData), std::end(zeroPacketData));

        CPPUNIT_ASSERT(zeroPacket.parse(std::move(zeroBuffer)));
        CPPUNIT_ASSERT(zeroPacket.getCounter() == 0);
    }

    // Test initial state before parsing
    void testInitialStateBeforeParsing()
    {
        PacketData packet{Packet::Type::PES_DATA};
        
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet.getDisplayOffset() == 0); // Default value
    }

    // Test state after failed parsing
    void testStateAfterFailedParsing()
    {
        std::uint8_t invalidPacketData[] =
        {
            0xF1, 0x00, 0x00, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x01, 0x02, 0x03        // data
        };

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(invalidPacketData), std::end(invalidPacketData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA); // Constructor type preserved
    }

    // Test large data payload
    void testLargeDataPayload()
    {
        // Create packet with 1000 bytes of data
        std::vector<std::uint8_t> packetData;
        
        // Header
        packetData.insert(packetData.end(), {0x01, 0x00, 0x00, 0x00}); // type
        packetData.insert(packetData.end(), {0x01, 0x23, 0x45, 0x67}); // counter
        
        // Size (4 channel_id + 4 channel_type + 1000 data = 1008)
        std::uint32_t size = 1008;
        packetData.push_back(size & 0xFF);
        packetData.push_back((size >> 8) & 0xFF);
        packetData.push_back((size >> 16) & 0xFF);
        packetData.push_back((size >> 24) & 0xFF);
        
        packetData.insert(packetData.end(), {0x05, 0x00, 0x00, 0x00}); // channel id
        packetData.insert(packetData.end(), {0x01, 0x00, 0x00, 0x00}); // channel type
        
        // 1000 bytes of test data
        for (int i = 0; i < 1000; ++i) {
            packetData.push_back(static_cast<std::uint8_t>(i % 256));
        }

        PacketData packet{Packet::Type::PES_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getDataSize() == 1000);
        
        // Verify first and last bytes of data
        const char* data = packet.getData();
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[0]) == 0);
        CPPUNIT_ASSERT(static_cast<std::uint8_t>(data[999]) == 231); // 999 % 256 = 231
    }

    // Test minimum valid packet sizes for each type
    void testMinimumValidPacketSizes()
    {
        // PES_DATA minimum: channel_id(4) + channel_type(4) + minimal_data(1) = 9 bytes
        std::uint8_t pesMinData[] =
        {
            0x01, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x09, 0x00, 0x00, 0x00, // size (9 bytes - channel_id + channel_type + 1 data byte)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x01, 0x00, 0x00, 0x00, // channel type
            0x42                    // minimal data
        };

        PacketData pesPacket{Packet::Type::PES_DATA};
        DataBufferPtr pesBuffer = std::make_unique<DataBuffer>(std::begin(pesMinData), std::end(pesMinData));

        CPPUNIT_ASSERT(pesPacket.parse(std::move(pesBuffer)));
        CPPUNIT_ASSERT(pesPacket.isValid());
        CPPUNIT_ASSERT(pesPacket.getDataSize() == 1);

        // TTML_DATA minimum: channel_id(4) + display_offset(8) + minimal_data(1) = 13 bytes
        std::uint8_t ttmlMinData[] =
        {
            0x08, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0D, 0x00, 0x00, 0x00, // size (13 bytes - channel_id + display_offset + 1 data byte)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // display offset
            0x43                    // minimal data
        };

        PacketData ttmlPacket{Packet::Type::TTML_DATA};
        DataBufferPtr ttmlBuffer = std::make_unique<DataBuffer>(std::begin(ttmlMinData), std::end(ttmlMinData));

        CPPUNIT_ASSERT(ttmlPacket.parse(std::move(ttmlBuffer)));
        CPPUNIT_ASSERT(ttmlPacket.isValid());
        CPPUNIT_ASSERT(ttmlPacket.getDataSize() == 1);
        CPPUNIT_ASSERT(ttmlPacket.getDisplayOffset() == 0);
    }

    // Test negative display offset
    void testNegativeDisplayOffset()
    {
        std::uint8_t packetData[] =
        {
            0x08, 0x00, 0x00, 0x00, // type (TTML_DATA)
            0x01, 0x23, 0x45, 0x67, // counter
            0x0F, 0x00, 0x00, 0x00, // size (15 bytes - channel_id + display_offset + 3 data bytes)
            0x05, 0x00, 0x00, 0x00, // channel id
            0x90, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // display offset (-6000 ms)
            0x44, 0x55, 0x66        // data
        };

        PacketData packet{Packet::Type::TTML_DATA};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getDisplayOffset() == -6000);
        CPPUNIT_ASSERT(packet.getDataSize() == 3);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketDataTest );
