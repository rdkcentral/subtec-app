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

namespace
{

class ThreadJoiner
{
public:
    explicit ThreadJoiner(std::thread& thread) :
            m_thread(thread)
    {
    }

    ~ThreadJoiner()
    {
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

private:
    std::thread& m_thread;
};

}

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
    CPPUNIT_TEST(testReadPacketLittleEndianSizeParsing);
    CPPUNIT_TEST(testReadPacketShortHeader);
    CPPUNIT_TEST(testReadPacketShortPayload);
    CPPUNIT_TEST(testReadPacketClientDisconnectReturnsZeroSize);
    CPPUNIT_TEST(testFullWorkflowConstructOpenReadClose);
    CPPUNIT_TEST(testReopenAfterClose);
    CPPUNIT_TEST(testMultiplePacketsWithDifferentSizes);

CPPUNIT_TEST_SUITE_END();

protected:
    // Helper function to get an available port
    unsigned short getAvailablePort()
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        CPPUNIT_ASSERT(sockfd >= 0);

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;

        CPPUNIT_ASSERT_EQUAL(0, bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)));

        socklen_t addrLen = sizeof(addr);
        CPPUNIT_ASSERT_EQUAL(0, getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &addrLen));

        unsigned short port = ntohs(addr.sin_port);
        ::close(sockfd);

        CPPUNIT_ASSERT(port != 0);
        return port;
    }

    // Helper to create a client socket and connect
    int createClientSocket(const std::string& ip, unsigned short port)
    {
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)
        {
            return -1;
        }

        for (int attempt = 0; attempt < 20; ++attempt)
        {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                return -1;
            }

            if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == 0)
            {
                return sockfd;
            }

            ::close(sockfd);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return -1;
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
        unsigned short testPort = getAvailablePort();
        Ipv4SocketSource source("127.0.0.1:" + std::to_string(testPort));

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testConstructorWithEmptyPath()
    {
        Ipv4SocketSource source("");

        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testConstructorWithLongPath()
    {
        std::string longPath(5000, 'x');
        longPath += ":8080";
        Ipv4SocketSource source(longPath);

        CPPUNIT_ASSERT_EQUAL(false, source.open());
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
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        source.close();
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testCloseAfterSuccessfulOpen()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();

        DataPacket packet(1024);
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        Ipv4SocketSource reopened(path);
        CPPUNIT_ASSERT_EQUAL(true, reopened.open());
        reopened.close();
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

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testCloseAfterFailedOpen()
    {
        Ipv4SocketSource source("invalid:path");

        CPPUNIT_ASSERT_EQUAL(false, source.open());
        source.close();
        CPPUNIT_ASSERT_EQUAL(false, source.open());
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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

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
        ThreadJoiner joiner(clientThread);

        // Should fail because buffer can't hold payload
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), packet.getSize()); // 12 header + 4 payload

        const std::uint8_t* buffer = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xAA), buffer[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xBB), buffer[13]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xCC), buffer[14]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xDD), buffer[15]);

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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize()); // Just header

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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5012), packet.getSize()); // 12 + 5000

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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        source.close();
    }

    void testReadPacketLittleEndianSizeParsing()
    {
        unsigned short testPort = getAvailablePort();
        std::string path = "127.0.0.1:" + std::to_string(testPort);
        Ipv4SocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        DataPacket packet(1024);
        std::vector<std::uint8_t> expectedPayload(258, 0x5A);

        std::thread clientThread([testPort, expectedPayload]() {
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
                    // Send packet with payload size 258 encoded as little-endian bytes 02 01 00 00.
                    std::vector<std::uint8_t> data(12 + expectedPayload.size(), 0);
                    data[8] = 0x02;
                    data[9] = 0x01;
                    data[10] = 0x00;
                    data[11] = 0x00;
                    std::copy(expectedPayload.begin(), expectedPayload.end(), data.begin() + 12);

                    size_t totalSent = 0;
                    while (totalSent < data.size())
                    {
                        ssize_t sent = send(sock, &data[totalSent], data.size() - totalSent, 0);
                        if (sent <= 0)
                        {
                            break;
                        }
                        totalSent += static_cast<size_t>(sent);
                    }
                }
                ::close(sock);
            }
        });
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(270), packet.getSize()); // 12 + 258

        const std::uint8_t* buffer = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x02), buffer[8]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x01), buffer[9]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buffer[10]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buffer[11]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x5A), buffer[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x5A), buffer[269]);

        source.close();
    }

    void testReadPacketShortHeader()
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
                std::vector<std::uint8_t> shortHeader = {0x01, 0x02, 0x03, 0x04};
                sendPacketData(sock, shortHeader);
                ::close(sock);
            }
        });
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        source.close();
    }

    void testReadPacketShortPayload()
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
                std::vector<std::uint8_t> payload = {0x10, 0x20, 0x30, 0x40};
                auto packetData = makePacket(payload);
                packetData.resize(14);
                sendPacketData(sock, packetData);
                ::close(sock);
            }
        });
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

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
        ThreadJoiner joiner(clientThread);

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize()); // EOF

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
        ThreadJoiner joiner(clientThread);

        // Read
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(15), packet.getSize());

        // Close
        source.close();

        DataPacket closedPacket(1024);
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(closedPacket));

        Ipv4SocketSource reopened(path);
        CPPUNIT_ASSERT_EQUAL(true, reopened.open());
        reopened.close();
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

        Ipv4SocketSource other(path);
        CPPUNIT_ASSERT_EQUAL(true, other.open());
        other.close();
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
        ThreadJoiner joiner(clientThread);

        // Read all three packets
        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(22), packet.getSize());

        CPPUNIT_ASSERT_EQUAL(true, source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(512), packet.getSize());

        source.close();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ipv4SocketSourceTest);
