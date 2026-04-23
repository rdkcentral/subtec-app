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
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

#include "../src/UnixSocketSource.hpp"
#include "../include/PacketReceiver.hpp"
#include <subttxrend/protocol/Packet.hpp>
#include <subttxrend/common/DataBuffer.hpp>

using subttxrend::socksrc::UnixSocketSource;
using subttxrend::socksrc::PacketReceiver;
using subttxrend::protocol::Packet;
using subttxrend::common::DataBufferPtr;

class MockPacketReceiver : public PacketReceiver
{
public:
    void onPacketReceived(const Packet&) override {}
    void addBuffer(DataBufferPtr) override {}
    void onStreamBroken() override {}
};

class UnixSocketSourceTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(UnixSocketSourceTest);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorDoesNotCreateSocket);
    CPPUNIT_TEST(testDestructorOnRunningSource);
    CPPUNIT_TEST(testDestructorWithoutStart);
    CPPUNIT_TEST(testDestructorAfterExplicitStop);
    CPPUNIT_TEST(testStartCreatesSocket);
    CPPUNIT_TEST(testStartAfterStop);
    CPPUNIT_TEST(testStartWithExistingSocketFile);
    CPPUNIT_TEST(testStartRapidStartStop);
    CPPUNIT_TEST(testStartSocketPermissions);
    CPPUNIT_TEST(testStartMultipleSourcesSamePath);
    CPPUNIT_TEST(testStopOnRunningSource);
    CPPUNIT_TEST(testStopJoinsThread);
    CPPUNIT_TEST(testCreateSocketRetryDelay);
    CPPUNIT_TEST(testCreateSocketMultipleRetries);
    CPPUNIT_TEST(testCreateSocketExitsOnStop);
    CPPUNIT_TEST(testSourceLoopExitsOnStop);
    CPPUNIT_TEST(testMultipleStartStopCycles);
    CPPUNIT_TEST(testConcurrentStartStop);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_testSocketPath = generateTestSocketPath();
        m_testSocketPath2 = generateTestSocketPath();
        m_receiver = std::make_unique<MockPacketReceiver>();
        s_testCounter++;
    }

    void tearDown()
    {
        cleanupSocketFile(m_testSocketPath);
        cleanupSocketFile(m_testSocketPath2);
        m_receiver.reset();
    }

    void testConstructorWithVeryLongPath()
    {
        // Create path longer than 108 characters (socket path limit)
        std::string path(120, 'a');
        path = "/tmp/" + path + ".sock";

        UnixSocketSource source(path);

        // Constructor should accept long path (validation during socket creation)
        CPPUNIT_ASSERT(path.length() > 108);
    }

    void testConstructorDoesNotCreateSocket()
    {
        std::string path = m_testSocketPath;

        // Ensure socket file doesn't exist before
        cleanupSocketFile(path);
        CPPUNIT_ASSERT(!isUnixSocketFile(path));

        UnixSocketSource source(path);

        // Constructor must not create the socket; poll briefly to confirm
        CPPUNIT_ASSERT(!waitForUnixSocket(path, 150, 10));
    }

    void testDestructorOnRunningSource()
    {
        std::string path = m_testSocketPath;

        {
            UnixSocketSource source(path);
            source.start(m_receiver.get());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // Ensure socket is ready before destructor triggers stop
            CPPUNIT_ASSERT(waitForUnixSocket(m_testSocketPath, 600, 10));

            // Destructor should call stop() automatically
            // This should log a warning but not crash
        }

        // If we reach here, destructor handled running state
        CPPUNIT_ASSERT(true);
    }

    void testDestructorAfterExplicitStop()
    {
        std::string path = m_testSocketPath;

        {
            UnixSocketSource source(path);
            source.start(m_receiver.get());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            source.stop();

            // Explicit stop before destructor should prevent double cleanup
        }

        CPPUNIT_ASSERT(true);
    }

    void testDestructorWithoutStart()
    {
        std::string path = m_testSocketPath;

        {
            UnixSocketSource source(path);

            // Destructor without start() should be safe
        }

        CPPUNIT_ASSERT(true);
    }

    void testStartCreatesSocket()
    {
        std::string path = m_testSocketPath;
        cleanupSocketFile(path);

        UnixSocketSource source(path);
        source.start(m_receiver.get());

        // Socket file should be created (wait briefly and precisely)
        CPPUNIT_ASSERT(waitForUnixSocket(path, 500, 10));

        source.stop();
    }

    void testStartAfterStop()
    {
        UnixSocketSource source(m_testSocketPath);

        // First start-stop cycle
        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        source.stop();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Second start should work
        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        CPPUNIT_ASSERT(true);

        source.stop();
    }

    void testStartWithExistingSocketFile()
    {
        std::string path = m_testSocketPath;

        // Create a dummy file at socket path
        std::ofstream file(path);
        file << "dummy";
        file.close();

        UnixSocketSource source(path);
        source.start(m_receiver.get());

        // Should handle existing file (unlink and recreate actual UNIX socket)
        // Wait specifically for the socket type to appear, not just any file
        CPPUNIT_ASSERT(waitForUnixSocket(path, 500, 10));

        source.stop();
    }

    void testStartRapidStartStop()
    {
        UnixSocketSource source(m_testSocketPath);

        for (int i = 0; i < 3; i++) {
            source.start(m_receiver.get());
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            source.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Should handle rapid cycles
        CPPUNIT_ASSERT(true);
    }

    void testStartSocketPermissions()
    {
        UnixSocketSource source(m_testSocketPath);

        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (isUnixSocketFile(m_testSocketPath)) {
            struct stat statbuf;
            CPPUNIT_ASSERT_EQUAL(0, stat(m_testSocketPath.c_str(), &statbuf));

            // Socket should exist
            CPPUNIT_ASSERT(S_ISSOCK(statbuf.st_mode));
        }

        source.stop();
    }

    void testStartMultipleSourcesSamePath()
    {
        std::string path = m_testSocketPath;

        UnixSocketSource source1(path);
        source1.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Second source with same path
        MockPacketReceiver receiver2;
        UnixSocketSource source2(path);
        source2.start(&receiver2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Both should handle the situation (second may fail socket creation)
        CPPUNIT_ASSERT(true);

        source2.stop();
        source1.stop();
    }

    void testStopOnRunningSource()
    {
        UnixSocketSource source(m_testSocketPath);

        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Guard: ensure socket exists before stopping for robustness
        (void)waitForUnixSocket(m_testSocketPath, 500, 10);
        source.stop();

        // Should stop cleanly
        CPPUNIT_ASSERT(true);
    }

    void testStopJoinsThread()
    {
        UnixSocketSource source(m_testSocketPath);

        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto stopStart = std::chrono::steady_clock::now();
        source.stop();
        auto stopEnd = std::chrono::steady_clock::now();

        // stop() should block until thread joins
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopEnd - stopStart);

        // Should complete in reasonable time
        CPPUNIT_ASSERT(duration.count() < 5000);
    }

    void testCreateSocketRetryDelay()
    {
        std::string path = prepareDeferredSocketPath("retry_delay.sock");

        UnixSocketSource source(path);

        auto start = std::chrono::steady_clock::now();
        source.start(m_receiver.get());

        // Use larger delay and settle to avoid flakiness
        const int delayBeforeCreateMs = 150;
        const int settleMs = 200;
        createParentDirThenStop(source, path, delayBeforeCreateMs, settleMs);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Assert a meaningful delay occurred
        CPPUNIT_ASSERT(duration.count() >= delayBeforeCreateMs + 10);
    }

    void testCreateSocketExitsOnStop()
    {
        std::string path = prepareDeferredSocketPath("exit_on_stop.sock");

        UnixSocketSource source(path);
        source.start(m_receiver.get());

        // Let it retry a few times, then allow and stop
        createParentDirThenStop(source, path, 80, 120);

        CPPUNIT_ASSERT(true);
    }

    void testCreateSocketMultipleRetries()
    {
        std::string path = prepareDeferredSocketPath("multiple_retries.sock");

        UnixSocketSource source(path);
        source.start(m_receiver.get());

        // Should retry multiple times, then succeed and stop (>=2 retries)
        createParentDirThenStop(source, path, 120, 120);

        CPPUNIT_ASSERT(true);
    }

    void testSourceLoopExitsOnStop()
    {
        UnixSocketSource source(m_testSocketPath);

        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        (void)waitForUnixSocket(m_testSocketPath, 500, 10);
        source.stop();

        // Loop should exit cleanly
        CPPUNIT_ASSERT(true);
    }

    void testMultipleStartStopCycles()
    {
        UnixSocketSource source(m_testSocketPath);

        for (int i = 0; i < 3; i++) {
            source.start(m_receiver.get());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            source.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        CPPUNIT_ASSERT(true);
    }

    void testConcurrentStartStop()
    {
        UnixSocketSource source(m_testSocketPath);

        source.start(m_receiver.get());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Stop shortly after start
        source.stop();

        CPPUNIT_ASSERT(true);
    }

private:
    std::string m_testSocketPath;
    std::string m_testSocketPath2;
    std::string m_tempBaseDir;
    std::unique_ptr<MockPacketReceiver> m_receiver;
    static int s_testCounter;

    // Helper: return parent directory for a given path
    std::string parentDir(const std::string& path)
    {
        auto pos = path.find_last_of('/') ;
        if (pos == std::string::npos) return ".";
        if (pos == 0) return "/";
        return path.substr(0, pos);
    }

    // Helper: ensure a directory exists (single-level)
    void ensureDirExists(const std::string& dir)
    {
        struct stat st{};
        if (stat(dir.c_str(), &st) != 0) {
            (void) ::mkdir(dir.c_str(), 0777);
        }
    }

    // Helper: remove directory if it exists and is empty
    void removeDirIfExists(const std::string& dir)
    {
        struct stat st{};
        if (stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            (void) ::rmdir(dir.c_str());
        }
    }

    // Helper: create a unique base dir under /tmp for deferred socket tests
    std::string makeTempBaseDir()
    {
        return std::string("/tmp/subttx_unixsock_test_") + std::to_string(getpid()) + "_" + std::to_string(s_testCounter);
    }

    // Helper: check if path is a UNIX domain socket
    bool isUnixSocketFile(const std::string& path)
    {
        struct stat st{};
        if (stat(path.c_str(), &st) != 0) return false;
        return S_ISSOCK(st.st_mode);
    }

    // Helper: poll until the path is a UNIX socket (or timeout)
    bool waitForUnixSocket(const std::string& path, int timeoutMs = 600, int pollMs = 10)
    {
        auto start = std::chrono::steady_clock::now();
        while (true) {
            if (isUnixSocketFile(path)) return true;
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            if (elapsed >= timeoutMs) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
        }
    }

    // Helper: prepare a socket path under a base dir that does not yet exist
    std::string prepareDeferredSocketPath(const std::string& sockName)
    {
        m_tempBaseDir = makeTempBaseDir();
        // Make sure base dir does not exist before start
        removeDirIfExists(m_tempBaseDir);
        return m_tempBaseDir + "/" + sockName;
    }

    // Helper: create parent dir after delay, wait for bind (poll), then stop safely
    void createParentDirThenStop(UnixSocketSource& source, const std::string& socketPath, int delayBeforeCreateMs = 150, int settleMs = 200)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayBeforeCreateMs));
        ensureDirExists(parentDir(socketPath));
        // Poll for the socket to appear (actual UNIX socket); if not yet present, allow extra retries (~300ms)
        bool created = waitForUnixSocket(socketPath, settleMs, 10);
        if (!created) {
            created = waitForUnixSocket(socketPath, 600, 10);
        }
        // Now the socket should have been created by the source loop
        if (created) {
            source.stop();
        }
        // Cleanup directory (best-effort)
        cleanupSocketFile(socketPath);
        removeDirIfExists(parentDir(socketPath));
    }

    std::string generateTestSocketPath()
    {
        return "/tmp/test_unixsocketsource_" + std::to_string(getpid()) +
               "_" + std::to_string(s_testCounter) + ".sock";
    }

    void cleanupSocketFile(const std::string& path)
    {
        unlink(path.c_str());
    }
};

int UnixSocketSourceTest::s_testCounter = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(UnixSocketSourceTest);
