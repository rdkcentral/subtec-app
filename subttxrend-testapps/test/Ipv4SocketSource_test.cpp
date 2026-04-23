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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <cstdint>

#include "Ipv4SocketSource.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class Ipv4SocketSourceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(Ipv4SocketSourceTest);
    CPPUNIT_TEST(testConstructorWithValidPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithLongPath);
    CPPUNIT_TEST(testOpenWithMissingPort);
    CPPUNIT_TEST(testOpenWithMissingIP);
    CPPUNIT_TEST(testOpenWithNoColon);
    CPPUNIT_TEST(testOpenWithMultipleColons);
    CPPUNIT_TEST(testOpenWithNonNumericPort);
    CPPUNIT_TEST(testOpenWithPortZero);
    CPPUNIT_TEST(testOpenWithInvalidIPAddress);
    CPPUNIT_TEST(testOpenWithHostnameNotIP);
    CPPUNIT_TEST(testOpenWithValidIPAndPort);
    CPPUNIT_TEST(testOpenWithWildcardAddress);
    CPPUNIT_TEST(testOpenMultipleTimesReturnsTrueWhenAlreadyOpen);
    CPPUNIT_TEST(testOpenPortAlreadyInUse);
    CPPUNIT_TEST(testCloseWithoutOpen);
    CPPUNIT_TEST(testCloseAfterSuccessfulOpen);
    CPPUNIT_TEST(testCloseMultipleTimes);
    CPPUNIT_TEST(testCloseAfterFailedOpen);
    CPPUNIT_TEST(testReadPacketWithBufferSmallerThanHeader);
    CPPUNIT_TEST(testReadPacketWithExactHeaderSizeBuffer);
    CPPUNIT_TEST(testReadPacketReceivesValidPacket);
    CPPUNIT_TEST(testReadPacketWithZeroPayloadSize);
    CPPUNIT_TEST(testReadPacketWithLargePayload);
    CPPUNIT_TEST(testReadPacketBufferTooSmallForPayload);
    CPPUNIT_TEST(testReadPacketMultiplePacketsSequentially);
    CPPUNIT_TEST(testReadPacketLittleEndianSizeParsing);
    CPPUNIT_TEST(testReadPacketClientDisconnectReturnsZeroSize);
    CPPUNIT_TEST(testFullWorkflowConstructOpenReadClose);
    CPPUNIT_TEST(testReopenAfterClose);
    CPPUNIT_TEST(testMultiplePacketsWithDifferentSizes);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_testPort = 0;
    }

    void tearDown() override
    {
        // Clean up any test resources
    }

protected:
    // Helper function to get an available port
    unsigned short getAvailablePort()
    {
        static unsigned short basePort = 9000;
        return basePort++;
    }

    // Helper to create a client socket and connect
    int createClientSocket(const std::string& ip, unsigned short port)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            return -1;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)
        {
            ::close(sockfd);
            return -1;
        }

        // Give server time to listen
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
        {
            ::close(sockfd);
            return -1;
        }

        return sockfd;
    }

    // Helper to send packet data
    bool sendPacketData(int sockfd, const std::vector<std::uint8_t>& data)
    {
        size_t totalSent = 0;
        while (totalSent < data.size())
        {
            ssize_t sent = send(sockfd, &data[totalSent], data.size() - totalSent, 0);
            if (sent <= 0)
            {
                return false;
            }
            totalSent += sent;
        }
        return true;
    }

    // Helper to create a packet with header and payload
    std::vector<std::uint8_t> makePacket(const std::vector<std::uint8_t>& payload)
    {
        std::vector<std::uint8_t> packet(12 + payload.size());

        // First 8 bytes can be anything (type, counter)
        packet[0] = 0x01;
        packet[1] = 0x02;
        packet[2] = 0x03;
        packet[3] = 0x04;
        packet[4] = 0x05;
        packet[5] = 0x06;
        packet[6] = 0x07;
        packet[7] = 0x08;

        // Bytes 8-11: payload size in little-endian
        std::uint32_t payloadSize = static_cast<std::uint32_t>(payload.size());
        packet[8] = static_cast<std::uint8_t>(payloadSize & 0xFF);
        packet[9] = static_cast<std::uint8_t>((payloadSize >> 8) & 0xFF);
        packet[10] = static_cast<std::uint8_t>((payloadSize >> 16) & 0xFF);
        packet[11] = static_cast<std::uint8_t>((payloadSize >> 24) & 0xFF);

        // Copy payload
        std::copy(payload.begin(), payload.end(), packet.begin() + 12);

        return packet;
    }

    void testConstructorWithValidPath()
    {
        Ipv4SocketSource source("127.0.0.1:8080");
        // Constructor should succeed - no exception
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithEmptyPath()
    {
        Ipv4SocketSource source("");
        // Constructor accepts empty path, validation happens at open()
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithLongPath()
    {
        std::string longPath(5000, 'x');
        longPath += ":8080";
        Ipv4SocketSource source(longPath);
        // Constructor accepts long path
        CPPUNIT_ASSERT(true);
    }

    // open() tests - path parsing
    void testOpenWithMissingPort()
    {
        Ipv4SocketSource source("127.0.0.1");
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithMissingIP()
    {
        Ipv4SocketSource source(":8080");
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithNoColon()
    {
        Ipv4SocketSource source("127.0.0.18080");
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithMultipleColons()
    {
        // This will take first colon, so IP="127.0.0.1", port="8080:9090"
        // sscanf will parse "8080" from "8080:9090" and succeed
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort) + ":extra";
        Ipv4SocketSource source(path);
        // This should succeed because sscanf will parse the port correctly
        bool result = source.open();
        source.close();
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testOpenWithNonNumericPort()
    {
        Ipv4SocketSource source("127.0.0.1:abc");
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithPortZero()
    {
        Ipv4SocketSource source("127.0.0.1:0");
        // Port 0 is valid - OS assigns ephemeral port
        bool result = source.open();
        source.close();
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testOpenWithInvalidIPAddress()
    {
        Ipv4SocketSource source("999.999.999.999:8080");
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithHostnameNotIP()
    {
        Ipv4SocketSource source("localhost:8080");
        // inet_pton expects numeric IP, not hostname
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithValidIPAndPort()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testOpenWithWildcardAddress()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "0.0.0.0:" + std::to_string(testPort);
        Ipv4SocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testOpenMultipleTimesReturnsTrueWhenAlreadyOpen()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        source.close();
    }

    void testOpenPortAlreadyInUse()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);

        Ipv4SocketSource source1(path);
        CPPUNIT_ASSERT_EQUAL(true, source1.open());

        Ipv4SocketSource source2(path);
        CPPUNIT_ASSERT_EQUAL(false, source2.open());

        source1.close();
    }

    void testCloseWithoutOpen()
    {
        Ipv4SocketSource source("127.0.0.1:8080");
        // Should not crash
        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testCloseAfterSuccessfulOpen()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testCloseMultipleTimes()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        source.open();
        source.close();
        source.close();
        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testCloseAfterFailedOpen()
    {
        Ipv4SocketSource source("invalid:path");
        source.open(); // fails
        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testReadPacketWithBufferSmallerThanHeader()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(10); // Less than 12 bytes

        // Start client connection in thread
        std::thread clientThread([testPort]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0)
            {
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(testPort);
                inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
                connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        clientThread.join();
        source.close();
    }

    void testReadPacketWithExactHeaderSizeBuffer()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(12); // Exact header size

        // Start client that sends packet with payload
        std::thread clientThread([testPort, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                std::vector<std::uint8_t> payload = {0x01, 0x02};
                auto packetData = makePacket(payload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        // Should fail because buffer can't hold payload
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        clientThread.join();
        source.close();
    }

    void testReadPacketReceivesValidPacket()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::vector<std::uint8_t> expectedPayload = {0xAA, 0xBB, 0xCC, 0xDD};

        std::thread clientThread([testPort, expectedPayload, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                auto packetData = makePacket(expectedPayload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), packet.getSize()); // 12 header + 4 payload

        const std::uint8_t* buffer = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xAA), buffer[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xBB), buffer[13]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xCC), buffer[14]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xDD), buffer[15]);

        clientThread.join();
        source.close();
    }

    void testReadPacketWithZeroPayloadSize()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::thread clientThread([testPort, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                std::vector<std::uint8_t> emptyPayload;
                auto packetData = makePacket(emptyPayload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize()); // Just header

        clientThread.join();
        source.close();
    }

    void testReadPacketWithLargePayload()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(10240);

        std::vector<std::uint8_t> largePayload(5000, 0x42);

        std::thread clientThread([testPort, largePayload, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                auto packetData = makePacket(largePayload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5012), packet.getSize()); // 12 + 5000

        clientThread.join();
        source.close();
    }

    void testReadPacketBufferTooSmallForPayload()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(20); // Can only hold 20 bytes total

        std::vector<std::uint8_t> largePayload(100, 0xFF);

        std::thread clientThread([testPort, largePayload, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                auto packetData = makePacket(largePayload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        clientThread.join();
        source.close();
    }

    void testReadPacketMultiplePacketsSequentially()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::thread clientThread([testPort, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                // Send three packets
                std::vector<std::uint8_t> payload1 = {0x01};
                auto packet1 = makePacket(payload1);
                sendPacketData(sock, packet1);

                std::vector<std::uint8_t> payload2 = {0x02, 0x03};
                auto packet2 = makePacket(payload2);
                sendPacketData(sock, packet2);

                std::vector<std::uint8_t> payload3 = {0x04, 0x05, 0x06};
                auto packet3 = makePacket(payload3);
                sendPacketData(sock, packet3);

                ::close(sock);
            }
        });

        // Read first packet
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(13), packet.getSize());

        // Read second packet
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getSize());

        // Read third packet
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(15), packet.getSize());

        clientThread.join();
        source.close();
    }

    void testReadPacketLittleEndianSizeParsing()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::thread clientThread([testPort]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0)
            {
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(testPort);
                inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

                if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0)
                {
                    // Send packet with size 0x04030201 in little-endian (bytes 8-11)
                    std::uint8_t data[12 + 4];
                    memset(data, 0, sizeof(data));
                    data[8] = 0x04;  // LSB
                    data[9] = 0x00;
                    data[10] = 0x00;
                    data[11] = 0x00; // MSB
                    // Payload is 4 bytes
                    data[12] = 0xAA;
                    data[13] = 0xBB;
                    data[14] = 0xCC;
                    data[15] = 0xDD;

                    send(sock, data, sizeof(data), 0);
                }
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), packet.getSize()); // 12 + 4

        clientThread.join();
        source.close();
    }

    void testReadPacketClientDisconnectReturnsZeroSize()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::thread clientThread([testPort]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0)
            {
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(testPort);
                inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

                if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0)
                {
                    // Connect but send nothing, then close
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                ::close(sock);
            }
        });

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize()); // EOF

        clientThread.join();
        source.close();
    }

    void testFullWorkflowConstructOpenReadClose()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        // Open
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);

        std::vector<std::uint8_t> payload = {0x11, 0x22, 0x33};

        std::thread clientThread([testPort, payload, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                auto packetData = makePacket(payload);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });

        // Read
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(15), packet.getSize());

        clientThread.join();

        // Close
        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testReopenAfterClose()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        // First open
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();

        // Reopen same source
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();

        CPPUNIT_ASSERT(true);
    }

    void testMultiplePacketsWithDifferentSizes()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(2048);

        std::thread clientThread([testPort, this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            int sock = createClientSocket("127.0.0.1", testPort);
            if (sock >= 0)
            {
                // Packet 1: no payload
                std::vector<std::uint8_t> payload1;
                auto packet1 = makePacket(payload1);
                sendPacketData(sock, packet1);

                // Packet 2: small payload
                std::vector<std::uint8_t> payload2(10, 0xAA);
                auto packet2 = makePacket(payload2);
                sendPacketData(sock, packet2);

                // Packet 3: large payload
                std::vector<std::uint8_t> payload3(500, 0xBB);
                auto packet3 = makePacket(payload3);
                sendPacketData(sock, packet3);

                ::close(sock);
            }
        });

        // Read all three packets
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(22), packet.getSize());

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(512), packet.getSize());

        clientThread.join();
        source.close();
    }

private:
    unsigned short m_testPort;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ipv4SocketSourceTest);
