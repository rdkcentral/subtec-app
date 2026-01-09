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
#include "WebvttController.hpp"
#include <subttxrend/protocol/PacketWebvttSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketWebvttTimestamp.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/webvttengine/WebvttEngine.hpp>
#include <subttxrend/webvttengine/WebVTTAttributes.hpp>
#include <memory>
#include <vector>
#include <chrono>

using namespace subttxrend::ctrl;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;
using namespace subttxrend::common;
using namespace subttxrend::webvttengine;

// Mock WebvttEngine for testing
class MockWebvttEngine : public WebvttEngine
{
public:
    MockWebvttEngine()
        : m_initialized(false)
        , m_started(false)
        , m_stopped(false)
        , m_paused(false)
        , m_muted(false)
        , m_relatedVideoSize(0, 0)
        , m_currentMediatime(0)
        , m_dataCount(0)
        , m_processCount(0)
        , m_lastDataSize(0)
        , m_lastDisplayOffset(0)
        , m_waitTime(0)
        , m_attributesSet(false)
    {}

    void init(const ConfigProvider* configProvider, std::weak_ptr<Window> gfxWindow) override
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
        m_stopped = false;
    }

    void stop() override
    {
        m_stopped = true;
        m_started = false;
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

    void setAttributes(const WebVTTAttributes& attributes) override
    {
        m_attributesSet = true;
        m_lastAttributes = attributes;
    }

    void addData(const std::uint8_t* buffer, std::size_t dataSize, std::int64_t displayOffsetMs) override
    {
        m_dataCount++;
        m_lastDataSize = dataSize;
        m_lastDisplayOffset = displayOffsetMs;
        if (buffer && dataSize > 0) {
            m_lastData.assign(buffer, buffer + dataSize);
        } else {
            m_lastData.clear();
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

    // Test helpers
    bool isInitialized() const { return m_initialized; }
    bool isStarted() const { return m_started; }
    bool isStopped() const { return m_stopped; }
    bool isPaused() const { return m_paused; }
    bool isMuted() const { return m_muted; }
    Size getRelatedVideoSize() const { return m_relatedVideoSize; }
    std::uint64_t getCurrentMediatime() const { return m_currentMediatime; }
    std::size_t getDataCount() const { return m_dataCount; }
    std::size_t getProcessCount() const { return m_processCount; }
    std::size_t getLastDataSize() const { return m_lastDataSize; }
    std::int64_t getLastDisplayOffset() const { return m_lastDisplayOffset; }
    const std::vector<std::uint8_t>& getLastData() const { return m_lastData; }
    bool wasAttributesSet() const { return m_attributesSet; }

    void setWaitTime(std::chrono::milliseconds waitTime) { m_waitTime = waitTime; }
    void resetCounters() { m_dataCount = 0; m_processCount = 0; }

private:
    bool m_initialized;
    bool m_started;
    bool m_stopped;
    bool m_paused;
    bool m_muted;
    Size m_relatedVideoSize;
    std::uint64_t m_currentMediatime;
    std::size_t m_dataCount;
    std::size_t m_processCount;
    std::size_t m_lastDataSize;
    std::int64_t m_lastDisplayOffset;
    std::vector<std::uint8_t> m_lastData;
    std::chrono::milliseconds m_waitTime;
    bool m_attributesSet;
    WebVTTAttributes m_lastAttributes;
    const ConfigProvider* m_configProvider;
    std::weak_ptr<Window> m_gfxWindow;
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

// Helper to create PacketWebvttSelection for testing
class PacketWebvttSelectionBuilder
{
public:
    static std::unique_ptr<PacketWebvttSelection> build(uint32_t channelId, uint32_t width, uint32_t height)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        // Header: type(4) + counter(4) + size(4) = 12 bytes
        // Data: channelId(4) + width(4) + height(4) = 12 bytes
        std::vector<uint8_t> data = {
            0x0F, 0x00, 0x00, 0x00, // type = WEBVTT_SELECTION (15)
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
        auto packet = std::make_unique<PacketWebvttSelection>();
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
            0x10, 0x00, 0x00, 0x00, // type = WEBVTT_DATA (16)
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
        auto packet = std::make_unique<PacketData>(Packet::Type::WEBVTT_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketWebvttTimestamp for testing
class PacketWebvttTimestampBuilder
{
public:
    static std::unique_ptr<PacketWebvttTimestamp> build(uint32_t channelId, uint64_t timestamp)
    {
        std::vector<uint8_t> data = {
            0x11, 0x00, 0x00, 0x00, // type = WEBVTT_TIMESTAMP (17)
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
        auto packet = std::make_unique<PacketWebvttTimestamp>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketSetCCAttributes for testing
class PacketSetCCAttributesBuilder
{
public:
    static std::unique_ptr<PacketSetCCAttributes> build(uint32_t channelId,
                                                        const std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>>& attributes)
    {
        // Fixed size: channelId(4) + ccType(4) + attribType(4) + 14 values(56)
        uint32_t size = 68;

        std::vector<uint8_t> data = {
            0x12, 0x00, 0x00, 0x00, // type = SET_CC_ATTRIBUTES (18)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x44, 0x00, 0x00, 0x00, // size = 68
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24),
            0x00, 0x00, 0x00, 0x00  // ccType (0 for WebVTT)
        };

        // Build attribute type bitmask
        uint32_t attribTypeMask = 0;
        std::vector<uint32_t> attribValues(14, 0x11223344); // Default values

        for (const auto& attr : attributes) {
            uint32_t bitIndex = static_cast<uint32_t>(attr.first);
            if (bitIndex < 14) {
                attribTypeMask |= (1 << bitIndex);
                attribValues[bitIndex] = attr.second;
            }
        }

        // Add attribType bitmask
        data.push_back(static_cast<uint8_t>(attribTypeMask));
        data.push_back(static_cast<uint8_t>(attribTypeMask >> 8));
        data.push_back(static_cast<uint8_t>(attribTypeMask >> 16));
        data.push_back(static_cast<uint8_t>(attribTypeMask >> 24));

        // Add all 14 attribute values
        for (const auto& val : attribValues) {
            data.push_back(static_cast<uint8_t>(val));
            data.push_back(static_cast<uint8_t>(val >> 8));
            data.push_back(static_cast<uint8_t>(val >> 16));
            data.push_back(static_cast<uint8_t>(val >> 24));
        }

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSetCCAttributes>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Global mock engine instance for factory
static MockWebvttEngine* g_mockEngine = nullptr;

// Override the factory for testing
namespace subttxrend {
namespace webvttengine {
namespace Factory {
    std::unique_ptr<WebvttEngine> createWebvttEngine()
    {
        if (g_mockEngine) {
            auto engine = g_mockEngine;
            g_mockEngine = nullptr;
            return std::unique_ptr<WebvttEngine>(engine);
        }
        return std::unique_ptr<WebvttEngine>(new MockWebvttEngine());
    }
}
}
}

class WebvttControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(WebvttControllerTest);

    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorInitializesEngine);
    CPPUNIT_TEST(testConstructorSelectsChannel);
    CPPUNIT_TEST(testConstructorWithZeroDimensions);
    CPPUNIT_TEST(testConstructorWithLargeDimensions);
    CPPUNIT_TEST(testProcessDelegatesToEngine);
    CPPUNIT_TEST(testMultipleProcessCalls);
    CPPUNIT_TEST(testAddDataWithValidPacket);
    CPPUNIT_TEST(testAddDataWithEmptyData);
    CPPUNIT_TEST(testAddDataWithLargeData);
    CPPUNIT_TEST(testAddDataWithNegativeOffset);
    CPPUNIT_TEST(testAddDataWithZeroOffset);
    CPPUNIT_TEST(testAddDataWithLargePositiveOffset);
    CPPUNIT_TEST(testAddDataMultipleTimes);
    CPPUNIT_TEST(testGetWaitTimeReturnsEngineValue);
    CPPUNIT_TEST(testGetWaitTimeWithZero);
    CPPUNIT_TEST(testGetWaitTimeWithLargeValue);
    CPPUNIT_TEST(testProcessTimestampWithPacket);
    CPPUNIT_TEST(testProcessTimestampWithZero);
    CPPUNIT_TEST(testProcessTimestampWithMaxValue);
    CPPUNIT_TEST(testProcessTimestampSequence);
    CPPUNIT_TEST(testProcessTimestampWithDecreasingValues);
    CPPUNIT_TEST(testMuteTrue);
    CPPUNIT_TEST(testMuteFalse);
    CPPUNIT_TEST(testRepeatedMuteTrue);
    CPPUNIT_TEST(testPausePausesEngine);
    CPPUNIT_TEST(testResumeResumesEngine);
    CPPUNIT_TEST(testPauseWhenAlreadyPaused);
    CPPUNIT_TEST(testResumeWhenNotPaused);
    CPPUNIT_TEST(testMultiplePauseResumeCycles);
    CPPUNIT_TEST(testRepeatedMuteFalse);
    CPPUNIT_TEST(testRapidMuteUnmuteToggle);
    CPPUNIT_TEST(testWantsDataMatchingChannel);
    CPPUNIT_TEST(testWantsDataNonMatchingChannel);
    CPPUNIT_TEST(testWantsDataInactiveChannel);
    CPPUNIT_TEST(testWantsDataWithZeroChannelId);
    CPPUNIT_TEST(testWantsDataWithMaxChannelId);
    CPPUNIT_TEST(testProcessCCAttributesFontColor);
    CPPUNIT_TEST(testProcessCCAttributesBackgroundColor);
    CPPUNIT_TEST(testProcessCCAttributesFontSize);
    CPPUNIT_TEST(testProcessCCAttributesFontStyle);
    CPPUNIT_TEST(testProcessCCAttributesEdgeType);
    CPPUNIT_TEST(testProcessCCAttributesEdgeColor);
    CPPUNIT_TEST(testProcessCCAttributesFontOpacity);
    CPPUNIT_TEST(testProcessCCAttributesBackgroundOpacity);
    CPPUNIT_TEST(testProcessCCAttributesWindowColor);
    CPPUNIT_TEST(testProcessCCAttributesWindowOpacity);
    CPPUNIT_TEST(testProcessCCAttributesMultiple);
    CPPUNIT_TEST(testProcessCCAttributesEmpty);
    CPPUNIT_TEST(testProcessCCAttributesUnsupported);
    CPPUNIT_TEST(testProcessCCAttributesWithInvalidValues);
    CPPUNIT_TEST(testSelectWithValidParameters);
    CPPUNIT_TEST(testSelectWithZeroChannelId);
    CPPUNIT_TEST(testSelectMultipleChannels);
    CPPUNIT_TEST(testSelectSameChannelTwice);
    CPPUNIT_TEST(testFullWorkflowConstructSelectAddDataProcess);
    CPPUNIT_TEST(testTimestampInterleavedWithData);
    CPPUNIT_TEST(testPauseResumeAffectsProcessing);
    CPPUNIT_TEST(testMuteStateWithDataProcessing);
    CPPUNIT_TEST(testChannelSwitching);
    CPPUNIT_TEST(testAttributesChangeDuringPlayback);
    CPPUNIT_TEST(testAddDataWhilePaused);
    CPPUNIT_TEST(testMuteWhilePaused);
    CPPUNIT_TEST(testCombinedPauseAndMuteStates);
    CPPUNIT_TEST(testProcessTimestampBeforeData);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockEngine = new MockWebvttEngine();
        g_mockEngine = m_mockEngine;
        m_mockConfig = std::make_unique<MockConfigProvider>();
        m_mockWindow = std::make_shared<MockWindow>();
    }

    void tearDown() override
    {
        m_mockEngine = nullptr;
        g_mockEngine = nullptr;
        m_mockConfig.reset();
        m_mockWindow.reset();
    }

protected:
    void testConstructorWithValidParameters()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(controller != nullptr);
        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());
    }

    void testConstructorInitializesEngine()
    {
        auto packet = PacketWebvttSelectionBuilder::build(5, 1280, 720);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT_EQUAL(Size(1280, 720), m_mockEngine->getRelatedVideoSize());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());
    }

    void testConstructorSelectsChannel()
    {
        auto packet = PacketWebvttSelectionBuilder::build(42, 3840, 2160);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT_EQUAL(Size(3840, 2160), m_mockEngine->getRelatedVideoSize());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());

        // Verify channel is set by checking wantsData
        auto dataPacket = PacketWebvttSelectionBuilder::build(42, 1920, 1080);
        CPPUNIT_ASSERT(controller->wantsData(*dataPacket));
    }

    void testConstructorWithZeroDimensions()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 0, 0);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT_EQUAL(Size(0, 0), m_mockEngine->getRelatedVideoSize());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());
    }

    void testConstructorWithLargeDimensions()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 7680, 4320);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT_EQUAL(Size(7680, 4320), m_mockEngine->getRelatedVideoSize());
    }

    void testProcessDelegatesToEngine()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), m_mockEngine->getProcessCount());

        controller->process();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());
    }

    void testMultipleProcessCalls()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        for (int i = 0; i < 10; i++) {
            controller->process();
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), m_mockEngine->getProcessCount());
    }

    void testAddDataWithValidPacket()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        std::vector<uint8_t> userData = {0x01, 0x02, 0x03, 0x04};
        auto dataPacket = PacketDataBuilder::build(1, userData, 1000);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), m_mockEngine->getLastDataSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(1000), m_mockEngine->getLastDisplayOffset());

        // Verify data content
        const auto& lastData = m_mockEngine->getLastData();
        CPPUNIT_ASSERT_EQUAL(userData.size(), lastData.size());
        for (size_t i = 0; i < userData.size(); i++) {
            CPPUNIT_ASSERT_EQUAL(userData[i], lastData[i]);
        }
    }

    void testAddDataWithEmptyData()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Use minimal 1-byte data instead of empty (PacketData requires buffer to be set)
        std::vector<uint8_t> minimalData = {0x00};
        auto dataPacket = PacketDataBuilder::build(1, minimalData, 0);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getLastDataSize());
    }

    void testAddDataWithLargeData()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        std::vector<uint8_t> largeData(10000, 0xAB);
        auto dataPacket = PacketDataBuilder::build(1, largeData, 5000);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10000), m_mockEngine->getLastDataSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(5000), m_mockEngine->getLastDisplayOffset());
    }

    void testAddDataWithNegativeOffset()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        std::vector<uint8_t> userData = {0xFF, 0xEE};
        auto dataPacket = PacketDataBuilder::build(1, userData, -5000);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(-5000), m_mockEngine->getLastDisplayOffset());
    }

    void testAddDataWithZeroOffset()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        std::vector<uint8_t> userData = {0x11, 0x22};
        auto dataPacket = PacketDataBuilder::build(1, userData, 0);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(0), m_mockEngine->getLastDisplayOffset());
    }

    void testAddDataWithLargePositiveOffset()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        std::vector<uint8_t> userData = {0xAA, 0xBB};
        int64_t largeOffset = 999999999LL;
        auto dataPacket = PacketDataBuilder::build(1, userData, largeOffset);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(largeOffset, m_mockEngine->getLastDisplayOffset());
    }

    void testAddDataMultipleTimes()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        for (int i = 0; i < 5; i++) {
            std::vector<uint8_t> userData = {static_cast<uint8_t>(i)};
            auto dataPacket = PacketDataBuilder::build(1, userData, i * 1000);
            controller->addData(*dataPacket);
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), m_mockEngine->getDataCount());
    }

    void testGetWaitTimeReturnsEngineValue()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        m_mockEngine->setWaitTime(std::chrono::milliseconds(100));

        auto waitTime = controller->getWaitTime();

        CPPUNIT_ASSERT_EQUAL(100L, waitTime.count());
    }

    void testGetWaitTimeWithZero()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        m_mockEngine->setWaitTime(std::chrono::milliseconds(0));

        auto waitTime = controller->getWaitTime();

        CPPUNIT_ASSERT_EQUAL(0L, waitTime.count());
    }

    void testGetWaitTimeWithLargeValue()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        m_mockEngine->setWaitTime(std::chrono::milliseconds(999999));

        auto waitTime = controller->getWaitTime();

        CPPUNIT_ASSERT_EQUAL(999999L, waitTime.count());
    }

    void testProcessTimestampWithPacket()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        auto timestampPacket = PacketWebvttTimestampBuilder::build(1, 5000);
        CPPUNIT_ASSERT(timestampPacket != nullptr);

        controller->processTimestamp(*timestampPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(5000), m_mockEngine->getCurrentMediatime());
    }

    void testProcessTimestampWithZero()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        auto timestampPacket = PacketWebvttTimestampBuilder::build(1, 0);
        CPPUNIT_ASSERT(timestampPacket != nullptr);

        controller->processTimestamp(*timestampPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), m_mockEngine->getCurrentMediatime());
    }

    void testProcessTimestampWithMaxValue()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        uint64_t maxTimestamp = 0xFFFFFFFFFFFFFFFFULL;
        auto timestampPacket = PacketWebvttTimestampBuilder::build(1, maxTimestamp);
        CPPUNIT_ASSERT(timestampPacket != nullptr);

        controller->processTimestamp(*timestampPacket);

        CPPUNIT_ASSERT_EQUAL(maxTimestamp, m_mockEngine->getCurrentMediatime());
    }

    void testProcessTimestampSequence()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        uint64_t timestamps[] = {1000, 2000, 3000, 4000, 5000};

        for (auto ts : timestamps) {
            auto timestampPacket = PacketWebvttTimestampBuilder::build(1, ts);
            controller->processTimestamp(*timestampPacket);
            CPPUNIT_ASSERT_EQUAL(ts, m_mockEngine->getCurrentMediatime());
        }
    }

    void testProcessTimestampWithDecreasingValues()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        auto timestampPacket1 = PacketWebvttTimestampBuilder::build(1, 10000);
        controller->processTimestamp(*timestampPacket1);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(10000), m_mockEngine->getCurrentMediatime());

        auto timestampPacket2 = PacketWebvttTimestampBuilder::build(1, 5000);
        controller->processTimestamp(*timestampPacket2);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(5000), m_mockEngine->getCurrentMediatime());
    }

    void testMuteTrue()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(!m_mockEngine->isMuted());

        controller->mute(true);

        CPPUNIT_ASSERT(m_mockEngine->isMuted());
    }

    void testMuteFalse()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        controller->mute(true);
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        controller->mute(false);

        CPPUNIT_ASSERT(!m_mockEngine->isMuted());
    }

    void testRepeatedMuteTrue()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        controller->mute(true);
        controller->mute(true);
        controller->mute(true);

        CPPUNIT_ASSERT(m_mockEngine->isMuted());
    }

    void testPausePausesEngine()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(!m_mockEngine->isPaused());

        controller->pause();

        CPPUNIT_ASSERT(m_mockEngine->isPaused());
    }

    void testResumeResumesEngine()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        controller->pause();
        CPPUNIT_ASSERT(m_mockEngine->isPaused());

        controller->resume();

        CPPUNIT_ASSERT(!m_mockEngine->isPaused());
    }

    void testPauseWhenAlreadyPaused()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        controller->pause();
        CPPUNIT_ASSERT(m_mockEngine->isPaused());

        controller->pause();

        CPPUNIT_ASSERT(m_mockEngine->isPaused());
    }

    void testResumeWhenNotPaused()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(!m_mockEngine->isPaused());

        controller->resume();

        CPPUNIT_ASSERT(!m_mockEngine->isPaused());
    }

    void testMultiplePauseResumeCycles()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        for (int i = 0; i < 5; i++) {
            controller->pause();
            CPPUNIT_ASSERT(m_mockEngine->isPaused());

            controller->resume();
            CPPUNIT_ASSERT(!m_mockEngine->isPaused());
        }
    }

    void testRepeatedMuteFalse()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        controller->mute(false);
        controller->mute(false);
        controller->mute(false);

        CPPUNIT_ASSERT(!m_mockEngine->isMuted());
    }

    void testRapidMuteUnmuteToggle()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        for (int i = 0; i < 10; i++) {
            controller->mute(true);
            CPPUNIT_ASSERT(m_mockEngine->isMuted());

            controller->mute(false);
            CPPUNIT_ASSERT(!m_mockEngine->isMuted());
        }
    }

    void testWantsDataMatchingChannel()
    {
        auto packet = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        auto dataPacket = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        CPPUNIT_ASSERT(controller->wantsData(*dataPacket));
    }

    void testWantsDataNonMatchingChannel()
    {
        auto packet = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        auto dataPacket = PacketWebvttSelectionBuilder::build(10, 1920, 1080);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        CPPUNIT_ASSERT(!controller->wantsData(*dataPacket));
    }

    void testWantsDataInactiveChannel()
    {
        auto packet = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        auto dataPacket = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        CPPUNIT_ASSERT(controller->wantsData(*dataPacket));

        // Destroy controller which resets channel
        controller.reset();

        // Create new controller with different channel
        m_mockEngine = new MockWebvttEngine();
        g_mockEngine = m_mockEngine;
        auto packet2 = PacketWebvttSelectionBuilder::build(10, 1920, 1080);
        auto controller2 = std::make_unique<WebvttController>(*packet2, *m_mockConfig, m_mockWindow);

        // Old channel packet should not be wanted
        CPPUNIT_ASSERT(!controller2->wantsData(*dataPacket));
    }

    void testWantsDataWithZeroChannelId()
    {
        auto packet = PacketWebvttSelectionBuilder::build(0, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        auto dataPacket = PacketWebvttSelectionBuilder::build(0, 1920, 1080);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        CPPUNIT_ASSERT(controller->wantsData(*dataPacket));

        auto wrongPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        CPPUNIT_ASSERT(!controller->wantsData(*wrongPacket));
    }

    void testWantsDataWithMaxChannelId()
    {
        uint32_t maxChannelId = 0xFFFFFFFF;
        auto packet = PacketWebvttSelectionBuilder::build(maxChannelId, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        auto dataPacket = PacketWebvttSelectionBuilder::build(maxChannelId, 1920, 1080);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        CPPUNIT_ASSERT(controller->wantsData(*dataPacket));

        auto wrongPacket = PacketWebvttSelectionBuilder::build(maxChannelId - 1, 1920, 1080);
        CPPUNIT_ASSERT(!controller->wantsData(*wrongPacket));
    }

    void testProcessCCAttributesFontColor()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_COLOR, 0xFF0000}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesBackgroundColor()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR, 0x00FF00}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesFontSize()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_SIZE, 24}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesFontStyle()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_STYLE, 1}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesEdgeType()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::EDGE_TYPE, 2}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesEdgeColor()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::EDGE_COLOR, 0x0000FF}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesFontOpacity()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_OPACITY, 128}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesBackgroundOpacity()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY, 200}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesWindowColor()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::WIN_COLOR, 0xFFFF00}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesWindowOpacity()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::WIN_OPACITY, 255}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesMultiple()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_COLOR, 0xFF0000},
            {PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR, 0x00FF00},
            {PacketSetCCAttributes::CcAttribType::FONT_SIZE, 18},
            {PacketSetCCAttributes::CcAttribType::FONT_OPACITY, 255},
            {PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY, 128}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesEmpty()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes;

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        // Should not crash with empty attributes
        controller->processSetCCAttributesPacket(*ccPacket);

        // setAttributes should still be called even with empty list
        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesUnsupported()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        // Use an attribute type that's in the enum but might not be fully supported
        // Cast to CcAttribType to simulate unsupported type
        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {static_cast<PacketSetCCAttributes::CcAttribType>(999), 12345}
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        // Should not crash with unsupported attribute
        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testProcessCCAttributesWithInvalidValues()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes = {
            {PacketSetCCAttributes::CcAttribType::FONT_COLOR, 0xFFFFFFFF},  // Max value
            {PacketSetCCAttributes::CcAttribType::FONT_SIZE, 0},            // Zero
            {PacketSetCCAttributes::CcAttribType::FONT_OPACITY, 1000}       // Out of normal range
        };

        auto ccPacket = PacketSetCCAttributesBuilder::build(1, attributes);
        CPPUNIT_ASSERT(ccPacket != nullptr);

        // Should handle boundary values gracefully
        controller->processSetCCAttributesPacket(*ccPacket);

        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());
    }

    void testSelectWithValidParameters()
    {
        auto packet = PacketWebvttSelectionBuilder::build(15, 1280, 720);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());
        CPPUNIT_ASSERT_EQUAL(Size(1280, 720), m_mockEngine->getRelatedVideoSize());

        // Verify channel was set correctly
        auto testPacket = PacketWebvttSelectionBuilder::build(15, 1920, 1080);
        CPPUNIT_ASSERT(controller->wantsData(*testPacket));
    }

    void testSelectWithZeroChannelId()
    {
        auto packet = PacketWebvttSelectionBuilder::build(0, 1920, 1080);
        CPPUNIT_ASSERT(packet != nullptr);

        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());

        auto testPacket = PacketWebvttSelectionBuilder::build(0, 1920, 1080);
        CPPUNIT_ASSERT(controller->wantsData(*testPacket));
    }

    void testSelectMultipleChannels()
    {
        // First channel
        auto packet1 = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto controller1 = std::make_unique<WebvttController>(*packet1, *m_mockConfig, m_mockWindow);

        auto testPacket1 = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        CPPUNIT_ASSERT(controller1->wantsData(*testPacket1));

        controller1.reset();

        // Second channel with new mock
        m_mockEngine = new MockWebvttEngine();
        g_mockEngine = m_mockEngine;

        auto packet2 = PacketWebvttSelectionBuilder::build(10, 1280, 720);
        auto controller2 = std::make_unique<WebvttController>(*packet2, *m_mockConfig, m_mockWindow);

        auto testPacket2 = PacketWebvttSelectionBuilder::build(10, 1920, 1080);
        CPPUNIT_ASSERT(controller2->wantsData(*testPacket2));

        // Should not want old channel
        CPPUNIT_ASSERT(!controller2->wantsData(*testPacket1));
    }

    void testSelectSameChannelTwice()
    {
        auto packet1 = PacketWebvttSelectionBuilder::build(7, 1920, 1080);
        auto controller1 = std::make_unique<WebvttController>(*packet1, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isStarted());
        auto testPacket = PacketWebvttSelectionBuilder::build(7, 1920, 1080);
        CPPUNIT_ASSERT(controller1->wantsData(*testPacket));

        controller1.reset();

        // Create new controller with same channel
        m_mockEngine = new MockWebvttEngine();
        g_mockEngine = m_mockEngine;

        auto packet2 = PacketWebvttSelectionBuilder::build(7, 1920, 1080);
        auto controller2 = std::make_unique<WebvttController>(*packet2, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isStarted());
        CPPUNIT_ASSERT(controller2->wantsData(*testPacket));
    }

    void testFullWorkflowConstructSelectAddDataProcess()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(m_mockEngine->isInitialized());
        CPPUNIT_ASSERT(m_mockEngine->isStarted());

        // Add data
        std::vector<uint8_t> userData = {0x01, 0x02, 0x03, 0x04};
        auto dataPacket = PacketDataBuilder::build(1, userData, 1000);
        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());

        // Process
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());

        // Add timestamp
        auto timestampPacket = PacketWebvttTimestampBuilder::build(1, 5000);
        controller->processTimestamp(*timestampPacket);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(5000), m_mockEngine->getCurrentMediatime());

        // Process again
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getProcessCount());
    }

    void testTimestampInterleavedWithData()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Add data, timestamp, data, timestamp pattern
        std::vector<uint8_t> data1 = {0x01, 0x02};
        auto dataPacket1 = PacketDataBuilder::build(1, data1, 1000);
        controller->addData(*dataPacket1);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());

        auto timestampPacket1 = PacketWebvttTimestampBuilder::build(1, 2000);
        controller->processTimestamp(*timestampPacket1);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(2000), m_mockEngine->getCurrentMediatime());

        std::vector<uint8_t> data2 = {0x03, 0x04};
        auto dataPacket2 = PacketDataBuilder::build(1, data2, 3000);
        controller->addData(*dataPacket2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getDataCount());

        auto timestampPacket2 = PacketWebvttTimestampBuilder::build(1, 4000);
        controller->processTimestamp(*timestampPacket2);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(4000), m_mockEngine->getCurrentMediatime());
    }

    void testPauseResumeAffectsProcessing()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(!m_mockEngine->isPaused());

        // Process while not paused
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());

        // Pause
        controller->pause();
        CPPUNIT_ASSERT(m_mockEngine->isPaused());

        // Process while paused
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getProcessCount());

        // Resume
        controller->resume();
        CPPUNIT_ASSERT(!m_mockEngine->isPaused());

        // Process while resumed
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), m_mockEngine->getProcessCount());
    }

    void testMuteStateWithDataProcessing()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Mute
        controller->mute(true);
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Add data while muted
        std::vector<uint8_t> userData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(1, userData, 1000);
        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Process while muted
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Unmute
        controller->mute(false);
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());

        // Add more data while unmuted
        auto dataPacket2 = PacketDataBuilder::build(1, userData, 2000);
        controller->addData(*dataPacket2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());
    }

    void testChannelSwitching()
    {
        // Start with channel 5
        auto packet1 = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto controller1 = std::make_unique<WebvttController>(*packet1, *m_mockConfig, m_mockWindow);

        auto testPacket5 = PacketWebvttSelectionBuilder::build(5, 1920, 1080);
        auto testPacket10 = PacketWebvttSelectionBuilder::build(10, 1920, 1080);

        CPPUNIT_ASSERT(controller1->wantsData(*testPacket5));
        CPPUNIT_ASSERT(!controller1->wantsData(*testPacket10));

        // Add some data
        std::vector<uint8_t> userData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(5, userData, 1000);
        controller1->addData(*dataPacket);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());

        // Destroy and switch to channel 10
        controller1.reset();

        // Create new mock engine for new controller
        m_mockEngine = new MockWebvttEngine();
        g_mockEngine = m_mockEngine;

        auto packet2 = PacketWebvttSelectionBuilder::build(10, 1280, 720);
        auto controller2 = std::make_unique<WebvttController>(*packet2, *m_mockConfig, m_mockWindow);

        CPPUNIT_ASSERT(controller2->wantsData(*testPacket10));
        CPPUNIT_ASSERT(!controller2->wantsData(*testPacket5));
        CPPUNIT_ASSERT(m_mockEngine->isStarted());
    }

    void testAttributesChangeDuringPlayback()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Add initial data
        std::vector<uint8_t> userData = {0x01, 0x02};
        auto dataPacket1 = PacketDataBuilder::build(1, userData, 1000);
        controller->addData(*dataPacket1);
        controller->process();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());

        // Change attributes mid-playback
        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes1 = {
            {PacketSetCCAttributes::CcAttribType::FONT_COLOR, 0xFF0000}
        };
        auto ccPacket1 = PacketSetCCAttributesBuilder::build(1, attributes1);
        controller->processSetCCAttributesPacket(*ccPacket1);
        CPPUNIT_ASSERT(m_mockEngine->wasAttributesSet());

        // Add more data
        auto dataPacket2 = PacketDataBuilder::build(1, userData, 2000);
        controller->addData(*dataPacket2);
        controller->process();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getProcessCount());

        // Change attributes again
        std::vector<std::pair<PacketSetCCAttributes::CcAttribType, uint32_t>> attributes2 = {
            {PacketSetCCAttributes::CcAttribType::FONT_SIZE, 20},
            {PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR, 0x00FF00}
        };
        auto ccPacket2 = PacketSetCCAttributesBuilder::build(1, attributes2);
        controller->processSetCCAttributesPacket(*ccPacket2);

        // Continue playback
        auto dataPacket3 = PacketDataBuilder::build(1, userData, 3000);
        controller->addData(*dataPacket3);
        controller->process();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), m_mockEngine->getProcessCount());
    }

    void testAddDataWhilePaused()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Pause before adding data
        controller->pause();
        CPPUNIT_ASSERT(m_mockEngine->isPaused());

        // Add data while paused - should still work
        std::vector<uint8_t> userData = {0x01, 0x02, 0x03};
        auto dataPacket = PacketDataBuilder::build(1, userData, 1000);
        controller->addData(*dataPacket);

        // Verify data was added despite being paused
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), m_mockEngine->getLastDataSize());
        CPPUNIT_ASSERT(m_mockEngine->isPaused());

        // Resume and verify state
        controller->resume();
        CPPUNIT_ASSERT(!m_mockEngine->isPaused());

        // Add more data after resume
        auto dataPacket2 = PacketDataBuilder::build(1, userData, 2000);
        controller->addData(*dataPacket2);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_mockEngine->getDataCount());
    }

    void testMuteWhilePaused()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        // Pause first
        controller->pause();
        CPPUNIT_ASSERT(m_mockEngine->isPaused());
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());

        // Mute while paused
        controller->mute(true);
        CPPUNIT_ASSERT(m_mockEngine->isPaused());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Unmute while still paused
        controller->mute(false);
        CPPUNIT_ASSERT(m_mockEngine->isPaused());
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());

        // Resume while unmuted
        controller->resume();
        CPPUNIT_ASSERT(!m_mockEngine->isPaused());
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());
    }

    void testCombinedPauseAndMuteStates()
    {
        auto packet = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*packet, *m_mockConfig, m_mockWindow);

        // Both paused and muted
        controller->pause();
        controller->mute(true);
        CPPUNIT_ASSERT(m_mockEngine->isPaused());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Add data in this state
        std::vector<uint8_t> userData = {0xFF};
        auto dataPacket = PacketDataBuilder::build(1, userData, 1000);
        controller->addData(*dataPacket);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());

        // Process in this state
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());
        CPPUNIT_ASSERT(m_mockEngine->isPaused());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Resume but keep muted
        controller->resume();
        CPPUNIT_ASSERT(!m_mockEngine->isPaused());
        CPPUNIT_ASSERT(m_mockEngine->isMuted());

        // Unmute (now both active)
        controller->mute(false);
        CPPUNIT_ASSERT(!m_mockEngine->isPaused());
        CPPUNIT_ASSERT(!m_mockEngine->isMuted());
    }

    void testProcessTimestampBeforeData()
    {
        auto selectionPacket = PacketWebvttSelectionBuilder::build(1, 1920, 1080);
        auto controller = std::make_unique<WebvttController>(*selectionPacket, *m_mockConfig, m_mockWindow);

        // Process timestamp before any data - should not crash
        auto timestampPacket = PacketWebvttTimestampBuilder::build(1, 10000);
        CPPUNIT_ASSERT(timestampPacket != nullptr);

        controller->processTimestamp(*timestampPacket);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(10000), m_mockEngine->getCurrentMediatime());

        // Now add data after timestamp
        std::vector<uint8_t> userData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(1, userData, 5000);
        controller->addData(*dataPacket);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getDataCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(10000), m_mockEngine->getCurrentMediatime());

        // Process should work fine
        controller->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), m_mockEngine->getProcessCount());
    }

private:
    MockWebvttEngine* m_mockEngine;
    std::unique_ptr<MockConfigProvider> m_mockConfig;
    std::shared_ptr<MockWindow> m_mockWindow;
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebvttControllerTest);
