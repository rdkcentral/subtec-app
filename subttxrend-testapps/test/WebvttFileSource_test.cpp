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
    CPPUNIT_TEST(testCompleteWorkflowOnSingleSource);
    CPPUNIT_TEST(testDataPath);
    CPPUNIT_TEST(testReadGuards);
    CPPUNIT_TEST(testCloseBehavior);
    CPPUNIT_TEST(testStatAfterOpen);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        tempFilePath = makeTempFilePath("webvtt_test");
    }

    void tearDown() override
    {
        removeFileNoThrow(tempFilePath);
    }

protected:
    std::string tempFilePath;

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

    void testConstructorWithValidPath()
    {
        WebvttFileSource source("/tmp/test.vtt");
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/test.vtt"), source.getPath());
        CPPUNIT_ASSERT_EQUAL(-1, source.getFileHandle());
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

    void testCompleteWorkflowOnSingleSource()
    {
        const std::vector<std::uint8_t> content = {0x41, 0x42, 0x43};
        WebvttFileSource source(tempFilePath);

        writeFile(tempFilePath, content);
        CPPUNIT_ASSERT(source.open());

        DataPacket packet(1024);
        DataPacket shortResetPacket(11);
        CPPUNIT_ASSERT(!source.readPacket(shortResetPacket));

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), packet.getSize());

        std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), type);

        std::uint32_t counter = readLeUint32(packet.getBuffer(), 4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), counter);

        std::uint32_t size = readLeUint32(packet.getBuffer(), 8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), size);

        const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x03), buf[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[3]);

        DataPacket shortSelectionPacket(23);
        CPPUNIT_ASSERT(!source.readPacket(shortSelectionPacket));

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());

        type = readLeUint32(packet.getBuffer(), 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(16), type);

        counter = readLeUint32(packet.getBuffer(), 4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), counter);

        std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(12), payloadSize);

        std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

        std::uint32_t width = readLeUint32(packet.getBuffer(), 16);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1920), width);

        std::uint32_t height = readLeUint32(packet.getBuffer(), 20);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1080), height);

        buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x80), buf[16]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x07), buf[17]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[18]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[19]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x38), buf[20]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x04), buf[21]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[22]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[23]);

        DataPacket shortTimestampPacket(23);
        CPPUNIT_ASSERT(!source.readPacket(shortTimestampPacket));

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24), packet.getSize());

        type = readLeUint32(packet.getBuffer(), 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(18), type);

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

        type = readLeUint32(packet.getBuffer(), 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type);

        counter = readLeUint32(packet.getBuffer(), 4);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(3), counter);

        payloadSize = readLeUint32(packet.getBuffer(), 8);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(15), payloadSize);

        channelId = readLeUint32(packet.getBuffer(), 12);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

        timestamp = readLeUint64(packet.getBuffer(), 16);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), timestamp);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(27), packet.getSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x41), packet.getBuffer()[24]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[25]);
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x43), packet.getBuffer()[26]);

        CPPUNIT_ASSERT(source.readPacket(packet));
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

        source.close();
    }

    void testDataPath()
    {
        {
            writeFile(tempFilePath, {});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), packet.getSize());

            source.close();
        }

        {
            writeFile(tempFilePath, {0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), packet.getSize());

            std::uint32_t type = readLeUint32(packet.getBuffer(), 0);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(17), type);

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(13), payloadSize);

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            std::uint64_t dataOffset = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), dataOffset);
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[24]);

            source.close();
        }

        {
            std::vector<std::uint8_t> content(100, 0x41);
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(50);
            CPPUNIT_ASSERT(!source.readPacket(packet));

            source.close();
        }

        {
            writeFile(tempFilePath, {0x41});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(0);
            CPPUNIT_ASSERT(!source.readPacket(packet));

            source.close();
        }

        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(26);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(26), packet.getSize());
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x41), packet.getBuffer()[24]);
            CPPUNIT_ASSERT_EQUAL(static_cast<char>(0x42), packet.getBuffer()[25]);

            source.close();
        }

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
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(280), packet.getSize());

            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (size_t i = 0; i < content.size(); ++i)
            {
                CPPUNIT_ASSERT_EQUAL(content[i], buf[24 + i]);
            }

            source.close();
        }

        {
            std::vector<std::uint8_t> content(10000, 0x55);
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(20000);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10024), packet.getSize());

            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (size_t i = 0; i < 100; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x55), buf[24 + i]);
            }

            source.close();
        }

        {
            std::string webvttContent = "WEBVTT\n\n00:00:01.000 --> 00:00:04.000\nHello World\n\n00:00:05.000 --> 00:00:08.000\nSubtitle Test\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        {
            std::string webvttContent = "WEBVTT - Test File\nKind: captions\nLanguage: en\n\n00:00:01.000 --> 00:00:04.000 align:middle line:90%\n<v Speaker>Test Caption</v>\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        {
            std::string webvttContent = "WEBVTT\n\nNOTE Test note\n\n00:00:01.000 --> 00:00:04.000\n<b>Bold</b> <i>Italic</i> <u>Underline</u>\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet));

            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);
            CPPUNIT_ASSERT(readContent.find("<b>Bold</b>") != std::string::npos);
            CPPUNIT_ASSERT(readContent.find("<i>Italic</i>") != std::string::npos);
            CPPUNIT_ASSERT(readContent.find("<u>Underline</u>") != std::string::npos);

            source.close();
        }

        {
            std::string webvttContent = "WEBVTT\n\n00:00:01.000 --> 00:00:04.000\n";
            webvttContent += "Hello 世界 🌍 Testing UTF-8\n";
            std::vector<std::uint8_t> content(webvttContent.begin(), webvttContent.end());
            writeFile(tempFilePath, content);

            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(10240);
            CPPUNIT_ASSERT(source.readPacket(packet));
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(24 + webvttContent.size()), packet.getSize());

            std::string readContent(packet.getBuffer() + 24, webvttContent.size());
            CPPUNIT_ASSERT_EQUAL(webvttContent, readContent);

            source.close();
        }

        {
            writeFile(tempFilePath, {0xAA, 0xBB, 0xCC, 0xDD});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            std::uint32_t channelId = readLeUint32(packet.getBuffer(), 12);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), channelId);

            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[12]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[13]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[14]);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[15]);

            source.close();
        }

        {
            writeFile(tempFilePath, {0xFF, 0xEE, 0xDD});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            std::uint64_t dataOffset = readLeUint64(packet.getBuffer(), 16);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint64_t>(0), dataOffset);

            const std::uint8_t* buf = reinterpret_cast<const std::uint8_t*>(packet.getBuffer());
            for (int i = 16; i < 24; ++i)
            {
                CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x00), buf[i]);
            }

            source.close();
        }

        {
            std::vector<std::uint8_t> content = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
            writeFile(tempFilePath, content);
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(source.readPacket(packet));

            std::uint32_t payloadSize = readLeUint32(packet.getBuffer(), 8);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(19), payloadSize);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(31), packet.getSize());

            source.close();
        }
    }

    void testReadGuards()
    {
        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());
            source.close();

            DataPacket packet(1024);
            CPPUNIT_ASSERT(!source.readPacket(packet));
        }

        {
            writeFile(tempFilePath, {0x41, 0x42});
            WebvttFileSource source(tempFilePath);

            DataPacket packet(1024);
            CPPUNIT_ASSERT(!source.readPacket(packet));
        }

        {
            WebvttFileSource source("/non/existent/path/file.vtt");
            CPPUNIT_ASSERT(!source.open());

            DataPacket packet(1024);
            CPPUNIT_ASSERT(!source.readPacket(packet));
        }
    }

    void testCloseBehavior()
    {
        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());
            CPPUNIT_ASSERT(source.getFileHandle() != -1);

            source.close();
            CPPUNIT_ASSERT_EQUAL(-1, source.getFileHandle());
        }

        {
            writeFile(tempFilePath, {0x41, 0x42, 0x43});
            WebvttFileSource source(tempFilePath);
            CPPUNIT_ASSERT(source.open());

            source.close();
            source.close();

            CPPUNIT_ASSERT_EQUAL(-1, source.getFileHandle());
        }
    }

    void testStatAfterOpen()
    {
        writeFile(tempFilePath, {0x41, 0x42, 0x43});

        WebvttFileSource source(tempFilePath);
        CPPUNIT_ASSERT(source.open());

        removeFileNoThrow(tempFilePath);

        DataPacket packet(1024);
        CPPUNIT_ASSERT(!source.readPacket(packet));

        source.close();
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebvttFileSourceTest);
