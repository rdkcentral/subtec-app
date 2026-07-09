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

#include "PacketWebvttSelection.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketWebvttSelection;

class PacketWebvttSelectionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketWebvttSelectionTest );
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testConstructorTypeIdentity);
    CPPUNIT_TEST(testConstructorVideoDimensionsZero);
    CPPUNIT_TEST(testConstructorCounterZero);
    CPPUNIT_TEST(testConstructorSizeZero);
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testParseVideoDimensionsZero);
    CPPUNIT_TEST(testParseVideoDimensionsMax);
    CPPUNIT_TEST(testParseHDResolution);
    CPPUNIT_TEST(testParseFullHDResolution);
    CPPUNIT_TEST(testParse4KResolution);
    CPPUNIT_TEST(testParseSDResolution);
    CPPUNIT_TEST(testParseUltraWideResolution);
    CPPUNIT_TEST(testParseDimensionsOne);
    CPPUNIT_TEST(testParseDimensionsMaxMinusOne);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadTypeZero);
    CPPUNIT_TEST(testBadTypeRandom);
    CPPUNIT_TEST(testBadTypeWebvttData);
    CPPUNIT_TEST(testBadTypeWebvttTimestamp);
    CPPUNIT_TEST(testBadSizeZero);
    CPPUNIT_TEST(testBadSizeOffByOneLess);
    CPPUNIT_TEST(testBadSizeOffByOneMore);
    CPPUNIT_TEST(testBadSizeOnlyChannelId);
    CPPUNIT_TEST(testBadSizeChannelIdAndWidth);
    CPPUNIT_TEST(testBadSizeExcessive);
    CPPUNIT_TEST(testBadSizeMaxValue);
    CPPUNIT_TEST(testParseEmptyBuffer);
    CPPUNIT_TEST(testParseHeaderOnly);
    CPPUNIT_TEST(testParseTruncatedAfterChannelId);
    CPPUNIT_TEST(testParseTruncatedAfterWidth);
    CPPUNIT_TEST(testParsePartialWidth);
    CPPUNIT_TEST(testParsePartialHeight);
    CPPUNIT_TEST(testParse23Bytes);
    CPPUNIT_TEST(testParse1Byte);
    CPPUNIT_TEST(testLittleEndianWidthExtraction);
    CPPUNIT_TEST(testLittleEndianHeightExtraction);
    CPPUNIT_TEST(testLittleEndianWidth32Bits);
    CPPUNIT_TEST(testLittleEndianHeight32Bits);
    CPPUNIT_TEST(testLittleEndianWidthAlternatingPattern);
    CPPUNIT_TEST(testLittleEndianHeightAlternatingPattern);
    CPPUNIT_TEST(testStateValidToValid);
    CPPUNIT_TEST(testStateValidToInvalid);
    CPPUNIT_TEST(testStateInvalidToValid);
    CPPUNIT_TEST(testStateMultipleValidParses);
    CPPUNIT_TEST(testStateAlternatingValidInvalid);
    CPPUNIT_TEST(testGettersBeforeParse);
    CPPUNIT_TEST(testGettersAfterFailedParse);
    CPPUNIT_TEST(testGetWidthMultipleCalls);
    CPPUNIT_TEST(testGetHeightMultipleCalls);
    CPPUNIT_TEST(testBaseClassChannelId);
    CPPUNIT_TEST(testBaseClassCounter);
    CPPUNIT_TEST(testBaseClassSize);
    CPPUNIT_TEST(testBaseClassParseFailure);
    CPPUNIT_TEST(testBaseClassChannelIdBoundary);
    CPPUNIT_TEST(testBaseClassCounterBoundary);
    CPPUNIT_TEST(testBoundaryWidthZeroHeightMax);
    CPPUNIT_TEST(testBoundaryWidthMaxHeightZero);
    CPPUNIT_TEST(testBoundaryWidthOneHeightMax);
    CPPUNIT_TEST(testBoundaryWidthMaxHeightOne);
    CPPUNIT_TEST(testBoundarySignedUnsignedBoundary);
    CPPUNIT_TEST(testBoundaryMaxSignedPositive);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testPacketReuseSustained);
    CPPUNIT_TEST(testPacketReuseSameData);
    CPPUNIT_TEST(testPacketReuseLargeToSmall);
    CPPUNIT_TEST(testCorruptedHeaderType);
    CPPUNIT_TEST(testCorruptedSize);
    CPPUNIT_TEST(testExtraDataAfterPacket);
    CPPUNIT_TEST(testGarbageInWidthField);
    CPPUNIT_TEST(testGarbageInHeightField);
    CPPUNIT_TEST(testEndToEndWorkflowBasic);
    CPPUNIT_TEST(testEndToEndWorkflowMultipleResolutions);
    CPPUNIT_TEST(testEndToEndWorkflowBoundaryValues);
    CPPUNIT_TEST(testEndToEndWorkflowInvalidPackets);
    CPPUNIT_TEST(testEndToEndWorkflowSequentialParsing);
    CPPUNIT_TEST(testBufferReaderIntegrationHeaderExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationPayloadExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationPartialData);
    CPPUNIT_TEST(testBufferReaderIntegrationMultipleCalls);
    CPPUNIT_TEST(testBufferReaderIntegrationLargeStream);
    CPPUNIT_TEST(testPolymorphismAsPacket);
    CPPUNIT_TEST(testPolymorphismAsPacketChannelSpecific);
    CPPUNIT_TEST(testPolymorphismParseVirtual);
    CPPUNIT_TEST(testPolymorphismTypeIdentity);
    CPPUNIT_TEST(testPolymorphismIsValidVirtual);
    CPPUNIT_TEST(testStateConsistencyAcrossParsing);
    CPPUNIT_TEST(testStateConsistencyAfterFailures);
    CPPUNIT_TEST(testStateConsistencyDimensionChanges);
    CPPUNIT_TEST(testStateConsistencyGettersSnapshot);
    CPPUNIT_TEST(testStateConsistencyHierarchicalFields);
    CPPUNIT_TEST(testErrorRecoveryAfterBadType);
    CPPUNIT_TEST(testErrorRecoveryAfterBadSize);
    CPPUNIT_TEST(testErrorRecoveryAfterTruncation);
    CPPUNIT_TEST(testErrorRecoveryMultipleFailures);
    CPPUNIT_TEST(testErrorRecoverySustained);
    CPPUNIT_TEST(testCrossComponentBufferToPacket);
    CPPUNIT_TEST(testCrossComponentMultiplePackets);
    CPPUNIT_TEST(testCrossComponentDimensionExtraction);
    CPPUNIT_TEST(testCrossComponentLargeStream);
    CPPUNIT_TEST(testCrossComponentMixedPacketTypes);
    CPPUNIT_TEST(testRealWorldVideoFormatChange);
    CPPUNIT_TEST(testRealWorldMultipleChannels);
    CPPUNIT_TEST(testRealWorldAspectRatioScenarios);
    CPPUNIT_TEST(testRealWorldErrorHandling);
    CPPUNIT_TEST(testRealWorldAdaptiveBitrate);
    CPPUNIT_TEST(testRealWorldDVBSubtitleStream);
    CPPUNIT_TEST(testRealWorldMobileRotation);
    CPPUNIT_TEST(testRealWorldHighFrequencyUpdates);
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
        std::uint32_t width = 1920,
        std::uint32_t height = 1080)
    {
        std::vector<std::uint8_t> packet = {
            0x0F, 0x00, 0x00, 0x00, // type (WEBVTT_SELECTION = 15)
            static_cast<std::uint8_t>(counter & 0xFF),
            static_cast<std::uint8_t>((counter >> 8) & 0xFF),
            static_cast<std::uint8_t>((counter >> 16) & 0xFF),
            static_cast<std::uint8_t>((counter >> 24) & 0xFF),
            0x0C, 0x00, 0x00, 0x00, // size (12 = 4 + 4 + 4)
            static_cast<std::uint8_t>(channelId & 0xFF),
            static_cast<std::uint8_t>((channelId >> 8) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 16) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 24) & 0xFF),
            static_cast<std::uint8_t>(width & 0xFF),
            static_cast<std::uint8_t>((width >> 8) & 0xFF),
            static_cast<std::uint8_t>((width >> 16) & 0xFF),
            static_cast<std::uint8_t>((width >> 24) & 0xFF),
            static_cast<std::uint8_t>(height & 0xFF),
            static_cast<std::uint8_t>((height >> 8) & 0xFF),
            static_cast<std::uint8_t>((height >> 16) & 0xFF),
            static_cast<std::uint8_t>((height >> 24) & 0xFF),
        };
        return packet;
    }

    void testConstructorInitialState()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testConstructorTypeIdentity()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
    }

    void testConstructorVideoDimensionsZero()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testConstructorCounterZero()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());
    }

    void testConstructorSizeZero()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testGood()
    {
        auto packetData = createValidPacket();

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01020304), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParseVideoDimensionsZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0, 0);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testParseVideoDimensionsMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 0xFFFFFFFF);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testParseHDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1280, 720);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testParseFullHDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParse4KResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 3840, 2160);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testParseSDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 640, 480);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());
    }

    void testParseUltraWideResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 2560, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2560), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParseDimensionsOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1, 1);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoHeight());
    }

    void testParseDimensionsMaxMinusOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFE, 0xFFFFFFFE);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFE), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFE), packet.getRelatedVideoHeight());
    }

    void testBadType()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x02; // TIMESTAMP type instead of WEBVTT_SELECTION

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeZero()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x00; // Invalid type

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeRandom()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0xFF; // Random invalid type

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeWebvttData()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x10; // WEBVTT_DATA type (16)

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadTypeWebvttTimestamp()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x11; // WEBVTT_TIMESTAMP type (17)

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeZero()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeOffByOneLess()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0B; // size = 11 (one byte short)
        packetData.pop_back();

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeOffByOneMore()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0D; // size = 13 (one byte extra)
        packetData.push_back(0xFF);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeOnlyChannelId()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x04; // size = 4 (only channelId)
        packetData.resize(16); // header(12) + channelId(4)

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeChannelIdAndWidth()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x08; // size = 8 (channelId + width only)
        packetData.resize(20); // header(12) + channelId(4) + width(4)

        PacketWebvttSelection packet;
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

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeMaxValue()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0xFF, 0xFF, 0xFF, 0xFF, // size (max value)
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseEmptyBuffer()
    {
        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseHeaderOnly()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterChannelId()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterWidth()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, 0x00, 0x00, // width (1920)
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParsePartialWidth()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, // width partial (2 bytes)
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParsePartialHeight()
    {
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, 0x00, 0x00, // width
            0x01, 0x02, 0x03, // height partial (3 bytes)
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParse23Bytes()
    {
        auto packetData = createValidPacket();
        packetData.pop_back(); // Remove one byte (23 bytes total instead of 24)

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParse1Byte()
    {
        std::uint8_t packetData[] = { 0x0F };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testLittleEndianWidthExtraction()
    {
        // Bytes [0x01,0x02,0x03,0x04] should become 0x04030201
        auto packetData = createValidPacket(0x01, 0x02, 0x04030201, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getRelatedVideoWidth());
    }

    void testLittleEndianHeightExtraction()
    {
        // Bytes [0x05,0x06,0x07,0x08] should become 0x08070605
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0x08070605);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), packet.getRelatedVideoHeight());
    }

    void testLittleEndianWidth32Bits()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x12345678, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getRelatedVideoWidth());
    }

    void testLittleEndianHeight32Bits()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0x87654321);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x87654321), packet.getRelatedVideoHeight());
    }

    void testLittleEndianWidthAlternatingPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xAAAAAAAA, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAAAAAAAA), packet.getRelatedVideoWidth());
    }

    void testLittleEndianHeightAlternatingPattern()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0x55555555);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55555555), packet.getRelatedVideoHeight());
    }

    void testStateValidToValid()
    {
        PacketWebvttSelection packet;

        // First parse: valid
        auto data1 = createValidPacket(0x01, 0x10, 1280, 720);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());

        // Second parse: valid with different dimensions
        auto data2 = createValidPacket(0x02, 0x20, 1920, 1080);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testStateValidToInvalid()
    {
        PacketWebvttSelection packet;

        // First parse: valid
        auto validData = createValidPacket(0x01, 0x02, 1920, 1080);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Second parse: invalid
        auto invalidData = createValidPacket(0x01, 0x02, 3840, 2160);
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testStateInvalidToValid()
    {
        PacketWebvttSelection packet;

        // First parse: invalid
        auto invalidData = createValidPacket(0x01, 0x02, 1920, 1080);
        invalidData[8] = 0x00; // Invalid size
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse: valid
        auto validData = createValidPacket(0x05, 0x06, 640, 480);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());
    }

    void testStateMultipleValidParses()
    {
        PacketWebvttSelection packet;

        for (int i = 0; i < 10; ++i) {
            std::uint32_t width = 640 + (i * 128);
            std::uint32_t height = 480 + (i * 72);
            auto packetData = createValidPacket(0x01, 0x10 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testStateAlternatingValidInvalid()
    {
        PacketWebvttSelection packet;

        for (int i = 0; i < 5; ++i) {
            // Valid packet
            auto validData = createValidPacket(0x100 + i, 0x200 + i, 1920, 1080);
            DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x100 + i), packet.getChannelId());

            // Invalid packet
            auto invalidData = createValidPacket(0x300 + i, 0x400 + i, 640, 480);
            invalidData[8] = 0x00; // Size = 0
            DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
            CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testGettersBeforeParse()
    {
        PacketWebvttSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testGettersAfterFailedParse()
    {
        PacketWebvttSelection packet;

        auto invalidData = createValidPacket();
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr buffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));

        // After failed parse, getters should return initial value
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testGetWidthMultipleCalls()
    {
        auto packetData = createValidPacket(0x01, 0x02, 2560, 1440);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // Call getter multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2560), packet.getRelatedVideoWidth());
        }
    }

    void testGetHeightMultipleCalls()
    {
        auto packetData = createValidPacket(0x01, 0x02, 2560, 1440);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // Call getter multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1440), packet.getRelatedVideoHeight());
        }
    }

    void testBaseClassChannelId()
    {
        auto packetData = createValidPacket(0xDEADBEEF, 0x02, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
    }

    void testBaseClassCounter()
    {
        auto packetData = createValidPacket(0x01, 0xCAFEBABE, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
    }

    void testBaseClassSize()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    void testBaseClassParseFailure()
    {
        // Create packet with valid header but truncated channel ID
        std::uint8_t packetData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, // channel id truncated
        };

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBaseClassChannelIdBoundary()
    {
        auto packetData = createValidPacket(0xFFFFFFFF, 0x02, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getChannelId());
    }

    void testBaseClassCounterBoundary()
    {
        auto packetData = createValidPacket(0x01, 0xFFFFFFFF, 1920, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());
    }

    void testBoundaryWidthZeroHeightMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0, 0xFFFFFFFF);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testBoundaryWidthMaxHeightZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 0);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testBoundaryWidthOneHeightMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1, 0xFFFFFFFF);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testBoundaryWidthMaxHeightOne()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 1);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoHeight());
    }

    void testBoundarySignedUnsignedBoundary()
    {
        // Test 0x80000000 (would be negative if signed, but treated as unsigned)
        auto packetData = createValidPacket(0x01, 0x02, 0x80000000, 0x80000000);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x80000000), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x80000000), packet.getRelatedVideoHeight());
    }

    void testBoundaryMaxSignedPositive()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x7FFFFFFF, 0x7FFFFFFF);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x7FFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x7FFFFFFF), packet.getRelatedVideoHeight());
    }

    void testPacketReuse()
    {
        PacketWebvttSelection packet;

        // First parse
        auto data1 = createValidPacket(0x01, 0x10, 640, 480);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());

        // Second parse with different data
        auto data2 = createValidPacket(0x02, 0x20, 1920, 1080);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x20), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testPacketReuseSustained()
    {
        PacketWebvttSelection packet;

        for (int i = 0; i < 100; ++i) {
            std::uint32_t width = 640 + (i * 10);
            std::uint32_t height = 480 + (i * 7);
            auto packetData = createValidPacket(0x100 + i, 0x200 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testPacketReuseSameData()
    {
        PacketWebvttSelection packet;
        auto packetData = createValidPacket(0x01, 0x02, 1280, 720);

        // Parse same data twice
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());

        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testPacketReuseLargeToSmall()
    {
        PacketWebvttSelection packet;

        // Parse large dimensions
        auto data1 = createValidPacket(0x01, 0x10, 0xFFFFFFFF, 0xFFFFFFFF);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());

        // Parse small dimensions
        auto data2 = createValidPacket(0x02, 0x20, 1, 1);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getRelatedVideoHeight());
    }

    void testCorruptedHeaderType()
    {
        auto packetData = createValidPacket();
        packetData[1] = 0xFF; // Corrupt type field

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testCorruptedSize()
    {
        auto packetData = createValidPacket();
        packetData[9] = 0xFF; // Corrupt size field

        PacketWebvttSelection packet;
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

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Should fail because there's extra data
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testGarbageInWidthField()
    {
        // Width with garbage value should be extracted as-is (no validation)
        auto packetData = createValidPacket(0x01, 0x02, 0xDEADBEEF, 1080);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getRelatedVideoWidth());
    }

    void testGarbageInHeightField()
    {
        // Height with garbage value should be extracted as-is (no validation)
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0xCAFEBABE);

        PacketWebvttSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getRelatedVideoHeight());
    }

    void testEndToEndWorkflowBasic()
    {
        // Test complete workflow: Buffer creation → Parsing → Validation → Data extraction
        std::uint32_t expectedChannelId = 0x12345678;
        std::uint32_t expectedCounter = 0xABCDEF01;
        std::uint32_t expectedWidth = 1920;
        std::uint32_t expectedHeight = 1080;

        auto packetData = createValidPacket(expectedChannelId, expectedCounter, expectedWidth, expectedHeight);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection packet;

        // Parse
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Validate
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);

        // Extract and verify all data
        CPPUNIT_ASSERT_EQUAL(expectedChannelId, packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(expectedCounter, packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(expectedWidth, packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(expectedHeight, packet.getRelatedVideoHeight());
    }

    void testEndToEndWorkflowMultipleResolutions()
    {
        // Test workflow with multiple resolution packets in sequence
        PacketWebvttSelection packet;

        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>> testCases = {
            {0x01, 0x100, 640, 480},      // SD
            {0x02, 0x200, 1280, 720},     // HD
            {0x03, 0x300, 1920, 1080},    // Full HD
            {0x04, 0x400, 2560, 1440},    // QHD
            {0x05, 0x500, 3840, 2160}     // 4K
        };

        for (const auto& testCase : testCases) {
            auto packetData = createValidPacket(
                std::get<0>(testCase),
                std::get<1>(testCase),
                std::get<2>(testCase),
                std::get<3>(testCase)
            );
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(testCase), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(testCase), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(testCase), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(std::get<3>(testCase), packet.getRelatedVideoHeight());
        }
    }

    void testEndToEndWorkflowBoundaryValues()
    {
        // Test workflow with boundary value packets
        PacketWebvttSelection packet;

        std::vector<std::pair<std::uint32_t, std::uint32_t>> boundaries = {
            {0, 0},
            {1, 1},
            {0xFFFFFFFF, 0xFFFFFFFF},
            {0, 0xFFFFFFFF},
            {0xFFFFFFFF, 0}
        };

        for (const auto& boundary : boundaries) {
            auto packetData = createValidPacket(0x01, 0x02, boundary.first, boundary.second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(boundary.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(boundary.second, packet.getRelatedVideoHeight());
        }
    }

    void testEndToEndWorkflowInvalidPackets()
    {
        // Test workflow with all invalid packets
        PacketWebvttSelection packet;

        // Bad type
        auto bad1 = createValidPacket(0x01, 0x100, 1920, 1080);
        bad1[0] = 0x00;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(bad1.begin(), bad1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Bad size
        auto bad2 = createValidPacket(0x02, 0x200, 1920, 1080);
        bad2[8] = 0x00;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(bad2.begin(), bad2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Truncated
        std::uint8_t truncated[] = {
            0x0F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testEndToEndWorkflowSequentialParsing()
    {
        // Test sequential parsing of 50 packets
        PacketWebvttSelection packet;

        for (int i = 0; i < 50; ++i) {
            std::uint32_t width = 1280 + (i * 10);
            std::uint32_t height = 720 + (i * 5);
            auto packetData = createValidPacket(0x01, 0x100 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testBufferReaderIntegrationHeaderExtraction()
    {
        // Test BufferReader correctly extracts header fields
        auto packetData = createValidPacket(0xAABBCCDD, 0x11223344, 1920, 1080);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify header extraction
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    void testBufferReaderIntegrationPayloadExtraction()
    {
        // Test BufferReader correctly extracts payload fields
        auto packetData = createValidPacket(0x12345678, 0x87654321, 0xDEADBEEF, 0xCAFEBABE);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify payload extraction
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getRelatedVideoHeight());
    }

    void testBufferReaderIntegrationPartialData()
    {
        // Test BufferReader handling of partial data
        std::uint8_t partialData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, 0x00 // partial width (3 bytes)
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(partialData), std::end(partialData));

        PacketWebvttSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testBufferReaderIntegrationMultipleCalls()
    {
        // Test BufferReader across multiple parse calls
        PacketWebvttSelection packet;

        for (int i = 0; i < 30; ++i) {
            std::uint32_t width = 640 + (i * 64);
            std::uint32_t height = 480 + (i * 36);
            auto packetData = createValidPacket(0x10 + i, 0x20 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testBufferReaderIntegrationLargeStream()
    {
        // Test BufferReader with large packet stream (150+ packets)
        PacketWebvttSelection packet;

        for (int i = 0; i < 150; ++i) {
            std::uint32_t width = 1920 + (i % 100);
            std::uint32_t height = 1080 + (i % 50);
            auto packetData = createValidPacket(0x1000 + i, 0x2000 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testPolymorphismAsPacket()
    {
        // Test polymorphic access through Packet base class pointer
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection concretePacket;
        Packet* basePtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), basePtr->getCounter());
    }

    void testPolymorphismAsPacketChannelSpecific()
    {
        // Test polymorphic access through PacketChannelSpecific base class pointer
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 3840, 2160);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(channelPtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(channelPtr->isValid() == true);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), channelPtr->getChannelId());
    }

    void testPolymorphismParseVirtual()
    {
        // Test virtual parse method through base pointer
        PacketWebvttSelection concretePacket;
        Packet* basePtr = &concretePacket;

        auto validData = createValidPacket(0x01, 0x02, 1280, 720);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer1)) == true);

        auto invalidData = createValidPacket(0x01, 0x02, 1920, 1080);
        invalidData[0] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer2)) == false);
    }

    void testPolymorphismTypeIdentity()
    {
        // Verify type identity is maintained through polymorphism
        PacketWebvttSelection concretePacket;
        Packet* basePtr = &concretePacket;
        PacketChannelSpecific* channelPtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT(concretePacket.getType() == Packet::Type::WEBVTT_SELECTION);
    }

    void testPolymorphismIsValidVirtual()
    {
        // Test isValid() through polymorphic interface
        PacketWebvttSelection concretePacket;
        Packet* basePtr = &concretePacket;

        CPPUNIT_ASSERT(basePtr->isValid() == false);

        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
        basePtr->parse(std::move(buffer));

        CPPUNIT_ASSERT(basePtr->isValid() == true);
    }

    void testStateConsistencyAcrossParsing()
    {
        // Test state remains consistent through multiple parsing operations
        PacketWebvttSelection packet;

        std::vector<std::pair<std::uint32_t, std::uint32_t>> dimensions = {
            {640, 480}, {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
        };

        for (size_t i = 0; i < dimensions.size(); ++i) {
            auto packetData = createValidPacket(0x10 + i, 0x20 + i, dimensions[i].first, dimensions[i].second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10 + i), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(dimensions[i].first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(dimensions[i].second, packet.getRelatedVideoHeight());

            // Verify state consistency
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        }
    }

    void testStateConsistencyAfterFailures()
    {
        // Test state consistency after parse failures
        PacketWebvttSelection packet;

        // Valid parse
        auto valid1 = createValidPacket(0x01, 0x100, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Failed parse (bad type)
        auto invalid = createValidPacket(0x02, 0x200, 1280, 720);
        invalid[0] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(invalid.begin(), invalid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Another valid parse - state should recover
        auto valid2 = createValidPacket(0x03, 0x300, 640, 480);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());
    }

    void testStateConsistencyDimensionChanges()
    {
        // Test state consistency with varying dimension values
        PacketWebvttSelection packet;

        for (int i = 0; i < 20; ++i) {
            std::uint32_t width = 100 * (i + 1);
            std::uint32_t height = 75 * (i + 1);
            auto packetData = createValidPacket(0x01, 0x100 + i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testStateConsistencyGettersSnapshot()
    {
        // Test all getters return consistent snapshot of parsed data
        auto packetData = createValidPacket(0xABCD, 0x1234, 2560, 1440);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketWebvttSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Call getters multiple times, verify consistency
        for (int i = 0; i < 5; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABCD), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1234), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2560), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1440), packet.getRelatedVideoHeight());
        }
    }

    void testStateConsistencyHierarchicalFields()
    {
        // Test base class fields update correctly alongside dimensions
        PacketWebvttSelection packet;

        for (int i = 0; i < 10; ++i) {
            std::uint32_t channelId = 0x100 + i;
            std::uint32_t counter = 0x200 + i;
            std::uint32_t width = 1920 + i;
            std::uint32_t height = 1080 + i;

            auto packetData = createValidPacket(channelId, counter, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT_EQUAL(channelId, packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(counter, packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testErrorRecoveryAfterBadType()
    {
        // Test recovery after bad type error
        PacketWebvttSelection packet;

        // Bad type
        auto badType = createValidPacket(0x01, 0x100, 1920, 1080);
        badType[0] = 0x05; // Wrong type
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(badType.begin(), badType.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery with valid packet
        auto valid = createValidPacket(0x02, 0x200, 1280, 720);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testErrorRecoveryAfterBadSize()
    {
        // Test recovery after bad size error
        PacketWebvttSelection packet;

        // Bad size
        auto badSize = createValidPacket(0x01, 0x100, 1920, 1080);
        badSize[8] = 0x08; // Wrong size
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(badSize.begin(), badSize.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery
        auto valid = createValidPacket(0x02, 0x200, 3840, 2160);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testErrorRecoveryAfterTruncation()
    {
        // Test recovery after truncation error
        PacketWebvttSelection packet;

        // Truncated packet
        std::uint8_t truncated[] = {
            0x0F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03
        };
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Recovery
        auto valid = createValidPacket(0x03, 0x300, 640, 480);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());
    }

    void testErrorRecoveryMultipleFailures()
    {
        // Test recovery after multiple consecutive failures
        PacketWebvttSelection packet;

        // Failure 1: Bad type
        auto bad1 = createValidPacket(0x01, 0x100, 1920, 1080);
        bad1[0] = 0xFF;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(bad1.begin(), bad1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);

        // Failure 2: Bad size
        auto bad2 = createValidPacket(0x02, 0x200, 1280, 720);
        bad2[8] = 0x00;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(bad2.begin(), bad2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Failure 3: Truncated
        std::uint8_t truncated[] = {
            0x0F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);

        // Recovery
        auto valid = createValidPacket(0x04, 0x400, 2560, 1440);
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(valid.begin(), valid.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2560), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1440), packet.getRelatedVideoHeight());
    }

    void testErrorRecoverySustained()
    {
        // Test sustained error recovery over many iterations
        PacketWebvttSelection packet;

        for (int i = 0; i < 20; ++i) {
            // Valid packet
            auto valid = createValidPacket(0x100 + i, 0x200 + i, 1920, 1080);
            DataBufferPtr validBuffer = std::make_unique<DataBuffer>(valid.begin(), valid.end());
            CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);

            // Invalid packet
            auto invalid = createValidPacket(0x300 + i, 0x400 + i, 640, 480);
            invalid[0] = 0xFF;
            DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalid.begin(), invalid.end());
            CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
            CPPUNIT_ASSERT(packet.isValid() == false);
        }
    }

    void testCrossComponentBufferToPacket()
    {
        // Test complete Buffer → BufferReader → Packet chain
        std::uint8_t rawData[] = {
            0x0F, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0xEF, 0xCD, 0xAB, 0x89, // channel id
            0x80, 0x07, 0x00, 0x00, // width (1920)
            0x38, 0x04, 0x00, 0x00  // height (1080)
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(rawData), std::end(rawData));

        PacketWebvttSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x89ABCDEF), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testCrossComponentMultiplePackets()
    {
        // Test multiple packets through complete component chain
        PacketWebvttSelection packet;

        for (int i = 0; i < 15; ++i) {
            auto packetData = createValidPacket(0x10 * i, 0x20 * i, 1280 + (i * 64), 720 + (i * 36));
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10 * i), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280 + (i * 64)), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720 + (i * 36)), packet.getRelatedVideoHeight());
        }
    }

    void testCrossComponentDimensionExtraction()
    {
        // Test dimension extraction across Buffer → BufferReader → Packet chain
        std::vector<std::pair<std::uint32_t, std::uint32_t>> dimensions = {
            {0, 0},
            {1, 1},
            {640, 480},
            {1920, 1080},
            {3840, 2160},
            {0xFFFFFFFF, 0xFFFFFFFF},
            {0x12345678, 0x87654321}
        };

        for (const auto& dim : dimensions) {
            auto packetData = createValidPacket(0x01, 0x02, dim.first, dim.second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            PacketWebvttSelection packet;
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(dim.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(dim.second, packet.getRelatedVideoHeight());
        }
    }

    void testCrossComponentLargeStream()
    {
        // Simulate large stream of packets across all components
        PacketWebvttSelection packet;

        for (int i = 0; i < 200; ++i) {
            std::uint32_t channelId = 0x1000 + (i % 16);
            std::uint32_t counter = 0x2000 + i;
            std::uint32_t width = 1920 + (i % 20);
            std::uint32_t height = 1080 + (i % 10);

            auto packetData = createValidPacket(channelId, counter, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(channelId, packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(counter, packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testCrossComponentMixedPacketTypes()
    {
        // Test correct packet parsing when only WEBVTT_SELECTION should be accepted
        PacketWebvttSelection packet;

        // Valid WEBVTT_SELECTION packet
        auto validWebvtt = createValidPacket(0x01, 0x100, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(validWebvtt.begin(), validWebvtt.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);

        // TTML_SELECTION packet (type 7) - should be rejected
        auto ttmlPacket = createValidPacket(0x02, 0x200, 1280, 720);
        ttmlPacket[0] = 0x07;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(ttmlPacket.begin(), ttmlPacket.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Another valid WEBVTT_SELECTION packet
        auto validWebvtt2 = createValidPacket(0x03, 0x300, 640, 480);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(validWebvtt2.begin(), validWebvtt2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
    }

    void testRealWorldVideoFormatChange()
    {
        // Simulate video format change scenario (SD → HD → Full HD → 4K)
        PacketWebvttSelection packet;

        // Start with SD
        auto sdData = createValidPacket(0x01, 0x100, 640, 480);
        DataBufferPtr sdBuffer = std::make_unique<DataBuffer>(sdData.begin(), sdData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(sdBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());

        // Upgrade to HD
        auto hdData = createValidPacket(0x01, 0x200, 1280, 720);
        DataBufferPtr hdBuffer = std::make_unique<DataBuffer>(hdData.begin(), hdData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(hdBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());

        // Upgrade to Full HD
        auto fhdData = createValidPacket(0x01, 0x300, 1920, 1080);
        DataBufferPtr fhdBuffer = std::make_unique<DataBuffer>(fhdData.begin(), fhdData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(fhdBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());

        // Upgrade to 4K
        auto uhd4kData = createValidPacket(0x01, 0x400, 3840, 2160);
        DataBufferPtr uhd4kBuffer = std::make_unique<DataBuffer>(uhd4kData.begin(), uhd4kData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(uhd4kBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testRealWorldMultipleChannels()
    {
        // Simulate multiple subtitle channels with different video dimensions
        std::vector<PacketWebvttSelection> channels(4);

        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> channelConfigs = {
            {0x01, 1920, 1080},  // Main channel
            {0x02, 1280, 720},   // Secondary channel
            {0x03, 3840, 2160},  // 4K channel
            {0x04, 640, 480}     // Low res channel
        };

        for (size_t i = 0; i < channelConfigs.size(); ++i) {
            auto packetData = createValidPacket(
                std::get<0>(channelConfigs[i]),
                0x1000 + i,
                std::get<1>(channelConfigs[i]),
                std::get<2>(channelConfigs[i])
            );

            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(channels[i].parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(channels[i].isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(channelConfigs[i]), channels[i].getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(channelConfigs[i]), channels[i].getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(channelConfigs[i]), channels[i].getRelatedVideoHeight());
        }
    }

    void testRealWorldAspectRatioScenarios()
    {
        // Test various aspect ratios (16:9, 4:3, 21:9, 1:1, etc.)
        PacketWebvttSelection packet;

        std::vector<std::pair<std::uint32_t, std::uint32_t>> aspectRatios = {
            {1920, 1080},  // 16:9 (Full HD)
            {1280, 720},   // 16:9 (HD)
            {640, 480},    // 4:3 (SD)
            {800, 600},    // 4:3
            {2560, 1080},  // 21:9 (Ultrawide)
            {3440, 1440},  // 21:9 (Ultrawide QHD)
            {1024, 768},   // 4:3 (XGA)
            {1366, 768},   // ~16:9 (WXGA)
            {1440, 1440},  // 1:1 (Square)
            {2160, 3840}   // 9:16 (Portrait 4K)
        };

        for (const auto& res : aspectRatios) {
            auto packetData = createValidPacket(0x01, 0x02, res.first, res.second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(res.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(res.second, packet.getRelatedVideoHeight());
        }
    }

    void testRealWorldErrorHandling()
    {
        // Simulate real-world error scenarios and recovery
        PacketWebvttSelection packet;

        // Scenario 1: Valid packet
        auto valid1 = createValidPacket(0x01, 0x100, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Scenario 2: Corrupted packet (network error simulation)
        auto corrupted = createValidPacket(0x01, 0x200, 3840, 2160);
        corrupted[14] = 0xFF; // Corrupt width byte
        corrupted[18] = 0xAA; // Corrupt height byte
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(corrupted.begin(), corrupted.end());
        // Should parse successfully despite corruption (data still structurally valid)
        bool parseResult2 = packet.parse(std::move(buffer2));

        // Scenario 3: Truncated packet (incomplete transmission)
        std::uint8_t truncated[] = {
            0x0F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
            0x01, 0x02, 0x03, 0x04
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Scenario 4: Recovery with valid packet
        auto valid2 = createValidPacket(0x01, 0x300, 1280, 720);
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(valid2.begin(), valid2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testRealWorldAdaptiveBitrate()
    {
        // Simulate adaptive bitrate streaming with resolution changes
        PacketWebvttSelection packet;

        // Quality levels for adaptive streaming
        std::vector<std::pair<std::uint32_t, std::uint32_t>> qualityLevels = {
            {3840, 2160},  // 4K (highest quality)
            {2560, 1440},  // QHD (high quality)
            {1920, 1080},  // Full HD (medium-high)
            {1280, 720},   // HD (medium)
            {854, 480},    // SD (low)
            {640, 360}     // LD (lowest quality)
        };

        // Simulate quality downgrades due to bandwidth
        for (size_t i = 0; i < qualityLevels.size(); ++i) {
            auto packetData = createValidPacket(0x01, 0x100 + i, qualityLevels[i].first, qualityLevels[i].second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(qualityLevels[i].first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(qualityLevels[i].second, packet.getRelatedVideoHeight());
        }

        // Simulate quality upgrades as bandwidth improves
        for (int i = qualityLevels.size() - 1; i >= 0; --i) {
            auto packetData = createValidPacket(0x01, 0x200 + i, qualityLevels[i].first, qualityLevels[i].second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT_EQUAL(qualityLevels[i].first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(qualityLevels[i].second, packet.getRelatedVideoHeight());
        }
    }

    void testRealWorldDVBSubtitleStream()
    {
        // Simulate DVB subtitle stream with WebVTT overlay dimensions
        PacketWebvttSelection packet;

        // Typical DVB broadcast resolutions
        std::vector<std::pair<std::uint32_t, std::uint32_t>> dvbResolutions = {
            {720, 576},    // PAL SD
            {720, 480},    // NTSC SD
            {1920, 1080},  // HD 1080i/p
            {1280, 720}    // HD 720p
        };

        for (const auto& res : dvbResolutions) {
            auto packetData = createValidPacket(0x01, 0x02, res.first, res.second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(res.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(res.second, packet.getRelatedVideoHeight());
        }
    }

    void testRealWorldMobileRotation()
    {
        // Simulate mobile device screen rotation (dimension swap)
        PacketWebvttSelection packet;

        // Portrait mode
        auto portraitData = createValidPacket(0x01, 0x100, 1080, 1920);
        DataBufferPtr portraitBuffer = std::make_unique<DataBuffer>(portraitData.begin(), portraitData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(portraitBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoHeight());

        // Landscape mode (rotated)
        auto landscapeData = createValidPacket(0x01, 0x200, 1920, 1080);
        DataBufferPtr landscapeBuffer = std::make_unique<DataBuffer>(landscapeData.begin(), landscapeData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(landscapeBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testRealWorldHighFrequencyUpdates()
    {
        // Test high-frequency dimension updates (simulating live video adjustments)
        PacketWebvttSelection packet;

        // 100 rapid updates with slight variations
        for (int i = 0; i < 100; ++i) {
            std::uint32_t width = 1920 + (i % 5);
            std::uint32_t height = 1080 + (i % 3);
            auto packetData = createValidPacket(0x01, i, width, height);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(width, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(height, packet.getRelatedVideoHeight());
        }
    }

    void testRealWorldConcurrentChannels()
    {
        // Test concurrent subtitle channels with interleaved packets
        std::vector<PacketWebvttSelection> channels(3);
        std::uint32_t baseWidth = 1920;
        std::uint32_t baseHeight = 1080;

        // Interleave packets from 3 channels
        for (int round = 0; round < 10; ++round) {
            for (int ch = 0; ch < 3; ++ch) {
                std::uint32_t channelId = 0x01 + ch;
                std::uint32_t counter = (round * 3) + ch;
                std::uint32_t width = baseWidth + (ch * 64);
                std::uint32_t height = baseHeight + (ch * 36);

                auto packetData = createValidPacket(channelId, counter, width, height);
                DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

                CPPUNIT_ASSERT(channels[ch].parse(std::move(buffer)) == true);
                CPPUNIT_ASSERT(channels[ch].isValid() == true);
                CPPUNIT_ASSERT_EQUAL(channelId, channels[ch].getChannelId());
                CPPUNIT_ASSERT_EQUAL(width, channels[ch].getRelatedVideoWidth());
                CPPUNIT_ASSERT_EQUAL(height, channels[ch].getRelatedVideoHeight());
            }
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketWebvttSelectionTest );
