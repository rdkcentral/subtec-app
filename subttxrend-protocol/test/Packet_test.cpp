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

#include "Packet.hpp"
#include "PacketResetAll.hpp"
#include "PacketResetChannel.hpp"
#include "Buffer.hpp"
#include "BufferReader.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketGeneric;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketResetAll;
using subttxrend::protocol::PacketResetChannel;
using subttxrend::protocol::Buffer;
using subttxrend::protocol::BufferReader;

class PacketTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTest );
    CPPUNIT_TEST(testExtractTypeValid);
    CPPUNIT_TEST(testExtractTypeInsufficientData);
    CPPUNIT_TEST(testExtractTypeEmpty);
    CPPUNIT_TEST(testExtractTypeLittleEndian);
    CPPUNIT_TEST(testExtractTypeInvalidValue);
    CPPUNIT_TEST(testGetTypeFromDataBuffer);
    CPPUNIT_TEST(testGetTypeTimestamp);
    CPPUNIT_TEST(testGetTypeInvalid);
    CPPUNIT_TEST(testGetTypeTruncated);
    CPPUNIT_TEST(testGetCounterValid);
    CPPUNIT_TEST(testGetCounterInsufficientData);
    CPPUNIT_TEST(testGetCounterLittleEndian);
    CPPUNIT_TEST(testGetCounterEmpty);
    CPPUNIT_TEST(testIsDataPacketTTML);
    CPPUNIT_TEST(testIsDataPacketPES);
    CPPUNIT_TEST(testIsDataPacketCC);
    CPPUNIT_TEST(testIsDataPacketWEBVTT);
    CPPUNIT_TEST(testIsDataPacketTimestamp);
    CPPUNIT_TEST(testIsDataPacketResetAll);
    CPPUNIT_TEST(testIsDataPacketInvalid);
    CPPUNIT_TEST(testGetHeaderSize);
    CPPUNIT_TEST(testGetSizeFromHeader);
    CPPUNIT_TEST(testGetSizeFromHeaderLittleEndian);
    CPPUNIT_TEST(testPacketGenericConstructorResetAll);
    CPPUNIT_TEST(testPacketGenericConstructorTimestamp);
    CPPUNIT_TEST(testPacketGenericConstructorPause);
    CPPUNIT_TEST(testPacketGenericGetType);
    CPPUNIT_TEST(testPacketChannelSpecificConstructor);
    CPPUNIT_TEST(testPacketChannelSpecificGetTypeResetChannel);
    CPPUNIT_TEST(testPacketChannelSpecificGetTypeMute);
    CPPUNIT_TEST(testPacketChannelSpecificGetChannelIdBeforeParse);
    CPPUNIT_TEST(testPacketParseValidMinimal);
    CPPUNIT_TEST(testPacketParseValidWithZeroCounter);
    CPPUNIT_TEST(testPacketParseValidWithMaxCounter);
    CPPUNIT_TEST(testPacketParseValidWithZeroSize);
    CPPUNIT_TEST(testPacketParseValidWithNonZeroSize);
    CPPUNIT_TEST(testPacketParseMismatchedType);
    CPPUNIT_TEST(testPacketParseInvalidType);
    CPPUNIT_TEST(testPacketParseTypeZero);
    CPPUNIT_TEST(testPacketParseTruncatedType);
    CPPUNIT_TEST(testPacketParseTruncatedCounter);
    CPPUNIT_TEST(testPacketParseTruncatedSize);
    CPPUNIT_TEST(testPacketParseSizeMismatchLarger);
    CPPUNIT_TEST(testPacketParseSizeMismatchSmaller);
    CPPUNIT_TEST(testPacketParseEmptyBuffer);
    CPPUNIT_TEST(testPacketParseOneByteBuffer);
    CPPUNIT_TEST(testPacketParseElevenByteBuffer);
    CPPUNIT_TEST(testPacketParseLittleEndianType);
    CPPUNIT_TEST(testPacketParseLittleEndianCounter);
    CPPUNIT_TEST(testPacketParseLittleEndianSize);
    CPPUNIT_TEST(testPacketParseAlternatingPattern);
    CPPUNIT_TEST(testPacketParseMultipleTimes);
    CPPUNIT_TEST(testPacketParseAfterFailedParse);
    CPPUNIT_TEST(testPacketParseMultipleFailures);
    CPPUNIT_TEST(testPacketParseGettersAfterSuccess);
    CPPUNIT_TEST(testPacketParseWithPayload);
    CPPUNIT_TEST(testPacketParseParseDataHeaderFails);
    CPPUNIT_TEST(testPacketInitialState);
    CPPUNIT_TEST(testPacketIsValidAfterSuccessfulParse);
    CPPUNIT_TEST(testPacketIsValidAfterFailedParse);
    CPPUNIT_TEST(testEndToEndPacketResetAll);
    CPPUNIT_TEST(testEndToEndPacketResetChannel);
    CPPUNIT_TEST(testEndToEndPacketResetChannelInvalidSize);
    CPPUNIT_TEST(testPolymorphismGetTypeMultipleTypes);
    CPPUNIT_TEST(testPolymorphismParseMultipleTypes);
    CPPUNIT_TEST(testStateConsistencyAcrossGetters);
    CPPUNIT_TEST(testStateConsistencyFailureToSuccess);
    CPPUNIT_TEST(testStateConsistencyZeroSize);
    CPPUNIT_TEST(testBufferReaderIntegrationHeaderParsing);
    CPPUNIT_TEST(testBufferReaderIntegrationPayloadParsing);
    CPPUNIT_TEST(testBufferReaderIntegrationOffsetTracking);
    CPPUNIT_TEST(testErrorRecoverySequentialParsing);
    CPPUNIT_TEST(testErrorRecoveryAlternatingValidInvalid);
    CPPUNIT_TEST(testErrorRecoveryCorruptedData);
    CPPUNIT_TEST(testTypeIdentificationExtractThenParse);
    CPPUNIT_TEST(testTypeIdentificationGetTypeFactory);
    CPPUNIT_TEST(testTypeIdentificationIsDataPacketFiltering);
    CPPUNIT_TEST(testEdgeCaseChannelIdBoundaries);
    CPPUNIT_TEST(testEdgeCaseCounterSequenceWrapping);
    CPPUNIT_TEST(testEdgeCaseZeroLengthPayload);
    CPPUNIT_TEST(testProtocolComplianceHeaderStructure);
    CPPUNIT_TEST(testProtocolComplianceFieldOrdering);
    CPPUNIT_TEST(testProtocolComplianceLittleEndianConsistency);
    CPPUNIT_TEST(testMemorySafetyLargeSizeValue);
    CPPUNIT_TEST(testMemorySafetySequentialReuseNoLeaks);
    CPPUNIT_TEST(testIntegrationStaticHelpersCombined);
    CPPUNIT_TEST(testIntegrationParseWithAllStaticHelpers);
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

    void testExtractTypeValid()
    {
        std::uint8_t data[] = {0x02, 0x00, 0x00, 0x00, 0xFF, 0xFF};
        Buffer buffer(reinterpret_cast<const char*>(data), sizeof(data));

        std::uint32_t type = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), type);
    }

    void testExtractTypeInsufficientData()
    {
        std::uint8_t data[] = {0x02, 0x00, 0x00};
        Buffer buffer(reinterpret_cast<const char*>(data), sizeof(data));

        std::uint32_t type = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(Packet::Type::INVALID), type);
    }

    void testExtractTypeEmpty()
    {
        std::uint8_t data[] = {};
        Buffer buffer(reinterpret_cast<const char*>(data), 0);

        std::uint32_t type = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(Packet::Type::INVALID), type);
    }

    void testExtractTypeLittleEndian()
    {
        std::uint8_t data[] = {0x01, 0x00, 0x00, 0x00};
        Buffer buffer(reinterpret_cast<const char*>(data), sizeof(data));

        std::uint32_t type = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), type);
    }

    void testExtractTypeInvalidValue()
    {
        std::uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
        Buffer buffer(reinterpret_cast<const char*>(data), sizeof(data));

        std::uint32_t type = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(Packet::Type::INVALID), type);
    }

    void testGetTypeFromDataBuffer()
    {
        std::uint8_t data[] = {0x02, 0x00, 0x00, 0x00, 0xFF, 0xFF};
        DataBuffer buffer(std::begin(data), std::end(data));

        Packet::Type type = Packet::getType(buffer);
        CPPUNIT_ASSERT(type == Packet::Type::TIMESTAMP);
    }

    void testGetTypeTimestamp()
    {
        std::uint8_t data[] = {0x02, 0x00, 0x00, 0x00};
        DataBuffer buffer(std::begin(data), std::end(data));

        Packet::Type type = Packet::getType(buffer);
        CPPUNIT_ASSERT(type == Packet::Type::TIMESTAMP);
    }

    void testGetTypeInvalid()
    {
        std::uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
        DataBuffer buffer(std::begin(data), std::end(data));

        Packet::Type type = Packet::getType(buffer);
        CPPUNIT_ASSERT(type == Packet::Type::INVALID);
    }

    void testGetTypeTruncated()
    {
        std::uint8_t data[] = {0x02, 0x00};
        DataBuffer buffer(std::begin(data), std::end(data));

        Packet::Type type = Packet::getType(buffer);
        CPPUNIT_ASSERT(type == Packet::Type::INVALID);
    }

    void testGetCounterValid()
    {
        std::uint8_t data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78  // counter
        };
        DataBuffer buffer(std::begin(data), std::end(data));

        std::uint32_t counter = Packet::getCounter(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x78563412), counter);
    }

    void testGetCounterInsufficientData()
    {
        std::uint8_t data[] = {0x02, 0x00, 0x00, 0x00};
        DataBuffer buffer(std::begin(data), std::end(data));

        std::uint32_t counter = Packet::getCounter(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);
    }

    void testGetCounterLittleEndian()
    {
        std::uint8_t data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04  // counter
        };
        DataBuffer buffer(std::begin(data), std::end(data));

        std::uint32_t counter = Packet::getCounter(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), counter);
    }

    void testGetCounterEmpty()
    {
        DataBuffer buffer;

        std::uint32_t counter = Packet::getCounter(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);
    }

    void testIsDataPacketTTML()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::TTML_DATA) == true);
    }

    void testIsDataPacketPES()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::PES_DATA) == true);
    }

    void testIsDataPacketCC()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::CC_DATA) == true);
    }

    void testIsDataPacketWEBVTT()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::WEBVTT_DATA) == true);
    }

    void testIsDataPacketTimestamp()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::TIMESTAMP) == false);
    }

    void testIsDataPacketResetAll()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::RESET_ALL) == false);
    }

    void testIsDataPacketInvalid()
    {
        CPPUNIT_ASSERT(Packet::isDataPacket(Packet::Type::INVALID) == false);
    }

    void testGetHeaderSize()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), Packet::getHeaderSize());
    }

    void testGetSizeFromHeader()
    {
        std::uint8_t data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter
            0x10, 0x00, 0x00, 0x00  // size
        };
        DataBuffer buffer(std::begin(data), std::end(data));

        std::uint32_t size = Packet::getSizeFromHeader(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00000010), size);
    }

    void testGetSizeFromHeaderLittleEndian()
    {
        std::uint8_t data[] = {
            0x02, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter
            0xFF, 0x00, 0x00, 0x00  // size
        };
        DataBuffer buffer(std::begin(data), std::end(data));

        std::uint32_t size = Packet::getSizeFromHeader(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x000000FF), size);
    }

    void testPacketGenericConstructorResetAll()
    {
        PacketResetAll packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    void testPacketGenericConstructorTimestamp()
    {
        PacketGeneric packet(Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
    }

    void testPacketGenericConstructorPause()
    {
        PacketGeneric packet(Packet::Type::PAUSE);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PAUSE);
    }

    void testPacketGenericGetType()
    {
        PacketGeneric packet(Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);

        // Type should be consistent
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    void testPacketChannelSpecificConstructor()
    {
        PacketResetChannel packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testPacketChannelSpecificGetTypeResetChannel()
    {
        PacketChannelSpecific packet(Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
    }

    void testPacketChannelSpecificGetTypeMute()
    {
        PacketChannelSpecific packet(Packet::Type::MUTE);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::MUTE);
    }

    void testPacketChannelSpecificGetChannelIdBeforeParse()
    {
        PacketResetChannel packet;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getChannelId());
    }

    void testPacketParseValidMinimal()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type (RESET_ALL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testPacketParseValidWithZeroCounter()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter (0)
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());
    }

    void testPacketParseValidWithMaxCounter()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter (max)
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());
    }

    void testPacketParseValidWithZeroSize()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size (0)
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testPacketParseValidWithNonZeroSize()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size (4)
            0x05, 0x00, 0x00, 0x00  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5), packet.getChannelId());
    }

    void testPacketParseMismatchedType()
    {
        std::uint8_t packetData[] = {
            0x02, 0x00, 0x00, 0x00, // type (TIMESTAMP, not RESET_ALL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet; // Expects RESET_ALL (3)
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseInvalidType()
    {
        std::uint8_t packetData[] = {
            0xFF, 0xFF, 0xFF, 0xFF, // type (INVALID)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseTypeZero()
    {
        std::uint8_t packetData[] = {
            0x00, 0x00, 0x00, 0x00, // type (0)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseTruncatedType()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00 // Only 2 bytes
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseTruncatedCounter()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00              // Only 2 bytes of counter
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseTruncatedSize()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00              // Only 2 bytes of size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseSizeMismatchLarger()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x10, 0x00, 0x00, 0x00  // size (16, but no data follows)
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseSizeMismatchSmaller()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
            0xFF, 0xFF, 0xFF, 0xFF  // Extra bytes
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseEmptyBuffer()
    {
        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseOneByteBuffer()
    {
        std::uint8_t packetData[] = {0x03};

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseElevenByteBuffer()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00        // Only 3 bytes of size field
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketParseLittleEndianType()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type = 3 (little endian)
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    void testPacketParseLittleEndianCounter()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (little endian)
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getCounter());
    }

    void testPacketParseLittleEndianSize()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size = 4 (little endian)
            0x05, 0x00, 0x00, 0x00  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
    }

    void testPacketParseAlternatingPattern()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type
            0xAA, 0x55, 0xAA, 0x55, // counter (alternating pattern)
            0x00, 0x00, 0x00, 0x00  // size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55AA55AA), packet.getCounter());
    }

    void testPacketParseMultipleTimes()
    {
        // First parse
        std::uint8_t packetData1[] = {
            0x03, 0x00, 0x00, 0x00,
            0x12, 0x34, 0x56, 0x78,
            0x00, 0x00, 0x00, 0x00
        };

        PacketResetAll packet;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(packetData1), std::end(packetData1));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x78563412), packet.getCounter());

        // Second parse with different data
        std::uint8_t packetData2[] = {
            0x03, 0x00, 0x00, 0x00,
            0xAA, 0xBB, 0xCC, 0xDD,
            0x00, 0x00, 0x00, 0x00
        };

        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(packetData2), std::end(packetData2));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDDCCBBAA), packet.getCounter());
    }

    void testPacketParseAfterFailedParse()
    {
        PacketResetAll packet;

        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0xF3, 0x00, 0x00, 0x00, // Invalid type
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Second parse with valid data
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x99, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99), packet.getCounter());
    }

    void testPacketParseMultipleFailures()
    {
        PacketResetAll packet;

        // First failure: invalid type
        std::uint8_t invalidType[] = {
            0xF1, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: truncated
        std::uint8_t truncated[] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(truncated), std::end(truncated));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);
        CPPUNIT_ASSERT(!packet.isValid());

        // Should still be able to parse valid packet
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x55, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55), packet.getCounter());
    }

    void testPacketParseGettersAfterSuccess()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x42, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x07, 0x00, 0x00, 0x00  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify all getters return consistent values
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x42), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), packet.getChannelId());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
    }

    void testPacketParseWithPayload()
    {
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size (4 bytes for channel ID)
            0x09, 0x00, 0x00, 0x00  // channel ID = 9
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(9), packet.getChannelId());
    }

    void testPacketParseParseDataHeaderFails()
    {
        // PacketResetChannel expects 4 bytes for channel ID, but size field says 4 and we provide 3
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x01, 0x00, 0x00, 0x00, // counter
            0x03, 0x00, 0x00, 0x00, // size (3 bytes, insufficient for channel ID)
            0x09, 0x00, 0x00        // Only 3 bytes
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPacketInitialState()
    {
        PacketResetAll packet;

        CPPUNIT_ASSERT(packet.isValid() == false);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    void testPacketIsValidAfterSuccessfulParse()
    {
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
    }

    void testPacketIsValidAfterFailedParse()
    {
        std::uint8_t packetData[] = {
            0xF3, 0x00, 0x00, 0x00, // Invalid type
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testEndToEndPacketResetAll()
    {
        // Complete end-to-end parsing of PacketResetAll
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type (RESET_ALL)
            0xAB, 0xCD, 0xEF, 0x12, // counter
            0x00, 0x00, 0x00, 0x00  // size (0)
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Verify all integration points
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12EFCDAB), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());

        // Verify packet can be queried multiple times
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
    }

    void testEndToEndPacketResetChannel()
    {
        // Complete end-to-end parsing of PacketChannelSpecific subclass
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x10, 0x20, 0x30, 0x40, // counter
            0x04, 0x00, 0x00, 0x00, // size (4 bytes)
            0xAA, 0xBB, 0xCC, 0xDD  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Verify complete workflow: parse -> validate -> extract
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x40302010), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDDCCBBAA), packet.getChannelId());
    }

    void testEndToEndPacketResetChannelInvalidSize()
    {
        // Verify derived class validation is integrated with base parsing
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0x10, 0x20, 0x30, 0x40, // counter
            0x03, 0x00, 0x00, 0x00, // size (3 bytes - invalid)
            0xAA, 0xBB, 0xCC        // Only 3 bytes
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Should fail due to parseDataHeader returning false
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testPolymorphismGetTypeMultipleTypes()
    {
        // Test polymorphic type identification through base pointer
        PacketResetAll resetAllPacket;
        PacketResetChannel resetChannelPacket;

        Packet* ptrResetAll = &resetAllPacket;
        Packet* ptrResetChannel = &resetChannelPacket;

        // Verify virtual getType() dispatch
        CPPUNIT_ASSERT(ptrResetAll->getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(ptrResetChannel->getType() == Packet::Type::RESET_CHANNEL);
    }

    void testPolymorphismParseMultipleTypes()
    {
        // Test polymorphic parsing through base pointer
        std::uint8_t resetAllData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        std::uint8_t resetChannelData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x05, 0x00, 0x00, 0x00
        };

        PacketResetAll resetAllPacket;
        PacketResetChannel resetChannelPacket;

        Packet* ptrResetAll = &resetAllPacket;
        Packet* ptrResetChannel = &resetChannelPacket;

        // Parse through base pointer
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(resetAllData), std::end(resetAllData));
        CPPUNIT_ASSERT(ptrResetAll->parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT(ptrResetAll->isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), ptrResetAll->getCounter());

        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(resetChannelData), std::end(resetChannelData));
        CPPUNIT_ASSERT(ptrResetChannel->parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(ptrResetChannel->isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), ptrResetChannel->getCounter());
    }

    void testStateConsistencyAcrossGetters()
    {
        // Verify state consistency across all getter methods
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type
            0x42, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x07, 0x00, 0x00, 0x00  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Call getters multiple times, verify consistency
        for (int i = 0; i < 3; ++i) {
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x42), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), packet.getChannelId());
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        }
    }

    void testStateConsistencyFailureToSuccess()
    {
        // Test state machine handling failure-to-success transition
        PacketResetAll packet;

        // First: invalid parse
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // State should reset for second parse
        std::uint8_t validData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x99, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99), packet.getCounter());
    }

    void testStateConsistencyZeroSize()
    {
        // Test consistency between parse and getters for zero-size packets
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x50, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testBufferReaderIntegrationHeaderParsing()
    {
        // Verify BufferReader integration for header extraction
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type (offset 0-3)
            0x11, 0x22, 0x33, 0x44, // counter (offset 4-7)
            0x00, 0x00, 0x00, 0x00  // size (offset 8-11)
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify BufferReader correctly extracted all header fields
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x44332211), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testBufferReaderIntegrationPayloadParsing()
    {
        // Verify BufferReader advances offset correctly for payload
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (header offset 0-11)
            0x01, 0x00, 0x00, 0x00, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD  // channel ID (payload at offset 12)
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify payload was correctly extracted after header
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDDCCBBAA), packet.getChannelId());
    }

    void testBufferReaderIntegrationOffsetTracking()
    {
        // Test that BufferReader offset detection works correctly
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF // Extra bytes should cause failure
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Should fail because size=0 but extra bytes present
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testErrorRecoverySequentialParsing()
    {
        // Test robust error recovery in streaming scenarios
        PacketResetAll packet;

        // Parse 1: valid
        std::uint8_t data1[] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(data1), std::end(data1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getCounter());

        // Parse 2: invalid
        std::uint8_t data2[] = {0xF3, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(data2), std::end(data2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Parse 3: valid again
        std::uint8_t data3[] = {0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(data3), std::end(data3));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), packet.getCounter());
    }

    void testErrorRecoveryAlternatingValidInvalid()
    {
        // Test alternating valid/invalid packets
        PacketResetChannel packet;

        // Valid
        std::uint8_t valid1[] = {0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(valid1), std::end(valid1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5), packet.getChannelId());

        // Invalid (wrong type)
        std::uint8_t invalid1[] = {0x03, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalid1), std::end(invalid1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Valid again
        std::uint8_t valid2[] = {0x04, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00};
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(valid2), std::end(valid2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), packet.getChannelId());
    }

    void testErrorRecoveryCorruptedData()
    {
        // Test recovery from corrupted packet data
        PacketResetAll packet;

        // Corrupted: truncated
        std::uint8_t corrupted[] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(corrupted), std::end(corrupted));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);
        CPPUNIT_ASSERT(!packet.isValid());

        // Valid: should work after corruption
        std::uint8_t valid[] = {0x03, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(valid), std::end(valid));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x42), packet.getCounter());
    }

    void testTypeIdentificationExtractThenParse()
    {
        // Test workflow: extractType -> create packet -> parse
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x50, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        // Step 1: Extract type using static helper
        Buffer buffer(reinterpret_cast<const char*>(packetData), sizeof(packetData));
        std::uint32_t extractedType = Packet::extractType(buffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), extractedType);

        // Step 2: Create appropriate packet based on type
        PacketResetAll packet;
        CPPUNIT_ASSERT(static_cast<std::uint32_t>(packet.getType()) == extractedType);

        // Step 3: Parse the packet
        DataBufferPtr dataBuffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        CPPUNIT_ASSERT(packet.parse(std::move(dataBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x50), packet.getCounter());
    }

    void testTypeIdentificationGetTypeFactory()
    {
        // Simulate factory pattern: use getType() to determine packet class
        std::uint8_t resetAllData[] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        std::uint8_t resetChannelData[] = {0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00};

        DataBuffer buffer1(std::begin(resetAllData), std::end(resetAllData));
        DataBuffer buffer2(std::begin(resetChannelData), std::end(resetChannelData));

        Packet::Type type1 = Packet::getType(buffer1);
        Packet::Type type2 = Packet::getType(buffer2);

        CPPUNIT_ASSERT(type1 == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(type2 == Packet::Type::RESET_CHANNEL);

        // Create packets based on identified types
        PacketResetAll packet1;
        PacketResetChannel packet2;

        CPPUNIT_ASSERT(packet1.getType() == type1);
        CPPUNIT_ASSERT(packet2.getType() == type2);
    }

    void testTypeIdentificationIsDataPacketFiltering()
    {
        // Test filtering workflow using isDataPacket()
        std::vector<Packet::Type> types = {
            Packet::Type::PES_DATA,
            Packet::Type::TTML_DATA,
            Packet::Type::CC_DATA,
            Packet::Type::WEBVTT_DATA,
            Packet::Type::TIMESTAMP,
            Packet::Type::RESET_ALL,
            Packet::Type::RESET_CHANNEL
        };

        int dataPacketCount = 0;
        int controlPacketCount = 0;

        for (auto type : types) {
            if (Packet::isDataPacket(type)) {
                dataPacketCount++;
            } else {
                controlPacketCount++;
            }
        }

        CPPUNIT_ASSERT_EQUAL(4, dataPacketCount);
        CPPUNIT_ASSERT_EQUAL(3, controlPacketCount);
    }

    void testEdgeCaseChannelIdBoundaries()
    {
        // Test boundary values in derived class integration
        std::uint8_t maxChannelData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF // Max channel ID
        };

        PacketResetChannel maxPacket;
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxChannelData), std::end(maxChannelData));

        CPPUNIT_ASSERT(maxPacket.parse(std::move(maxBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), maxPacket.getChannelId());

        // Test zero channel ID
        std::uint8_t zeroChannelData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x02, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 // Zero channel ID
        };

        PacketResetChannel zeroPacket;
        DataBufferPtr zeroBuffer = std::make_unique<DataBuffer>(std::begin(zeroChannelData), std::end(zeroChannelData));

        CPPUNIT_ASSERT(zeroPacket.parse(std::move(zeroBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), zeroPacket.getChannelId());
    }

    void testEdgeCaseCounterSequenceWrapping()
    {
        // Test counter values across boundaries
        PacketResetAll packet;

        // Max counter
        std::uint8_t maxData[] = {0x03, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr maxBuffer = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        CPPUNIT_ASSERT(packet.parse(std::move(maxBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());

        // Wrap to zero
        std::uint8_t zeroData[] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr zeroBuffer = std::make_unique<DataBuffer>(std::begin(zeroData), std::end(zeroData));
        CPPUNIT_ASSERT(packet.parse(std::move(zeroBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());

        // Continue from 1
        std::uint8_t oneData[] = {0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        DataBufferPtr oneBuffer = std::make_unique<DataBuffer>(std::begin(oneData), std::end(oneData));
        CPPUNIT_ASSERT(packet.parse(std::move(oneBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packet.getCounter());
    }

    void testEdgeCaseZeroLengthPayload()
    {
        // Test minimum payload size in derived class
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x42, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00 // Zero payload size
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getSize());
    }

    void testProtocolComplianceHeaderStructure()
    {
        // Verify 12-byte header structure is consistent
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), Packet::getHeaderSize());

        // Parse packet and verify header fields are at correct offsets
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00, // type at offset 0
            0x11, 0x22, 0x33, 0x44, // counter at offset 4
            0x00, 0x00, 0x00, 0x00  // size at offset 8
        };

        DataBuffer buffer(std::begin(packetData), std::end(packetData));

        // Verify static helpers extract from correct offsets
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), Packet::extractType(Buffer(buffer.data(), buffer.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x44332211), Packet::getCounter(buffer));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), Packet::getSizeFromHeader(buffer));
    }

    void testProtocolComplianceFieldOrdering()
    {
        // Verify field extraction order: type -> counter -> size
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00, // type (RESET_CHANNEL)
            0xAA, 0xBB, 0xCC, 0xDD, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x11, 0x22, 0x33, 0x44  // channel ID
        };

        PacketResetChannel packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify all fields parsed in correct order
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDDCCBBAA), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x44332211), packet.getChannelId());
    }

    void testProtocolComplianceLittleEndianConsistency()
    {
        // Verify little-endian handling across all fields
        std::uint8_t packetData[] = {
            0x01, 0x02, 0x03, 0x04, // type (becomes 0x04030201)
            0x05, 0x06, 0x07, 0x08, // counter (becomes 0x08070605)
            0x00, 0x00, 0x00, 0x00  // size
        };

        // This will fail because type doesn't match RESET_ALL, but we can verify extraction
        DataBuffer buffer(std::begin(packetData), std::end(packetData));

        std::uint32_t type = Packet::extractType(Buffer(buffer.data(), buffer.size()));
        std::uint32_t counter = Packet::getCounter(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), type);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), counter);
    }

    void testMemorySafetyLargeSizeValue()
    {
        // Test safe handling of malicious large size values
        std::uint8_t packetData[] = {
            0x03, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0x7F  // Large size (2GB - 1)
        };

        PacketResetAll packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Should fail safely due to size mismatch validation
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testMemorySafetySequentialReuseNoLeaks()
    {
        // Test packet object reuse doesn't cause memory issues
        PacketResetChannel packet;

        for (int i = 0; i < 5; ++i) {
            std::uint8_t packetData[] = {
                0x04, 0x00, 0x00, 0x00,
                static_cast<std::uint8_t>(i), 0x00, 0x00, 0x00,
                0x04, 0x00, 0x00, 0x00,
                static_cast<std::uint8_t>(i * 10), 0x00, 0x00, 0x00
            };

            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i * 10), packet.getChannelId());
        }
    }

    void testIntegrationStaticHelpersCombined()
    {
        // Test all static helpers work together
        std::uint8_t packetData[] = {
            0x08, 0x00, 0x00, 0x00, // TTML_DATA type
            0x99, 0x00, 0x00, 0x00, // counter
            0x0A, 0x00, 0x00, 0x00  // size
        };

        DataBuffer buffer(std::begin(packetData), std::end(packetData));
        Buffer rawBuffer(buffer.data(), buffer.size());

        // Use all static helpers
        std::uint32_t extractedType = Packet::extractType(rawBuffer);
        Packet::Type enumType = Packet::getType(buffer);
        std::uint32_t counter = Packet::getCounter(buffer);
        std::uint32_t size = Packet::getSizeFromHeader(buffer);
        std::uint32_t headerSize = Packet::getHeaderSize();
        bool isData = Packet::isDataPacket(enumType);

        // Verify all results
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(8), extractedType);
        CPPUNIT_ASSERT(enumType == Packet::Type::TTML_DATA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99), counter);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x0A), size);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), headerSize);
        CPPUNIT_ASSERT(isData == true);
    }

    void testIntegrationParseWithAllStaticHelpers()
    {
        // Complete workflow: static helpers -> parse -> validate
        std::uint8_t packetData[] = {
            0x04, 0x00, 0x00, 0x00,
            0x42, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00,
            0x77, 0x00, 0x00, 0x00
        };

        DataBuffer buffer(std::begin(packetData), std::end(packetData));

        // Step 1: Use static helpers for pre-validation
        Packet::Type type = Packet::getType(buffer);
        std::uint32_t counter = Packet::getCounter(buffer);
        std::uint32_t size = Packet::getSizeFromHeader(buffer);

        CPPUNIT_ASSERT(type == Packet::Type::RESET_CHANNEL);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x42), counter);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(4), size);

        // Step 2: Parse packet
        PacketResetChannel packet;
        DataBufferPtr parseBuffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        CPPUNIT_ASSERT(packet.parse(std::move(parseBuffer)) == true);

        // Step 3: Verify parsed values match static helper results
        CPPUNIT_ASSERT(packet.getType() == type);
        CPPUNIT_ASSERT_EQUAL(counter, packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(size, packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x77), packet.getChannelId());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTest );
