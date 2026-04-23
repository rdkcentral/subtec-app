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

#include "SmartFileSource.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class SmartFileSourceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(SmartFileSourceTest);
    CPPUNIT_TEST(testConstructorWithValidPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithPathContainingSpaces);
    CPPUNIT_TEST(testConstructorWithPathContainingSpecialCharacters);
    CPPUNIT_TEST(testOpenNonExistingFileReturnsFalse);
    CPPUNIT_TEST(testOpenMultipleTimesSucceeds);
    CPPUNIT_TEST(testOpenEmptyFileThenReadPacketReturnsTrueAndZeroSize);
    CPPUNIT_TEST(testCloseWithoutOpenSucceeds);
    CPPUNIT_TEST(testCloseMultipleTimesSucceeds);
    CPPUNIT_TEST(testCloseAfterOpenThenReopenSucceeds);
    CPPUNIT_TEST(testReadPacketBeforeOpenReturnsFalse);
    CPPUNIT_TEST(testReadPacketAfterCloseReturnsFalse);
    CPPUNIT_TEST(testReadNonTimestampPacketPassesThrough);
    CPPUNIT_TEST(testReadMultipleNonTimestampPacketsInSequence);
    CPPUNIT_TEST(testReadVeryLargePacket);
    CPPUNIT_TEST(testSize24ButWrongTypeDoesNotSleep);
    CPPUNIT_TEST(testSize24Type2ButBytes1to3NotZeroDoesNotSleep);
    CPPUNIT_TEST(testFirstValidTimestampPacketDoesNotSleepNoticeably);
    CPPUNIT_TEST(testSecondValidTimestampPacketSleepsWhenBehind);
    CPPUNIT_TEST(testSecondTimestampWithSameValueDoesNotSleep);
    CPPUNIT_TEST(testMultipleTimestampPacketsInSequence);
    CPPUNIT_TEST(testTimestampPacketWithZeroTimestamp);
    CPPUNIT_TEST(testTimestampPacketWithMaxUint64Value);
    CPPUNIT_TEST(testTimestampByteOrderLittleEndian);
    CPPUNIT_TEST(testMixedTimestampAndNonTimestampPackets);
    CPPUNIT_TEST(testOpenResetsTimestampStateAvoidingUnderflowSleep);
    CPPUNIT_TEST(testReopenAfterReadingMultiplePackets);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override {}
    void tearDown() override {}

