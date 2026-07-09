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

#include "PacketTtmlTimestamp.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketTtmlTimestamp;

class PacketTtmlTimestampTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTtmlTimestampTest );
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testConstructorTypeIdentity);
    CPPUNIT_TEST(testConstructorTimestampZero);
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testParseTimestampZero);
    CPPUNIT_TEST(testParseSmallTimestamp);
    CPPUNIT_TEST(testParseMediumTimestamp);
    CPPUNIT_TEST(testParseMaxTimestamp);
    CPPUNIT_TEST(testParseSpecificPattern);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadTypeZero);
    CPPUNIT_TEST(testBadTypeRandom);
    CPPUNIT_TEST(testBadSizeZero);
    CPPUNIT_TEST(testBadSizeOffByOneLess);
    CPPUNIT_TEST(testBadSizeOffByOneMore);
    CPPUNIT_TEST(testBadSizeMissingTimestamp);
    CPPUNIT_TEST(testBadSizeExcessive);
    CPPUNIT_TEST(testParseEmptyBuffer);
    CPPUNIT_TEST(testParseHeaderOnly);
    CPPUNIT_TEST(testParseTruncatedAfterChannelId);
    CPPUNIT_TEST(testParsePartialTimestamp1Byte);
    CPPUNIT_TEST(testParsePartialTimestamp4Bytes);
    CPPUNIT_TEST(testParsePartialTimestamp7Bytes);
    CPPUNIT_TEST(testParse23Bytes);
    CPPUNIT_TEST(testLittleEndianByteOrder);
    CPPUNIT_TEST(testLittleEndianAlternatingPattern);
    CPPUNIT_TEST(testLittleEndianHighByte);
    CPPUNIT_TEST(testStateValidToValid);
    CPPUNIT_TEST(testStateValidToInvalid);
    CPPUNIT_TEST(testStateInvalidToValid);
    CPPUNIT_TEST(testStateMultipleValidParses);
    CPPUNIT_TEST(testGetterBeforeParse);
    CPPUNIT_TEST(testGetterAfterFailedParse);
    CPPUNIT_TEST(testGetterMultipleCalls);
    CPPUNIT_TEST(testBaseClassChannelId);
    CPPUNIT_TEST(testBaseClassCounter);
    CPPUNIT_TEST(testBaseClassSize);
    CPPUNIT_TEST(testBaseClassParseFailure);
    CPPUNIT_TEST(testBoundaryTimestampOne);
    CPPUNIT_TEST(testBoundaryMaxSigned64);
    CPPUNIT_TEST(testBoundaryMinNegativeSigned);
    CPPUNIT_TEST(testBoundaryMaxMinusOne);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testAlternatingValidInvalid);
    CPPUNIT_TEST(testCorruptedHeaderType);
    CPPUNIT_TEST(testCorruptedSize);
    CPPUNIT_TEST(testExtraDataAfterPacket);
    CPPUNIT_TEST(testEndToEndWorkflowBasic);
    CPPUNIT_TEST(testEndToEndWorkflowMultiplePackets);
    CPPUNIT_TEST(testEndToEndWorkflowMixedResults);
    CPPUNIT_TEST(testEndToEndWorkflowAllInvalid);
    CPPUNIT_TEST(testEndToEndWorkflowSequentialTimestamps);
    CPPUNIT_TEST(testBufferReaderIntegrationComplete);
    CPPUNIT_TEST(testBufferReaderIntegrationPartial);
    CPPUNIT_TEST(testBufferReaderIntegrationMultipleCalls);
    CPPUNIT_TEST(testPolymorphismAsPacket);
    CPPUNIT_TEST(testPolymorphismAsPacketChannelSpecific);
    CPPUNIT_TEST(testPolymorphismTypeIdentity);
    CPPUNIT_TEST(testStateConsistencyAcrossParsing);
    CPPUNIT_TEST(testStateConsistencyAfterFailures);
    CPPUNIT_TEST(testStateConsistencyTimestampProgression);
    CPPUNIT_TEST(testStateConsistencyRepeatedValues);
    CPPUNIT_TEST(testErrorRecoveryAfterBadType);
    CPPUNIT_TEST(testErrorRecoveryAfterBadSize);
    CPPUNIT_TEST(testErrorRecoveryAfterTruncation);
    CPPUNIT_TEST(testErrorRecoveryMultipleFailures);
    CPPUNIT_TEST(testCrossComponentBufferToPacket);
    CPPUNIT_TEST(testCrossComponentMultiplePacketParsing);
    CPPUNIT_TEST(testCrossComponentTimestampExtraction);
    CPPUNIT_TEST(testCrossComponentLargePacketStream);
    CPPUNIT_TEST(testRealWorldStreamingScenario);
    CPPUNIT_TEST(testRealWorldMultipleChannels);
    CPPUNIT_TEST(testRealWorldTimestampProgression);
    CPPUNIT_TEST(testRealWorldErrorHandlingScenario);
    CPPUNIT_TEST(testRealWorldHighFrequencyUpdates);
    CPPUNIT_TEST(testRealWorldVariousTimestampRanges);
    CPPUNIT_TEST(testRealWorldPTSWraparound);
    CPPUNIT_TEST(testRealWorldConcurrentChannels);
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

    // Helper function to create valid packet data
    std::vector<std::uint8_t> createValidPacket(
        std::uint32_t channelId = 0x01020304,
        std::uint32_t counter = 0x12345678,
        std::uint64_t timestamp = 1638360000000ULL)
    {
        std::vector<std::uint8_t> packet = {
            0x09, 0x00, 0x00, 0x00, // type (TTML_TIMESTAMP = 9)
            static_cast<std::uint8_t>(counter & 0xFF),
            static_cast<std::uint8_t>((counter >> 8) & 0xFF),
            static_cast<std::uint8_t>((counter >> 16) & 0xFF),
            static_cast<std::uint8_t>((counter >> 24) & 0xFF),
            0x0C, 0x00, 0x00, 0x00, // size (12 = 4 + 8)
            static_cast<std::uint8_t>(channelId & 0xFF),
            static_cast<std::uint8_t>((channelId >> 8) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 16) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 24) & 0xFF),
            static_cast<std::uint8_t>(timestamp & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 8) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 16) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 24) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 32) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 40) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 48) & 0xFF),
            static_cast<std::uint8_t>((timestamp >> 56) & 0xFF),
        };
        return packet;
    }

    void testConstructorInitialState()
    {
        PacketTtmlTimestamp packet;

        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testConstructorTypeIdentity()
    {
        PacketTtmlTimestamp packet;

        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
    }

    void testConstructorTimestampZero()
    {
        PacketTtmlTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    void testGood()
    {
        auto packetData = createValidPacket();

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01020304), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360000000ULL), packet.getTimestamp());
    }

    void testParseTimestampZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    void testParseSmallTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1000), packet.getTimestamp());
    }

    void testParseMediumTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1638360000000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360000000ULL), packet.getTimestamp());
    }

    void testParseMaxTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFFFFFFFFFFULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), packet.getTimestamp());
    }

    void testParseSpecificPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x0102030405060708ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0102030405060708ULL), packet.getTimestamp());
    }

    void testBadType()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x02; // TIMESTAMP type instead of TTML_TIMESTAMP

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeZero()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x00; // Invalid type

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeRandom()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0xFF; // Random invalid type

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeZero()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeOffByOneLess()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0B; // size = 11 (one byte short)
        packetData.pop_back();

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeOffByOneMore()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0D; // size = 13 (one byte extra)
        packetData.push_back(0xFF);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeMissingTimestamp()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x04; // size = 4 (only channelId, missing timestamp)
        packetData.resize(16); // header(12) + channelId(4)

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeExcessive()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x64; // size = 100 (excessive)
        for (int i = 0; i < 88; ++i) {
            packetData.push_back(0xFF);
        }

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseEmptyBuffer()
    {
        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseHeaderOnly()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterChannelId()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParsePartialTimestamp1Byte()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, // timestamp partially present (1 byte)
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParsePartialTimestamp4Bytes()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, // timestamp partially present (4 bytes)
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParsePartialTimestamp7Bytes()
    {
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // timestamp partially present (7 bytes)
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParse23Bytes()
    {
        auto packetData = createValidPacket();
        packetData.pop_back(); // Remove one byte (23 bytes total instead of 24)

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testLittleEndianByteOrder()
    {
        // Bytes [0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08] should become 0x0807060504030201
        auto packetData = createValidPacket(0x01, 0x02, 0x0807060504030201ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), packet.getTimestamp());
    }

    void testLittleEndianAlternatingPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xAA55AA55AA55AA55ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xAA55AA55AA55AA55ULL), packet.getTimestamp());
    }

    void testLittleEndianHighByte()
    {
        // Test that 0x0000000000000001 != 0x0100000000000000
        auto packetData1 = createValidPacket(0x01, 0x02, 0x0000000000000001ULL);
        auto packetData2 = createValidPacket(0x01, 0x02, 0x0100000000000000ULL);

        PacketTtmlTimestamp packet1;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(packetData1.begin(), packetData1.end());
        CPPUNIT_ASSERT(packet1.parse(std::move(buffer1)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000000000001ULL), packet1.getTimestamp());

        PacketTtmlTimestamp packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(packetData2.begin(), packetData2.end());
        CPPUNIT_ASSERT(packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0100000000000000ULL), packet2.getTimestamp());

        CPPUNIT_ASSERT(packet1.getTimestamp() != packet2.getTimestamp());
    }

    void testStateValidToValid()
    {
        PacketTtmlTimestamp packet;

        // First parse: valid
        auto data1 = createValidPacket(0x01, 0x10, 1000ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1000), packet.getTimestamp());

        // Second parse: valid with different timestamp
        auto data2 = createValidPacket(0x02, 0x20, 2000ULL);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(2000), packet.getTimestamp());
    }

    void testStateValidToInvalid()
    {
        PacketTtmlTimestamp packet;

        // First parse: valid
        auto validData = createValidPacket(0x01, 0x02, 1000ULL);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Second parse: invalid
        auto invalidData = createValidPacket(0x01, 0x02, 2000ULL);
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testStateInvalidToValid()
    {
        PacketTtmlTimestamp packet;

        // First parse: invalid
        auto invalidData = createValidPacket(0x01, 0x02, 1000ULL);
        invalidData[8] = 0x00; // Invalid size
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse: valid
        auto validData = createValidPacket(0x05, 0x06, 3000ULL);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3000), packet.getTimestamp());
    }

    void testStateMultipleValidParses()
    {
        PacketTtmlTimestamp packet;

        for (int i = 0; i < 5; ++i) {
            std::uint64_t timestamp = 1000ULL * (i + 1);
            auto packetData = createValidPacket(0x01, 0x10 + i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testGetterBeforeParse()
    {
        PacketTtmlTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testGetterAfterFailedParse()
    {
        PacketTtmlTimestamp packet;

        auto invalidData = createValidPacket();
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr buffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));

        // After failed parse, getter should return initial value
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    void testGetterMultipleCalls()
    {
        auto packetData = createValidPacket(0x01, 0x02, 123456789ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // Call getter multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(123456789ULL), packet.getTimestamp());
        }
    }

    void testBaseClassChannelId()
    {
        auto packetData = createValidPacket(0xDEADBEEF, 0x02, 1000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
    }

    void testBaseClassCounter()
    {
        auto packetData = createValidPacket(0x01, 0xCAFEBABE, 1000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
    }

    void testBaseClassSize()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    void testBaseClassParseFailure()
    {
        // Create packet with valid header but truncated channel ID
        std::uint8_t packetData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, // channel id truncated
        };

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBoundaryTimestampOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1), packet.getTimestamp());
    }

    void testBoundaryMaxSigned64()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x7FFFFFFFFFFFFFFFULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x7FFFFFFFFFFFFFFFULL), packet.getTimestamp());
    }

    void testBoundaryMinNegativeSigned()
    {
        // Test 0x8000000000000000 (would be negative if signed, but treated as unsigned)
        auto packetData = createValidPacket(0x01, 0x02, 0x8000000000000000ULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x8000000000000000ULL), packet.getTimestamp());
    }

    void testBoundaryMaxMinusOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFFFFFFFFFEULL);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFEULL), packet.getTimestamp());
    }

    void testPacketReuse()
    {
        PacketTtmlTimestamp packet;

        // First parse
        auto data1 = createValidPacket(0x01, 0x10, 1000ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1000), packet.getTimestamp());

        // Second parse with different data
        auto data2 = createValidPacket(0x02, 0x20, 2000ULL);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x20), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(2000), packet.getTimestamp());
    }

    void testAlternatingValidInvalid()
    {
        PacketTtmlTimestamp packet;

        for (int i = 0; i < 3; ++i) {
            // Valid packet
            auto validData = createValidPacket(0x100 + i, 0x200 + i, 1000ULL * (i + 1));
            DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x100 + i), packet.getChannelId());

            // Invalid packet
            auto invalidData = createValidPacket(0x300 + i, 0x400 + i, 2000ULL);
            invalidData[8] = 0x00; // Size = 0
            DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
            CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testCorruptedHeaderType()
    {
        auto packetData = createValidPacket();
        packetData[1] = 0xFF; // Corrupt type field

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testCorruptedSize()
    {
        auto packetData = createValidPacket();
        packetData[9] = 0xFF; // Corrupt size field

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testExtraDataAfterPacket()
    {
        auto packetData = createValidPacket();
        packetData.push_back(0xFF);
        packetData.push_back(0xFE);
        packetData.push_back(0xFD);

        PacketTtmlTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Should fail because there's extra data
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testEndToEndWorkflowBasic()
    {
        // Test complete workflow: Buffer creation -> Parsing -> Validation -> Data extraction
        std::uint32_t expectedChannelId = 0x12345678;
        std::uint32_t expectedCounter = 0xABCDEF01;
        std::uint64_t expectedTimestamp = 1638360000000ULL;

        auto packetData = createValidPacket(expectedChannelId, expectedCounter, expectedTimestamp);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketTtmlTimestamp packet;

        // Parse
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Validate
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);

        // Extract and verify all data
        CPPUNIT_ASSERT_EQUAL(expectedChannelId, packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(expectedCounter, packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(expectedTimestamp, packet.getTimestamp());
    }

    void testEndToEndWorkflowMultiplePackets()
    {
        // Test workflow with multiple packets in sequence
        PacketTtmlTimestamp packet;

        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint64_t>> testCases = {
            {0x01, 0x100, 1000000000ULL},
            {0x02, 0x200, 2000000000ULL},
            {0x03, 0x300, 3000000000ULL},
            {0x04, 0x400, 4000000000ULL},
            {0x05, 0x500, 5000000000ULL}
        };

        for (const auto& testCase : testCases) {
            auto packetData = createValidPacket(
                std::get<0>(testCase),
                std::get<1>(testCase),
                std::get<2>(testCase)
            );
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(testCase), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(testCase), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(testCase), packet.getTimestamp());
        }
    }

    void testEndToEndWorkflowMixedResults()
    {
        // Test workflow with alternating valid and invalid packets
        PacketTtmlTimestamp packet;

        // Valid packet 1
        auto valid1 = createValidPacket(0x01, 0x100, 1111111111ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1111111111ULL), packet.getTimestamp());

        // Invalid packet (bad type)
        auto invalid1 = createValidPacket(0x02, 0x200, 2222222222ULL);
        invalid1[0] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(invalid1.begin(), invalid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Valid packet 2
        auto valid2 = createValidPacket(0x03, 0x300, 3333333333ULL);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3333333333ULL), packet.getTimestamp());

        // Invalid packet (truncated)
        std::uint8_t truncated[] = {
            0x09, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03
        };
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Valid packet 3
        auto valid3 = createValidPacket(0x05, 0x500, 5555555555ULL);
        DataBufferPtr buffer5 = std::make_unique<DataBuffer>(valid3.begin(), valid3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer5)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(5555555555ULL), packet.getTimestamp());
    }

    void testEndToEndWorkflowAllInvalid()
    {
        // Test workflow with all invalid packets
        PacketTtmlTimestamp packet;

        // Bad type
        auto bad1 = createValidPacket(0x01, 0x100, 1000ULL);
        bad1[0] = 0x00;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(bad1.begin(), bad1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Bad size
        auto bad2 = createValidPacket(0x02, 0x200, 2000ULL);
        bad2[8] = 0x00;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(bad2.begin(), bad2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Truncated
        std::uint8_t truncated[] = {
            0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testEndToEndWorkflowSequentialTimestamps()
    {
        // Test workflow with sequential timestamps (simulating video timeline)
        PacketTtmlTimestamp packet;
        std::uint64_t baseTimestamp = 1638360000000ULL; // Base timestamp

        for (int i = 0; i < 10; ++i) {
            std::uint64_t timestamp = baseTimestamp + (i * 1000); // Increment by 1 second
            auto packetData = createValidPacket(0x01, 0x100 + i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testBufferReaderIntegrationComplete()
    {
        // Test complete integration with BufferReader for full packet
        auto packetData = createValidPacket(0xAABBCCDD, 0x11223344, 0x0102030405060708ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketTtmlTimestamp packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify BufferReader correctly extracted all fields
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0102030405060708ULL), packet.getTimestamp());
    }

    void testBufferReaderIntegrationPartial()
    {
        // Test BufferReader handling of partial data
        std::uint8_t partialData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x11, 0x22, 0x33 // partial timestamp (3 bytes)
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(partialData), std::end(partialData));

        PacketTtmlTimestamp packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testBufferReaderIntegrationMultipleCalls()
    {
        // Test BufferReader across multiple parse calls
        PacketTtmlTimestamp packet;

        for (int i = 0; i < 20; ++i) {
            std::uint64_t timestamp = 1000000ULL * i;
            auto packetData = createValidPacket(0x10 + i, 0x20 + i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testPolymorphismAsPacket()
    {
        // Test polymorphic access through Packet base class pointer
        auto packetData = createValidPacket(0x01, 0x02, 123456789ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketTtmlTimestamp concretePacket;
        Packet* basePtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), basePtr->getCounter());
    }

    void testPolymorphismAsPacketChannelSpecific()
    {
        // Test polymorphic access through PacketChannelSpecific base class pointer
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 987654321ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketTtmlTimestamp concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(channelPtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(channelPtr->isValid() == true);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), channelPtr->getChannelId());
    }

    void testPolymorphismTypeIdentity()
    {
        // Verify type identity is maintained through polymorphism
        PacketTtmlTimestamp concretePacket;
        Packet* basePtr = &concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT(concretePacket.getType() == Packet::Type::TTML_TIMESTAMP);
    }

    void testStateConsistencyAcrossParsing()
    {
        // Test state remains consistent through multiple parsing operations
        PacketTtmlTimestamp packet;

        std::vector<std::uint64_t> timestamps = {
            0ULL, 1000ULL, 1000000ULL, 1000000000ULL, 1000000000000ULL
        };

        for (size_t i = 0; i < timestamps.size(); ++i) {
            auto packetData = createValidPacket(0x10 + i, 0x20 + i, timestamps[i]);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10 + i), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(timestamps[i], packet.getTimestamp());

            // Verify state is consistent
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        }
    }

    void testStateConsistencyAfterFailures()
    {
        // Test state consistency after parse failures
        PacketTtmlTimestamp packet;

        // Valid parse
        auto valid1 = createValidPacket(0x01, 0x100, 1111ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Failed parse (bad type)
        auto invalid = createValidPacket(0x02, 0x200, 2222ULL);
        invalid[0] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(invalid.begin(), invalid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Another valid parse - state should recover
        auto valid2 = createValidPacket(0x03, 0x300, 3333ULL);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3333ULL), packet.getTimestamp());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x03), packet.getChannelId());
    }

    void testStateConsistencyTimestampProgression()
    {
        // Test state consistency with progressing timestamps
        PacketTtmlTimestamp packet;
        std::uint64_t baseTime = 1638360000000ULL;

        for (int i = 0; i < 50; ++i) {
            std::uint64_t currentTime = baseTime + (i * 40); // 40ms increments
            auto packetData = createValidPacket(0x01, 0x1000 + i, currentTime);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(currentTime, packet.getTimestamp());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1000 + i), packet.getCounter());
        }
    }

    void testStateConsistencyRepeatedValues()
    {
        // Test state consistency with repeated timestamp values
        PacketTtmlTimestamp packet;
        std::uint64_t repeatedTimestamp = 1234567890ULL;

        for (int i = 0; i < 10; ++i) {
            auto packetData = createValidPacket(0x01, 0x100 + i, repeatedTimestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(repeatedTimestamp, packet.getTimestamp());
        }
    }

    void testErrorRecoveryAfterBadType()
    {
        // Test recovery after bad type error
        PacketTtmlTimestamp packet;

        // Bad type
        auto badType = createValidPacket(0x01, 0x100, 1000ULL);
        badType[0] = 0x05; // Wrong type
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(badType.begin(), badType.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery with valid packet
        auto valid = createValidPacket(0x02, 0x200, 2000ULL);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(2000ULL), packet.getTimestamp());
    }

    void testErrorRecoveryAfterBadSize()
    {
        // Test recovery after bad size error
        PacketTtmlTimestamp packet;

        // Bad size
        auto badSize = createValidPacket(0x01, 0x100, 1000ULL);
        badSize[8] = 0x08; // Wrong size
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(badSize.begin(), badSize.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery
        auto valid = createValidPacket(0x02, 0x200, 2000ULL);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(2000ULL), packet.getTimestamp());
    }

    void testErrorRecoveryAfterTruncation()
    {
        // Test recovery after truncation error
        PacketTtmlTimestamp packet;

        // Truncated packet
        std::uint8_t truncated[] = {
            0x09, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03
        };
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery
        auto valid = createValidPacket(0x03, 0x300, 3000ULL);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(3000ULL), packet.getTimestamp());
    }

    void testErrorRecoveryMultipleFailures()
    {
        // Test recovery after multiple consecutive failures
        PacketTtmlTimestamp packet;

        // Failure 1: Bad type
        auto bad1 = createValidPacket(0x01, 0x100, 1000ULL);
        bad1[0] = 0xFF;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(bad1.begin(), bad1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);

        // Failure 2: Bad size
        auto bad2 = createValidPacket(0x02, 0x200, 2000ULL);
        bad2[8] = 0x00;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(bad2.begin(), bad2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Failure 3: Truncated
        std::uint8_t truncated[] = {
            0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);

        // Recovery
        auto valid = createValidPacket(0x04, 0x400, 4000ULL);
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(4000ULL), packet.getTimestamp());
    }

    void testCrossComponentBufferToPacket()
    {
        // Test complete Buffer -> BufferReader -> Packet chain
        std::uint8_t rawData[] = {
            0x09, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0xEF, 0xCD, 0xAB, 0x89, // channel id
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 // timestamp
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(rawData), std::end(rawData));

        PacketTtmlTimestamp packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x89ABCDEF), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), packet.getTimestamp());
    }

    void testCrossComponentMultiplePacketParsing()
    {
        // Test multiple packets through complete component chain
        PacketTtmlTimestamp packet;

        for (int i = 0; i < 10; ++i) {
            auto packetData = createValidPacket(0x10 * i, 0x20 * i, 1000000ULL * i);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10 * i), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1000000ULL * i), packet.getTimestamp());
        }
    }

    void testCrossComponentTimestampExtraction()
    {
        // Test timestamp extraction across Buffer -> BufferReader -> Packet chain
        std::vector<std::uint64_t> timestamps = {
            0ULL,
            1ULL,
            0xFFULL,
            0xFFFFULL,
            0xFFFFFFFFULL,
            0xFFFFFFFFFFFFULL,
            0xFFFFFFFFFFFFFFFFULL,
            0x0102030405060708ULL,
            0x8877665544332211ULL,
            0xAAAAAAAAAAAAAAAAULL
        };

        for (const auto& ts : timestamps) {
            auto packetData = createValidPacket(0x01, 0x02, ts);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            PacketTtmlTimestamp packet;
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(ts, packet.getTimestamp());
        }
    }

    void testCrossComponentLargePacketStream()
    {
        // Simulate large stream of packets across all components
        PacketTtmlTimestamp packet;

        for (int i = 0; i < 200; ++i) {
            std::uint32_t channelId = 0x1000 + (i % 16);
            std::uint32_t counter = 0x2000 + i;
            std::uint64_t timestamp = 1638360000000ULL + (i * 33); // 33ms increments

            auto packetData = createValidPacket(channelId, counter, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(channelId, packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(counter, packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testRealWorldStreamingScenario()
    {
        // Simulate real-world streaming subtitle timestamp scenario
        PacketTtmlTimestamp packet;
        std::uint64_t baseTimestamp = 1638360000000ULL; // Start time

        // Simulate 30 seconds of subtitles at 25fps (750 frames)
        for (int frame = 0; frame < 750; ++frame) {
            std::uint64_t timestamp = baseTimestamp + (frame * 40); // 40ms per frame
            auto packetData = createValidPacket(0x01, frame, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testRealWorldMultipleChannels()
    {
        // Simulate multiple subtitle channels with independent timestamps
        std::vector<PacketTtmlTimestamp> channels(4);

        std::vector<std::pair<std::uint32_t, std::uint64_t>> channelData = {
            {0x01, 1638360000000ULL},  // Channel 1
            {0x02, 1638360005000ULL},  // Channel 2 (5s offset)
            {0x03, 1638360010000ULL},  // Channel 3 (10s offset)
            {0x04, 1638360015000ULL}   // Channel 4 (15s offset)
        };

        for (size_t i = 0; i < channelData.size(); ++i) {
            auto packetData = createValidPacket(
                channelData[i].first,
                0x1000 + i,
                channelData[i].second
            );

            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(channels[i].parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(channels[i].isValid() == true);
            CPPUNIT_ASSERT_EQUAL(channelData[i].first, channels[i].getChannelId());
            CPPUNIT_ASSERT_EQUAL(channelData[i].second, channels[i].getTimestamp());
        }
    }

    void testRealWorldTimestampProgression()
    {
        // Test realistic timestamp progression with various frame rates
        PacketTtmlTimestamp packet;

        // Test different frame rates
        std::vector<std::pair<int, int>> frameRates = {
            {24, 41},  // 24fps: ~41.67ms per frame
            {25, 40},  // 25fps: 40ms per frame
            {30, 33},  // 30fps: ~33.33ms per frame
            {50, 20},  // 50fps: 20ms per frame
            {60, 16}   // 60fps: ~16.67ms per frame
        };

        for (const auto& fps : frameRates) {
            std::uint64_t baseTime = 1638360000000ULL;

            for (int frame = 0; frame < 100; ++frame) {
                std::uint64_t timestamp = baseTime + (frame * fps.second);
                auto packetData = createValidPacket(0x01, frame, timestamp);
                DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

                CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
                CPPUNIT_ASSERT(packet.isValid() == true);
                CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
            }
        }
    }

    void testRealWorldErrorHandlingScenario()
    {
        // Simulate real-world error scenarios and recovery
        PacketTtmlTimestamp packet;

        // Scenario 1: Valid packet
        auto valid1 = createValidPacket(0x01, 0x100, 1638360000000ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Scenario 2: Corrupted packet (network error simulation)
        auto corrupted = createValidPacket(0x01, 0x200, 1638360001000ULL);
        corrupted[10] = 0xFF; // Corrupt channel id byte
        corrupted[15] = 0xAA; // Corrupt timestamp byte
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(corrupted.begin(), corrupted.end());
        // Should parse successfully despite corruption (data still structurally valid)
        bool parseResult2 = packet.parse(std::move(buffer2));

        // Scenario 3: Truncated packet (incomplete transmission)
        std::uint8_t truncated[] = {
            0x09, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x01, 0x02
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Scenario 4: Recovery with valid packet
        auto valid2 = createValidPacket(0x01, 0x300, 1638360002000ULL);
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360002000ULL), packet.getTimestamp());
    }

    void testRealWorldHighFrequencyUpdates()
    {
        // Test high-frequency timestamp updates (every millisecond)
        PacketTtmlTimestamp packet;
        std::uint64_t baseTimestamp = 1638360000000ULL;

        // 1000 packets, one per millisecond
        for (int i = 0; i < 1000; ++i) {
            std::uint64_t timestamp = baseTimestamp + i;
            auto packetData = createValidPacket(0x01, i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testRealWorldVariousTimestampRanges()
    {
        // Test various realistic timestamp ranges
        PacketTtmlTimestamp packet;

        std::vector<std::uint64_t> timestampRanges = {
            0ULL,                      // Zero/start
            1000ULL,                   // 1 second in ms
            60000ULL,                  // 1 minute in ms
            3600000ULL,                // 1 hour in ms
            86400000ULL,               // 1 day in ms
            1638360000000ULL,          // Recent timestamp (Dec 2021)
            1700000000000ULL,          // Future timestamp (Nov 2023)
            0x7FFFFFFFFFFFFFFFULL,     // Max signed int64
            0xFFFFFFFFFFFFFFFFULL      // Max uint64
        };

        for (const auto& ts : timestampRanges) {
            auto packetData = createValidPacket(0x01, 0x02, ts);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(ts, packet.getTimestamp());
        }
    }

    void testRealWorldPTSWraparound()
    {
        // Test PTS (Presentation Time Stamp) wraparound scenario
        PacketTtmlTimestamp packet;

        // Simulate timestamps near max value
        std::uint64_t nearMax = 0xFFFFFFFFFFFFF000ULL;

        for (int i = 0; i < 20; ++i) {
            std::uint64_t timestamp = nearMax + (i * 256);
            auto packetData = createValidPacket(0x01, i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    void testRealWorldConcurrentChannels()
    {
        // Test concurrent subtitle channels with interleaved timestamps
        std::vector<PacketTtmlTimestamp> channels(3);
        std::uint64_t baseTime = 1638360000000ULL;

        // Interleave packets from 3 channels
        for (int round = 0; round < 10; ++round) {
            for (int ch = 0; ch < 3; ++ch) {
                std::uint32_t channelId = 0x01 + ch;
                std::uint32_t counter = (round * 3) + ch;
                std::uint64_t timestamp = baseTime + (counter * 100); // 100ms increments

                auto packetData = createValidPacket(channelId, counter, timestamp);
                DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

                CPPUNIT_ASSERT(channels[ch].parse(std::move(buffer)) == true);
                CPPUNIT_ASSERT(channels[ch].isValid() == true);
                CPPUNIT_ASSERT_EQUAL(channelId, channels[ch].getChannelId());
                CPPUNIT_ASSERT_EQUAL(timestamp, channels[ch].getTimestamp());
            }
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTtmlTimestampTest );
