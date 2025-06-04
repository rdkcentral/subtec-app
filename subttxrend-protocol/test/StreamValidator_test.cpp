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

#include "StreamValidator.hpp"
#include "PacketResetAll.hpp"
#include "PacketTimestamp.hpp"

using subttxrend::common::DataBuffer;
using subttxrend::common::DataBufferPtr;
using subttxrend::protocol::Packet;
using subttxrend::protocol::PacketTimestamp;
using subttxrend::protocol::PacketResetAll;
using subttxrend::protocol::StreamValidator;

class PacketStreamValidatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PacketStreamValidatorTest );
    CPPUNIT_TEST(testStream);
CPPUNIT_TEST_SUITE_END();

private:
    Packet& generatePacketTimestamp(std::uint32_t counter)
    {
        static PacketTimestamp packetTimestamp;
        static std::uint8_t packetBytes[12 + 12];

        std::size_t index = 0;

        // type
        packetBytes[index++] = 2;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // counter
        packetBytes[index++] = (counter >> 0) & 0xFF;
        packetBytes[index++] = (counter >> 8) & 0xFF;
        packetBytes[index++] = (counter >> 16) & 0xFF;
        packetBytes[index++] = (counter >> 24) & 0xFF;

        // size
        packetBytes[index++] = 12;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // timestamp
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // stc
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetBytes), std::end(packetBytes));

        CPPUNIT_ASSERT(packetTimestamp.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packetTimestamp.isValid());

        return packetTimestamp;
    }

    Packet& generatePacketResetAll()
    {
        static PacketResetAll packetResetAll;
        static std::uint8_t packetBytes[12];

        std::size_t index = 0;

        // type
        packetBytes[index++] = 3;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // counter
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // size
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(std::begin(packetBytes), std::end(packetBytes));

        CPPUNIT_ASSERT(packetResetAll.parse(std::move(buffer)));
        CPPUNIT_ASSERT(packetResetAll.isValid());

        return packetResetAll;
    }

    Packet& generatePacketInvalid()
    {
        static PacketResetAll packetInvalid;

        return packetInvalid;
    }

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testStream()
    {
        StreamValidator validator;

        // initially it should be valid
        CPPUNIT_ASSERT(validator.isValid());

        // reset all
        CPPUNIT_ASSERT(validator.validate(generatePacketResetAll()));

        // some valid packets
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(3)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(4)));

        // invalid counter - still will be considered valid
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(10)));

        // try to get back to valid - shall not switch stream validity
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(5)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(11)));

        // reset all
        CPPUNIT_ASSERT(validator.validate(generatePacketResetAll()));

        // reset all (repeated)
        CPPUNIT_ASSERT(validator.validate(generatePacketResetAll()));

        // invalid
        CPPUNIT_ASSERT(!validator.validate(generatePacketInvalid()));

        // some valid packets (shall not be accepted)
        CPPUNIT_ASSERT(!validator.validate(generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(!validator.validate(generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(!validator.validate(generatePacketTimestamp(3)));
        CPPUNIT_ASSERT(!validator.validate(generatePacketTimestamp(4)));

        // reset all
        CPPUNIT_ASSERT(validator.validate(generatePacketResetAll()));

        // some valid packets
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(validator.validate(generatePacketTimestamp(3)));

        // invalid
        CPPUNIT_ASSERT(!validator.validate(generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.validate(generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.validate(generatePacketInvalid()));

        // status shall be invalid
        CPPUNIT_ASSERT(!validator.isValid());

        // reset all (repeated)
        CPPUNIT_ASSERT(validator.validate(generatePacketResetAll()));

        // status shall be valid
        CPPUNIT_ASSERT(validator.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketStreamValidatorTest );
