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

#include "PacketTimestamp.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketTimestamp;

class PacketTimestampTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTimestampTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadSize);
    CPPUNIT_TEST(testTooShort);
    CPPUNIT_TEST(testTooLong);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testGetTypeBasic);
    CPPUNIT_TEST(testGetCounterBasic);
    CPPUNIT_TEST(testGetSizeBasic);
    CPPUNIT_TEST(testGetTimestampBasic);
    CPPUNIT_TEST(testGetStcBasic);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testParseMethodBasic);
    CPPUNIT_TEST(testZeroValues);
    CPPUNIT_TEST(testSimpleIncrementalValues);
    CPPUNIT_TEST(testTimestampBoundaryValues);
    CPPUNIT_TEST(testStcBoundaryValues);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testTimestampEndianness);
    CPPUNIT_TEST(testStcEndianness);
    CPPUNIT_TEST(testCounterEndianness);
    CPPUNIT_TEST(testSizeEdgeCases);
    CPPUNIT_TEST(testTypeAlternativeValues);
    CPPUNIT_TEST(testTypeInvalidValues);
    CPPUNIT_TEST(testTruncatedPacketVariations);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testMultipleParseCalls);
    CPPUNIT_TEST(testParseAfterFailedParse);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testSequentialFailures);
    CPPUNIT_TEST(testCorruptedDataFields);
    CPPUNIT_TEST(testEndiannessPatternsComprehensive);
    CPPUNIT_TEST(testExactSizeMatching);
    CPPUNIT_TEST(testProtocolCompliance);
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
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 12);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x0123456789ABCDEF);
        CPPUNIT_ASSERT(packet.getStc() == 0x44332211);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF2, 0x00, 0x00, 0x00, // type (invalid)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x01, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, // stc (cut)
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
                0x00, 0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test constructor and initial state
    void testConstructorInitialState()
    {
        PacketTimestamp packet;
        
        CPPUNIT_ASSERT(!packet.isValid()); // Should be invalid before parsing
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP); // Type should be set by constructor
    }

    // Test getType method
    void testGetTypeBasic()
    {
        PacketTimestamp packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
        
        // Type should remain consistent after successful parsing
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, // timestamp
            0xAA, 0xBB, 0xCC, 0xDD, // stc
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
    }

    // Test getCounter method
    void testGetCounterBasic()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter (little endian)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412); // Little endian conversion
    }

    // Test getSize method
    void testGetSizeBasic()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size (must be 12 for timestamp)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 12);
    }

    // Test getTimestamp method
    void testGetTimestampBasic()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, // timestamp (little endian)
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getTimestamp() == 0xF0DEBC9A78563412); // Little endian conversion
    }

    // Test getStc method
    void testGetStcBasic()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x12, 0x34, 0x56, 0x78, // stc (little endian)
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getStc() == 0x78563412); // Little endian conversion
    }

    // Test isValid method
    void testIsValidBasic()
    {
        PacketTimestamp packet;
        
        // Should be invalid initially
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Should be valid after successful parse
        std::uint8_t validData[] = {
            0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x11, 0x22, 0x33, 0x44,
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    // Test parse method return values
    void testParseMethodBasic()
    {
        PacketTimestamp packet;
        
        // Test successful parse returns true
        std::uint8_t validData[] = {
            0x02, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xAA, 0xBB, 0xCC, 0xDD,
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        
        // Test failed parse returns false
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // invalid type
            0x00, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x11, 0x22, 0x33, 0x44,
        };
        
        PacketTimestamp packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet2.parse(std::move(buffer2)) == false);
    }

    // Test zero values
    void testZeroValues()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter (0)
            0x0C, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // timestamp (0)
            0x00, 0x00, 0x00, 0x00, // stc (0)
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0);
        CPPUNIT_ASSERT(packet.getStc() == 0);
    }

    // Test simple incremental values
    void testSimpleIncrementalValues()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter (1)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // timestamp (1)
            0x01, 0x00, 0x00, 0x00, // stc (1)
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 1);
        CPPUNIT_ASSERT(packet.getTimestamp() == 1);
        CPPUNIT_ASSERT(packet.getStc() == 1);
    }

    // Test timestamp boundary values
    void testTimestampBoundaryValues()
    {
        // Test maximum uint64_t timestamp value
        std::uint8_t maxTimestampData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // timestamp (max uint64_t)
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxTimestampData), std::end(maxTimestampData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getTimestamp() == 0xFFFFFFFFFFFFFFFF);

        // Test power-of-2 boundary timestamp
        std::uint8_t powerOf2Data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, // timestamp (0x8000000000000000)
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp powerOf2Packet;
        DataBufferPtr powerOf2Buffer = std::make_unique<DataBuffer>(std::begin(powerOf2Data), std::end(powerOf2Data));
        
        CPPUNIT_ASSERT(powerOf2Packet.parse(std::move(powerOf2Buffer)));
        CPPUNIT_ASSERT(powerOf2Packet.isValid());
        CPPUNIT_ASSERT(powerOf2Packet.getTimestamp() == 0x8000000000000000);
    }

    // Test STC boundary values
    void testStcBoundaryValues()
    {
        // Test maximum uint32_t STC value
        std::uint8_t maxStcData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0xFF, 0xFF, 0xFF, 0xFF, // stc (max uint32_t)
        };
        
        PacketTimestamp maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxStcData), std::end(maxStcData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getStc() == 0xFFFFFFFF);

        // Test power-of-2 boundary STC
        std::uint8_t powerOf2StcData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x00, 0x00, 0x00, 0x80, // stc (0x80000000)
        };
        
        PacketTimestamp powerOf2Packet;
        DataBufferPtr powerOf2Buffer = std::make_unique<DataBuffer>(std::begin(powerOf2StcData), std::end(powerOf2StcData));
        
        CPPUNIT_ASSERT(powerOf2Packet.parse(std::move(powerOf2Buffer)));
        CPPUNIT_ASSERT(powerOf2Packet.isValid());
        CPPUNIT_ASSERT(powerOf2Packet.getStc() == 0x80000000);
    }

    // Test counter boundary values
    void testCounterBoundaryValues()
    {
        // Test maximum uint32_t counter value
        std::uint8_t maxCounterData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter (max uint32_t)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxCounterData), std::end(maxCounterData));
        
        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)));
        CPPUNIT_ASSERT(maxPacket.isValid());
        CPPUNIT_ASSERT(maxPacket.getCounter() == 0xFFFFFFFF);

        // Test mid-range counter value
        std::uint8_t midCounterData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x80, 0x7F, // counter (0x7F800000)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp midPacket;
        DataBufferPtr midBuffer = std::make_unique<DataBuffer>(std::begin(midCounterData), std::end(midCounterData));
        
        CPPUNIT_ASSERT(midPacket.parse(std::move(midBuffer)));
        CPPUNIT_ASSERT(midPacket.isValid());
        CPPUNIT_ASSERT(midPacket.getCounter() == 0x7F800000);
    }

    // Test timestamp endianness patterns
    void testTimestampEndianness()
    {
        // Test specific little-endian timestamp value
        std::uint8_t endiannessData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp (little endian: 0x0807060504030201)
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x0807060504030201);
    }

    // Test STC endianness patterns
    void testStcEndianness()
    {
        // Test specific little-endian STC value
        std::uint8_t endiannessData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc (little endian: 0x44332211)
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getStc() == 0x44332211);
    }

    // Test counter endianness patterns
    void testCounterEndianness()
    {
        // Test specific little-endian counter value
        std::uint8_t endiannessData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (little endian: 0x04030201)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(endiannessData), std::end(endiannessData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x04030201);
    }

    // Test size field edge cases
    void testSizeEdgeCases()
    {
        // Test with size = 0 (should fail)
        std::uint8_t size0Data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };
        
        PacketTimestamp size0Packet;
        DataBufferPtr size0Buffer = std::make_unique<DataBuffer>(std::begin(size0Data), std::end(size0Data));
        
        CPPUNIT_ASSERT(!size0Packet.parse(std::move(size0Buffer)));
        CPPUNIT_ASSERT(!size0Packet.isValid());

        // Test with size = 11 (should fail)
        std::uint8_t size11Data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0B, 0x00, 0x00, 0x00, // size (11)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, // 11 bytes
        };
        
        PacketTimestamp size11Packet;
        DataBufferPtr size11Buffer = std::make_unique<DataBuffer>(std::begin(size11Data), std::end(size11Data));
        
        CPPUNIT_ASSERT(!size11Packet.parse(std::move(size11Buffer)));
        CPPUNIT_ASSERT(!size11Packet.isValid());

        // Test with large size value (should fail)
        std::uint8_t largeSizeData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0xFF, 0xFF, 0xFF, 0x7F, // size (large value)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp largeSizePacket;
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
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp resetAllPacket;
        DataBufferPtr resetAllBuffer = std::make_unique<DataBuffer>(std::begin(resetAllData), std::end(resetAllData));
        
        CPPUNIT_ASSERT(!resetAllPacket.parse(std::move(resetAllBuffer)));
        CPPUNIT_ASSERT(!resetAllPacket.isValid());

        // Test with PES_DATA type (should fail)
        std::uint8_t pesData[] = {
            0x01, 0x00, 0x00, 0x00, // type (PES_DATA)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp pesPacket;
        DataBufferPtr pesBuffer = std::make_unique<DataBuffer>(std::begin(pesData), std::end(pesData));
        
        CPPUNIT_ASSERT(!pesPacket.parse(std::move(pesBuffer)));
        CPPUNIT_ASSERT(!pesPacket.isValid());

        // Test with RESET_CHANNEL type (should fail)
        std::uint8_t resetChannelData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp resetChannelPacket;
        DataBufferPtr resetChannelBuffer = std::make_unique<DataBuffer>(std::begin(resetChannelData), std::end(resetChannelData));
        
        CPPUNIT_ASSERT(!resetChannelPacket.parse(std::move(resetChannelBuffer)));
        CPPUNIT_ASSERT(!resetChannelPacket.isValid());
    }

    // Test invalid type values
    void testTypeInvalidValues()
    {
        // Test with completely invalid type (0)
        std::uint8_t type0Data[] = {
            0x00, 0x00, 0x00, 0x00, // type (0)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp type0Packet;
        DataBufferPtr type0Buffer = std::make_unique<DataBuffer>(std::begin(type0Data), std::end(type0Data));
        
        CPPUNIT_ASSERT(!type0Packet.parse(std::move(type0Buffer)));
        CPPUNIT_ASSERT(!type0Packet.isValid());

        // Test with INVALID type value
        std::uint8_t invalidTypeData[] = {
            0xFF, 0xFF, 0xFF, 0xFF, // type (INVALID)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp invalidPacket;
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidTypeData), std::end(invalidTypeData));
        
        CPPUNIT_ASSERT(!invalidPacket.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!invalidPacket.isValid());

        // Test with random type value
        std::uint8_t randomTypeData[] = {
            0x99, 0x88, 0x77, 0x66, // type (random)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp randomPacket;
        DataBufferPtr randomBuffer = std::make_unique<DataBuffer>(std::begin(randomTypeData), std::end(randomTypeData));
        
        CPPUNIT_ASSERT(!randomPacket.parse(std::move(randomBuffer)));
        CPPUNIT_ASSERT(!randomPacket.isValid());
    }

    // Test various truncated packet scenarios
    void testTruncatedPacketVariations()
    {
        // Test with partial type field
        std::uint8_t partialTypeData[] = {
            0x02, 0x00, // partial type
        };
        
        PacketTimestamp partialTypePacket;
        DataBufferPtr partialTypeBuffer = std::make_unique<DataBuffer>(std::begin(partialTypeData), std::end(partialTypeData));
        
        CPPUNIT_ASSERT(!partialTypePacket.parse(std::move(partialTypeBuffer)));
        CPPUNIT_ASSERT(!partialTypePacket.isValid());

        // Test with missing counter field
        std::uint8_t missingCounterData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, // partial counter
        };
        
        PacketTimestamp missingCounterPacket;
        DataBufferPtr missingCounterBuffer = std::make_unique<DataBuffer>(std::begin(missingCounterData), std::end(missingCounterData));
        
        CPPUNIT_ASSERT(!missingCounterPacket.parse(std::move(missingCounterBuffer)));
        CPPUNIT_ASSERT(!missingCounterPacket.isValid());

        // Test with missing size field
        std::uint8_t missingSizeData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, // partial size
        };
        
        PacketTimestamp missingSizePacket;
        DataBufferPtr missingSizeBuffer = std::make_unique<DataBuffer>(std::begin(missingSizeData), std::end(missingSizeData));
        
        CPPUNIT_ASSERT(!missingSizePacket.parse(std::move(missingSizeBuffer)));
        CPPUNIT_ASSERT(!missingSizePacket.isValid());

        // Test with partial timestamp (4 bytes)
        std::uint8_t partialTimestampData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // partial timestamp (4 bytes)
        };
        
        PacketTimestamp partialTimestampPacket;
        DataBufferPtr partialTimestampBuffer = std::make_unique<DataBuffer>(std::begin(partialTimestampData), std::end(partialTimestampData));
        
        CPPUNIT_ASSERT(!partialTimestampPacket.parse(std::move(partialTimestampBuffer)));
        CPPUNIT_ASSERT(!partialTimestampPacket.isValid());

        // Test with missing STC (timestamp complete, STC missing)
        std::uint8_t missingStcData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp (complete)
            0x11, 0x22, // partial STC
        };
        
        PacketTimestamp missingStcPacket;
        DataBufferPtr missingStcBuffer = std::make_unique<DataBuffer>(std::begin(missingStcData), std::end(missingStcData));
        
        CPPUNIT_ASSERT(!missingStcPacket.parse(std::move(missingStcBuffer)));
        CPPUNIT_ASSERT(!missingStcPacket.isValid());
    }

    // Test empty buffer
    void testEmptyBuffer()
    {
        PacketTimestamp emptyPacket;
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!emptyPacket.parse(std::move(emptyBuffer)));
        CPPUNIT_ASSERT(!emptyPacket.isValid());
    }

    // Test multiple parse calls on same packet
    void testMultipleParseCalls()
    {
        std::uint8_t validData1[] = {
            0x02, 0x00, 0x00, 0x00,
            0x12, 0x34, 0x56, 0x78,
            0x0C, 0x00, 0x00, 0x00,
            0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
            0x33, 0x44, 0x55, 0x66,
        };
        
        PacketTimestamp packet;
        
        // First parse
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(validData1), std::end(validData1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x2211FFEEDDCCBBAA);
        CPPUNIT_ASSERT(packet.getStc() == 0x66554433);

        // Second parse with different data
        std::uint8_t validData2[] = {
            0x02, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44,
            0x0C, 0x00, 0x00, 0x00,
            0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC,
            0xDD, 0xEE, 0xFF, 0x00,
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData2), std::end(validData2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0xCCBBAA9988776655);
        CPPUNIT_ASSERT(packet.getStc() == 0x00FFEEDD);
    }

    // Test parse after failed parse
    void testParseAfterFailedParse()
    {
        PacketTimestamp packet;
        
        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // invalid type
            0x01, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x11, 0x22, 0x33, 0x44,
        };
        
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse with valid data
        std::uint8_t validData[] = {
            0x02, 0x00, 0x00, 0x00,
            0x99, 0x88, 0x77, 0x66,
            0x0C, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0x99, 0xAA, 0xBB, 0xCC,
        };
        
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x66778899);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x8877665544332211);
        CPPUNIT_ASSERT(packet.getStc() == 0xCCBBAA99);
    }

    // Test packet reuse with different scenarios
    void testPacketReuse()
    {
        PacketTimestamp packet;
        
        // Parse 1: Valid packet
        std::uint8_t data1[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(data1), std::end(data1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 1);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x10);
        CPPUNIT_ASSERT(packet.getStc() == 0x20);

        // Parse 2: Another valid packet
        std::uint8_t data2[] = {0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(data2), std::end(data2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 2);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x30);
        CPPUNIT_ASSERT(packet.getStc() == 0x40);

        // Parse 3: Invalid packet
        std::uint8_t data3[] = {0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x99};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(data3), std::end(data3));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Test sequential failures
    void testSequentialFailures()
    {
        PacketTimestamp packet;
        
        // First failure: invalid type
        std::uint8_t invalidType[] = {0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: invalid size
        std::uint8_t invalidSize[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x99};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidSize), std::end(invalidSize));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Third failure: truncated
        std::uint8_t truncated[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Should still be able to parse valid packet after multiple failures
        std::uint8_t validData[] = {0x02, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0x00, 0x00};
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x99);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0xAA);
        CPPUNIT_ASSERT(packet.getStc() == 0xBB);
    }

    // Test corrupted data fields
    void testCorruptedDataFields()
    {
        // Test with corrupted type field (partial corruption)
        std::uint8_t corruptedTypeData[] = {
            0x02, 0xFF, 0x00, 0x00, // type (partially corrupted)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp corruptedTypePacket;
        DataBufferPtr corruptedTypeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedTypeData), std::end(corruptedTypeData));
        
        CPPUNIT_ASSERT(!corruptedTypePacket.parse(std::move(corruptedTypeBuffer)));
        CPPUNIT_ASSERT(!corruptedTypePacket.isValid());

        // Test with corrupted size field
        std::uint8_t corruptedSizeData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0xFF, 0x00, 0x00, // size (corrupted)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp corruptedSizePacket;
        DataBufferPtr corruptedSizeBuffer = std::make_unique<DataBuffer>(std::begin(corruptedSizeData), std::end(corruptedSizeData));
        
        CPPUNIT_ASSERT(!corruptedSizePacket.parse(std::move(corruptedSizeBuffer)));
        CPPUNIT_ASSERT(!corruptedSizePacket.isValid());

        // Test with corrupted counter field (should still parse successfully)
        std::uint8_t corruptedCounterData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0xFF, 0xFF, 0xFF, // counter (partially corrupted)
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp
            0x11, 0x22, 0x33, 0x44, // stc
        };
        
        PacketTimestamp corruptedCounterPacket;
        DataBufferPtr corruptedCounterBuffer = std::make_unique<DataBuffer>(std::begin(corruptedCounterData), std::end(corruptedCounterData));
        
        CPPUNIT_ASSERT(corruptedCounterPacket.parse(std::move(corruptedCounterBuffer)));
        CPPUNIT_ASSERT(corruptedCounterPacket.isValid());
        CPPUNIT_ASSERT(corruptedCounterPacket.getCounter() == 0xFFFFFF01);
    }

    // Test comprehensive endianness patterns
    void testEndiannessPatternsComprehensive()
    {
        // Test alternating byte pattern
        std::uint8_t alternatingData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0xAA, 0x55, 0xAA, 0x55, // counter (alternating pattern)
            0x0C, 0x00, 0x00, 0x00, // size
            0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, // timestamp (alternating pattern)
            0xAA, 0x55, 0xAA, 0x55, // stc (alternating pattern)
        };
        
        PacketTimestamp alternatingPacket;
        DataBufferPtr alternatingBuffer = std::make_unique<DataBuffer>(std::begin(alternatingData), std::end(alternatingData));
        
        CPPUNIT_ASSERT(alternatingPacket.parse(std::move(alternatingBuffer)));
        CPPUNIT_ASSERT(alternatingPacket.isValid());
        CPPUNIT_ASSERT(alternatingPacket.getCounter() == 0x55AA55AA);
        CPPUNIT_ASSERT(alternatingPacket.getTimestamp() == 0xAA55AA55AA55AA55);
        CPPUNIT_ASSERT(alternatingPacket.getStc() == 0x55AA55AA);

        // Test ascending byte pattern
        std::uint8_t ascendingData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (ascending)
            0x0C, 0x00, 0x00, 0x00, // size
            0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, // timestamp (ascending)
            0x0D, 0x0E, 0x0F, 0x10, // stc (ascending)
        };
        
        PacketTimestamp ascendingPacket;
        DataBufferPtr ascendingBuffer = std::make_unique<DataBuffer>(std::begin(ascendingData), std::end(ascendingData));
        
        CPPUNIT_ASSERT(ascendingPacket.parse(std::move(ascendingBuffer)));
        CPPUNIT_ASSERT(ascendingPacket.isValid());
        CPPUNIT_ASSERT(ascendingPacket.getCounter() == 0x04030201);
        CPPUNIT_ASSERT(ascendingPacket.getTimestamp() == 0x0C0B0A0908070605);
        CPPUNIT_ASSERT(ascendingPacket.getStc() == 0x100F0E0D);
    }

    // Test exact size matching requirements
    void testExactSizeMatching()
    {
        // Test that size field must exactly match available data
        std::uint8_t sizeMismatchData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x10, 0x00, 0x00, 0x00, // size (16, but only 12 bytes of data follow)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp (8 bytes)
            0x11, 0x22, 0x33, 0x44, // stc (4 bytes)
        };
        
        PacketTimestamp sizeMismatchPacket;
        DataBufferPtr sizeMismatchBuffer = std::make_unique<DataBuffer>(std::begin(sizeMismatchData), std::end(sizeMismatchData));
        
        CPPUNIT_ASSERT(!sizeMismatchPacket.parse(std::move(sizeMismatchBuffer)));
        CPPUNIT_ASSERT(!sizeMismatchPacket.isValid());

        // Test with size exactly matching data (should succeed)
        std::uint8_t exactSizeData[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size (exactly 12)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp (exactly 8 bytes)
            0x11, 0x22, 0x33, 0x44, // stc (exactly 4 bytes)
        };
        
        PacketTimestamp exactSizePacket;
        DataBufferPtr exactSizeBuffer = std::make_unique<DataBuffer>(std::begin(exactSizeData), std::end(exactSizeData));
        
        CPPUNIT_ASSERT(exactSizePacket.parse(std::move(exactSizeBuffer)));
        CPPUNIT_ASSERT(exactSizePacket.isValid());
        CPPUNIT_ASSERT(exactSizePacket.getSize() == 12);
        CPPUNIT_ASSERT(exactSizePacket.getTimestamp() == 0x0807060504030201);
        CPPUNIT_ASSERT(exactSizePacket.getStc() == 0x44332211);
    }

    // Test protocol compliance
    void testProtocolCompliance()
    {
        // Test fixed packet structure validation
        std::uint8_t validProtocolData[] = {
            0x02, 0x00, 0x00, 0x00, // type (TIMESTAMP)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size (exactly 12 bytes)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // timestamp (8 bytes)
            0x11, 0x22, 0x33, 0x44, // stc (4 bytes)
        };
        
        PacketTimestamp protocolPacket;
        DataBufferPtr protocolBuffer = std::make_unique<DataBuffer>(std::begin(validProtocolData), std::end(validProtocolData));
        
        CPPUNIT_ASSERT(protocolPacket.parse(std::move(protocolBuffer)));
        CPPUNIT_ASSERT(protocolPacket.isValid());
        CPPUNIT_ASSERT(protocolPacket.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(protocolPacket.getSize() == 12); // Fixed size requirement
        CPPUNIT_ASSERT(protocolPacket.getTimestamp() == 0x0807060504030201);
        CPPUNIT_ASSERT(protocolPacket.getStc() == 0x44332211);

        // Verify field ordering and structure
        CPPUNIT_ASSERT(protocolPacket.getCounter() == 1);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTimestampTest );
