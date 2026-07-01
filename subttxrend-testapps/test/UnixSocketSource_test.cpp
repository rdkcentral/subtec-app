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
#include <utility>
#include <vector>
#include <cstdint>
#include <string>

#include "UnixSocketSource.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class UnixSocketSourceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(UnixSocketSourceTest);
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
    CPPUNIT_TEST(testOpenWithExistingSocketFile);
    CPPUNIT_TEST(testOpenWithExistingRegularFile);
    CPPUNIT_TEST(testOpenWithNonExistentParentDirectory);
    CPPUNIT_TEST(testOpenWithReadOnlyDirectory);
    CPPUNIT_TEST(testOpenVerifiesSocketFileCreated);
    CPPUNIT_TEST(testOpenWithPathAtBoundary);
    CPPUNIT_TEST(testOpenWithPathAtLimitFails);
    CPPUNIT_TEST(testOpenAfterFailedOpenAttempt);
    CPPUNIT_TEST(testOpenAfterSuccessfulOpenAndClose);
    CPPUNIT_TEST(testCloseOnOpenSocket);
    CPPUNIT_TEST(testCloseOnAlreadyClosedSocket);
    CPPUNIT_TEST(testCloseOnNeverOpenedSocket);
    CPPUNIT_TEST(testCloseMultipleTimesInSequence);
    CPPUNIT_TEST(testReadPacketWithValidDataAvailable);
    CPPUNIT_TEST(testReadPacketSkipsZeroLengthPackets);
    CPPUNIT_TEST(testReadPacketWithMultipleZeroLengthPacketsFollowedByData);
    CPPUNIT_TEST(testReadPacketOnClosedSocket);
    CPPUNIT_TEST(testReadPacketOnUnopenedSocket);
    CPPUNIT_TEST(testReadPacketWithMaximumCapacityPacket);
    CPPUNIT_TEST(testReadPacketWithMinimumCapacityPacket);
    CPPUNIT_TEST(testReadPacketSetsPacketSizeCorrectly);
    CPPUNIT_TEST(testReadPacketWithMultiplePacketsSequentially);
    CPPUNIT_TEST(testReadPacketWithDataLargerThanBuffer);
    CPPUNIT_TEST(testGetPathReturnsCorrectPath);
    CPPUNIT_TEST(testFullWorkflowConstructOpenReadClose);
    CPPUNIT_TEST(testReopenAfterClose);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_createdSockets.clear();
        m_createdDirectories.clear();
        m_testSocketCounter = 0;
    }

    void tearDown() override
    {
        for (const auto& path : m_createdDirectories)
        {
            (void) ::chmod(path.c_str(), 0755);
        }

        // Clean up any test socket files
        for (const auto& path : m_createdSockets)
        {
            (void) ::unlink(path.c_str());
        }
        m_createdSockets.clear();

        for (const auto& path : m_createdDirectories)
        {
            (void) ::rmdir(path.c_str());
        }
        m_createdDirectories.clear();
    }

