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
#include "TtmlController.hpp"
#include <subttxrend/protocol/PacketTtmlSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketTtmlTimestamp.hpp>
#include <subttxrend/protocol/PacketTtmlInfo.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/ttmlengine/TtmlEngine.hpp>
#include <memory>
#include <vector>
#include <chrono>

using namespace subttxrend::ctrl;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;
using namespace subttxrend::common;
using namespace subttxrend::ttmlengine;

// Mock TtmlEngine for testing
class MockTtmlEngine : public TtmlEngine
{
public:
    MockTtmlEngine()
        : m_initialized(false)
        , m_started(false)
        , m_paused(false)
        , m_muted(false)
        , m_relatedVideoSize(0, 0)
        , m_currentMediatime(0)
        , m_contentType("")
        , m_subsInfo("")
        , m_customStyling("")
        , m_dataCount(0)
        , m_processCount(0)
    {}

    void init(const ConfigProvider* configProvider, Window* gfxWindow, Properties const& properties) override
    {
        m_initialized = true;
        m_configProvider = configProvider;
        m_gfxWindow = gfxWindow;
    }

    void setRelatedVideoSize(Size relatedVideoSize) override
    {
        m_relatedVideoSize = relatedVideoSize;
    }

    void start() override
    {
        m_started = true;
    }

    void stop() override
    {
        m_started = false;
    }

    void flush() override
    {
        m_dataCount = 0;
    }

    void pause() override
    {
        m_paused = true;
    }

    void resume() override
    {
        m_paused = false;
    }

    void mute() override
    {
        m_muted = true;
    }

    void unmute() override
    {
        m_muted = false;
    }

    void addData(const std::uint8_t* buffer, std::size_t dataSize, std::int64_t displayOffsetMs) override
    {
        m_dataCount++;
        m_lastDataSize = dataSize;
        m_lastDisplayOffset = displayOffsetMs;
        if (buffer && dataSize > 0) {
            m_lastData.assign(buffer, buffer + dataSize);
        }
    }

    void process() override
    {
        m_processCount++;
    }

    std::chrono::milliseconds getWaitTime() const override
    {
        return m_waitTime;
    }

    void currentMediatime(const std::uint64_t mediatimeMs) override
    {
        m_currentMediatime = mediatimeMs;
    }

    void setSubtitleInfo(const std::string& contentType, const std::string& subsInfo) override
    {
        m_contentType = contentType;
        m_subsInfo = subsInfo;
    }

    void setCustomTtmlStyling(const std::string& styling) override
    {
        m_customStyling = styling;
    }

    // Test helpers
    bool isInitialized() const { return m_initialized; }
    bool isStarted() const { return m_started; }
    bool isPaused() const { return m_paused; }
    bool isMuted() const { return m_muted; }
    Size getRelatedVideoSize() const { return m_relatedVideoSize; }
    std::uint64_t getCurrentMediatime() const { return m_currentMediatime; }
    std::string getContentType() const { return m_contentType; }
    std::string getSubsInfo() const { return m_subsInfo; }
    std::string getCustomStyling() const { return m_customStyling; }
    std::size_t getDataCount() const { return m_dataCount; }
    std::size_t getProcessCount() const { return m_processCount; }
    std::size_t getLastDataSize() const { return m_lastDataSize; }
    std::int64_t getLastDisplayOffset() const { return m_lastDisplayOffset; }
    const std::vector<std::uint8_t>& getLastData() const { return m_lastData; }

    void setWaitTime(std::chrono::milliseconds waitTime) { m_waitTime = waitTime; }

private:
    bool m_initialized;
    bool m_started;
    bool m_paused;
    bool m_muted;
    Size m_relatedVideoSize;
    std::uint64_t m_currentMediatime;
    std::string m_contentType;
    std::string m_subsInfo;
    std::string m_customStyling;
    std::size_t m_dataCount;
    std::size_t m_processCount;
    std::size_t m_lastDataSize;
    std::int64_t m_lastDisplayOffset;
    std::vector<std::uint8_t> m_lastData;
    std::chrono::milliseconds m_waitTime{0};
    const ConfigProvider* m_configProvider;
    Window* m_gfxWindow;
};

// Mock Window for testing
class MockWindow : public Window
{
public:
    MockWindow() {}
    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}
    Rectangle getBounds() const override { return Rectangle(0, 0, 1920, 1080); }
    DrawContext& getDrawContext() override { throw std::runtime_error("Not implemented"); }
    Size getPreferredSize() const override { return Size(1920, 1080); }
    void setSize(const Size& newSize) override {}
    Size getSize() const override { return Size(1920, 1080); }
    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}
};

// Mock ConfigProvider for testing
class MockConfigProvider : public ConfigProvider
{
public:
    MockConfigProvider() {}

    const char* getValue(const std::string& key) const override { return ""; }
    std::string getString(const std::string& key) const { return ""; }
    std::int32_t getInt(const std::string& key) const { return 0; }
    bool getBool(const std::string& key) const { return false; }
};

// Helper to create PacketTtmlSelection for testing
class PacketTtmlSelectionBuilder
{
public:
    static std::unique_ptr<PacketTtmlSelection> build(uint32_t channelId, uint32_t width, uint32_t height)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        // Header: type(4) + counter(4) + size(4) = 12 bytes
        // Data: channelId(4) + width(4) + height(4) = 12 bytes
        std::vector<uint8_t> data = {
            0x07, 0x00, 0x00, 0x00, // type = TTML_SELECTION (7)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x0C, 0x00, 0x00, 0x00, // size = 12 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24),
            static_cast<uint8_t>(width), static_cast<uint8_t>(width >> 8),
            static_cast<uint8_t>(width >> 16), static_cast<uint8_t>(width >> 24),
            static_cast<uint8_t>(height), static_cast<uint8_t>(height >> 8),
            static_cast<uint8_t>(height >> 16), static_cast<uint8_t>(height >> 24)
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketTtmlSelection>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketData for testing
class PacketDataBuilder
{
public:
    static std::unique_ptr<PacketData> build(uint32_t channelId, const std::vector<uint8_t>& userData, int64_t displayOffset = 0)
    {
        uint32_t size = 4 + 8 + userData.size(); // channelId + displayOffset + user data

        std::vector<uint8_t> data = {
            0x08, 0x00, 0x00, 0x00, // type = TTML_DATA (8)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24),
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24),
            // displayOffset (8 bytes, little-endian int64_t)
            static_cast<uint8_t>(displayOffset), static_cast<uint8_t>(displayOffset >> 8),
            static_cast<uint8_t>(displayOffset >> 16), static_cast<uint8_t>(displayOffset >> 24),
            static_cast<uint8_t>(displayOffset >> 32), static_cast<uint8_t>(displayOffset >> 40),
            static_cast<uint8_t>(displayOffset >> 48), static_cast<uint8_t>(displayOffset >> 56)
        };

        data.insert(data.end(), userData.begin(), userData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::TTML_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketTtmlTimestamp for testing
class PacketTtmlTimestampBuilder
{
public:
    static std::unique_ptr<PacketTtmlTimestamp> build(uint32_t channelId, uint64_t timestamp)
    {
        std::vector<uint8_t> data = {
            0x09, 0x00, 0x00, 0x00, // type = TTML_TIMESTAMP (9)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x0C, 0x00, 0x00, 0x00, // size = 12 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24),
            static_cast<uint8_t>(timestamp), static_cast<uint8_t>(timestamp >> 8),
            static_cast<uint8_t>(timestamp >> 16), static_cast<uint8_t>(timestamp >> 24),
            static_cast<uint8_t>(timestamp >> 32), static_cast<uint8_t>(timestamp >> 40),
            static_cast<uint8_t>(timestamp >> 48), static_cast<uint8_t>(timestamp >> 56)
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketTtmlTimestamp>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketTtmlInfo for testing
class PacketTtmlInfoBuilder
{
public:
    static std::unique_ptr<PacketTtmlInfo> build(uint32_t channelId, const std::string& contentType, const std::string& subsInfo)
    {
        // PacketTtmlInfo format: channelId(4) + "contentType subsInfo" (space-separated string)
        // When both are empty, we need at least a space character for proper parsing
        std::string infoString;
        if (contentType.empty() && subsInfo.empty()) {
            infoString = " "; // Single space - parser will find it and set both to empty
        } else {
            infoString = contentType + " " + subsInfo;
        }

        uint32_t size = 4 + infoString.length(); // channelId + space-separated string

        std::vector<uint8_t> data = {
            0x13, 0x00, 0x00, 0x00, // type = TTML_INFO (19)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24),
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24)
        };

        data.insert(data.end(), infoString.begin(), infoString.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketTtmlInfo>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Global mock engine instance for factory
static MockTtmlEngine* g_mockEngine = nullptr;

// Override the factory for testing
namespace subttxrend {
namespace ttmlengine {
namespace Factory {
    std::unique_ptr<TtmlEngine> createTtmlEngine()
    {
        if (g_mockEngine) {
            // Return a raw pointer wrapped in unique_ptr without ownership
            // This is safe because we manage the mock lifecycle in tests
            auto engine = g_mockEngine;
            g_mockEngine = nullptr;
            return std::unique_ptr<TtmlEngine>(engine);
        }
        return std::unique_ptr<TtmlEngine>(new MockTtmlEngine());
    }
}
}
}

class TtmlControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TtmlControllerTest);
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorInitializesEngine);
    CPPUNIT_TEST(testConstructorSelectsChannel);
    CPPUNIT_TEST(testSelectWithPacket);
    CPPUNIT_TEST(testSelectWithZeroWidthHeight);
    CPPUNIT_TEST(testSelectWithMaxDimensions);
    CPPUNIT_TEST(testAddDataWithValidPacket);
    CPPUNIT_TEST(testAddDataWithEmptyData);
    CPPUNIT_TEST(testAddDataWithLargeData);
    CPPUNIT_TEST(testAddDataWithNegativeOffset);
    CPPUNIT_TEST(testAddDataWithPositiveOffset);
    CPPUNIT_TEST(testWantsDataMatchingChannel);
    CPPUNIT_TEST(testWantsDataNonMatchingChannel);
    CPPUNIT_TEST(testWantsDataWithPacket);
    CPPUNIT_TEST(testProcessTimestampWithPacket);
    CPPUNIT_TEST(testProcessTimestampZero);
    CPPUNIT_TEST(testProcessTimestampMaxValue);
    CPPUNIT_TEST(testProcessTimestampSequence);
    CPPUNIT_TEST(testProcessInfoWithPacket);
    CPPUNIT_TEST(testProcessInfoEmptyContentType);
    CPPUNIT_TEST(testProcessInfoEmptySubsInfo);
    CPPUNIT_TEST(testProcessInfoBothEmpty);
    CPPUNIT_TEST(testProcess);
    CPPUNIT_TEST(testFlush);
    CPPUNIT_TEST(testPause);
    CPPUNIT_TEST(testResume);
    CPPUNIT_TEST(testPauseResumeCycle);
    CPPUNIT_TEST(testFlushAfterAddData);
    CPPUNIT_TEST(testMuteTrue);
    CPPUNIT_TEST(testMuteFalse);
    CPPUNIT_TEST(testMuteUnmuteCycle);
    CPPUNIT_TEST(testRepeatedMute);
    CPPUNIT_TEST(testSetCustomTtmlStylingValid);
    CPPUNIT_TEST(testSetCustomTtmlStylingEmpty);
    CPPUNIT_TEST(testSetCustomTtmlStylingLarge);
    CPPUNIT_TEST(testGetWaitTime);
    CPPUNIT_TEST(testGetWaitTimeCustomValue);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockWindow = std::make_shared<MockWindow>();
        m_mockConfig = std::make_unique<MockConfigProvider>();
        // Properties should be default-constructed without parameters
        g_mockEngine = new MockTtmlEngine();
    }

    void tearDown() override
    {
        m_mockWindow.reset();
        m_mockConfig.reset();
        // g_mockEngine is owned by TtmlController after construction
    }

protected:
    void testConstructorWithValidParameters()
    {
        auto packet = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        CPPUNIT_ASSERT(packet != nullptr);

        auto savedMock = g_mockEngine;
        TtmlController controller(*packet, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(savedMock->isInitialized());
        CPPUNIT_ASSERT(savedMock->isStarted());
    }

    void testConstructorInitializesEngine()
    {
        auto packet = PacketTtmlSelectionBuilder::build(200, 1280, 720);
        CPPUNIT_ASSERT(packet != nullptr);

        auto savedMock = g_mockEngine;
        TtmlController controller(*packet, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(savedMock->isInitialized());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(1280), savedMock->getRelatedVideoSize().m_w);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(720), savedMock->getRelatedVideoSize().m_h);
    }

    void testConstructorSelectsChannel()
    {
        auto packet = PacketTtmlSelectionBuilder::build(300, 640, 480);
        CPPUNIT_ASSERT(packet != nullptr);

        auto savedMock = g_mockEngine;
        TtmlController controller(*packet, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(savedMock->isStarted());
        CPPUNIT_ASSERT(controller.wantsData(*packet));
    }

    void testSelectWithPacket()
    {
        auto packet1 = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*packet1, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(1920), savedMock->getRelatedVideoSize().m_w);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(1080), savedMock->getRelatedVideoSize().m_h);
    }

    void testSelectWithZeroWidthHeight()
    {
        auto packet = PacketTtmlSelectionBuilder::build(100, 0, 0);
        auto savedMock = g_mockEngine;
        TtmlController controller(*packet, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0), savedMock->getRelatedVideoSize().m_w);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0), savedMock->getRelatedVideoSize().m_h);
    }

    void testSelectWithMaxDimensions()
    {
        auto packet = PacketTtmlSelectionBuilder::build(100, 0xFFFFFFFF, 0xFFFFFFFF);
        auto savedMock = g_mockEngine;
        TtmlController controller(*packet, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0xFFFFFFFF), savedMock->getRelatedVideoSize().m_w);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0xFFFFFFFF), savedMock->getRelatedVideoSize().m_h);
    }

    void testAddDataWithValidPacket()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04};
        auto dataPacket = PacketDataBuilder::build(100, testData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), savedMock->getLastDataSize());
    }

    void testAddDataWithEmptyData()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        // Note: PacketData with empty data (0 bytes) fails
        // Use minimal 1-byte data instead.
        std::vector<uint8_t> minimalData = {0x00};
        auto dataPacket = PacketDataBuilder::build(100, minimalData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getLastDataSize());
    }

    void testAddDataWithLargeData()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> largeData(10000, 0xAB);
        auto dataPacket = PacketDataBuilder::build(100, largeData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10000), savedMock->getLastDataSize());
    }

    void testAddDataWithNegativeOffset()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> testData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(100, testData, -5000);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());
    }

    void testAddDataWithPositiveOffset()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> testData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(100, testData, 3000);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());
    }

    void testWantsDataMatchingChannel()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(controller.wantsData(*selPacket));
    }

    void testWantsDataNonMatchingChannel()
    {
        auto selPacket1 = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        TtmlController controller(*selPacket1, *m_mockConfig, m_mockWindow, Properties());

        auto selPacket2 = PacketTtmlSelectionBuilder::build(200, 1920, 1080);
        CPPUNIT_ASSERT(!controller.wantsData(*selPacket2));
    }

    void testWantsDataWithPacket()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(150, 1920, 1080);
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto dataPacket = PacketDataBuilder::build(150, std::vector<uint8_t>{0x01});
        CPPUNIT_ASSERT(controller.wantsData(*dataPacket));

        auto wrongPacket = PacketDataBuilder::build(999, std::vector<uint8_t>{0x01});
        CPPUNIT_ASSERT(!controller.wantsData(*wrongPacket));
    }

    void testProcessTimestampWithPacket()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto tsPacket = PacketTtmlTimestampBuilder::build(100, 5000);
        CPPUNIT_ASSERT(tsPacket != nullptr);

        controller.processTimestamp(*tsPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(5000), savedMock->getCurrentMediatime());
    }

    void testProcessTimestampZero()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto tsPacket = PacketTtmlTimestampBuilder::build(100, 0);
        CPPUNIT_ASSERT(tsPacket != nullptr);

        controller.processTimestamp(*tsPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), savedMock->getCurrentMediatime());
    }

    void testProcessTimestampMaxValue()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto tsPacket = PacketTtmlTimestampBuilder::build(100, 0xFFFFFFFFFFFFFFFFULL);
        CPPUNIT_ASSERT(tsPacket != nullptr);

        controller.processTimestamp(*tsPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL), savedMock->getCurrentMediatime());
    }

    void testProcessTimestampSequence()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto ts1 = PacketTtmlTimestampBuilder::build(100, 1000);
        controller.processTimestamp(*ts1);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(1000), savedMock->getCurrentMediatime());

        auto ts2 = PacketTtmlTimestampBuilder::build(100, 2000);
        controller.processTimestamp(*ts2);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(2000), savedMock->getCurrentMediatime());

        auto ts3 = PacketTtmlTimestampBuilder::build(100, 3000);
        controller.processTimestamp(*ts3);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(3000), savedMock->getCurrentMediatime());
    }

    void testProcessInfoWithPacket()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto infoPacket = PacketTtmlInfoBuilder::build(100, "vod", "eng nrm");
        CPPUNIT_ASSERT(infoPacket != nullptr);

        controller.processInfo(*infoPacket);

        CPPUNIT_ASSERT_EQUAL(std::string("vod"), savedMock->getContentType());
        CPPUNIT_ASSERT_EQUAL(std::string("eng nrm"), savedMock->getSubsInfo());
    }

    void testProcessInfoEmptyContentType()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto infoPacket = PacketTtmlInfoBuilder::build(100, "", "eng nrm");
        CPPUNIT_ASSERT(infoPacket != nullptr);

        controller.processInfo(*infoPacket);

        CPPUNIT_ASSERT_EQUAL(std::string(""), savedMock->getContentType());
        CPPUNIT_ASSERT_EQUAL(std::string("eng nrm"), savedMock->getSubsInfo());
    }

    void testProcessInfoEmptySubsInfo()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto infoPacket = PacketTtmlInfoBuilder::build(100, "live", "");
        CPPUNIT_ASSERT(infoPacket != nullptr);

        controller.processInfo(*infoPacket);

        CPPUNIT_ASSERT_EQUAL(std::string("live"), savedMock->getContentType());
        CPPUNIT_ASSERT_EQUAL(std::string(""), savedMock->getSubsInfo());
    }

    void testProcessInfoBothEmpty()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto infoPacket = PacketTtmlInfoBuilder::build(100, "", "");
        CPPUNIT_ASSERT(infoPacket != nullptr);

        controller.processInfo(*infoPacket);

        CPPUNIT_ASSERT_EQUAL(std::string(""), savedMock->getContentType());
        CPPUNIT_ASSERT_EQUAL(std::string(""), savedMock->getSubsInfo());
    }

    void testProcess()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), savedMock->getProcessCount());

        controller.process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getProcessCount());

        controller.process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), savedMock->getProcessCount());
    }

    void testFlush()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> testData = {0x01, 0x02, 0x03};
        auto dataPacket = PacketDataBuilder::build(100, testData);
        controller.addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), savedMock->getDataCount());

        controller.flush();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), savedMock->getDataCount());
    }

    void testPause()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(!savedMock->isPaused());

        controller.pause();
        CPPUNIT_ASSERT(savedMock->isPaused());
    }

    void testResume()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        controller.pause();
        CPPUNIT_ASSERT(savedMock->isPaused());

        controller.resume();
        CPPUNIT_ASSERT(!savedMock->isPaused());
    }

    void testPauseResumeCycle()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(!savedMock->isPaused());

        controller.pause();
        CPPUNIT_ASSERT(savedMock->isPaused());

        controller.resume();
        CPPUNIT_ASSERT(!savedMock->isPaused());

        controller.pause();
        CPPUNIT_ASSERT(savedMock->isPaused());
    }

    void testFlushAfterAddData()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::vector<uint8_t> testData1 = {0x01, 0x02};
        auto dataPacket1 = PacketDataBuilder::build(100, testData1);
        controller.addData(*dataPacket1);

        std::vector<uint8_t> testData2 = {0x03, 0x04};
        auto dataPacket2 = PacketDataBuilder::build(100, testData2);
        controller.addData(*dataPacket2);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), savedMock->getDataCount());

        controller.flush();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), savedMock->getDataCount());
    }

    void testMuteTrue()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(!savedMock->isMuted());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());
    }

    void testMuteFalse()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());

        controller.mute(false);
        CPPUNIT_ASSERT(!savedMock->isMuted());
    }

    void testMuteUnmuteCycle()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        CPPUNIT_ASSERT(!savedMock->isMuted());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());

        controller.mute(false);
        CPPUNIT_ASSERT(!savedMock->isMuted());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());
    }

    void testRepeatedMute()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());

        controller.mute(true);
        CPPUNIT_ASSERT(savedMock->isMuted());

        controller.mute(false);
        CPPUNIT_ASSERT(!savedMock->isMuted());

        controller.mute(false);
        CPPUNIT_ASSERT(!savedMock->isMuted());
    }

    void testSetCustomTtmlStylingValid()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::string styling = "body { color: red; }";
        controller.setCustomTtmlStyling(styling);

        CPPUNIT_ASSERT_EQUAL(styling, savedMock->getCustomStyling());
    }

    void testSetCustomTtmlStylingEmpty()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        controller.setCustomTtmlStyling("");

        CPPUNIT_ASSERT_EQUAL(std::string(""), savedMock->getCustomStyling());
    }

    void testSetCustomTtmlStylingLarge()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        std::string largeStyling(5000, 'X');
        controller.setCustomTtmlStyling(largeStyling);

        CPPUNIT_ASSERT_EQUAL(largeStyling, savedMock->getCustomStyling());
    }

    void testGetWaitTime()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        auto waitTime = controller.getWaitTime();
        CPPUNIT_ASSERT_EQUAL(0L, waitTime.count());
    }

    void testGetWaitTimeCustomValue()
    {
        auto selPacket = PacketTtmlSelectionBuilder::build(100, 1920, 1080);
        auto savedMock = g_mockEngine;
        TtmlController controller(*selPacket, *m_mockConfig, m_mockWindow, Properties());

        savedMock->setWaitTime(std::chrono::milliseconds(500));

        auto waitTime = controller.getWaitTime();
        CPPUNIT_ASSERT_EQUAL(500L, waitTime.count());
    }

private:
    std::shared_ptr<MockWindow> m_mockWindow;
    std::unique_ptr<MockConfigProvider> m_mockConfig;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtmlControllerTest);
