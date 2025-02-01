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
                0x0F, 0x00, 0x00, 0x00, // type
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
                0x0F, 0x00, 0x00, 0x00, // type
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
                0x0F, 0x00, 0x00, 0x00, // type
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
                0x0F, 0x00, 0x00, 0x00, // type
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
                0x0F, 0x00, 0x00, 0x00, // type
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTtmlInfoTest );
