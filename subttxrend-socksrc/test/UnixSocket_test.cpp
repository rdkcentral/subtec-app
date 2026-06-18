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
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <exception>
#include <cstring>

#include "../src/UnixSocket.hpp"

using subttxrend::socksrc::UnixSocket;
using subttxrend::common::DataBuffer;

// ============================================================================
// Test Fixture
// ============================================================================

class UnixSocketTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(UnixSocketTest);
    CPPUNIT_TEST(testConstructorWithValidShortPath);
    CPPUNIT_TEST(testConstructorWithValidRelativePath);
    CPPUNIT_TEST(testConstructorWithPathAtMaximumLength);
    CPPUNIT_TEST(testConstructorWithPathExactlyAtLimit);
    CPPUNIT_TEST(testConstructorWithPathExceedingLimit);
    CPPUNIT_TEST(testConstructorWithPathToNonExistentDirectory);
    CPPUNIT_TEST(testConstructorCreatesAfUnixSocket);
    CPPUNIT_TEST(testConstructorWithPathContainingSpaces);
    CPPUNIT_TEST(testConstructorUnlinkAndReusePath);
    CPPUNIT_TEST(testConstructorPathWithSlash);
    CPPUNIT_TEST(testConstructorMultipleInstancesDifferentPaths);
    CPPUNIT_TEST(testGetSocketBufferSizeReturnsReasonableValue);
    CPPUNIT_TEST(testGetSocketBufferSizeCalledMultipleTimes);
    CPPUNIT_TEST(testGetSocketBufferSizeConstQualifier);
    CPPUNIT_TEST(testPeekBytesWithEmptyBuffer);
    CPPUNIT_TEST(testPeekBytesWithBufferCapacity1);
    CPPUNIT_TEST(testPeekBytesWithBufferLargerThanData);
    CPPUNIT_TEST(testPeekBytesReturnValue);
    CPPUNIT_TEST(testPeekBytesTwiceConsecutively);
    CPPUNIT_TEST(testReadBytesWithEmptyBuffer);
    CPPUNIT_TEST(testReadBytesWithBufferCapacity1);
    CPPUNIT_TEST(testReadBytesWithBufferLargerThanData);
    CPPUNIT_TEST(testReadBytesWithZeroLengthDatagram);
    CPPUNIT_TEST(testReadBytesWithNoDataAfterZeroLengthDatagram);
    CPPUNIT_TEST(testReadBytesWithLargeBuffer);
    CPPUNIT_TEST(testReadBytesAfterPeekBytes);
    CPPUNIT_TEST(testReadBytesDataIntegrity);
    CPPUNIT_TEST(testSocketExceptionConstruction);
    CPPUNIT_TEST(testSocketExceptionWithEmptyMessage);
    CPPUNIT_TEST(testSocketExceptionWithLongMessage);
    CPPUNIT_TEST(testHandleValidSocketFd);
    CPPUNIT_TEST(testSocketExceptionMessageContainsError);
    CPPUNIT_TEST(testHandleMultipleOperations);
    CPPUNIT_TEST(testCreateSocketWriteDataReadBytes);
    CPPUNIT_TEST(testHandleOwnershipRAII);
    CPPUNIT_TEST(testHandleConsistency);
    CPPUNIT_TEST(testPeekReadPeekSequence);
    CPPUNIT_TEST(testMultipleSocketsIndependence);
    CPPUNIT_TEST(testSocketBufferConsistency);
    CPPUNIT_TEST(testSocketOperationsAfterShutdown);
    CPPUNIT_TEST(testErrorPropagationHierarchy);
    CPPUNIT_TEST(testSocketWithZeroByteBuffer);
    CPPUNIT_TEST(testSocketWithMaxBufferSize);
    CPPUNIT_TEST(testReadConsumesData);
    CPPUNIT_TEST(testConcurrentSocketCreation);
    CPPUNIT_TEST(testPathEndingWithoutExtension);
    CPPUNIT_TEST(testPathWithDotPrefix);
    CPPUNIT_TEST(testBufferResizeToZero);
    CPPUNIT_TEST(testBufferResizeToLarge);
    CPPUNIT_TEST(testPeekWithVariousBufferSizes);
    CPPUNIT_TEST(testReadWithVariousBufferSizes);
    CPPUNIT_TEST(testSocketStateAfterException);
    CPPUNIT_TEST(testSocketCreationTiming);
    CPPUNIT_TEST(testBufferDataIntegrityAfterPeek);
    CPPUNIT_TEST(testBufferDataIntegrityAfterRead);
    CPPUNIT_TEST(testSocketValidityAfterOperations);
    CPPUNIT_TEST(testBufferStateConsistency);
    CPPUNIT_TEST(testHighFrequencyReadOperations);
    CPPUNIT_TEST(testBufferReallocationsPattern);
    CPPUNIT_TEST(testMultipleSocketsDataIntegrity);
    CPPUNIT_TEST(testExceptionRecoveryPattern);
    CPPUNIT_TEST(testSocketLifecycleStressTest);
    CPPUNIT_TEST(testComplexDataFlowScenario);
    CPPUNIT_TEST(testResourceCleanupVerification);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Create unique test socket paths
        m_testSocketPath = generateTestSocketPath();
        m_testSocketPath2 = generateTestSocketPath();
    }

    void tearDown()
    {
        // Clean up test socket files
        cleanupSocketFile(m_testSocketPath);
        cleanupSocketFile(m_testSocketPath2);
    }

    void testConstructorWithValidShortPath()
    {
        // Create socket with valid absolute path (tests basic constructor functionality)
        std::string path = m_testSocketPath;

        try {
            UnixSocket socket(path);

            // Verify socket file was created
            struct stat statbuf;
            CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &statbuf));
            CPPUNIT_ASSERT(S_ISSOCK(statbuf.st_mode));
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }
    }

    void testConstructorWithValidRelativePath()
    {
        // Use relative path
        std::string path = "./test_socket_relative.sock";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);

            // Verify socket was created
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }

        cleanupSocketFile(path);
    }

    void testConstructorWithPathAtMaximumLength()
    {
        // Create path just under the limit
        struct sockaddr_un addr{};
        std::string path = "/tmp/" + std::string(sizeof(addr.sun_path) - std::strlen("/tmp/") - 1, 'a');

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(sizeof(addr.sun_path) - 1), path.size());

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            cleanupSocketFile(path);
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }

        cleanupSocketFile(path);
    }

    void testConstructorWithPathExactlyAtLimit()
    {
        // Create path exactly at sizeof(addr.sun_path)
        struct sockaddr_un addr{};
        std::string path(sizeof(addr.sun_path), 'x');

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown SocketException for path at size limit");
        }
        catch (const UnixSocket::SocketException& e) {
            // Expected exception
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.find("too long") != std::string::npos);
        }
    }

    void testConstructorWithPathExceedingLimit()
    {
        // Create path exceeding sizeof(addr.sun_path)
        struct sockaddr_un addr{};
        std::string path(sizeof(addr.sun_path) + 10, 'y');

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown SocketException for oversized path");
        }
        catch (const UnixSocket::SocketException& e) {
            // Expected exception
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.find("too long") != std::string::npos);
        }
    }

    void testConstructorWithEmptyStringPath()
    {
        // Empty string path
        std::string path = "";

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(!socketFileExists(path));
            CPPUNIT_ASSERT(socket.getSocketBufferSize() > 0);
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }
    }

    void testConstructorUnlinkAndReusePath()
    {
        // Consolidated test: unlink-before-bind, reuse, and sequential creation
        std::string path = m_testSocketPath;

        // Create initial socket and ensure file exists
        {
            UnixSocket s1(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Recreate on same path after destruction
        try {
            UnixSocket s2(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Expected to reuse path after destruction: ") + e.what());
        }

        // Create then unlink manually and recreate to exercise unlink-before-bind
        {
            UnixSocket s3(path);
        }
        unlink(path.c_str());
        try {
            UnixSocket s4(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Failed to bind after manual unlink: ") + e.what());
        }
    }

    void testConstructorWithPathToNonExistentDirectory()
    {
        // Path to non-existent directory
        std::string path = "/nonexistent_dir_12345/test.sock";

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown SocketException for invalid directory");
        }
        catch (const UnixSocket::SocketException& e) {
            // Expected exception
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.find("Cannot bind") != std::string::npos);
        }
    }

    void testConstructorWithPathContainingSpaces()
    {
        // Path with spaces
        std::string path = "/tmp/test socket with spaces.sock";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            cleanupSocketFile(path);
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }

        cleanupSocketFile(path);
    }

    void testConstructorCreatesAfUnixSocket()
    {
        // Verify AF_UNIX socket family
        std::string path = m_testSocketPath;

        UnixSocket socket(path);

        // Socket file should exist and be of type socket
        struct stat statbuf;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &statbuf));
        CPPUNIT_ASSERT(S_ISSOCK(statbuf.st_mode));
    }

    void testConstructorPathWithSlash()
    {
        // Path ending with slash (invalid)
        std::string path = "/tmp/testsocket/";

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown SocketException for invalid path ending with slash");
        }
        catch (const UnixSocket::SocketException& e) {
            CPPUNIT_ASSERT(std::string(e.what()).find("Cannot bind") != std::string::npos);
        }
    }

    void testConstructorMultipleInstancesDifferentPaths()
    {
        // Multiple sockets with different paths
        std::string path1 = m_testSocketPath;
        std::string path2 = m_testSocketPath2;

        UnixSocket socket1(path1);
        UnixSocket socket2(path2);

        // Both should exist
        CPPUNIT_ASSERT(socketFileExists(path1));
        CPPUNIT_ASSERT(socketFileExists(path2));
    }

    void testGetSocketBufferSizeReturnsReasonableValue()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Should be reasonable (between 1KB and 10MB)
        CPPUNIT_ASSERT(bufferSize >= 1024);
        CPPUNIT_ASSERT(bufferSize <= 10 * 1024 * 1024);
    }

    void testGetSocketBufferSizeCalledMultipleTimes()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t size1 = socket.getSocketBufferSize();
        std::size_t size2 = socket.getSocketBufferSize();
        std::size_t size3 = socket.getSocketBufferSize();

        // Should return consistent value
        CPPUNIT_ASSERT_EQUAL(size1, size2);
        CPPUNIT_ASSERT_EQUAL(size2, size3);
    }

    void testGetSocketBufferSizeConstQualifier()
    {
        std::string path = m_testSocketPath;
        const UnixSocket socket(path);

        // Should be callable on const object
        std::size_t bufferSize = socket.getSocketBufferSize();

        CPPUNIT_ASSERT(bufferSize > 0);
    }

    void testPeekBytesWithEmptyBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Ensure non-blocking behavior while expecting 0 bytes
        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(0);

        // Peek with zero capacity buffer should be a no-op and return 0
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testPeekBytesWithBufferCapacity1()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "x";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(1);

        // Peek with minimal capacity should return the payload
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), buffer.size());
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, 1) == 0);
    }

    void testPeekBytesWithBufferLargerThanData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "short";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(1024);

        // Large buffer vs small payload should yield payload length
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), buffer.size());
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testPeekBytesTwiceConsecutively()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "peek";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        // Peek twice - both should see same data
        std::size_t bytesRead1 = socket.peekBytes(buffer1);
        std::size_t bytesRead2 = socket.peekBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(bytesRead1, bytesRead2);
        CPPUNIT_ASSERT_EQUAL(buffer1.size(), buffer2.size());
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead1);
        CPPUNIT_ASSERT(std::memcmp(buffer1.data(), payload, bytesRead1) == 0);
        CPPUNIT_ASSERT(std::memcmp(buffer2.data(), payload, bytesRead2) == 0);
    }

    void testPeekBytesReturnValue()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(256);

        std::size_t result = socket.peekBytes(buffer);

        // Return value should match buffer size
        CPPUNIT_ASSERT_EQUAL(result, buffer.size());
    }

    void testReadBytesWithEmptyBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(0);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testReadBytesWithBufferCapacity1()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "x";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(1);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), buffer.size());
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, 1) == 0);
    }

    void testReadBytesWithBufferLargerThanData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "small";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(2048);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testReadBytesWithZeroLengthDatagram()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(100);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testReadBytesWithNoDataAfterZeroLengthDatagram()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(100);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testReadBytesAfterPeekBytes()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "peekread";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer peekBuffer(100);
        DataBuffer readBuffer(100);

        // Peek then read
        std::size_t peeked = socket.peekBytes(peekBuffer);
        std::size_t read = socket.readBytes(readBuffer);

        // Should read same amount
        CPPUNIT_ASSERT_EQUAL(peeked, read);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), read);
        CPPUNIT_ASSERT(std::memcmp(peekBuffer.data(), payload, read) == 0);
        CPPUNIT_ASSERT(std::memcmp(readBuffer.data(), payload, read) == 0);
    }

    void testReadBytesWithLargeBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "large";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(64 * 1024);
        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testReadBytesDataIntegrity()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload = "integrity";
        enqueueDatagram(path, payload, strlen(payload));

        DataBuffer buffer(512);

        std::size_t bytesRead = socket.readBytes(buffer);

        // Buffer size should match bytes read
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testSocketExceptionConstruction()
    {
        // Test exception construction with message
        UnixSocket::SocketException ex("Test error message");

        std::string msg = ex.what();
        CPPUNIT_ASSERT_EQUAL(std::string("Test error message"), msg);
    }

    void testSocketExceptionWithEmptyMessage()
    {
        // Exception with empty message
        UnixSocket::SocketException ex("");

        std::string msg = ex.what();
        CPPUNIT_ASSERT_EQUAL(std::string(""), msg);
    }

    void testSocketExceptionWithLongMessage()
    {
        // Exception with very long message
        std::string longMsg(1000, 'x');
        UnixSocket::SocketException ex(longMsg);

        std::string msg = ex.what();
        CPPUNIT_ASSERT_EQUAL(longMsg, msg);
    }

    void testSocketExceptionMessageContainsError()
    {
        // Test that thrown exceptions contain error information
        try {
            UnixSocket socket("/invalid/nonexistent/path.sock");
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const UnixSocket::SocketException& e) {
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.length() > 0);
            // Should contain meaningful error text
            CPPUNIT_ASSERT(msg.find("Cannot bind") != std::string::npos ||
                          msg.find("too long") != std::string::npos ||
                          msg.find("socket") != std::string::npos);
        }
    }

    void testHandleValidSocketFd()
    {
        // Verify socket has valid file descriptor
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Socket should be created successfully
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testHandleOwnershipRAII()
    {
        // Verify RAII cleanup - destructor properly closes socket handle
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            // Socket is active within scope
            std::size_t bufferSize = socket.getSocketBufferSize();
            CPPUNIT_ASSERT(bufferSize > 0);
        }

        // After scope exit, socket handle should be closed
        // We can verify by creating a new socket on the same path
        // If handle was properly closed, this should succeed
        UnixSocket newSocket(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testHandleMultipleOperations()
    {
        // Multiple operations on same handle
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Multiple operations should work
        std::size_t size1 = socket.getSocketBufferSize();
        std::size_t size2 = socket.getSocketBufferSize();

        CPPUNIT_ASSERT_EQUAL(size1, size2);

        DataBuffer buffer(100);
        const char* payload = "ops";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t peeked = socket.peekBytes(buffer);
        std::size_t read = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), peeked);
        CPPUNIT_ASSERT_EQUAL(peeked, read);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, read) == 0);

        // All operations should complete
    }

    void testHandleConsistency()
    {
        // Handle consistency across operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(50);
        DataBuffer buffer2(50);

        const char* payload = "hc";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes1 = socket.peekBytes(buffer1);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes2 = socket.peekBytes(buffer2);

        // Operations use same handle
        CPPUNIT_ASSERT_EQUAL(bytes1, bytes2);
        CPPUNIT_ASSERT(std::memcmp(buffer1.data(), payload, bytes1) == 0);
        CPPUNIT_ASSERT(std::memcmp(buffer2.data(), payload, bytes2) == 0);
    }

    void testCreateSocketWriteDataReadBytes()
    {
        // End-to-end test: create socket, write externally, read
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Send data using helper to reduce duplication
        const char* testData = "World";
        enqueueDatagram(path, testData, strlen(testData));

        DataBuffer buffer(100);
        std::size_t read = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(testData), read);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), testData, read) == 0);
    }

    void testPeekReadPeekSequence()
    {
        // Test peek-read-peek sequence
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Use a single-byte datagram to ensure non-blocking and consume on read
        enqueueSingleByteDatagram(path);

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);
        DataBuffer buffer3(100);

        // Peek, read, peek again
        socket.peekBytes(buffer1);
        socket.readBytes(buffer2);
        // Ensure final peek does not block and still expects 0 bytes
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer3);

        // After read, subsequent peek should see no data (or new data)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer3.size());
    }

    void testMultipleSocketsIndependence()
    {
        // Multiple sockets should be independent
        std::string path1 = m_testSocketPath;
        std::string path2 = m_testSocketPath2;

        UnixSocket socket1(path1);
        UnixSocket socket2(path2);

        // Both should exist independently
        CPPUNIT_ASSERT(socketFileExists(path1));
        CPPUNIT_ASSERT(socketFileExists(path2));

        // Operations on one don't affect the other
        std::size_t size1 = socket1.getSocketBufferSize();
        std::size_t size2 = socket2.getSocketBufferSize();

        CPPUNIT_ASSERT(size1 > 0);
        CPPUNIT_ASSERT(size2 > 0);
    }

    void testSocketBufferConsistency()
    {
        // Buffer size query consistent with operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Can allocate buffer of this size
        DataBuffer buffer(bufferSize);

        // Operations should work with buffer
        const char* payload = "buf";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t peeked = socket.peekBytes(buffer);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), peeked);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, peeked) == 0);

        CPPUNIT_ASSERT(bufferSize > 0);
    }

    void testSocketOperationsAfterShutdown()
    {
        // Operations after shutdown
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        socket.shutdown();

        // Operations may fail or return 0 after shutdown
        DataBuffer buffer(100);
        std::size_t result = 0;
        bool threw = false;
        try {
            result = socket.readBytes(buffer);
        }
        catch (const UnixSocket::SocketException&) {
            threw = true;
        }

        CPPUNIT_ASSERT(threw || result == 0);
    }

    void testErrorPropagationHierarchy()
    {
        // Exception propagation through hierarchy
        try {
            UnixSocket socket("/nonexistent/path.sock");
        }
        catch (const std::exception& e) {
            // Catchable as std::exception
            CPPUNIT_ASSERT(std::string(e.what()).length() > 0);
        }
    }

    void testSocketWithZeroByteBuffer()
    {
        // Socket with zero-byte buffer
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);
        DataBuffer buffer(0);

        std::size_t result = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), result);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testSocketWithMaxBufferSize()
    {
        // Socket with maximum buffer size
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t maxSize = socket.getSocketBufferSize();
        DataBuffer buffer(maxSize);

        // Should handle max buffer
        const char* payload = "max";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT(buffer.capacity() >= maxSize);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
    }

    void testReadConsumesData()
    {
        // Verify read consumes data
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        const char* payload1 = "first";
        const char* payload2 = "second";

        enqueueDatagram(path, payload1, strlen(payload1));
        enqueueDatagram(path, payload2, strlen(payload2));

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        std::size_t bytes1 = socket.readBytes(buffer1);
        std::size_t bytes2 = socket.readBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(strlen(payload1), bytes1);
        CPPUNIT_ASSERT_EQUAL(strlen(payload2), bytes2);
        CPPUNIT_ASSERT(std::memcmp(buffer1.data(), payload1, bytes1) == 0);
        CPPUNIT_ASSERT(std::memcmp(buffer2.data(), payload2, bytes2) == 0);
    }

    void testConcurrentSocketCreation()
    {
        // Create sockets concurrently on different paths to verify thread-safety
        std::string path1 = m_testSocketPath;
        std::string path2 = m_testSocketPath2;

        std::exception_ptr ex1 = nullptr;
        std::exception_ptr ex2 = nullptr;

        auto makeSocket = [](const std::string& p, std::exception_ptr& ep) {
            try {
                UnixSocket s(p);
                // Let the socket exist until thread end
                (void)s.getSocketBufferSize();
            } catch (...) {
                ep = std::current_exception();
            }
        };

        std::thread t1(makeSocket, std::cref(path1), std::ref(ex1));
        std::thread t2(makeSocket, std::cref(path2), std::ref(ex2));
        t1.join();
        t2.join();

        // No exceptions in either thread
        CPPUNIT_ASSERT(ex1 == nullptr);
        CPPUNIT_ASSERT(ex2 == nullptr);

        // Both should coexist
        CPPUNIT_ASSERT(socketFileExists(path1));
        CPPUNIT_ASSERT(socketFileExists(path2));
    }

    void testPathEndingWithoutExtension()
    {
        // Path without .sock extension
        std::string path = "/tmp/test_socket_no_ext";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            cleanupSocketFile(path);
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }

        cleanupSocketFile(path);
    }

    void testPathWithDotPrefix()
    {
        // Hidden file with dot prefix
        std::string path = "/tmp/.hidden_socket.sock";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            cleanupSocketFile(path);
            CPPUNIT_FAIL(std::string("Unexpected exception: ") + e.what());
        }

        cleanupSocketFile(path);
    }

    void testBufferResizeToZero()
    {
        // Buffer resize to zero
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(100);
        // Avoid blocking by enqueueing a zero-length datagram
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // With no data, buffer should resize to 0
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testBufferResizeToLarge()
    {
        // Buffer operations with large initial capacity
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(50000);
        const char* payload = "big";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
    }

    void testPeekWithVariousBufferSizes()
    {
        // Peek with various buffer sizes
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(10);
        DataBuffer buffer2(100);
        DataBuffer buffer3(1000);

        const char* payload = "abc";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes1 = socket.peekBytes(buffer1);
        std::size_t read1 = socket.readBytes(buffer1);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes2 = socket.peekBytes(buffer2);
        std::size_t read2 = socket.readBytes(buffer2);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes3 = socket.peekBytes(buffer3);
        std::size_t read3 = socket.readBytes(buffer3);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytes1);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytes2);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytes3);
        CPPUNIT_ASSERT_EQUAL(bytes1, read1);
        CPPUNIT_ASSERT_EQUAL(bytes2, read2);
        CPPUNIT_ASSERT_EQUAL(bytes3, read3);
    }

    void testReadWithVariousBufferSizes()
    {
        // Read with various buffer sizes
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(10);
        DataBuffer buffer2(100);

        const char* payload = "read";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes1 = socket.readBytes(buffer1);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytes2 = socket.readBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytes1);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytes2);
        CPPUNIT_ASSERT(std::memcmp(buffer1.data(), payload, bytes1) == 0);
        CPPUNIT_ASSERT(std::memcmp(buffer2.data(), payload, bytes2) == 0);
    }

    void testSocketStateAfterException()
    {
        // Verify clean state after exception
        try {
            UnixSocket socket("/invalid/path.sock");
        }
        catch (const UnixSocket::SocketException&) {
            // Exception caught, state should be clean
        }

        // Can create valid socket after exception
        std::string path = m_testSocketPath;
        UnixSocket socket(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testSocketCreationTiming()
    {
        // Multiple socket creations should complete quickly
        for (int i = 0; i < 5; i++) {
            std::string path = generateTestSocketPath();
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
            cleanupSocketFile(path);
        }
    }

    void testBufferDataIntegrityAfterPeek()
    {
        // Buffer data integrity after peek
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(200);

        const char* payload = "integrity";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytesRead = socket.peekBytes(buffer);

        // Buffer should be in valid state
        CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testBufferDataIntegrityAfterRead()
    {
        // Buffer data integrity after read
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(200);

        const char* payload = "integrity";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytesRead = socket.readBytes(buffer);

        // Buffer should be in valid state
        CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        CPPUNIT_ASSERT(std::memcmp(buffer.data(), payload, bytesRead) == 0);
    }

    void testSocketValidityAfterOperations()
    {
        // Socket remains valid after multiple operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Perform multiple operations
        size_t size1 = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(size1 > 0);

        size_t size2 = socket.getSocketBufferSize();
        CPPUNIT_ASSERT_EQUAL(size1, size2);

        DataBuffer buffer(100);

        // Operations should not invalidate socket
        const char* payload = "valid";
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t bytesRead = socket.peekBytes(buffer);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);

        size_t size3 = socket.getSocketBufferSize();
        CPPUNIT_ASSERT_EQUAL(size1, size3);
    }

    void testBufferStateConsistency()
    {
        // Buffer maintains consistent state across operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer;
        size_t initialCapacity = buffer.capacity();
        const char* payload = "state";

        buffer.resize(512);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t peeked = socket.peekBytes(buffer);

        // Buffer should maintain reasonable state
        CPPUNIT_ASSERT(buffer.capacity() >= initialCapacity);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), peeked);

        buffer.resize(1024);
        enqueueDatagram(path, payload, strlen(payload));
        std::size_t read = socket.readBytes(buffer);

        // Buffer should still maintain reasonable state
        CPPUNIT_ASSERT(buffer.capacity() >= initialCapacity);
        CPPUNIT_ASSERT_EQUAL(strlen(payload), read);
    }

    void testHighFrequencyReadOperations()
    {
        // Test rapid successive read operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Send multiple datagrams rapidly using helper
        const int numMessages = 10;
        for (int i = 0; i < numMessages; i++) {
            std::string testData = "Message" + std::to_string(i);
            enqueueDatagram(path, testData.c_str(), testData.length());
        }

        sleep(1); // Allow messages to arrive

        // Perform high-frequency reads
        int messagesRead = 0;
        for (int i = 0; i < numMessages; i++) {
            DataBuffer buffer;
            buffer.resize(1024);
            try {
                size_t bytesRead = socket.readBytes(buffer);
                if (bytesRead > 0) {
                    messagesRead++;
                    CPPUNIT_ASSERT(buffer.size() > 0);
                }
            } catch (...) {
                break; // No more messages
            }
        }

        CPPUNIT_ASSERT(messagesRead > 0);
        unlink(path.c_str());
    }

    void testBufferReallocationsPattern()
    {
        // Test various buffer reallocation patterns
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer;
        const char* payload = "x";

        // Pattern 1: Growing buffer
        for (size_t size = 100; size <= 2000; size += 100) {
            buffer.resize(size);
            enqueueDatagram(path, payload, strlen(payload));
            std::size_t bytesRead = socket.peekBytes(buffer);
            CPPUNIT_ASSERT(buffer.capacity() >= size);
            CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        }

        // Pattern 2: Shrinking buffer
        for (size_t size = 2000; size >= 100; size -= 100) {
            buffer.resize(size);
            enqueueDatagram(path, payload, strlen(payload));
            std::size_t bytesRead = socket.peekBytes(buffer);
            // Capacity may not shrink, but should remain valid
            CPPUNIT_ASSERT(buffer.capacity() > 0);
            CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        }

        // Pattern 3: Random-like sizes
        size_t testSizes[] = {50, 500, 100, 1000, 200, 800};
        for (size_t size : testSizes) {
            buffer.resize(size);
            enqueueDatagram(path, payload, strlen(payload));
            std::size_t bytesRead = socket.readBytes(buffer);
            CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
            CPPUNIT_ASSERT_EQUAL(strlen(payload), bytesRead);
        }

        unlink(path.c_str());
    }

    void testMultipleSocketsDataIntegrity()
    {
        // Test data integrity with multiple sockets
        std::string path1 = m_testSocketPath;
        std::string path2 = m_testSocketPath2;

        UnixSocket socket1(path1);
        UnixSocket socket2(path2);

        // Send different data to each socket using helper

        // Send different data to each socket
        const char* data1 = "Socket1Data";
        const char* data2 = "Socket2Data";

        enqueueDatagram(path1, data1, strlen(data1));
        enqueueDatagram(path2, data2, strlen(data2));

        sleep(1);

        // Read from both sockets
        DataBuffer buffer1, buffer2;
        buffer1.resize(1024);
        buffer2.resize(1024);

        socket1.readBytes(buffer1);
        socket2.readBytes(buffer2);

        // Verify data integrity and independence
        CPPUNIT_ASSERT(buffer1.size() == strlen(data1));
        CPPUNIT_ASSERT(buffer2.size() == strlen(data2));
        CPPUNIT_ASSERT(memcmp(buffer1.data(), data1, strlen(data1)) == 0);
        CPPUNIT_ASSERT(memcmp(buffer2.data(), data2, strlen(data2)) == 0);

        unlink(path1.c_str());
        unlink(path2.c_str());
    }

    void testExceptionRecoveryPattern()
    {
        // Test recovery patterns after exceptions

        // Pattern 1: Exception during construction
        try {
            UnixSocket socket("/invalid/nonexistent/directory/socket.sock");
            CPPUNIT_FAIL("Should have thrown exception");
        } catch (const UnixSocket::SocketException& e) {
            CPPUNIT_ASSERT(e.what() != nullptr);
            CPPUNIT_ASSERT(strlen(e.what()) > 0);
        }

        // Pattern 2: Successful creation after exception
        std::string validPath = m_testSocketPath;
        UnixSocket socket(validPath);
        CPPUNIT_ASSERT(socketFileExists(validPath));

        // Pattern 3: Operations after exception
        size_t bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

        DataBuffer buffer;
        buffer.resize(1024);

        // Read on empty socket can block; avoid blocking using zero-length datagram
        enqueueZeroLengthDatagram(validPath);
        (void)socket.readBytes(buffer);

        // Should still be able to query buffer size
        bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

        unlink(validPath.c_str());
    }

    void testSocketLifecycleStressTest()
    {
        // Stress test complete socket lifecycle
        const int cycles = 15;

        for (int cycle = 0; cycle < cycles; cycle++) {
            std::string path = generateTestSocketPath();

            // Creation
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            // Query operations
            size_t bufferSize = socket.getSocketBufferSize();
            CPPUNIT_ASSERT(bufferSize > 0);

            // Send and receive
            std::string testData = "Cycle" + std::to_string(cycle);
            enqueueDatagram(path, testData.c_str(), testData.length());

            usleep(100000); // 100ms

            DataBuffer buffer;
            buffer.resize(1024);

            // Peek
            size_t peeked = socket.peekBytes(buffer);
            if (peeked > 0) {
                CPPUNIT_ASSERT(buffer.size() > 0);
            }
        }
    }

    void testComplexDataFlowScenario()
    {
        // Complex scenario: multiple operations in sequence
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Send first message using helper
        const char* msg1 = "FirstMessage";
        enqueueDatagram(path, msg1, strlen(msg1));
        sleep(1);

        // Peek first message
        DataBuffer buffer;
        buffer.resize(1024);
        size_t peeked = socket.peekBytes(buffer);
        CPPUNIT_ASSERT(peeked == strlen(msg1));

        // Query buffer size
        size_t bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

        // Peek again (should get same message)
        buffer.resize(1024);
        size_t peeked2 = socket.peekBytes(buffer);
        CPPUNIT_ASSERT_EQUAL(peeked, peeked2);

        // Read first message (removes it)
        buffer.resize(1024);
        size_t read1 = socket.readBytes(buffer);
        CPPUNIT_ASSERT_EQUAL(peeked, read1);
        CPPUNIT_ASSERT(memcmp(buffer.data(), msg1, strlen(msg1)) == 0);

        // Send second message using helper
        const char* msg2 = "SecondMessage";
        enqueueDatagram(path, msg2, strlen(msg2));
        sleep(1);

        // Read second message directly
        buffer.resize(1024);
        size_t read2 = socket.readBytes(buffer);
        CPPUNIT_ASSERT(read2 == strlen(msg2));
        CPPUNIT_ASSERT(memcmp(buffer.data(), msg2, strlen(msg2)) == 0);

        // Verify socket still operational
        bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

        unlink(path.c_str());
    }

    void testResourceCleanupVerification()
    {
        // Verify proper resource cleanup
        std::string path = m_testSocketPath;

        {
            // Scope 1: Create and use socket
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            size_t bufferSize = socket.getSocketBufferSize();
            CPPUNIT_ASSERT(bufferSize > 0);

            DataBuffer buffer;
            buffer.resize(1024);

            // Perform operations (avoid blocking by enqueueing before IO)
            const char* payload = "clean";
            enqueueDatagram(path, payload, strlen(payload));
            std::size_t peeked = socket.peekBytes(buffer);
            std::size_t read = socket.readBytes(buffer);
            CPPUNIT_ASSERT_EQUAL(strlen(payload), peeked);
            CPPUNIT_ASSERT_EQUAL(peeked, read);
            socket.shutdown();

            // Socket file should still exist
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        // Socket destroyed, but file remains

        // Create new socket with same path
        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            // Should be fully functional
            size_t bufferSize = socket.getSocketBufferSize();
            CPPUNIT_ASSERT(bufferSize > 0);
        }

        // Verify file still exists (unlink handled by constructor)
        CPPUNIT_ASSERT(socketFileExists(path));

        // Final cleanup
        unlink(path.c_str());
        CPPUNIT_ASSERT(!socketFileExists(path));
    }

private:
    std::string m_testSocketPath;
    std::string m_testSocketPath2;
    static int s_testCounter;
    static void enqueueZeroLengthDatagram(const std::string& path)
    {
        // Delegate to generic helper to avoid duplication
        enqueueDatagram(path, "", 0);
    }

    static void enqueueDatagram(const std::string& path, const void* data, size_t len)
    {
        int clientSock = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if (clientSock < 0) {
            return;
        }

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

        (void)::sendto(clientSock, data, len, 0, (struct sockaddr*)&addr, sizeof(addr));
        ::close(clientSock);
    }

    static void enqueueSingleByteDatagram(const std::string& path)
    {
        char b = 'x';
        enqueueDatagram(path, &b, 1);
    }

    std::string generateTestSocketPath()
    {
        return "/tmp/test_unixsocket_" + std::to_string(getpid()) +
               "_" + std::to_string(s_testCounter++) + ".sock";
    }

    void cleanupSocketFile(const std::string& path)
    {
        unlink(path.c_str());
    }

    bool socketFileExists(const std::string& path)
    {
        struct stat statbuf;
        return (stat(path.c_str(), &statbuf) == 0);
    }
};

int UnixSocketTest::s_testCounter = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(UnixSocketTest);