protected:
    static void appendLeUint32(std::vector<std::uint8_t>& out, std::uint32_t value)
    {
        out.push_back(static_cast<std::uint8_t>(value & 0xFF));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
        out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
        out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
    }

    static std::vector<std::uint8_t> makePacket(std::uint32_t type,
                                                std::uint32_t counter,
                                                const std::vector<std::uint8_t>& payload)
    {
        std::vector<std::uint8_t> packet;
        packet.reserve(12 + payload.size());

        appendLeUint32(packet, type);
        appendLeUint32(packet, counter);
        appendLeUint32(packet, static_cast<std::uint32_t>(payload.size()));
        packet.insert(packet.end(), payload.begin(), payload.end());

        return packet;
    }

    static std::vector<std::uint8_t> makeTimestampPacket(std::uint64_t timestampMs,
                                                         std::uint32_t counter,
                                                         std::uint32_t stc)
    {
        // SmartFileSource expects a full packet size of 24 bytes:
        // 12 bytes header + 8 bytes timestamp + 4 bytes stc.
        std::vector<std::uint8_t> payload;
        payload.reserve(12);

        // timestamp (8 bytes, little-endian)
        for (int i = 0; i < 8; ++i)
        {
            payload.push_back(static_cast<std::uint8_t>((timestampMs >> (8 * i)) & 0xFF));
        }

        // stc (4 bytes, little-endian)
        payload.push_back(static_cast<std::uint8_t>(stc & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((stc >> 8) & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((stc >> 16) & 0xFF));
        payload.push_back(static_cast<std::uint8_t>((stc >> 24) & 0xFF));

        // type 2 == TIMESTAMP packet in protocol
        return makePacket(2U, counter, payload);
    }

    static std::string makeTempFilePath(const std::string& baseName)
    {
        // Keep it simple and local: tests run from build dir.
        // Use a time-based suffix to avoid collisions when tests are re-run.
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        return baseName + "_" + std::to_string(stamp) + ".bin";
    }

    static void writeFile(const std::string& path, const std::vector<std::vector<std::uint8_t>>& packets)
    {
        std::ofstream os(path, std::ios::binary | std::ios::trunc);
        CPPUNIT_ASSERT_MESSAGE("Failed to create test file", os.is_open());

        for (const auto& packet : packets)
        {
            os.write(reinterpret_cast<const char*>(packet.data()), static_cast<std::streamsize>(packet.size()));
            CPPUNIT_ASSERT_MESSAGE("Failed to write packet to file", os.good());
        }

        os.close();
        CPPUNIT_ASSERT_MESSAGE("Failed to close test file", os.good());
    }

    static void removeFileNoThrow(const std::string& path)
    {
        (void)std::remove(path.c_str());
    }

    static std::uint64_t measureReadMs(SmartFileSource& source, DataPacket& packet)
    {
        auto start = std::chrono::steady_clock::now();
        bool ok = source.readPacket(packet);
        auto end = std::chrono::steady_clock::now();
        CPPUNIT_ASSERT(ok);
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }

    void testConstructorWithValidPath()
    {
        const std::string path = "valid/path/to/file.bin";
        SmartFileSource source(path);
        // Constructor succeeds - verify through subsequent operations
        CPPUNIT_ASSERT_EQUAL(false, source.open()); // File doesn't exist, but constructor succeeded
    }

    void testConstructorWithEmptyPath()
    {
        const std::string path = "";
        SmartFileSource source(path);
        // Constructor accepts empty path
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(4096, 'x');
        longPath += ".bin";
        SmartFileSource source(longPath);
        // Constructor accepts long path
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testConstructorWithPathContainingSpaces()
    {
        const std::string path = "path with spaces/file.bin";
        SmartFileSource source(path);
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testConstructorWithPathContainingSpecialCharacters()
    {
        const std::string path = "path/@#$/file!@#.bin";
        SmartFileSource source(path);
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenNonExistingFileReturnsFalse()
    {
        const std::string path = makeTempFilePath("SmartFileSource_does_not_exist");
        // do not create the file

        SmartFileSource source(path);
        CPPUNIT_ASSERT_EQUAL(false, source.open());
    }

    void testOpenMultipleTimesSucceeds()
    {
        const std::string path = makeTempFilePath("SmartFileSource_multi_open");
        writeFile(path, {});

        SmartFileSource source(path);
        CPPUNIT_ASSERT_EQUAL(true, source.open());
        CPPUNIT_ASSERT_EQUAL(true, source.open()); // Second open also succeeds
        CPPUNIT_ASSERT_EQUAL(true, source.open()); // Third open also succeeds

        source.close();
        removeFileNoThrow(path);
    }

    void testOpenEmptyFileThenReadPacketReturnsTrueAndZeroSize()
    {
        const std::string path = makeTempFilePath("SmartFileSource_empty");
        writeFile(path, {});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

        source.close();
        removeFileNoThrow(path);
    }

    void testCloseWithoutOpenSucceeds()
    {
        const std::string path = "nonexistent.bin";
        SmartFileSource source(path);
        source.close(); // Should not crash
        CPPUNIT_ASSERT(true); // Test passes if we reach here
    }

    void testCloseMultipleTimesSucceeds()
    {
        const std::string path = makeTempFilePath("SmartFileSource_multi_close");
        writeFile(path, {});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        source.close();
        source.close(); // Second close should be safe
        source.close(); // Third close should be safe

        CPPUNIT_ASSERT(true); // Test passes if no crash
        removeFileNoThrow(path);
    }

    void testCloseAfterOpenThenReopenSucceeds()
    {
        const std::string path = makeTempFilePath("SmartFileSource_close_reopen");
        writeFile(path, {});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());
        source.close();
        CPPUNIT_ASSERT(source.open()); // Should succeed after close

        source.close();
        removeFileNoThrow(path);
    }

    void testReadPacketBeforeOpenReturnsFalse()
    {
        const std::string path = makeTempFilePath("SmartFileSource_before_open");
        writeFile(path, {});

        SmartFileSource source(path);
        DataPacket packet(1024);

        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet));

        removeFileNoThrow(path);
    }

    void testReadPacketAfterCloseReturnsFalse()
    {
        const std::string path = makeTempFilePath("SmartFileSource_read_after_close");
        auto packet1 = makePacket(1U, 0U, {});
        writeFile(path, {packet1});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet)); // Read succeeds while open

        source.close();
        CPPUNIT_ASSERT_EQUAL(false, source.readPacket(packet)); // Read fails after close

        removeFileNoThrow(path);
    }

    void testReadNonTimestampPacketPassesThrough()
    {
        const std::string path = makeTempFilePath("SmartFileSource_nontimestamp");

        // Make a header-only packet (payload size = 0) -> total size 12.
        std::vector<std::uint8_t> packetBytes = makePacket(1U, 0xAABBCCDDU, {});
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packetBytes.size());

        writeFile(path, {packetBytes});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());

        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), packetBytes.data(), packetBytes.size()));

        source.close();
        removeFileNoThrow(path);
    }

    void testReadMultipleNonTimestampPacketsInSequence()
    {
        const std::string path = makeTempFilePath("SmartFileSource_multi_read");

        auto p1 = makePacket(1U, 100U, {0xAA, 0xBB});
        auto p2 = makePacket(3U, 200U, {0xCC, 0xDD, 0xEE});
        auto p3 = makePacket(5U, 300U, {});
        writeFile(path, {p1, p2, p3});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getSize()); // 12 header + 2 payload
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), p1.data(), p1.size()));

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(15), packet.getSize()); // 12 header + 3 payload
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), p2.data(), p2.size()));

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize()); // 12 header + 0 payload
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), p3.data(), p3.size()));

        // EOF
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

        source.close();
        removeFileNoThrow(path);
    }

    void testReadVeryLargePacket()
    {
        const std::string path = makeTempFilePath("SmartFileSource_large_packet");

        std::vector<std::uint8_t> largePayload(65536, 0x42);
        auto largePacket = makePacket(1U, 0U, largePayload);
        writeFile(path, {largePacket});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(512 * 1024); // Large enough buffer
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12 + 65536), packet.getSize());

        source.close();
        removeFileNoThrow(path);
    }

    void testSize24ButWrongTypeDoesNotSleep()
    {
        const std::string path = makeTempFilePath("SmartFileSource_wrong_type");

        // First: valid timestamp to set previous timestamps.
        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);

        // Second: packet with total size 24, but type != 2 (so SmartFileSource must not sleep).
        // We'll still set the payload such that if it were interpreted as timestamp it would be far in the future.
        std::vector<std::uint8_t> futurePayload;
        futurePayload.reserve(12);
        std::uint64_t farFuture = 5000U;
        for (int i = 0; i < 8; ++i)
        {
            futurePayload.push_back(static_cast<std::uint8_t>((farFuture >> (8 * i)) & 0xFF));
        }
        appendLeUint32(futurePayload, 0U);

        auto notTimestamp = makePacket(3U, 2U, futurePayload);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), notTimestamp.size());

        writeFile(path, {ts1, notTimestamp});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        (void)measureReadMs(source, packet);

        const std::uint64_t elapsedMs = measureReadMs(source, packet);

        // If SmartFileSource incorrectly treated this as timestamp, it could sleep up to ~4s.
        // Upper bound keeps this test meaningful.
        CPPUNIT_ASSERT(elapsedMs < 500U);

        source.close();
        removeFileNoThrow(path);
    }

    void testSize24Type2ButBytes1to3NotZeroDoesNotSleep()
    {
        const std::string path = makeTempFilePath("SmartFileSource_invalid_timestamp_header");

        // Create a packet with size 24, type 2, but bytes[1-3] of type field not all zero
        // SmartFileSource checks: (data[0] == 2) && (data[1] == 0) && (data[2] == 0) && (data[3] == 0)
        // If any of bytes 1-3 are non-zero, it should NOT be treated as a timestamp packet.
        std::vector<std::uint8_t> invalidPayload(12, 0);
        auto invalidPacket = makePacket(2U, 2U, invalidPayload);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), invalidPacket.size());

        // Corrupt byte[1] of the type field (which is at packet index 1)
        invalidPacket[1] = 0x01; // Now type bytes are [0x02, 0x01, 0x00, 0x00]

        writeFile(path, {invalidPacket});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);

        // This should be treated as a non-timestamp packet (no sleep)
        const std::uint64_t elapsedMs = measureReadMs(source, packet);
        CPPUNIT_ASSERT(elapsedMs < 200U);

        source.close();
        removeFileNoThrow(path);
    }

    void testFirstValidTimestampPacketDoesNotSleepNoticeably()
    {
        const std::string path = makeTempFilePath("SmartFileSource_first_timestamp");

        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), ts1.size());
        writeFile(path, {ts1});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        const std::uint64_t elapsedMs = measureReadMs(source, packet);

        // No sleep should happen for the first valid timestamp packet.
        // Keep threshold generous to avoid flakiness on slower systems.
        CPPUNIT_ASSERT(elapsedMs < 200U);

        source.close();
        removeFileNoThrow(path);
    }

    void testSecondValidTimestampPacketSleepsWhenBehind()
    {
        const std::string path = makeTempFilePath("SmartFileSource_sleep");

        // Two timestamp packets with a large delta.
        // Second read should sleep roughly (packetDiff - clockDiff).
        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        auto ts2 = makeTimestampPacket(1400U, 2U, 0U); // +400ms
        writeFile(path, {ts1, ts2});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        (void)measureReadMs(source, packet); // first packet initializes state

        const std::uint64_t elapsedMs = measureReadMs(source, packet);

        // Expect a noticeable sleep; use a lower bound to avoid scheduler variance.
        CPPUNIT_ASSERT(elapsedMs >= 200U);

        source.close();
        removeFileNoThrow(path);
    }

    void testSecondTimestampWithSameValueDoesNotSleep()
    {
        const std::string path = makeTempFilePath("SmartFileSource_same_timestamp");

        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        auto ts2 = makeTimestampPacket(1000U, 2U, 0U); // Same timestamp
        writeFile(path, {ts1, ts2});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        (void)measureReadMs(source, packet);

        const std::uint64_t elapsedMs = measureReadMs(source, packet);
        // No sleep for same timestamp (packetDiff = 0)
        CPPUNIT_ASSERT(elapsedMs < 200U);

        source.close();
        removeFileNoThrow(path);
    }

    void testMultipleTimestampPacketsInSequence()
    {
        const std::string path = makeTempFilePath("SmartFileSource_multi_timestamps");

        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        auto ts2 = makeTimestampPacket(1100U, 2U, 0U); // +100ms
        auto ts3 = makeTimestampPacket(1200U, 3U, 0U); // +100ms
        writeFile(path, {ts1, ts2, ts3});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        (void)measureReadMs(source, packet); // First: initializes state

        const std::uint64_t elapsed2 = measureReadMs(source, packet); // Second: small sleep
        const std::uint64_t elapsed3 = measureReadMs(source, packet); // Third: small sleep

        // Both should have some delay (but keep threshold loose for CI)
        CPPUNIT_ASSERT(elapsed2 < 500U);
        CPPUNIT_ASSERT(elapsed3 < 500U);

        source.close();
        removeFileNoThrow(path);
    }

    void testTimestampPacketWithZeroTimestamp()
    {
        const std::string path = makeTempFilePath("SmartFileSource_zero_timestamp");

        auto ts1 = makeTimestampPacket(0U, 1U, 0U);
        auto ts2 = makeTimestampPacket(100U, 2U, 0U);
        writeFile(path, {ts1, ts2});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());

        source.close();
        removeFileNoThrow(path);
    }

    void testTimestampPacketWithMaxUint64Value()
    {
        const std::string path = makeTempFilePath("SmartFileSource_max_timestamp");

        // Use max value with a small delta to avoid long sleep
        std::uint64_t maxVal = 0xFFFFFFFFFFFFFFFFULL;
        auto ts1 = makeTimestampPacket(maxVal - 50U, 1U, 0U);
        auto ts2 = makeTimestampPacket(maxVal, 2U, 0U);
        writeFile(path, {ts1, ts2});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());

        // Second read with small timestamp delta won't cause excessive sleep
        const std::uint64_t elapsedMs = measureReadMs(source, packet);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
        CPPUNIT_ASSERT(elapsedMs < 500U);

        source.close();
        removeFileNoThrow(path);
    }

    void testTimestampByteOrderLittleEndian()
    {
        const std::string path = makeTempFilePath("SmartFileSource_byte_order");

        // Create a timestamp with a specific pattern to verify byte order
        // 0x0102030405060708 in little-endian should be bytes: 08 07 06 05 04 03 02 01
        std::uint64_t timestamp = 0x0102030405060708ULL;
        auto ts = makeTimestampPacket(timestamp, 1U, 0U);

        // Verify the timestamp bytes are in little-endian order in the packet
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x08), ts[12]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x07), ts[13]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x06), ts[14]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x05), ts[15]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x04), ts[16]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), ts[17]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x02), ts[18]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x01), ts[19]);

        writeFile(path, {ts});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet));

        // Verify the packet was read correctly
        CPPUNIT_ASSERT_EQUAL(0, memcmp(packet.getBuffer(), ts.data(), ts.size()));

        source.close();
        removeFileNoThrow(path);
    }

    void testMixedTimestampAndNonTimestampPackets()
    {
        const std::string path = makeTempFilePath("SmartFileSource_mixed_packets");

        auto p1 = makePacket(1U, 100U, {0xAA});
        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        auto p2 = makePacket(3U, 200U, {0xBB, 0xCC});
        auto ts2 = makeTimestampPacket(1050U, 2U, 0U);
        auto p3 = makePacket(5U, 300U, {});
        writeFile(path, {p1, ts1, p2, ts2, p3});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(13), packet.getSize()); // Non-timestamp

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize()); // First timestamp

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), packet.getSize()); // Non-timestamp

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize()); // Second timestamp

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize()); // Non-timestamp

        source.close();
        removeFileNoThrow(path);
    }

    void testOpenResetsTimestampStateAvoidingUnderflowSleep()
    {
        const std::string path = makeTempFilePath("SmartFileSource_reopen_reset");

        // File has 2 timestamps increasing.
        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        auto ts2 = makeTimestampPacket(1300U, 2U, 0U);
        writeFile(path, {ts1, ts2});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        (void)measureReadMs(source, packet);
        (void)measureReadMs(source, packet);

        source.close();

        // Reopen: internal previous timestamp state must be cleared.
        // If it wasn't, reading ts1 (1000) after previously reading ts2 (1300)
        // would cause an unsigned underflow and potentially an enormous sleep.
        CPPUNIT_ASSERT(source.open());

        const std::uint64_t elapsedMs = measureReadMs(source, packet);
        CPPUNIT_ASSERT(elapsedMs < 200U);

        source.close();
        removeFileNoThrow(path);
    }

    void testReopenAfterReadingMultiplePackets()
    {
        const std::string path = makeTempFilePath("SmartFileSource_reopen_multi");

        auto p1 = makePacket(1U, 100U, {0xAA});
        auto p2 = makePacket(2U, 200U, {0xBB});
        auto ts1 = makeTimestampPacket(1000U, 1U, 0U);
        writeFile(path, {p1, p2, ts1});

        SmartFileSource source(path);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source.readPacket(packet)); // Read p1
        CPPUNIT_ASSERT(source.readPacket(packet)); // Read p2
        CPPUNIT_ASSERT(source.readPacket(packet)); // Read ts1

        source.close();

        // Reopen and read from beginning
        CPPUNIT_ASSERT(source.open());

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(13), packet.getSize()); // p1 again

        source.close();
        removeFileNoThrow(path);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SmartFileSourceTest);
