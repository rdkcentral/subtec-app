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

    void init(const std::string & = {}) override {}
    void shutdown() override {}
    void execute() override {}
    WindowPtr createWindow() override { return nullptr; }
    FontStripPtr createFontStrip(const Size&, const std::size_t) override { return nullptr; }
    void attach(WindowPtr) override {}
    void detach(WindowPtr) override {}
};

// Mock Window for testing
class MockWindow : public Window
{
public:
    MockWindow() : m_visible(false), m_visibleTrueCount(0), m_visibleFalseCount(0) {}
    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener*) override {}
    void removeKeyEventListener(KeyEventListener*) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
        static class MockDrawContext : public DrawContext {
        public:
            void fillRectangle(ColorArgb, const Rectangle&) override {}
            void drawUnderline(ColorArgb, const Rectangle&) override {}
            void drawPixmap(const ClutBitmap&, const Rectangle&, const Rectangle&) override {}
            void drawBitmap(const Bitmap&, const Rectangle&) override {}
            void drawGlyph(const FontStripPtr&, std::int32_t, const Rectangle&, ColorArgb, ColorArgb) override {}
            void drawString(PrerenderedFont&, const Rectangle&, const std::vector<GlyphData>&, const ColorArgb, const ColorArgb, int = 0, int = 0) override {}
        } mockContext;
        return mockContext;
    }

    Size getPreferredSize() const override {
        return Size(1920, 1080);
    }

    void setSize(const Size& newSize) override { m_size = newSize; }
    Size getSize() const override {
        return m_size;
    }

    void setVisible(bool visible) override {
        m_visible = visible;
        if (visible) {
            ++m_visibleTrueCount;
        }
        else {
            ++m_visibleFalseCount;
        }
    }
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection) override {}

    bool isVisible() const { return m_visible; }
    int getVisibleTrueCount() const { return m_visibleTrueCount; }
    int getVisibleFalseCount() const { return m_visibleFalseCount; }

private:
    bool m_visible;
    int m_visibleTrueCount;
    int m_visibleFalseCount;
    Size m_size{1920, 1080};
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
        // PES data requires: channel_id(4) + channel_type(4) + data
        uint32_t size = 8 + pesData.size();

        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            0x00, 0x00, 0x00, 0x00 // channel type
        };

        data.insert(data.end(), pesData.begin(), pesData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
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
        CPPUNIT_ASSERT_MESSAGE("Controller should show the window when active", m_window->isVisible());
        controller.process();

        CPPUNIT_ASSERT_MESSAGE("Process should not hide the active window", m_window->isVisible());
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
        const int visibleFalseCountBeforeProcess = m_window->getVisibleFalseCount();
        controller.process();

        CPPUNIT_ASSERT(!m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(visibleFalseCountBeforeProcess, m_window->getVisibleFalseCount());

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
        CPPUNIT_ASSERT_EQUAL(pesData.size(), dataPacket->getDataSize());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), static_cast<unsigned char>(dataPacket->getData()[0]));
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), static_cast<unsigned char>(dataPacket->getData()[1]));
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(1), static_cast<unsigned char>(dataPacket->getData()[2]));
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0xBD), static_cast<unsigned char>(dataPacket->getData()[3]));

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
        const int visibleFalseCountBeforeData = m_window->getVisibleFalseCount();

        auto pesData = PacketDataBuilder::createPesPacket(30);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);

        controller.addData(*dataPacket);

        CPPUNIT_ASSERT(!m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(visibleFalseCountBeforeData, m_window->getVisibleFalseCount());

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
        CPPUNIT_ASSERT_MESSAGE("Deactivate should hide the window", !m_window->isVisible());
        const int visibleTrueCountBeforeActivate = m_window->getVisibleTrueCount();
        controller.activate();

        CPPUNIT_ASSERT_MESSAGE("Activate should show the window again", m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > visibleTrueCountBeforeActivate);
    }

    void testActivateWhenAlreadyActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        const int visibleFalseCountBeforeActivate = m_window->getVisibleFalseCount();
        const int visibleTrueCountBeforeActivate = m_window->getVisibleTrueCount();
        controller.activate();

        CPPUNIT_ASSERT_MESSAGE("Redundant activate should leave the window visible", m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleFalseCount() > visibleFalseCountBeforeActivate);
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > visibleTrueCountBeforeActivate);
    }

    void testDeactivateWhenActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.deactivate();

        CPPUNIT_ASSERT_MESSAGE("Deactivate should hide the window", !m_window->isVisible());
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

        CPPUNIT_ASSERT_MESSAGE("Muting should hide the window", !m_window->isVisible());
    }

    void testUnmuteWhenMuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.mute(true);
        controller.mute(false);

        CPPUNIT_ASSERT_MESSAGE("Unmuting should show the window", m_window->isVisible());

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
        const int visibleFalseCountAfterFirstMute = m_window->getVisibleFalseCount();
        controller.mute(true); // Should be idempotent

        CPPUNIT_ASSERT(!m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(visibleFalseCountAfterFirstMute, m_window->getVisibleFalseCount());

        // Verify controller remains functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after redundant mute",
                               controller.wantsData(*testPacket));
    }

    void testUnmuteWhenAlreadyUnmuted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        const int visibleTrueCountBeforeUnmute = m_window->getVisibleTrueCount();
        controller.mute(false);
        controller.mute(false); // Should be idempotent

        CPPUNIT_ASSERT(m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(visibleTrueCountBeforeUnmute, m_window->getVisibleTrueCount());

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

        CPPUNIT_ASSERT_MESSAGE("Complete workflow should end with a hidden window after deactivate",
                       !m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > 0);
        CPPUNIT_ASSERT(m_window->getVisibleFalseCount() > 0);
    }

    void testWorkflowWithMuting()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, 1, 2);
        DvbSubController controller(*packet, m_window, m_engine, *m_stcProvider);

        controller.activate();
        controller.mute(true);
        CPPUNIT_ASSERT(!m_window->isVisible());

        auto pesData = PacketDataBuilder::createPesPacket(50);
        auto dataPacket = PacketDataBuilder::buildWithPesData(100, pesData);
        controller.addData(*dataPacket);

        controller.process();

        controller.mute(false);
        CPPUNIT_ASSERT(m_window->isVisible());
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
        CPPUNIT_ASSERT(!m_window->isVisible());

        // Add data while stopped (should be ignored)
        auto pesData1 = PacketDataBuilder::createPesPacket(30);
        auto dataPacket1 = PacketDataBuilder::buildWithPesData(100, pesData1);
        controller.addData(*dataPacket1);
        CPPUNIT_ASSERT(!m_window->isVisible());

        // Now activate and add data
        controller.activate();
        CPPUNIT_ASSERT(m_window->isVisible());
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