protected:
    class JoiningThread
    {
    public:
        explicit JoiningThread(std::thread worker) :
                m_worker(std::move(worker))
        {
            // noop
        }

        ~JoiningThread()
        {
            if (m_worker.joinable())
            {
                m_worker.join();
            }
        }

        void join()
        {
            if (m_worker.joinable())
            {
                m_worker.join();
            }
        }

    private:
        std::thread m_worker;
    };

    std::vector<std::string> m_createdSockets;
    std::vector<std::string> m_createdDirectories;
    int m_testSocketCounter;

    // Helper function to generate unique socket path
    std::string getTestSocketPath()
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        std::string path = "/tmp/test_unix_socket_" + std::to_string(stamp) +
                          "_" + std::to_string(m_testSocketCounter++);
        m_createdSockets.push_back(path);
        return path;
    }

    std::string getTestDirectoryPath(const std::string& prefix)
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        std::string path = "/tmp/" + prefix + "_" + std::to_string(stamp) +
                          "_" + std::to_string(m_testSocketCounter++);
        m_createdDirectories.push_back(path);
        return path;
    }

    std::string getPathWithLength(std::size_t length, char fill = 'x')
    {
        const std::string prefix = "/tmp/";
        return prefix + std::string(length - prefix.size(), fill);
    }

    // Helper to send datagram to Unix socket
    bool sendDatagram(const std::string& targetPath, const std::vector<std::uint8_t>& data)
    {
        int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            return false;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, targetPath.c_str(), sizeof(addr.sun_path) - 1);

        ssize_t sent = sendto(sockfd, data.data(), data.size(), 0,
                             reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

        ::close(sockfd);
        return sent == static_cast<ssize_t>(data.size());
    }

    // Helper to send zero-length datagram
    bool sendZeroLengthDatagram(const std::string& targetPath)
    {
        return sendDatagram(targetPath, std::vector<std::uint8_t>());
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
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        // Verify object created successfully by checking that open() can be called
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testConstructorWithEmptyPath()
    {
        UnixSocketSource source("");
        // Object created successfully with empty path
        CPPUNIT_ASSERT_EQUAL(std::string(""), source.getPath());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(200, 'x');
        UnixSocketSource source(longPath);
        // Constructor succeeds; validation happens in open()
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testConstructorWithBoundaryPathLength()
    {
        // Unix socket path limit is typically 108 bytes including null terminator
        // So 107 characters should be at the boundary
        std::string boundaryPath = getPathWithLength(sizeof(sockaddr_un().sun_path) - 1);
        UnixSocketSource source(boundaryPath);
        // Path at exact boundary should work
        bool openResult = source.open();
        source.close();
        CPPUNIT_ASSERT_EQUAL(true, openResult);
    }

    void testConstructorWithSpecialCharactersInPath()
    {
        std::string path = getTestSocketPath() + "_special!@#$%";
        UnixSocketSource source(path);
        // Special characters are allowed in Unix paths
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testDestructorOnUnopenedSocket()
    {
        std::string path = getTestSocketPath();
        {
            UnixSocketSource source(path);
            // Destructor called without open
        }
        // No crash expected
        CPPUNIT_ASSERT(true);
    }

    void testDestructorOnOpenSocket()
    {
        std::string path = getTestSocketPath();
        {
            UnixSocketSource source(path);
            CPPUNIT_ASSERT_EQUAL(true, source.open());
            // Destructor called with open socket
        }
        // No crash expected
        CPPUNIT_ASSERT(true);
    }

    void testDestructorAfterExplicitClose()
    {
        std::string path = getTestSocketPath();
        {
            UnixSocketSource source(path);
            source.open();
            source.close();
            // Destructor called after explicit close
        }
        // No double-close issues expected
        CPPUNIT_ASSERT(true);
    }

    void testOpenWithValidPath()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // Verify socket file was created
        struct stat st;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &st));
        CPPUNIT_ASSERT(S_ISSOCK(st.st_mode));

        source.close();
    }

    void testOpenCalledTwiceReturnsTrue()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        source.close();
    }

    void testOpenWithPathTooLong()
    {
        // Path longer than sizeof(sockaddr_un.sun_path) which is typically 108
        std::string longPath(150, 'x');
        UnixSocketSource source(longPath);

        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithExistingSocketFile()
    {
        std::string path = getTestSocketPath();

        // Create first socket
        UnixSocketSource source1(path);
        CPPUNIT_ASSERT_EQUAL(true, source1.open());
        source1.close();

        // Open second socket with same path (should unlink and recreate)
        UnixSocketSource source2(path);
        CPPUNIT_ASSERT_EQUAL(true, source2.open());
        source2.close();
    }

    void testOpenWithExistingRegularFile()
    {
        std::string path = getTestSocketPath();

        // Create a regular file at the socket path
        CPPUNIT_ASSERT(createRegularFile(path));

        // Should unlink the file and create socket
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // Verify it's now a socket
        struct stat st;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &st));
        CPPUNIT_ASSERT(S_ISSOCK(st.st_mode));

        source.close();
    }

    void testOpenWithNonExistentParentDirectory()
    {
        std::string path = "/nonexistent/directory/socket";
        UnixSocketSource source(path);

        // bind() should fail because parent directory doesn't exist
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenWithReadOnlyDirectory()
    {
        std::string dirPath = getTestDirectoryPath("readonly_test_dir");
        std::string socketPath = dirPath + "/socket";

        m_createdSockets.push_back(socketPath);

        CPPUNIT_ASSERT_EQUAL(0, mkdir(dirPath.c_str(), 0555));

        if (::geteuid() == 0)
        {
            return;
        }

        UnixSocketSource source(socketPath);
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenVerifiesSocketFileCreated()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        // Before open, file shouldn't exist
        struct stat st;
        CPPUNIT_ASSERT_EQUAL(-1, stat(path.c_str(), &st));

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // After open, socket file should exist
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &st));
        CPPUNIT_ASSERT(S_ISSOCK(st.st_mode));

        source.close();
    }

    void testOpenWithPathAtBoundary()
    {
        // Path exactly at limit (107 characters for most systems)
        std::string boundaryPath = getPathWithLength(sizeof(sockaddr_un().sun_path) - 1, 'a');
        UnixSocketSource source(boundaryPath);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testOpenWithPathAtLimitFails()
    {
        std::string limitPath = getPathWithLength(sizeof(sockaddr_un().sun_path), 'b');
        UnixSocketSource source(limitPath);

        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenAfterFailedOpenAttempt()
    {
        std::string invalidPath(150, 'x');
        std::string validPath = getTestSocketPath();

        UnixSocketSource source(invalidPath);
        CPPUNIT_ASSERT_EQUAL(false, source.open());

        // Create new source with valid path
        UnixSocketSource source2(validPath);
        CPPUNIT_ASSERT_EQUAL(true, source2.open());
        source2.close();
    }

    void testOpenAfterSuccessfulOpenAndClose()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();

        // Reopen should succeed
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();
    }

    void testCloseOnOpenSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        source.open();
        source.close();

        // Verify no crash and operation is clean
        CPPUNIT_ASSERT(true);
    }

    void testCloseOnAlreadyClosedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        source.open();
        source.close();
        source.close(); // Second close

        CPPUNIT_ASSERT(true);
    }

    void testCloseOnNeverOpenedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        source.close();
        CPPUNIT_ASSERT(true);
    }

    void testCloseMultipleTimesInSequence()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        source.open();
        source.close();
        source.close();
        source.close();
        source.close();

        CPPUNIT_ASSERT(true);
    }

    void testReadPacketWithValidDataAvailable()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // Send data in background thread
        std::vector<std::uint8_t> testData = {0x01, 0x02, 0x03, 0x04, 0x05};
        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), 5));

        source.close();
    }

    void testReadPacketSkipsZeroLengthPackets()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {0xAA, 0xBB};

        // Send zero-length packet followed by real data
        bool emptySendResult = false;
        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &emptySendResult, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            emptySendResult = sendZeroLengthDatagram(path);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, emptySendResult);
        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), 2));

        source.close();
    }

    void testReadPacketWithMultipleZeroLengthPacketsFollowedByData()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {0x11, 0x22, 0x33};

        // Send multiple zero-length packets followed by real data
        bool emptySendResult1 = false;
        bool emptySendResult2 = false;
        bool emptySendResult3 = false;
        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &emptySendResult1, &emptySendResult2,
                &emptySendResult3, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            emptySendResult1 = sendZeroLengthDatagram(path);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            emptySendResult2 = sendZeroLengthDatagram(path);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            emptySendResult3 = sendZeroLengthDatagram(path);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, emptySendResult1);
        CPPUNIT_ASSERT_EQUAL(true, emptySendResult2);
        CPPUNIT_ASSERT_EQUAL(true, emptySendResult3);
        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), 3));

        source.close();
    }

    void testReadPacketOnClosedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        source.open();
        source.close();

        DataPacket packet(1024);
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));
    }

    void testReadPacketOnUnopenedSocket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        DataPacket packet(1024);
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));
    }

    void testReadPacketWithMaximumCapacityPacket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // Create large data packet
        std::vector<std::uint8_t> largeData(8192, 0xFF);
        for (size_t i = 0; i < largeData.size(); ++i)
        {
            largeData[i] = static_cast<std::uint8_t>(i % 256);
        }

        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, largeData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, largeData);
        }));

        DataPacket packet(16384);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8192), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), largeData.data(), largeData.size()));

        source.close();
    }

    void testReadPacketWithMinimumCapacityPacket()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {0x42};

        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[0]);

        source.close();
    }

    void testReadPacketSetsPacketSizeCorrectly()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        size_t initialSize = packet.getSize();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), initialSize);

        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), testData.size()));

        source.close();
    }

    void testReadPacketWithMultiplePacketsSequentially()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> data1 = {0x01, 0x02};
        std::vector<std::uint8_t> data2 = {0x03, 0x04, 0x05};
        std::vector<std::uint8_t> data3 = {0x06};

        bool sendResult1 = false;
        bool sendResult2 = false;
        bool sendResult3 = false;
        JoiningThread senderThread(std::thread([this, &sendResult1, &sendResult2,
                &sendResult3, path, data1, data2, data3]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult1 = sendDatagram(path, data1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            sendResult2 = sendDatagram(path, data2);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            sendResult3 = sendDatagram(path, data3);
        }));

        DataPacket packet1(1024);
        bool readResult1 = source.readPacket(packet1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet1.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet1.getBuffer(), data1.data(), data1.size()));

        DataPacket packet2(1024);
        bool readResult2 = source.readPacket(packet2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), packet2.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet2.getBuffer(), data2.data(), data2.size()));

        DataPacket packet3(1024);
        bool readResult3 = source.readPacket(packet3);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult1);
        CPPUNIT_ASSERT_EQUAL(true, sendResult2);
        CPPUNIT_ASSERT_EQUAL(true, sendResult3);
        CPPUNIT_ASSERT_EQUAL(true, readResult1);
        CPPUNIT_ASSERT_EQUAL(true, readResult2);
        CPPUNIT_ASSERT_EQUAL(true, readResult3);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), packet3.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet3.getBuffer(), data3.data(), data3.size()));

        source.close();
    }

    void testReadPacketWithDataLargerThanBuffer()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        // Send 100 bytes but buffer can only hold 50
        std::vector<std::uint8_t> largeData(100, 0xAA);

        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, largeData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, largeData);
        }));

        DataPacket packet(50);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        // Should truncate to buffer size
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(50), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), largeData.data(), packet.getSize()));

        source.close();
    }

    void testGetPathReturnsCorrectPath()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(path, source.getPath());
    }

    void testFullWorkflowConstructOpenReadClose()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {0xDE, 0xAD, 0xBE, 0xEF};
        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), testData.size()));

        source.close();
    }

    void testReopenAfterClose()
    {
        std::string path = getTestSocketPath();
        UnixSocketSource source(path);

        // First open-close cycle
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        source.close();

        // Second open-close cycle
        CPPUNIT_ASSERT_EQUAL(true, source.open());

        std::vector<std::uint8_t> testData = {0x12, 0x34};
        bool sendResult = false;
        JoiningThread senderThread(std::thread([this, &sendResult, path, testData]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            sendResult = sendDatagram(path, testData);
        }));

        DataPacket packet(1024);
        bool readResult = source.readPacket(packet);
        senderThread.join();

        CPPUNIT_ASSERT_EQUAL(true, sendResult);
        CPPUNIT_ASSERT_EQUAL(true, readResult);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), testData.data(), testData.size()));

        source.close();
    }
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(UnixSocketSourceTest);
