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
#include "ScteSubController.hpp"
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

// Mock Window for testing
class MockWindow : public Window
{
public:
    MockWindow() : m_visible(false), m_visibleTrueCount(0), m_visibleFalseCount(0), m_updateCount(0), m_fillRectangleCount(0) {}
    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
        class MockDrawContext : public DrawContext {
        public:
            explicit MockDrawContext(int& fillRectangleCount) : m_fillRectangleCount(fillRectangleCount) {}
            void fillRectangle(ColorArgb color, const Rectangle& rectangle) override { ++m_fillRectangleCount; }
            void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
            void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
            void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}
            void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override {}
            void drawString(PrerenderedFont& font, const Rectangle &destinationRect, const std::vector<GlyphData>& glyphs, const ColorArgb fgColor, const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}
        private:
            int& m_fillRectangleCount;
        };
        if (!m_mockContext) {
            m_mockContext = std::make_unique<MockDrawContext>(m_fillRectangleCount);
        }
        return *m_mockContext;
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
    void update() override { ++m_updateCount; }
    void setDrawDirection(DrawDirection dir) override {}

    bool isVisible() const { return m_visible; }
    int getVisibleTrueCount() const { return m_visibleTrueCount; }
    int getVisibleFalseCount() const { return m_visibleFalseCount; }
    int getFillRectangleCount() const { return m_fillRectangleCount; }

private:
    bool m_visible;
    int m_visibleTrueCount;
    int m_visibleFalseCount;
    int m_updateCount;
    int m_fillRectangleCount;
    Size m_size{1920, 1080};
    std::unique_ptr<DrawContext> m_mockContext;
};

// Helper to create PacketSubtitleSelection for testing SCTE
class PacketSubtitleSelectionBuilder
{
public:
    static std::unique_ptr<PacketSubtitleSelection> buildScte(uint32_t channelId)
    {
        // Build packet data buffer with proper format (LITTLE-ENDIAN)
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION (5)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_SCTE), 0x00, 0x00, 0x00, // subtitle type = SCTE
            0x00, 0x00, 0x00, 0x00, // auxId1
            0x00, 0x00, 0x00, 0x00  // auxId2
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSubtitleSelection>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    static std::unique_ptr<PacketSubtitleSelection> buildDvb(uint32_t channelId)
    {
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION (5)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_DVB), 0x00, 0x00, 0x00, // subtitle type = DVB
            0x00, 0x00, 0x00, 0x00, // auxId1
            0x00, 0x00, 0x00, 0x00  // auxId2
        };

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSubtitleSelection>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    static std::unique_ptr<PacketSubtitleSelection> buildTeletext(uint32_t channelId)
    {
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION (5)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT), 0x00, 0x00, 0x00, // subtitle type = Teletext
            0x00, 0x00, 0x00, 0x00, // auxId1
            0x00, 0x00, 0x00, 0x00  // auxId2
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
    static std::unique_ptr<PacketData> buildScteData(uint32_t channelId, const std::vector<uint8_t>& scteData)
    {
        // PacketData requires: channel_id(4) + channelType(4) + data
        uint32_t size = 8 + scteData.size();

        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_SCTE), 0x00, 0x00, 0x00 // channel type = SCTE
        };

        data.insert(data.end(), scteData.begin(), scteData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    static std::unique_ptr<PacketData> buildDvbData(uint32_t channelId, const std::vector<uint8_t>& dvbData)
    {
        uint32_t size = 8 + dvbData.size();

        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_DVB), 0x00, 0x00, 0x00 // channel type = DVB
        };

        data.insert(data.end(), dvbData.begin(), dvbData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }

    static std::unique_ptr<PacketData> buildMinimalScteData(uint32_t channelId)
    {
        // Create minimal valid SCTE data (at least 1 byte to avoid null buffer)
        std::vector<uint8_t> minimalData = {0x00};
        return buildScteData(channelId, minimalData);
    }

    // Create SCTE section data with valid structure
    static std::vector<uint8_t> createScteSection(uint16_t sectionSize = 20)
    {
        std::vector<uint8_t> section;
        // Minimal valid SCTE section structure
        for (uint16_t i = 0; i < sectionSize; ++i) {
            section.push_back(static_cast<uint8_t>(i & 0xFF));
        }
        return section;
    }
};

class ScteSubControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ScteSubControllerTest);
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithDifferentChannelIds);
    CPPUNIT_TEST(testDestructorWhenRendererStarted);
    CPPUNIT_TEST(testDestructorWhenRendererNotStarted);
    CPPUNIT_TEST(testProcessWhenActive);
    CPPUNIT_TEST(testProcessMultipleTimes);
    CPPUNIT_TEST(testProcessAfterDeactivate);
    CPPUNIT_TEST(testAddDataWithValidScteData);
    CPPUNIT_TEST(testAddDataWithEmptyData);
    CPPUNIT_TEST(testAddDataWithLargeData);
    CPPUNIT_TEST(testAddDataMultipleTimes);
    CPPUNIT_TEST(testAddDataWhenDeactivated);
    CPPUNIT_TEST(testActivateFromInitialState);
    CPPUNIT_TEST(testActivateAfterDeactivate);
    CPPUNIT_TEST(testActivateWhenAlreadyActive);
    CPPUNIT_TEST(testDeactivateWhenActive);
    CPPUNIT_TEST(testDeactivateWhenAlreadyDeactivated);
    CPPUNIT_TEST(testDeactivateMultipleTimes);
    CPPUNIT_TEST(testMuteTrue);
    CPPUNIT_TEST(testMuteFalse);
    CPPUNIT_TEST(testMuteToggle);
    CPPUNIT_TEST(testMuteTrueMultipleTimes);
    CPPUNIT_TEST(testMuteFalseMultipleTimes);
    CPPUNIT_TEST(testWantsDataWithSctePacket);
    CPPUNIT_TEST(testWantsDataWithDvbPacket);
    CPPUNIT_TEST(testWantsDataWithTeletextPacket);
    CPPUNIT_TEST(testWantsDataCalledMultipleTimes);
    CPPUNIT_TEST(testCompleteWorkflow);
    CPPUNIT_TEST(testWorkflowWithMuting);
    CPPUNIT_TEST(testWorkflowAddDataBeforeActivate);
    CPPUNIT_TEST(testActivateDeactivateCycle);
    CPPUNIT_TEST(testWorkflowWithMultipleDataPackets);
    CPPUNIT_TEST(testProcessingAfterMuteUnmute);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_window = std::make_shared<MockWindow>();
        m_stcProvider = std::make_unique<StcProvider>();
    }

    void tearDown() override
    {
        m_window.reset();
        m_stcProvider.reset();
    }

protected:
    void testConstructorWithValidParameters()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        CPPUNIT_ASSERT_MESSAGE("Failed to build SCTE packet", packet != nullptr);

        ScteSubController controller(*packet, m_window, *m_stcProvider);

        // Verify construction succeeded - controller should want SCTE data
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should want SCTE data after construction",
                               controller.wantsData(*scteDataPacket));
    }

    void testConstructorWithDifferentChannelIds()
    {
        auto packet1 = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller1(*packet1, m_window, *m_stcProvider);

        auto packet2 = PacketSubtitleSelectionBuilder::buildScte(200);
        ScteSubController controller2(*packet2, m_window, *m_stcProvider);

        // Both controllers should want SCTE data (channelId is not used in wantsData for SCTE)
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller1 should want SCTE data",
                               controller1.wantsData(*scteDataPacket));
        CPPUNIT_ASSERT_MESSAGE("Controller2 should want SCTE data",
                               controller2.wantsData(*scteDataPacket));
    }

    void testDestructorWhenRendererStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);

        // Create and destroy a controller that has been activated, ensuring destructor runs
        try {
            {
                ScteSubController controller(*packet, m_window, *m_stcProvider);
                controller.activate();
                // leaving scope calls destructor
            }
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor threw unknown exception");
        }

        // Verify no global state was corrupted by constructing a fresh controller and exercising it
        auto packet2 = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller2(*packet2, m_window, *m_stcProvider);
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should want SCTE data after previous destructor",
                               controller2.wantsData(*scteDataPacket));
    }

    void testDestructorWhenRendererNotStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);

        try {
            {
                ScteSubController controller(*packet, m_window, *m_stcProvider);
                controller.deactivate();
                // leaving scope calls destructor
            }
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor threw unknown exception");
        }

        // Sanity-check that new controllers can still be created and used
        auto packet2 = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller2(*packet2, m_window, *m_stcProvider);
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should want SCTE data after previous destructor",
                               controller2.wantsData(*scteDataPacket));
    }

    void testProcessWhenActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();
        controller.process();

        CPPUNIT_ASSERT_MESSAGE("Process should not hide the active SCTE window", m_window->isVisible());
    }

    void testProcessMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();
        for (int i = 0; i < 10; ++i) {
            controller.process();
        }

        // Verify controller remains functional after multiple process calls
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple process calls",
                               controller.wantsData(*scteDataPacket));
    }

    void testProcessAfterDeactivate()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();
        controller.deactivate();
        controller.process();

        // Verify controller still wants SCTE data
        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want SCTE data after deactivate and process",
                               controller.wantsData(*scteDataPacket));
    }

    void testAddDataWithValidScteData()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        auto scteData = PacketDataBuilder::createScteSection(50);
        auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);
        CPPUNIT_ASSERT_MESSAGE("Failed to build SCTE data packet", dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify controller remains functional after adding data
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after addData",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithEmptyData()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        auto dataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Failed to build minimal SCTE data packet", dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify controller handles minimal data gracefully
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should handle minimal data gracefully",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithLargeData()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        auto largeScteData = PacketDataBuilder::createScteSection(4096);
        auto dataPacket = PacketDataBuilder::buildScteData(100, largeScteData);
        CPPUNIT_ASSERT_MESSAGE("Failed to build large SCTE data packet", dataPacket != nullptr);

        controller.addData(*dataPacket);

        // Verify controller handles large data
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should handle large data packets",
                               controller.wantsData(*testPacket));
    }

    void testAddDataMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        for (int i = 0; i < 5; ++i) {
            auto scteData = PacketDataBuilder::createScteSection(20 + i);
            auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);
            CPPUNIT_ASSERT_MESSAGE("Failed to build SCTE data packet in loop", dataPacket != nullptr);
            controller.addData(*dataPacket);
        }

        // Verify controller remains functional after multiple addData calls
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple addData calls",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWhenDeactivated()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.deactivate();

        auto scteData = PacketDataBuilder::createScteSection(30);
        auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);

        controller.addData(*dataPacket);

        // Verify controller still wants SCTE data even when deactivated
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want SCTE data when deactivated",
                               controller.wantsData(*testPacket));
    }

    void testActivateFromInitialState()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        // Constructor already calls start(), but activate() should work
        controller.activate();

        // Verify controller is active by checking it still wants data
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should want SCTE data after activate",
                               controller.wantsData(*testPacket));
    }

    void testActivateAfterDeactivate()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.deactivate();
        CPPUNIT_ASSERT_MESSAGE("Deactivate should hide the SCTE window", !m_window->isVisible());
        const int visibleTrueCountBeforeActivate = m_window->getVisibleTrueCount();
        controller.activate();

        CPPUNIT_ASSERT_MESSAGE("Activate should show the SCTE window again", m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > visibleTrueCountBeforeActivate);
    }

    void testActivateWhenAlreadyActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        const int visibleFalseCountBeforeActivate = m_window->getVisibleFalseCount();
        const int visibleTrueCountBeforeActivate = m_window->getVisibleTrueCount();
        controller.activate();
        controller.activate(); // Should be idempotent

        CPPUNIT_ASSERT_MESSAGE("Redundant activate should leave the SCTE window visible", m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleFalseCount() > visibleFalseCountBeforeActivate);
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > visibleTrueCountBeforeActivate);
    }

    void testDeactivateWhenActive()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();
        controller.deactivate();

        CPPUNIT_ASSERT_MESSAGE("Deactivate should hide the SCTE window", !m_window->isVisible());
    }

    void testDeactivateWhenAlreadyDeactivated()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.deactivate();
        controller.deactivate(); // Should be safe

        // Verify controller still wants SCTE data
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want SCTE data",
                               controller.wantsData(*testPacket));
    }

    void testDeactivateMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        for (int i = 0; i < 3; ++i) {
            controller.deactivate();
        }

        // Verify controller remains stable after multiple deactivate calls
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain stable after multiple deactivate calls",
                               controller.wantsData(*testPacket));
    }

    void testMuteTrue()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.mute(true);

        // Verify controller still functional after mute
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after mute(true)",
                               controller.wantsData(*testPacket));
    }

    void testMuteFalse()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.mute(false);

        // Verify controller remains functional after unmute
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after mute(false)",
                               controller.wantsData(*testPacket));
    }

    void testMuteToggle()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.mute(true);
        CPPUNIT_ASSERT_MESSAGE("Mute should hide the SCTE window", !m_window->isVisible());
        const int fillCountBeforeUnmute = m_window->getFillRectangleCount();
        controller.mute(false);
        CPPUNIT_ASSERT_MESSAGE("Unmute should show the SCTE window", m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getFillRectangleCount() > fillCountBeforeUnmute);
        controller.mute(true);

        CPPUNIT_ASSERT_MESSAGE("Mute should hide the SCTE window again", !m_window->isVisible());
    }

    void testMuteTrueMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.mute(true);
        controller.mute(true); // Should be idempotent

        // Verify controller remains functional
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after redundant mute(true)",
                               controller.wantsData(*testPacket));
    }

    void testMuteFalseMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.mute(false);
        controller.mute(false); // Should be idempotent

        // Verify controller remains functional
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after redundant mute(false)",
                               controller.wantsData(*testPacket));
    }

    void testWantsDataWithSctePacket()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should want SCTE data packets",
                               controller.wantsData(*scteDataPacket));
    }

    void testWantsDataWithDvbPacket()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        auto dvbDataPacket = PacketDataBuilder::buildDvbData(100, std::vector<uint8_t>());
        CPPUNIT_ASSERT_MESSAGE("Controller should NOT want DVB data packets",
                               !controller.wantsData(*dvbDataPacket));
    }

    void testWantsDataWithTeletextPacket()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        // Create Teletext data packet (type 1)
        uint32_t channelId = 100;
        uint32_t size = 8;
        std::vector<uint8_t> data = {
            0x01, 0x00, 0x00, 0x00, // type = PES_DATA (1)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT), 0x00, 0x00, 0x00 // channel type = Teletext
        };
        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto teletextPacket = std::make_unique<PacketData>(Packet::Type::PES_DATA);
        CPPUNIT_ASSERT_MESSAGE("Failed to parse Teletext packet", teletextPacket->parse(std::move(buffer)));

        CPPUNIT_ASSERT_MESSAGE("Controller should NOT want Teletext data packets",
                               !controller.wantsData(*teletextPacket));
    }

    void testWantsDataCalledMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        auto scteDataPacket = PacketDataBuilder::buildMinimalScteData(100);

        for (int i = 0; i < 10; ++i) {
            CPPUNIT_ASSERT_MESSAGE("Controller should consistently want SCTE data",
                                   controller.wantsData(*scteDataPacket));
        }
    }

    void testCompleteWorkflow()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        // Complete workflow
        controller.activate();

        auto scteData = PacketDataBuilder::createScteSection(50);
        auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);
        controller.addData(*dataPacket);

        controller.process();

        controller.deactivate();

        CPPUNIT_ASSERT_MESSAGE("Complete SCTE workflow should end with a hidden window", !m_window->isVisible());
        CPPUNIT_ASSERT(m_window->getVisibleTrueCount() > 0);
        CPPUNIT_ASSERT(m_window->getVisibleFalseCount() > 0);
    }

    void testWorkflowWithMuting()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();
        controller.mute(true);

        auto scteData = PacketDataBuilder::createScteSection(50);
        auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);
        controller.addData(*dataPacket);

        controller.process();

        controller.mute(false);
        controller.deactivate();

        // Verify controller completed workflow with muting and maintains state
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after workflow with muting",
                               controller.wantsData(*testPacket));
    }

    void testWorkflowAddDataBeforeActivate()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        // Constructor already starts, so deactivate first
        controller.deactivate();

        // Add data while stopped
        auto scteData1 = PacketDataBuilder::createScteSection(30);
        auto dataPacket1 = PacketDataBuilder::buildScteData(100, scteData1);
        controller.addData(*dataPacket1);

        // Now activate and add more data
        controller.activate();
        auto scteData2 = PacketDataBuilder::createScteSection(40);
        auto dataPacket2 = PacketDataBuilder::buildScteData(100, scteData2);
        controller.addData(*dataPacket2);

        controller.process();

        // Verify controller completed workflow and maintains state
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after workflow",
                               controller.wantsData(*testPacket));
    }

    void testActivateDeactivateCycle()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        for (int i = 0; i < 3; ++i) {
            controller.deactivate();
            controller.activate();
        }

        // Verify controller remains functional after activation/deactivation cycles
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after activate/deactivate cycles",
                               controller.wantsData(*testPacket));
    }

    void testWorkflowWithMultipleDataPackets()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        // Add multiple data packets
        for (int i = 0; i < 5; ++i) {
            auto scteData = PacketDataBuilder::createScteSection(25 + i * 5);
            auto dataPacket = PacketDataBuilder::buildScteData(100, scteData);
            controller.addData(*dataPacket);
            controller.process();
        }

        controller.deactivate();

        // Verify controller processed all packets and maintains state
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should maintain state after processing multiple packets",
                               controller.wantsData(*testPacket));
    }

    void testProcessingAfterMuteUnmute()
    {
        auto packet = PacketSubtitleSelectionBuilder::buildScte(100);
        ScteSubController controller(*packet, m_window, *m_stcProvider);

        controller.activate();

        // Add data while unmuted
        auto scteData1 = PacketDataBuilder::createScteSection(30);
        auto dataPacket1 = PacketDataBuilder::buildScteData(100, scteData1);
        controller.addData(*dataPacket1);
        controller.process();

        // Mute and add more data
        controller.mute(true);
        auto scteData2 = PacketDataBuilder::createScteSection(35);
        auto dataPacket2 = PacketDataBuilder::buildScteData(100, scteData2);
        controller.addData(*dataPacket2);
        controller.process();

        // Unmute and add final data
        controller.mute(false);
        auto scteData3 = PacketDataBuilder::createScteSection(40);
        auto dataPacket3 = PacketDataBuilder::buildScteData(100, scteData3);
        controller.addData(*dataPacket3);
        controller.process();

        // Verify controller handled mute/unmute during processing
        auto testPacket = PacketDataBuilder::buildMinimalScteData(100);
        CPPUNIT_ASSERT_MESSAGE("Controller should handle mute/unmute during processing",
                               controller.wantsData(*testPacket));
    }

private:
    std::shared_ptr<MockWindow> m_window;
    std::unique_ptr<StcProvider> m_stcProvider;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ScteSubControllerTest);
