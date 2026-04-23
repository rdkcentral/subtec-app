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

#include "PacketSubtitleSelection.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketSubtitleSelection;

class PacketSubtitleSelectionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketSubtitleSelectionTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadSize);
    CPPUNIT_TEST(testTooShort);
    CPPUNIT_TEST(testTooLong);
    
    CPPUNIT_TEST(testConstructorAndBasicMethods);
    CPPUNIT_TEST(testGetAuxId1Basic);
    CPPUNIT_TEST(testGetAuxId2Basic);
    CPPUNIT_TEST(testSubtitleTypeDVB);
    CPPUNIT_TEST(testSubtitleTypeTeletext);
    CPPUNIT_TEST(testSubtitleTypeSCTE);
    CPPUNIT_TEST(testSubtitleTypeCC);
    CPPUNIT_TEST(testInvalidSubtitleType);
    CPPUNIT_TEST(testSubtitleTypeBoundaryValues);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testChannelIdBoundaryValues);
    CPPUNIT_TEST(testAuxIdBoundaryValues);
    CPPUNIT_TEST(testEndiannessPatternsComprehensive);
    CPPUNIT_TEST(testTruncatedPacketVariations);
    CPPUNIT_TEST(testPacketReuse);
    CPPUNIT_TEST(testCorruptedHeaderFields);
    CPPUNIT_TEST(testEmptyBuffer);
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
                0x05, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x10, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x01, 0x00, 0x00, 0x00, // subtitles_type
                0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
                0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SUBTITLE_SELECTION);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 16);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x04030201);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 1);
        CPPUNIT_ASSERT(packet.getAuxId1() == 0xCCDDEEFF);
        CPPUNIT_ASSERT(packet.getAuxId2() == 0xBBCCDDEE);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0x05, 0xFF, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x10, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x01, 0x00, 0x00, 0x00, // subtitles_type
                0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
                0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x05, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x11, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x01, 0x00, 0x00, 0x00, // subtitles_type
                0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
                0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
                0x00
        };

        PacketSubtitleSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x05, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x10, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x01, 0x00, 0x00, 0x00, // subtitles_type
                0xFF, 0xEE, // aux_id1 (cut)
        };

        PacketSubtitleSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x05, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x10, 0x00, 0x00, 0x00, // size
                0x01, 0x02, 0x03, 0x04, // channel id
                0x01, 0x00, 0x00, 0x00, // subtitles_type
                0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
                0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
                0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketSubtitleSelection packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }
    
    // Basic API functionality
    void testConstructorAndBasicMethods()
    {
        PacketSubtitleSelection packet;
        
        // Test initial state before parsing
        CPPUNIT_ASSERT(!packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::SUBTITLE_SELECTION);
        
        // Test basic parsing and validation
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x02, 0x00, 0x00, 0x00, // subtitles_type = SCTE
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));
        bool parseResult = packet.parse(std::move(buffer));
        
        CPPUNIT_ASSERT(parseResult);
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_SCTE);
    }
    
    void testGetAuxId1Basic()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0x12, 0x34, 0x56, 0x78, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getAuxId1() == 0x78563412); // little-endian
    }
    
    void testGetAuxId2Basic()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xAA, 0xBB, 0xCC, 0xDD, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.getAuxId2() == 0xDDCCBBAA); // little-endian
    }
    
    
    // Subtitle type values
    void testSubtitleTypeDVB()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x00, 0x00, 0x00, 0x00, // subtitles_type = DVB
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_DVB);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 0);
    }
    
    void testSubtitleTypeTeletext()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type = TELETEXT
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 1);
    }
    
    void testSubtitleTypeSCTE()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x02, 0x00, 0x00, 0x00, // subtitles_type = SCTE
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_SCTE);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 2);
    }
    
    void testSubtitleTypeCC()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x03, 0x00, 0x00, 0x00, // subtitles_type = CC
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_CC);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 3);
    }
    
    void testInvalidSubtitleType()
    {
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x99, 0x00, 0x00, 0x00, // subtitles_type = invalid (153)
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // Should parse successfully but return the invalid value
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == 153);
    }
    
    void testSubtitleTypeBoundaryValues()
    {
        // Test subtitle type value 4 (just after highest valid type)
        std::uint8_t packetData4[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x04, 0x00, 0x00, 0x00, // subtitles_type = 4
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet4;
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(packetData4), std::end(packetData4));
        CPPUNIT_ASSERT(packet4.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(packet4.getSubtitlesType() == 4);

        // Test maximum subtitle type value
        std::uint8_t packetDataMax[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0xFF, 0xFF, 0xFF, 0xFF, // subtitles_type = 0xFFFFFFFF
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(packetDataMax), std::end(packetDataMax));
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getSubtitlesType() == 0xFFFFFFFF);
    }
    
    // Boundary values and edge cases
    void testCounterBoundaryValues()
    {
        // Test minimum counter value
        std::uint8_t packetDataMin[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x00, 0x00, 0x00, 0x00, // counter = 0
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(packetDataMin), std::end(packetDataMin));
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getCounter() == 0);

        // Test maximum counter value
        std::uint8_t packetDataMax[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0xFF, 0xFF, 0xFF, 0xFF, // counter = 0xFFFFFFFF
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(packetDataMax), std::end(packetDataMax));
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getCounter() == 0xFFFFFFFF);
    }
    
    void testChannelIdBoundaryValues()
    {
        // Test minimum channel ID value
        std::uint8_t packetDataMin[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x00, 0x00, 0x00, 0x00, // channel id = 0
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(packetDataMin), std::end(packetDataMin));
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getChannelId() == 0);

        // Test maximum channel ID value
        std::uint8_t packetDataMax[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0xFF, 0xFF, 0xFF, 0xFF, // channel id = 0xFFFFFFFF
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(packetDataMax), std::end(packetDataMax));
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getChannelId() == 0xFFFFFFFF);
    }
    
    void testAuxIdBoundaryValues()
    {
        // Test minimum aux ID values
        std::uint8_t packetDataMin[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0x00, 0x00, 0x00, 0x00, // aux_id1 = 0
            0x00, 0x00, 0x00, 0x00, // aux_id2 = 0
        };

        PacketSubtitleSelection packetMin;
        DataBufferPtr bufferMin = std::make_unique<DataBuffer>(std::begin(packetDataMin), std::end(packetDataMin));
        CPPUNIT_ASSERT(packetMin.parse(std::move(bufferMin)));
        CPPUNIT_ASSERT(packetMin.getAuxId1() == 0);
        CPPUNIT_ASSERT(packetMin.getAuxId2() == 0);

        // Test maximum aux ID values
        std::uint8_t packetDataMax[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xFF, 0xFF, 0xFF, // aux_id1 = 0xFFFFFFFF
            0xFF, 0xFF, 0xFF, 0xFF, // aux_id2 = 0xFFFFFFFF
        };

        PacketSubtitleSelection packetMax;
        DataBufferPtr bufferMax = std::make_unique<DataBuffer>(std::begin(packetDataMax), std::end(packetDataMax));
        CPPUNIT_ASSERT(packetMax.parse(std::move(bufferMax)));
        CPPUNIT_ASSERT(packetMax.getAuxId1() == 0xFFFFFFFF);
        CPPUNIT_ASSERT(packetMax.getAuxId2() == 0xFFFFFFFF);
    }
    
    void testEndiannessPatternsComprehensive()
    {
        // Test specific endianness patterns
        std::uint8_t packetData[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x78, 0x56, 0x34, 0x12, // counter = 0x12345678 in little-endian
            0x10, 0x00, 0x00, 0x00, // size
            0xEF, 0xCD, 0xAB, 0x89, // channel id = 0x89ABCDEF in little-endian
            0x02, 0x00, 0x00, 0x00, // subtitles_type = SCTE
            0x01, 0x23, 0x45, 0x67, // aux_id1 = 0x67452301 in little-endian
            0x89, 0xAB, 0xCD, 0xEF, // aux_id2 = 0xEFCDAB89 in little-endian
        };

        PacketSubtitleSelection packet;
        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getCounter() == 0x12345678);
        CPPUNIT_ASSERT(packet.getChannelId() == 0x89ABCDEF);
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_SCTE);
        CPPUNIT_ASSERT(packet.getAuxId1() == 0x67452301);
        CPPUNIT_ASSERT(packet.getAuxId2() == 0xEFCDAB89);
    }
    
    // Error handling and packet variations
    void testTruncatedPacketVariations()
    {
        // Test packet truncated at different points
        
        // Truncated after type
        std::uint8_t packetData1[] =
        {
            0x05, 0x00, 0x00, 0x00, // type only
        };
        PacketSubtitleSelection packet1;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(packetData1), std::end(packetData1));
        CPPUNIT_ASSERT(!packet1.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet1.isValid());
        
        // Truncated after counter
        std::uint8_t packetData2[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
        };
        PacketSubtitleSelection packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(packetData2), std::end(packetData2));
        CPPUNIT_ASSERT(!packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet2.isValid());
        
        // Truncated after size
        std::uint8_t packetData3[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
        };
        PacketSubtitleSelection packet3;
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(packetData3), std::end(packetData3));
        CPPUNIT_ASSERT(!packet3.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet3.isValid());
        
        // Truncated after channel id
        std::uint8_t packetData4[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
        };
        PacketSubtitleSelection packet4;
        DataBufferPtr buffer4 = std::make_unique<DataBuffer>(std::begin(packetData4), std::end(packetData4));
        CPPUNIT_ASSERT(!packet4.parse(std::move(buffer4)));
        CPPUNIT_ASSERT(!packet4.isValid());
        
        // Truncated after subtitle type
        std::uint8_t packetData5[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
        };
        PacketSubtitleSelection packet5;
        DataBufferPtr buffer5 = std::make_unique<DataBuffer>(std::begin(packetData5), std::end(packetData5));
        CPPUNIT_ASSERT(!packet5.parse(std::move(buffer5)));
        CPPUNIT_ASSERT(!packet5.isValid());
    }
    
    void testPacketReuse()
    {
        // Test reusing the same packet object for multiple parses
        PacketSubtitleSelection packet;
        
        // First parse - valid packet
        std::uint8_t packetData1[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type = TELETEXT
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };
        
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(packetData1), std::end(packetData1));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT);
        
        // Second parse - different valid packet
        std::uint8_t packetData2[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x11, 0x22, 0x33, 0x44, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x05, 0x06, 0x07, 0x08, // channel id
            0x02, 0x00, 0x00, 0x00, // subtitles_type = SCTE
            0x11, 0x22, 0x33, 0x44, // aux_id1
            0x55, 0x66, 0x77, 0x88, // aux_id2
        };
        
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(packetData2), std::end(packetData2));
        CPPUNIT_ASSERT(packet.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getSubtitlesType() == PacketSubtitleSelection::SUBTITLES_TYPE_SCTE);
        CPPUNIT_ASSERT(packet.getCounter() == 0x44332211);
        
        // Third parse - invalid packet
        std::uint8_t packetData3[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, // truncated
        };
        
        DataBufferPtr buffer3 = std::make_unique<DataBuffer>(std::begin(packetData3), std::end(packetData3));
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer3)));
        CPPUNIT_ASSERT(!packet.isValid());
    }
    
    void testCorruptedHeaderFields()
    {
        // Test with wrong packet type in header
        std::uint8_t packetDataWrongType[] =
        {
            0x99, 0x00, 0x00, 0x00, // wrong type (should be 0x05)
            0x01, 0x23, 0x45, 0x67, // counter
            0x10, 0x00, 0x00, 0x00, // size
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet1;
        DataBufferPtr buffer1 = std::make_unique<DataBuffer>(std::begin(packetDataWrongType), std::end(packetDataWrongType));
        CPPUNIT_ASSERT(!packet1.parse(std::move(buffer1)));
        CPPUNIT_ASSERT(!packet1.isValid());
        
        // Test with wrong size in header
        std::uint8_t packetDataWrongSize[] =
        {
            0x05, 0x00, 0x00, 0x00, // type
            0x01, 0x23, 0x45, 0x67, // counter
            0x20, 0x00, 0x00, 0x00, // wrong size (should be 0x10)
            0x01, 0x02, 0x03, 0x04, // channel id
            0x01, 0x00, 0x00, 0x00, // subtitles_type
            0xFF, 0xEE, 0xDD, 0xCC, // aux_id1
            0xEE, 0xDD, 0xCC, 0xBB, // aux_id2
        };

        PacketSubtitleSelection packet2;
        DataBufferPtr buffer2 = std::make_unique<DataBuffer>(std::begin(packetDataWrongSize), std::end(packetDataWrongSize));
        CPPUNIT_ASSERT(!packet2.parse(std::move(buffer2)));
        CPPUNIT_ASSERT(!packet2.isValid());
    }
    
    void testEmptyBuffer()
    {
        PacketSubtitleSelection packet;
        
        // Test with completely empty buffer - use default constructor like other tests
        DataBufferPtr buffer = std::make_unique<DataBuffer>();
        
        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketSubtitleSelectionTest );
