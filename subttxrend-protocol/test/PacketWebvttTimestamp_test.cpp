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

#include "PacketWebvttTimestamp.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketWebvttTimestamp;

class PacketWebvttTimestampTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketWebvttTimestampTest );
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testConstructorTypeIdentity);
    CPPUNIT_TEST(testConstructorTimestampZero);
    CPPUNIT_TEST(testConstructorCounterZero);
    CPPUNIT_TEST(testConstructorSizeZero);
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testParseTimestampZero);
    CPPUNIT_TEST(testParseSmallTimestamp);
    CPPUNIT_TEST(testParseMediumTimestamp);
    CPPUNIT_TEST(testParseMaxTimestamp);
    CPPUNIT_TEST(testParseTimestampOne);
    CPPUNIT_TEST(testParseMaxSigned64);
    CPPUNIT_TEST(testParseSpecificPattern);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadTypeZero);
    CPPUNIT_TEST(testBadTypeRandom);
    CPPUNIT_TEST(testBadTypeWebvttData);
    CPPUNIT_TEST(testBadTypeWebvttSelection);
    CPPUNIT_TEST(testBadTypeTtmlTimestamp);
    CPPUNIT_TEST(testBadSizeZero);
    CPPUNIT_TEST(testBadSizeOffByOneLess);
    CPPUNIT_TEST(testBadSizeOffByOneMore);
    CPPUNIT_TEST(testBadSizeOnlyChannelId);
    CPPUNIT_TEST(testBadSizeChannelIdPartialTimestamp);
    CPPUNIT_TEST(testBadSizeExcessive);
    CPPUNIT_TEST(testBadSizeMaxValue);
    CPPUNIT_TEST(testParseEmptyBuffer);
    CPPUNIT_TEST(testParseHeaderOnly);
    CPPUNIT_TEST(testParseTruncatedAfterChannelId);
    CPPUNIT_TEST(testParsePartialTimestamp1Byte);
    CPPUNIT_TEST(testParsePartialTimestamp4Bytes);
    CPPUNIT_TEST(testParsePartialTimestamp7Bytes);
    CPPUNIT_TEST(testParse23Bytes);
    CPPUNIT_TEST(testParse1Byte);
    CPPUNIT_TEST(testLittleEndianByteOrder);
    CPPUNIT_TEST(testLittleEndianAlternatingPattern);
    CPPUNIT_TEST(testLittleEndianHighByte);
    CPPUNIT_TEST(testLittleEndianHighOrderByte);
    CPPUNIT_TEST(testPacketReuseTwice);
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
    CPPUNIT_TEST(testBaseClassChannelIdBoundary);
    CPPUNIT_TEST(testBaseClassCounterBoundary);
    CPPUNIT_TEST(testPolymorphismAsPacket);
    CPPUNIT_TEST(testPolymorphismAsPacketChannelSpecific);
    CPPUNIT_TEST(testPolymorphismTypeIdentity);
    CPPUNIT_TEST(testPolymorphismParseVirtual);
    CPPUNIT_TEST(testPolymorphismIsValidVirtual);
    CPPUNIT_TEST(testBufferReaderIntegrationComplete);
    CPPUNIT_TEST(testBufferReaderIntegrationPartial);
    CPPUNIT_TEST(testBufferReaderIntegrationMultipleCalls);
    CPPUNIT_TEST(testBufferReaderIntegrationTimestampExtraction);
    CPPUNIT_TEST(testErrorRecoveryAfterBadType);
    CPPUNIT_TEST(testErrorRecoveryAfterBadSize);
    CPPUNIT_TEST(testErrorRecoveryAfterTruncation);
    CPPUNIT_TEST(testErrorRecoveryMultipleFailures);
    CPPUNIT_TEST(testRealWorldStreamingScenario);
    CPPUNIT_TEST(testRealWorldMultipleChannels);
    CPPUNIT_TEST(testRealWorldTimestampProgression);
    CPPUNIT_TEST(testRealWorldHighFrequencyUpdates);
    CPPUNIT_TEST(testRealWorldPTSWraparound);
    CPPUNIT_TEST(testRealWorldVariousTimestampRanges);
    CPPUNIT_TEST(testRealWorldErrorHandling);
    CPPUNIT_TEST(testExtraDataAfterPacket);
    CPPUNIT_TEST(testCorruptedHeaderType);
    CPPUNIT_TEST(testCorruptedSize);
    CPPUNIT_TEST(testStateConsistencyAcrossParsing);
    CPPUNIT_TEST(testStateConsistencyAfterFailures);
    CPPUNIT_TEST(testStateConsistencyTimestampProgression);
    CPPUNIT_TEST(testStateConsistencyHierarchicalFields);
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
            0x11, 0x00, 0x00, 0x00, // type (WEBVTT_TIMESTAMP = 17)
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

    // Constructor initializes packet as invalid
    void testConstructorInitialState()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Constructor sets correct packet type
    void testConstructorTypeIdentity()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_TIMESTAMP);
    }

    // Constructor initializes timestamp to zero
    void testConstructorTimestampZero()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    // Constructor initializes counter to zero
    void testConstructorCounterZero()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());
    }

    // Constructor initializes size to zero
    void testConstructorSizeZero()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    // Parse valid packet with typical timestamp
    void testGood()
    {
        auto packetData = createValidPacket();

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01020304), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360000000ULL), packet.getTimestamp());
    }

    // Parse with timestamp = 0
    void testParseTimestampZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    // Parse with small timestamp (1000ms)
    void testParseSmallTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1000), packet.getTimestamp());
    }

    // Parse with medium timestamp (1638360000000)
    void testParseMediumTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1638360000000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360000000ULL), packet.getTimestamp());
    }

    // Parse with maximum uint64 timestamp
    void testParseMaxTimestamp()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFFFFFFFFFFULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFFFFFFFFFFFFFFFFULL), packet.getTimestamp());
    }

    // Parse with timestamp = 1
    void testParseTimestampOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1), packet.getTimestamp());
    }

    // Parse with max signed int64
    void testParseMaxSigned64()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x7FFFFFFFFFFFFFFFULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x7FFFFFFFFFFFFFFFULL), packet.getTimestamp());
    }

    // Parse with specific pattern
    void testParseSpecificPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x0102030405060708ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0102030405060708ULL), packet.getTimestamp());
    }

    // Parse packet with wrong type
    void testBadType()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x02; // TIMESTAMP type instead of WEBVTT_TIMESTAMP

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with type = 0
    void testBadTypeZero()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x00; // Invalid type

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with type = 0xFF
    void testBadTypeRandom()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0xFF; // Random invalid type

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with WEBVTT_DATA type
    void testBadTypeWebvttData()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x10; // WEBVTT_DATA type (16)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with WEBVTT_SELECTION type
    void testBadTypeWebvttSelection()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x0F; // WEBVTT_SELECTION type (15)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with TTML_TIMESTAMP type
    void testBadTypeTtmlTimestamp()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x09; // TTML_TIMESTAMP type (9)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 0
    void testBadSizeZero()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 11 (one byte short)
    void testBadSizeOffByOneLess()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0B; // size = 11
        packetData.pop_back(); // Remove one byte

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 13 (one byte extra)
    void testBadSizeOffByOneMore()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0D; // size = 13
        packetData.push_back(0xFF); // Add extra byte

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 4 (only channelId)
    void testBadSizeOnlyChannelId()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x04; // size = 4 (only channelId, missing timestamp)
        packetData.resize(16); // header(12) + channelId(4)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 8 (channelId + partial timestamp)
    void testBadSizeChannelIdPartialTimestamp()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x08; // size = 8 (channelId + 4 bytes of timestamp)
        packetData.resize(20); // header(12) + channelId(4) + partial timestamp(4)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 100 (excessive)
    void testBadSizeExcessive()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x64; // size = 100
        for (int i = 0; i < 88; ++i) {
            packetData.push_back(0xFF);
        }

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse packet with size = 0xFFFFFFFF
    void testBadSizeMaxValue()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0xFF, 0xFF, 0xFF, 0xFF, // size (max value)
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse empty buffer
    void testParseEmptyBuffer()
    {
        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse buffer with only header (12 bytes)
    void testParseHeaderOnly()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse buffer truncated after channelId
    void testParseTruncatedAfterChannelId()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse buffer with 1 byte of timestamp
    void testParsePartialTimestamp1Byte()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, // timestamp partially present (1 byte)
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse buffer with 4 bytes of timestamp
    void testParsePartialTimestamp4Bytes()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, // timestamp partially present (4 bytes)
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse buffer with 7 bytes of timestamp
    void testParsePartialTimestamp7Bytes()
    {
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // timestamp partially present (7 bytes)
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse 23-byte buffer (one byte short)
    void testParse23Bytes()
    {
        auto packetData = createValidPacket();
        packetData.pop_back(); // Remove one byte (23 bytes total instead of 24)

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Parse 1-byte buffer
    void testParse1Byte()
    {
        std::uint8_t packetData[] = { 0x11 };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Verify little-endian extraction
    void testLittleEndianByteOrder()
    {
        // Bytes [0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08] should become 0x0807060504030201
        auto packetData = createValidPacket(0x01, 0x02, 0x0807060504030201ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), packet.getTimestamp());
    }

    // Verify little-endian with alternating pattern
    void testLittleEndianAlternatingPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xAA55AA55AA55AA55ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xAA55AA55AA55AA55ULL), packet.getTimestamp());
    }

    // Verify 0x0000000000000001 != 0x0100000000000000
    void testLittleEndianHighByte()
    {
        // Test that 0x0000000000000001 != 0x0100000000000000
        auto packetData1 = createValidPacket(0x01, 0x02, 0x0000000000000001ULL);
        auto packetData2 = createValidPacket(0x01, 0x02, 0x0100000000000000ULL);

        PacketWebvttTimestamp packet1;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(packetData1.begin(), packetData1.end());
        CPPUNIT_ASSERT(packet1.parse(std::move(buffer1)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0000000000000001ULL), packet1.getTimestamp());

        PacketWebvttTimestamp packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(packetData2.begin(), packetData2.end());
        CPPUNIT_ASSERT(packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0100000000000000ULL), packet2.getTimestamp());

        CPPUNIT_ASSERT(packet1.getTimestamp() != packet2.getTimestamp());
    }

    // Parse with high byte set
    void testLittleEndianHighOrderByte()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFF00000000000000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0xFF00000000000000ULL), packet.getTimestamp());
    }

    // Parse valid packet twice (packet reuse)
    void testPacketReuseTwice()
    {
        PacketWebvttTimestamp packet;

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

    // Parse valid then invalid packet
    void testStateValidToInvalid()
    {
        PacketWebvttTimestamp packet;

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

    // Parse invalid then valid packet
    void testStateInvalidToValid()
    {
        PacketWebvttTimestamp packet;

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

    // Parse multiple valid packets sequentially
    void testStateMultipleValidParses()
    {
        PacketWebvttTimestamp packet;

        for (int i = 0; i < 5; ++i) {
            std::uint64_t timestamp = 1000ULL * (i + 1);
            auto packetData = createValidPacket(0x01, 0x10 + i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    // Verify getTimestamp() before parsing
    void testGetterBeforeParse()
    {
        PacketWebvttTimestamp packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Verify getTimestamp() after failed parse
    void testGetterAfterFailedParse()
    {
        PacketWebvttTimestamp packet;

        auto invalidData = createValidPacket();
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr buffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));

        // After failed parse, getter should return initial value
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), packet.getTimestamp());
    }

    // Call getTimestamp() multiple times
    void testGetterMultipleCalls()
    {
        auto packetData = createValidPacket(0x01, 0x02, 123456789ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // Call getter multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(123456789ULL), packet.getTimestamp());
        }
    }

    // Verify getChannelId() with various values
    void testBaseClassChannelId()
    {
        auto packetData = createValidPacket(0xDEADBEEF, 0x02, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
    }

    // Verify getCounter() with various values
    void testBaseClassCounter()
    {
        auto packetData = createValidPacket(0x01, 0xCAFEBABE, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
    }

    // Verify getSize() returns 12 for valid packet
    void testBaseClassSize()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    // Test base class parse failure
    void testBaseClassParseFailure()
    {
        // Create packet with valid header but truncated channel ID
        std::uint8_t packetData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, // channel id truncated
        };

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Verify channelId boundary values
    void testBaseClassChannelIdBoundary()
    {
        auto packetData = createValidPacket(0xFFFFFFFF, 0x02, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getChannelId());
    }

    // Verify counter boundary values
    void testBaseClassCounterBoundary()
    {
        auto packetData = createValidPacket(0x01, 0xFFFFFFFF, 1000ULL);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());
    }

    // Access via Packet* base pointer
    void testPolymorphismAsPacket()
    {
        auto packetData = createValidPacket(0x01, 0x02, 123456789ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttTimestamp concretePacket;
        Packet* basePtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), basePtr->getCounter());
    }

    // Access via PacketChannelSpecific* pointer
    void testPolymorphismAsPacketChannelSpecific()
    {
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 987654321ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttTimestamp concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(channelPtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(channelPtr->isValid() == true);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), channelPtr->getChannelId());
    }

    // Verify type identity through polymorphism
    void testPolymorphismTypeIdentity()
    {
        PacketWebvttTimestamp concretePacket;
        Packet* basePtr = &concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT(concretePacket.getType() == Packet::Type::WEBVTT_TIMESTAMP);
    }

    // Call parse() through base pointer
    void testPolymorphismParseVirtual()
    {
        PacketWebvttTimestamp concretePacket;
        Packet* basePtr = &concretePacket;

        auto validData = createValidPacket(0x01, 0x02, 1280ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer1)) == true);

        auto invalidData = createValidPacket(0x01, 0x02, 1920ULL);
        invalidData[0] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer2)) == false);
    }

    // Call isValid() through base pointer
    void testPolymorphismIsValidVirtual()
    {
        PacketWebvttTimestamp concretePacket;
        Packet* basePtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->isValid() == false);

        auto packetData = createValidPacket(0x01, 0x02, 1920ULL);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
        basePtr->parse(std::move(buffer));

        CPPUNIT_ASSERT(basePtr->isValid() == true);
    }

    // Complete Buffer→BufferReader→Packet chain
    void testBufferReaderIntegrationComplete()
    {
        std::uint8_t rawData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0xEF, 0xCD, 0xAB, 0x89, // channel id
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 // timestamp
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(rawData), std::end(rawData));

        PacketWebvttTimestamp packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x89ABCDEF), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0x0807060504030201ULL), packet.getTimestamp());
    }

    // BufferReader handling of insufficient data
    void testBufferReaderIntegrationPartial()
    {
        std::uint8_t partialData[] = {
            0x11, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x11, 0x22, 0x33 // partial timestamp (3 bytes)
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(partialData), std::end(partialData));

        PacketWebvttTimestamp packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    // BufferReader across multiple parse calls
    void testBufferReaderIntegrationMultipleCalls()
    {
        PacketWebvttTimestamp packet;

        for (int i = 0; i < 20; ++i) {
            std::uint64_t timestamp = 1000000ULL * i;
            auto packetData = createValidPacket(0x10 + i, 0x20 + i, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }

    // BufferReader uint64 extraction with various patterns
    void testBufferReaderIntegrationTimestampExtraction()
    {
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

            PacketWebvttTimestamp packet;
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(ts, packet.getTimestamp());
        }
    }

    // Recovery after bad type error
    void testErrorRecoveryAfterBadType()
    {
        PacketWebvttTimestamp packet;

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

    // Recovery after bad size error
    void testErrorRecoveryAfterBadSize()
    {
        PacketWebvttTimestamp packet;

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

    // Recovery after truncation error
    void testErrorRecoveryAfterTruncation()
    {
        PacketWebvttTimestamp packet;

        // Truncated packet
        std::uint8_t truncated[] = {
            0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
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

    // Recovery after multiple consecutive failures
    void testErrorRecoveryMultipleFailures()
    {
        PacketWebvttTimestamp packet;

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
            0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
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

    // Simulate subtitle streaming at 25fps
    void testRealWorldStreamingScenario()
    {
        PacketWebvttTimestamp packet;
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

    // Multiple channels with independent timestamps
    void testRealWorldMultipleChannels()
    {
        std::vector<PacketWebvttTimestamp> channels(4);

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

    // Timestamp progression at various frame rates
    void testRealWorldTimestampProgression()
    {
        PacketWebvttTimestamp packet;

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

    // High-frequency updates (1000 packets/second)
    void testRealWorldHighFrequencyUpdates()
    {
        PacketWebvttTimestamp packet;
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

    // PTS wraparound near max uint64
    void testRealWorldPTSWraparound()
    {
        PacketWebvttTimestamp packet;

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

    // Various realistic timestamp ranges
    void testRealWorldVariousTimestampRanges()
    {
        PacketWebvttTimestamp packet;

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

    // Network error simulation (corrupted packets)
    void testRealWorldErrorHandling()
    {
        PacketWebvttTimestamp packet;

        // Scenario 1: Valid packet
        auto valid1 = createValidPacket(0x01, 0x100, 1638360000000ULL);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Scenario 2: Truncated packet (incomplete transmission)
        std::uint8_t truncated[] = {
            0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x01, 0x02
        };
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Scenario 3: Recovery with valid packet
        auto valid2 = createValidPacket(0x01, 0x300, 1638360002000ULL);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(1638360002000ULL), packet.getTimestamp());
    }

    // Parse packet with extra data after valid content
    void testExtraDataAfterPacket()
    {
        auto packetData = createValidPacket();
        packetData.push_back(0xFF);
        packetData.push_back(0xFE);
        packetData.push_back(0xFD);

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Should fail because there's extra data
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Corrupted type field (non-zero high bytes)
    void testCorruptedHeaderType()
    {
        auto packetData = createValidPacket();
        packetData[1] = 0xFF; // Corrupt type field

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Corrupted size field (high bytes set)
    void testCorruptedSize()
    {
        auto packetData = createValidPacket();
        packetData[9] = 0xFF; // Corrupt size field

        PacketWebvttTimestamp packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Verify all fields consistent after parsing
    void testStateConsistencyAcrossParsing()
    {
        PacketWebvttTimestamp packet;

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
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_TIMESTAMP);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        }
    }

    // State consistency after parse failures
    void testStateConsistencyAfterFailures()
    {
        PacketWebvttTimestamp packet;

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

    // State consistency with progressing timestamps
    void testStateConsistencyTimestampProgression()
    {
        PacketWebvttTimestamp packet;
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

    // Base class and derived class fields updated together
    void testStateConsistencyHierarchicalFields()
    {
        PacketWebvttTimestamp packet;

        for (int i = 0; i < 10; ++i) {
            std::uint32_t channelId = 0x100 + i;
            std::uint32_t counter = 0x200 + i;
            std::uint64_t timestamp = 1000000ULL * i;

            auto packetData = createValidPacket(channelId, counter, timestamp);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT_EQUAL(channelId, packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(counter, packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(timestamp, packet.getTimestamp());
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketWebvttTimestampTest );
