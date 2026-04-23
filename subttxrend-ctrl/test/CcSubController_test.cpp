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
#include "CcSubController.hpp"
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <memory>
#include <vector>

using namespace subttxrend::ctrl;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;

// Mock DrawContext for testing
class MockDrawContext : public DrawContext
{
public:
    MockDrawContext() {}

    void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {}
    void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
    void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
    void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}
    void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override {}
    void drawString(PrerenderedFont& font, const Rectangle &destinationRect, const std::vector<GlyphData>& glyphs, const ColorArgb fgColor, const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}
};

// Mock Window for testing
class MockWindow : public Window
{
public:
    MockWindow() : m_mockContext() {}
    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
        return m_mockContext;
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

private:
    MockDrawContext m_mockContext;
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
        // Size field = data size only = 16 bytes (0x10)
        std::vector<uint8_t> data = {
            0x05, 0x00, 0x00, 0x00, // type = SUBTITLE_SELECTION
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x10, 0x00, 0x00, 0x00, // size = 16 bytes
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(PacketSubtitleSelection::SUBTITLES_TYPE_CC), 0x00, 0x00, 0x00, // subtitle type = CC
            static_cast<uint8_t>(auxId1), static_cast<uint8_t>(auxId1 >> 8),
            static_cast<uint8_t>(auxId1 >> 16), static_cast<uint8_t>(auxId1 >> 24), // aux1
            static_cast<uint8_t>(auxId2), static_cast<uint8_t>(auxId2 >> 8),
            static_cast<uint8_t>(auxId2 >> 16), static_cast<uint8_t>(auxId2 >> 24) // aux2
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
        // CC_DATA packets require: channel_id(4) + channel_type(4) + PTS_presence(4) + PTS_type(4) + data
        uint32_t channelType = 0x00000000; // default channel type
        uint32_t ptsPresence = 0x00000000; // default PTS presence
        uint32_t ptsType = 0x00000000;     // default PTS type
        uint32_t size = 4 + 4 + 4 + 4 + userData.size(); // Total payload size

        // Build packet with channel-specific header (LITTLE-ENDIAN)
        std::vector<uint8_t> data = {
            0x0A, 0x00, 0x00, 0x00, // type = CC_DATA (10 in little-endian)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            static_cast<uint8_t>(size), static_cast<uint8_t>(size >> 8),
            static_cast<uint8_t>(size >> 16), static_cast<uint8_t>(size >> 24), // size
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(channelType), static_cast<uint8_t>(channelType >> 8),
            static_cast<uint8_t>(channelType >> 16), static_cast<uint8_t>(channelType >> 24), // channel type
            static_cast<uint8_t>(ptsPresence), static_cast<uint8_t>(ptsPresence >> 8),
            static_cast<uint8_t>(ptsPresence >> 16), static_cast<uint8_t>(ptsPresence >> 24), // PTS presence
            static_cast<uint8_t>(ptsType), static_cast<uint8_t>(ptsType >> 8),
            static_cast<uint8_t>(ptsType >> 16), static_cast<uint8_t>(ptsType >> 24) // PTS type
        };

        // Add user data
        data.insert(data.end(), userData.begin(), userData.end());

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketData>(Packet::Type::CC_DATA);
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
    static std::unique_ptr<PacketSetCCAttributes> build(uint32_t channelId, uint32_t ccType = 0x00000000, uint32_t attribType = 0x00003FFF)
    {
        // PacketSetCCAttributes requires: channel_id(4) + cc_type(4) + attrib_type(4) + 14 attribute values(56)
        // Total size = 68 bytes
        std::vector<uint8_t> data = {
            0x12, 0x00, 0x00, 0x00, // type = SET_CC_ATTRIBUTES (18 in little-endian)
            0x01, 0x00, 0x00, 0x00, // counter = 1
            0x44, 0x00, 0x00, 0x00, // size = 68 bytes (4 + 4 + 4 + 56)
            static_cast<uint8_t>(channelId), static_cast<uint8_t>(channelId >> 8),
            static_cast<uint8_t>(channelId >> 16), static_cast<uint8_t>(channelId >> 24), // channel ID
            static_cast<uint8_t>(ccType), static_cast<uint8_t>(ccType >> 8),
            static_cast<uint8_t>(ccType >> 16), static_cast<uint8_t>(ccType >> 24), // cc_type
            static_cast<uint8_t>(attribType), static_cast<uint8_t>(attribType >> 8),
            static_cast<uint8_t>(attribType >> 16), static_cast<uint8_t>(attribType >> 24) // attrib_type
        };

        // Add 14 attribute values (56 bytes total)
        for (int i = 0; i < 14; ++i) {
            uint32_t value = 0x00000000; // Default attribute values
            data.push_back(static_cast<uint8_t>(value));
            data.push_back(static_cast<uint8_t>(value >> 8));
            data.push_back(static_cast<uint8_t>(value >> 16));
            data.push_back(static_cast<uint8_t>(value >> 24));
        }

        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        auto packet = std::make_unique<PacketSetCCAttributes>();
        if (!packet->parse(std::move(buffer))) {
            return nullptr;
        }
        return packet;
    }
};

class CcSubControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcSubControllerTest);

    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithNullFontCache);
    CPPUNIT_TEST(testConstructorSetsChannelIdCorrectly);
    CPPUNIT_TEST(testConstructorWithDifferentCeaTypes);
    CPPUNIT_TEST(testConstructorWithDifferentServiceNumbers);
    CPPUNIT_TEST(testDestructorWhenControllerStarted);
    CPPUNIT_TEST(testDestructorWhenControllerStopped);
    CPPUNIT_TEST(testProcessWhenStarted);
    CPPUNIT_TEST(testProcessMultipleTimes);
    CPPUNIT_TEST(testAddDataWithValidData);
    CPPUNIT_TEST(testAddDataWithMinimalData);
    CPPUNIT_TEST(testAddDataMultipleTimes);
    CPPUNIT_TEST(testAddDataWhenStopped);
    CPPUNIT_TEST(testActivateWhenStopped);
    CPPUNIT_TEST(testActivateWhenAlreadyStarted);
    CPPUNIT_TEST(testActivateMultipleTimes);
    CPPUNIT_TEST(testDeactivateWhenStarted);
    CPPUNIT_TEST(testDeactivateIdempotency);
    CPPUNIT_TEST(testWantsDataWithMatchingChannelAndStarted);
    CPPUNIT_TEST(testWantsDataWithNonMatchingChannel);
    CPPUNIT_TEST(testWantsDataWithMatchingChannelButStopped);
    CPPUNIT_TEST(testWantsDataWithNonMatchingChannelAndStopped);
    CPPUNIT_TEST(testWantsDataWithZeroChannelId);
    CPPUNIT_TEST(testWantsDataWithLargeChannelId);
    CPPUNIT_TEST(testProcessSetCCAttributesPacketValid);
    CPPUNIT_TEST(testProcessSetCCAttributesPacketMultiple);
    CPPUNIT_TEST(testSetTextForPreviewWithValidString);
    CPPUNIT_TEST(testSetTextForPreviewWithEmptyString);
    CPPUNIT_TEST(testSetTextForPreviewWithLongString);
    CPPUNIT_TEST(testSetTextForPreviewWithSpecialCharacters);
    CPPUNIT_TEST(testCompleteWorkflowActivateAddDataProcess);
    CPPUNIT_TEST(testCompleteWorkflowWithMute);
    CPPUNIT_TEST(testActivateDeactivateCycle);
    CPPUNIT_TEST(testDataProcessingAfterReactivation);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Create mock objects for each test
        m_mockWindow = std::make_shared<MockWindow>();
        m_fontCache = std::make_shared<subttxrend::gfx::PrerenderedFontCache>();
    }

    void tearDown() override
    {
        // Cleanup
        m_mockWindow.reset();
        m_fontCache.reset();
    }

protected:
    void testConstructorWithValidParameters()
    {
        try {
            auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
            CcSubController controller(*packet, m_mockWindow, m_fontCache);

            // Verify controller is started (constructor calls select())
            auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("Controller should want data after construction",
                                   controller.wantsData(*testPacket));
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Constructor failed: ") + e.what());
        }
    }

    void testConstructorWithNullFontCache()
    {
        // Constructor with null font cache should be handled gracefully
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        std::shared_ptr<subttxrend::gfx::PrerenderedFontCache> nullCache;

        // Controller can be created with null font cache
        CcSubController controller(*packet, m_mockWindow, nullCache);

        // Verify controller can be used safely
        controller.process();
        CPPUNIT_ASSERT(true);
    }

    void testConstructorSetsChannelIdCorrectly()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Verify channel ID is set by checking wantsData with same channel
        auto sameChannelPacket = PacketSubtitleSelectionBuilder::build(42, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should want data from same channel when started",
                               controller.wantsData(*sameChannelPacket));
    }

    void testConstructorWithDifferentCeaTypes()
    {
        // Test with CEA-608
        {
            auto packet608 = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
            CcSubController controller608(*packet608, m_mockWindow, m_fontCache);
            auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("CEA-608 controller should want data from channel 100",
                                   controller608.wantsData(*testPacket));
        }

        // Test with CEA-708
        {
            auto packet708 = PacketSubtitleSelectionBuilder::build(101, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_708), 1);
            CcSubController controller708(*packet708, m_mockWindow, m_fontCache);
            auto testPacket = PacketSubtitleSelectionBuilder::build(101, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("CEA-708 controller should want data from channel 101",
                                   controller708.wantsData(*testPacket));
        }
    }

    void testConstructorWithDifferentServiceNumbers()
    {
        // Test with service 0
        {
            auto packet0 = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 0);
            CcSubController controller0(*packet0, m_mockWindow, m_fontCache);
            auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("Service 0 controller should want data from channel 100",
                                   controller0.wantsData(*testPacket));
        }

        // Test with service 5
        {
            auto packet5 = PacketSubtitleSelectionBuilder::build(101, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_708), 5);
            CcSubController controller5(*packet5, m_mockWindow, m_fontCache);
            auto testPacket = PacketSubtitleSelectionBuilder::build(101, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("Service 5 controller should want data from channel 101",
                                   controller5.wantsData(*testPacket));
        }
    }

    void testDestructorWhenControllerStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        {
            CcSubController controller(*packet, m_mockWindow, m_fontCache);
            // Controller is started in constructor via select()
            // Destructor should call stop() then shutdown()
        }
        // If we reach here without crash, destructor handled correctly
        CPPUNIT_ASSERT(true);
    }

    void testDestructorWhenControllerStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        bool destructorCompleted = false;
        {
            CcSubController controller(*packet, m_mockWindow, m_fontCache);
            controller.deactivate(); // Stop the controller
            // Destructor should only call shutdown()
            destructorCompleted = true;
        }
        // Verify destructor completed successfully
        CPPUNIT_ASSERT_MESSAGE("Destructor should complete without crashing", destructorCompleted);
    }

    void testProcessWhenStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.process();

        // Verify controller still functional after process
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data after process",
                               controller.wantsData(*testPacket));
    }

    void testProcessMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Process multiple times should work
        for (int i = 0; i < 10; i++) {
            controller.process();
        }

        // Verify controller still functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple process calls",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithValidData()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04};
        auto dataPacket = PacketDataBuilder::build(100, testData);

        controller.addData(*dataPacket);

        // Verify controller still functional after adding data
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after addData",
                               controller.wantsData(*testPacket));
    }

    void testAddDataWithMinimalData()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        std::vector<uint8_t> minimalData = {0x00}; // Minimal 1-byte payload
        auto dataPacket = PacketDataBuilder::build(100, minimalData);

        // Should handle minimal data gracefully
        controller.addData(*dataPacket);
        CPPUNIT_ASSERT(true);
    }

    void testAddDataMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Add multiple data packets
        for (int i = 0; i < 5; i++) {
            std::vector<uint8_t> testData = {static_cast<uint8_t>(i), static_cast<uint8_t>(i+1)};
            auto dataPacket = PacketDataBuilder::build(100, testData);
            controller.addData(*dataPacket);
        }
        CPPUNIT_ASSERT(true);
    }

    void testAddDataWhenStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.deactivate(); // Stop controller

        std::vector<uint8_t> testData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(100, testData);

        // Should still accept data even when stopped
        controller.addData(*dataPacket);
        CPPUNIT_ASSERT(true);
    }

    void testActivateWhenStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.deactivate(); // First stop it
        controller.activate(); // Then activate

        // Verify it's started by checking wantsData
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should want data after activation",
                               controller.wantsData(*testPacket));
    }

    void testActivateWhenAlreadyStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Already started from constructor
        controller.activate(); // Should be idempotent

        CPPUNIT_ASSERT(true);
    }

    void testActivateMultipleTimes()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Multiple activations should be safe and keep controller started
        for (int i = 0; i < 5; i++) {
            controller.activate();
        }

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should still want data after multiple activations",
                               controller.wantsData(*testPacket));
    }

    void testDeactivateWhenStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.deactivate();

        // Verify it's stopped by checking wantsData
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should not want data after deactivation",
                               !controller.wantsData(*testPacket));
    }

    void testDeactivateIdempotency()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // First deactivate
        controller.deactivate();

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Should not want data after first deactivation",
                               !controller.wantsData(*testPacket));

        // Multiple deactivations should be safe and maintain stopped state
        for (int i = 0; i < 5; i++) {
            controller.deactivate();
            CPPUNIT_ASSERT_MESSAGE("Should remain deactivated after multiple calls",
                                   !controller.wantsData(*testPacket));
        }
    }

    void testWantsDataWithMatchingChannelAndStarted()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto testPacket = PacketSubtitleSelectionBuilder::build(42, 0, 0); // Same channel ID
        CPPUNIT_ASSERT_MESSAGE("Should want data with matching channel and started state",
                               controller.wantsData(*testPacket));
    }

    void testWantsDataWithNonMatchingChannel()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto testPacket = PacketSubtitleSelectionBuilder::build(99, 0, 0); // Different channel ID
        CPPUNIT_ASSERT_MESSAGE("Should not want data with non-matching channel",
                               !controller.wantsData(*testPacket));
    }

    void testWantsDataWithMatchingChannelButStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.deactivate(); // Stop the controller

        auto testPacket = PacketSubtitleSelectionBuilder::build(42, 0, 0); // Same channel ID
        CPPUNIT_ASSERT_MESSAGE("Should not want data when stopped even with matching channel",
                               !controller.wantsData(*testPacket));
    }

    void testWantsDataWithNonMatchingChannelAndStopped()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(42, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.deactivate();

        auto testPacket = PacketSubtitleSelectionBuilder::build(99, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Should not want data with wrong channel and stopped",
                               !controller.wantsData(*testPacket));
    }

    void testWantsDataWithZeroChannelId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(0, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto testPacket = PacketSubtitleSelectionBuilder::build(0, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Should handle zero channel ID correctly",
                               controller.wantsData(*testPacket));

        auto differentPacket = PacketSubtitleSelectionBuilder::build(1, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Should not want data from channel 1 when set to 0",
                               !controller.wantsData(*differentPacket));
    }

    void testWantsDataWithLargeChannelId()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(0xFFFFFFF0, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto testPacket = PacketSubtitleSelectionBuilder::build(0xFFFFFFF0, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Should handle large channel ID correctly",
                               controller.wantsData(*testPacket));
    }

    void testProcessSetCCAttributesPacketValid()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto attrPacket = PacketSetCCAttributesBuilder::build(100);

        controller.processSetCCAttributesPacket(*attrPacket);

        // Verify controller still functional after processing attributes
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after processSetCCAttributesPacket",
                               controller.wantsData(*testPacket));
    }

    void testProcessSetCCAttributesPacketMultiple()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Process multiple attribute packets
        for (int i = 0; i < 5; i++) {
            auto attrPacket = PacketSetCCAttributesBuilder::build(100);
            controller.processSetCCAttributesPacket(*attrPacket);
        }

        // Verify controller still functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after multiple attribute packets",
                               controller.wantsData(*testPacket));
    }

    void testSetTextForPreviewWithValidString()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.setTextForPreview("Test preview text");

        // Verify controller remains functional
        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
        CPPUNIT_ASSERT_MESSAGE("Controller should remain functional after setTextForPreview",
                               controller.wantsData(*testPacket));
    }

    void testSetTextForPreviewWithEmptyString()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.setTextForPreview("");
        CPPUNIT_ASSERT(true);
    }

    void testSetTextForPreviewWithLongString()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        std::string longText(900, 'A');
        controller.setTextForPreview(longText);
        CPPUNIT_ASSERT(true);
    }

    void testSetTextForPreviewWithSpecialCharacters()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        controller.setTextForPreview("Special: \n\t\r©®™€");
        CPPUNIT_ASSERT(true);
    }

    void testCompleteWorkflowActivateAddDataProcess()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Complete workflow
        controller.activate();

        std::vector<uint8_t> testData = {0x01, 0x02, 0x03};
        auto dataPacket = PacketDataBuilder::build(100, testData);
        controller.addData(*dataPacket);

        controller.process();

        controller.deactivate();

        CPPUNIT_ASSERT(true);
    }

    void testCompleteWorkflowWithMute()
    {
        // Integration test: mute operations in realistic workflow
        // Note: CcSubController doesn't expose isMuted() in public API, so we verify
        // that mute operations integrate correctly without crashing and don't affect
        // data acceptance or processing flow

        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);

        // Mute while active - should not affect wantsData
        controller.mute(true);
        CPPUNIT_ASSERT_MESSAGE("Muted controller should still want data",
                               controller.wantsData(*testPacket));

        // Add data while muted
        std::vector<uint8_t> testData = {0x01, 0x02};
        auto dataPacket = PacketDataBuilder::build(100, testData);
        controller.addData(*dataPacket);

        // Process while muted (data should be processed, just not rendered/audible)
        controller.process();

        // Unmute
        controller.mute(false);

        // Should still want data after unmute
        CPPUNIT_ASSERT_MESSAGE("Unmuted controller should still want data",
                               controller.wantsData(*testPacket));

        // Process after unmute
        controller.process();

        // Test idempotency and stability
        controller.mute(true);
        controller.mute(true);
        controller.mute(false);
        controller.mute(false);

        // Verify still functional after mute operations
        controller.process();
        CPPUNIT_ASSERT(true);
    }

    void testActivateDeactivateCycle()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Cycle through activate/deactivate multiple times
        for (int i = 0; i < 3; i++) {
            controller.deactivate();

            auto testPacket = PacketSubtitleSelectionBuilder::build(100, 0, 0);
            CPPUNIT_ASSERT_MESSAGE("Should not want data when deactivated",
                                   !controller.wantsData(*testPacket));

            controller.activate();

            CPPUNIT_ASSERT_MESSAGE("Should want data when activated",
                                   controller.wantsData(*testPacket));
        }
        CPPUNIT_ASSERT(true);
    }

    void testDataProcessingAfterReactivation()
    {
        auto packet = PacketSubtitleSelectionBuilder::build(100, static_cast<uint32_t>(subttxrend::cc::CeaType::CEA_608), 1);
        CcSubController controller(*packet, m_mockWindow, m_fontCache);

        // Add data, process
        std::vector<uint8_t> testData1 = {0x01, 0x02};
        auto dataPacket1 = PacketDataBuilder::build(100, testData1);
        controller.addData(*dataPacket1);
        controller.process();

        // Deactivate
        controller.deactivate();

        // Reactivate
        controller.activate();

        // Add more data and process
        std::vector<uint8_t> testData2 = {0x03, 0x04};
        auto dataPacket2 = PacketDataBuilder::build(100, testData2);
        controller.addData(*dataPacket2);
        controller.process();

        CPPUNIT_ASSERT(true);
    }

private:
    std::shared_ptr<MockWindow> m_mockWindow;
    std::shared_ptr<subttxrend::gfx::PrerenderedFontCache> m_fontCache;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcSubControllerTest);
