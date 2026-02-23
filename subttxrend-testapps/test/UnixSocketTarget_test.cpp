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
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <cstdint>
#include <string>

#include "UnixSocketTarget.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class UnixSocketTargetTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(UnixSocketTargetTest);
    CPPUNIT_TEST(testConstructorWithValidShortPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithBoundaryPathLength);
    CPPUNIT_TEST(testConstructorWithSpecialCharactersInPath);
    CPPUNIT_TEST(testDestructorOnUnopenedSocket);
    CPPUNIT_TEST(testDestructorOnOpenSocket);
    CPPUNIT_TEST(testDestructorAfterExplicitClose);
    CPPUNIT_TEST(testOpenWithValidPath);
    CPPUNIT_TEST(testOpenCalledTwiceReturnsTrue);
    CPPUNIT_TEST(testOpenWithPathTooLong);
    CPPUNIT_TEST(testOpenWithNonExistentSocket);
    CPPUNIT_TEST(testOpenWithExistingRegularFile);
    CPPUNIT_TEST(testOpenWithPathAtBoundary);
    CPPUNIT_TEST(testOpenAfterFailedOpenAttempt);
    CPPUNIT_TEST(testOpenAfterSuccessfulOpenAndClose);
    CPPUNIT_TEST(testOpenWithPathExactlyAtLimit);
    CPPUNIT_TEST(testOpenWithPathOneByteOverLimit);
    CPPUNIT_TEST(testCloseOnOpenSocket);
    CPPUNIT_TEST(testCloseOnAlreadyClosedSocket);
    CPPUNIT_TEST(testCloseOnNeverOpenedSocket);
    CPPUNIT_TEST(testCloseMultipleTimesInSequence);
    CPPUNIT_TEST(testWantsMorePacketsAlwaysReturnsTrue);
    CPPUNIT_TEST(testWantsMorePacketsOnUnopenedSocket);
    CPPUNIT_TEST(testWantsMorePacketsOnClosedSocket);
    CPPUNIT_TEST(testWantsMorePacketsMultipleCalls);
    CPPUNIT_TEST(testWritePacketWithValidDataOnOpenSocket);
    CPPUNIT_TEST(testWritePacketWithZeroSizePacket);
    CPPUNIT_TEST(testWritePacketOnClosedSocket);
    CPPUNIT_TEST(testWritePacketOnUnopenedSocket);
    CPPUNIT_TEST(testWritePacketWithSmallPacket);
    CPPUNIT_TEST(testWritePacketWithLargePacket);
    CPPUNIT_TEST(testWritePacketMultipleConsecutive);
    CPPUNIT_TEST(testWritePacketAfterServerClosed);
    CPPUNIT_TEST(testWritePacketVerifyDataReceived);
    CPPUNIT_TEST(testWritePacketVerifyCorrectSize);
    CPPUNIT_TEST(testGetPathReturnsCorrectPath);
    CPPUNIT_TEST(testFullWorkflowConstructOpenWriteClose);
    CPPUNIT_TEST(testReopenAfterClose);
    CPPUNIT_TEST(testMultipleWritesVerifyOrdering);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_testSocketCounter = 0;
    }

    void tearDown() override
    {
        // Clean up any test socket files
        for (const auto& path : m_createdSockets)
        {
            unlink(path.c_str());
        }
        m_createdSockets.clear();

        // Close any open server sockets
        for (int fd : m_openServerSockets)
        {
            if (fd >= 0)
            {
                ::close(fd);
            }
        }
        m_openServerSockets.clear();
    }

protected:
    std::vector<std::string> m_createdSockets;
    std::vector<int> m_openServerSockets;
    int m_testSocketCounter;

    // Helper function to generate unique socket path
    std::string getTestSocketPath()
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        std::string path = "/tmp/test_unix_socket_target_" + std::to_string(stamp) +
                          "_" + std::to_string(m_testSocketCounter++);
        m_createdSockets.push_back(path);
        return path;
    }

    // Helper to create a server socket that target can connect to
    int createServerSocket(const std::string& path)
    {
        int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            return -1;
        }

        struct sockaddr_un addr;
        if (path.size() >= sizeof(addr.sun_path))
        {
            ::close(sockfd);
            return -1;
        }

        // Remove existing socket file if it exists
        unlink(path.c_str());

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

        if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            ::close(sockfd);
            return -1;
        }

        m_openServerSockets.push_back(sockfd);
        return sockfd;
    }

    // Helper to receive data from server socket
    ssize_t receiveData(int sockfd, char* buffer, size_t bufferSize, int timeoutMs = 1000)
    {
        // Set socket to non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        auto startTime = std::chrono::steady_clock::now();
        while (true)
        {
            ssize_t received = recv(sockfd, buffer, bufferSize, 0);
            if (received >= 0)
            {
                // Restore blocking mode
                fcntl(sockfd, F_SETFL, flags);
                return received;
            }

            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                // Real error
                fcntl(sockfd, F_SETFL, flags);
                return -1;
            }

            // Check timeout
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            if (elapsed >= timeoutMs)
            {
                fcntl(sockfd, F_SETFL, flags);
                return -1; // Timeout
            }

            // Small sleep to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Helper to create a regular file at path
    bool createRegularFile(const std::string& path)
    {
        int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0666);
        if (fd < 0)
        {
            return false;
        }
        const char* data = "test";
        ssize_t written = write(fd, data, 4);
        ::close(fd);
        return written == 4;
    }

    void testConstructorWithValidShortPath()
    {
        std::string path = "/tmp/test_socket";
        UnixSocketTarget target(path);
        // Verify object created successfully by checking that getPath() works
        CPPUNIT_ASSERT_EQUAL(path, target.getPath());
    }

    void testConstructorWithEmptyPath()
    {
        UnixSocketTarget target("");
        // Object created successfully with empty path
        CPPUNIT_ASSERT_EQUAL(std::string(""), target.getPath());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(200, 'x');
        UnixSocketTarget target(longPath);
        // Constructor succeeds; validation happens in open()
        CPPUNIT_ASSERT_EQUAL(longPath, target.getPath());
    }

    void testConstructorWithBoundaryPathLength()
    {
        // Unix socket path limit is typically 108 bytes including null terminator
        // So 107 characters should be at the boundary
        std::string boundaryPath = "/tmp/" + std::string(102, 'x');
        UnixSocketTarget target(boundaryPath);
        CPPUNIT_ASSERT_EQUAL(boundaryPath, target.getPath());
    }

    void testConstructorWithSpecialCharactersInPath()
    {
        std::string path = "/tmp/socket_special!@#$%";
        UnixSocketTarget target(path);
        // Special characters are allowed in Unix paths
        CPPUNIT_ASSERT_EQUAL(path, target.getPath());
    }

    void testDestructorOnUnopenedSocket()
    {
        std::string path = getTestSocketPath();
        {
            UnixSocketTarget target(path);
            // Destructor called without open
        }
        // No crash expected
        CPPUNIT_ASSERT(true);
    }

    void testDestructorOnOpenSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        {
            UnixSocketTarget target(path);
            CPPUNIT_ASSERT_EQUAL(true, target.open());
            // Destructor called with open socket
        }
        // Socket should be cleaned up, no crash expected
        CPPUNIT_ASSERT(true);
    }

    void testDestructorAfterExplicitClose()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        {
            UnixSocketTarget target(path);
            target.open();
            target.close();
            // Destructor called after explicit close
        }
        // No double-close issues expected
        CPPUNIT_ASSERT(true);
    }

    void testOpenWithValidPath()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        target.close();
    }

    void testOpenCalledTwiceReturnsTrue()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);

        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.open()); // Second call should return true
        CPPUNIT_ASSERT_EQUAL(true, target.open()); // Third call should also return true

        target.close();
    }

    void testOpenWithPathTooLong()
    {
        // Path longer than sizeof(sockaddr_un.sun_path) which is typically 108
        std::string longPath(150, 'x');
        UnixSocketTarget target(longPath);

        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testOpenWithNonExistentSocket()
    {
        std::string path = "/tmp/nonexistent_socket_xyz123";
        UnixSocketTarget target(path);

        // Should fail because no server is listening at this path
        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testOpenWithExistingRegularFile()
    {
        std::string path = getTestSocketPath();

        // Create a regular file at the socket path
        CPPUNIT_ASSERT(createRegularFile(path));

        UnixSocketTarget target(path);
        // Should fail because target tries to connect to a regular file, not a socket
        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testOpenWithPathAtBoundary()
    {
        // Path exactly at limit (107 characters for most systems)
        std::string boundaryPath = "/tmp/" + std::string(102, 'a');

        int serverFd = createServerSocket(boundaryPath);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(boundaryPath);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        target.close();
    }

    void testOpenAfterFailedOpenAttempt()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        // First attempt should fail (no server)
        CPPUNIT_ASSERT_EQUAL(false, target.open());

        // Create server
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        // Second attempt should succeed
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        target.close();
    }

    void testOpenAfterSuccessfulOpenAndClose()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);

        // First open
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();

        // Second open after close
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
    }

    void testOpenWithPathExactlyAtLimit()
    {
        // Test path with exactly 107 characters (limit - 1 for null terminator)
        std::string exactPath = std::string(107, 'x');

        UnixSocketTarget target(exactPath);
        // Should fail size check: path.size() >= sizeof(addr.sun_path)
        // sizeof(addr.sun_path) is typically 108, so 107 should pass the check
        // but will fail to connect since path doesn't exist
        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testOpenWithPathOneByteOverLimit()
    {
        // Test path with 108 characters (at the limit)
        std::string overPath = std::string(108, 'x');

        UnixSocketTarget target(overPath);
        // Should fail size check: path.size() >= sizeof(addr.sun_path)
        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testCloseOnOpenSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        target.close();
        // No crash expected, close should succeed
        CPPUNIT_ASSERT(true);
    }

    void testCloseOnAlreadyClosedSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        target.open();
        target.close();
        target.close(); // Second close

        // No crash expected, close is idempotent
        CPPUNIT_ASSERT(true);
    }

    void testCloseOnNeverOpenedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        target.close();
        // Close on unopened socket should be safe
        CPPUNIT_ASSERT(true);
    }

    void testCloseMultipleTimesInSequence()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        target.close();
        target.close();
        target.close();

        // Multiple closes should all be safe
        CPPUNIT_ASSERT(true);
    }

    void testWantsMorePacketsAlwaysReturnsTrue()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        target.open();

        // Should always return true
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testWantsMorePacketsOnUnopenedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        // Should return true even when not opened
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
    }

    void testWantsMorePacketsOnClosedSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        target.open();
        target.close();

        // Should return true even after close
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
    }

    void testWantsMorePacketsMultipleCalls()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        // All calls should return true
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
    }

    void testWritePacketWithValidDataOnOpenSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 10);
        packet.setSize(10);

        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(true, result);

        target.close();
    }

    void testWritePacketWithZeroSizePacket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(10);
        packet.setSize(0); // Zero size packet

        // Write should succeed (0 bytes written == 0 bytes requested)
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(true, result);

        target.close();
    }

    void testWritePacketOnClosedSocket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        target.open();
        target.close();

        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 10);
        packet.setSize(10);

        // Writing to closed socket should fail
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketOnUnopenedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketTarget target(path);

        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 10);
        packet.setSize(10);

        // Writing to unopened socket should fail
        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testWritePacketWithSmallPacket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(1);
        char* buffer = packet.getBuffer();
        buffer[0] = 0x99;
        packet.setSize(1);

        bool result = target.writePacket(packet);
        CPPUNIT_ASSERT_EQUAL(true, result);

        target.close();
    }

    void testWritePacketWithLargePacket()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(65536);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0xAA, 65536);
        packet.setSize(65536);

        bool result = target.writePacket(packet);
        // Large datagram might fail or succeed depending on system limits
        // Just verify it doesn't crash
        (void)result;
        CPPUNIT_ASSERT(true);

        target.close();
    }

    void testWritePacketMultipleConsecutive()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

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

        // All writes should succeed
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet1));
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet2));
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet3));

        target.close();
    }

    void testWritePacketAfterServerClosed()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Close server socket
        ::close(serverFd);
        unlink(path.c_str());

        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0x42, 10);
        packet.setSize(10);

        // Write might still succeed for DGRAM sockets as they are connectionless
        // Just verify it doesn't crash
        bool result = target.writePacket(packet);
        (void)result;
        CPPUNIT_ASSERT(true);

        target.close();
    }

    void testWritePacketVerifyDataReceived()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Prepare packet with known data
        DataPacket packet(10);
        char* buffer = packet.getBuffer();
        for (int i = 0; i < 10; i++)
        {
            buffer[i] = static_cast<char>(i);
        }
        packet.setSize(10);

        // Write packet
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));

        // Receive and verify data
        char recvBuffer[100];
        ssize_t received = receiveData(serverFd, recvBuffer, sizeof(recvBuffer), 2000);
        CPPUNIT_ASSERT_EQUAL(static_cast<ssize_t>(10), received);

        // Verify data content
        for (int i = 0; i < 10; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(i), recvBuffer[i]);
        }

        target.close();
    }

    void testWritePacketVerifyCorrectSize()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Create packet with capacity 100 but size 25
        DataPacket packet(100);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0xFF, 25);
        packet.setSize(25);

        // Write packet
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));

        // Receive and verify only 25 bytes received, not 100
        char recvBuffer[150];
        ssize_t received = receiveData(serverFd, recvBuffer, sizeof(recvBuffer), 2000);
        CPPUNIT_ASSERT_EQUAL(static_cast<ssize_t>(25), received);

        target.close();
    }

    void testGetPathReturnsCorrectPath()
    {
        std::string path = "/tmp/test_socket_path_123";
        UnixSocketTarget target(path);

        CPPUNIT_ASSERT_EQUAL(path, target.getPath());
    }

    void testFullWorkflowConstructOpenWriteClose()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);

        // Open
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Write
        DataPacket packet(20);
        char* buffer = packet.getBuffer();
        std::memset(buffer, 0xBB, 20);
        packet.setSize(20);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));

        // Close
        target.close();

        CPPUNIT_ASSERT(true);
    }

    void testReopenAfterClose()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);

        // First open-write-close cycle
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        DataPacket packet1(10);
        packet1.setSize(10);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet1));
        target.close();

        // Second open-write-close cycle
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        DataPacket packet2(10);
        packet2.setSize(10);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet2));
        target.close();

        CPPUNIT_ASSERT(true);
    }

    void testMultipleWritesVerifyOrdering()
    {
        std::string path = getTestSocketPath();
        int serverFd = createServerSocket(path);
        CPPUNIT_ASSERT(serverFd >= 0);

        UnixSocketTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Write three packets with different data
        DataPacket packet1(5);
        char* buffer1 = packet1.getBuffer();
        for (int i = 0; i < 5; i++) buffer1[i] = 0x01;
        packet1.setSize(5);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet1));

        DataPacket packet2(5);
        char* buffer2 = packet2.getBuffer();
        for (int i = 0; i < 5; i++) buffer2[i] = 0x02;
        packet2.setSize(5);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet2));

        DataPacket packet3(5);
        char* buffer3 = packet3.getBuffer();
        for (int i = 0; i < 5; i++) buffer3[i] = 0x03;
        packet3.setSize(5);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet3));

        // Receive and verify ordering
        char recvBuffer[10];

        ssize_t recv1 = receiveData(serverFd, recvBuffer, sizeof(recvBuffer), 2000);
        CPPUNIT_ASSERT_EQUAL(static_cast<ssize_t>(5), recv1);
        for (int i = 0; i < 5; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x01), recvBuffer[i]);
        }

        ssize_t recv2 = receiveData(serverFd, recvBuffer, sizeof(recvBuffer), 2000);
        CPPUNIT_ASSERT_EQUAL(static_cast<ssize_t>(5), recv2);
        for (int i = 0; i < 5; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x02), recvBuffer[i]);
        }

        ssize_t recv3 = receiveData(serverFd, recvBuffer, sizeof(recvBuffer), 2000);
        CPPUNIT_ASSERT_EQUAL(static_cast<ssize_t>(5), recv3);
        for (int i = 0; i < 5; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x03), recvBuffer[i]);
        }

        target.close();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UnixSocketTargetTest);
