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
#include "PacketResetAll.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketResetAll;

class PacketResetAllTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketResetAllTest );
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testBadType);
    CPPUNIT_TEST(testBadCounter);
    CPPUNIT_TEST(testBadSize);
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
                0x03, 0x00, 0x00, 0x00, // type
                0x00, 0x00, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::RESET_ALL);
        CPPUNIT_ASSERT(packet.getCounter() == 0);
        CPPUNIT_ASSERT(packet.getSize() == 0);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF3, 0x00, 0x00, 0x00, // type (invalid)
                0x00, 0x00, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadCounter()
    {
        std::uint8_t packetData[] =
        {
                0x03, 0x00, 0x00, 0x00, // type
                0x00, 0x01, 0x00, 0x00, // counter
                0x00, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        // for reset all counter is not taken into account, packet is valid
        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x03, 0x00, 0x00, 0x00, // type (invalid)
                0x00, 0x00, 0x00, 0x00, // counter
                0x10, 0x00, 0x00, 0x00, // size
        };

        PacketResetAll packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketResetAllTest );
