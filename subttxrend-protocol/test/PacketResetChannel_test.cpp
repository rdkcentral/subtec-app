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

#include "PacketResetChannel.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::BufferReader;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketResetChannel;

class PacketResetChannelTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketResetChannelTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadSize);
    CPPUNIT_TEST(testTooShort);
    CPPUNIT_TEST(testTooLong);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testGetTypeBasic);
    CPPUNIT_TEST(testGetCounterBasic);
    CPPUNIT_TEST(testGetSizeBasic);
    CPPUNIT_TEST(testGetChannelIdBasic);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testParseMethodBasic);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testCounterEndianness);
    CPPUNIT_TEST(testChannelIdBoundaryValues);
    CPPUNIT_TEST(testChannelIdEndianness);
    CPPUNIT_TEST(testSizeEdgeCases);
    CPPUNIT_TEST(testTypeAlternativeValues);
    CPPUNIT_TEST(testTypeInvalidValues);
    CPPUNIT_TEST(testTruncatedPacketVariations);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testMultipleParseCalls);
    CPPUNIT_TEST(testParseAfterFailedParse);
    CPPUNIT_TEST(testSequentialFailures);
    CPPUNIT_TEST(testCorruptedHeaderFields);
    CPPUNIT_TEST(testExactSizeMatching);
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
                0x04, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketResetChannel packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 4);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x04030201);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF4, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketResetChannel packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x04, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x05, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x00
        };

        PacketResetChannel packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x04, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02,  // channel id (cut)
        };

        PacketResetChannel packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x04, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x00, 0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketResetChannel packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test constructor and initial state
    void testConstructorInitialState()
    {
        PacketResetChannel packet;
        
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL); // Type should be set by constructor
    }

    // Test getType method
    void testGetTypeBasic()
    {
        PacketResetChannel packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        
        // Type should remain consistent after successful parsing
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
    }

    // Test getCounter method
    void testGetCounterBasic()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter (little endian)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412); // Little endian conversion
    }

    // Test getSize method
    void testGetSizeBasic()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size (must be 4 for reset channel)
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 4);
    }

    // Test getChannelId method
    void testGetChannelIdBasic()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x12, 0x34, 0x56, 0x78, // channel id (little endian)
        };
        
        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0x78563412); // Little endian conversion
    }

    // Test isValid method
    void testIsValidBasic()
    {
        PacketResetChannel packet;
        
        // Should be invalid initially
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Should be valid after successful parse
        std::uint8_t validData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04,
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    // Test parse method return values
    void testParseMethodBasic()
    {
        PacketResetChannel packet;
        
        // Test successful parse returns true
        std::uint8_t validData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF,
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        
        // Test failed parse returns false
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // invalid type
            0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04,
        };
        
        PacketResetChannel packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet2.parse(std::move(buffer2)) == false);
    }

    // L2 - Edge cases and error handling

    // Test counter boundary values
    void testCounterBoundaryValues()
    {
        // Test maximum uint32_t counter value
        std::uint8_t maxCounterData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter (max uint32_t)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxCounterData), std::end(maxCounterData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getCounter() == 0xFFFFFFFF);

        // Test zero counter value
        std::uint8_t zeroCounterData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter (0)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel zeroPacket;
        DataBufferPtr zeroBuffer = std::make_unique<DataBuffer>(std::begin(zeroCounterData), std::end(zeroCounterData));
        
        CPPUNIT_ASSERT(zeroPacket.parse(std::move(zeroBuffer)));
        CPPUNIT_ASSERT(zeroPacket.isValid());
        CPPUNIT_ASSERT(zeroPacket.getCounter() == 0);

        // Test mid-range counter value
        std::uint8_t midCounterData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x80, 0x7F, // counter (0x7F800000)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel midPacket;
        DataBufferPtr midBuffer = std::make_unique<DataBuffer>(std::begin(midCounterData), std::end(midCounterData));
        
        CPPUNIT_ASSERT(midPacket.parse(std::move(midBuffer)));
        CPPUNIT_ASSERT(midPacket.isValid());
        CPPUNIT_ASSERT(midPacket.getCounter() == 0x7F800000);
    }

    // Test counter endianness patterns
    void testCounterEndianness()
    {
        // Test specific little-endian counter value
        std::uint8_t endiannessData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (little endian: 0x04030201)
            0x04, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
        };
        
        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x04030201);
    }

    // Test channel ID boundary values
    void testChannelIdBoundaryValues()
    {
        // Test maximum uint32_t channel ID
        std::uint8_t maxChannelData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // channel id (max uint32_t)
        };
        
        PacketResetChannel maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxChannelData), std::end(maxChannelData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getChannelId() == 0xFFFFFFFF);

        // Test zero channel ID
        std::uint8_t zeroChannelData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, // channel id (0)
        };
        
        PacketResetChannel zeroPacket;
        DataBufferPtr zeroBuffer = std::make_unique<DataBuffer>(std::begin(zeroChannelData), std::end(zeroChannelData));
        
        CPPUNIT_ASSERT(zeroPacket.parse(std::move(zeroBuffer)));
        CPPUNIT_ASSERT(zeroPacket.isValid());
        CPPUNIT_ASSERT(zeroPacket.getChannelId() == 0);
    }

    // Test channel ID endianness patterns
    void testChannelIdEndianness()
    {
        // Test specific little-endian channel ID value
        std::uint8_t endiannessData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x11, 0x22, 0x33, 0x44, // channel id (little endian: 0x44332211)
        };
        
        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getChannelId() == 0x44332211);
    }

    // Test size field edge cases
    void testSizeEdgeCases()
    {
        // Test with size = 0 (should fail)
        std::uint8_t size0Data[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };
        
        PacketResetChannel size0Packet;
        DataBufferPtr size0Buffer = std::make_unique<DataBuffer>(std::begin(size0Data), std::end(size0Data));
        
        CPPUNIT_ASSERT(!size0Packet.parse(std::move(size0Buffer)));
        CPPUNIT_ASSERT(!size0Packet.isValid());

        // Test with size = 3 (should fail)
        std::uint8_t size3Data[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x03, 0x00, 0x00, 0x00, // size (3)
            0x01, 0x02, 0x03,       // partial channel id
        };
        
        PacketResetChannel size3Packet;
        DataBufferPtr size3Buffer = std::make_unique<DataBuffer>(std::begin(size3Data), std::end(size3Data));
        
        CPPUNIT_ASSERT(!size3Packet.parse(std::move(size3Buffer)));
        CPPUNIT_ASSERT(!size3Packet.isValid());

        // Test with large size value (should fail)
        std::uint8_t largeSizeData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0xFF, 0xFF, 0xFF, 0x7F, // size (large value)
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel largeSizePacket;
        DataBufferPtr largeSizeBuffer = std::make_unique<DataBuffer>(std::begin(largeSizeData), std::end(largeSizeData));
        
        CPPUNIT_ASSERT(!largeSizePacket.parse(std::move(largeSizeBuffer)));
        CPPUNIT_ASSERT(!largeSizePacket.isValid());
    }

    // Test alternative packet type values
    void testTypeAlternativeValues()
    {
        // Test with RESET_ALL type (should fail)
        std::uint8_t resetAllData[] = {
            0x03, 0x00, 0x00, 0x00, // type (RESET_ALL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel resetAllPacket;
        DataBufferPtr resetAllBuffer = std::make_unique<DataBuffer>(std::begin(resetAllData), std::end(resetAllData));
        
        CPPUNIT_ASSERT(!resetAllPacket.parse(std::move(resetAllBuffer)));
        CPPUNIT_ASSERT(!resetAllPacket.isValid());

        // Test with PES_DATA type (should fail)
        std::uint8_t pesData[] = {
            0x01, 0x00, 0x00, 0x00, // type (PES_DATA)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel pesPacket;
        DataBufferPtr pesBuffer = std::make_unique<DataBuffer>(std::begin(pesData), std::end(pesData));
        
        CPPUNIT_ASSERT(!pesPacket.parse(std::move(pesBuffer)));
        CPPUNIT_ASSERT(!pesPacket.isValid());

        // Test with TIMESTAMP type (should fail)
        std::uint8_t timestampData[] = {
            0x02, 0x00, 0x00, 0x00, // type (TIMESTAMP)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel timestampPacket;
        DataBufferPtr timestampBuffer = std::make_unique<DataBuffer>(std::begin(timestampData), std::end(timestampData));
        
        CPPUNIT_ASSERT(!timestampPacket.parse(std::move(timestampBuffer)));
        CPPUNIT_ASSERT(!timestampPacket.isValid());
    }

    // Test invalid type values
    void testTypeInvalidValues()
    {
        // Test with completely invalid type (0)
        std::uint8_t type0Data[] = {
            0x00, 0x00, 0x00, 0x00, // type (0)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel type0Packet;
        DataBufferPtr type0Buffer = std::make_unique<DataBuffer>(std::begin(type0Data), std::end(type0Data));
        
        CPPUNIT_ASSERT(!type0Packet.parse(std::move(type0Buffer)));
        CPPUNIT_ASSERT(!type0Packet.isValid());

        // Test with INVALID type value
        std::uint8_t invalidTypeData[] = {
            0xFF, 0xFF, 0xFF, 0xFF, // type (INVALID)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel invalidPacket;
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidTypeData), std::end(invalidTypeData));
        
        CPPUNIT_ASSERT(!invalidPacket.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!invalidPacket.isValid());

        // Test with random type value
        std::uint8_t randomTypeData[] = {
            0x99, 0x88, 0x77, 0x66, // type (random)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel randomPacket;
        DataBufferPtr randomBuffer = std::make_unique<DataBuffer>(std::begin(randomTypeData), std::end(randomTypeData));
        
        CPPUNIT_ASSERT(!randomPacket.parse(std::move(randomBuffer)));
        CPPUNIT_ASSERT(!randomPacket.isValid());
    }

    // Test various truncated packet scenarios
    void testTruncatedPacketVariations()
    {
        // Test with missing type field (empty buffer handled in testEmptyBuffer)
        std::uint8_t partialTypeData[] = {
            0x04, 0x00, // partial type
        };
        
        PacketResetChannel partialTypePacket;
        DataBufferPtr partialTypeBuffer = std::make_unique<DataBuffer>(std::begin(partialTypeData), std::end(partialTypeData));
        
        CPPUNIT_ASSERT(!partialTypePacket.parse(std::move(partialTypeBuffer)));
        CPPUNIT_ASSERT(!partialTypePacket.isValid());

        // Test with missing counter field
        std::uint8_t missingCounterData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, // partial counter
        };
        
        PacketResetChannel missingCounterPacket;
        DataBufferPtr missingCounterBuffer = std::make_unique<DataBuffer>(std::begin(missingCounterData), std::end(missingCounterData));
        
        CPPUNIT_ASSERT(!missingCounterPacket.parse(std::move(missingCounterBuffer)));
        CPPUNIT_ASSERT(!missingCounterPacket.isValid());

        // Test with missing size field
        std::uint8_t missingSizeData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, // partial size
        };
        
        PacketResetChannel missingSizePacket;
        DataBufferPtr missingSizeBuffer = std::make_unique<DataBuffer>(std::begin(missingSizeData), std::end(missingSizeData));
        
        CPPUNIT_ASSERT(!missingSizePacket.parse(std::move(missingSizeBuffer)));
        CPPUNIT_ASSERT(!missingSizePacket.isValid());

        // Test with partial channel ID (1 byte)
        std::uint8_t partial1ChannelData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, // partial channel id (1 byte)
        };
        
        PacketResetChannel partial1ChannelPacket;
        DataBufferPtr partial1ChannelBuffer = std::make_unique<DataBuffer>(std::begin(partial1ChannelData), std::end(partial1ChannelData));
        
        CPPUNIT_ASSERT(!partial1ChannelPacket.parse(std::move(partial1ChannelBuffer)));
        CPPUNIT_ASSERT(!partial1ChannelPacket.isValid());
    }

    // Test empty buffer
    void testEmptyBuffer()
    {
        PacketResetChannel emptyPacket;
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!emptyPacket.parse(std::move(emptyBuffer)));
        CPPUNIT_ASSERT(!emptyPacket.isValid());
    }

    // Test multiple parse calls on same packet
    void testMultipleParseCalls()
    {
        std::uint8_t validData1[] = {
            0x04, 0x00, 0x00, 0x00,
            0x12, 0x34, 0x56, 0x78,
            0x04, 0x00, 0x00, 0x00,
            0xAA, 0xBB, 0xCC, 0xDD,
        };
        
        PacketResetChannel packet;
        
        // First parse
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(validData1), std::end(validData1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xDDCCBBAA);

        // Second parse with different data
        std::uint8_t validData2[] = {
            0x04, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44,
            0x04, 0x00, 0x00, 0x00,
            0x55, 0x66, 0x77, 0x88,
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData2), std::end(validData2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x88776655);
    }

    // Test parse after failed parse
    void testParseAfterFailedParse()
    {
        PacketResetChannel packet;
        
        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // invalid type
            0x01, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04,
        };
        
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse with valid data
        std::uint8_t validData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x99, 0x88, 0x77, 0x66,
            0x04, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44,
        };
        
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x66778899);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x44332211);
    }

    // Test sequential failures
    void testSequentialFailures()
    {
        PacketResetChannel packet;
        
        // First failure: invalid type
        std::uint8_t invalidType[] = {0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: invalid size
        std::uint8_t invalidSize[] = {0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x99};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidSize), std::end(invalidSize));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Third failure: truncated
        std::uint8_t truncated[] = {0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Should still be able to parse valid packet after multiple failures
        std::uint8_t validData[] = {0x04, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00};
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x99);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xAA);
    }

    // Test corrupted header fields
    void testCorruptedHeaderFields()
    {
        // Test with corrupted type field (partial corruption)
        std::uint8_t corruptedTypeData[] = {
            0x04, 0xFF, 0x00, 0x00, // type (partially corrupted)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel corruptedTypePacket;
        DataBufferPtr corruptedTypeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedTypeData), std::end(corruptedTypeData));
        
        CPPUNIT_ASSERT(!corruptedTypePacket.parse(std::move(corruptedTypeBuffer)));
        CPPUNIT_ASSERT(!corruptedTypePacket.isValid());

        // Test with corrupted size field
        std::uint8_t corruptedSizeData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0xFF, 0x00, 0x00, // size (corrupted)
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel corruptedSizePacket;
        DataBufferPtr corruptedSizeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedSizeData), std::end(corruptedSizeData));
        
        CPPUNIT_ASSERT(!corruptedSizePacket.parse(std::move(corruptedSizeBuffer)));
        CPPUNIT_ASSERT(!corruptedSizePacket.isValid());

        // Test with corrupted counter field
        std::uint8_t corruptedCounterData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0xFF, 0xFF, 0xFF, // counter (partially corrupted)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketResetChannel corruptedCounterPacket;
        DataBufferPtr corruptedCounterBuffer = std::make_unique<DataBuffer>(std::begin(corruptedCounterData), std::end(corruptedCounterData));
        
        CPPUNIT_ASSERT(corruptedCounterPacket.parse(std::move(corruptedCounterBuffer)));
        CPPUNIT_ASSERT(corruptedCounterPacket.isValid());
        CPPUNIT_ASSERT(corruptedCounterPacket.getCounter() == 0xFFFFFF01);
    }

    void testExactSizeMatching()
    {
        // Test that size field must exactly match available data
        std::uint8_t sizeMismatchData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x08, 0x00, 0x00, 0x00, // size (8, but only 4 bytes of data follow)
            0x01, 0x02, 0x03, 0x04, // channel id (4 bytes)
        };
        
        PacketResetChannel sizeMismatchPacket;
        DataBufferPtr sizeMismatchBuffer = std::make_unique<DataBuffer>(std::begin(sizeMismatchData), std::end(sizeMismatchData));
        
        CPPUNIT_ASSERT(!sizeMismatchPacket.parse(std::move(sizeMismatchBuffer)));
        CPPUNIT_ASSERT(!sizeMismatchPacket.isValid());

        // Test with size smaller than available data (already covered in testTooLong)
        // Test with size exactly matching data (should succeed)
        std::uint8_t exactSizeData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size (exactly 4)
            0x01, 0x02, 0x03, 0x04, // channel id (exactly 4 bytes)
        };
        
        PacketResetChannel exactSizePacket;
        DataBufferPtr exactSizeBuffer = std::make_unique<DataBuffer>(std::begin(exactSizeData), std::end(exactSizeData));
        
        CPPUNIT_ASSERT(exactSizePacket.parse(std::move(exactSizeBuffer)));
        CPPUNIT_ASSERT(exactSizePacket.isValid());
        CPPUNIT_ASSERT(exactSizePacket.getSize() == 4);
        CPPUNIT_ASSERT(exactSizePacket.getChannelId() == 0x04030201);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketResetChannelTest );
