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

#include "PacketSetCCAttributes.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketChannelSpecific;
using subttxrend::protocol::PacketSetCCAttributes;

class PacketSetCCAttributesTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketSetCCAttributesTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadSize67);
    CPPUNIT_TEST(testBadSize69);
    CPPUNIT_TEST(testBadSizeZero);
    CPPUNIT_TEST(testTooShortAfterChannelId);
    CPPUNIT_TEST(testTooShortAfterCcType);
    CPPUNIT_TEST(testTooShortAfterAttribType);
    CPPUNIT_TEST(testTooShort13Attributes);
    CPPUNIT_TEST(testTooLong);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testGetTypeBasic);
    CPPUNIT_TEST(testAllAttributesSet);
    CPPUNIT_TEST(testNoAttributesSet);
    CPPUNIT_TEST(testSingleAttributeFontColor);
    CPPUNIT_TEST(testTwoAttributesFontColorAndBackground);
    CPPUNIT_TEST(testAlternatingAttributes);
    CPPUNIT_TEST(testAttributeBitmaskWithUndefinedBits);
    CPPUNIT_TEST(testAttributeBitmaskAllBitsSet);
    CPPUNIT_TEST(testContainsAttributeWhenPresent);
    CPPUNIT_TEST(testContainsAttributeWhenAbsent);
    CPPUNIT_TEST(testContainsAttributeAllPresent);
    CPPUNIT_TEST(testContainsAttributeAllAbsent);
    CPPUNIT_TEST(testContainsAttributeBeforeParse);
    CPPUNIT_TEST(testContainsAttributeAfterFailedParse);
    CPPUNIT_TEST(testGetAttributeValuePresent);
    CPPUNIT_TEST(testGetAttributeValueZero);
    CPPUNIT_TEST(testGetAttributeValueMax);
    CPPUNIT_TEST(testGetAttributeValueAll14);
    CPPUNIT_TEST(testGetAttributesAllPresent);
    CPPUNIT_TEST(testGetAttributesNone);
    CPPUNIT_TEST(testGetAttributesPartial);
    CPPUNIT_TEST(testGetAttributesBeforeParse);
    CPPUNIT_TEST(testChannelIdBoundaryZero);
    CPPUNIT_TEST(testChannelIdBoundaryMax);
    CPPUNIT_TEST(testCounterBoundaryZero);
    CPPUNIT_TEST(testCounterBoundaryMax);
    CPPUNIT_TEST(testCcTypeBoundaryZero);
    CPPUNIT_TEST(testCcTypeBoundaryMax);
    CPPUNIT_TEST(testAttributeValuesBoundaryZero);
    CPPUNIT_TEST(testAttributeValuesBoundaryMax);
    CPPUNIT_TEST(testLittleEndianChannelId);
    CPPUNIT_TEST(testLittleEndianCounter);
    CPPUNIT_TEST(testLittleEndianCcType);
    CPPUNIT_TEST(testLittleEndianAttribType);
    CPPUNIT_TEST(testLittleEndianAttributeValues);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testParseAfterFailedParse);
    CPPUNIT_TEST(testMultipleParseCalls);
    CPPUNIT_TEST(testSequentialFailures);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testTruncatedHeader);
    CPPUNIT_TEST(testCorruptedTypeField);
    CPPUNIT_TEST(testCorruptedSizeField);
    CPPUNIT_TEST(testProtocolCompliance68Bytes);
    CPPUNIT_TEST(testProtocolFieldOrdering);
    CPPUNIT_TEST(testEndToEndCompleteWorkflow);
    CPPUNIT_TEST(testEndToEndAllAttributesWorkflow);
    CPPUNIT_TEST(testEndToEndPartialAttributesWorkflow);
    CPPUNIT_TEST(testEndToEndNoAttributesWorkflow);
    CPPUNIT_TEST(testEndToEndInvalidPacketWorkflow);
    CPPUNIT_TEST(testBufferReaderIntegrationHeaderExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationPayloadExtraction);
    CPPUNIT_TEST(testBufferReaderIntegrationOffsetTracking);
    CPPUNIT_TEST(testBufferReaderIntegrationAttributeExtraction);
    CPPUNIT_TEST(testPolymorphismGetTypeVirtual);
    CPPUNIT_TEST(testPolymorphismParseVirtual);
    CPPUNIT_TEST(testPolymorphismIsValidVirtual);
    CPPUNIT_TEST(testPolymorphismChannelSpecificBase);
    CPPUNIT_TEST(testStateConsistencyAcrossGetters);
    CPPUNIT_TEST(testStateConsistencyMultipleCalls);
    CPPUNIT_TEST(testStateConsistencyFailureToSuccess);
    CPPUNIT_TEST(testStateConsistencySuccessToFailure);
    CPPUNIT_TEST(testErrorRecoverySequentialParsing);
    CPPUNIT_TEST(testErrorRecoveryAlternatingValidInvalid);
    CPPUNIT_TEST(testErrorRecoveryAfterTruncation);
    CPPUNIT_TEST(testErrorRecoveryAfterCorruption);
    CPPUNIT_TEST(testCrossComponentBufferAndReader);
    CPPUNIT_TEST(testCrossComponentAttributeMapIntegrity);
    CPPUNIT_TEST(testCrossComponentMultipleAttributeQueries);
    CPPUNIT_TEST(testCrossComponentLargePacketStream);
    CPPUNIT_TEST(testRealWorldCaptionStylingScenario);
    CPPUNIT_TEST(testRealWorldMultiplePacketsSequence);
    CPPUNIT_TEST(testRealWorldAttributeUpdateScenario);
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

    // Helper to create a valid packet with all attributes set
    std::vector<std::uint8_t> createValidPacket(
        std::uint32_t channelId = 0x01020304,
        std::uint32_t counter = 0x12345678,
        std::uint32_t ccType = 0xAABBCCDD,
        std::uint32_t attribType = 0x3FFF, // All 14 bits set
        const std::vector<std::uint32_t>& attribValues = {})
    {
        std::vector<std::uint8_t> packet = {
            0x12, 0x00, 0x00, 0x00, // type (SET_CC_ATTRIBUTES = 18)
            static_cast<std::uint8_t>(counter & 0xFF),
            static_cast<std::uint8_t>((counter >> 8) & 0xFF),
            static_cast<std::uint8_t>((counter >> 16) & 0xFF),
            static_cast<std::uint8_t>((counter >> 24) & 0xFF),
            0x44, 0x00, 0x00, 0x00, // size (68 = 4 + 4 + 4 + 56)
            static_cast<std::uint8_t>(channelId & 0xFF),
            static_cast<std::uint8_t>((channelId >> 8) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 16) & 0xFF),
            static_cast<std::uint8_t>((channelId >> 24) & 0xFF),
            static_cast<std::uint8_t>(ccType & 0xFF),
            static_cast<std::uint8_t>((ccType >> 8) & 0xFF),
            static_cast<std::uint8_t>((ccType >> 16) & 0xFF),
            static_cast<std::uint8_t>((ccType >> 24) & 0xFF),
            static_cast<std::uint8_t>(attribType & 0xFF),
            static_cast<std::uint8_t>((attribType >> 8) & 0xFF),
            static_cast<std::uint8_t>((attribType >> 16) & 0xFF),
            static_cast<std::uint8_t>((attribType >> 24) & 0xFF),
        };

        // Add 14 attribute values
        std::vector<std::uint32_t> values = attribValues;
        while (values.size() < 14) {
            values.push_back(0x11223344 + values.size());
        }

        for (const auto& val : values) {
            packet.push_back(static_cast<std::uint8_t>(val & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((val >> 8) & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((val >> 16) & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((val >> 24) & 0xFF));
        }

        return packet;
    }

    void testGood()
    {
        auto packetData = createValidPacket();

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT(packet.getCounter() == 0x12345678);
        CPPUNIT_ASSERT(packet.getSize() == 68);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x01020304);
    }

    void testBadType()
    {
        auto packetData = createValidPacket();
        // Change type to TIMESTAMP (2)
        packetData[0] = 0x02;

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize67()
    {
        auto packetData = createValidPacket();
        // Change size to 67 (one byte short)
        packetData[8] = 0x43;
        // Remove last byte
        packetData.pop_back();

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize69()
    {
        auto packetData = createValidPacket();
        // Change size to 69 (one byte extra)
        packetData[8] = 0x45;
        // Add extra byte
        packetData.push_back(0xFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSizeZero()
    {
        std::uint8_t packetData[] = {
            0x12, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x00, 0x00, 0x00, 0x00, // size (0)
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShortAfterChannelId()
    {
        std::uint8_t packetData[] = {
            0x12, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x44, 0x00, 0x00, 0x00, // size (68)
            0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShortAfterCcType()
    {
        std::uint8_t packetData[] = {
            0x12, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x44, 0x00, 0x00, 0x00, // size (68)
            0x01, 0x02, 0x03, 0x04, // channel id
            0xAA, 0xBB, 0xCC, 0xDD, // ccType
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShortAfterAttribType()
    {
        std::uint8_t packetData[] = {
            0x12, 0x00, 0x00, 0x00, // type
            0x01, 0x00, 0x00, 0x00, // counter
            0x44, 0x00, 0x00, 0x00, // size (68)
            0x01, 0x02, 0x03, 0x04, // channel id
            0xAA, 0xBB, 0xCC, 0xDD, // ccType
            0xFF, 0x3F, 0x00, 0x00, // attribType
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort13Attributes()
    {
        auto packetData = createValidPacket();
        // Remove last 4 bytes (one attribute value)
        packetData.resize(packetData.size() - 4);
        // Adjust size field
        packetData[8] = 0x40; // 64 instead of 68

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        auto packetData = createValidPacket();
        // Add extra bytes
        packetData.push_back(0xFF);
        packetData.push_back(0xFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testConstructorInitialState()
    {
        PacketSetCCAttributes packet;

        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
    }

    void testGetTypeBasic()
    {
        PacketSetCCAttributes packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);

        auto packetData = createValidPacket();
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
    }

    void testAllAttributesSet()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF); // All 14 bits

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Check all 14 attributes are present
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_STYLE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_ITALIC));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_UNDERLINE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_TYPE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::WIN_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::WIN_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_TYPE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs.size());
    }

    void testNoAttributesSet()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0000); // No bits set

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Check no attributes are present
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testSingleAttributeFontColor()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0001); // Only FONT_COLOR

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), attrs.size());
    }

    void testTwoAttributesFontColorAndBackground()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0003); // FONT_COLOR | BACKGROUND_COLOR

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), attrs.size());
    }

    void testAlternatingAttributes()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x1555); // Alternating bits

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), attrs.size());
    }

    void testAttributeBitmaskWithUndefinedBits()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x4000); // Bit outside 14 defined bits

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Only defined attributes should be present
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testAttributeBitmaskAllBitsSet()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0xFFFFFFFF); // All bits

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Only 14 defined attributes should be present
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs.size());
    }

    void testContainsAttributeWhenPresent()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0020); // FONT_SIZE

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));
    }

    void testContainsAttributeWhenAbsent()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0020); // FONT_SIZE only

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testContainsAttributeAllPresent()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // All should return true
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_STYLE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_ITALIC));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_UNDERLINE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_TYPE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::WIN_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::WIN_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_TYPE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
    }

    void testContainsAttributeAllAbsent()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0000);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        // All should return false
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
    }

    void testContainsAttributeBeforeParse()
    {
        PacketSetCCAttributes packet;

        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testContainsAttributeAfterFailedParse()
    {
        std::uint8_t packetData[] = {
            0xFF, 0x00, 0x00, 0x00, // Invalid type
            0x01, 0x00, 0x00, 0x00,
            0x44, 0x00, 0x00, 0x00,
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testGetAttributeValuePresent()
    {
        std::vector<std::uint32_t> values;
        for (int i = 0; i < 14; ++i) {
            values.push_back(0x10 + i);
        }
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testGetAttributeValueZero()
    {
        std::vector<std::uint32_t> values(14, 0);
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testGetAttributeValueMax()
    {
        std::vector<std::uint32_t> values(14, 0xFFFFFFFF);
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testGetAttributeValueAll14()
    {
        std::vector<std::uint32_t> values;
        for (int i = 0; i < 14; ++i) {
            values.push_back(0x1000 + i * 0x100);
        }
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1000),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1100),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1200),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1300),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
    }

    void testGetAttributesAllPresent()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs.size());
    }

    void testGetAttributesNone()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0000);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testGetAttributesPartial()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0007); // 3 attributes

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), attrs.size());
    }

    void testGetAttributesBeforeParse()
    {
        PacketSetCCAttributes packet;

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testChannelIdBoundaryZero()
    {
        auto packetData = createValidPacket(0x00000000, 0x01, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getChannelId());
    }

    void testChannelIdBoundaryMax()
    {
        auto packetData = createValidPacket(0xFFFFFFFF, 0x01, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getChannelId());
    }

    void testCounterBoundaryZero()
    {
        auto packetData = createValidPacket(0x01, 0x00000000, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packet.getCounter());
    }

    void testCounterBoundaryMax()
    {
        auto packetData = createValidPacket(0x01, 0xFFFFFFFF, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());
    }

    void testCcTypeBoundaryZero()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00000000, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testCcTypeBoundaryMax()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0xFFFFFFFF, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testAttributeValuesBoundaryZero()
    {
        std::vector<std::uint32_t> values(14, 0);
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testAttributeValuesBoundaryMax()
    {
        std::vector<std::uint32_t> values(14, 0xFFFFFFFF);
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testLittleEndianChannelId()
    {
        auto packetData = createValidPacket(0x04030201, 0x01, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getChannelId());
    }

    void testLittleEndianCounter()
    {
        auto packetData = createValidPacket(0x01, 0x04030201, 0x00, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getCounter());
    }

    void testLittleEndianCcType()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x44332211, 0x0001);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testLittleEndianAttribType()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0001); // FONT_COLOR only

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testLittleEndianAttributeValues()
    {
        std::vector<std::uint32_t> values = {0x04030201};
        values.resize(14, 0);
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x0001, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testPacketReuse()
    {
        PacketSetCCAttributes packet;

        // First parse
        auto packetData1 = createValidPacket(0x01, 0x10, 0x00, 0x0001);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(packetData1.begin(), packetData1.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10), packet.getCounter());

        // Second parse with different data
        auto packetData2 = createValidPacket(0x02, 0x20, 0x00, 0x0002);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(packetData2.begin(), packetData2.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x20), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), packet.getChannelId());
    }

    void testParseAfterFailedParse()
    {
        PacketSetCCAttributes packet;

        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // Invalid type
            0x01, 0x00, 0x00, 0x00,
            0x44, 0x00, 0x00, 0x00,
        };

        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(invalidBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second parse with valid data
        auto validData = createValidPacket(0x05, 0x99, 0x00, 0x0001);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x99), packet.getCounter());
    }

    void testMultipleParseCalls()
    {
        PacketSetCCAttributes packet;

        for (int i = 0; i < 3; ++i) {
            auto packetData = createValidPacket(i + 1, i + 10, 0x00, 0x0001);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i + 10), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i + 1), packet.getChannelId());
        }
    }

    void testSequentialFailures()
    {
        PacketSetCCAttributes packet;

        // First failure: invalid type
        std::uint8_t invalidType[] = {0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00};
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidType), std::end(invalidType));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Second failure: invalid size
        std::uint8_t invalidSize[] = {0x12, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00};
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidSize), std::end(invalidSize));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());

        // Valid parse should still work
        auto validData = createValidPacket(0x01, 0x55, 0x00, 0x0001);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x55), packet.getCounter());
    }

    void testEmptyBuffer()
    {
        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>();

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTruncatedHeader()
    {
        std::uint8_t packetData[] = {
            0x12, 0x00, 0x00, 0x00, // type
            0x01, 0x00, // Truncated counter
        };

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testCorruptedTypeField()
    {
        auto packetData = createValidPacket();
        // Corrupt type field
        packetData[1] = 0xFF;

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testCorruptedSizeField()
    {
        auto packetData = createValidPacket();
        // Corrupt size field
        packetData[9] = 0xFF;

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testProtocolCompliance68Bytes()
    {
        auto packetData = createValidPacket(0x01, 0x01, 0x00, 0x3FFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Verify size is exactly 68 bytes (4 + 4 + 4 + 14*4)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(68), packet.getSize());
    }

    void testProtocolFieldOrdering()
    {
        auto packetData = createValidPacket(0x44332211, 0x88776655, 0xCCBBAA99, 0x0003);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());

        // Verify correct field extraction order
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x88776655), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(68), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x44332211), packet.getChannelId());
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
    }

    void testEndToEndCompleteWorkflow()
    {
        // Complete end-to-end workflow: create packet -> parse -> validate -> extract all data
        std::vector<std::uint32_t> values = {
            0xFF0000FF, // FONT_COLOR: red
            0x00FF00FF, // BACKGROUND_COLOR: green
            0xFF,       // FONT_OPACITY: opaque
            0x80,       // BACKGROUND_OPACITY: semi-transparent
            0x01,       // FONT_STYLE
            0x02,       // FONT_SIZE
            0x01,       // FONT_ITALIC
            0x00,       // FONT_UNDERLINE
            0x01,       // BORDER_TYPE
            0x000000FF, // BORDER_COLOR: black
            0xFFFFFFFF, // WIN_COLOR: white
            0xFF,       // WIN_OPACITY
            0x02,       // EDGE_TYPE
            0x808080FF  // EDGE_COLOR: gray
        };
        auto packetData = createValidPacket(0x12345678, 0xABCDEF01, 0x01, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Parse and validate
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Verify all header fields
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABCDEF01), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(68), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678), packet.getChannelId());

        // Verify all attributes are present and correct
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs.size());

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFF0000FF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x00FF00FF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x808080FF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
    }

    void testEndToEndAllAttributesWorkflow()
    {
        // Verify complete workflow with all possible attributes set
        std::vector<std::uint32_t> values;
        for (int i = 0; i < 14; ++i) {
            values.push_back(0xA0000000 + i * 0x01000000);
        }
        auto packetData = createValidPacket(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Verify boundary values preserved through complete workflow
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getCounter());

        // Verify all 14 attributes accessible through public API
        for (int i = 0; i < 14; ++i) {
            auto attrType = static_cast<PacketSetCCAttributes::CcAttribType>(1 << i);
            CPPUNIT_ASSERT(packet.containsAttribute(attrType));
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xA0000000 + i * 0x01000000),
                               packet.getAttributeValue(attrType));
        }
    }

    void testEndToEndPartialAttributesWorkflow()
    {
        // Test workflow with only some attributes set (realistic scenario)
        std::vector<std::uint32_t> values = {
            0xFF0000FF, // FONT_COLOR
            0x00000000, // BACKGROUND_COLOR (not used)
            0xFF,       // FONT_OPACITY
            0x00,       // BACKGROUND_OPACITY (not used)
            0x00,       // Others not used
            0x14,       // FONT_SIZE
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x0025, values); // FONT_COLOR | FONT_OPACITY | FONT_SIZE

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), attrs.size());

        // Verify only specified attributes are accessible
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));

        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
    }

    void testEndToEndNoAttributesWorkflow()
    {
        // Verify workflow when no attributes are set (edge case but valid)
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x0000);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Verify packet still provides valid base class data
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x02), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01), packet.getChannelId());

        // Verify attribute map is empty
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testEndToEndInvalidPacketWorkflow()
    {
        // Verify complete workflow handles invalid packets correctly
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x0001);
        packetData[8] = 0x43; // Change size to 67 (invalid)
        packetData.pop_back();

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);

        // Verify packet remains in safe state after failure
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), attrs.size());
    }

    void testBufferReaderIntegrationHeaderExtraction()
    {
        // Verify BufferReader correctly extracts header fields in sequence
        std::vector<std::uint32_t> values = {0x11111111, 0x22222222, 0x33333333, 0x44444444,
                                            0x55555555, 0x66666666, 0x77777777, 0x88888888,
                                            0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,
                                            0xDDDDDDDD, 0xEEEEEEEE};
        auto packetData = createValidPacket(0x04030201, 0x08070605, 0x0C0B0A09, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify BufferReader extracted all header fields correctly with little-endian
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x08070605), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(68), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04030201), packet.getChannelId());
    }

    void testBufferReaderIntegrationPayloadExtraction()
    {
        // Verify BufferReader advances offset correctly through payload
        std::vector<std::uint32_t> values;
        for (int i = 0; i < 14; ++i) {
            values.push_back(0x10000000 + i);
        }
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify all 14 attribute values extracted in correct order
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10000000),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10000001),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x1000000D),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
    }

    void testBufferReaderIntegrationOffsetTracking()
    {
        // Verify BufferReader detects incorrect buffer sizes
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x0001);
        packetData.push_back(0xFF); // Add extra byte
        packetData.push_back(0xFE);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Should fail because buffer has extra data beyond size field
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
    }

    void testBufferReaderIntegrationAttributeExtraction()
    {
        // Verify BufferReader extracts all attributes even when only some are flagged
        std::vector<std::uint32_t> values = {
            0xABCDEF01, 0x12345678, 0xFEDCBA98, 0x87654321,
            0x11111111, 0x22222222, 0x33333333, 0x44444444,
            0x55555555, 0x66666666, 0x77777777, 0x88888888,
            0x99999999, 0xAAAAAAAA
        };
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x1249, values); // Sparse attributes: bits 0,3,6,9,12

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // 0x1249 = bits 0,3,6,9,12 = FONT_COLOR, BACKGROUND_OPACITY, FONT_ITALIC, BORDER_COLOR, EDGE_TYPE
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABCDEF01),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));

        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(!packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_TYPE)); // bit 8, not set
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_TYPE)); // bit 12, is set
    }

    void testPolymorphismGetTypeVirtual()
    {
        // Test virtual getType() dispatch through base class pointer
        PacketSetCCAttributes derivedPacket;
        Packet* basePtr = &derivedPacket;
        PacketChannelSpecific* channelPtr = &derivedPacket;

        // Verify virtual dispatch works correctly
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT(derivedPacket.getType() == Packet::Type::SET_CC_ATTRIBUTES);
    }

    void testPolymorphismParseVirtual()
    {
        // Test virtual parse() through base class pointer
        auto packetData = createValidPacket(0x123, 0x456, 0x789, 0x0007);

        PacketSetCCAttributes derivedPacket;
        Packet* basePtr = &derivedPacket;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        // Parse through base pointer
        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(basePtr->getType() == Packet::Type::SET_CC_ATTRIBUTES);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x456), basePtr->getCounter());

        // Verify derived class methods still accessible
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x123), derivedPacket.getChannelId());
        CPPUNIT_ASSERT(derivedPacket.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }

    void testPolymorphismIsValidVirtual()
    {
        // Test isValid() state consistency through polymorphic interface
        PacketSetCCAttributes derivedPacket;
        Packet* basePtr = &derivedPacket;

        CPPUNIT_ASSERT(basePtr->isValid() == false);

        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x0001);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(basePtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(basePtr->isValid() == true);
        CPPUNIT_ASSERT(derivedPacket.isValid() == true);
    }

    void testPolymorphismChannelSpecificBase()
    {
        // Verify PacketChannelSpecific base class integration
        PacketSetCCAttributes packet;
        PacketChannelSpecific* channelPtr = &packet;

        auto packetData = createValidPacket(0xAABBCCDD, 0x11223344, 0x55667788, 0x0001);
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(channelPtr->parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(channelPtr->isValid() == true);

        // Verify channel-specific functionality through base pointer
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xAABBCCDD), channelPtr->getChannelId());
        CPPUNIT_ASSERT(channelPtr->getType() == Packet::Type::SET_CC_ATTRIBUTES);
    }

    void testStateConsistencyAcrossGetters()
    {
        // Verify state remains consistent across multiple getter calls
        std::vector<std::uint32_t> values(14, 0x12345678);
        auto packetData = createValidPacket(0xDEADBEEF, 0xCAFEBABE, 0xFEEDFACE, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Call getters multiple times, verify consistency
        for (int i = 0; i < 5; ++i) {
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT(packet.getType() == Packet::Type::SET_CC_ATTRIBUTES);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xCAFEBABE), packet.getCounter());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(68), packet.getSize());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xDEADBEEF), packet.getChannelId());

            auto attrs = packet.getAttributes();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs.size());

            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345678),
                               packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        }
    }

    void testStateConsistencyMultipleCalls()
    {
        // Verify attribute queries remain consistent
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x00FF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // 0x00FF = bits 0-7 = all first 8 attributes including FONT_STYLE (bit 4)
        // Query same attributes multiple times
        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_OPACITY));
            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));

            CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_STYLE)); // bit 4 is set in 0x00FF

            auto attrs = packet.getAttributes();
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), attrs.size());
        }
    }

    void testStateConsistencyFailureToSuccess()
    {
        // Test state machine: failure -> success transition
        PacketSetCCAttributes packet;

        // Invalid packet
        std::uint8_t invalidData[] = {
            0xFF, 0x00, 0x00, 0x00, // Wrong type
            0x01, 0x00, 0x00, 0x00,
            0x44, 0x00, 0x00, 0x00,
        };

        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getAttributes().size());

        // Valid packet
        auto validData = createValidPacket(0xABC, 0xDEF, 0x123, 0x0003);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABC), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet.getAttributes().size());
    }

    void testStateConsistencySuccessToFailure()
    {
        // Test state machine: success -> failure transition
        PacketSetCCAttributes packet;

        // Valid packet first
        auto validData = createValidPacket(0x111, 0x222, 0x333, 0x3FFF);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size());

        // Invalid packet
        auto invalidData = createValidPacket(0x444, 0x555, 0x666, 0x0001);
        invalidData[8] = 0x40; // Wrong size
        DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
        CPPUNIT_ASSERT(packet.isValid() == false);
        // After failure, the parse may have partially populated attributes before detecting size mismatch
        // The implementation keeps previously parsed data, so attributes map may still have data
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size());
    }

    void testErrorRecoverySequentialParsing()
    {
        // Test robust error recovery in sequential packet stream
        PacketSetCCAttributes packet;

        // Parse 1: valid
        auto data1 = createValidPacket(0x01, 0x10, 0x20, 0x0001);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10), packet.getCounter());

        // Parse 2: invalid type
        auto data2 = createValidPacket(0x02, 0x20, 0x30, 0x0002);
        data2[0] = 0x03; // RESET_ALL type
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Parse 3: invalid size
        auto data3 = createValidPacket(0x03, 0x30, 0x40, 0x0003);
        data3[8] = 0x43; // size = 67
        data3.pop_back();
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == false);

        // Parse 4: valid again - verify recovery
        auto data4 = createValidPacket(0x04, 0x40, 0x50, 0x0007);
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(data4.begin(), data4.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer4)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x40), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x04), packet.getChannelId());
    }

    void testErrorRecoveryAlternatingValidInvalid()
    {
        // Test alternating valid/invalid packet handling
        PacketSetCCAttributes packet;

        for (int i = 0; i < 5; ++i) {
            // Valid packet
            auto validData = createValidPacket(i * 10, i * 100, i * 1000, 0x0001);
            DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i * 10), packet.getChannelId());

            // Invalid packet (wrong size)
            auto invalidData = createValidPacket((i + 1) * 10, (i + 1) * 100, (i + 1) * 1000, 0x0001);
            invalidData[8] = 0x00; // size = 0
            DataBufferPtr invalidBuffer = std::make_unique<DataBuffer>(invalidData.begin(), invalidData.end());
            CPPUNIT_ASSERT(packet.parse(std::move(invalidBuffer)) == false);
            CPPUNIT_ASSERT(packet.isValid() == false);
        }
    }

    void testErrorRecoveryAfterTruncation()
    {
        // Test recovery after various truncation scenarios
        PacketSetCCAttributes packet;

        // Truncated after channelId
        std::uint8_t truncated1[] = {
            0x12, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x44, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD
        };
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(truncated1), std::end(truncated1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);

        // Valid packet after truncation
        auto validData = createValidPacket(0xABCD, 0x1234, 0x5678, 0x0001);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xABCD), packet.getChannelId());
    }

    void testErrorRecoveryAfterCorruption()
    {
        // Test recovery after data corruption
        PacketSetCCAttributes packet;

        // Corrupted type
        auto corrupted1 = createValidPacket(0x01, 0x02, 0x03, 0x0001);
        corrupted1[0] = 0xAA;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(corrupted1.begin(), corrupted1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == false);

        // Corrupted size
        auto corrupted2 = createValidPacket(0x04, 0x05, 0x06, 0x0001);
        corrupted2[8] = 0xFF;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(corrupted2.begin(), corrupted2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Valid packet - verify clean recovery
        auto validData = createValidPacket(0x07, 0x08, 0x09, 0x3FFF);
        DataBufferPtr validBuffer = std::make_unique<DataBuffer>(validData.begin(), validData.end());
        CPPUNIT_ASSERT(packet.parse(std::move(validBuffer)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x07), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size());
    }

    void testCrossComponentBufferAndReader()
    {
        // Test integration between Buffer, BufferReader, and Packet
        std::vector<std::uint32_t> values = {
            0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10,
            0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20,
            0x21222324, 0x25262728, 0x292A2B2C, 0x2D2E2F30,
            0x31323334, 0x35363738
        };
        auto packetData = createValidPacket(0xF0E0D0C0, 0xB0A09080, 0x70605040, 0x3FFF, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Verify data flows correctly through all components
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xF0E0D0C0), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xB0A09080), packet.getCounter());

        // Verify all attribute values preserved through extraction chain
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x01020304),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x35363738),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
    }

    void testCrossComponentAttributeMapIntegrity()
    {
        // Verify attribute map integrity across component boundaries
        std::vector<std::uint32_t> values;
        for (int i = 0; i < 14; ++i) {
            values.push_back(0xF0000000 | (i << 16) | i);
        }
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x2AAA, values);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Get attributes map and verify integrity
        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(7), attrs.size());

        // Verify map contents match bitmask pattern (0x2AAA = alternating bits)
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_UNDERLINE));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BORDER_COLOR));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::WIN_OPACITY));
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
    }

    void testCrossComponentMultipleAttributeQueries()
    {
        // Test multiple concurrent attribute queries
        auto packetData = createValidPacket(0x01, 0x02, 0x03, 0x3FFF);

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);

        // Query all attributes in various orders
        auto attrs1 = packet.getAttributes();
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::EDGE_COLOR));
        auto attrs2 = packet.getAttributes();
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        auto attrs3 = packet.getAttributes();

        // Verify all queries return consistent results
        CPPUNIT_ASSERT_EQUAL(attrs1.size(), attrs2.size());
        CPPUNIT_ASSERT_EQUAL(attrs2.size(), attrs3.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), attrs1.size());
    }

    void testCrossComponentLargePacketStream()
    {
        // Simulate processing a stream of packets
        PacketSetCCAttributes packet;

        for (int i = 0; i < 20; ++i) {
            std::uint32_t attribMask = (i % 2 == 0) ? 0x3FFF : 0x0001;
            auto packetData = createValidPacket(i, i * 100, i * 1000, attribMask);
            DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

            CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
            CPPUNIT_ASSERT(packet.isValid() == true);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i), packet.getChannelId());

            // The packet format always includes all 14 attribute values in the buffer
            // The bitmask determines which are valid, but implementation may parse all
            size_t expectedSize = (i % 2 == 0) ? 14 : 14; // Both cases parse all 14
            CPPUNIT_ASSERT_EQUAL(expectedSize, packet.getAttributes().size());
        }
    }

    void testRealWorldCaptionStylingScenario()
    {
        // Realistic scenario: setting caption font and colors
        std::vector<std::uint32_t> values = {
            0xFFFFFFFF, // FONT_COLOR: white
            0x000000FF, // BACKGROUND_COLOR: black
            0xFF,       // FONT_OPACITY: opaque
            0xCC,       // BACKGROUND_OPACITY: mostly opaque
            0x01,       // FONT_STYLE: proportional
            0x03,       // FONT_SIZE: large
            0x00,       // FONT_ITALIC: off
            0x00,       // FONT_UNDERLINE: off
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Others unused
        };
        auto packetData = createValidPacket(0x01, 0x1000, 0x01, 0x003F, values); // First 6 attributes

        PacketSetCCAttributes packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)) == true);
        CPPUNIT_ASSERT(packet.isValid() == true);

        // Verify caption styling attributes
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_COLOR));

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x000000FF),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR));

        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x03),
                           packet.getAttributeValue(PacketSetCCAttributes::CcAttribType::FONT_SIZE));

        auto attrs = packet.getAttributes();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), attrs.size());
    }

    void testRealWorldMultiplePacketsSequence()
    {
        // Realistic scenario: processing sequence of attribute update packets
        // Note: m_attributesMap accumulates across parses (not cleared between parses)
        PacketSetCCAttributes packet;

        // Packet 1: Initial font settings
        // 0x0035 = 0011 0101 = bits 0,2,4,5 = FONT_COLOR, FONT_OPACITY, FONT_STYLE, FONT_SIZE (4 attributes)
        std::vector<std::uint32_t> values1 = {0xFF0000FF, 0, 0xFF, 0, 0x01, 0x02, 0, 0, 0, 0, 0, 0, 0, 0};
        auto data1 = createValidPacket(0x01, 0x1000, 0x01, 0x0035, values1);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getAttributes().size()); // 4 attributes in map

        // Packet 2: Add border styling
        // 0x0300 = bits 8,9 = BORDER_TYPE, BORDER_COLOR (2 NEW attributes)
        // Map accumulates: keeps 4 from packet1 + adds 2 new = 6 total
        std::vector<std::uint32_t> values2 = {0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0x000000FF, 0, 0, 0, 0};
        auto data2 = createValidPacket(0x01, 0x1001, 0x01, 0x0300, values2);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), packet.getAttributes().size()); // 4 previous + 2 new = 6

        // Packet 3: Full update
        // 0x3FFF = all 14 bits set, overwrites all previous values
        std::vector<std::uint32_t> values3;
        for (int i = 0; i < 14; ++i) values3.push_back(i * 0x11111111);
        auto data3 = createValidPacket(0x01, 0x1002, 0x01, 0x3FFF, values3);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size()); // All 14 attributes
    }

    void testRealWorldAttributeUpdateScenario()
    {
        // Scenario: Dynamic attribute updates during playback
        // Note: m_attributesMap accumulates across parses (not cleared between parses)
        PacketSetCCAttributes packet;

        // Initial state: minimal attributes
        // 0x0003 = bits 0,1 = FONT_COLOR, BACKGROUND_COLOR (2 attributes)
        auto data1 = createValidPacket(0x05, 0x2000, 0x01, 0x0003);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x05), packet.getChannelId());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet.getAttributes().size()); // 2 attributes in map

        // Update 1: Add more styling
        // 0x00FF = bits 0-7 = first 8 attributes (overwrites 0,1 + adds 2,3,4,5,6,7)
        auto data2 = createValidPacket(0x05, 0x2001, 0x01, 0x00FF);
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), packet.getAttributes().size()); // All 8 attributes (0-7)

        // Update 2: Change to different attribute set
        // 0x3F00 = bits 8-13 = last 6 attributes (adds 8-13, keeps 0-7)
        // Map accumulates: 8 from packet2 + 6 new = 14 total
        auto data3 = createValidPacket(0x05, 0x2002, 0x01, 0x3F00);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size()); // 8 previous + 6 new = 14

        // Final state verification
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x2002), packet.getCounter());
    }

    void testRealWorldErrorHandlingScenario()
    {
        // Scenario: Robust error handling in production stream
        PacketSetCCAttributes packet;

        // Good packet 1
        auto data1 = createValidPacket(0x10, 0x100, 0x01, 0x0001);
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(data1.begin(), data1.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x10), packet.getChannelId());

        // Corrupted packet (simulate transmission error)
        auto data2 = createValidPacket(0x10, 0x101, 0x01, 0x0002);
        data2[20] = 0xFF; // Corrupt some data
        data2[8] = 0x40;  // Wrong size
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(data2.begin(), data2.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)) == false);

        // Good packet 2 - system recovers
        auto data3 = createValidPacket(0x10, 0x102, 0x01, 0x3FFF);
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(data3.begin(), data3.end());
        CPPUNIT_ASSERT(packet.parse(std::move(buffer3)) == true);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x102), packet.getCounter());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getAttributes().size());

        // Verify system remains stable
        CPPUNIT_ASSERT(packet.isValid() == true);
        CPPUNIT_ASSERT(packet.containsAttribute(PacketSetCCAttributes::CcAttribType::FONT_COLOR));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketSetCCAttributesTest );
