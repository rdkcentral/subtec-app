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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketSubtitleSelectionTest );
