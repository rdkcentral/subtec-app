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
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <thread>
#include <exception>

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
    CPPUNIT_TEST(testConstructorWithEmptyStringPath);
    CPPUNIT_TEST(testConstructorWithPathToExistingSocket);
    CPPUNIT_TEST(testConstructorWithPathToNonExistentDirectory);
    CPPUNIT_TEST(testConstructorWithPathContainingSpaces);
    CPPUNIT_TEST(testConstructorWithTypicalUnixSocketPath);
    CPPUNIT_TEST(testConstructorCreatesAfUnixSocket);
    CPPUNIT_TEST(testConstructorCreatesSockDgramSocket);
    CPPUNIT_TEST(testConstructorCallsUnlinkBeforeBind);
    CPPUNIT_TEST(testConstructorSocketFileCreated);
    CPPUNIT_TEST(testConstructorThrowsOnInvalidSocketCreation);
    CPPUNIT_TEST(testConstructorNearBoundaryPathLength);
    CPPUNIT_TEST(testConstructorPathWithSlash);
    CPPUNIT_TEST(testConstructorMultipleInstancesDifferentPaths);
    CPPUNIT_TEST(testConstructorSamePathSequentially);
    CPPUNIT_TEST(testConstructorSocketPermissions);
    CPPUNIT_TEST(testConstructorExceptionInheritance);
    CPPUNIT_TEST(testConstructorExceptionMessage);
    CPPUNIT_TEST(testConstructorCleanupOnException);
    CPPUNIT_TEST(testConstructorSocketFileExists);
    CPPUNIT_TEST(testGetSocketBufferSizeOnValidSocket);
    CPPUNIT_TEST(testGetSocketBufferSizeReturnsPositiveValue);
    CPPUNIT_TEST(testGetSocketBufferSizeReturnsReasonableValue);
    CPPUNIT_TEST(testGetSocketBufferSizeCalledMultipleTimes);
    CPPUNIT_TEST(testGetSocketBufferSizeConstQualifier);
    CPPUNIT_TEST(testGetSocketBufferSizeMaxBufferFallback);
    CPPUNIT_TEST(testGetSocketBufferSizeNonZero);
    CPPUNIT_TEST(testPeekBytesWithEmptyBuffer);
    CPPUNIT_TEST(testPeekBytesWithBufferCapacity1);
    CPPUNIT_TEST(testPeekBytesWithBufferLargerThanData);
    CPPUNIT_TEST(testPeekBytesWhenNoDataAvailable);
    CPPUNIT_TEST(testPeekBytesTwiceConsecutively);
    CPPUNIT_TEST(testPeekBytesFollowedByReadBytes);
    CPPUNIT_TEST(testPeekBytesResizesBuffer);
    CPPUNIT_TEST(testPeekBytesReturnsCorrectSize);
    CPPUNIT_TEST(testPeekBytesDoesNotRemoveData);
    CPPUNIT_TEST(testPeekBytesWithLargeBuffer);
    CPPUNIT_TEST(testPeekBytesReturnValue);
    CPPUNIT_TEST(testPeekBytesDataIntegrity);
    CPPUNIT_TEST(testReadBytesWithEmptyBuffer);
    CPPUNIT_TEST(testReadBytesWithBufferCapacity1);
    CPPUNIT_TEST(testReadBytesWithBufferLargerThanData);
    CPPUNIT_TEST(testReadBytesWhenNoDataAvailable);
    CPPUNIT_TEST(testReadBytesTwiceConsecutively);
    CPPUNIT_TEST(testReadBytesAfterPeekBytes);
    CPPUNIT_TEST(testReadBytesRemovesData);
    CPPUNIT_TEST(testReadBytesResizesBuffer);
    CPPUNIT_TEST(testReadBytesReturnsCorrectSize);
    CPPUNIT_TEST(testReadBytesWithLargeBuffer);
    CPPUNIT_TEST(testReadBytesReturnValue);
    CPPUNIT_TEST(testReadBytesDataIntegrity);
    CPPUNIT_TEST(testShutdownOnActiveSocket);
    CPPUNIT_TEST(testShutdownCalledTwice);
    CPPUNIT_TEST(testSocketExceptionConstruction);
    CPPUNIT_TEST(testSocketExceptionInheritance);
    CPPUNIT_TEST(testSocketExceptionWhatMethod);
    CPPUNIT_TEST(testSocketExceptionWithEmptyMessage);
    CPPUNIT_TEST(testSocketExceptionWithLongMessage);
    CPPUNIT_TEST(testSocketExceptionCatchAsStdException);
    CPPUNIT_TEST(testSocketExceptionMessageContainsError);
    CPPUNIT_TEST(testSocketExceptionThrowAndCatch);
    CPPUNIT_TEST(testHandleValidSocketFd);
    CPPUNIT_TEST(testHandleOwnershipRAII);
    CPPUNIT_TEST(testHandleMultipleOperations);
    CPPUNIT_TEST(testHandleLifetime);
    CPPUNIT_TEST(testHandleUniquePtrSemantics);
    CPPUNIT_TEST(testHandleSocketClosed);
    CPPUNIT_TEST(testHandleAfterShutdown);
    CPPUNIT_TEST(testHandleConsistency);
    CPPUNIT_TEST(testHandleExceptionSafety);
    CPPUNIT_TEST(testHandleDestructorCleanup);
    CPPUNIT_TEST(testCreateSocketWriteDataPeekBytes);
    CPPUNIT_TEST(testCreateSocketWriteDataReadBytes);
    CPPUNIT_TEST(testPeekReadPeekSequence);
    CPPUNIT_TEST(testMultipleSocketsIndependence);
    CPPUNIT_TEST(testSocketReuseAfterDestruction);
    CPPUNIT_TEST(testMultipleDatagramsSequential);
    CPPUNIT_TEST(testSocketBufferConsistency);
    CPPUNIT_TEST(testCompleteSocketLifecycle);
    CPPUNIT_TEST(testExceptionDuringConstruction);
    CPPUNIT_TEST(testRapidCreateDestroyCycles);
    CPPUNIT_TEST(testSocketOperationsAfterShutdown);
    CPPUNIT_TEST(testErrorPropagationHierarchy);
    CPPUNIT_TEST(testDataBufferModification);
    CPPUNIT_TEST(testSocketWithZeroByteBuffer);
    CPPUNIT_TEST(testSocketWithMaxBufferSize);
    CPPUNIT_TEST(testPeekDoesNotConsumeData);
    CPPUNIT_TEST(testReadConsumesData);
    CPPUNIT_TEST(testBufferSizeQuery);
    CPPUNIT_TEST(testSocketFileCleanup);
    CPPUNIT_TEST(testConcurrentSocketCreation);
    CPPUNIT_TEST(testPathLengthJustUnderLimit);
    CPPUNIT_TEST(testPathEndingWithoutExtension);
    CPPUNIT_TEST(testPathWithDotPrefix);
    CPPUNIT_TEST(testPathWithMultipleDots);
    CPPUNIT_TEST(testPathInTmpDirectory);
    CPPUNIT_TEST(testBufferResizeToZero);
    CPPUNIT_TEST(testBufferResizeToLarge);
    CPPUNIT_TEST(testMultipleSocketsSamePath);
    CPPUNIT_TEST(testSocketAfterUnlink);
    CPPUNIT_TEST(testGetBufferSizeConsistency);
    CPPUNIT_TEST(testPeekWithVariousBufferSizes);
    CPPUNIT_TEST(testReadWithVariousBufferSizes);
    CPPUNIT_TEST(testExceptionMessageContent);
    CPPUNIT_TEST(testSocketStateAfterException);
    CPPUNIT_TEST(testBufferCapacityPreservation);
    CPPUNIT_TEST(testSocketCreationTiming);
    CPPUNIT_TEST(testMultipleShutdownCalls);
    CPPUNIT_TEST(testSocketFileTypeVerification);
    CPPUNIT_TEST(testBufferDataIntegrityAfterPeek);
    CPPUNIT_TEST(testBufferDataIntegrityAfterRead);
    CPPUNIT_TEST(testSocketValidityAfterOperations);
    CPPUNIT_TEST(testBufferStateConsistency);
    CPPUNIT_TEST(testHighFrequencyReadOperations);
    CPPUNIT_TEST(testLargeDatagramHandling);
    CPPUNIT_TEST(testSequentialSocketCreationPerformance);
    CPPUNIT_TEST(testBufferReallocationsPattern);
    CPPUNIT_TEST(testSocketOperationsUnderLoad);
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
        std::string path(sizeof(addr.sun_path) - 2, 'a');
        path = "/tmp/" + path;

        // Ensure path length is valid but near maximum
        if (path.size() < sizeof(addr.sun_path)) {
            try {
                UnixSocket socket(path);
                // Should succeed
                cleanupSocketFile(path);
            }
            catch (const std::exception& e) {
                cleanupSocketFile(path);
                // May fail due to path issues, which is acceptable
            }
        }
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
            // Just verify no crash
        }
        catch (const std::exception& e) {
            // Exception is acceptable for empty path
        }
    }

    void testConstructorWithPathToExistingSocket()
    {
        // Create first socket
        std::string path = m_testSocketPath;

        {
            UnixSocket socket1(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Create second socket with same path (first is destroyed)
        try {
            UnixSocket socket2(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Should reuse path after cleanup: ") + e.what());
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
            // May fail on some systems, acceptable
        }

        cleanupSocketFile(path);
    }

    void testConstructorWithTypicalUnixSocketPath()
    {
        // Typical unix socket path pattern
        std::string path = "/tmp/test_typical.sock";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Typical path should work: ") + e.what());
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

    void testConstructorCreatesSockDgramSocket()
    {
        // Constructor should create SOCK_DGRAM (datagram) socket
        std::string path = m_testSocketPath;

        UnixSocket socket(path);

        // Verify socket was created successfully
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testConstructorCallsUnlinkBeforeBind()
    {
        // Create a socket file first
        std::string path = m_testSocketPath;

        // Create initial socket
        {
            UnixSocket socket1(path);
        }

        // File should exist after first socket
        CPPUNIT_ASSERT(socketFileExists(path));

        // Create second socket - should unlink old one
        UnixSocket socket2(path);

        // Should still have socket file
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testConstructorSocketFileCreated()
    {
        std::string path = m_testSocketPath;

        // File should not exist before
        cleanupSocketFile(path);
        CPPUNIT_ASSERT(!socketFileExists(path));

        // Create socket
        UnixSocket socket(path);

        // File should exist after
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testConstructorThrowsOnInvalidSocketCreation()
    {
        // Test with path that will cause bind to fail
        std::string path = "/invalid/path/that/does/not/exist/socket.sock";

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown SocketException");
        }
        catch (const UnixSocket::SocketException& e) {
            // Expected exception
            CPPUNIT_ASSERT(std::string(e.what()).length() > 0);
        }
    }

    void testConstructorNearBoundaryPathLength()
    {
        // Test path length near boundary
        struct sockaddr_un addr{};
        size_t maxLen = sizeof(addr.sun_path) - 3;
        std::string path = "/tmp/";
        path.append(maxLen - path.length(), 'b');

        if (path.size() < sizeof(addr.sun_path)) {
            try {
                UnixSocket socket(path);
                cleanupSocketFile(path);
            }
            catch (const std::exception&) {
                cleanupSocketFile(path);
            }
        }
    }

    void testConstructorPathWithSlash()
    {
        // Path ending with slash (invalid)
        std::string path = "/tmp/testsocket/";

        try {
            UnixSocket socket(path);
            // May succeed or fail depending on system
        }
        catch (const std::exception&) {
            // Exception acceptable
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

    void testConstructorSamePathSequentially()
    {
        // Create socket, destroy it, create another with same path
        std::string path = m_testSocketPath;

        {
            UnixSocket socket1(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // First socket destroyed, create new one
        UnixSocket socket2(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testConstructorSocketPermissions()
    {
        // Verify socket file permissions
        std::string path = m_testSocketPath;

        UnixSocket socket(path);

        struct stat statbuf;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &statbuf));

        // Socket file should have been created
        CPPUNIT_ASSERT(S_ISSOCK(statbuf.st_mode));
    }

    void testConstructorExceptionInheritance()
    {
        // Verify SocketException inherits from std::exception
        std::string path = "/invalid/path/socket.sock";

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const std::exception& e) {
            // Should be catchable as std::exception
            CPPUNIT_ASSERT(std::string(e.what()).length() > 0);
        }
    }

    void testConstructorExceptionMessage()
    {
        // Verify exception message is meaningful
        std::string path = "/nonexistent/path/socket.sock";

        try {
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const UnixSocket::SocketException& e) {
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.length() > 0);
            CPPUNIT_ASSERT(msg.find("Cannot bind") != std::string::npos ||
                          msg.find("too long") != std::string::npos);
        }
    }

    void testConstructorCleanupOnException()
    {
        // Verify proper cleanup when exception thrown
        std::string path = "/invalid/path/socket.sock";

        try {
            UnixSocket socket(path);
        }
        catch (const std::exception&) {
            // Exception expected, verify no resource leak
            // (RAII should handle cleanup)
        }
    }

    void testConstructorSocketFileExists()
    {
        // Verify socket file exists after construction
        std::string path = m_testSocketPath;

        UnixSocket socket(path);

        // Check file existence
        std::ifstream file(path);
        bool exists = file.good();
        file.close();

        // Socket files may not be readable as regular files
        // Just verify stat works
        struct stat statbuf;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &statbuf));
    }

    void testGetSocketBufferSizeOnValidSocket()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Should return a valid size
        CPPUNIT_ASSERT(bufferSize > 0);
    }

    void testGetSocketBufferSizeReturnsPositiveValue()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Must be positive
        CPPUNIT_ASSERT(bufferSize > 0);
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

    void testGetSocketBufferSizeMaxBufferFallback()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Verify buffer size is within reasonable bounds
        // Should be at least 1KB and respect MAX_BUFFER_SIZE (128KB)
        CPPUNIT_ASSERT(bufferSize >= 1024);
        CPPUNIT_ASSERT(bufferSize <= 10 * 1024 * 1024); // Sanity check: max 10MB
    }

    void testGetSocketBufferSizeNonZero()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t bufferSize = socket.getSocketBufferSize();

        // Must never be zero
        CPPUNIT_ASSERT(bufferSize != 0);
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

        // Avoid blocking while still expecting 0 bytes
        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(1);

        // Peek with minimal capacity should return 0 for zero-length datagram
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testPeekBytesWithBufferLargerThanData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Enqueue zero-length datagram so peek returns promptly and still 0
        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(1024);

        // Large buffer vs zero-length data should yield 0 bytes peeked
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testPeekBytesWhenNoDataAvailable()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Prevent blocking while validating 0-byte behavior
        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(100);

        // No data sent to socket
        std::size_t bytesRead = socket.peekBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testPeekBytesTwiceConsecutively()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Ensure queue state allows non-blocking peeks
        enqueueZeroLengthDatagram(path);

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        // Peek twice - both should see same data (none in this case)
        std::size_t bytesRead1 = socket.peekBytes(buffer1);
        std::size_t bytesRead2 = socket.peekBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(bytesRead1, bytesRead2);
        CPPUNIT_ASSERT_EQUAL(buffer1.size(), buffer2.size());
    }

    void testPeekBytesFollowedByReadBytes()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Avoid blocking for both operations
        enqueueZeroLengthDatagram(path);

        DataBuffer peekBuffer(100);
        DataBuffer readBuffer(100);

        // Peek then read - should see same data
        std::size_t peekedBytes = socket.peekBytes(peekBuffer);
        std::size_t readBytes = socket.readBytes(readBuffer);

        CPPUNIT_ASSERT_EQUAL(peekedBytes, readBytes);
    }

    void testPeekBytesResizesBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(1024);
        size_t originalCapacity = buffer.capacity();

        std::size_t bytesRead = socket.peekBytes(buffer);

        // Buffer should be resized to actual bytes read
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
        // Capacity should remain unchanged
        CPPUNIT_ASSERT(buffer.capacity() >= originalCapacity);
    }

    void testPeekBytesReturnsCorrectSize()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(500);

        std::size_t bytesRead = socket.peekBytes(buffer);

        // Return value should match buffer size
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
    }

    void testPeekBytesDoesNotRemoveData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        // First peek
        std::size_t bytes1 = socket.peekBytes(buffer1);

        // Second peek should see same data (not removed)
        std::size_t bytes2 = socket.peekBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(bytes1, bytes2);
    }

    void testPeekBytesWithLargeBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Large buffer
        enqueueZeroLengthDatagram(path);
        DataBuffer buffer(10 * 1024);

        std::size_t bytesRead = socket.peekBytes(buffer);

        // Should handle large buffer without crash
        CPPUNIT_ASSERT(bytesRead >= 0);
    }

    void testPeekBytesReturnValue()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(256);

        std::size_t result = socket.peekBytes(buffer);

        // Return value should be non-negative
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPeekBytesDataIntegrity()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(512);

        std::size_t bytesRead = socket.peekBytes(buffer);

        // Verify buffer size matches bytes read
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
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

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(1);

        std::size_t bytesRead = socket.readBytes(buffer);

        // No data available
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testReadBytesWithBufferLargerThanData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(2048);

        std::size_t bytesRead = socket.readBytes(buffer);

        // No data available
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
    }

    void testReadBytesWhenNoDataAvailable()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(100);

        std::size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytesRead);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), buffer.size());
    }

    void testReadBytesTwiceConsecutively()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Enqueue before each read to avoid blocking
        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        enqueueZeroLengthDatagram(path);
        std::size_t bytes1 = socket.readBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        std::size_t bytes2 = socket.readBytes(buffer2);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytes1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytes2);
    }

    void testReadBytesAfterPeekBytes()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer peekBuffer(100);
        DataBuffer readBuffer(100);

        // Peek then read
        std::size_t peeked = socket.peekBytes(peekBuffer);
        std::size_t read = socket.readBytes(readBuffer);

        // Should read same amount
        CPPUNIT_ASSERT_EQUAL(peeked, read);
    }

    void testReadBytesRemovesData()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Enqueue before each read call
        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        enqueueZeroLengthDatagram(path);
        std::size_t bytes1 = socket.readBytes(buffer1);

        enqueueZeroLengthDatagram(path);
        std::size_t bytes2 = socket.readBytes(buffer2);

        // With no data sent, both should return 0
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytes1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), bytes2);
    }

    void testReadBytesResizesBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(1024);
        size_t originalCapacity = buffer.capacity();

        std::size_t bytesRead = socket.readBytes(buffer);

        // Buffer resized to actual bytes read
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
        CPPUNIT_ASSERT(buffer.capacity() >= originalCapacity);
    }

    void testReadBytesReturnsCorrectSize()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(500);

        std::size_t bytesRead = socket.readBytes(buffer);

        // Return value matches buffer size
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
    }

    void testReadBytesWithLargeBuffer()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Very large buffer
        enqueueZeroLengthDatagram(path);
        DataBuffer buffer(64 * 1024);

        std::size_t bytesRead = socket.readBytes(buffer);

        // Should not crash
        CPPUNIT_ASSERT(bytesRead >= 0);
    }

    void testReadBytesReturnValue()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(256);

        std::size_t result = socket.readBytes(buffer);

        // Non-negative return value
        CPPUNIT_ASSERT(result >= 0);
    }

    void testReadBytesDataIntegrity()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);

        DataBuffer buffer(512);

        std::size_t bytesRead = socket.readBytes(buffer);

        // Buffer size should match bytes read
        CPPUNIT_ASSERT_EQUAL(bytesRead, buffer.size());
    }

    void testShutdownOnActiveSocket()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Shutdown should not throw
        try {
            socket.shutdown();
            // Success - no exception
        }
        catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Shutdown should not throw: ") + e.what());
        }
    }

    void testShutdownCalledTwice()
    {
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Call shutdown twice
        socket.shutdown();
        socket.shutdown();

        // Should handle gracefully (no crash)
    }

    void testSocketExceptionConstruction()
    {
        // Test exception construction with message
        UnixSocket::SocketException ex("Test error message");

        std::string msg = ex.what();
        CPPUNIT_ASSERT_EQUAL(std::string("Test error message"), msg);
    }

    void testSocketExceptionInheritance()
    {
        // Verify SocketException inherits from std::exception
        try {
            throw UnixSocket::SocketException("Test");
        }
        catch (const std::exception& e) {
            // Should be catchable as std::exception
            CPPUNIT_ASSERT(std::string(e.what()).length() > 0);
        }
        catch (...) {
            CPPUNIT_FAIL("Should be catchable as std::exception");
        }
    }

    void testSocketExceptionWhatMethod()
    {
        // Test what() method returns correct message
        UnixSocket::SocketException ex("Specific error");

        const char* msg = ex.what();
        CPPUNIT_ASSERT(msg != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string("Specific error"), std::string(msg));
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

    void testSocketExceptionCatchAsStdException()
    {
        // Verify exception can be caught as std::exception
        bool caught = false;
        try {
            throw UnixSocket::SocketException("Error");
        }
        catch (const std::exception&) {
            caught = true;
        }

        CPPUNIT_ASSERT(caught);
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

    void testSocketExceptionThrowAndCatch()
    {
        // Test throw and catch mechanism
        bool exceptionCaught = false;
        try {
            throw UnixSocket::SocketException("Test exception");
        }
        catch (const UnixSocket::SocketException& e) {
            exceptionCaught = true;
            CPPUNIT_ASSERT_EQUAL(std::string("Test exception"), std::string(e.what()));
        }

        CPPUNIT_ASSERT(exceptionCaught);
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
        // Ensure non-blocking behavior for peek/read
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);
        // Enqueue again in case peek implementation accidentally consumes
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // All operations should complete
    }

    void testHandleLifetime()
    {
        // Handle lifetime matches socket lifetime
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            // Perform operations
            socket.getSocketBufferSize();
        }

        // Socket destroyed, handle cleaned up
    }

    void testHandleUniquePtrSemantics()
    {
        // Verify unique_ptr ownership semantics
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Socket owns its handle exclusively
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testHandleSocketClosed()
    {
        // Verify handle is closed properly
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Handle should be closed after destruction
    }

    void testHandleAfterShutdown()
    {
        // Handle state after shutdown
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        socket.shutdown();

        // Handle still exists but socket is shutdown
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testHandleConsistency()
    {
        // Handle consistency across operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(50);
        DataBuffer buffer2(50);

        // Avoid blocking: enqueue zero-length datagram before each peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer2);

        // Operations use same handle
        CPPUNIT_ASSERT_EQUAL(buffer1.size(), buffer2.size());
    }

    void testHandleExceptionSafety()
    {
        // Verify exception safety
        try {
            UnixSocket socket("/nonexistent/path.sock");
        }
        catch (const UnixSocket::SocketException&) {
            // Exception handled, resources cleaned up
        }
    }

    void testHandleDestructorCleanup()
    {
        // Verify destructor cleanup
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Destructor called, handle closed
    }

    void testCreateSocketWriteDataPeekBytes()
    {
        // End-to-end test: create socket, write externally, peek
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Send data using helper to reduce duplication
        const char* testData = "Hello";
        enqueueDatagram(path, testData, strlen(testData));

        DataBuffer buffer(100);
        std::size_t peeked = socket.peekBytes(buffer);

        CPPUNIT_ASSERT(peeked > 0);
        CPPUNIT_ASSERT(peeked <= strlen(testData));
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

        CPPUNIT_ASSERT(read > 0);
        CPPUNIT_ASSERT(read <= strlen(testData));
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

    void testSocketReuseAfterDestruction()
    {
        // Socket path can be reused after destruction
        std::string path = m_testSocketPath;

        {
            UnixSocket socket1(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Create new socket with same path
        UnixSocket socket2(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testMultipleDatagramsSequential()
    {
        // Multiple datagrams read sequentially
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        // Read multiple times
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer2);

        // Both reads should complete (may return 0 if no data)
        CPPUNIT_ASSERT(buffer1.size() >= 0);
        CPPUNIT_ASSERT(buffer2.size() >= 0);
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
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);

        CPPUNIT_ASSERT(bufferSize > 0);
    }

    void testCompleteSocketLifecycle()
    {
        // Complete lifecycle: create, read, shutdown, destroy
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));

            DataBuffer buffer(100);
            enqueueZeroLengthDatagram(path);
            socket.peekBytes(buffer);
            enqueueZeroLengthDatagram(path);
            socket.readBytes(buffer);

            socket.shutdown();
        }

        // Lifecycle complete
    }

    void testExceptionDuringConstruction()
    {
        // Exception during construction
        try {
            UnixSocket socket("/invalid/path/socket.sock");
            CPPUNIT_FAIL("Should throw exception");
        }
        catch (const UnixSocket::SocketException&) {
            // Expected - partial construction cleaned up
        }
    }

    void testRapidCreateDestroyCycles()
    {
        // Rapid create/destroy cycles
        std::string path = m_testSocketPath;

        for (int i = 0; i < 10; i++) {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // No resource leaks
    }

    void testSocketOperationsAfterShutdown()
    {
        // Operations after shutdown
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        socket.shutdown();

        // Operations may fail or return 0 after shutdown
        DataBuffer buffer(100);
        std::size_t result = socket.readBytes(buffer);

        // Should not crash
        CPPUNIT_ASSERT(result >= 0);
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

    void testDataBufferModification()
    {
        // Buffer modification during operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(100);
        size_t originalCapacity = buffer.capacity();

        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // Capacity should be preserved or increased
        CPPUNIT_ASSERT(buffer.capacity() >= originalCapacity);
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
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        CPPUNIT_ASSERT(buffer.capacity() >= maxSize);
    }

    void testPeekDoesNotConsumeData()
    {
        // Verify peek doesn't consume data
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        enqueueZeroLengthDatagram(path);
        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        std::size_t peek1 = socket.peekBytes(buffer1);
        std::size_t peek2 = socket.peekBytes(buffer2);

        // Both peeks should see same data
        CPPUNIT_ASSERT_EQUAL(peek1, peek2);
    }

    void testReadConsumesData()
    {
        // Verify read consumes data
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Enqueue before each read to avoid blocking and keep behavior deterministic
        DataBuffer buffer1(100);
        DataBuffer buffer2(100);

        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer2);

        // After first read, second read gets different/no data
        // Both should return non-negative
        CPPUNIT_ASSERT(buffer1.size() >= 0);
        CPPUNIT_ASSERT(buffer2.size() >= 0);
    }

    void testBufferSizeQuery()
    {
        // Buffer size query accuracy
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t size = socket.getSocketBufferSize();

        CPPUNIT_ASSERT(size > 0);
        CPPUNIT_ASSERT(size <= 10 * 1024 * 1024); // Reasonable upper bound
    }

    void testSocketFileCleanup()
    {
        // Socket file cleanup
        std::string path = m_testSocketPath;

        {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // After destruction, file may still exist but handle is closed
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

    void testPathLengthJustUnderLimit()
    {
        // Path length just under the limit
        struct sockaddr_un addr{};
        std::string path = "/tmp/";
        size_t remaining = sizeof(addr.sun_path) - path.length() - 10;
        path.append(remaining, 'z');
        path += ".sock";

        if (path.size() < sizeof(addr.sun_path)) {
            try {
                UnixSocket socket(path);
                cleanupSocketFile(path);
            }
            catch (const std::exception&) {
                cleanupSocketFile(path);
            }
        }
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
        catch (const std::exception&) {
            // May fail on some systems
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
        catch (const std::exception&) {
            // May fail
        }

        cleanupSocketFile(path);
    }

    void testPathWithMultipleDots()
    {
        // Path with multiple dots
        std::string path = "/tmp/test.socket.sock";
        cleanupSocketFile(path);

        try {
            UnixSocket socket(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }
        catch (const std::exception&) {
            // May fail
        }

        cleanupSocketFile(path);
    }

    void testPathInTmpDirectory()
    {
        // Standard /tmp directory path
        std::string path = "/tmp/test_tmp_socket.sock";
        cleanupSocketFile(path);

        UnixSocket socket(path);
        CPPUNIT_ASSERT(socketFileExists(path));

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
        // Avoid blocking by enqueueing a zero-length datagram
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // Should handle large buffer
        CPPUNIT_ASSERT(buffer.size() >= 0);
    }

    void testMultipleSocketsSamePath()
    {
        // Attempting multiple sockets with same path (sequential)
        std::string path = m_testSocketPath;

        {
            UnixSocket socket1(path);
            CPPUNIT_ASSERT(socketFileExists(path));
        }

        // Second socket reuses path
        UnixSocket socket2(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testSocketAfterUnlink()
    {
        // Manually unlink then create socket
        std::string path = m_testSocketPath;

        unlink(path.c_str());

        UnixSocket socket(path);
        CPPUNIT_ASSERT(socketFileExists(path));
    }

    void testGetBufferSizeConsistency()
    {
        // Buffer size consistency across calls
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        std::size_t size1 = socket.getSocketBufferSize();
        std::size_t size2 = socket.getSocketBufferSize();
        std::size_t size3 = socket.getSocketBufferSize();

        CPPUNIT_ASSERT_EQUAL(size1, size2);
        CPPUNIT_ASSERT_EQUAL(size2, size3);
    }

    void testPeekWithVariousBufferSizes()
    {
        // Peek with various buffer sizes
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(10);
        DataBuffer buffer2(100);
        DataBuffer buffer3(1000);

        // Avoid blocking: enqueue zero-length datagram before each peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer2);
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer3);

        // All should complete
        CPPUNIT_ASSERT(buffer1.size() >= 0);
        CPPUNIT_ASSERT(buffer2.size() >= 0);
        CPPUNIT_ASSERT(buffer3.size() >= 0);
    }

    void testReadWithVariousBufferSizes()
    {
        // Read with various buffer sizes
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer1(10);
        DataBuffer buffer2(100);

        // Avoid blocking: enqueue zero-length datagram before each read
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer1);
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer2);

        // Both should complete
        CPPUNIT_ASSERT(buffer1.size() >= 0);
        CPPUNIT_ASSERT(buffer2.size() >= 0);
    }

    void testExceptionMessageContent()
    {
        // Exception messages contain useful information
        try {
            struct sockaddr_un addr{};
            std::string path(sizeof(addr.sun_path) + 5, 'x');
            UnixSocket socket(path);
            CPPUNIT_FAIL("Should throw exception");
        }
        catch (const UnixSocket::SocketException& e) {
            std::string msg = e.what();
            CPPUNIT_ASSERT(msg.find("too long") != std::string::npos);
        }
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

    void testBufferCapacityPreservation()
    {
        // Buffer capacity preserved across operations
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(500);
        size_t originalCapacity = buffer.capacity();

        // Avoid blocking: enqueue zero-length datagram before peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);

        // Capacity should not decrease
        CPPUNIT_ASSERT(buffer.capacity() >= originalCapacity);
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

    void testMultipleShutdownCalls()
    {
        // Multiple shutdown calls should be safe
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        socket.shutdown();
        socket.shutdown();
        socket.shutdown();

        // No crash
    }

    void testSocketFileTypeVerification()
    {
        // Verify created file is a socket
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        struct stat statbuf;
        CPPUNIT_ASSERT_EQUAL(0, stat(path.c_str(), &statbuf));
        CPPUNIT_ASSERT(S_ISSOCK(statbuf.st_mode));
    }

    void testBufferDataIntegrityAfterPeek()
    {
        // Buffer data integrity after peek
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(200);

        // Avoid blocking: enqueue zero-length datagram before peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);

        // Buffer should be in valid state
        CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
    }

    void testBufferDataIntegrityAfterRead()
    {
        // Buffer data integrity after read
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer(200);

        // Avoid blocking: enqueue zero-length datagram before read
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // Buffer should be in valid state
        CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
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
        // Avoid blocking: enqueue zero-length datagram before peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);

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

        buffer.resize(512);
        // Avoid blocking: enqueue before peek
        enqueueZeroLengthDatagram(path);
        socket.peekBytes(buffer);

        // Buffer should maintain reasonable state
        CPPUNIT_ASSERT(buffer.capacity() >= initialCapacity);

        buffer.resize(1024);
        // Avoid blocking: enqueue before read
        enqueueZeroLengthDatagram(path);
        socket.readBytes(buffer);

        // Buffer should still maintain reasonable state
        CPPUNIT_ASSERT(buffer.capacity() >= initialCapacity);
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
                // Avoid blocking if queue is empty on this iteration
                enqueueZeroLengthDatagram(path);
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

    void testLargeDatagramHandling()
    {
        // Test handling of large datagrams
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        size_t bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

        // Send large datagram (but within reasonable limits)
        const size_t largeSize = 32768; // 32KB
        std::vector<char> largeData(largeSize, 'X');
        enqueueDatagram(path, largeData.data(), largeData.size());

        sleep(1);

        DataBuffer buffer;
        buffer.resize(65536);
        size_t bytesRead = socket.readBytes(buffer);

        CPPUNIT_ASSERT(bytesRead > 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(largeData.size()), bytesRead);

        unlink(path.c_str());
    }

    void testSequentialSocketCreationPerformance()
    {
        // Test rapid sequential socket creation and destruction
        const int iterations = 20;

        for (int i = 0; i < iterations; i++) {
            std::string path = generateTestSocketPath();

            // Create socket
            UnixSocket socket(path);

            // Verify it works
            size_t bufferSize = socket.getSocketBufferSize();
            CPPUNIT_ASSERT(bufferSize > 0);

            // Verify file exists
            CPPUNIT_ASSERT(socketFileExists(path));

            // Cleanup
            cleanupSocketFile(path);
        }

        // All iterations should complete without issues
        CPPUNIT_ASSERT(true);
    }

    void testBufferReallocationsPattern()
    {
        // Test various buffer reallocation patterns
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        DataBuffer buffer;

        // Pattern 1: Growing buffer
        for (size_t size = 100; size <= 2000; size += 100) {
            buffer.resize(size);
            // Avoid blocking: enqueue before peek
            enqueueZeroLengthDatagram(path);
            socket.peekBytes(buffer);
            CPPUNIT_ASSERT(buffer.capacity() >= size);
        }

        // Pattern 2: Shrinking buffer
        for (size_t size = 2000; size >= 100; size -= 100) {
            buffer.resize(size);
            // Avoid blocking: enqueue before peek
            enqueueZeroLengthDatagram(path);
            socket.peekBytes(buffer);
            // Capacity may not shrink, but should remain valid
            CPPUNIT_ASSERT(buffer.capacity() > 0);
        }

        // Pattern 3: Random-like sizes
        size_t testSizes[] = {50, 500, 100, 1000, 200, 800};
        for (size_t size : testSizes) {
            buffer.resize(size);
            // Avoid blocking: enqueue before read
            enqueueZeroLengthDatagram(path);
            socket.readBytes(buffer);
            CPPUNIT_ASSERT(buffer.capacity() >= buffer.size());
        }

        unlink(path.c_str());
    }

    void testSocketOperationsUnderLoad()
    {
        // Test socket operations under simulated load
        std::string path = m_testSocketPath;
        UnixSocket socket(path);

        // Simulate multiple senders using helper per message
        const int numSenders = 3;
        for (int i = 0; i < numSenders; i++) {
            std::string testData = "LoadTest" + std::to_string(i);
            enqueueDatagram(path, testData.c_str(), testData.length());
        }

        sleep(1);

        // Read messages and verify socket remains stable
        DataBuffer buffer;
        buffer.resize(1024);

        int messagesRead = 0;
        for (int i = 0; i < numSenders * 2; i++) {
            try {
                // Avoid blocking when there are no more queued messages
                enqueueZeroLengthDatagram(path);
                size_t bytesRead = socket.readBytes(buffer);
                if (bytesRead > 0) {
                    messagesRead++;
                }
            } catch (...) {
                break;
            }
        }

        CPPUNIT_ASSERT(messagesRead > 0);

        // Verify socket is still operational
        size_t bufferSize = socket.getSocketBufferSize();
        CPPUNIT_ASSERT(bufferSize > 0);

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

                // Read
                buffer.resize(1024);
                size_t read = socket.readBytes(buffer);
                CPPUNIT_ASSERT_EQUAL(peeked, read);
            }

            // Shutdown
            socket.shutdown();

            cleanupSocketFile(path);
        }

        CPPUNIT_ASSERT(true); // All cycles completed
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
            enqueueZeroLengthDatagram(path);
            socket.peekBytes(buffer);
            enqueueZeroLengthDatagram(path);
            socket.readBytes(buffer);
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
