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
#include "PacketResetAll.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketResetAll;

class PacketResetAllTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketResetAllTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadCounter);
    CPPUNIT_TEST(testBadSize);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testGetTypeBasic);
    CPPUNIT_TEST(testGetCounterBasic);
    CPPUNIT_TEST(testGetSizeBasic);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testTypeAlternativeValues);
    CPPUNIT_TEST(testTruncatedPacket);
    CPPUNIT_TEST(testExtraTrailingBytes);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testMultipleParseCalls);
    CPPUNIT_TEST(testParseAfterFailedParse);
    CPPUNIT_TEST(testEndianness);
    CPPUNIT_TEST(testCorruptedHeaderFields);
    CPPUNIT_TEST(testSequentialFailures);
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
                0x03, 0x00, 0x00, 0x00, // type
                0x00, 0x00, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF3, 0x00, 0x00, 0x00, // type (invalid)
                0x00, 0x00, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadCounter()
    {
        std::uint8_t packetData[] =
        {
                0x03, 0x00, 0x00, 0x00, // type
                0x00, 0x01, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // for reset all counter is not taken into account, packet is valid
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x03, 0x00, 0x00, 0x00, // type
                0x00, 0x00, 0x00, 0x00, // counter
                0x10, 0x00, 0x00, 0x00, // size (non-zero, should be 0)
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test constructor and initial state
    void testConstructorInitialState()
    {
        PacketResetAll packet;
        
        CPPUNIT_ASSERT(!packet.isValid()); // Should be invalid before parsing
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL); // Type should be set by constructor
    }

    // Test getType method
    void testGetTypeBasic()
    {
        PacketResetAll packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        
        // Type should remain consistent after successful parsing
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    // Test getCounter method
    void testGetCounterBasic()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter (little endian)
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412); // Little endian conversion
    }

    // Test getSize method
    void testGetSizeBasic()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (must be 0)
        };
        
        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 0);
    }

    // Test isValid method
    void testIsValidBasic()
    {
        PacketResetAll packet;
        
        // Should be invalid initially
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Should be valid after successful parse
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    // Test counter boundary values
    void testCounterBoundaryValues()
    {
        // Test maximum uint32_t counter value
        std::uint8_t maxCounterData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter (max uint32_t)
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxCounterData), std::end(maxCounterData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getCounter() == 0xFFFFFFFF);
    }

    // Test alternative type values
    void testTypeAlternativeValues()
    {
        // Test with PES_DATA type (should fail)
        std::uint8_t pesTypeData[] = {
            0x01, 0x00, 0x00, 0x00, // type (PES_DATA)
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll pesPacket;
        DataBufferPtr pesBuffer = std::make_unique<DataBuffer>(std::begin(pesTypeData), std::end(pesTypeData));
        
        CPPUNIT_ASSERT(!pesPacket.parse(std::move(pesBuffer)));
        CPPUNIT_ASSERT(!pesPacket.isValid());

        // Test with TIMESTAMP type (should fail)
        std::uint8_t timestampTypeData[] = {
            0x02, 0x00, 0x00, 0x00, // type (TIMESTAMP)
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll timestampPacket;
        DataBufferPtr timestampBuffer = std::make_unique<DataBuffer>(std::begin(timestampTypeData), std::end(timestampTypeData));
        
        CPPUNIT_ASSERT(!timestampPacket.parse(std::move(timestampBuffer)));
        CPPUNIT_ASSERT(!timestampPacket.isValid());
    }

    // Test truncated packet
    void testTruncatedPacket()
    {
        // Test with missing size field
        std::uint8_t truncatedData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            // missing size field
        };
        
        PacketResetAll truncatedPacket;
        DataBufferPtr truncatedBuffer = std::make_unique<DataBuffer>(std::begin(truncatedData), std::end(truncatedData));
        
        CPPUNIT_ASSERT(!truncatedPacket.parse(std::move(truncatedBuffer)));
        CPPUNIT_ASSERT(!truncatedPacket.isValid());

        // Test with partially missing counter
        std::uint8_t partialData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, // partial counter
        };
        
        PacketResetAll partialPacket;
        DataBufferPtr partialBuffer = std::make_unique<DataBuffer>(std::begin(partialData), std::end(partialData));
        
        CPPUNIT_ASSERT(!partialPacket.parse(std::move(partialBuffer)));
        CPPUNIT_ASSERT(!partialPacket.isValid());
    }

    // Test extra trailing bytes
    void testExtraTrailingBytes()
    {
        std::uint8_t extraBytesData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // extra bytes (should cause failure)
        };
        
        PacketResetAll extraBytesPacket;
        DataBufferPtr extraBytesBuffer = std::make_unique<DataBuffer>(std::begin(extraBytesData), std::end(extraBytesData));
        
        CPPUNIT_ASSERT(!extraBytesPacket.parse(std::move(extraBytesBuffer)));
        CPPUNIT_ASSERT(!extraBytesPacket.isValid());
    }

    // Test empty buffer
    void testEmptyBuffer()
    {
        PacketResetAll emptyPacket;
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!emptyPacket.parse(std::move(emptyBuffer)));
        CPPUNIT_ASSERT(!emptyPacket.isValid());
    }

    // Test multiple parse calls on same packet
    void testMultipleParseCalls()
    {
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x12, 0x34, 0x56, 0x78,
            0x00, 0x00, 0x00, 0x00,
        };
        
        PacketResetAll packet;
        
        // First parse
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412);

        // Second parse with different data
        std::uint8_t differentData[] = {
            0x03, 0x00, 0x00, 0x00,
            0xAA, 0xBB, 0xCC, 0xDD,
            0x00, 0x00, 0x00, 0x00,
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(differentData), std::end(differentData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0xDDCCBBAA);
    }

    // Test parse after failed parse
    void testParseAfterFailedParse()
    {
        PacketResetAll packet;
        
        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0xF3, 0x00, 0x00, 0x00, // invalid type
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
        };
        
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse with valid data
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44,
            0x00, 0x00, 0x00, 0x00,
        };
        
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
    }

    // Test endianness handling
    void testEndianness()
    {
        // Test specific little-endian counter value
        std::uint8_t endiannessData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (little endian: 0x04030201)
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x04030201);
    }

    // Test corrupted header fields
    void testCorruptedHeaderFields()
    {
        // Test with corrupted type field (partial corruption)
        std::uint8_t corruptedTypeData[] = {
            0x03, 0xFF, 0x00, 0x00, // type (partially corrupted)
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        
        PacketResetAll corruptedTypePacket;
        DataBufferPtr corruptedTypeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedTypeData), std::end(corruptedTypeData));
        
        CPPUNIT_ASSERT(!corruptedTypePacket.parse(std::move(corruptedTypeBuffer)));
        CPPUNIT_ASSERT(!corruptedTypePacket.isValid());

        // Test with corrupted size field
        std::uint8_t corruptedSizeData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x00, 0xFF, 0x00, 0x00, // size (corrupted)
        };
        
        PacketResetAll corruptedSizePacket;
        DataBufferPtr corruptedSizeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedSizeData), std::end(corruptedSizeData));
        
        CPPUNIT_ASSERT(!corruptedSizePacket.parse(std::move(corruptedSizeBuffer)));
        CPPUNIT_ASSERT(!corruptedSizePacket.isValid());
    }

    // Test sequential failures
    void testSequentialFailures()
    {
        PacketResetAll packet;
        
        // First failure: invalid type
        std::uint8_t invalidType[] = {0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: invalid size
        std::uint8_t invalidSize[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidSize), std::end(invalidSize));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Third failure: truncated
        std::uint8_t truncated[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Should still be able to parse valid packet after multiple failures
        std::uint8_t validData[] = {0x03, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x99);
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketResetAllTest );
