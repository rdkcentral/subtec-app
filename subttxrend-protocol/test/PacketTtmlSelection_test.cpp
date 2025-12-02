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

#include "PacketTtmlSelection.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketTtmlSelection;

class PacketTtmlSelectionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTtmlSelectionTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadSizeTooSmall);
    CPPUNIT_TEST(testBadSizeTooLarge);
    CPPUNIT_TEST(testBadSizeZero);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testConstructorTypeIdentity);
    CPPUNIT_TEST(testConstructorVideoDimensionsZero);
    CPPUNIT_TEST(testParseValidPacketBasic);
    CPPUNIT_TEST(testParseVideoWidthZero);
    CPPUNIT_TEST(testParseVideoHeightZero);
    CPPUNIT_TEST(testParseVideoWidthMax);
    CPPUNIT_TEST(testParseVideoHeightMax);
    CPPUNIT_TEST(testParseBothDimensionsZero);
    CPPUNIT_TEST(testParseBothDimensionsMax);
    CPPUNIT_TEST(testParseHDResolution);
    CPPUNIT_TEST(testParseFullHDResolution);
    CPPUNIT_TEST(testParse4KResolution);
    CPPUNIT_TEST(testParseSDResolution);
    CPPUNIT_TEST(testParseNonStandardResolution);
    CPPUNIT_TEST(testParseSizeOffByOneLess);
    CPPUNIT_TEST(testParseSizeOffByOneMore);
    CPPUNIT_TEST(testParseSizeMissingOneField);
    CPPUNIT_TEST(testParseSizeExcessiveExtra);
    CPPUNIT_TEST(testParseTruncatedAfterHeader);
    CPPUNIT_TEST(testParseTruncatedAfterChannelId);
    CPPUNIT_TEST(testParseTruncatedAfterWidth);
    CPPUNIT_TEST(testParseTruncatedWidthPartial);
    CPPUNIT_TEST(testParseTruncatedHeightPartial);
    CPPUNIT_TEST(testParseEmptyBuffer);
    CPPUNIT_TEST(testParseHeaderOnly);
    CPPUNIT_TEST(testLittleEndianWidthExtraction);
    CPPUNIT_TEST(testLittleEndianHeightExtraction);
    CPPUNIT_TEST(testLittleEndianBothFields);
    CPPUNIT_TEST(testStateValidToInvalid);
    CPPUNIT_TEST(testStateInvalidToValid);
    CPPUNIT_TEST(testStateMultipleValidParses);
    CPPUNIT_TEST(testStateMultipleDifferentParses);
    CPPUNIT_TEST(testGettersBeforeParse);
    CPPUNIT_TEST(testGettersAfterFailedParse);
    CPPUNIT_TEST(testBaseClassChannelId);
    CPPUNIT_TEST(testBaseClassCounter);
    CPPUNIT_TEST(testBaseClassSize);
    CPPUNIT_TEST(testBaseClassParseFailure);
    CPPUNIT_TEST(testWidthZeroHeightMax);
    CPPUNIT_TEST(testWidthMaxHeightZero);
    CPPUNIT_TEST(testMixedEndianness);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testParseAfterFailure);
    CPPUNIT_TEST(testSequentialFailures);
    CPPUNIT_TEST(testCorruptedType);
    CPPUNIT_TEST(testCorruptedSize);
    CPPUNIT_TEST(testExtraDataAfterPacket);
    CPPUNIT_TEST(testEndToEndCompleteWorkflow);
    CPPUNIT_TEST(testEndToEndMultipleResolutions);
    CPPUNIT_TEST(testEndToEndBoundaryValues);
    CPPUNIT_TEST(testEndToEndInvalidPacketWorkflow);
    CPPUNIT_TEST(testEndToEndSequentialParsing);
    CPPUNIT_TEST(testBufferReaderIntegrationHeaderExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationPayloadExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationOffsetTracking);
    CPPUNIT_TEST(testBufferReaderIntegrationCompleteRead);
    CPPUNIT_TEST(testPolymorphismGetTypeVirtual);
    CPPUNIT_TEST(testPolymorphismParseVirtual);
    CPPUNIT_TEST(testPolymorphismIsValidVirtual);
    CPPUNIT_TEST(testPolymorphismChannelSpecificBase);
    CPPUNIT_TEST(testPolymorphismGettersVirtual);
    CPPUNIT_TEST(testStateConsistencyAcrossGetters);
    CPPUNIT_TEST(testStateConsistencyMultipleCalls);
    CPPUNIT_TEST(testStateConsistencyFailureToSuccess);
    CPPUNIT_TEST(testStateConsistencySuccessToFailure);
    CPPUNIT_TEST(testErrorRecoverySequentialParsing);
    CPPUNIT_TEST(testErrorRecoveryAlternatingValidInvalid);
    CPPUNIT_TEST(testErrorRecoveryAfterTruncation);
    CPPUNIT_TEST(testErrorRecoveryAfterCorruption);
    CPPUNIT_TEST(testCrossComponentBufferAndReader);
    CPPUNIT_TEST(testCrossComponentMultiplePacketParsing);
    CPPUNIT_TEST(testCrossComponentDimensionExtraction);
    CPPUNIT_TEST(testCrossComponentLargePacketStream);
    CPPUNIT_TEST(testRealWorldVideoFormatChange);
    CPPUNIT_TEST(testRealWorldMultipleChannels);
    CPPUNIT_TEST(testRealWorldAspectRatioScenario);
    CPPUNIT_TEST(testRealWorldErrorHandlingScenario);
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
            0x07, 0x00, 0x00, 0x00, // type (TTML_SELECTION = 7)
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

    void testGood()
    {
        auto packetData = createValidPacket();

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01020304), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testBadType()
    {
        auto packetData = createValidPacket();
        packetData[0] = 0x02; // TIMESTAMP type

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeTooSmall()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0B; // size = 11 (one byte short)
        packetData.pop_back();

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeTooLarge()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0D; // size = 13 (one byte extra)
        packetData.push_back(0xFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeZero()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testConstructorInitialState()
    {
        PacketTtmlSelection packet;

        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testConstructorTypeIdentity()
    {
        PacketTtmlSelection packet;

        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
    }

    void testConstructorVideoDimensionsZero()
    {
        PacketTtmlSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testParseValidPacketBasic()
    {
        auto packetData = createValidPacket(0x100, 0x200, 1280, 720);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testParseVideoWidthZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParseVideoHeightZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testParseVideoWidthMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParseVideoHeightMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0xFFFFFFFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testParseBothDimensionsZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0, 0);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testParseBothDimensionsMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 0xFFFFFFFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testParseHDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1280, 720);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testParseFullHDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testParse4KResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 3840, 2160);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testParseSDResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 640, 480);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());
    }

    void testParseNonStandardResolution()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1366, 768);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1366), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(768), packet.getRelatedVideoHeight());
    }

    void testParseSizeOffByOneLess()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0B; // size = 11 instead of 12
        packetData.pop_back();

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseSizeOffByOneMore()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x0D; // size = 13 instead of 12
        packetData.push_back(0xFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseSizeMissingOneField()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x08; // size = 8 (missing height field)
        packetData.resize(packetData.size() - 4);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseSizeExcessiveExtra()
    {
        auto packetData = createValidPacket();
        packetData[8] = 0x70; // size = 112 (excessive)
        for (int i = 0; i < 100; ++i) {
            packetData.push_back(0xFF);
        }

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterHeader()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterChannelId()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedAfterWidth()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, 0x00, 0x00, // width = 1920
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedWidthPartial()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, // width partially present (2 bytes)
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseTruncatedHeightPartial()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, 0x00, 0x00, // width = 1920
            0x38, 0x04, // height partially present (2 bytes)
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseEmptyBuffer()
    {
        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testParseHeaderOnly()
    {
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testLittleEndianWidthExtraction()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0x04030201, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getRelatedVideoWidth());
    }

    void testLittleEndianHeightExtraction()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 0x08070605);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), packet.getRelatedVideoHeight());
    }

    void testLittleEndianBothFields()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xAABBCCDD, 0x11223344);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), packet.getRelatedVideoHeight());
    }

    void testStateValidToInvalid()
    {
        PacketTtmlSelection packet;

        // First parse: valid
        auto validData = createValidPacket(0x01, 0x02, 1920, 1080);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Second parse: invalid
        auto invalidData = createValidPacket();
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testStateInvalidToValid()
    {
        PacketTtmlSelection packet;

        // First parse: invalid
        auto invalidData = createValidPacket();
        invalidData[8] = 0x00; // Invalid size
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse: valid
        auto validData = createValidPacket(0x05, 0x06, 3840, 2160);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testStateMultipleValidParses()
    {
        PacketTtmlSelection packet;

        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        for (int i = 0; i < 3; ++i) {
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
        }
    }

    void testStateMultipleDifferentParses()
    {
        PacketTtmlSelection packet;

        // Parse 1
        auto data1 = createValidPacket(0x01, 0x10, 640, 480);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480), packet.getRelatedVideoHeight());

        // Parse 2
        auto data2 = createValidPacket(0x02, 0x20, 1920, 1080);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());

        // Parse 3
        auto data3 = createValidPacket(0x03, 0x30, 3840, 2160);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testGettersBeforeParse()
    {
        PacketTtmlSelection packet;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testGettersAfterFailedParse()
    {
        PacketTtmlSelection packet;

        auto invalidData = createValidPacket();
        invalidData[0] = 0xFF; // Invalid type
        DataBufferPtr buffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));

        // After failed parse, getters should return initial values
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testBaseClassChannelId()
    {
        auto packetData = createValidPacket(0xDEADBEEF, 0x02, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
    }

    void testBaseClassCounter()
    {
        auto packetData = createValidPacket(0x01, 0xCAFEBABE, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
    }

    void testBaseClassSize()
    {
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    void testBaseClassParseFailure()
    {
        // Create packet with valid header but truncated channel ID
        std::uint8_t packetData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, // channel id truncated
        };

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testWidthZeroHeightMax()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0, 0xFFFFFFFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoHeight());
    }

    void testWidthMaxHeightZero()
    {
        auto packetData = createValidPacket(0x01, 0x02, 0xFFFFFFFF, 0);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testMixedEndianness()
    {
        auto packetData = createValidPacket(0x04030201, 0x08070605, 0x0C0B0A09, 0x100F0E0D);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x0C0B0A09), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x100F0E0D), packet.getRelatedVideoHeight());
    }

    void testPacketReuse()
    {
        PacketTtmlSelection packet;

        // First parse
        auto data1 = createValidPacket(0x01, 0x10, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());

        // Second parse with different data
        auto data2 = createValidPacket(0x02, 0x20, 3840, 2160);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x20), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testParseAfterFailure()
    {
        PacketTtmlSelection packet;

        // First parse: invalid
        auto invalidData = createValidPacket();
        invalidData[8] = 0xFF; // Invalid size
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse: valid
        auto validData = createValidPacket(0x99, 0xAA, 1280, 720);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testSequentialFailures()
    {
        PacketTtmlSelection packet;

        // First failure: invalid type
        std::uint8_t invalidType[] = {
            0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: invalid size
        std::uint8_t invalidSize[] = {
            0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidSize), std::end(invalidSize));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Valid parse should still work
        auto validData = createValidPacket(0x55, 0x66, 1920, 1080);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55), packet.getChannelId());
    }

    void testCorruptedType()
    {
        auto packetData = createValidPacket();
        packetData[1] = 0xFF; // Corrupt type field

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testCorruptedSize()
    {
        auto packetData = createValidPacket();
        packetData[9] = 0xFF; // Corrupt size field

        PacketTtmlSelection packet;
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

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Should fail because there's extra data
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testEndToEndCompleteWorkflow()
    {
        // Complete end-to-end workflow: create packet -> parse -> validate -> extract all data
        auto packetData = createValidPacket(0x12345678, 0xABCDEF01, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Parse and validate
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Verify all header fields
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABCDEF01), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getChannelId());

        // Verify video dimensions
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testEndToEndMultipleResolutions()
    {
        // Test multiple common resolutions in sequence
        std::vector<std::pair<std::uint32_t, std::uint32_t>> resolutions = {
            {640, 480},      // SD
            {1280, 720},     // HD
            {1920, 1080},    // Full HD
            {3840, 2160},    // 4K
            {7680, 4320}     // 8K
        };

        for (const auto& res : resolutions) {
            auto packetData = createValidPacket(0x01, 0x02, res.first, res.second);

            PacketTtmlSelection packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(res.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(res.second, packet.getRelatedVideoHeight());
        }
    }

    void testEndToEndBoundaryValues()
    {
        // Test end-to-end with boundary values
        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>> testCases = {
            {0x00000000, 0x00000000, 0, 0},
            {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
            {0x12345678, 0x87654321, 0, 0xFFFFFFFF},
            {0xDEADBEEF, 0xCAFEBABE, 0xFFFFFFFF, 0}
        };

        for (const auto& testCase : testCases) {
            auto packetData = createValidPacket(
                std::get<0>(testCase),
                std::get<1>(testCase),
                std::get<2>(testCase),
                std::get<3>(testCase)
            );

            PacketTtmlSelection packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(testCase), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(testCase), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(testCase), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(std::get<3>(testCase), packet.getRelatedVideoHeight());
        }
    }

    void testEndToEndInvalidPacketWorkflow()
    {
        // Verify complete workflow handles invalid packets correctly
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);
        packetData[8] = 0x0D; // Change size to 13 (invalid)
        packetData.push_back(0xFF);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Verify packet remains in safe state after failure
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());
    }

    void testEndToEndSequentialParsing()
    {
        // Parse multiple packets sequentially to verify state management
        PacketTtmlSelection packet;

        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>> packets = {
            {0x01, 0x10, 640, 480},
            {0x02, 0x20, 1280, 720},
            {0x03, 0x30, 1920, 1080},
            {0x04, 0x40, 3840, 2160}
        };

        for (const auto& pkt : packets) {
            auto packetData = createValidPacket(
                std::get<0>(pkt),
                std::get<1>(pkt),
                std::get<2>(pkt),
                std::get<3>(pkt)
            );

            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(pkt), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(pkt), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(pkt), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(std::get<3>(pkt), packet.getRelatedVideoHeight());
        }
    }

    void testBufferReaderIntegrationHeaderExtraction()
    {
        // Test BufferReader integration for header extraction
        auto packetData = createValidPacket(0xAABBCCDD, 0x11223344, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify header fields extracted correctly via BufferReader
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
    }

    void testBufferReaderIntegrationPayloadExtraction()
    {
        // Test BufferReader integration for payload extraction
        auto packetData = createValidPacket(0x12345678, 0xABCDEF01, 0x87654321, 0xFEDCBA98);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify payload fields extracted correctly
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x87654321), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFEDCBA98), packet.getRelatedVideoHeight());
    }

    void testBufferReaderIntegrationOffsetTracking()
    {
        // Verify BufferReader correctly tracks offsets during parsing
        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Parse should consume entire buffer (24 bytes)
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // All fields should be extracted in correct order
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
    }

    void testBufferReaderIntegrationCompleteRead()
    {
        // Test complete read of all packet fields via BufferReader
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify all fields read correctly without corruption
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x87654321), packet.getRelatedVideoHeight());
    }

    void testPolymorphismGetTypeVirtual()
    {
        // Test virtual getType() dispatch through base class pointer
        PacketTtmlSelection derivedPacket;
        Packet* basePtr = &derivedPacket;
        PacketChannelSpecific* channelPtr = &derivedPacket;

        // Verify virtual dispatch works correctly
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT(derivedPacket.getType() == Packet::Type::TTML_SELECTION);
    }

    void testPolymorphismParseVirtual()
    {
        // Test virtual parse() through base class pointer
        auto packetData = createValidPacket(0x123, 0x456, 1920, 1080);

        PacketTtmlSelection derivedPacket;
        Packet* basePtr = &derivedPacket;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Parse through base pointer
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x456), basePtr->getCounter());

        // Verify derived class methods still accessible
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x123), derivedPacket.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), derivedPacket.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), derivedPacket.getRelatedVideoHeight());
    }

    void testPolymorphismIsValidVirtual()
    {
        // Test isValid() state consistency through polymorphic interface
        PacketTtmlSelection derivedPacket;
        Packet* basePtr = &derivedPacket;

        CPPUNIT_ASSERT(basePtr->isValid() == false);

        auto packetData = createValidPacket(0x01, 0x02, 1920, 1080);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(derivedPacket.isValid() == true);
    }

    void testPolymorphismChannelSpecificBase()
    {
        // Verify PacketChannelSpecific base class integration
        PacketTtmlSelection packet;
        PacketChannelSpecific* channelPtr = &packet;

        auto packetData = createValidPacket(0xAABBCCDD, 0x11223344, 3840, 2160);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(channelPtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(channelPtr->isValid() == true);

        // Verify channel-specific functionality through base pointer
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), channelPtr->getChannelId());
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::TTML_SELECTION);
    }

    void testPolymorphismGettersVirtual()
    {
        // Test getter methods through polymorphic base class pointers
        PacketTtmlSelection derivedPacket;
        Packet* basePtr = &derivedPacket;
        PacketChannelSpecific* channelPtr = &derivedPacket;

        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 7680, 4320);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);

        // Verify getters through different base pointers
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), basePtr->getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), basePtr->getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), channelPtr->getChannelId());

        // Verify derived class specific getters
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7680), derivedPacket.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4320), derivedPacket.getRelatedVideoHeight());
    }

    void testStateConsistencyAcrossGetters()
    {
        // Verify state remains consistent across multiple getter calls
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 1920, 1080);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Call getters multiple times, verify consistency
        for (int i = 0; i < 5; ++i) {
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), packet.getSize());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());
        }
    }

    void testStateConsistencyMultipleCalls()
    {
        // Verify dimension queries remain consistent
        auto packetData = createValidPacket(0x01, 0x02, 3840, 2160);

        PacketTtmlSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Query dimensions multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01), packet.getChannelId());
            CPPUNIT_ASSERT(packet.isValid() == true);
        }
    }

    void testStateConsistencyFailureToSuccess()
    {
        // Test state machine: failure -> success transition
        PacketTtmlSelection packet;

        // Invalid packet
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // Wrong type
            0x01, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00,
        };

        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getRelatedVideoHeight());

        // Valid packet
        auto validData = createValidPacket(0xABC, 0xDEF, 1280, 720);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABC), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testStateConsistencySuccessToFailure()
    {
        // Test state machine: success -> failure transition
        PacketTtmlSelection packet;

        // Valid packet first
        auto validData = createValidPacket(0x111, 0x222, 1920, 1080);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), packet.getRelatedVideoHeight());

        // Invalid packet
        auto invalidData = createValidPacket(0x444, 0x555, 3840, 2160);
        invalidData[8] = 0x0D; // Wrong size (13 instead of 12)
        invalidData.push_back(0xFF);
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testErrorRecoverySequentialParsing()
    {
        // Test robust error recovery in sequential packet stream
        PacketTtmlSelection packet;

        // Parse 1: valid
        auto data1 = createValidPacket(0x01, 0x10, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());

        // Parse 2: invalid type
        auto data2 = createValidPacket(0x02, 0x20, 3840, 2160);
        data2[0] = 0x03; // RESET_ALL type
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Parse 3: valid (recovery)
        auto data3 = createValidPacket(0x03, 0x30, 1280, 720);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x30), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1280), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(720), packet.getRelatedVideoHeight());
    }

    void testErrorRecoveryAlternatingValidInvalid()
    {
        // Test alternating valid and invalid packets
        PacketTtmlSelection packet;

        for (int i = 0; i < 3; ++i) {
            // Valid packet
            auto validData = createValidPacket(0x100 + i, 0x200 + i, 1920, 1080);
            DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x100 + i), packet.getChannelId());

            // Invalid packet
            auto invalidData = createValidPacket(0x300 + i, 0x400 + i, 3840, 2160);
            invalidData[8] = 0x00; // Size = 0
            DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
            CPPUNIT_ASSERT(packet.isValid() == false);
        }
    }

    void testErrorRecoveryAfterTruncation()
    {
        // Test recovery after truncated packet
        PacketTtmlSelection packet;

        // Truncated packet
        std::uint8_t truncatedData[] = {
            0x07, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x0C, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x80, 0x07, // width truncated (only 2 bytes)
        };
        DataBufferPtr truncBuffer = std::make_unique<DataBuffer>(std::begin(truncatedData), std::end(truncatedData));
        CPPUNIT_ASSERT(packet.parse(std::move(truncBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Valid packet (recovery)
        auto validData = createValidPacket(0xAAA, 0xBBB, 1920, 1080);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAAA), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), packet.getRelatedVideoWidth());
    }

    void testErrorRecoveryAfterCorruption()
    {
        // Test recovery after corrupted packet
        PacketTtmlSelection packet;

        // Corrupted packet (bad type)
        auto corruptedData = createValidPacket(0x01, 0x02, 1920, 1080);
        corruptedData[0] = 0xFF;
        DataBufferPtr corruptBuffer = std::make_unique<DataBuffer>(corruptedData.begin(), corruptedData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(corruptBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Valid packet (recovery)
        auto validData = createValidPacket(0xFEDCBA98, 0x12345678, 3840, 2160);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFEDCBA98), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3840), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2160), packet.getRelatedVideoHeight());
    }

    void testCrossComponentBufferAndReader()
    {
        // Test Buffer and BufferReader integration with packet parsing
        auto packetData = createValidPacket(0x11223344, 0x55667788, 0xAABBCCDD, 0xEEFF0011);

        // Create buffer from raw data
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        PacketTtmlSelection packet;
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify all components worked together correctly
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x11223344), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55667788), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), packet.getRelatedVideoWidth());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xEEFF0011), packet.getRelatedVideoHeight());
    }

    void testCrossComponentMultiplePacketParsing()
    {
        // Parse multiple packets to test cross-component interactions
        PacketTtmlSelection packet;

        for (int i = 0; i < 5; ++i) {
            auto packetData = createValidPacket(0x10 * i, 0x20 * i, 640 + (i * 640), 480 + (i * 540));
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10 * i), packet.getChannelId());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(640 + (i * 640)), packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(480 + (i * 540)), packet.getRelatedVideoHeight());
        }
    }

    void testCrossComponentDimensionExtraction()
    {
        // Test dimension extraction across Buffer -> BufferReader -> Packet chain
        std::vector<std::pair<std::uint32_t, std::uint32_t>> dimensions = {
            {0x00000000, 0x00000000},
            {0x12345678, 0x87654321},
            {0xFFFFFFFF, 0xFFFFFFFF},
            {1920, 1080},
            {3840, 2160}
        };

        for (const auto& dim : dimensions) {
            auto packetData = createValidPacket(0x01, 0x02, dim.first, dim.second);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            PacketTtmlSelection packet;
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(dim.first, packet.getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(dim.second, packet.getRelatedVideoHeight());
        }
    }

    void testCrossComponentLargePacketStream()
    {
        // Simulate large stream of packets
        PacketTtmlSelection packet;

        for (int i = 0; i < 100; ++i) {
            std::uint32_t channelId = 0x1000 + i;
            std::uint32_t counter = 0x2000 + i;
            std::uint32_t width = 1920 + (i % 10);
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

    void testRealWorldVideoFormatChange()
    {
        // Simulate video format change scenario (e.g., SD to HD to 4K)
        PacketTtmlSelection packet;

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
        std::vector<PacketTtmlSelection> packets(4);

        std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> channels = {
            {0x01, 1920, 1080},  // Main channel
            {0x02, 1280, 720},   // Secondary channel
            {0x03, 3840, 2160},  // 4K channel
            {0x04, 640, 480}     // Low res channel
        };

        for (size_t i = 0; i < channels.size(); ++i) {
            auto packetData = createValidPacket(
                std::get<0>(channels[i]),
                0x1000 + i,
                std::get<1>(channels[i]),
                std::get<2>(channels[i])
            );

            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packets[i].parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packets[i].isValid() == true);
            CPPUNIT_ASSERT_EQUAL(std::get<0>(channels[i]), packets[i].getChannelId());
            CPPUNIT_ASSERT_EQUAL(std::get<1>(channels[i]), packets[i].getRelatedVideoWidth());
            CPPUNIT_ASSERT_EQUAL(std::get<2>(channels[i]), packets[i].getRelatedVideoHeight());
        }
    }

    void testRealWorldAspectRatioScenario()
    {
        // Test various aspect ratios (16:9, 4:3, 21:9, etc.)
        PacketTtmlSelection packet;

        std::vector<std::pair<std::uint32_t, std::uint32_t>> aspectRatios = {
            {1920, 1080},  // 16:9 (Full HD)
            {1280, 720},   // 16:9 (HD)
            {640, 480},    // 4:3 (SD)
            {800, 600},    // 4:3
            {2560, 1080},  // 21:9 (Ultrawide)
            {3440, 1440},  // 21:9 (Ultrawide)
            {1024, 768},   // 4:3 (XGA)
            {1366, 768}    // ~16:9 (WXGA)
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

    void testRealWorldErrorHandlingScenario()
    {
        // Simulate real-world error scenarios and recovery
        PacketTtmlSelection packet;

        // Scenario 1: Valid packet
        auto valid1 = createValidPacket(0x01, 0x100, 1920, 1080);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(valid1.begin(), valid1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Scenario 2: Corrupted packet (network error simulation)
        auto corrupted = createValidPacket(0x01, 0x200, 3840, 2160);
        corrupted[10] = 0xFF; // Corrupt a byte
        corrupted[15] = 0xAA;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(corrupted.begin(), corrupted.end());
        // Parsing may succeed depending on what was corrupted; just verify it handles gracefully
        bool parseResult2 = packet.parse(std::move(buffer2));
        // Either succeeds with corrupted data or fails gracefully

        // Scenario 3: Truncated packet (incomplete transmission)
        std::uint8_t truncated[] = {
            0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTtmlSelectionTest );
