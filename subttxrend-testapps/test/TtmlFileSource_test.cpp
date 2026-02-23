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

#include "TtmlFileSource.hpp"
#include "DataPacket.hpp"

using namespace subttxrend::testapps;

class TtmlFileSourceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(TtmlFileSourceTest);
    CPPUNIT_TEST(testConstructorWithValidPath);
    CPPUNIT_TEST(testConstructorWithEmptyPath);
    CPPUNIT_TEST(testConstructorWithVeryLongPath);
    CPPUNIT_TEST(testConstructorWithSpecialCharactersInPath);
    CPPUNIT_TEST(testConstructorWithNonExistentPath);
    CPPUNIT_TEST(testCompleteWorkflowWithStaticVariableBug);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Generate unique temp file path for each test
        tempFilePath = makeTempFilePath("ttml_test");
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
        return baseName + "_" + std::to_string(stamp) + ".xml";
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

    void testConstructorWithValidPath()
    {
        TtmlFileSource source("/tmp/test.xml");
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/test.xml"), source.getPath());
    }

    void testConstructorWithEmptyPath()
    {
        TtmlFileSource source("");
        CPPUNIT_ASSERT_EQUAL(std::string(""), source.getPath());
    }

    void testConstructorWithVeryLongPath()
    {
        std::string longPath(5000, 'a');
        longPath += ".xml";
        TtmlFileSource source(longPath);
        CPPUNIT_ASSERT_EQUAL(longPath, source.getPath());
    }

    void testConstructorWithSpecialCharactersInPath()
    {
        std::string specialPath = "/tmp/test file with spaces & special#chars.xml";
        TtmlFileSource source(specialPath);
        CPPUNIT_ASSERT_EQUAL(specialPath, source.getPath());
    }

    void testConstructorWithNonExistentPath()
    {
        TtmlFileSource source("/non/existent/path/file.xml");
        CPPUNIT_ASSERT_EQUAL(std::string("/non/existent/path/file.xml"), source.getPath());
    }

    void testCompleteWorkflowWithStaticVariableBug()
    {
        // PHASE 1: Validate RESET packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // First packet should be RESET
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), type); // RESET type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);

            std::uint32_t size = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), size);

            source.close();
        }

        // PHASE 2: Validate SELECTION packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Second packet (globally) should be SELECTION
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), type); // SELECTION type

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), counter);

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint32_t width = readLeUint32(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(800), width);

            std::uint32_t height = readLeUint32(packet.getBuffer(), 20);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(600), height);

            // Verify little-endian encoding for width (800 = 0x320)
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x20), buf[16]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), buf[17]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[18]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[19]);

            source.close();
        }

        // PHASE 3: Validate TIMESTAMP packet
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Third packet (globally) should be TIMESTAMP
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(9), type); // TIMESTAMP type

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

        // PHASE 4: Validate DATA packet with empty file
        {
            writeFile(tempFilePath, {});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            // Empty file causes read() to return 0, which is treated as EOF
            // This is the actual behavior of the production code
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

        // PHASE 5: Validate DATA packet with 1-byte file
        {
            writeFile(tempFilePath, {0x42});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(8), type);

            // Verify file content
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[24]);

            source.close();
        }

        // PHASE 6: Validate DATA packet with small file and specific content
        {
            std::vector<std::uint8_t> content = {0x01, 0x02, 0x03, 0x04, 0x05};
            writeFile(tempFilePath, content);
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(29), packet.getSize()); // 24 + 5

            // Verify payload size in header
            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5 + 4 + 8), payloadSize); // fileSize + channelId + 8

            // Verify file content preserved
            const char* buf = packet.getBuffer();
            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<char>(content[i]), buf[24 + i]);
            }

            source.close();
        }

        // PHASE 7: Validate DATA packet with binary content (all byte values)
        {
            std::vector<std::uint8_t> content(256);
            for (int i = 0; i < 256; ++i)
            {
                content[i] = static_cast<std::uint8_t>(i);
            }
            writeFile(tempFilePath, content);
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(2048);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(280), packet.getSize()); // 24 + 256

            // Verify all bytes preserved correctly
            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(content[i], buf[24 + i]);
            }

            source.close();
        }

        // PHASE 8: Validate DATA packet with larger file
        {
            std::vector<std::uint8_t> content(10000, 0x55);
            writeFile(tempFilePath, content);
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(20000);
            CPPUNIT_ASSERT(source.readPacket(packet));

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10024), packet.getSize()); // 24 + 10000

            source.close();
        }

        // PHASE 9: Test insufficient capacity for DATA packet
        {
            std::vector<std::uint8_t> content(100, 0x41);
            writeFile(tempFilePath, content);
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(50); // Insufficient: need 124 bytes (24 + 100)
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 10: Test zero capacity
        {
            writeFile(tempFilePath, {0x41});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(0);
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail

            source.close();
        }

        // PHASE 11: Test readPacket after close (file handle = -1)
        {
            writeFile(tempFilePath, {0x41, 0x42});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());
            source.close();

            DataPacket packet(1024);
            bool result = source.readPacket(packet);
            CPPUNIT_ASSERT(!result); // Should fail because file is closed
        }

        // PHASE 12: Test EOF after reading file
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet with content
            CPPUNIT_ASSERT(source.readPacket(packet)); // Should return EOF

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

        // PHASE 13: Test with valid TTML content
        {
            std::string ttmlContent = "<?xml version=\"1.0\"?><tt xmlns=\"http://www.w3.org/ns/ttml\"><body><div><p>Test</p></div></body></tt>";
            std::vector<std::uint8_t> content(ttmlContent.begin(), ttmlContent.end());
            writeFile(tempFilePath, content);

            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet)); // DATA packet

            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + ttmlContent.size()), packet.getSize());

            // Verify TTML content preserved
            std::string readContent(packet.getBuffer() + 24, ttmlContent.size());
            CPPUNIT_ASSERT_EQUAL(ttmlContent, readContent);

            source.close();
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtmlFileSourceTest);
