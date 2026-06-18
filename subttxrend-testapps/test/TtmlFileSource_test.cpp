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
    CPPUNIT_TEST(testCompleteWorkflowOnSingleSource);

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

    void testCompleteWorkflowOnSingleSource()
    {
        // Validate the observable packet sequence on a single source instance.
        {
            const std::vector<std::uint8_t> content = {0x41, 0x42, 0x43};
            writeFile(tempFilePath, content);
            TtmlFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);

            CPPUNIT_ASSERT(source.readPacket(packet));

            // First packet should be RESET
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());
            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), type);

            std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);

            std::uint32_t size = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), size);

            CPPUNIT_ASSERT(source.readPacket(packet));

            // Second packet should be SELECTION
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7), type);

            counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), counter);

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint32_t width = readLeUint32(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(800), width);

            std::uint32_t height = readLeUint32(packet.getBuffer(), 20);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(600), height);

            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x20), buf[16]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), buf[17]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[18]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[19]);

            CPPUNIT_ASSERT(source.readPacket(packet));

            // Third packet should be TIMESTAMP
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());
            type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(9), type);

            counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), counter);

            payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

            channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint64_t timestamp = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), timestamp);

            buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (int i = 16; i < 24; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[i]);
            }

            CPPUNIT_ASSERT(source.readPacket(packet));

            // Fourth packet should be DATA for the file content.
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(27), packet.getSize());
            type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(8), type);

            counter = readLeUint32(packet.getBuffer(), 4);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), counter);

            payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(15), payloadSize);

            channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<char>(content[i]), packet.getBuffer()[24 + i]);
            }

            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtmlFileSourceTest);
