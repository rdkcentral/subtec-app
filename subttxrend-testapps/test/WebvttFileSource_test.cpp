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

#include "WebvttFileSource.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class WebvttFileSourceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(WebvttFileSourceTest);
    CPPUNIT_TEST(testConstructorWithValidPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithSpecialCharactersInPath);
    CPPUNIT_TEST(testConstructorWithNonExistentPath);
    CPPUNIT_TEST(testCompleteWorkflowWithStaticVariable);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Generate unique temp file path for each test
        tempFilePath = makeTempFilePath("webvtt_test");
    }

    void tearDown() override
    {
        // Clean up temp file
        removeFileNoThrow(tempFilePath);
    }

protected:
    std::string tempFilePath;

    // Helper methods
    static std::string makeTempFilePath(const std::string& baseName)
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        return baseName + "_" + std::to_string(stamp) + ".vtt";
    }

    static void writeFile(const std::string& path, const std::vector<std::uint8_t>& content)
    {
        std::ofstream os(path, std::ios::binary | std::ios::trunc);
        CPPUNIT_ASSERT_MESSAGE("Failed to create test file", os.is_open());

        if (!content.empty())
        {
            os.write(reinterpret_cast<const char*>(content.data()),
                    static_cast<std::streamsize>(content.size()));
            CPPUNIT_ASSERT_MESSAGE("Failed to write to file", os.good());
        }

        os.close();
        CPPUNIT_ASSERT_MESSAGE("Failed to close test file", os.good());
    }

    static void removeFileNoThrow(const std::string& path)
    {
        (void)std::remove(path.c_str());
    }

    static std::uint32_t readLeUint32(const char* buffer, size_t offset)
    {
        const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(buffer + offset);
        return static_cast<std::uint32_t>(ptr[0]) |
               (static_cast<std::uint32_t>(ptr[1]) << 8) |
               (static_cast<std::uint32_t>(ptr[2]) << 16) |
               (static_cast<std::uint32_t>(ptr[3]) << 24);
    }

    static std::uint64_t readLeUint64(const char* buffer, size_t offset)
    {
        const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(buffer + offset);
        return static_cast<std::uint64_t>(ptr[0]) |
               (static_cast<std::uint64_t>(ptr[1]) << 8) |
               (static_cast<std::uint64_t>(ptr[2]) << 16) |
               (static_cast<std::uint64_t>(ptr[3]) << 24) |
               (static_cast<std::uint64_t>(ptr[4]) << 32) |
               (static_cast<std::uint64_t>(ptr[5]) << 40) |
               (static_cast<std::uint64_t>(ptr[6]) << 48) |
               (static_cast<std::uint64_t>(ptr[7]) << 56);
    }

    // Constructor tests
    void testConstructorWithValidPath()
    {
        WebvttFileSource source("/tmp/test.vtt");
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/test.vtt"), source.getPath());
    }

    void testConstructorWithEmptyPath()
    {
        WebvttFileSource source("");
        CPPUNIT_ASSERT_EQUAL(std::string(""), source.getPath());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(5000, 'a');
        longPath += ".vtt";
        WebvttFileSource source(longPath);
        CPPUNIT_ASSERT_EQUAL(longPath, source.getPath());
    }

    void testConstructorWithSpecialCharactersInPath()
    {
        std::string specialPath = "/tmp/test file with spaces & special#chars.vtt";
        WebvttFileSource source(specialPath);
        CPPUNIT_ASSERT_EQUAL(specialPath, source.getPath());
    }

    void testConstructorWithNonExistentPath()
    {
        WebvttFileSource source("/non/existent/path/file.vtt");
        CPPUNIT_ASSERT_EQUAL(std::string("/non/existent/path/file.vtt"), source.getPath());
    }

    // Comprehensive test that validates the complete WebvttFileSource behavior
    // NOTE: This single test is necessary due to a CRITICAL BUG in the production code:
    // The readPacket() method uses static variables (resetSent, headerSent, timestampSent, counter)
    // which are shared across ALL instances of WebvttFileSource. This means:
    // - The packet sequence (RESET -> SELECTION -> TIMESTAMP -> DATA) is global, not per-instance
    // - Multiple test methods would interfere with each other
    // - This test validates the complete workflow in one method
    void testCompleteWorkflowWithStaticVariable()
    {
        // PHASE 1: Validate RESET packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // First packet should be RESET
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), type); // RESET_ALL type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);

            std::uint32_t size = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), size);

            // Verify little-endian encoding for type (3 = 0x03)
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), buf[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[1]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[2]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[3]);

            source.close();
        }

        // PHASE 2: Validate RESET packet with insufficient buffer capacity
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10); // Insufficient: need 12 bytes
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 3: Validate SELECTION packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Second packet (globally) should be SELECTION
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(16), type); // WVTT_SELECTION type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), counter);

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint32_t width = readLeUint32(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), width);

            std::uint32_t height = readLeUint32(packet.getBuffer(), 20);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), height);

            // Verify little-endian encoding for width (1920 = 0x780)
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x80), buf[16]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x07), buf[17]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[18]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[19]);

            // Verify little-endian encoding for height (1080 = 0x438)
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x38), buf[20]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x04), buf[21]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[22]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[23]);

            source.close();
        }

        // PHASE 4: Validate SELECTION packet with insufficient buffer capacity
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(20); // Insufficient: need 24 bytes
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 5: Validate TIMESTAMP packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Third packet (globally) should be TIMESTAMP
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(18), type); // WVTT_TIMESTAMP type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), counter);

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint64_t timestamp = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), timestamp);

            // Verify little-endian encoding for timestamp (all zeroes)
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (int i = 16; i < 24; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[i]);
            }

            source.close();
        }

        // PHASE 6: Validate TIMESTAMP packet with insufficient buffer capacity
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(20); // Insufficient: need 24 bytes
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 7: Validate first DATA packet (all headers sent by now)
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Verify counter incremented from TIMESTAMP's 2
            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), counter);

            source.close();
        }

        // PHASE 8: Validate DATA packet with empty file
        {
            writeFile(tempFilePath, {});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Empty file causes read() to return 0, which is treated as EOF
            // NOTE: Counter still increments (4->5) even though packet.setSize(0) is called
            // This is because counter++ happens when building header before the read operation
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

        // PHASE 9: Validate DATA packet with 1-byte file
        {
            writeFile(tempFilePath, {0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type); // WVTT_DATA type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5), counter); // Counter=5 (PHASE 8 incremented even with empty file)

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1 + 4 + 8), payloadSize); // fileSize + channelId + dataOffset

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint64_t dataOffset = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), dataOffset);

            // Verify file content
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[24]);

            source.close();
        }

        // PHASE 10: Validate DATA packet with small file and specific content
        {
            std::vector<std::uint8_t> content = {0x01, 0x02, 0x03, 0x04, 0x05};
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(29), packet.getSize()); // 24 + 5

            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type); // WVTT_DATA type

            // Verify payload size in header
            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5 + 4 + 8), payloadSize); // fileSize + channelId + dataOffset

            // Verify file content preserved
            const char* buf = packet.getBuffer();
            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<char>(content[i]), buf[24 + i]);
            }

            source.close();
        }

        // PHASE 11: Validate DATA packet with binary content (all byte values)
        {
            std::vector<std::uint8_t> content(256);
            for (int i = 0; i < 256; ++i)
            {
                content[i] = static_cast<std::uint8_t>(i);
            }
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(2048);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(280), packet.getSize()); // 24 + 256

            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type); // WVTT_DATA type

            // Verify all bytes preserved correctly
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(content[i], buf[24 + i]);
            }

            source.close();
        }

        // PHASE 12: Validate DATA packet with larger file
        {
            std::vector<std::uint8_t> content(10000, 0x55);
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(20000);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10024), packet.getSize()); // 24 + 10000

            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type); // WVTT_DATA type

            // Verify content
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (size_t i = 0; i < 100; ++i) // Spot check first 100 bytes
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x55), buf[24 + i]);
            }

            source.close();
        }

        // PHASE 13: Test insufficient capacity for DATA packet
        {
            std::vector<std::uint8_t> content(100, 0x41);
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(50); // Insufficient: need 124 bytes (24 + 100)
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 14: Test zero capacity
        {
            writeFile(tempFilePath, {0x41});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(0);
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 15: Test exact capacity boundary for DATA packet
        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(26); // Exact capacity for DATA (24 + 2)
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(result); // Should succeed
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(26), packet.getSize());

            // Verify content
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x41), packet.getBuffer()[24]);
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[25]);

            source.close();
        }

        // PHASE 16: Test readPacket after close (file handle = -1)
        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());
            source.close();

            DataPacket packet(1024);
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail because file is closed
        }

        // PHASE 17: Test readPacket without calling open first
        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);
            // Don't call open()

            DataPacket packet(1024);
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail because file is not open
        }

        // PHASE 18: Test EOF after reading file
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet with content
            CPPUNIT_ASSERT(source.readPacket(packet)); // Should return EOF

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

        // PHASE 19: Test with valid WebVTT content
        {
            std::string webvttContent = "WEBVTT\n\n00:00:01.000 --> 00:00:04.000\nHello World\n\n00:00:05.000 --> 00:00:08.000\nSubtitle Test\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type); // WVTT_DATA type

            // Verify WebVTT content preserved
            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        // PHASE 20: Test with WebVTT metadata and cue settings
        {
            std::string webvttContent = "WEBVTT - Test File\nKind: captions\nLanguage: en\n\n00:00:01.000 --> 00:00:04.000 align:middle line:90%\n<v Speaker>Test Caption</v>\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            // Verify content integrity
            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        // PHASE 21: Test with special WebVTT characters and formatting
        {
            std::string webvttContent = "WEBVTT\n\nNOTE Test note\n\n00:00:01.000 --> 00:00:04.000\n<b>Bold</b> <i>Italic</i> <u>Underline</u>\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            // Verify HTML tags preserved
            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);
            CPPUNIT_ASSERT(readContent.find("<b>Bold</b>") != std::string::npos);
            CPPUNIT_ASSERT(readContent.find("<i>Italic</i>") != std::string::npos);
            CPPUNIT_ASSERT(readContent.find("<u>Underline</u>") != std::string::npos);

            source.close();
        }

        // PHASE 22: Test open() with non-existent file
        {
            WebvttFileSource source("/non/existent/path/file.vtt");
            bool openResult = source.open();
            CPPUNIT_ASSERT(!openResult); // Should fail

            // Verify subsequent readPacket fails
            DataPacket packet(1024);
            bool readResult = source.readPacket(packet);
            CPPUNIT_ASSERT(!readResult); // Should fail
        }

        // PHASE 23: Test close() and verify file handle invalidation
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            // Verify file handle is valid (not -1)
            int fileHandle = source.getFileHandle();
            CPPUNIT_ASSERT(fileHandle != -1);

            source.close();

            // Verify file handle is now invalid
            fileHandle = source.getFileHandle();
            CPPUNIT_ASSERT_EQUAL(-1, fileHandle);
        }

        // PHASE 24: Test multiple close() calls
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            source.close();
            source.close(); // Second close should not crash

            int fileHandle = source.getFileHandle();
            CPPUNIT_ASSERT_EQUAL(-1, fileHandle);
        }

        // PHASE 25: Test DATA packet counter increment consistency
        {
            writeFile(tempFilePath, {0x11, 0x22, 0x33});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            // Verify counter has incremented from previous tests
            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            // Counter should be > 0 due to cumulative effect of static variable
            CPPUNIT_ASSERT(counter > 0);

            source.close();
        }

        // PHASE 26: Test with UTF-8 encoded WebVTT content
        {
            std::string webvttContent = "WEBVTT\n\n00:00:01.000 --> 00:00:04.000\n";
            webvttContent += "Hello 世界 🌍 Testing UTF-8\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            // Verify UTF-8 content preserved byte-for-byte
            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        // PHASE 27: Test DATA packet channelId field is always 0
        {
            writeFile(tempFilePath, {0xAA, 0xBB, 0xCC, 0xDD});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            // Verify hardcoded to 0 in little-endian
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[12]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[13]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[14]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[15]);

            source.close();
        }

        // PHASE 28: Test DATA packet dataOffset field is always 0
        {
            writeFile(tempFilePath, {0xFF, 0xEE, 0xDD});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            std::uint64_t dataOffset = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), dataOffset);

            // Verify all 8 bytes are zero
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (int i = 16; i < 24; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[i]);
            }

            source.close();
        }

        // PHASE 29: Test DATA packet size calculation formula
        {
            std::vector<std::uint8_t> content = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            // Size should be: fileSize (7) + channelId (4) + dataOffset (8) = 19
            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(19), payloadSize);

            // Total packet size should be: header (24) + fileSize (7) = 31
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(31), packet.getSize());

            source.close();
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebvttFileSourceTest);
