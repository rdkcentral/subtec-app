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

#include "PacketTeletextSelection.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketTeletextSelection;

class PacketTeletextSelectionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTeletextSelectionTest );
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
    CPPUNIT_TEST(testGetInitialMagazineBasic);
    CPPUNIT_TEST(testGetInitialPageBasic);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testParseMethodBasic);
    CPPUNIT_TEST(testDefaultMagazineAndPageValues);
    CPPUNIT_TEST(testZeroValues);
    CPPUNIT_TEST(testSimpleIncrementalValues);
    CPPUNIT_TEST(testSize4PacketStructure);
    CPPUNIT_TEST(testSize12PacketStructure);
    CPPUNIT_TEST(testSizeEdgeCases);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testChannelIdBoundaryValues);
    CPPUNIT_TEST(testMagazineBoundaryValues);
    CPPUNIT_TEST(testPageBoundaryValues);
    CPPUNIT_TEST(testCounterEndianness);
    CPPUNIT_TEST(testChannelIdEndianness);
    CPPUNIT_TEST(testMagazineEndianness);
    CPPUNIT_TEST(testPageEndianness);
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
    CPPUNIT_TEST(testAllGettersAfterFailedParsing);
    CPPUNIT_TEST(testCorruptedHeaderFields);
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
                0x06, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketTeletextSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 4);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x04030201);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0x06, 0x00, 0xFF, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
        };

        PacketTeletextSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x06, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x05, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x00
        };

        PacketTeletextSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x06, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x02, 0x03, 0x04, // channel id (cut)
        };

        PacketTeletextSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x06, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x04, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketTeletextSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    // Basic API coverage tests
    void testConstructorInitialState()
    {
        PacketTeletextSelection packet;
        
        // Test initial state after construction
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x1);  // Default value
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);        // Default value
    }

    void testGetTypeBasic()
    {
        PacketTeletextSelection packet;
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        
        // Type should remain consistent after parse attempt
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        packet.parse(std::move(buffer));
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
    }

    void testGetCounterBasic()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter (little endian)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x12345678);
    }

    void testGetSizeBasic()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size (4 bytes)
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getSize() == 4);
    }

    void testGetChannelIdBasic()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x78, 0x56, 0x34, 0x12, // channel id (little endian)
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0x12345678);
    }

    void testGetInitialMagazineBasic()
    {
        // Test default value with size 4 packet (no magazine/page data)
        std::uint8_t packetData4[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet4;
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(packetData4), std::end(packetData4));
        
        CPPUNIT_ASSERT(packet4.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet4.getInitialMagazine() == 0x1); // Default value
        
        // Test with size 12 packet (includes magazine/page data)
        std::uint8_t packetData12[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x05, 0x00, 0x00, 0x00, // magazine
            0x06, 0x00, 0x00, 0x00, // page
        };
        
        PacketTeletextSelection packet12;
        DataBufferPtr buffer12 = std::make_unique<DataBuffer>(std::begin(packetData12), std::end(packetData12));
        
        CPPUNIT_ASSERT(packet12.parse(std::move(buffer12)));
        CPPUNIT_ASSERT(packet12.getInitialMagazine() == 0x5);
    }

    void testGetInitialPageBasic()
    {
        // Test default value with size 4 packet (no magazine/page data)
        std::uint8_t packetData4[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet4;
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(packetData4), std::end(packetData4));
        
        CPPUNIT_ASSERT(packet4.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet4.getInitialPage() == 0); // Default value
        
        // Test with size 12 packet (includes magazine/page data)
        std::uint8_t packetData12[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x05, 0x00, 0x00, 0x00, // magazine
            0x07, 0x00, 0x00, 0x00, // page
        };
        
        PacketTeletextSelection packet12;
        DataBufferPtr buffer12 = std::make_unique<DataBuffer>(std::begin(packetData12), std::end(packetData12));
        
        CPPUNIT_ASSERT(packet12.parse(std::move(buffer12)));
        CPPUNIT_ASSERT(packet12.getInitialPage() == 0x7);
    }

    void testIsValidBasic()
    {
        PacketTeletextSelection packet;
        
        // Initially invalid
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Valid after successful parse
        std::uint8_t validData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Invalid after failed parse
        PacketTeletextSelection packet2;
        std::uint8_t invalidData[] = {
            0x06, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet2.isValid());
    }

    void testParseMethodBasic()
    {
        PacketTeletextSelection packet;
        
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xAA, 0xBB, 0xCC, 0xDD, // channel id
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        // Test parse return value
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        
        // Verify all fields were parsed correctly
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getSize() == 4);
        CPPUNIT_ASSERT(packet.getChannelId() == 0xDDCCBBAA);
    }

    void testDefaultMagazineAndPageValues()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size = 4 (no magazine/page data)
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Should use default values
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x1);
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);
    }

    void testZeroValues()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter = 0
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x00, 0x00, 0x00, 0x00, // channel id = 0
            0x00, 0x00, 0x00, 0x00, // magazine = 0
            0x00, 0x00, 0x00, 0x00, // page = 0
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0);
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);
    }

    void testSimpleIncrementalValues()
    {
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x02, 0x03, 0x04, // counter (incremental bytes)
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x05, 0x06, 0x07, 0x08, // channel id (incremental bytes)
            0x09, 0x0A, 0x0B, 0x0C, // magazine (incremental bytes)
            0x0D, 0x0E, 0x0F, 0x10, // page (incremental bytes)
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x04030201);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x08070605);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x0C0B0A09);
        CPPUNIT_ASSERT(packet.getInitialPage() == 0x100F0E0D);
    }

    void testSize4PacketStructure()
    {
        // Test minimal valid packet structure
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0xFF, 0xEE, 0xDD, 0xCC, // counter
            0x04, 0x00, 0x00, 0x00, // size = 4 (only channel id)
            0x11, 0x22, 0x33, 0x44, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSize() == 4);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x44332211);
        
        // Should use default magazine/page values
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x1);
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);
    }

    void testSize12PacketStructure()
    {
        // Test full packet structure with magazine and page
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0xAA, 0xBB, 0xCC, 0xDD, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12 (channel id + magazine + page)
            0x11, 0x22, 0x33, 0x44, // channel id
            0x55, 0x66, 0x77, 0x88, // magazine
            0x99, 0xAA, 0xBB, 0xCC, // page
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSize() == 12);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x44332211);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x88776655);
        CPPUNIT_ASSERT(packet.getInitialPage() == 0xCCBBAA99);
    }

    // Edge cases and error handling tests
    void testSizeEdgeCases()
    {
        // Test invalid sizes (not 4 or 12)
        std::uint8_t invalidSizes[] = {0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16};
        
        for (std::uint8_t invalidSize : invalidSizes) {
            std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            invalidSize, 0x00, 0x00, 0x00, // invalid size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x00, 0x00, 0x00, 0x00, // padding
            0x00, 0x00, 0x00, 0x00, // padding
            0x00, 0x00, 0x00, 0x00  // padding
            };
            
            PacketTeletextSelection packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            
            CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testCounterBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter = 0
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getCounter() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter = 0xFFFFFFFF
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getCounter() == 0xFFFFFFFF);
    }

    void testChannelIdBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, // channel id = 0
        };
        
        PacketTeletextSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getChannelId() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // channel id = 0xFFFFFFFF
        };
        
        PacketTeletextSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getChannelId() == 0xFFFFFFFF);
    }

    void testMagazineBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x00, 0x00, 0x00, 0x00, // magazine = 0
            0x01, 0x02, 0x03, 0x04, // page
        };
        
        PacketTeletextSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getInitialMagazine() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0xFF, 0xFF, 0xFF, 0xFF, // magazine = 0xFFFFFFFF
            0x01, 0x02, 0x03, 0x04, // page
        };
        
        PacketTeletextSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getInitialMagazine() == 0xFFFFFFFF);
    }

    void testPageBoundaryValues()
    {
        // Test minimum value (0)
        std::uint8_t minData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, // magazine
            0x00, 0x00, 0x00, 0x00, // page = 0
        };
        
        PacketTeletextSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(minData), std::end(minData));
        
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getInitialPage() == 0);
        
        // Test maximum value (0xFFFFFFFF)
        std::uint8_t maxData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, // magazine
            0xFF, 0xFF, 0xFF, 0xFF, // page = 0xFFFFFFFF
        };
        
        PacketTeletextSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(maxData), std::end(maxData));
        
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getInitialPage() == 0xFFFFFFFF);
    }

    void testCounterEndianness()
    {
        // Test specific endianness patterns for counter field
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x12, 0x34, 0x56, 0x78, // counter (little endian pattern)
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getCounter() == 0x78563412); // Little endian interpretation
    }

    void testChannelIdEndianness()
    {
        // Test specific endianness patterns for channel ID field
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0xAB, 0xCD, 0xEF, 0x12, // channel id (little endian pattern)
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getChannelId() == 0x12EFCDAB); // Little endian interpretation
    }

    void testMagazineEndianness()
    {
        // Test specific endianness patterns for magazine field
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0xFE, 0xDC, 0xBA, 0x98, // magazine (little endian pattern)
            0x01, 0x02, 0x03, 0x04, // page
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x98BADCFE); // Little endian interpretation
    }

    void testPageEndianness()
    {
        // Test specific endianness patterns for page field
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x02, 0x03, 0x04, // magazine
            0x87, 0x65, 0x43, 0x21, // page (little endian pattern)
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getInitialPage() == 0x21436587); // Little endian interpretation
    }

    void testEndiannessPatternsComprehensive()
    {
        // Test comprehensive endianness with alternating bit patterns
        std::uint8_t packetData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0xAA, 0x55, 0xAA, 0x55, // counter (alternating pattern)
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x33, 0xCC, 0x33, 0xCC, // channel id (alternating pattern)
            0x0F, 0xF0, 0x0F, 0xF0, // magazine (alternating pattern)
            0x99, 0x66, 0x99, 0x66, // page (alternating pattern)
        };
        
        PacketTeletextSelection alternatingPacket;
        DataBufferPtr alternatingBuffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        
        CPPUNIT_ASSERT(alternatingPacket.parse(std::move(alternatingBuffer)));
        CPPUNIT_ASSERT(alternatingPacket.getCounter() == 0x55AA55AA);
        CPPUNIT_ASSERT(alternatingPacket.getChannelId() == 0xCC33CC33);
        CPPUNIT_ASSERT(alternatingPacket.getInitialMagazine() == 0xF00FF00F);
        CPPUNIT_ASSERT(alternatingPacket.getInitialPage() == 0x66996699);
    }

    void testTypeAlternativeValues()
    {
        // Test with exact expected type value (0x06)
        std::uint8_t validType[] = {
            0x06, 0x00, 0x00, 0x00, // type = 6 (TELETEXT_SELECTION)
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(validType), std::end(validType));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
    }

    void testTypeInvalidValues()
    {
        // Test various invalid type values
        std::uint32_t invalidTypes[] = {0, 1, 2, 3, 4, 5, 7, 8, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};
        
        for (std::uint32_t invalidType : invalidTypes) {
            std::uint8_t packetData[] = {
            static_cast<std::uint8_t>(invalidType & 0xFF),
            static_cast<std::uint8_t>((invalidType >> 8) & 0xFF),
            static_cast<std::uint8_t>((invalidType >> 16) & 0xFF),
            static_cast<std::uint8_t>((invalidType >> 24) & 0xFF),
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            };
            
            PacketTeletextSelection packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            
            CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testCorruptedTypeField()
    {
        // Test corrupted type field that's close but not exact
        std::uint8_t corruptedData[] = {
            0x06, 0x01, 0x00, 0x00, // type = 0x00000106 (corrupted)
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(corruptedData), std::end(corruptedData));
        
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTruncatedPacketVariations()
    {
        // Test various truncation points
        std::uint8_t fullPacket[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x02, 0x03, 0x04, // channel id
            0x05, 0x06, 0x07, 0x08, // magazine
            0x09, 0x0A, 0x0B, 0x0C, // page
        };
        
        // Test truncation at various points
        for (std::size_t truncateAt = 1; truncateAt < sizeof(fullPacket); ++truncateAt) {
            PacketTeletextSelection packet;
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(fullPacket), std::begin(fullPacket) + truncateAt);
            
            CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(!packet.isValid());
        }
    }

    void testEmptyBuffer()
    {
        PacketTeletextSelection packet;
        
        // Test with completely empty buffer - use default constructor like other tests
        DataBufferPtr emptyBuffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!packet.parse(std::move(emptyBuffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testMultipleParseCalls()
    {
        PacketTeletextSelection packet;
        
        // First parse with valid data
        std::uint8_t validData[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Second parse with different valid data
        std::uint8_t validData2[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // different counter
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x05, 0x06, 0x07, 0x08, // different channel id
            0x09, 0x0A, 0x0B, 0x0C, // magazine
            0x0D, 0x0E, 0x0F, 0x10, // page
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData2), std::end(validData2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        
        // Verify new values were parsed
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x08070605);
    }

    void testParseAfterFailedParse()
    {
        PacketTeletextSelection packet;
        
        // First parse with invalid data
        std::uint8_t invalidData[] = {
            0x06, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet.isValid());
        
        // Second parse with valid data should succeed
        std::uint8_t validData[] = {
            0x06, 0x00, 0x00, 0x00, // valid type
            0x11, 0x22, 0x33, 0x44, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x05, 0x06, 0x07, 0x08, // channel id
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(validData), std::end(validData));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testPacketReuse()
    {
        PacketTeletextSelection packet;
        
        // Parse multiple different packets with same object
        for (int i = 0; i < 5; ++i) {
            std::uint8_t packetData[] = {
                0x06, 0x00, 0x00, 0x00, // type
                static_cast<std::uint8_t>(i), 0x00, 0x00, 0x00, // counter = i
                0x04, 0x00, 0x00, 0x00, // size
                static_cast<std::uint8_t>(i + 1), 0x00, 0x00, 0x00, // channel id = i+1
            };
            
            DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
            CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
            CPPUNIT_ASSERT(packet.isValid());
            CPPUNIT_ASSERT(packet.getCounter() == static_cast<std::uint32_t>(i));
            CPPUNIT_ASSERT(packet.getChannelId() == static_cast<std::uint32_t>(i + 1));
        }
    }

    void testSequentialFailures()
    {
        PacketTeletextSelection packet;
        
        // Multiple sequential failures should maintain invalid state
        std::uint8_t invalidData1[] = {
            0x06, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
        };
        
        std::uint8_t invalidData2[] = {
            0x06, 0x00, 0x00, 0x00, // valid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x05, 0x00, 0x00, 0x00, // invalid size
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
        PacketTeletextSelection packet;
        
        // Test all getters before any parsing
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
        CPPUNIT_ASSERT(packet.getChannelId() == 0);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x1); // Default
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);       // Default
    }

    void testAllGettersAfterFailedParsing()
    {
        PacketTeletextSelection packet;
        
        // Failed parse
        std::uint8_t invalidData[] = {
            0x06, 0x00, 0xFF, 0x00, // invalid type
            0x01, 0x23, 0x45, 0x67, // counter
            0x04, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(invalidData), std::end(invalidData));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        
        // Test all getters after failed parsing
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        // Note: counter, size, channelId behavior after failed parse is implementation-dependent
        // but should not crash
        packet.getCounter();
        packet.getSize();
        packet.getChannelId();
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 0x1); // Should retain defaults
        CPPUNIT_ASSERT(packet.getInitialPage() == 0);
    }

    void testCorruptedHeaderFields()
    {
        // Test with corrupted size field that doesn't match actual data size
        std::uint8_t corruptedSize[] = {
            0x06, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x08, 0x00, 0x00, 0x00, // size = 8 (invalid for this packet type)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x05, 0x06, 0x07, 0x08, // extra data
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(corruptedSize), std::end(corruptedSize));
        
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testExactSizeMatching()
    {
        // Test that packet requires exact size match (no extra or missing bytes)
        
        // Size 4 packet with exact 16 bytes total (4 header + 4 size + 4 channel + 4 data)
        std::uint8_t exactSize4[] = {
            0x06, 0x00, 0x00, 0x00, // type (4 bytes)
            0x01, 0x23, 0x45, 0x67, // counter (4 bytes)
            0x04, 0x00, 0x00, 0x00, // size = 4 (4 bytes)
            0x01, 0x02, 0x03, 0x04, // channel id (4 bytes) - total 16 bytes
        };
        
        PacketTeletextSelection packet4;
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(exactSize4), std::end(exactSize4));
        CPPUNIT_ASSERT(packet4.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet4.isValid());
        
        // Size 12 packet with exact 24 bytes total
        std::uint8_t exactSize12[] = {
            0x06, 0x00, 0x00, 0x00, // type (4 bytes)
            0x01, 0x23, 0x45, 0x67, // counter (4 bytes)
            0x0C, 0x00, 0x00, 0x00, // size = 12 (4 bytes)
            0x01, 0x02, 0x03, 0x04, // channel id (4 bytes)
            0x05, 0x06, 0x07, 0x08, // magazine (4 bytes)
            0x09, 0x0A, 0x0B, 0x0C, // page (4 bytes) - total 24 bytes
        };
        
        PacketTeletextSelection packet12;
        DataBufferPtr buffer12 = std::make_unique<DataBuffer>(std::begin(exactSize12), std::end(exactSize12));
        CPPUNIT_ASSERT(packet12.parse(std::move(buffer12)));
        CPPUNIT_ASSERT(packet12.isValid());
    }

    void testProtocolCompliance()
    {
        // Test protocol compliance with various realistic scenarios
        
        // Test realistic channel IDs, magazines, and pages
        std::uint8_t realisticData[] = {
            0x06, 0x00, 0x00, 0x00, // type = TELETEXT_SELECTION
            0x00, 0x10, 0x00, 0x00, // counter = 0x1000
            0x0C, 0x00, 0x00, 0x00, // size = 12
            0x01, 0x00, 0x00, 0x00, // channel id = 1
            0x01, 0x00, 0x00, 0x00, // magazine = 1 (typical teletext magazine)
            0x64, 0x00, 0x00, 0x00, // page = 100 (typical teletext page)
        };
        
        PacketTeletextSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(realisticData), std::end(realisticData));
        
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TELETEXT_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x1000);
        CPPUNIT_ASSERT(packet.getSize() == 12);
        CPPUNIT_ASSERT(packet.getChannelId() == 1);
        CPPUNIT_ASSERT(packet.getInitialMagazine() == 1);
        CPPUNIT_ASSERT(packet.getInitialPage() == 100);
        
        // Test minimal compliant packet (size 4)
        std::uint8_t minimalData[] = {
            0x06, 0x00, 0x00, 0x00, // type = TELETEXT_SELECTION
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x04, 0x00, 0x00, 0x00, // size = 4 (minimal)
            0x01, 0x00, 0x00, 0x00, // channel id = 1
        };
        
        PacketTeletextSelection minimalPacket;
        DataBufferPtr minimalBuffer = std::make_unique<DataBuffer>(std::begin(minimalData), std::end(minimalData));
        
        CPPUNIT_ASSERT(minimalPacket.parse(std::move(minimalBuffer)));
        CPPUNIT_ASSERT(minimalPacket.isValid());
        CPPUNIT_ASSERT(minimalPacket.getSize() == 4);
        CPPUNIT_ASSERT(minimalPacket.getInitialMagazine() == 0x1); // Default
        CPPUNIT_ASSERT(minimalPacket.getInitialPage() == 0);       // Default
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTeletextSelectionTest );
