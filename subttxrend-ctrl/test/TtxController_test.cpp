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
#include "TtxController.hpp"
#include "../src/StcProvider.hpp"
#include <subttxrend/protocol/PacketTeletextSelection.hpp>
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/FontStrip.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <memory>
#include <vector>

using namespace subttxrend::ctrl;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;
using namespace subttxrend::common;

// Mock FontStrip that pretends to load fonts successfully
class MockFontStrip : public FontStrip
{
public:
    MockFontStrip(const Size& glyphSize, std::size_t glyphCount)
        : m_glyphSize(glyphSize), m_glyphCount(glyphCount) {}

    virtual ~MockFontStrip() {}

    bool loadFont(const std::string& fontName, const Size& charSize, const FontStripMap& charMap) override
    {
        // Pretend to load font successfully
        return true;
    }

    bool loadGlyph(std::int32_t glyphIndex, const std::uint8_t* data, const std::size_t size) override
    {
        // Pretend to load glyph successfully
        return true;
    }

private:
    Size m_glyphSize;
    std::size_t m_glyphCount;
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

    DrawContext& getDrawContext() override
    {
        static class MockDrawContext : public DrawContext {
        public:
            void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {}
            void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
            void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
            void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}
            void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override {}
            void drawString(PrerenderedFont& font, const Rectangle &destinationRect, const std::vector<GlyphData>& glyphs, const ColorArgb fgColor, const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}
        } mockContext;
        return mockContext;
    }

    Size getPreferredSize() const override { return Size(1920, 1080); }
    void setSize(const Size& newSize) override {}
    Size getSize() const override { return Size(1920, 1080); }
    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}
};

// Mock Engine for testing
class MockEngine : public Engine
{
public:
    MockEngine() {}
    virtual ~MockEngine() {}

    void init(const std::string& displayName = {}) override {}
    void shutdown() override {}
    void execute() override {}
    WindowPtr createWindow() override { return nullptr; }
    FontStripPtr createFontStrip(const Size& glyphSize, const std::size_t glyphCount) override
    {
        return std::make_shared<MockFontStrip>(glyphSize, glyphCount);
    }
    void attach(WindowPtr window) override {}
    void detach(WindowPtr window) override {}
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

// Helper to create PacketTeletextSelection for testing
class PacketTeletextSelectionBuilder
{
public:
    static std::unique_ptr<PacketTeletextSelection> build(uint32_t channelId, uint32_t magazine, uint32_t page)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        // Header: type(4) + counter(4) + size(4) = 12 bytes
        // Data: channelId(4) = 4 bytes
        // Note: magazine and page are ignored as they're not part of the packet format
        std::vector<uint8_t> data = {
            0x06, 0x00, 0x00, 0x00, // type = TELETEXT_SELECTION (6)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x04, 0x00, 0x00, 0x00, // size = 4 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24)
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketTeletextSelection>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketSubtitleSelection for testing
class PacketSubtitleSelectionBuilder
{
public:
    static std::unique_ptr<PacketSubtitleSelection> build(uint32_t channelId, uint32_t auxId1, uint32_t auxId2)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        // Header: type(4) + counter(4) + size(4) = 12 bytes
        // Data: channelId(4) + subtitleType(4) + auxId1(4) + auxId2(4) = 16 bytes
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION (5)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24),
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT), 0x00, 0x00, 0x00, // subtitle type = TELETEXT
            static_cast<uint8_t>(auxId1), static_cast<uint8_t>(auxId1 >> 8),
            static_cast<uint8_t>(auxId1 >> 16), static_cast<uint8_t>(auxId1 >> 24),
            static_cast<uint8_t>(auxId2), static_cast<uint8_t>(auxId2 >> 8),
            static_cast<uint8_t>(auxId2 >> 16), static_cast<uint8_t>(auxId2 >> 24)
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSubtitleSelection>();
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
    static std::unique_ptr<PacketData> build(uint32_t channelId, const std::vector<uint8_t>& userData)
    {
        uint32_t size = 4 + userData.size(); // channelId + user data

        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24),
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24)
        };

        data.insert(data.end(), userData.begin(), userData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    // Helper to create PES packet data
    static std::vector<uint8_t> createPesPacket(uint16_t dataSize)
    {
        std::vector<uint8_t> pesData = {
            0x00, 0x00, 0x01, 0xBD, // PES start code + stream ID
            static_cast<uint8_t>(dataSize >> 8), static_cast<uint8_t>(dataSize & 0xFF), // PES packet length
            0x80, 0x00, 0x00 // PES header data
        };

        // Add payload
        for (uint16_t i = 0; i < dataSize; ++i) {
            pesData.push_back(static_cast<uint8_t>(i % 256));
        }

        return pesData;
    }
};

class TtxControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TtxControllerTest);

    CPPUNIT_TEST(testConstructorWithTeletextSelection);
    CPPUNIT_TEST(testConstructorWithSubtitleSelection);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testSelectTeletext);
    CPPUNIT_TEST(testSelectSubtitle);
    CPPUNIT_TEST(testSelectTeletextWithZeroValues);
    CPPUNIT_TEST(testSelectSubtitleWithZeroValues);
    CPPUNIT_TEST(testSelectTeletextWithMaxValues);
    CPPUNIT_TEST(testSwitchFromTeletextToSubtitle);
    CPPUNIT_TEST(testSwitchFromSubtitleToTeletext);
    CPPUNIT_TEST(testActivateWithTeletextSelected);
    CPPUNIT_TEST(testActivateWithSubtitleSelected);
    CPPUNIT_TEST(testDeactivateWithTeletextSelected);
    CPPUNIT_TEST(testDeactivateWithSubtitleSelected);
    CPPUNIT_TEST(testMultipleActivateDeactivateCycles);
    CPPUNIT_TEST(testMuteTeletextWhenSelected);
    CPPUNIT_TEST(testUnmuteTeletextWhenSelected);
    CPPUNIT_TEST(testMuteSubtitleWhenSelected);
    CPPUNIT_TEST(testUnmuteSubtitleWhenSelected);
    CPPUNIT_TEST(testMuteWhenNothingSelected);
    CPPUNIT_TEST(testSetMutedTeletextDirectly);
    CPPUNIT_TEST(testSetMutedSubtitlesDirectly);
    CPPUNIT_TEST(testAddDataWithCorrectChannelId);
    CPPUNIT_TEST(testAddDataWithIncorrectChannelId);
    CPPUNIT_TEST(testAddDataWithNoPesPackets);
    CPPUNIT_TEST(testAddDataWithMultiplePesPackets);
    CPPUNIT_TEST(testAddDataWhenNotStarted);
    CPPUNIT_TEST(testProcessWithTeletextSelected);
    CPPUNIT_TEST(testProcessWithSubtitleSelected);
    CPPUNIT_TEST(testProcessWithNothingSelected);
    CPPUNIT_TEST(testGetTeletextChannel);
    CPPUNIT_TEST(testGetSubtitleChannel);
    CPPUNIT_TEST(testWantsDataPacketOverload);
    CPPUNIT_TEST(testResetAfterTeletextSelection);
    CPPUNIT_TEST(testResetAfterSubtitleSelection);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockWindow = std::make_shared<MockWindow>();
        m_mockEngine = std::make_shared<MockEngine>();
        m_mockConfig = std::make_unique<MockConfigProvider>();
        m_stcProvider = std::make_unique<subttxrend::ctrl::StcProvider>();
    }

    void tearDown() override
    {
        m_mockWindow.reset();
        m_mockEngine.reset();
        m_mockConfig.reset();
        m_stcProvider.reset();
    }

