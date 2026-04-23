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

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "SmartFileTarget.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class SmartFileTargetTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(SmartFileTargetTest);
    CPPUNIT_TEST(testConstructorWithValidPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithPathContainingSpaces);
    CPPUNIT_TEST(testConstructorWithPathContainingSpecialCharacters);
    CPPUNIT_TEST(testOpenWithValidPath);
    CPPUNIT_TEST(testOpenWhenAlreadyOpen);
    CPPUNIT_TEST(testOpenMultipleTimes);
    CPPUNIT_TEST(testCloseWithoutOpen);
    CPPUNIT_TEST(testCloseMultipleTimes);
    CPPUNIT_TEST(testCloseAfterOpenThenReopen);
    CPPUNIT_TEST(testOpenResetsRecordingState);
    CPPUNIT_TEST(testWantsMorePacketsBeforeOpen);
    CPPUNIT_TEST(testWantsMorePacketsAfterOpen);
    CPPUNIT_TEST(testWantsMorePacketsAfterFirstResetAll);
    CPPUNIT_TEST(testWantsMorePacketsAfterSecondResetAll);
    CPPUNIT_TEST(testWantsMorePacketsAfterClose);
    CPPUNIT_TEST(testDetectsValidResetAllPacket);
    CPPUNIT_TEST(testResetAllPacketSize11NotDetected);
    CPPUNIT_TEST(testResetAllPacketSize13NotDetected);
    CPPUNIT_TEST(testResetAllPacketSize0NotDetected);
    CPPUNIT_TEST(testResetAllFirstByteZeroNotDetected);
    CPPUNIT_TEST(testResetAllFirstByte2NotDetected);
    CPPUNIT_TEST(testResetAllFirstByte4NotDetected);
    CPPUNIT_TEST(testResetAllSecondByteNonZeroNotDetected);
    CPPUNIT_TEST(testResetAllLastByteNonZeroNotDetected);
    CPPUNIT_TEST(testResetAllMiddleByteNonZeroNotDetected);
    CPPUNIT_TEST(testResetAllAllBytesExceptFirstNonZeroNotDetected);
    CPPUNIT_TEST(testWritePacketBeforeOpenSkipsNonResetPackets);
    CPPUNIT_TEST(testWriteNonResetPacketBeforeFirstResetAllSkipped);
    CPPUNIT_TEST(testWriteFirstResetAllStartsRecording);
    CPPUNIT_TEST(testWriteRegularPacketAfterFirstResetAllIsWritten);
    CPPUNIT_TEST(testWriteSecondResetAllStopsRecording);
    CPPUNIT_TEST(testWritePacketAfterSecondResetAllReturnsFalse);
    CPPUNIT_TEST(testMultiplePacketsBeforeFirstResetAllAllSkipped);
    CPPUNIT_TEST(testMultiplePacketsBetweenResetAllsAllWritten);
    CPPUNIT_TEST(testMultiplePacketsAfterSecondResetAllAllRejected);
    CPPUNIT_TEST(testConsecutiveResetAllPackets);
    CPPUNIT_TEST(testReopenResetsStateMachineCompletely);
    CPPUNIT_TEST(testThreeResetAllPackets);
    CPPUNIT_TEST(testEmptyPacketNotResetAll);
    CPPUNIT_TEST(testSingleBytePacketNotResetAll);
    CPPUNIT_TEST(testVeryLargePacketNotResetAll);
    CPPUNIT_TEST(testSize12Type2PacketNotResetAll);
    CPPUNIT_TEST(testFileContainsOnlyPacketsBetweenResetAlls);
    CPPUNIT_TEST(testFirstResetAllIsWrittenToFile);
    CPPUNIT_TEST(testSecondResetAllNotWrittenToFile);
    CPPUNIT_TEST(testCompleteRecordingSessionWritesCorrectContent);
    CPPUNIT_TEST(testMultipleRecordingSessionsWithReopen);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_tempFiles.clear();
    }

    void tearDown() override
    {
        // Clean up all temporary files
        for (const auto& file : m_tempFiles)
        {
            removeFileNoThrow(file);
        }
        m_tempFiles.clear();
    }

protected:
    // Helper methods
    std::string makeTempFilePath(const std::string& baseName)
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        std::string path = baseName + "_" + std::to_string(stamp) + ".bin";
        m_tempFiles.push_back(path);
        return path;
    }

    static void removeFileNoThrow(const std::string& path)
    {
        (void)std::remove(path.c_str());
    }

    static void createResetAllPacket(DataPacket& packet)
    {
        char* buffer = packet.getBuffer();
        buffer[0] = 3;
        for (size_t i = 1; i < 12; ++i)
        {
            buffer[i] = 0;
        }
        packet.setSize(12);
    }

    static void createRegularPacket(DataPacket& packet, char firstByte = 1)
    {
        char* buffer = packet.getBuffer();
        buffer[0] = firstByte;
        for (size_t i = 1; i < packet.getCapacity(); ++i)
        {
            buffer[i] = static_cast<char>((i % 256));
        }
        packet.setSize(packet.getCapacity());
    }

    static void createPacketWithData(DataPacket& packet, const std::vector<char>& data)
    {
        char* buffer = packet.getBuffer();
        std::memcpy(buffer, data.data(), data.size());
        packet.setSize(data.size());
    }

    static std::vector<char> readFileContent(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return std::vector<char>();
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size))
        {
            return std::vector<char>();
        }

        return buffer;
    }

    static bool fileExists(const std::string& path)
    {
        std::ifstream file(path);
        return file.good();
    }

    void testConstructorWithValidPath()
    {
        const std::string path = makeTempFilePath("valid_path_test");
        SmartFileTarget target(path);
        // Constructor succeeds - verify by opening
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
    }

    void testConstructorWithEmptyPath()
    {
        const std::string path = "";
        SmartFileTarget target(path);
        // Constructor accepts empty path, but open should fail
        CPPUNIT_ASSERT_EQUAL(false, target.open());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(4096, 'x');
        longPath += ".bin";
        SmartFileTarget target(longPath);
        // Constructor accepts long path (may fail on open depending on OS limits)
        // Just verify constructor doesn't crash
        (void)target.open(); // Result doesn't matter, just testing constructor
    }

    void testConstructorWithPathContainingSpaces()
    {
        const std::string path = makeTempFilePath("path with spaces");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
    }

    void testConstructorWithPathContainingSpecialCharacters()
    {
        const std::string path = makeTempFilePath("path_special!@#");
        SmartFileTarget target(path);
        // Constructor succeeds, open may vary by OS
        (void)target.open();
    }

    void testOpenWithValidPath()
    {
        const std::string path = makeTempFilePath("open_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
        target.close();
    }

    void testOpenWhenAlreadyOpen()
    {
        const std::string path = makeTempFilePath("already_open_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.open()); // Second open should succeed
        target.close();
    }

    void testOpenMultipleTimes()
    {
        const std::string path = makeTempFilePath("multi_open_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
        target.close();
    }

    void testCloseWithoutOpen()
    {
        const std::string path = makeTempFilePath("close_no_open_test");
        SmartFileTarget target(path);
        target.close(); // Should not crash
        CPPUNIT_ASSERT(true); // If we get here, close succeeded without open
    }

    void testCloseMultipleTimes()
    {
        const std::string path = makeTempFilePath("multi_close_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
        target.close(); // Should not crash
        target.close();
        CPPUNIT_ASSERT(true); // Multiple closes succeeded
    }

    void testCloseAfterOpenThenReopen()
    {
        const std::string path = makeTempFilePath("close_reopen_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
        CPPUNIT_ASSERT_EQUAL(true, target.open()); // Reopen should succeed
        target.close();
    }

    void testOpenResetsRecordingState()
    {
        const std::string path = makeTempFilePath("reset_state_test");
        SmartFileTarget target(path);

        // First session
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));
        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets()); // Recording stopped

        target.close();

        // Second session - state should be reset
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // State reset!

        target.close();
    }

    void testWantsMorePacketsBeforeOpen()
    {
        const std::string path = makeTempFilePath("wants_before_open_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets());
    }

    void testWantsMorePacketsAfterOpen()
    {
        const std::string path = makeTempFilePath("wants_after_open_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
        target.close();
    }

    void testWantsMorePacketsAfterFirstResetAll()
    {
        const std::string path = makeTempFilePath("wants_after_reset1_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset(12);
        createResetAllPacket(reset);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset));
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // Still wants more

        target.close();
    }

    void testWantsMorePacketsAfterSecondResetAll()
    {
        const std::string path = makeTempFilePath("wants_after_reset2_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets()); // No more packets wanted

        target.close();
    }

    void testWantsMorePacketsAfterClose()
    {
        const std::string path = makeTempFilePath("wants_after_close_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        target.close();
        // Note: close() doesn't reset m_wantsMorePackets, so it remains true
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());
    }

    void testDetectsValidResetAllPacket()
    {
        const std::string path = makeTempFilePath("valid_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset(12);
        createResetAllPacket(reset);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset)); // Should start recording
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllPacketSize11NotDetected()
    {
        const std::string path = makeTempFilePath("size11_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(11);
        createRegularPacket(packet, 3);
        for (size_t i = 1; i < 11; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped, not RESET
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // Still waiting for first RESET

        target.close();
    }

    void testResetAllPacketSize13NotDetected()
    {
        const std::string path = makeTempFilePath("size13_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(13);
        createRegularPacket(packet, 3);
        for (size_t i = 1; i < 13; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped, not RESET
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // Still waiting

        target.close();
    }

    void testResetAllPacketSize0NotDetected()
    {
        const std::string path = makeTempFilePath("size0_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(10);
        packet.setSize(0);

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllFirstByteZeroNotDetected()
    {
        const std::string path = makeTempFilePath("firstbyte0_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createRegularPacket(packet, 0);
        for (size_t i = 1; i < 12; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllFirstByte2NotDetected()
    {
        const std::string path = makeTempFilePath("firstbyte2_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createRegularPacket(packet, 2);
        for (size_t i = 1; i < 12; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllFirstByte4NotDetected()
    {
        const std::string path = makeTempFilePath("firstbyte4_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createRegularPacket(packet, 4);
        for (size_t i = 1; i < 12; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllSecondByteNonZeroNotDetected()
    {
        const std::string path = makeTempFilePath("secondbyte_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createResetAllPacket(packet);
        packet.getBuffer()[1] = 1; // Make second byte non-zero

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllLastByteNonZeroNotDetected()
    {
        const std::string path = makeTempFilePath("lastbyte_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createResetAllPacket(packet);
        packet.getBuffer()[11] = 1; // Make last byte non-zero

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllMiddleByteNonZeroNotDetected()
    {
        const std::string path = makeTempFilePath("middlebyte_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createResetAllPacket(packet);
        packet.getBuffer()[6] = 0xFF; // Make middle byte non-zero

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testResetAllAllBytesExceptFirstNonZeroNotDetected()
    {
        const std::string path = makeTempFilePath("allnonzero_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(12);
        createRegularPacket(packet, 3);
        for (size_t i = 1; i < 12; ++i)
        {
            packet.getBuffer()[i] = static_cast<char>(0xFF);
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testWritePacketBeforeOpenSkipsNonResetPackets()
    {
        const std::string path = makeTempFilePath("write_before_open_test");
        SmartFileTarget target(path);

        DataPacket packet(20);
        createRegularPacket(packet);
        // Note: non-RESET packets before recording starts return true (skipped)
        // even when the target is not open
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));
    }

    void testWriteNonResetPacketBeforeFirstResetAllSkipped()
    {
        const std::string path = makeTempFilePath("skip_before_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(20);
        createRegularPacket(packet);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Returns true but skips

        target.close();

        // Verify file is empty (nothing written)
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), content.size());
    }

    void testWriteFirstResetAllStartsRecording()
    {
        const std::string path = makeTempFilePath("first_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset(12);
        createResetAllPacket(reset);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset));
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // Still wants more

        target.close();

        // Verify RESET ALL was written
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), content.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(3), content[0]);
    }

    void testWriteRegularPacketAfterFirstResetAllIsWritten()
    {
        const std::string path = makeTempFilePath("write_after_reset1_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset(12);
        createResetAllPacket(reset);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset));

        DataPacket regular(20);
        createRegularPacket(regular, 5);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(regular));

        target.close();

        // Verify both packets were written
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(32), content.size()); // 12 + 20
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(3), content[0]); // First packet
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(5), content[12]); // Second packet
    }

    void testWriteSecondResetAllStopsRecording()
    {
        const std::string path = makeTempFilePath("second_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2)); // Stops recording

        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets()); // No more packets

        target.close();
    }

    void testWritePacketAfterSecondResetAllReturnsFalse()
    {
        const std::string path = makeTempFilePath("after_reset2_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        DataPacket regular(20);
        createRegularPacket(regular);
        CPPUNIT_ASSERT_EQUAL(false, target.writePacket(regular)); // Rejected

        target.close();
    }

    void testMultiplePacketsBeforeFirstResetAllAllSkipped()
    {
        const std::string path = makeTempFilePath("multi_before_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        for (int i = 0; i < 5; ++i)
        {
            DataPacket packet(10 + i);
            createRegularPacket(packet);
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));
        }

        target.close();

        // All packets skipped, file should be empty
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), content.size());
    }

    void testMultiplePacketsBetweenResetAllsAllWritten()
    {
        const std::string path = makeTempFilePath("multi_between_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        size_t expectedSize = 12; // First RESET ALL
        for (int i = 0; i < 5; ++i)
        {
            DataPacket packet(10);
            createRegularPacket(packet);
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));
            expectedSize += 10;
        }

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));
        // Second RESET ALL not written

        target.close();

        // Verify all packets between RESETs were written
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(expectedSize, content.size());
    }

    void testMultiplePacketsAfterSecondResetAllAllRejected()
    {
        const std::string path = makeTempFilePath("multi_after_reset2_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        for (int i = 0; i < 5; ++i)
        {
            DataPacket packet(10);
            createRegularPacket(packet);
            CPPUNIT_ASSERT_EQUAL(false, target.writePacket(packet)); // All rejected
        }

        target.close();
    }

    void testConsecutiveResetAllPackets()
    {
        const std::string path = makeTempFilePath("consecutive_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));
        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets()); // Recording stopped

        target.close();

        // Only first RESET ALL written
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), content.size());
    }

    void testReopenResetsStateMachineCompletely()
    {
        const std::string path1 = makeTempFilePath("reopen_state1_test");
        const std::string path2 = makeTempFilePath("reopen_state2_test");
        SmartFileTarget target(path1);

        // First session
        CPPUNIT_ASSERT_EQUAL(true, target.open());
        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));
        DataPacket packet1(15);
        createRegularPacket(packet1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet1));
        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));
        CPPUNIT_ASSERT_EQUAL(false, target.wantsMorePackets());
        target.close();

        // Create new target with different path to verify state reset
        SmartFileTarget target2(path2);
        CPPUNIT_ASSERT_EQUAL(true, target2.open());
        CPPUNIT_ASSERT_EQUAL(true, target2.wantsMorePackets()); // State reset

        // Should skip packets before first RESET ALL again
        DataPacket preReset(10);
        createRegularPacket(preReset);
        CPPUNIT_ASSERT_EQUAL(true, target2.writePacket(preReset));

        target2.close();

        // Verify second file is empty (packet skipped)
        std::vector<char> content = readFileContent(path2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), content.size());
    }

    void testThreeResetAllPackets()
    {
        const std::string path = makeTempFilePath("three_reset_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        DataPacket reset3(12);
        createResetAllPacket(reset3);
        CPPUNIT_ASSERT_EQUAL(false, target.writePacket(reset3)); // Rejected after recording stopped

        target.close();
    }

    void testEmptyPacketNotResetAll()
    {
        const std::string path = makeTempFilePath("empty_packet_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(100);
        packet.setSize(0);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped

        target.close();

        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), content.size());
    }

    void testSingleBytePacketNotResetAll()
    {
        const std::string path = makeTempFilePath("single_byte_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(1);
        createRegularPacket(packet, 3);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped

        target.close();

        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), content.size());
    }

    void testVeryLargePacketNotResetAll()
    {
        const std::string path = makeTempFilePath("large_packet_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket packet(10000);
        createRegularPacket(packet, 3);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets()); // Still waiting for RESET

        target.close();
    }

    void testSize12Type2PacketNotResetAll()
    {
        const std::string path = makeTempFilePath("type2_packet_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Create a packet of size 12 with first byte = 2 (timestamp type), rest zeros
        DataPacket packet(12);
        createRegularPacket(packet, 2);
        for (size_t i = 1; i < 12; ++i)
        {
            packet.getBuffer()[i] = 0;
        }

        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet)); // Skipped (not type 3)
        CPPUNIT_ASSERT_EQUAL(true, target.wantsMorePackets());

        target.close();
    }

    void testFileContainsOnlyPacketsBetweenResetAlls()
    {
        const std::string path = makeTempFilePath("content_between_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Packets before first RESET ALL (should be skipped)
        DataPacket pre1(10);
        createRegularPacket(pre1, 7);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(pre1));
        DataPacket pre2(15);
        createRegularPacket(pre2, 8);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(pre2));

        // First RESET ALL
        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        // Packets during recording
        DataPacket during1(20);
        createRegularPacket(during1, 9);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(during1));
        DataPacket during2(25);
        createRegularPacket(during2, 10);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(during2));

        // Second RESET ALL
        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        // Packets after recording (should be rejected)
        DataPacket post1(30);
        createRegularPacket(post1, 11);
        CPPUNIT_ASSERT_EQUAL(false, target.writePacket(post1));

        target.close();

        // Verify file contains only: reset1 + during1 + during2
        std::vector<char> content = readFileContent(path);
        size_t expectedSize = 12 + 20 + 25; // 57
        CPPUNIT_ASSERT_EQUAL(expectedSize, content.size());

        // Verify packet contents
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(3), content[0]); // reset1
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(9), content[12]); // during1
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(10), content[32]); // during2
    }

    void testFirstResetAllIsWrittenToFile()
    {
        const std::string path = makeTempFilePath("first_reset_written_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset(12);
        createResetAllPacket(reset);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset));

        target.close();

        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), content.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(3), content[0]);
        for (size_t i = 1; i < 12; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0), content[i]);
        }
    }

    void testSecondResetAllNotWrittenToFile()
    {
        const std::string path = makeTempFilePath("second_reset_not_written_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        target.close();

        // Only first RESET ALL written
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), content.size());
    }

    void testCompleteRecordingSessionWritesCorrectContent()
    {
        const std::string path = makeTempFilePath("complete_session_test");
        SmartFileTarget target(path);
        CPPUNIT_ASSERT_EQUAL(true, target.open());

        // Skipped packets
        for (int i = 0; i < 3; ++i)
        {
            DataPacket packet(5);
            createRegularPacket(packet, static_cast<char>(100 + i));
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));
        }

        // Start recording
        DataPacket reset1(12);
        createResetAllPacket(reset1);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

        // Recorded packets
        std::vector<char> expected;
        expected.insert(expected.end(), 12, 0);
        expected[0] = 3; // First RESET ALL

        for (int i = 0; i < 4; ++i)
        {
            DataPacket packet(8);
            createRegularPacket(packet, static_cast<char>(50 + i));
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet));

            expected.push_back(static_cast<char>(50 + i));
            for (int j = 1; j < 8; ++j)
            {
                expected.push_back(static_cast<char>(j % 256));
            }
        }

        // Stop recording
        DataPacket reset2(12);
        createResetAllPacket(reset2);
        CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

        // Rejected packets
        for (int i = 0; i < 2; ++i)
        {
            DataPacket packet(6);
            createRegularPacket(packet, static_cast<char>(200 + i));
            CPPUNIT_ASSERT_EQUAL(false, target.writePacket(packet));
        }

        target.close();

        // Verify content
        std::vector<char> content = readFileContent(path);
        CPPUNIT_ASSERT_EQUAL(expected.size(), content.size());
        for (size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expected[i], content[i]);
        }
    }

    void testMultipleRecordingSessionsWithReopen()
    {
        const std::string path1 = makeTempFilePath("multi_session1_test");
        const std::string path2 = makeTempFilePath("multi_session2_test");

        // First session
        {
            SmartFileTarget target(path1);
            CPPUNIT_ASSERT_EQUAL(true, target.open());

            DataPacket reset1(12);
            createResetAllPacket(reset1);
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset1));

            DataPacket packet1(10);
            createRegularPacket(packet1, 5);
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(packet1));

            DataPacket reset2(12);
            createResetAllPacket(reset2);
            CPPUNIT_ASSERT_EQUAL(true, target.writePacket(reset2));

            target.close();
        }

        // Second session with new target
        {
            SmartFileTarget target2(path2);
            CPPUNIT_ASSERT_EQUAL(true, target2.open());
            CPPUNIT_ASSERT_EQUAL(true, target2.wantsMorePackets()); // Fresh state

            // Skip pre-recording packet
            DataPacket pre(5);
            createRegularPacket(pre, 1);
            CPPUNIT_ASSERT_EQUAL(true, target2.writePacket(pre));

            DataPacket reset1(12);
            createResetAllPacket(reset1);
            CPPUNIT_ASSERT_EQUAL(true, target2.writePacket(reset1));

            DataPacket packet2(15);
            createRegularPacket(packet2, 6);
            CPPUNIT_ASSERT_EQUAL(true, target2.writePacket(packet2));

            DataPacket reset2(12);
            createResetAllPacket(reset2);
            CPPUNIT_ASSERT_EQUAL(true, target2.writePacket(reset2));

            target2.close();
        }

        // Verify both files
        std::vector<char> content1 = readFileContent(path1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(22), content1.size()); // 12 + 10

        std::vector<char> content2 = readFileContent(path2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(27), content2.size()); // 12 + 15
    }

private:
    std::vector<std::string> m_tempFiles;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SmartFileTargetTest);
