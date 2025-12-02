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

#include "PacketParser.hpp"
#include "Packet.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketParser;

class PacketParserTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(PacketParserTest);
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testParseValidPesDataPacket);
    CPPUNIT_TEST(testParseValidTimestampPacket);
    CPPUNIT_TEST(testParseValidResetAllPacket);
    CPPUNIT_TEST(testParseValidResetChannelPacket);
    CPPUNIT_TEST(testParseValidSubtitleSelectionPacket);
    CPPUNIT_TEST(testParseValidTeletextSelectionPacket);
    CPPUNIT_TEST(testParseValidTtmlSelectionPacket);
    CPPUNIT_TEST(testParseValidTtmlDataPacket);
    CPPUNIT_TEST(testParseValidTtmlTimestampPacket);
    CPPUNIT_TEST(testParseValidCcDataPacket);
    CPPUNIT_TEST(testParseValidPausePacket);
    CPPUNIT_TEST(testParseValidResumePacket);
    CPPUNIT_TEST(testParseValidMutePacket);
    CPPUNIT_TEST(testParseValidUnmutePacket);
    CPPUNIT_TEST(testParseValidWebvttSelectionPacket);
    CPPUNIT_TEST(testParseValidWebvttDataPacket);
    CPPUNIT_TEST(testParseValidWebvttTimestampPacket);
    CPPUNIT_TEST(testParseValidSetCCAttributesPacket);
    CPPUNIT_TEST(testParseValidTtmlInfoPacket);
    CPPUNIT_TEST(testParseValidFlushPacket);
    CPPUNIT_TEST(testParseUnknownPacketType);
    CPPUNIT_TEST(testParseInvalidTypeMarker);
    CPPUNIT_TEST(testParseTypeNotInRegisteredMap);
    CPPUNIT_TEST(testParseTypeZero);
    CPPUNIT_TEST(testParseTypeMax);
    CPPUNIT_TEST(testParseEmptyBuffer);
    CPPUNIT_TEST(testParseBufferSizeLessThan4Bytes);
    CPPUNIT_TEST(testParseBufferSize4BytesTypeOnly);
    CPPUNIT_TEST(testParseBufferSize8BytesTypeAndCounter);
    CPPUNIT_TEST(testParseCorruptedPacketTypeField);
    CPPUNIT_TEST(testParseValidTypeButMalformedPayload);
    CPPUNIT_TEST(testSequentialParseCallsWithDifferentTypes);
    CPPUNIT_TEST(testParseReturnsInvalidAfterMultipleFailures);
    CPPUNIT_TEST(testParseRecoveryAfterInvalidPacket);
    CPPUNIT_TEST(testParsePacketReferenceSemantics);
    CPPUNIT_TEST(testParsePacketWithCounterZero);
    CPPUNIT_TEST(testParsePacketWithCounterMaxValue);
    CPPUNIT_TEST(testParseTruncatedHeader);
    CPPUNIT_TEST(testParsePacketWithWrongTypeValue);
    CPPUNIT_TEST(testParsePacketWithInvalidSizeField);
    CPPUNIT_TEST(testExtractTypeValidPacket);
    CPPUNIT_TEST(testExtractTypeInvalidPacket);
    CPPUNIT_TEST(testParserStateIsolationBetweenCalls);
    CPPUNIT_TEST(testPacketReusePattern);
    CPPUNIT_TEST(testPreviousPacketInvalidatedOnNewParse);
    CPPUNIT_TEST(testParseAllRegisteredPacketTypes);
    CPPUNIT_TEST(testParseAlternatingPacketTypes);
    CPPUNIT_TEST(testSubtitleWorkflowSequence);
    CPPUNIT_TEST(testTtmlWorkflowComplete);
    CPPUNIT_TEST(testWebvttWorkflowComplete);
    CPPUNIT_TEST(testClosedCaptionWorkflow);
    CPPUNIT_TEST(testPlaybackControlWorkflow);
    CPPUNIT_TEST(testAudioControlWorkflow);
    CPPUNIT_TEST(testChannelResetWorkflow);
    CPPUNIT_TEST(testGlobalResetWorkflow);
    CPPUNIT_TEST(testMixedValidInvalidPacketStream);
    CPPUNIT_TEST(testPacketStreamAfterInvalidPacket);
    CPPUNIT_TEST(testAlternatingMultiChannelPackets);
    CPPUNIT_TEST(testLargeDataPacketParsing);
    CPPUNIT_TEST(testRapidSuccessionPackets);
    CPPUNIT_TEST(testBufferIntegration);
    CPPUNIT_TEST(testDataBufferLifecycle);
    CPPUNIT_TEST(testAllPacketTypesInSession);
    CPPUNIT_TEST(testCounterProgression);
    CPPUNIT_TEST(testFlushFollowedByNewData);
    CPPUNIT_TEST(testPacketDataPolymorphism);
    CPPUNIT_TEST(testPacketGenericSubtypes);
    CPPUNIT_TEST(testPacketChannelSpecificSubtypes);
    CPPUNIT_TEST(testCompleteSubtitleSessionSimulation);
    CPPUNIT_TEST(testChannelSwitchingScenario);
    CPPUNIT_TEST(testErrorRecoveryInStream);
    CPPUNIT_TEST(testSubtitleSelectionToDataToTimestamp);
    CPPUNIT_TEST(testTtmlSelectionInfoDataTimestamp);
    CPPUNIT_TEST(testWebvttSelectionDataTimestamp);
    CPPUNIT_TEST(testCcDataWithAttributesWorkflow);
    CPPUNIT_TEST(testCcDataSequenceWithTimestamps);
    CPPUNIT_TEST(testPauseResumeWithDataPackets);
    CPPUNIT_TEST(testMuteUnmuteWithSubtitles);
    CPPUNIT_TEST(testResetChannelFollowedByNewData);
    CPPUNIT_TEST(testResetAllFollowedByNewSession);
    CPPUNIT_TEST(testStreamWithIntermittentInvalidPackets);
    CPPUNIT_TEST(testMultipleChannelsWithResets);
    CPPUNIT_TEST(testInterleavedMultiChannelStream);
    CPPUNIT_TEST(testMaximumSizeDataPacket);
    CPPUNIT_TEST(testContinuousStreamParsing);
    CPPUNIT_TEST(testCounterProgressionAcrossPackets);
    CPPUNIT_TEST(testCounterWrapAround);
    CPPUNIT_TEST(testFlushInMultiChannelScenario);
    CPPUNIT_TEST(testMultiFormatSession);
    CPPUNIT_TEST(testRecoveryAfterMultipleFailures);

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

    DataBufferPtr createValidPacket(Packet::Type type, std::uint32_t counter = 0x12345678)
    {
        std::uint32_t typeValue = static_cast<std::uint32_t>(type);
        std::vector<std::uint8_t> data;

        // Type field (4 bytes, little endian)
        data.push_back(typeValue & 0xFF);
        data.push_back((typeValue >> 8) & 0xFF);
        data.push_back((typeValue >> 16) & 0xFF);
        data.push_back((typeValue >> 24) & 0xFF);

        // Counter field (4 bytes, little endian)
        data.push_back(counter & 0xFF);
        data.push_back((counter >> 8) & 0xFF);
        data.push_back((counter >> 16) & 0xFF);
        data.push_back((counter >> 24) & 0xFF);

        // Size field depends on packet type
        std::uint32_t dataSize = 0;
        std::vector<std::uint8_t> payloadData;

        switch (type)
        {
            case Packet::Type::PES_DATA:
                // PES_DATA: channel ID (4) + channel type (4) + minimal data (1)
                dataSize = 9;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Channel type
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Minimal data
                payloadData.push_back(0xAA);
                break;

            case Packet::Type::CC_DATA:
                // CC_DATA: channel ID (4) + channel type (4) + PTS presence (4) + PTS (4) + minimal data (1)
                dataSize = 17;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Channel type
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // PTS presence
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // PTS
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Minimal data
                payloadData.push_back(0xAA);
                break;

            case Packet::Type::TTML_DATA:
            case Packet::Type::WEBVTT_DATA:
                // TTML_DATA/WEBVTT_DATA: channel ID (4) + display offset (8) + minimal data (1)
                dataSize = 13;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Display offset (8 bytes)
                for (int i = 0; i < 8; i++)
                    payloadData.push_back(0x00);
                // Minimal data
                payloadData.push_back(0xAA);
                break;

            case Packet::Type::TIMESTAMP:
            case Packet::Type::TTML_TIMESTAMP:
            case Packet::Type::WEBVTT_TIMESTAMP:
                // Timestamp packets: timestamp (8 bytes) + STC (4 bytes)
                dataSize = 12;
                // Timestamp (8 bytes)
                for (int i = 0; i < 8; i++) payloadData.push_back(0x11 + i);
                // STC (4 bytes)
                for (int i = 0; i < 4; i++) payloadData.push_back(0x22 + i);
                break;

            case Packet::Type::RESET_CHANNEL:
                // Reset channel: channel ID (4 bytes)
                dataSize = 4;
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                break;

            case Packet::Type::SUBTITLE_SELECTION:
                // SUBTITLE_SELECTION: channel ID (4) + subtitlesType (4) + auxId1 (4) + auxId2 (4)
                dataSize = 16;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // subtitlesType
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // auxId1
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // auxId2
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                break;

            case Packet::Type::TELETEXT_SELECTION:
                // Teletext selection: channel ID + magazine + page (12 bytes)
                dataSize = 12;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Magazine
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Page
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                break;

            case Packet::Type::TTML_SELECTION:
            case Packet::Type::WEBVTT_SELECTION:
                // TTML/WebVTT selection: channel ID + width + height
                dataSize = 12;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Width
                payloadData.push_back(0x80);
                payloadData.push_back(0x07);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Height
                payloadData.push_back(0x38);
                payloadData.push_back(0x04);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                break;

            case Packet::Type::TTML_INFO:
                // TTML info: channel ID + 4 additional fields (each 4 bytes)
                dataSize = 20;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // Additional fields (4 x 4 bytes)
                for (int i = 0; i < 16; i++) payloadData.push_back(0x00);
                break;

            case Packet::Type::SET_CC_ATTRIBUTES:
                // CC attributes: channel ID + ccType + attribType + 14 attributes (each 4 bytes)
                // Total = 4 + 4 + 4 + (14 * 4) = 68
                dataSize = 68;
                // Channel ID (4 bytes)
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // ccType (4 bytes)
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // attribType bitmask (enable all 14 attributes) 0x00003FFF little-endian
                payloadData.push_back(0xFF); // low byte
                payloadData.push_back(0x3F); // next byte
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                // 14 attributes (each 4 bytes -> 56 bytes total)
                for (int i = 0; i < 56; i++)
                    payloadData.push_back(0x00);
                break;

            case Packet::Type::RESET_ALL:
                // Control packets with no payload
                dataSize = 0;
                break;

            case Packet::Type::PAUSE:
            case Packet::Type::RESUME:
            case Packet::Type::MUTE:
            case Packet::Type::UNMUTE:
            case Packet::Type::FLUSH:
                // Control packets with channel ID only (extends PacketChannelSpecific)
                dataSize = 4;
                // Channel ID
                payloadData.push_back(0x01);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                payloadData.push_back(0x00);
                break;

            default:
                dataSize = 0;
                break;
        }

        // Size field (4 bytes, little endian)
        data.push_back(dataSize & 0xFF);
        data.push_back((dataSize >> 8) & 0xFF);
        data.push_back((dataSize >> 16) & 0xFF);
        data.push_back((dataSize >> 24) & 0xFF);

        // Payload data
        data.insert(data.end(), payloadData.begin(), payloadData.end());

        return std::make_unique<DataBuffer>(data.begin(), data.end());
    }

    void testConstructorInitialization()
    {
        PacketParser parser;

        // Parser should be constructed successfully
        // We test indirectly by parsing a known packet type
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_ALL);
        const Packet& packet = parser.parse(std::move(buffer));

        // Should be able to parse a registered packet type
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testParseValidPesDataPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, 0x11223344);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x11223344);
    }

    void testParseValidTimestampPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TIMESTAMP, 0xAABBCCDD);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet.getCounter() == 0xAABBCCDD);
    }

    void testParseValidResetAllPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_ALL, 0x99887766);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getCounter() == 0x99887766);
    }

    void testParseValidResetChannelPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_CHANNEL, 0x12345678);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT(packet.getCounter() == 0x12345678);
    }

    void testParseValidSubtitleSelectionPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::SUBTITLE_SELECTION, 0x11112222);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SUBTITLE_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x11112222);
    }

    void testParseValidTeletextSelectionPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TELETEXT_SELECTION, 0x33334444);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x33334444);
    }

    void testParseValidTtmlSelectionPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TTML_SELECTION, 0x55556666);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x55556666);
    }

    void testParseValidTtmlDataPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TTML_DATA, 0x77778888);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x77778888);
    }

    void testParseValidTtmlTimestampPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TTML_TIMESTAMP, 0x99990000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_TIMESTAMP);
        CPPUNIT_ASSERT(packet.getCounter() == 0x99990000);
    }

    void testParseValidCcDataPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::CC_DATA, 0xAAAABBBB);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::CC_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0xAAAABBBB);
    }

    void testParseValidPausePacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::PAUSE, 0xCCCCDDDD);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PAUSE);
        CPPUNIT_ASSERT(packet.getCounter() == 0xCCCCDDDD);
    }

    void testParseValidResumePacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESUME, 0xEEEEFFFF);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESUME);
        CPPUNIT_ASSERT(packet.getCounter() == 0xEEEEFFFF);
    }

    void testParseValidMutePacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::MUTE, 0x11110000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::MUTE);
        CPPUNIT_ASSERT(packet.getCounter() == 0x11110000);
    }

    void testParseValidUnmutePacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::UNMUTE, 0x22220000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::UNMUTE);
        CPPUNIT_ASSERT(packet.getCounter() == 0x22220000);
    }

    void testParseValidWebvttSelectionPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::WEBVTT_SELECTION, 0x33330000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x33330000);
    }

    void testParseValidWebvttDataPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::WEBVTT_DATA, 0x44440000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x44440000);
    }

    void testParseValidWebvttTimestampPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::WEBVTT_TIMESTAMP, 0x55550000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::WEBVTT_TIMESTAMP);
        CPPUNIT_ASSERT(packet.getCounter() == 0x55550000);
    }

    void testParseValidSetCCAttributesPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::SET_CC_ATTRIBUTES, 0x66660000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT(packet.getCounter() == 0x66660000);
    }

    void testParseValidTtmlInfoPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::TTML_INFO, 0x77770000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0x77770000);
    }

    void testParseValidFlushPacket()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::FLUSH, 0x88880000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::FLUSH);
        CPPUNIT_ASSERT(packet.getCounter() == 0x88880000);
    }

    void testParseUnknownPacketType()
    {
        PacketParser parser;

        // Create packet with unknown type (not in enum)
        std::uint8_t unknownTypeData[] = {
            0x64, 0x00, 0x00, 0x00, // type (100, not registered)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(unknownTypeData), std::end(unknownTypeData));
        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseInvalidTypeMarker()
    {
        PacketParser parser;

        // Create packet with INVALID type marker (0xFFFFFFFF)
        std::uint8_t invalidTypeData[] = {
            0xFF, 0xFF, 0xFF, 0xFF, // type (INVALID)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(invalidTypeData), std::end(invalidTypeData));
        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseTypeNotInRegisteredMap()
    {
        PacketParser parser;

        // Create packet with type value not in registered map
        std::uint8_t unregisteredTypeData[] = {
            0xAB, 0xCD, 0xEF, 0x12, // type (random value)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(unregisteredTypeData), std::end(unregisteredTypeData));
        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet without crashing
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseTypeZero()
    {
        PacketParser parser;

        // Create packet with type 0
        std::uint8_t zeroTypeData[] = {
            0x00, 0x00, 0x00, 0x00, // type (0)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(zeroTypeData), std::end(zeroTypeData));
        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseTypeMax()
    {
        PacketParser parser;

        // Create packet with Type::MAX value (should not be registered)
        std::uint32_t maxType = static_cast<std::uint32_t>(Packet::Type::MAX);
        std::uint8_t maxTypeData[] = {
            static_cast<std::uint8_t>(maxType & 0xFF),
            static_cast<std::uint8_t>((maxType >> 8) & 0xFF),
            static_cast<std::uint8_t>((maxType >> 16) & 0xFF),
            static_cast<std::uint8_t>((maxType >> 24) & 0xFF),
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(maxTypeData), std::end(maxTypeData));
        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseEmptyBuffer()
    {
        PacketParser parser;
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();

        const Packet& packet = parser.parse(std::move(emptyBuffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseBufferSizeLessThan4Bytes()
    {
        PacketParser parser;

        // Buffer with only 3 bytes (cannot extract type)
        std::uint8_t tinyData[] = {0x03, 0x00, 0x00};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(tinyData), std::end(tinyData));

        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseBufferSize4BytesTypeOnly()
    {
        PacketParser parser;

        // Buffer with only type field (4 bytes)
        std::uint8_t typeOnlyData[] = {0x03, 0x00, 0x00, 0x00};
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(typeOnlyData), std::end(typeOnlyData));

        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet (missing counter and size)
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseBufferSize8BytesTypeAndCounter()
    {
        PacketParser parser;

        // Buffer with type and counter only (8 bytes, missing size)
        std::uint8_t typeCounterData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(typeCounterData), std::end(typeCounterData));

        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet (missing size)
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseCorruptedPacketTypeField()
    {
        PacketParser parser;

        // Type field contains gibberish
        std::uint8_t corruptedData[] = {
            0xDE, 0xAD, 0xBE, 0xEF, // corrupted type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(corruptedData), std::end(corruptedData));

        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParseValidTypeButMalformedPayload()
    {
        PacketParser parser;

        // Valid type but incorrect size causes parsing to fail
        std::uint8_t malformedData[] = {
            0x03, 0x00, 0x00, 0x00, // type (RESET_ALL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x05, 0x00, 0x00, 0x00, // size (wrong, should be 0 for RESET_ALL)
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // extra data
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(malformedData), std::end(malformedData));

        const Packet& packet = parser.parse(std::move(buffer));

        // Should return invalid packet (parsing failed)
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testSequentialParseCallsWithDifferentTypes()
    {
        PacketParser parser;

        // Parse RESET_ALL
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::RESET_ALL, 0x11111111);
        const Packet& packet1 = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(packet1.isValid());
        CPPUNIT_ASSERT(packet1.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet1.getCounter() == 0x11111111);

        // Parse TIMESTAMP
        DataBufferPtr buffer2 = createValidPacket(Packet::Type::TIMESTAMP, 0x22222222);
        const Packet& packet2 = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(packet2.isValid());
        CPPUNIT_ASSERT(packet2.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet2.getCounter() == 0x22222222);

        // Parse PES_DATA
        DataBufferPtr buffer3 = createValidPacket(Packet::Type::PES_DATA, 0x33333333);
        const Packet& packet3 = parser.parse(std::move(buffer3));
        CPPUNIT_ASSERT(packet3.isValid());
        CPPUNIT_ASSERT(packet3.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet3.getCounter() == 0x33333333);

        // Parse PAUSE
        DataBufferPtr buffer4 = createValidPacket(Packet::Type::PAUSE, 0x44444444);
        const Packet& packet4 = parser.parse(std::move(buffer4));
        CPPUNIT_ASSERT(packet4.isValid());
        CPPUNIT_ASSERT(packet4.getType() == Packet::Type::PAUSE);
        CPPUNIT_ASSERT(packet4.getCounter() == 0x44444444);
    }

    void testParseReturnsInvalidAfterMultipleFailures()
    {
        PacketParser parser;

        // First invalid parse
        std::uint8_t invalidData1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidData1), std::end(invalidData1));
        const Packet& packet1 = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(!packet1.isValid());

        // Second invalid parse
        std::uint8_t invalidData2[] = {0xAA, 0xBB, 0xCC, 0xDD, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData2), std::end(invalidData2));
        const Packet& packet2 = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(!packet2.isValid());

        // Third invalid parse
        std::uint8_t invalidData3[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(invalidData3), std::end(invalidData3));
        const Packet& packet3 = parser.parse(std::move(buffer3));
        CPPUNIT_ASSERT(!packet3.isValid());
        CPPUNIT_ASSERT(packet3.getType() == Packet::Type::INVALID);
    }

    void testParseRecoveryAfterInvalidPacket()
    {
        PacketParser parser;

        // Parse invalid packet
        std::uint8_t invalidData[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        const Packet& invalidPacket = parser.parse(std::move(invalidBuffer));
        CPPUNIT_ASSERT(!invalidPacket.isValid());

        // Parse valid packet after invalid - parser should recover
        DataBufferPtr validBuffer = createValidPacket(Packet::Type::RESET_ALL, 0x12345678);
        const Packet& validPacket = parser.parse(std::move(validBuffer));
        CPPUNIT_ASSERT(validPacket.isValid());
        CPPUNIT_ASSERT(validPacket.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(validPacket.getCounter() == 0x12345678);
    }

    void testParsePacketReferenceSemantics()
    {
        PacketParser parser;

        // Parse first packet and get reference
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::RESET_ALL, 0xAAAAAAAA);
        const Packet& packet1 = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(packet1.isValid());
        CPPUNIT_ASSERT(packet1.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet1.getCounter() == 0xAAAAAAAA);

        // Parse second packet - first reference should be invalidated
        DataBufferPtr buffer2 = createValidPacket(Packet::Type::TIMESTAMP, 0xBBBBBBBB);
        const Packet& packet2 = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(packet2.isValid());
        CPPUNIT_ASSERT(packet2.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet2.getCounter() == 0xBBBBBBBB);

        // Note: packet1 reference may now point to different data (object reuse)
        // This is expected behavior per class documentation
    }

    void testParsePacketWithCounterZero()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_ALL, 0x00000000);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
    }

    void testParsePacketWithCounterMaxValue()
    {
        PacketParser parser;
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_ALL, 0xFFFFFFFF);

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getCounter() == 0xFFFFFFFF);
    }

    void testParseTruncatedHeader()
    {
        PacketParser parser;

        // Header with partial size field
        std::uint8_t truncatedData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, // partial size (only 2 bytes)
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(truncatedData), std::end(truncatedData));

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParsePacketWithWrongTypeValue()
    {
        PacketParser parser;

        // Type value that exists in enum but has wrong value bytes
        std::uint8_t wrongTypeData[] = {
            0x03, 0xFF, 0x00, 0x00, // type (partially correct, partially wrong)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(wrongTypeData), std::end(wrongTypeData));

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParsePacketWithInvalidSizeField()
    {
        PacketParser parser;

        // RESET_ALL should have size=0, but this has size=10
        std::uint8_t invalidSizeData[] = {
            0x03, 0x00, 0x00, 0x00, // type (RESET_ALL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x0A, 0x00, 0x00, 0x00, // size (wrong, should be 0)
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // extra data
        };
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(invalidSizeData), std::end(invalidSizeData));

        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testExtractTypeValidPacket()
    {
        PacketParser parser;

        // Create valid packet and verify type extraction
        DataBufferPtr buffer = createValidPacket(Packet::Type::TIMESTAMP, 0x12345678);
        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
    }

    void testExtractTypeInvalidPacket()
    {
        PacketParser parser;

        // Empty buffer should result in INVALID type
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        const Packet& packet = parser.parse(std::move(emptyBuffer));

        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::INVALID);
    }

    void testParserStateIsolationBetweenCalls()
    {
        PacketParser parser;

        // Parse first packet
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::PAUSE, 0x11111111);
        const Packet& packet1 = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(packet1.isValid());
        CPPUNIT_ASSERT(packet1.getType() == Packet::Type::PAUSE);
        CPPUNIT_ASSERT(packet1.getCounter() == 0x11111111);

        // Parse second packet with different type
        DataBufferPtr buffer2 = createValidPacket(Packet::Type::RESUME, 0x22222222);
        const Packet& packet2 = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(packet2.isValid());
        CPPUNIT_ASSERT(packet2.getType() == Packet::Type::RESUME);
        CPPUNIT_ASSERT(packet2.getCounter() == 0x22222222);

        // Packets should be independent
        CPPUNIT_ASSERT(packet1.getType() != packet2.getType());
        CPPUNIT_ASSERT(packet1.getCounter() != packet2.getCounter());
    }

    void testPacketReusePattern()
    {
        PacketParser parser;

        // Parser reuses internal packet objects
        // Parse same type multiple times
        for (int i = 0; i < 5; i++)
        {
            std::uint32_t counter = 0x10000000 * (i + 1);
            DataBufferPtr buffer = createValidPacket(Packet::Type::MUTE, counter);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::MUTE);
            CPPUNIT_ASSERT(packet.getCounter() == counter);
        }
    }

    void testPreviousPacketInvalidatedOnNewParse()
    {
        PacketParser parser;

        // Parse first packet
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::FLUSH, 0xAAAAAAAA);
        const Packet& firstPacket = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(firstPacket.isValid());
        CPPUNIT_ASSERT(firstPacket.getType() == Packet::Type::FLUSH);

        // Store type and counter from first packet
        Packet::Type firstType = firstPacket.getType();
        std::uint32_t firstCounter = firstPacket.getCounter();

        // Parse second packet
        DataBufferPtr buffer2 = createValidPacket(Packet::Type::MUTE, 0xBBBBBBBB);
        const Packet& secondPacket = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(secondPacket.isValid());
        CPPUNIT_ASSERT(secondPacket.getType() == Packet::Type::MUTE);

        // First packet reference values should remain consistent with what we stored
        CPPUNIT_ASSERT(firstType == Packet::Type::FLUSH);
        CPPUNIT_ASSERT(firstCounter == 0xAAAAAAAA);
    }

    void testParseAllRegisteredPacketTypes()
    {
        PacketParser parser;

        // Array of all registered packet types
        Packet::Type types[] = {
            Packet::Type::PES_DATA,
            Packet::Type::TIMESTAMP,
            Packet::Type::RESET_ALL,
            Packet::Type::RESET_CHANNEL,
            Packet::Type::SUBTITLE_SELECTION,
            Packet::Type::TELETEXT_SELECTION,
            Packet::Type::TTML_SELECTION,
            Packet::Type::TTML_DATA,
            Packet::Type::TTML_TIMESTAMP,
            Packet::Type::CC_DATA,
            Packet::Type::PAUSE,
            Packet::Type::RESUME,
            Packet::Type::MUTE,
            Packet::Type::UNMUTE,
            Packet::Type::WEBVTT_SELECTION,
            Packet::Type::WEBVTT_DATA,
            Packet::Type::WEBVTT_TIMESTAMP,
            Packet::Type::SET_CC_ATTRIBUTES,
            Packet::Type::TTML_INFO,
            Packet::Type::FLUSH
        };

        // Parse each packet type
        for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(types[i], 0x10000000 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == types[i]);
            CPPUNIT_ASSERT(packet.getCounter() == (0x10000000 + i));
        }
    }

    void testParseAlternatingPacketTypes()
    {
        PacketParser parser;

        // Alternate between different packet types
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::PES_DATA, 0x11111111);
        const Packet& packet1 = parser.parse(std::move(buffer1));
        CPPUNIT_ASSERT(packet1.isValid());
        CPPUNIT_ASSERT(packet1.getType() == Packet::Type::PES_DATA);

        DataBufferPtr buffer2 = createValidPacket(Packet::Type::TIMESTAMP, 0x22222222);
        const Packet& packet2 = parser.parse(std::move(buffer2));
        CPPUNIT_ASSERT(packet2.isValid());
        CPPUNIT_ASSERT(packet2.getType() == Packet::Type::TIMESTAMP);

        DataBufferPtr buffer3 = createValidPacket(Packet::Type::PES_DATA, 0x33333333);
        const Packet& packet3 = parser.parse(std::move(buffer3));
        CPPUNIT_ASSERT(packet3.isValid());
        CPPUNIT_ASSERT(packet3.getType() == Packet::Type::PES_DATA);

        DataBufferPtr buffer4 = createValidPacket(Packet::Type::TIMESTAMP, 0x44444444);
        const Packet& packet4 = parser.parse(std::move(buffer4));
        CPPUNIT_ASSERT(packet4.isValid());
        CPPUNIT_ASSERT(packet4.getType() == Packet::Type::TIMESTAMP);

        DataBufferPtr buffer5 = createValidPacket(Packet::Type::RESET_ALL, 0x55555555);
        const Packet& packet5 = parser.parse(std::move(buffer5));
        CPPUNIT_ASSERT(packet5.isValid());
        CPPUNIT_ASSERT(packet5.getType() == Packet::Type::RESET_ALL);
    }

    // Subtitle workflow: SELECTION -> DATA -> TIMESTAMP
    void testSubtitleWorkflowSequence()
    {
        PacketParser parser;

        // Step 1: Subtitle selection
        DataBufferPtr selectionBuffer = createValidPacket(Packet::Type::SUBTITLE_SELECTION, 0x00000001);
        const Packet& selection = parser.parse(std::move(selectionBuffer));
        CPPUNIT_ASSERT(selection.isValid());
        CPPUNIT_ASSERT(selection.getType() == Packet::Type::SUBTITLE_SELECTION);
        CPPUNIT_ASSERT(selection.getCounter() == 1);

        // Step 2: PES data packet
        DataBufferPtr dataBuffer = createValidPacket(Packet::Type::PES_DATA, 0x00000002);
        const Packet& data = parser.parse(std::move(dataBuffer));
        CPPUNIT_ASSERT(data.isValid());
        CPPUNIT_ASSERT(data.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(data.getCounter() == 2);

        // Step 3: Timestamp packet
        DataBufferPtr timestampBuffer = createValidPacket(Packet::Type::TIMESTAMP, 0x00000003);
        const Packet& timestamp = parser.parse(std::move(timestampBuffer));
        CPPUNIT_ASSERT(timestamp.isValid());
        CPPUNIT_ASSERT(timestamp.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(timestamp.getCounter() == 3);
    }

    // TTML workflow: SELECTION -> INFO -> DATA -> TIMESTAMP
    void testTtmlWorkflowComplete()
    {
        PacketParser parser;

        // Step 1: TTML selection
        DataBufferPtr selectionBuffer = createValidPacket(Packet::Type::TTML_SELECTION, 0x00000010);
        const Packet& selection = parser.parse(std::move(selectionBuffer));
        CPPUNIT_ASSERT(selection.isValid());
        CPPUNIT_ASSERT(selection.getType() == Packet::Type::TTML_SELECTION);

        // Step 2: TTML info
        DataBufferPtr infoBuffer = createValidPacket(Packet::Type::TTML_INFO, 0x00000011);
        const Packet& info = parser.parse(std::move(infoBuffer));
        CPPUNIT_ASSERT(info.isValid());
        CPPUNIT_ASSERT(info.getType() == Packet::Type::TTML_INFO);

        // Step 3: TTML data
        DataBufferPtr dataBuffer = createValidPacket(Packet::Type::TTML_DATA, 0x00000012);
        const Packet& data = parser.parse(std::move(dataBuffer));
        CPPUNIT_ASSERT(data.isValid());
        CPPUNIT_ASSERT(data.getType() == Packet::Type::TTML_DATA);

        // Step 4: TTML timestamp
        DataBufferPtr timestampBuffer = createValidPacket(Packet::Type::TTML_TIMESTAMP, 0x00000013);
        const Packet& timestamp = parser.parse(std::move(timestampBuffer));
        CPPUNIT_ASSERT(timestamp.isValid());
        CPPUNIT_ASSERT(timestamp.getType() == Packet::Type::TTML_TIMESTAMP);
    }

    // WebVTT workflow: SELECTION -> DATA -> TIMESTAMP
    void testWebvttWorkflowComplete()
    {
        PacketParser parser;

        // Step 1: WebVTT selection
        DataBufferPtr selectionBuffer = createValidPacket(Packet::Type::WEBVTT_SELECTION, 0x00000020);
        const Packet& selection = parser.parse(std::move(selectionBuffer));
        CPPUNIT_ASSERT(selection.isValid());
        CPPUNIT_ASSERT(selection.getType() == Packet::Type::WEBVTT_SELECTION);

        // Step 2: WebVTT data
        DataBufferPtr dataBuffer = createValidPacket(Packet::Type::WEBVTT_DATA, 0x00000021);
        const Packet& data = parser.parse(std::move(dataBuffer));
        CPPUNIT_ASSERT(data.isValid());
        CPPUNIT_ASSERT(data.getType() == Packet::Type::WEBVTT_DATA);

        // Step 3: WebVTT timestamp
        DataBufferPtr timestampBuffer = createValidPacket(Packet::Type::WEBVTT_TIMESTAMP, 0x00000022);
        const Packet& timestamp = parser.parse(std::move(timestampBuffer));
        CPPUNIT_ASSERT(timestamp.isValid());
        CPPUNIT_ASSERT(timestamp.getType() == Packet::Type::WEBVTT_TIMESTAMP);
    }

    // Closed caption workflow: CC_DATA -> SET_CC_ATTRIBUTES
    void testClosedCaptionWorkflow()
    {
        PacketParser parser;

        // Step 1: CC data
        DataBufferPtr dataBuffer = createValidPacket(Packet::Type::CC_DATA, 0x00000030);
        const Packet& data = parser.parse(std::move(dataBuffer));
        CPPUNIT_ASSERT(data.isValid());
        CPPUNIT_ASSERT(data.getType() == Packet::Type::CC_DATA);

        // Step 2: Set CC attributes
        DataBufferPtr attrBuffer = createValidPacket(Packet::Type::SET_CC_ATTRIBUTES, 0x00000031);
        const Packet& attr = parser.parse(std::move(attrBuffer));
        CPPUNIT_ASSERT(attr.isValid());
        CPPUNIT_ASSERT(attr.getType() == Packet::Type::SET_CC_ATTRIBUTES);
    }

    // Playback control: PAUSE -> RESUME
    void testPlaybackControlWorkflow()
    {
        PacketParser parser;

        // Pause
        DataBufferPtr pauseBuffer = createValidPacket(Packet::Type::PAUSE, 0x00000040);
        const Packet& pause = parser.parse(std::move(pauseBuffer));
        CPPUNIT_ASSERT(pause.isValid());
        CPPUNIT_ASSERT(pause.getType() == Packet::Type::PAUSE);

        // Resume
        DataBufferPtr resumeBuffer = createValidPacket(Packet::Type::RESUME, 0x00000041);
        const Packet& resume = parser.parse(std::move(resumeBuffer));
        CPPUNIT_ASSERT(resume.isValid());
        CPPUNIT_ASSERT(resume.getType() == Packet::Type::RESUME);
    }

    // Audio control: MUTE -> UNMUTE
    void testAudioControlWorkflow()
    {
        PacketParser parser;

        // Mute
        DataBufferPtr muteBuffer = createValidPacket(Packet::Type::MUTE, 0x00000050);
        const Packet& mute = parser.parse(std::move(muteBuffer));
        CPPUNIT_ASSERT(mute.isValid());
        CPPUNIT_ASSERT(mute.getType() == Packet::Type::MUTE);

        // Unmute
        DataBufferPtr unmuteBuffer = createValidPacket(Packet::Type::UNMUTE, 0x00000051);
        const Packet& unmute = parser.parse(std::move(unmuteBuffer));
        CPPUNIT_ASSERT(unmute.isValid());
        CPPUNIT_ASSERT(unmute.getType() == Packet::Type::UNMUTE);
    }

    // Channel reset: RESET_CHANNEL -> new data
    void testChannelResetWorkflow()
    {
        PacketParser parser;

        // Reset channel
        DataBufferPtr resetBuffer = createValidPacket(Packet::Type::RESET_CHANNEL, 0x00000060);
        const Packet& reset = parser.parse(std::move(resetBuffer));
        CPPUNIT_ASSERT(reset.isValid());
        CPPUNIT_ASSERT(reset.getType() == Packet::Type::RESET_CHANNEL);

        // New data after reset
        DataBufferPtr dataBuffer = createValidPacket(Packet::Type::PES_DATA, 0x00000061);
        const Packet& data = parser.parse(std::move(dataBuffer));
        CPPUNIT_ASSERT(data.isValid());
        CPPUNIT_ASSERT(data.getType() == Packet::Type::PES_DATA);
    }

    // Global reset: RESET_ALL -> new session
    void testGlobalResetWorkflow()
    {
        PacketParser parser;

        // Parse some packets
        DataBufferPtr buffer1 = createValidPacket(Packet::Type::PES_DATA, 0x00000070);
        parser.parse(std::move(buffer1));

        // Global reset
        DataBufferPtr resetBuffer = createValidPacket(Packet::Type::RESET_ALL, 0x00000071);
        const Packet& reset = parser.parse(std::move(resetBuffer));
        CPPUNIT_ASSERT(reset.isValid());
        CPPUNIT_ASSERT(reset.getType() == Packet::Type::RESET_ALL);

        // New session after reset
        DataBufferPtr newBuffer = createValidPacket(Packet::Type::SUBTITLE_SELECTION, 0x00000072);
        const Packet& newPacket = parser.parse(std::move(newBuffer));
        CPPUNIT_ASSERT(newPacket.isValid());
        CPPUNIT_ASSERT(newPacket.getType() == Packet::Type::SUBTITLE_SELECTION);
    }

    // Mixed valid and invalid packet stream
    void testMixedValidInvalidPacketStream()
    {
        PacketParser parser;

        // Valid packet 1
        DataBufferPtr valid1 = createValidPacket(Packet::Type::PES_DATA, 0x00000080);
        const Packet& p1 = parser.parse(std::move(valid1));
        CPPUNIT_ASSERT(p1.isValid());

        // Invalid packet
        std::uint8_t invalidData[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr invalid = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        const Packet& pInvalid = parser.parse(std::move(invalid));
        CPPUNIT_ASSERT(!pInvalid.isValid());

        // Valid packet 2 - parser should recover
        DataBufferPtr valid2 = createValidPacket(Packet::Type::TIMESTAMP, 0x00000081);
        const Packet& p2 = parser.parse(std::move(valid2));
        CPPUNIT_ASSERT(p2.isValid());

        // Another invalid
        std::uint8_t invalidData2[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr invalid2 = std::make_unique<DataBuffer>(std::begin(invalidData2), std::end(invalidData2));
        const Packet& pInvalid2 = parser.parse(std::move(invalid2));
        CPPUNIT_ASSERT(!pInvalid2.isValid());

        // Valid packet 3 - continue working
        DataBufferPtr valid3 = createValidPacket(Packet::Type::RESET_ALL, 0x00000082);
        const Packet& p3 = parser.parse(std::move(valid3));
        CPPUNIT_ASSERT(p3.isValid());
    }

    // Parser continues after invalid packet
    void testPacketStreamAfterInvalidPacket()
    {
        PacketParser parser;

        // Invalid packet
        std::uint8_t invalidData[] = {0xAB, 0xCD, 0xEF, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        const Packet& invalidPacket = parser.parse(std::move(invalidBuffer));
        CPPUNIT_ASSERT(!invalidPacket.isValid());

        // Stream of valid packets after invalid
        for (int i = 0; i < 5; i++)
        {
            DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, 0x00000090 + i);
            const Packet& packet = parser.parse(std::move(buffer));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
            CPPUNIT_ASSERT(packet.getCounter() == (0x00000090 + i));
        }
    }

    // Alternating packets from multiple channels
    void testAlternatingMultiChannelPackets()
    {
        PacketParser parser;

        // Channel 1 selection
        DataBufferPtr ch1_sel = createValidPacket(Packet::Type::SUBTITLE_SELECTION, 0x000000A0);
        const Packet& p1 = parser.parse(std::move(ch1_sel));
        CPPUNIT_ASSERT(p1.isValid());

        // Channel 2 selection
        DataBufferPtr ch2_sel = createValidPacket(Packet::Type::SUBTITLE_SELECTION, 0x000000A1);
        const Packet& p2 = parser.parse(std::move(ch2_sel));
        CPPUNIT_ASSERT(p2.isValid());

        // Channel 1 data
        DataBufferPtr ch1_data = createValidPacket(Packet::Type::PES_DATA, 0x000000A2);
        const Packet& p3 = parser.parse(std::move(ch1_data));
        CPPUNIT_ASSERT(p3.isValid());

        // Channel 2 data
        DataBufferPtr ch2_data = createValidPacket(Packet::Type::PES_DATA, 0x000000A3);
        const Packet& p4 = parser.parse(std::move(ch2_data));
        CPPUNIT_ASSERT(p4.isValid());
    }

    // Large data packet
    void testLargeDataPacketParsing()
    {
        PacketParser parser;

        // Create packet with larger payload
        std::vector<std::uint8_t> largeData;
        std::uint32_t typeValue = static_cast<std::uint32_t>(Packet::Type::PES_DATA);

        // Type
        largeData.push_back(typeValue & 0xFF);
        largeData.push_back((typeValue >> 8) & 0xFF);
        largeData.push_back((typeValue >> 16) & 0xFF);
        largeData.push_back((typeValue >> 24) & 0xFF);

        // Counter
        std::uint32_t counter = 0x000000B0;
        largeData.push_back(counter & 0xFF);
        largeData.push_back((counter >> 8) & 0xFF);
        largeData.push_back((counter >> 16) & 0xFF);
        largeData.push_back((counter >> 24) & 0xFF);

        // Size (8 header bytes + 1024 data bytes)
        std::uint32_t dataSize = 8 + 1024;
        largeData.push_back(dataSize & 0xFF);
        largeData.push_back((dataSize >> 8) & 0xFF);
        largeData.push_back((dataSize >> 16) & 0xFF);
        largeData.push_back((dataSize >> 24) & 0xFF);

        // Channel ID
        for (int i = 0; i < 4; i++) largeData.push_back(0x01);

        // Channel type
        for (int i = 0; i < 4; i++) largeData.push_back(0x01);

        // Large payload data (1024 bytes)
        for (int i = 0; i < 1024; i++) largeData.push_back(0xAA);

        DataBufferPtr buffer = std::make_unique<DataBuffer>(largeData.begin(), largeData.end());
        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x000000B0);
    }

    // Rapid succession of packets
    void testRapidSuccessionPackets()
    {
        PacketParser parser;

        // Parse 20 packets rapidly
        for (int i = 0; i < 20; i++)
        {
            Packet::Type type = (i % 2 == 0) ? Packet::Type::PES_DATA : Packet::Type::TIMESTAMP;
            DataBufferPtr buffer = createValidPacket(type, 0x000000C0 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == type);
            CPPUNIT_ASSERT(packet.getCounter() == (0x000000C0 + i));
        }
    }

    // Buffer integration
    void testBufferIntegration()
    {
        PacketParser parser;

        // Parser uses Buffer internally via extractType
        DataBufferPtr buffer = createValidPacket(Packet::Type::RESET_ALL, 0x000000D0);
        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    // DataBuffer lifecycle
    void testDataBufferLifecycle()
    {
        PacketParser parser;

        // DataBuffer ownership transferred to parser
        DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, 0x000000E0);
        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        // After parse, buffer ownership may be transferred to packet for data packets
    }

    // All packet types in single session
    void testAllPacketTypesInSession()
    {
        PacketParser parser;

        Packet::Type allTypes[] = {
            Packet::Type::PES_DATA,
            Packet::Type::TIMESTAMP,
            Packet::Type::RESET_ALL,
            Packet::Type::RESET_CHANNEL,
            Packet::Type::SUBTITLE_SELECTION,
            Packet::Type::TELETEXT_SELECTION,
            Packet::Type::TTML_SELECTION,
            Packet::Type::TTML_DATA,
            Packet::Type::TTML_TIMESTAMP,
            Packet::Type::CC_DATA,
            Packet::Type::PAUSE,
            Packet::Type::RESUME,
            Packet::Type::MUTE,
            Packet::Type::UNMUTE,
            Packet::Type::WEBVTT_SELECTION,
            Packet::Type::WEBVTT_DATA,
            Packet::Type::WEBVTT_TIMESTAMP,
            Packet::Type::SET_CC_ATTRIBUTES,
            Packet::Type::TTML_INFO,
            Packet::Type::FLUSH
        };

        // Parse all types without interference
        for (size_t i = 0; i < sizeof(allTypes) / sizeof(allTypes[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(allTypes[i], 0x000000F0 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == allTypes[i]);
        }
    }

    // Counter progression
    void testCounterProgression()
    {
        PacketParser parser;

        // Sequential packets with incrementing counters
        for (std::uint32_t counter = 1; counter <= 10; counter++)
        {
            DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, counter);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getCounter() == counter);
        }
    }

    // Flush followed by new data
    void testFlushFollowedByNewData()
    {
        PacketParser parser;

        // Parse some data
        DataBufferPtr data1 = createValidPacket(Packet::Type::PES_DATA, 0x00000100);
        parser.parse(std::move(data1));

        // Flush
        DataBufferPtr flushBuffer = createValidPacket(Packet::Type::FLUSH, 0x00000101);
        const Packet& flush = parser.parse(std::move(flushBuffer));
        CPPUNIT_ASSERT(flush.isValid());
        CPPUNIT_ASSERT(flush.getType() == Packet::Type::FLUSH);

        // New data after flush
        DataBufferPtr data2 = createValidPacket(Packet::Type::PES_DATA, 0x00000102);
        const Packet& newData = parser.parse(std::move(data2));
        CPPUNIT_ASSERT(newData.isValid());
        CPPUNIT_ASSERT(newData.getType() == Packet::Type::PES_DATA);
    }

    // PacketData polymorphism for all data types
    void testPacketDataPolymorphism()
    {
        PacketParser parser;

        Packet::Type dataTypes[] = {
            Packet::Type::PES_DATA,
            Packet::Type::TTML_DATA,
            Packet::Type::CC_DATA,
            Packet::Type::WEBVTT_DATA
        };

        // All should use PacketData implementation
        for (size_t i = 0; i < sizeof(dataTypes) / sizeof(dataTypes[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(dataTypes[i], 0x00000110 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == dataTypes[i]);
        }
    }

    // PacketGeneric subtypes
    void testPacketGenericSubtypes()
    {
        PacketParser parser;

        Packet::Type genericTypes[] = {
            Packet::Type::RESET_ALL,
            Packet::Type::PAUSE,
            Packet::Type::RESUME,
            Packet::Type::MUTE,
            Packet::Type::UNMUTE,
            Packet::Type::FLUSH
        };

        // All control packets should parse correctly
        for (size_t i = 0; i < sizeof(genericTypes) / sizeof(genericTypes[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(genericTypes[i], 0x00000120 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == genericTypes[i]);
        }
    }

    // PacketChannelSpecific subtypes
    void testPacketChannelSpecificSubtypes()
    {
        PacketParser parser;

        Packet::Type channelTypes[] = {
            Packet::Type::RESET_CHANNEL,
            Packet::Type::SUBTITLE_SELECTION,
            Packet::Type::TELETEXT_SELECTION,
            Packet::Type::TTML_SELECTION,
            Packet::Type::WEBVTT_SELECTION,
            Packet::Type::SET_CC_ATTRIBUTES,
            Packet::Type::TTML_INFO
        };

        // All channel-specific packets should correctly parse channel ID
        for (size_t i = 0; i < sizeof(channelTypes) / sizeof(channelTypes[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(channelTypes[i], 0x00000130 + i);
            const Packet& packet = parser.parse(std::move(buffer));

            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == channelTypes[i]);
        }
    }

    // Complete subtitle session simulation
    void testCompleteSubtitleSessionSimulation()
    {
        PacketParser parser;
        std::uint32_t counter = 0;

        // Session start: Selection
        DataBufferPtr sel = createValidPacket(Packet::Type::SUBTITLE_SELECTION, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(sel)).isValid());

        // Subtitle data stream
        for (int i = 0; i < 5; i++)
        {
            // Data packet
            DataBufferPtr data = createValidPacket(Packet::Type::PES_DATA, counter++);
            CPPUNIT_ASSERT(parser.parse(std::move(data)).isValid());

            // Timestamp
            DataBufferPtr ts = createValidPacket(Packet::Type::TIMESTAMP, counter++);
            CPPUNIT_ASSERT(parser.parse(std::move(ts)).isValid());
        }

        // Pause
        DataBufferPtr pause = createValidPacket(Packet::Type::PAUSE, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(pause)).isValid());

        // Resume
        DataBufferPtr resume = createValidPacket(Packet::Type::RESUME, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(resume)).isValid());

        // More data
        DataBufferPtr data = createValidPacket(Packet::Type::PES_DATA, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(data)).isValid());

        // Session end: Reset
        DataBufferPtr reset = createValidPacket(Packet::Type::RESET_ALL, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(reset)).isValid());
    }

    // Channel switching scenario
    void testChannelSwitchingScenario()
    {
        PacketParser parser;
        std::uint32_t counter = 0;

        // Channel 1 active
        DataBufferPtr ch1_sel = createValidPacket(Packet::Type::SUBTITLE_SELECTION, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(ch1_sel)).isValid());

        DataBufferPtr ch1_data = createValidPacket(Packet::Type::PES_DATA, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(ch1_data)).isValid());

        // Reset channel 1
        DataBufferPtr ch1_reset = createValidPacket(Packet::Type::RESET_CHANNEL, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(ch1_reset)).isValid());

        // Switch to channel 2
        DataBufferPtr ch2_sel = createValidPacket(Packet::Type::SUBTITLE_SELECTION, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(ch2_sel)).isValid());

        DataBufferPtr ch2_data = createValidPacket(Packet::Type::PES_DATA, counter++);
        CPPUNIT_ASSERT(parser.parse(std::move(ch2_data)).isValid());
    }

    // Error recovery in stream
    void testErrorRecoveryInStream()
    {
        PacketParser parser;
        std::uint32_t counter = 0;

        // Valid packets
        for (int i = 0; i < 3; i++)
        {
            DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, counter++);
            CPPUNIT_ASSERT(parser.parse(std::move(buffer)).isValid());
        }

        // Corrupted packet
        std::uint8_t corruptedData[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr corrupt = std::make_unique<DataBuffer>(std::begin(corruptedData), std::end(corruptedData));
        CPPUNIT_ASSERT(!parser.parse(std::move(corrupt)).isValid());

        // Recovery: continue with valid packets
        for (int i = 0; i < 3; i++)
        {
            DataBufferPtr buffer = createValidPacket(Packet::Type::PES_DATA, counter++);
            CPPUNIT_ASSERT(parser.parse(std::move(buffer)).isValid());
        }
    }

    void testSubtitleSelectionToDataToTimestamp()
    {
        PacketParser parser;
        std::uint32_t counter = 100;

        // Selection packet
        DataBufferPtr buf1 = createValidPacket(Packet::Type::SUBTITLE_SELECTION, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());
        CPPUNIT_ASSERT(pkt1.getType() == Packet::Type::SUBTITLE_SELECTION);
        CPPUNIT_ASSERT(pkt1.getCounter() == 100);

        // Multiple data packets
        for (int i = 0; i < 3; i++)
        {
            DataBufferPtr buf = createValidPacket(Packet::Type::PES_DATA, counter++);
            const Packet& pkt = parser.parse(std::move(buf));
            CPPUNIT_ASSERT(pkt.isValid());
            CPPUNIT_ASSERT(pkt.getType() == Packet::Type::PES_DATA);
        }

        // Timestamp packet
        DataBufferPtr buf2 = createValidPacket(Packet::Type::TIMESTAMP, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());
        CPPUNIT_ASSERT(pkt2.getType() == Packet::Type::TIMESTAMP);
    }

    void testTtmlSelectionInfoDataTimestamp()
    {
        PacketParser parser;

        // Complete TTML workflow
        Packet::Type workflow[] = {
            Packet::Type::TTML_SELECTION,
            Packet::Type::TTML_INFO,
            Packet::Type::TTML_DATA,
            Packet::Type::TTML_TIMESTAMP
        };

        for (size_t i = 0; i < sizeof(workflow) / sizeof(workflow[0]); i++)
        {
            DataBufferPtr buffer = createValidPacket(workflow[i], i + 1);
            const Packet& packet = parser.parse(std::move(buffer));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getType() == workflow[i]);
            CPPUNIT_ASSERT(packet.getCounter() == (i + 1));
        }
    }

    void testWebvttSelectionDataTimestamp()
    {
        PacketParser parser;

        // WebVTT workflow with multiple data packets
        DataBufferPtr buf1 = createValidPacket(Packet::Type::WEBVTT_SELECTION, 1);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());

        // Multiple data packets
        for (std::uint32_t i = 2; i < 7; i++)
        {
            DataBufferPtr buf = createValidPacket(Packet::Type::WEBVTT_DATA, i);
            const Packet& pkt = parser.parse(std::move(buf));
            CPPUNIT_ASSERT(pkt.isValid());
            CPPUNIT_ASSERT(pkt.getCounter() == i);
        }

        DataBufferPtr buf2 = createValidPacket(Packet::Type::WEBVTT_TIMESTAMP, 7);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());
    }

    void testCcDataWithAttributesWorkflow()
    {
        PacketParser parser;

        // CC Data
        DataBufferPtr buf1 = createValidPacket(Packet::Type::CC_DATA, 1);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());
        CPPUNIT_ASSERT(pkt1.getType() == Packet::Type::CC_DATA);

        // Set CC Attributes
        DataBufferPtr buf2 = createValidPacket(Packet::Type::SET_CC_ATTRIBUTES, 2);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());
        CPPUNIT_ASSERT(pkt2.getType() == Packet::Type::SET_CC_ATTRIBUTES);

        // More CC Data
        DataBufferPtr buf3 = createValidPacket(Packet::Type::CC_DATA, 3);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());
        CPPUNIT_ASSERT(pkt3.getType() == Packet::Type::CC_DATA);
    }

    void testCcDataSequenceWithTimestamps()
    {
        PacketParser parser;
        std::uint32_t counter = 400;

        for (int cycle = 0; cycle < 3; cycle++)
        {
            // CC Data
            DataBufferPtr buf1 = createValidPacket(Packet::Type::CC_DATA, counter++);
            const Packet& pkt1 = parser.parse(std::move(buf1));
            CPPUNIT_ASSERT(pkt1.isValid());

            // Timestamp
            DataBufferPtr buf2 = createValidPacket(Packet::Type::TIMESTAMP, counter++);
            const Packet& pkt2 = parser.parse(std::move(buf2));
            CPPUNIT_ASSERT(pkt2.isValid());
        }
    }

    void testPauseResumeWithDataPackets()
    {
        PacketParser parser;
        std::uint32_t counter = 500;

        // Data before pause
        DataBufferPtr buf1 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());

        // Pause
        DataBufferPtr buf2 = createValidPacket(Packet::Type::PAUSE, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());

        // Resume
        DataBufferPtr buf3 = createValidPacket(Packet::Type::RESUME, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());

        // Data after resume
        DataBufferPtr buf4 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt4 = parser.parse(std::move(buf4));
        CPPUNIT_ASSERT(pkt4.isValid());
    }

    void testMuteUnmuteWithSubtitles()
    {
        PacketParser parser;
        std::uint32_t counter = 600;

        // Subtitle data
        DataBufferPtr buf1 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());

        // Mute
        DataBufferPtr buf2 = createValidPacket(Packet::Type::MUTE, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());

        // More subtitle data while muted
        DataBufferPtr buf3 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());

        // Unmute
        DataBufferPtr buf4 = createValidPacket(Packet::Type::UNMUTE, counter++);
        const Packet& pkt4 = parser.parse(std::move(buf4));
        CPPUNIT_ASSERT(pkt4.isValid());
    }

    void testResetChannelFollowedByNewData()
    {
        PacketParser parser;
        std::uint32_t counter = 700;

        // Data on channel
        DataBufferPtr buf1 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());

        // Reset channel
        DataBufferPtr buf2 = createValidPacket(Packet::Type::RESET_CHANNEL, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());

        // New data after reset
        DataBufferPtr buf3 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());
        CPPUNIT_ASSERT(pkt3.getType() == Packet::Type::PES_DATA);
    }

    void testResetAllFollowedByNewSession()
    {
        PacketParser parser;
        std::uint32_t counter = 800;

        // Old session data
        for (int i = 0; i < 5; i++)
        {
            DataBufferPtr buf = createValidPacket(Packet::Type::PES_DATA, counter++);
            parser.parse(std::move(buf));
        }

        // Reset all
        DataBufferPtr buf1 = createValidPacket(Packet::Type::RESET_ALL, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());

        // New session
        DataBufferPtr buf2 = createValidPacket(Packet::Type::SUBTITLE_SELECTION, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());

        DataBufferPtr buf3 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());
    }

    void testStreamWithIntermittentInvalidPackets()
    {
        PacketParser parser;
        std::uint32_t counter = 900;

        for (int i = 0; i < 10; i++)
        {
            if (i % 3 == 0)
            {
                // Invalid packet
                std::uint8_t invalidData[] = {0xAB, 0xCD, 0xEF, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                DataBufferPtr buf = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
                const Packet& pkt = parser.parse(std::move(buf));
                CPPUNIT_ASSERT(!pkt.isValid());
            }
            else
            {
                // Valid packet
                DataBufferPtr buf = createValidPacket(Packet::Type::PES_DATA, counter++);
                const Packet& pkt = parser.parse(std::move(buf));
                CPPUNIT_ASSERT(pkt.isValid());
            }
        }
    }

    void testMultipleChannelsWithResets()
    {
        PacketParser parser;
        std::uint32_t counter = 1100;

        // Channel 1 data
        DataBufferPtr buf1 = createValidPacket(Packet::Type::PES_DATA, counter++);
        parser.parse(std::move(buf1));

        // Channel 2 data
        DataBufferPtr buf2 = createValidPacket(Packet::Type::PES_DATA, counter++);
        parser.parse(std::move(buf2));

        // Reset channel 1
        DataBufferPtr buf3 = createValidPacket(Packet::Type::RESET_CHANNEL, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());

        // Channel 2 continues
        DataBufferPtr buf4 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt4 = parser.parse(std::move(buf4));
        CPPUNIT_ASSERT(pkt4.isValid());

        // New data on channel 1
        DataBufferPtr buf5 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt5 = parser.parse(std::move(buf5));
        CPPUNIT_ASSERT(pkt5.isValid());
    }

    void testInterleavedMultiChannelStream()
    {
        PacketParser parser;
        std::uint32_t counter = 1200;

        // Simulate 3 channels with interleaved data
        for (int round = 0; round < 5; round++)
        {
            for (int channelId = 1; channelId <= 3; channelId++)
            {
                DataBufferPtr buf = createValidPacket(Packet::Type::PES_DATA, counter++);
                const Packet& pkt = parser.parse(std::move(buf));
                CPPUNIT_ASSERT(pkt.isValid());
                CPPUNIT_ASSERT(pkt.getType() == Packet::Type::PES_DATA);
            }
        }
    }

    void testMaximumSizeDataPacket()
    {
        PacketParser parser;

        // Create maximum reasonable size packet (10KB)
        std::uint32_t typeValue = static_cast<std::uint32_t>(Packet::Type::TTML_DATA);
        std::vector<std::uint8_t> data;

        // Header
        data.push_back(typeValue & 0xFF);
        data.push_back((typeValue >> 8) & 0xFF);
        data.push_back((typeValue >> 16) & 0xFF);
        data.push_back((typeValue >> 24) & 0xFF);
        data.push_back(0xFF);
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);

        // Size (10000 bytes)
        std::uint32_t maxSize = 10000;
        data.push_back(maxSize & 0xFF);
        data.push_back((maxSize >> 8) & 0xFF);
        data.push_back((maxSize >> 16) & 0xFF);
        data.push_back((maxSize >> 24) & 0xFF);

        // Channel ID + type
        for (int i = 0; i < 8; i++) data.push_back(0x02);

        // Large payload
        for (size_t i = 0; i < maxSize - 8; i++) data.push_back(0x54); // 'T'

        DataBufferPtr buffer = std::make_unique<DataBuffer>(data.begin(), data.end());
        const Packet& packet = parser.parse(std::move(buffer));

        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_DATA);
    }

    void testContinuousStreamParsing()
    {
        PacketParser parser;
        std::uint32_t counter = 3000;

        // Simulate continuous stream of various packet types
        Packet::Type types[] = {
            Packet::Type::SUBTITLE_SELECTION,
            Packet::Type::PES_DATA,
            Packet::Type::PES_DATA,
            Packet::Type::TIMESTAMP,
            Packet::Type::PES_DATA,
            Packet::Type::TIMESTAMP
        };

        // Repeat pattern 10 times
        for (int cycle = 0; cycle < 10; cycle++)
        {
            for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
            {
                DataBufferPtr buf = createValidPacket(types[i], counter++);
                const Packet& pkt = parser.parse(std::move(buf));
                CPPUNIT_ASSERT(pkt.isValid());
            }
        }
    }

    void testCounterProgressionAcrossPackets()
    {
        PacketParser parser;

        // Parse packets with incrementing counter
        for (std::uint32_t counter = 1; counter <= 50; counter++)
        {
            DataBufferPtr buf = createValidPacket(Packet::Type::PES_DATA, counter);
            const Packet& pkt = parser.parse(std::move(buf));
            CPPUNIT_ASSERT(pkt.isValid());
            CPPUNIT_ASSERT(pkt.getCounter() == counter);
        }
    }

    void testCounterWrapAround()
    {
        PacketParser parser;

        // Test counter wrap-around at maximum value
        std::uint32_t counters[] = {
            0xFFFFFFFD,
            0xFFFFFFFE,
            0xFFFFFFFF,
            0x00000000,
            0x00000001
        };

        for (size_t i = 0; i < sizeof(counters) / sizeof(counters[0]); i++)
        {
            DataBufferPtr buf = createValidPacket(Packet::Type::TIMESTAMP, counters[i]);
            const Packet& pkt = parser.parse(std::move(buf));
            CPPUNIT_ASSERT(pkt.isValid());
            CPPUNIT_ASSERT(pkt.getCounter() == counters[i]);
        }
    }

    void testFlushInMultiChannelScenario()
    {
        PacketParser parser;
        std::uint32_t counter = 4100;

        // Multiple channels active
        DataBufferPtr buf1 = createValidPacket(Packet::Type::PES_DATA, counter++);
        parser.parse(std::move(buf1));

        DataBufferPtr buf2 = createValidPacket(Packet::Type::PES_DATA, counter++);
        parser.parse(std::move(buf2));

        // Flush all
        DataBufferPtr buf3 = createValidPacket(Packet::Type::FLUSH, counter++);
        const Packet& pkt3 = parser.parse(std::move(buf3));
        CPPUNIT_ASSERT(pkt3.isValid());

        // Resume with new data
        DataBufferPtr buf4 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt4 = parser.parse(std::move(buf4));
        CPPUNIT_ASSERT(pkt4.isValid());
    }

    void testMultiFormatSession()
    {
        PacketParser parser;
        std::uint32_t counter = 6000;

        // Session with multiple subtitle formats

        // TTML stream
        DataBufferPtr buf1 = createValidPacket(Packet::Type::TTML_SELECTION, counter++);
        parser.parse(std::move(buf1));

        DataBufferPtr buf2 = createValidPacket(Packet::Type::TTML_DATA, counter++);
        parser.parse(std::move(buf2));

        // WebVTT stream
        DataBufferPtr buf3 = createValidPacket(Packet::Type::WEBVTT_SELECTION, counter++);
        parser.parse(std::move(buf3));

        DataBufferPtr buf4 = createValidPacket(Packet::Type::WEBVTT_DATA, counter++);
        parser.parse(std::move(buf4));

        // CC stream
        DataBufferPtr buf5 = createValidPacket(Packet::Type::CC_DATA, counter++);
        const Packet& pkt5 = parser.parse(std::move(buf5));
        CPPUNIT_ASSERT(pkt5.isValid());
        CPPUNIT_ASSERT(pkt5.getType() == Packet::Type::CC_DATA);
    }

    void testRecoveryAfterMultipleFailures()
    {
        PacketParser parser;
        std::uint32_t counter = 8000;

        // Multiple corrupted packets
        for (int i = 0; i < 5; i++)
        {
            std::uint8_t badData[] = {0xFF, 0xFF, 0xFF, 0xFF, static_cast<std::uint8_t>(i), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            DataBufferPtr buf = std::make_unique<DataBuffer>(std::begin(badData), std::end(badData));
            const Packet& pkt = parser.parse(std::move(buf));
            CPPUNIT_ASSERT(!pkt.isValid());
        }

        // Parser should still work after multiple failures
        DataBufferPtr buf1 = createValidPacket(Packet::Type::RESET_ALL, counter++);
        const Packet& pkt1 = parser.parse(std::move(buf1));
        CPPUNIT_ASSERT(pkt1.isValid());
        CPPUNIT_ASSERT(pkt1.getType() == Packet::Type::RESET_ALL);

        // Continue with normal packets
        DataBufferPtr buf2 = createValidPacket(Packet::Type::PES_DATA, counter++);
        const Packet& pkt2 = parser.parse(std::move(buf2));
        CPPUNIT_ASSERT(pkt2.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PacketParserTest);
