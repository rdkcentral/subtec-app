/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <cstring>
#include <limits>

#include "Ipv4SocketTarget.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class Ipv4SocketTargetTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(Ipv4SocketTargetTest);
    CPPUNIT_TEST(testConstructorWithValidIpPortFormat);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithSpecialCharacters);
    CPPUNIT_TEST(testConstructorWithNullCharacterInPath);
    CPPUNIT_TEST(testDestructorOnUnopenedSocket);
    CPPUNIT_TEST(testOpenWithMissingColonSeparator);
    CPPUNIT_TEST(testOpenWithEmptyStringPath);
    CPPUNIT_TEST(testOpenWithOnlyColon);
    CPPUNIT_TEST(testOpenWithColonAtStart);
    CPPUNIT_TEST(testOpenWithColonAtEnd);
    CPPUNIT_TEST(testOpenWithMultipleColons);
    CPPUNIT_TEST(testOpenWithPortOverflow);
    CPPUNIT_TEST(testOpenWithNegativePort);
    CPPUNIT_TEST(testOpenWithNonNumericPort);
    CPPUNIT_TEST(testOpenWithExtremelyLargePort);
    CPPUNIT_TEST(testOpenWithInvalidIpOctets);
    CPPUNIT_TEST(testOpenWithIncompleteIp);
    CPPUNIT_TEST(testOpenWithTooManyOctets);
    CPPUNIT_TEST(testOpenWithAlphabeticIp);
    CPPUNIT_TEST(testOpenWithWhitespaceInIp);
    CPPUNIT_TEST(testOpenMultipleTimesAfterFailure);
    CPPUNIT_TEST(testCloseWhenNotOpen);
    CPPUNIT_TEST(testWantsMorePacketsOnUnopenedSocket);
    CPPUNIT_TEST(testWantsMorePacketsMultipleCalls);
    CPPUNIT_TEST(testWritePacketWithZeroSize);
    CPPUNIT_TEST(testWritePacketWithSmallPacket);
    CPPUNIT_TEST(testWritePacketWithLargePacket);
    CPPUNIT_TEST(testWritePacketWhenSocketClosed);
    CPPUNIT_TEST(testWritePacketMultipleConsecutive);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Setup code if needed
    }

    void tearDown() override
    {
        // Cleanup code if needed
    }

protected:
    void testConstructorWithValidIpPortFormat()
    {
        Ipv4SocketTarget target("192.168.1.1:8080");
        // Constructor should succeed without throwing
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithEmptyPath()
    {
        Ipv4SocketTarget target("");
        // Constructor should accept empty path (validation happens in open())
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(10000, 'x');
        longPath += ":8080";
        Ipv4SocketTarget target(longPath);
        // Constructor should handle long paths without crashing
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithSpecialCharacters()
    {
        Ipv4SocketTarget target("!@#$%^&*():1234");
        // Constructor should accept any string
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithNullCharacterInPath()
    {
        std::string pathWithNull = "192.168.1.1";
        pathWithNull += '\0';
        pathWithNull += ":8080";
        Ipv4SocketTarget target(pathWithNull);
        // Constructor should handle null character in string
        CPPUNIT_ASSERT(true);
    }

    void testDestructorOnUnopenedSocket()
    {
        {
            Ipv4SocketTarget target("192.168.1.1:8080");
            // Destructor should handle unopened socket safely
        }
        // If we reach here, destructor succeeded
        CPPUNIT_ASSERT(true);
    }

    void testOpenWithMissingColonSeparator()
    {
        Ipv4SocketTarget target("192.168.1.1");
        bool result = target.open();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithEmptyStringPath()
    {
        Ipv4SocketTarget target("");
        bool result = target.open();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithOnlyColon()
    {
        Ipv4SocketTarget target(":");
        bool result = target.open();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithColonAtStart()
    {
        Ipv4SocketTarget target(":8080");
        bool result = target.open();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithColonAtEnd()
    {
        Ipv4SocketTarget target("192.168.1.1:");
        bool result = target.open();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithMultipleColons()
    {
        Ipv4SocketTarget target("192.168.1.1:8080:9090");
        bool result = target.open();
        // Will parse first part as IP, second as port, likely fail in port parsing
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithPortOverflow()
    {
        Ipv4SocketTarget target("127.0.0.1:65536");
        bool result = target.open();
        // Port overflow - scanf may succeed but value will wrap or fail
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithNegativePort()
    {
        Ipv4SocketTarget target("127.0.0.1:-1");
        bool result = target.open();
        // Negative port should fail parsing
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithNonNumericPort()
    {
        Ipv4SocketTarget target("127.0.0.1:abc");
        bool result = target.open();
        // Non-numeric port should fail sscanf
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithExtremelyLargePort()
    {
        Ipv4SocketTarget target("127.0.0.1:999999999");
        bool result = target.open();
        // Extremely large port will overflow unsigned short
        CPPUNIT_ASSERT_EQUAL(false, result);
    }


    void testOpenWithInvalidIpOctets()
    {
        Ipv4SocketTarget target("999.999.999.999:8080");
        bool result = target.open();
        // Invalid IP octets should fail inet_pton
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithIncompleteIp()
    {
        Ipv4SocketTarget target("192.168.1:8080");
        bool result = target.open();
        // Incomplete IP should fail inet_pton
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithTooManyOctets()
    {
        Ipv4SocketTarget target("192.168.1.1.1:8080");
        bool result = target.open();
        // Too many octets should fail inet_pton
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithAlphabeticIp()
    {
        Ipv4SocketTarget target("abc.def.ghi.jkl:8080");
        bool result = target.open();
        // Alphabetic IP should fail inet_pton
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenWithWhitespaceInIp()
    {
        Ipv4SocketTarget target(" 192.168.1.1:8080");
        bool result = target.open();
        // Whitespace in IP should fail inet_pton
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testOpenMultipleTimesAfterFailure()
    {
        Ipv4SocketTarget target("invalid");
        bool result1 = target.open();
        bool result2 = target.open();
        bool result3 = target.open();
        // All attempts should fail with invalid path
        CPPUNIT_ASSERT_EQUAL(false, result1);
        CPPUNIT_ASSERT_EQUAL(false, result2);
        CPPUNIT_ASSERT_EQUAL(false, result3);
    }

    void testCloseWhenNotOpen()
    {
        Ipv4SocketTarget target("192.168.1.1:8080");
        target.close();
        // Close on unopened socket should be safe
        CPPUNIT_ASSERT(true);
    }

    void testWantsMorePacketsOnUnopenedSocket()
    {
        Ipv4SocketTarget target("192.168.1.1:8080");
        bool result = target.wantsMorePackets();
        // Should always return true regardless of socket state
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testWantsMorePacketsMultipleCalls()
    {
        Ipv4SocketTarget target("192.168.1.1:8080");
        bool result1 = target.wantsMorePackets();
        bool result2 = target.wantsMorePackets();
        bool result3 = target.wantsMorePackets();
        // All calls should return true
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(true, result3);
    }

    void testWritePacketWithZeroSize()
    {
        Ipv4SocketTarget target("127.0.0.1:9999");
        DataPacket packet(100);
        packet.setSize(0); // Zero size packet

        // Writing zero-size packet on unopened socket should fail
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketWithSmallPacket()
    {
        Ipv4SocketTarget target("127.0.0.1:9999");
        DataPacket packet(1);
        char* buffer = packet.getBuffer();
        buffer[0] = 0x42;
        packet.setSize(1);

        // Writing on unopened socket should fail
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketWithLargePacket()
    {
        Ipv4SocketTarget target("127.0.0.1:9999");
        DataPacket packet(65536);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 65536);
        packet.setSize(65536);

        // Writing large packet on unopened socket should fail
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketWhenSocketClosed()
    {
        Ipv4SocketTarget target("127.0.0.1:9999");
        (void)target.open();
        target.close();

        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 10);
        packet.setSize(10);
        bool result = target.writePacket(packet);
        // Writing to closed socket should fail
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketMultipleConsecutive()
    {
        Ipv4SocketTarget target("127.0.0.1:9999");

        DataPacket packet1(10);
        char* buffer1 = packet1.getBuffer();
        std::memset(buffer1, 0x11, 10);
        packet1.setSize(10);

        DataPacket packet2(20);
        char* buffer2 = packet2.getBuffer();
        std::memset(buffer2, 0x22, 20);
        packet2.setSize(20);

        DataPacket packet3(30);
        char* buffer3 = packet3.getBuffer();
        std::memset(buffer3, 0x33, 30);
        packet3.setSize(30);

        // All writes should fail on unopened socket
        bool result1 = target.writePacket(packet1);
        bool result2 = target.writePacket(packet2);
        bool result3 = target.writePacket(packet3);

        CPPUNIT_ASSERT_EQUAL(false, result1);
        CPPUNIT_ASSERT_EQUAL(false, result2);
        CPPUNIT_ASSERT_EQUAL(false, result3);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ipv4SocketTargetTest);
