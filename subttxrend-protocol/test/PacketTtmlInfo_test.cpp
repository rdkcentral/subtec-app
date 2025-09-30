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

#include "PacketTtmlInfo.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketTtmlInfo;

class PacketTtmlInfoTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTtmlInfoTest );
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
    CPPUNIT_TEST(testGetContentTypeBasic);
    CPPUNIT_TEST(testGetSubtitleInfoBasic);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testParseMethodBasic);
    CPPUNIT_TEST(testSpaceSeparatorParsing);
    CPPUNIT_TEST(testNoSpaceSeparator);
    CPPUNIT_TEST(testSingleCharWithSpace);
    CPPUNIT_TEST(testMultipleSpaces);
    CPPUNIT_TEST(testEmptyContentType);
    CPPUNIT_TEST(testEmptySubtitleInfo);
    CPPUNIT_TEST(testMinimumValidSize);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testChannelIdBoundaryValues);
    CPPUNIT_TEST(testSizeBoundaryValues);
    CPPUNIT_TEST(testCounterEndianness);
    CPPUNIT_TEST(testChannelIdEndianness);
    CPPUNIT_TEST(testSizeEndianness);
    CPPUNIT_TEST(testEndiannessPatternsComprehensive);
    CPPUNIT_TEST(testTypeAlternativeValues);
    CPPUNIT_TEST(testTypeInvalidValues);
    CPPUNIT_TEST(testCorruptedTypeField);
    CPPUNIT_TEST(testTruncatedPacketVariations);
    CPPUNIT_TEST(testEmptyBuffer);
    CPPUNIT_TEST(testMultipleParseCalls);
    CPPUNIT_TEST(testParseAfterFailedParse);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testSequentialFailures);
    CPPUNIT_TEST(testAllGettersBeforeParsing);
    CPPUNIT_TEST(testBothEmpty);
    CPPUNIT_TEST(testOnlySpaces);
    CPPUNIT_TEST(testSpecialCharacters);
    CPPUNIT_TEST(testVeryLongStrings);
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
                0x13, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0F, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x6E, 0x72, 0x6D, // subtitle info "vod eng nrm"
        };

        PacketTtmlInfo packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 15);
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng nrm");

        std::uint8_t anotherPacketData[] =
        {
                0x13, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0F, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x6C, 0x69, 0x76, 0x65, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x68, 0x68, // subtitle info "live eng hh"
        };

        buffer = std::make_unique<DataBuffer>(std::begin(anotherPacketData), std::end(anotherPacketData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 15);
        CPPUNIT_ASSERT(packet.getContentType() == "live");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng hh");
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF2, 0x00, 0x00, 0x00, // type (invalid)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0E, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x68, 0x68, // subtitle info "vod eng hh"
        };

        PacketTtmlInfo packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x13, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x01, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x68, 0x68, // subtitle info "vod eng hh"
        };

        PacketTtmlInfo packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x13, 0x00, 0x00, 0x00, // type (corrected to TTML_INFO)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0A, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x65, 0x6E, // subtitle info cut
        };

        PacketTtmlInfo packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x13, 0x00, 0x00, 0x00, // type (corrected to TTML_INFO)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0A, 0x00, 0x00, 0x00, // data size
                0x76, 0x54, 0x32, 0x10, // channel id
                0x65, 0x6E, 0x67, 0x20, 0x68, 0x68, 0x68, 0x68, // subtitle info too long
        };

        PacketTtmlInfo packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Basic API coverage tests
    void testConstructorInitialState()
    {
        PacketTtmlInfo packet;
        
        // Test initial state after construction
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testGetTypeBasic()
    {
        PacketTtmlInfo packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        
        // Type should remain consistent after parse attempt
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        packet.parse(std::move(buffer));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
    }

    void testGetCounterBasic()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter (little endian)
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x12345678);
    }

    void testGetSizeBasic()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size (8 bytes)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 8);
    }

    void testGetChannelIdBasic()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x78, 0x56, 0x34, 0x12, // channel id (little endian)
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0x12345678);
    }

    void testGetContentTypeBasic()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, // "vod eng"
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng");
    }

    void testGetSubtitleInfoBasic()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0F, 0x00, 0x00, 0x00, // size = 15 (4 bytes channel + 11 bytes content)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x6C, 0x69, 0x76, 0x65, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x68, 0x68, // "live eng hh"
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "live");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng hh");
    }

    void testIsValidBasic()
    {
        PacketTtmlInfo packet;
        
        // Initially invalid
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Valid after successful parse
        std::uint8_t validData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Invalid after failed parse
        PacketTtmlInfo packet2;
        std::uint8_t invalidData[] = {
            0x13, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet2.isValid());
    }

    void testParseMethodBasic()
    {
        PacketTtmlInfo packet;
        
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, // "vod eng"
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        // Test parse return value
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        
        // Verify all fields were parsed correctly
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getSize() == 11);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xDDCCBBAA);
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng");
    }

    void testSpaceSeparatorParsing()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x12, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x6C, 0x69, 0x76, 0x65, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x73, 0x75, 0x62, 0x74, 0x69, // "live eng subti"
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "live");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng subti");
    }

    void testNoSpaceSeparator()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x07, 0x00, 0x00, 0x00, // size = 7 (4 bytes channel + 3 bytes content)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, // "vod" (no space)
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        // According to implementation: no space found means both strings remain empty
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testSingleCharWithSpace()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x06, 0x00, 0x00, 0x00, // size = 6 (4 bytes channel + 2 bytes content)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x20, // "v " (single char + space)
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "v");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testMultipleSpaces()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0D, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, 0x20, 0x65, 0x6E, 0x67, 0x20, // "vod  eng "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == " eng ");
    }

    void testEmptyContentType()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x07, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x20, 0x65, 0x6E, // " en" (space at beginning)
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "en");
    }

    void testEmptySubtitleInfo()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod " (space at end)
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testMinimumValidSize()
    {
        // Minimum valid size should be 4 (channel ID) + 1 (at least one byte of data)
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x05, 0x00, 0x00, 0x00, // size = 5 (minimum)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, // "v" (single character, no space)
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        // According to implementation: no space found means both strings remain empty
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    // Edge cases and error handling tests
    void testCounterBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter = 0
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getCounter() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter = 0xFFFFFFFF
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getCounter() == 0xFFFFFFFF);
    }

    void testChannelIdBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, // channel id = 0
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getChannelId() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // channel id = 0xFFFFFFFF
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getChannelId() == 0xFFFFFFFF);
    }

    void testSizeBoundaryValues()
    {
        // Test size that's too small (less than channel ID size)
        std::uint8_t tooSmallData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x03, 0x00, 0x00, 0x00, // size = 3 (too small)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, // "vod"
        };
        
        PacketTtmlInfo packetSmall;
        DataBufferPtr bufferSmall = std::make_unique<DataBuffer>(std::begin(tooSmallData), std::end(tooSmallData));
        
        CPPUNIT_ASSERT(!packetSmall.parse(std::move(bufferSmall)));
        CPPUNIT_ASSERT(!packetSmall.isValid());
    }

    void testCounterEndianness()
    {
        // Test specific endianness patterns for counter field
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter (little endian pattern)
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412); // Little endian interpretation
    }

    void testChannelIdEndianness()
    {
        // Test specific endianness patterns for channel ID field
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0xAB, 0xCD, 0xEF, 0x12, // channel id (little endian pattern)
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0x12EFCDAB); // Little endian interpretation
    }

    void testSizeEndianness()
    {
        // Test specific endianness patterns for size field
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size = 8 (little endian)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 8); // Little endian interpretation
    }

    void testEndiannessPatternsComprehensive()
    {
        // Test comprehensive endianness with alternating bit patterns
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0xAA, 0x55, 0xAA, 0x55, // counter (alternating pattern)
            0x08, 0x00, 0x00, 0x00, // size = 8
            0x33, 0xCC, 0x33, 0xCC, // channel id (alternating pattern)
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x55AA55AA);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xCC33CC33);
    }

    void testTypeAlternativeValues()
    {
        // Test with exact expected type value (0x13)
        std::uint8_t validType[] = {
            0x13, 0x00, 0x00, 0x00, // type = 19 (TTML_INFO)
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validType), std::end(validType));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
    }

    void testTypeInvalidValues()
    {
        // Test various invalid type values
        std::uint32_t invalidTypes[] = {0, 1, 2, 6, 10, 18, 20, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};
        
        for (std::uint32_t invalidType : invalidTypes) {
            std::uint8_t packetData[] = {
                static_cast<std::uint8_t>(invalidType & 0xFF),
                static_cast<std::uint8_t>((invalidType >> 8) & 0xFF),
                static_cast<std::uint8_t>((invalidType >> 16) & 0xFF),
                static_cast<std::uint8_t>((invalidType >> 24) & 0xFF),
                0x01, 0x23, 0x45, 0x67, // counter
                0x08, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x76, 0x6F, 0x64, 0x20, // "vod "
            };
            
            PacketTtmlInfo packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            
            CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testCorruptedTypeField()
    {
        // Test corrupted type field that's close but not exact
        std::uint8_t corruptedData[] = {
            0x13, 0x01, 0x00, 0x00, // type = 0x00000113 (corrupted)
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(corruptedData), std::end(corruptedData));
        
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTruncatedPacketVariations()
    {
        // Test various truncation points
        std::uint8_t fullPacket[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0B, 0x00, 0x00, 0x00, // size = 11
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, // "vod eng"
        };
        
        // Test truncation at various points
        for (std::size_t truncateAt = 1; truncateAt < sizeof(fullPacket); ++truncateAt) {
            PacketTtmlInfo packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(fullPacket), std::begin(fullPacket) + truncateAt);
            
            CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testEmptyBuffer()
    {
        PacketTtmlInfo packet;
        
        // Test with completely empty buffer - use default constructor like other tests
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!packet.parse(std::move(emptyBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testMultipleParseCalls()
    {
        PacketTtmlInfo packet;
        
        // First parse with valid data
        std::uint8_t validData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Second parse with different valid data
        std::uint8_t validData2[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // different counter
            0x0D, 0x00, 0x00, 0x00, // size = 13
            0x05, 0x06, 0x07, 0x08, // different channel id
            0x6C, 0x69, 0x76, 0x65, 0x20, 0x65, 0x6E, 0x67, 0x20, // "live eng "
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData2), std::end(validData2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Verify new values were parsed
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x08070605);
        CPPUNIT_ASSERT(packet.getContentType() == "live");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng ");
    }

    void testParseAfterFailedParse()
    {
        PacketTtmlInfo packet;
        
        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0x13, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x20, // "vod "
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Second parse with valid data should succeed
        std::uint8_t validData[] = {
            0x13, 0x00, 0x00, 0x00, // valid type
            0x11, 0x22, 0x33, 0x44, // counter
            0x0B, 0x00, 0x00, 0x00, // size
            0x05, 0x06, 0x07, 0x08, // channel id
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, // "vod eng"
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testPacketReuse()
    {
        PacketTtmlInfo packet;
        
        // Parse multiple different packets with same object
        for (int i = 0; i < 3; ++i) {
            std::uint8_t packetData[] = {
                0x13, 0x00, 0x00, 0x00, // type
                static_cast<std::uint8_t>(i), 0x00, 0x00, 0x00, // counter = i
                0x08, 0x00, 0x00, 0x00, // size
                static_cast<std::uint8_t>(i + 1), 0x00, 0x00, 0x00, // channel id = i+1
                0x76, 0x6F, 0x64, 0x20, // "vod "
            };
            
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getCounter() == static_cast<std::uint32_t>(i));
            CPPUNIT_ASSERT(packet.getChannelId() == static_cast<std::uint32_t>(i + 1));
            CPPUNIT_ASSERT(packet.getContentType() == "vod");
        }
    }

    void testSequentialFailures()
    {
        PacketTtmlInfo packet;
        
        // Multiple sequential failures should maintain invalid state
        std::uint8_t invalidData1[] = {
            0x13, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size
        };
        
        std::uint8_t invalidData2[] = {
            0x13, 0x00, 0x00, 0x00, // valid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x02, 0x00, 0x00, 0x00, // invalid size (too small)
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidData1), std::end(invalidData1));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData2), std::end(invalidData2));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testAllGettersBeforeParsing()
    {
        PacketTtmlInfo packet;
        
        // Test all getters before any parsing
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testBothEmpty()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x05, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x20, // single space
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "");
    }

    void testOnlySpaces()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x07, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x20, 0x20, 0x20, // three spaces
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "  ");
    }

    void testSpecialCharacters()
    {
        std::uint8_t packetData[] = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0F, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x76, 0x6F, 0x64, 0x2D, 0x31, 0x20, 0x65, 0x6E, 0x67, 0x2D, 0x55, // "vod-1 eng-U"
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "vod-1");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng-U");
    }

    void testVeryLongStrings()
    {
        // Create a packet with long content type and subtitle info
        std::vector<std::uint8_t> packetData = {
            0x13, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
        };
        
        // Long content: "verylongcontenttype verylongsubtitleinformation..."
        std::string longContent = "verylongcontenttype verylongsubtitleinformationwithmanycharacters";
        std::uint32_t size = 4 + longContent.length(); // channel id + content
        
        // Add size (little endian)
        packetData.push_back(size & 0xFF);
        packetData.push_back((size >> 8) & 0xFF);
        packetData.push_back((size >> 16) & 0xFF);
        packetData.push_back((size >> 24) & 0xFF);
        
        // Add channel id
        packetData.insert(packetData.end(), {0x01, 0x02, 0x03, 0x04});
        
        // Add content
        packetData.insert(packetData.end(), longContent.begin(), longContent.end());
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetData.begin(), packetData.end());
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getContentType() == "verylongcontenttype");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "verylongsubtitleinformationwithmanycharacters");
    }

    void testExactSizeMatching()
    {
        // Test that packet requires exact size match
        std::uint8_t exactData[] = {
            0x13, 0x00, 0x00, 0x00, // type (4 bytes)
            0x01, 0x23, 0x45, 0x67, // counter (4 bytes)
            0x0B, 0x00, 0x00, 0x00, // size = 11 (4 bytes)
            0x01, 0x02, 0x03, 0x04, // channel id (4 bytes)
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, // content: "vod eng" (7 bytes) - total 19 bytes
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(exactData), std::end(exactData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSize() == 11);
        CPPUNIT_ASSERT(packet.getContentType() == "vod");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng");
    }

    void testProtocolCompliance()
    {
        // Test protocol compliance with realistic scenarios
        
        // Test realistic TTML info packets
        std::uint8_t realisticData[] = {
            0x13, 0x00, 0x00, 0x00, // type = TTML_INFO
            0x00, 0x10, 0x00, 0x00, // counter = 0x1000
            0x13, 0x00, 0x00, 0x00, // size = 19
            0x01, 0x00, 0x00, 0x00, // channel id = 1
            0x6C, 0x69, 0x76, 0x65, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x68, 0x64, 0x20, 0x63, 0x63, 0x31, // "live eng hd cc1"
        };
        
        PacketTtmlInfo packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(realisticData), std::end(realisticData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TTML_INFO);
        CPPUNIT_ASSERT(packet.getCounter() == 0x1000);
        CPPUNIT_ASSERT(packet.getSize() == 19);
        CPPUNIT_ASSERT(packet.getChannelId() == 1);
        CPPUNIT_ASSERT(packet.getContentType() == "live");
        CPPUNIT_ASSERT(packet.getSubtitleInfo() == "eng hd cc1");
        
        // Test another realistic scenario
        std::uint8_t vodData[] = {
            0x13, 0x00, 0x00, 0x00, // type = TTML_INFO
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x0F, 0x00, 0x00, 0x00, // size = 15
            0x02, 0x00, 0x00, 0x00, // channel id = 2
            0x76, 0x6F, 0x64, 0x20, 0x65, 0x6E, 0x67, 0x20, 0x6E, 0x6F, 0x72, // "vod eng nor"
        };
        
        PacketTtmlInfo vodPacket;
        DataBufferPtr vodBuffer = std::make_unique<DataBuffer>(std::begin(vodData), std::end(vodData));
        
        CPPUNIT_ASSERT(vodPacket.parse(std::move(vodBuffer)));
        CPPUNIT_ASSERT(vodPacket.isValid());
        CPPUNIT_ASSERT(vodPacket.getContentType() == "vod");
        CPPUNIT_ASSERT(vodPacket.getSubtitleInfo() == "eng nor");
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTtmlInfoTest );
