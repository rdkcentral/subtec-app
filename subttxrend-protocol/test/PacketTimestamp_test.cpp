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

#include "PacketTimestamp.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketTimestamp;

class PacketTimestampTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketTimestampTest );
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
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet.isValid());
        CPPUNIT_ASSERT(packet.getType() == Packet::Type::TIMESTAMP);
        CPPUNIT_ASSERT(packet.getCounter() == 0x67452301);
        CPPUNIT_ASSERT(packet.getSize() == 12);
        CPPUNIT_ASSERT(packet.getTimestamp() == 0x0123456789ABCDEF);
        CPPUNIT_ASSERT(packet.getStc() == 0x44332211);
    }

    void testBadType()
    {
        std::uint8_t packetData[] =
        {
                0xF2, 0x00, 0x00, 0x00, // type (invalid)
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testBadSize()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x01, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooShort()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, // stc (cut)
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }

    void testTooLong()
    {
        std::uint8_t packetData[] =
        {
                0x02, 0x00, 0x00, 0x00, // type
                0x01, 0x23, 0x45, 0x67, // counter
                0x0C, 0x00, 0x00, 0x00, // size
                0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, // timestamp
                0x11, 0x22, 0x33, 0x44, // stc
                0x00, 0x00, 0xFF, 0xFF  // (extra bytes)
        };

        PacketTimestamp packet;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetData), std::end(packetData));

        CPPUNIT_ASSERT(!packet.parse(std::move(buffer)));
        CPPUNIT_ASSERT(!packet.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketTimestampTest );
