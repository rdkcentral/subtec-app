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

#include "Packet.hpp"
#include "PacketData.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketData;

class PacketDataTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketDataTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
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
        char packetData[] =
        {
                0x01, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0B, 0x00, 0x00, 0x00, // size
                0x05, 0x00, 0x00, 0x00, // channel id
                0x01, 0x00, 0x00, 0x00, // channel type
                0x01, 0x02, 0x03        // PES data
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::PES_DATA);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 8 + 3);
        CPPUNIT_ASSERT(packet.getChannelId() == 5);
        CPPUNIT_ASSERT(packet.getChannelType() == 1);
        CPPUNIT_ASSERT(packet.getDataSize() == 3);
        CPPUNIT_ASSERT(packet.getData()[0] == 0x01);
        CPPUNIT_ASSERT(packet.getData()[1] == 0x02);
        CPPUNIT_ASSERT(packet.getData()[2] == 0x03);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF1, 0x00, 0x00, 0x00, // type (invalid)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0B, 0x00, 0x00, 0x00, // size
                0x05, 0x00, 0x00, 0x00, // channel id
                0x01, 0x00, 0x00, 0x00, // channel type
                0x01, 0x02, 0x03        // PES data
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x01, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0B, 0x00, 0x00, 0x00, // size
                0x05, 0x00, 0x00, 0x00, // channel id
                0x01, 0x00, 0x00,       // (not enough bytes)
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x01, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0B, 0x00, 0x00, 0x00, // size
                0x05, 0x00, 0x00, 0x00, // channel id
                0x01, 0x00, 0x00, 0x00, // channel type
                0x01, 0x02, 0x03,       // PES data
                0x00, 0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketData packet{Packet::Type::PES_DATA};

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketDataTest );
