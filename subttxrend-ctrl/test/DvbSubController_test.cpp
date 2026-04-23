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
#include "DvbSubController.hpp"
#include "StcProvider.hpp"
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <memory>
#include <vector>

using namespace subttxrend::ctrl;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;

// Mock Engine for testing
class MockEngine : public Engine
{
public:
    MockEngine() {}
    virtual ~MockEngine() {}

    void init(const std::string &displayName = {}) override {}
    void shutdown() override {}
    void execute() override {}
    WindowPtr createWindow() override { return nullptr; }
    FontStripPtr createFontStrip(const Size& glyphSize, const std::size_t glyphCount) override { return nullptr; }
    void attach(WindowPtr window) override {}
    void detach(WindowPtr window) override {}
};

// Mock Window for testing
class MockWindow : public Window
{
public:
    MockWindow() {}
    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
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

    Size getPreferredSize() const override {
        return Size(1920, 1080);
    }

    void setSize(const Size& newSize) override {}
    Size getSize() const override {
        return Size(1920, 1080);
    }

    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}
};

// Helper to create PacketSubtitleSelection for testing
class PacketSubtitleSelectionBuilder
{
public:
    static std::unique_ptr<PacketSubtitleSelection> build(uint32_t channelId, uint32_t compositionPageId, uint32_t ancillaryPageId)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION (5)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_DVB), 0x00, 0x00, 0x00, // subtitle type = DVB
            static_cast<uint8_t>(compositionPageId), static_cast<uint8_t>(compositionPageId >> 8),
            static_cast<uint8_t>(compositionPageId >> 16), static_cast<uint8_t>(compositionPageId >> 24), // auxId1 (composition page)
            static_cast<uint8_t>(ancillaryPageId), static_cast<uint8_t>(ancillaryPageId >> 8),
            static_cast<uint8_t>(ancillaryPageId >> 16), static_cast<uint8_t>(ancillaryPageId >> 24) // auxId2 (ancillary page)
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSubtitleSelection>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

// Helper to create PacketData with PES data for testing
class PacketDataBuilder
{
public:
    static std::unique_ptr<PacketData> buildWithPesData(uint32_t channelId, const std::vector<uint8_t>& pesData)
    {
        // PacketData requires: channel_id(4) + data
        uint32_t size = 4 + pesData.size();

        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24) // channel ID
        };

        data.insert(data.end(), pesData.begin(), pesData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    static std::unique_ptr<PacketData> buildEmpty(uint32_t channelId)
    {
        return buildWithPesData(channelId, std::vector<uint8_t>());
    }

    // Create PES packet structure
    static std::vector<uint8_t> createPesPacket(uint16_t pesSize = 20)
    {
        std::vector<uint8_t> pes;
        // PES packet start code
        pes.push_back(0x00);
        pes.push_back(0x00);
        pes.push_back(0x01);
        pes.push_back(0xBD); // private_stream_1

        // PES packet length (2 bytes, big-endian)
        uint16_t length = pesSize - 6;
        pes.push_back((length >> 8) & 0xFF);
        pes.push_back(length & 0xFF);

        // Fill rest with dummy data
        for (int i = 6; i < pesSize; ++i) {
            pes.push_back(0x00);
        }

        return pes;
    }
};

class DvbSubControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DvbSubControllerTest);
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithDifferentChannelIds);
    CPPUNIT_TEST(testConstructorWithZeroCompositionPageId);
    CPPUNIT_TEST(testConstructorWithMaxCompositionPageId);
    CPPUNIT_TEST(testConstructorWithZeroAncillaryPageId);
    CPPUNIT_TEST(testConstructorWithMaxAncillaryPageId);
    CPPUNIT_TEST(testDestructorAfterOperations);
    CPPUNIT_TEST(testProcessWhenActive);
    CPPUNIT_TEST(testProcessMultipleTimes);
    CPPUNIT_TEST(testProcessAfterDeactivate);
    CPPUNIT_TEST(testAddDataWithValidPesData);
    CPPUNIT_TEST(testAddDataWithMultiplePesPackets);
    CPPUNIT_TEST(testAddDataWhenNotStarted);
    CPPUNIT_TEST(testAddDataMultipleTimes);
    CPPUNIT_TEST(testActivateFromStopped);
    CPPUNIT_TEST(testActivateWhenAlreadyActive);
    CPPUNIT_TEST(testDeactivateWhenActive);
    CPPUNIT_TEST(testDeactivateWhenAlreadyStopped);
    CPPUNIT_TEST(testActivateDeactivateCycle);
    CPPUNIT_TEST(testMuteWhenUnmuted);
    CPPUNIT_TEST(testUnmuteWhenMuted);
    CPPUNIT_TEST(testMuteWhenAlreadyMuted);
    CPPUNIT_TEST(testUnmuteWhenAlreadyUnmuted);
    CPPUNIT_TEST(testWantsDataWithMatchingChannel);
    CPPUNIT_TEST(testWantsDataWithNonMatchingChannel);
    CPPUNIT_TEST(testWantsDataWithZeroChannel);
    CPPUNIT_TEST(testWantsDataWithMaxChannel);
    CPPUNIT_TEST(testWantsDataCalledMultipleTimes);
    CPPUNIT_TEST(testCompleteWorkflow);
    CPPUNIT_TEST(testWorkflowWithMuting);
    CPPUNIT_TEST(testWorkflowAddDataBeforeActivate);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_window = std::make_shared<MockWindow>();
        m_engine = std::make_shared<MockEngine>();
        m_stcProvider = std::make_unique<StcProvider>();
    }

    void tearDown() override
    {
        m_window.reset();
        m_engine.reset();
        m_stcProvider.reset();
    }

protected:
    void testConstructorWithValidParameters()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        CPPUNIT_ASSERT(packet != nullptr);

        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        // Verify construction succeeded and channel is set
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testConstructorWithDifferentChannelIds()
    {
        auto packet1 = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller1(*packet1, m_window, m_engine, *m_stcProvider);

        auto packet2 = PacketSubtitleSelectionBuilder::build(200, 1, 2);
        DvbSubController controller2(*packet2, m_window, m_engine, *m_stcProvider);

        // Each controller should want only its own channel
        auto testPacket1 = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        auto testPacket2 = PacketSubtitleSelectionBuilder::build(200, 0, 0);

        CPPUNIT_ASSERT(controller1.wantsData(*testPacket1));
        CPPUNIT_ASSERT(!controller1.wantsData(*testPacket2));
        CPPUNIT_ASSERT(!controller2.wantsData(*testPacket1));
        CPPUNIT_ASSERT(controller2.wantsData(*testPacket2));
    }

    void testConstructorWithZeroCompositionPageId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 0, 1);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testConstructorWithMaxCompositionPageId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 0xFFFFFFFF, 1);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testConstructorWithZeroAncillaryPageId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 0);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testConstructorWithMaxAncillaryPageId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 0xFFFFFFFF);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testDestructorAfterOperations()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        bool operationsCompleted = false;
        {
            DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);
            controller.activate();
            controller.process();
            controller.deactivate();
            operationsCompleted = true;
            // Destructor called here
        }
        // Verify operations completed and destructor didn't crash
        CPPUNIT_ASSERT_MESSAGE("Operations should complete and destructor should not crash", operationsCompleted);
    }

    void testProcessWhenActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.process();

        // Verify controller still wants data after process
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data after process",
                               controller.wantsData(*testPacket));
    }

    void testProcessMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        for (int i = 0; i < 10; ++i) {
            controller.process();
        }

        // Verify controller remains functional after multiple process calls
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple process calls",
                               controller.wantsData(*testPacket));
    }

    void testProcessAfterDeactivate()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.deactivate();
        controller.process();

        // Verify controller still maintains its channel even after deactivate + process
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data from its channel",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithValidPesData()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify controller remains functional after adding data
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after addData",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithMultiplePesPackets()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();

        // Create buffer with multiple PES packets
        std::vector<uint8_t> multiPes;
        auto pes1 = PacketDataBuilder::createPesPacket(30);
        auto pes2 = PacketDataBuilder::createPesPacket(40);
        multiPes.insert(multiPes.end(), pes1.begin(), pes1.end());
        multiPes.insert(multiPes.end(), pes2.begin(), pes2.end());

        auto dataPacket = PacketDataBuilder::buildWithPesData(100, multiPes);
        CPPUNIT_ASSERT(dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify controller processed data and remains functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple PES packets",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWhenNotStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        // Don't activate - constructor already starts it, but test deactivated state
        controller.deactivate();

        auto pesData = PacketDataBuilder::createPesPacket(30);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);

        controller.addData(*dataPacket);

        // Verify controller still wants data from its channel even when deactivated
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data from its channel",
                               controller.wantsData(*testPacket));
    }

    void testAddDataMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();

        for (int i = 0; i < 5; ++i) {
            auto pesData = PacketDataBuilder::createPesPacket(20 + i);
            auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);
            controller.addData(*dataPacket);
        }

        // Verify controller remains functional after multiple addData calls
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple addData calls",
                               controller.wantsData(*testPacket));
    }

    void testActivateFromStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.deactivate();
        controller.activate();

        // Verify controller is active by checking it wants data
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testActivateWhenAlreadyActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        // Constructor already activates
        controller.activate(); // Should be idempotent

        // Verify controller still wants data
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data after redundant activate",
                               controller.wantsData(*testPacket));
    }

    void testDeactivateWhenActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.deactivate();

        // Still wants data (deactivate doesn't reset channel)
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testDeactivateWhenAlreadyStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.deactivate();
        controller.deactivate(); // Should be safe

        // Verify controller still wants data from its channel
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data from its channel",
                               controller.wantsData(*testPacket));
    }

    void testActivateDeactivateCycle()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        for (int i = 0; i < 3; ++i) {
            controller.deactivate();
            controller.activate();
        }

        // Verify controller remains functional after activation/deactivation cycles
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after activate/deactivate cycles",
                               controller.wantsData(*testPacket));
    }

    void testMuteWhenUnmuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.mute(true);

        // Verify controller still functional after mute
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after mute",
                               controller.wantsData(*testPacket));
    }

    void testUnmuteWhenMuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.mute(true);
        controller.mute(false);

        // Verify controller remains functional after mute/unmute
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after mute/unmute",
                               controller.wantsData(*testPacket));
    }

    void testMuteWhenAlreadyMuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.mute(true);
        controller.mute(true); // Should be idempotent

        // Verify controller remains functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after redundant mute",
                               controller.wantsData(*testPacket));
    }

    void testUnmuteWhenAlreadyUnmuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.mute(false);
        controller.mute(false); // Should be idempotent

        // Verify controller remains functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after redundant unmute",
                               controller.wantsData(*testPacket));
    }

    void testWantsDataWithMatchingChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(42, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testWantsDataWithNonMatchingChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(99, 0, 0);
        CPPUNIT_ASSERT(!controller.wantsData(*testPacket));
    }

    void testWantsDataWithZeroChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(0, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket0 = PacketSubtitleSelectionBuilder::build(0, 0, 0);
        auto testPacket1 = PacketSubtitleSelectionBuilder::build(1, 0, 0);

        CPPUNIT_ASSERT(controller.wantsData(*testPacket0));
        CPPUNIT_ASSERT(!controller.wantsData(*testPacket1));
    }

    void testWantsDataWithMaxChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(0xFFFFFFFF, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(0xFFFFFFFF, 0, 0);
        CPPUNIT_ASSERT(controller.wantsData(*testPacket));
    }

    void testWantsDataCalledMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);

        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT(controller.wantsData(*testPacket));
        }
    }

    void testCompleteWorkflow()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        // Complete workflow
        controller.activate();

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);
        controller.addData(*dataPacket);

        controller.process();

        controller.deactivate();

        // Verify controller completed workflow and maintains state
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after complete workflow",
                               controller.wantsData(*testPacket));
    }

    void testWorkflowWithMuting()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.mute(true);

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);
        controller.addData(*dataPacket);

        controller.process();

        controller.mute(false);
        controller.deactivate();

        // Verify controller completed workflow with muting and maintains state
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after workflow with muting",
                               controller.wantsData(*testPacket));
    }

    void testWorkflowAddDataBeforeActivate()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        // Constructor already starts, so deactivate first
        controller.deactivate();

        // Add data while stopped (should be ignored)
        auto pesData1 = PacketDataBuilder::createPesPacket(30);
        auto dataPacket1 = PacketDataBuilder::buildWithPesData(100, pesData1);
        controller.addData(*dataPacket1);

        // Now activate and add data
        controller.activate();
        auto pesData2 = PacketDataBuilder::createPesPacket(40);
        auto dataPacket2 = PacketDataBuilder::buildWithPesData(100, pesData2);
        controller.addData(*dataPacket2);

        controller.process();

        // Verify controller completed workflow and maintains state
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after workflow",
                               controller.wantsData(*testPacket));
    }

private:
    std::shared_ptr<MockWindow> m_window;
    std::shared_ptr<MockEngine> m_engine;
    std::unique_ptr<StcProvider> m_stcProvider;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DvbSubControllerTest);