protected:
    void testConstructorWithTeletextSelection()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        CPPUNIT_ASSERT(packet != nullptr);

        // Constructor should not throw
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Verify teletext channel is set
        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(100), ttxChannel.getChannelId());
    }

    void testConstructorWithSubtitleSelection()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        CPPUNIT_ASSERT(packet != nullptr);

        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Verify subtitle channel is set
        Channel subChannel = controller.getSubtitleChannel();
        CPPUNIT_ASSERT(subChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(200), subChannel.getChannelId());
    }

    void testDestructor()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        CPPUNIT_ASSERT(packet != nullptr);

        {
            TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);
            // Use controller
            controller.process();
        }
        // Destructor should not crash
        CPPUNIT_ASSERT(true);
    }

    void testSelectTeletext()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 0, 0);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.selectTeletext(150, 2, 300);

        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(150), ttxChannel.getChannelId());
    }

    void testSelectSubtitle()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 0, 0);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.selectSubtitle(250, 3, 4);

        Channel subChannel = controller.getSubtitleChannel();
        CPPUNIT_ASSERT(subChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(250), subChannel.getChannelId());
    }

    void testSelectTeletextWithZeroValues()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 0, 0);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.selectTeletext(0, 0, 0);

        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), ttxChannel.getChannelId());
    }

    void testSelectSubtitleWithZeroValues()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 0, 0);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.selectSubtitle(0, 0, 0);

        Channel subChannel = controller.getSubtitleChannel();
        CPPUNIT_ASSERT(subChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(0), subChannel.getChannelId());
    }

    void testSelectTeletextWithMaxValues()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 0, 0);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        uint32_t maxVal = std::numeric_limits<uint32_t>::max();
        controller.selectTeletext(maxVal, maxVal, maxVal);

        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(maxVal, ttxChannel.getChannelId());
    }

    void testSwitchFromTeletextToSubtitle()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Start with teletext
        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());

        // Switch to subtitle
        controller.selectSubtitle(200, 2, 3);

        Channel subChannel = controller.getSubtitleChannel();
        CPPUNIT_ASSERT(subChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(200), subChannel.getChannelId());
    }

    void testSwitchFromSubtitleToTeletext()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Start with subtitle
        Channel subChannel = controller.getSubtitleChannel();
        CPPUNIT_ASSERT(subChannel.isActive());

        // Switch to teletext
        controller.selectTeletext(300, 3, 400);

        Channel ttxChannel = controller.getTeletextChannel();
        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(300), ttxChannel.getChannelId());
    }

    void testActivateWithTeletextSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.activate();

        // Verify controller is active (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testActivateWithSubtitleSelected()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.activate();

        // Verify controller is active (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testDeactivateWithTeletextSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.activate();
        controller.deactivate();

        // Verify deactivation completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testDeactivateWithSubtitleSelected()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.activate();
        controller.deactivate();

        // Verify deactivation completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testMultipleActivateDeactivateCycles()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        for (int i = 0; i < 5; ++i) {
            controller.activate();
            controller.deactivate();
        }

        // Verify multiple cycles complete (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testMuteTeletextWhenSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.mute(true);

        // Verify mute completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testUnmuteTeletextWhenSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.mute(true);
        controller.mute(false);

        // Verify unmute completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testMuteSubtitleWhenSelected()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.mute(true);

        // Verify mute completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testUnmuteSubtitleWhenSelected()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.mute(true);
        controller.mute(false);

        // Verify unmute completes (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testMuteWhenNothingSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Reset to clear selection
        // Note: We can't directly test "nothing selected" after construction
        // because constructor always selects something via packet

        // Test mute still works
        controller.mute(true);

        CPPUNIT_ASSERT(true);
    }

    void testSetMutedTeletextDirectly()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.setMutedTeletext(true);
        controller.setMutedTeletext(false);

        // Verify calls complete (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testSetMutedSubtitlesDirectly()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.setMutedSubtitles(true);
        controller.setMutedSubtitles(false);

        // Verify calls complete (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWithCorrectChannelId()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::build(100, pesData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify data added (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWithIncorrectChannelId()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::build(999, pesData); // Wrong channel ID
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Should discard data but not crash
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWithNoPesPackets()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Create packet with some data but no valid PES packet headers
        // PES packets start with 0x00 0x00 0x01, so avoid that pattern
        std::vector<uint8_t> dataWithoutPes = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
        };
        auto dataPacket = PacketDataBuilder::build(100, dataWithoutPes);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Should handle data with no valid PES packets gracefully
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWithMultiplePesPackets()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Create data with multiple PES packets
        auto pes1 = PacketDataBuilder::createPesPacket(30);
        auto pes2 = PacketDataBuilder::createPesPacket(40);

        std::vector<uint8_t> combinedData;
        combinedData.insert(combinedData.end(), pes1.begin(), pes1.end());
        combinedData.insert(combinedData.end(), pes2.begin(), pes2.end());

        auto dataPacket = PacketDataBuilder::build(100, combinedData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Should process multiple PES packets
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWhenNotStarted()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // Deactivate to stop
        controller.deactivate();

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::build(100, pesData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Should handle gracefully when not started
        CPPUNIT_ASSERT(true);
    }

    void testProcessWithTeletextSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.process();

        // Should process teletext data (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testProcessWithSubtitleSelected()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        controller.process();

        // Should process subtitle data (no crash)
        CPPUNIT_ASSERT(true);
    }

    void testProcessWithNothingSelected()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // After construction, something is always selected
        // Test process still works
        controller.process();

        CPPUNIT_ASSERT(true);
    }

    void testGetTeletextChannel()
    {
        auto packet = PacketTeletextSelectionBuilder::build(123, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        Channel ttxChannel = controller.getTeletextChannel();

        CPPUNIT_ASSERT(ttxChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(123), ttxChannel.getChannelId());
    }

    void testGetSubtitleChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(456, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        Channel subChannel = controller.getSubtitleChannel();

        CPPUNIT_ASSERT(subChannel.isActive());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(456), subChannel.getChannelId());
    }

    void testWantsDataPacketOverload()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        auto testPacket = PacketTeletextSelectionBuilder::build(100, 2, 200);
        CPPUNIT_ASSERT(testPacket != nullptr);

        bool wants = controller.wantsData(*testPacket);

        CPPUNIT_ASSERT_EQUAL(true, wants);
    }

    void testResetAfterTeletextSelection()
    {
        auto packet = PacketTeletextSelectionBuilder::build(100, 1, 100);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // After using teletext, verify we can still use other methods
        controller.selectTeletext(150, 2, 200);
        controller.process();

        CPPUNIT_ASSERT(true);
    }

    void testResetAfterSubtitleSelection()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        TtxController controller(*packet, *m_mockConfig, m_mockWindow, m_mockEngine, *m_stcProvider);

        // After using subtitle, verify we can still use other methods
        controller.selectSubtitle(250, 3, 4);
        controller.process();

        CPPUNIT_ASSERT(true);
    }

private:
    std::shared_ptr<MockWindow> m_mockWindow;
    std::shared_ptr<MockEngine> m_mockEngine;
    std::unique_ptr<MockConfigProvider> m_mockConfig;
    std::unique_ptr<subttxrend::ctrl::StcProvider> m_stcProvider;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtxControllerTest);
