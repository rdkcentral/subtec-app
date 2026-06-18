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
#include <memory>
#include <array>

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
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testIsValidBasic);
    CPPUNIT_TEST(testIsValidAfterInvalidation);
    CPPUNIT_TEST(testIsValidAfterReset);
    CPPUNIT_TEST(testValidateBasicValid);
    CPPUNIT_TEST(testValidateBasicInvalid);
    CPPUNIT_TEST(testValidateReturnValue);
    CPPUNIT_TEST(testValidateCounterStubAcceptsAnyValue);
    CPPUNIT_TEST(testResetAllBasic);
    CPPUNIT_TEST(testResetAllMultiple);
    CPPUNIT_TEST(testResetAllAfterInvalid);
    CPPUNIT_TEST(testTimestampBasic);
    CPPUNIT_TEST(testTimestampSequential);
    CPPUNIT_TEST(testTimestampNonSequentialAcceptedWithCounterStub);
    CPPUNIT_TEST(testTimestampBoundaryValues);
    CPPUNIT_TEST(testCounterBoundaryValues);
    CPPUNIT_TEST(testCounterWrapAround);
    CPPUNIT_TEST(testInvalidPacketTypes);
    CPPUNIT_TEST(testInvalidPacketData);
    CPPUNIT_TEST(testStateTransitions);
    CPPUNIT_TEST(testErrorRecovery);
    CPPUNIT_TEST(testMultipleErrors);
    CPPUNIT_TEST(testPacketReuse);
CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<Packet> generatePacketTimestamp(std::uint32_t counter)
    {
        auto packet = std::make_unique<PacketTimestamp>();
        std::array<std::uint8_t, 24> packetBytes{};

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

        // timestamp (8 bytes)
        for (int i = 0; i < 8; ++i) packetBytes[index++] = 0;

        // stc (4 bytes)
        for (int i = 0; i < 4; ++i) packetBytes[index++] = 0;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetBytes.begin(), packetBytes.end());

        CPPUNIT_ASSERT(packet->parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet->isValid());

        return packet;
    }

    std::unique_ptr<Packet> generatePacketResetAll()
    {
        auto packet = std::make_unique<PacketResetAll>();
        std::array<std::uint8_t, 12> packetBytes{};

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

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetBytes.begin(), packetBytes.end());

        CPPUNIT_ASSERT(packet->parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet->isValid());

        return packet;
    }

    std::unique_ptr<Packet> generatePacketTimestampWithValues(std::uint32_t counter, std::uint64_t timestamp, std::uint32_t stc)
    {
        auto packet = std::make_unique<PacketTimestamp>();
        std::array<std::uint8_t, 24> packetBytes{};

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

        // timestamp (8 bytes)
        packetBytes[index++] = (timestamp >> 0) & 0xFF;
        packetBytes[index++] = (timestamp >> 8) & 0xFF;
        packetBytes[index++] = (timestamp >> 16) & 0xFF;
        packetBytes[index++] = (timestamp >> 24) & 0xFF;
        packetBytes[index++] = (timestamp >> 32) & 0xFF;
        packetBytes[index++] = (timestamp >> 40) & 0xFF;
        packetBytes[index++] = (timestamp >> 48) & 0xFF;
        packetBytes[index++] = (timestamp >> 56) & 0xFF;

        // stc (4 bytes)
        packetBytes[index++] = (stc >> 0) & 0xFF;
        packetBytes[index++] = (stc >> 8) & 0xFF;
        packetBytes[index++] = (stc >> 16) & 0xFF;
        packetBytes[index++] = (stc >> 24) & 0xFF;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetBytes.begin(), packetBytes.end());

        CPPUNIT_ASSERT(packet->parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet->isValid());

        return packet;
    }

    std::unique_ptr<Packet> generatePacketResetAllWithCounter(std::uint32_t counter)
    {
        auto packet = std::make_unique<PacketResetAll>();
        std::array<std::uint8_t, 12> packetBytes{};

        std::size_t index = 0;

        // type
        packetBytes[index++] = 3;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        // counter
        packetBytes[index++] = (counter >> 0) & 0xFF;
        packetBytes[index++] = (counter >> 8) & 0xFF;
        packetBytes[index++] = (counter >> 16) & 0xFF;
        packetBytes[index++] = (counter >> 24) & 0xFF;

        // size
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;
        packetBytes[index++] = 0;

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetBytes.begin(), packetBytes.end());

        CPPUNIT_ASSERT(packet->parse(std::move(buffer)));
        CPPUNIT_ASSERT(packet->isValid());

        return packet;
    }

    std::unique_ptr<Packet> generatePacketInvalid()
    {
        auto packet = std::make_unique<PacketResetAll>();
        std::array<std::uint8_t, 12> packetBytes{};

        std::size_t index = 0;

        // Invalid packet type
        packetBytes[index++] = 0xFF;
        packetBytes[index++] = 0xFF;
        packetBytes[index++] = 0xFF;
        packetBytes[index++] = 0xFF;

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

        DataBufferPtr buffer = std::make_unique<DataBuffer>(packetBytes.begin(), packetBytes.end());

        // This should fail to parse due to invalid type, making it invalid
        packet->parse(std::move(buffer));
        // Don't assert validity since we want an invalid packet

        return packet;
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
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));

        // some valid packets
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(3)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(4)));

        // invalid counter - still will be considered valid
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(10)));

        // try to get back to valid - shall not switch stream validity
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(5)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(11)));

        // reset all
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));

        // reset all (repeated)
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));

        // invalid
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));

        // some valid packets (shall not be accepted)
        CPPUNIT_ASSERT(!validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(!validator.validate(*generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(!validator.validate(*generatePacketTimestamp(3)));
        CPPUNIT_ASSERT(!validator.validate(*generatePacketTimestamp(4)));

        // reset all
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));

        // some valid packets
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(3)));

        // invalid
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));

        // status shall be invalid
        CPPUNIT_ASSERT(!validator.isValid());

        // reset all (repeated)
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));

        // status shall be valid
        CPPUNIT_ASSERT(validator.isValid());
    }

    // Basic API coverage tests
    void testConstructorInitialState()
    {
        StreamValidator validator;
        
        // Test initial state after construction
        CPPUNIT_ASSERT(validator.isValid()); // Should start in valid state
    }

    void testIsValidBasic()
    {
        StreamValidator validator;
        
        // Initially valid
        CPPUNIT_ASSERT(validator.isValid());
        
        // Still valid after valid packet
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
        
        // Still valid after another valid packet
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testIsValidAfterInvalidation()
    {
        StreamValidator validator;
        
        // Start valid
        CPPUNIT_ASSERT(validator.isValid());
        
        // Invalidate with bad packet
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Should remain invalid
        CPPUNIT_ASSERT(!validator.isValid());
    }

    void testIsValidAfterReset()
    {
        StreamValidator validator;
        
        // Invalidate first
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Reset with RESET_ALL
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testValidateBasicValid()
    {
        StreamValidator validator;
        
        // Test with RESET_ALL packet
        bool result = validator.validate(*generatePacketResetAll());
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(validator.isValid());
        
        // Test with TIMESTAMP packet
        result = validator.validate(*generatePacketTimestamp(1));
        CPPUNIT_ASSERT(result);
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testValidateBasicInvalid()
    {
        StreamValidator validator;
        
        // Test with invalid packet
        bool result = validator.validate(*generatePacketInvalid());
        CPPUNIT_ASSERT(!result);
        CPPUNIT_ASSERT(!validator.isValid());
    }

    void testValidateReturnValue()
    {
        StreamValidator validator;
        
        // Return value should match isValid() state
        bool result = validator.validate(*generatePacketResetAll());
        CPPUNIT_ASSERT(result == validator.isValid());
        
        result = validator.validate(*generatePacketTimestamp(1));
        CPPUNIT_ASSERT(result == validator.isValid());
        
        result = validator.validate(*generatePacketInvalid());
        CPPUNIT_ASSERT(result == validator.isValid());
    }

    void testValidateCounterStubAcceptsAnyValue()
    {
        StreamValidator validator;
        
        // validateCounter() is currently stubbed to accept any value.
        CPPUNIT_ASSERT(validator.validateCounter(0));
        CPPUNIT_ASSERT(validator.validateCounter(1));
        CPPUNIT_ASSERT(validator.validateCounter(100));
        CPPUNIT_ASSERT(validator.validateCounter(0xFFFFFFFF));
    }

    void testResetAllBasic()
    {
        StreamValidator validator;
        
        // Test basic RESET_ALL handling
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
        
        // Validator should remain valid
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testResetAllMultiple()
    {
        StreamValidator validator;
        
        // Multiple RESET_ALL packets should all be accepted
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testResetAllAfterInvalid()
    {
        StreamValidator validator;
        
        // Invalidate stream
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // RESET_ALL should restore validity
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
    }

    // Edge cases and error handling tests
    void testTimestampBasic()
    {
        StreamValidator validator;
        
        // Initialize with RESET_ALL
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Test basic timestamp packet
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testTimestampSequential()
    {
        StreamValidator validator;
        
        // Initialize
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Test sequential timestamps
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(2)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(3)));
        
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testTimestampNonSequentialAcceptedWithCounterStub()
    {
        StreamValidator validator;
        
        // Initialize
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Non-sequential timestamp counters are currently accepted.
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(5)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(10)));
        
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testTimestampBoundaryValues()
    {
        StreamValidator validator;
        
        // Initialize
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Test boundary values
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(0)));
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(0xFFFFFFFF)));
        
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testCounterBoundaryValues()
    {
        StreamValidator validator;
        
        // Test boundary values for counter
        CPPUNIT_ASSERT(validator.validateCounter(0));
        CPPUNIT_ASSERT(validator.validateCounter(0xFFFFFFFF));
        
        // Test mid-range values
        CPPUNIT_ASSERT(validator.validateCounter(0x80000000));
        CPPUNIT_ASSERT(validator.validateCounter(0x7FFFFFFF));
    }

    void testCounterWrapAround()
    {
        StreamValidator validator;
        
        // Test counter wrap around scenario
        CPPUNIT_ASSERT(validator.validateCounter(0xFFFFFFFE));
        CPPUNIT_ASSERT(validator.validateCounter(0xFFFFFFFF));
        CPPUNIT_ASSERT(validator.validateCounter(0)); // Wrap to 0
        CPPUNIT_ASSERT(validator.validateCounter(1));
    }

    void testInvalidPacketTypes()
    {
        StreamValidator validator;
        
        // Test various invalid packet scenarios
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Reset for next test
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Test another invalid packet
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
    }

    void testInvalidPacketData()
    {
        StreamValidator validator;
        
        // Test corrupted packet structure using our invalid packet generator
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Reset
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        
        // Test another invalid packet scenario
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
    }

    void testStateTransitions()
    {
        StreamValidator validator;
        
        // Valid -> Invalid -> Valid transition
        CPPUNIT_ASSERT(validator.isValid()); // Start valid
        
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid()); // Now invalid
        
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid()); // Back to valid
        
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        CPPUNIT_ASSERT(validator.isValid()); // Still valid
    }

    void testErrorRecovery()
    {
        StreamValidator validator;
        
        // Sequence: Valid -> Error -> Reset -> Valid
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(1)));
        
        // Introduce error
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Recover with RESET_ALL
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
        
        // Continue normal operation
        CPPUNIT_ASSERT(validator.validate(*generatePacketTimestamp(100)));
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testMultipleErrors()
    {
        StreamValidator validator;
        
        // Multiple consecutive errors
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        CPPUNIT_ASSERT(!validator.validate(*generatePacketInvalid()));
        CPPUNIT_ASSERT(!validator.isValid());
        
        // Recovery should still work
        CPPUNIT_ASSERT(validator.validate(*generatePacketResetAll()));
        CPPUNIT_ASSERT(validator.isValid());
    }

    void testPacketReuse()
    {
        StreamValidator validator;
        
        // Reuse same packet multiple times
        auto packet = generatePacketTimestamp(42);

        CPPUNIT_ASSERT(validator.validate(*packet));
        CPPUNIT_ASSERT(validator.validate(*packet));
        CPPUNIT_ASSERT(validator.validate(*packet));
        
        CPPUNIT_ASSERT(validator.isValid());
        
        // Reuse RESET_ALL packet
        auto resetPacket = generatePacketResetAll();
        CPPUNIT_ASSERT(validator.validate(*resetPacket));
        CPPUNIT_ASSERT(validator.validate(*resetPacket));
        CPPUNIT_ASSERT(validator.validate(*resetPacket));
        
        CPPUNIT_ASSERT(validator.isValid());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( PacketStreamValidatorTest );
