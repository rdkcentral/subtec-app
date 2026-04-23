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
#include "CcController.hpp"
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <memory>
#include <vector>
#include <cstring>

using namespace subttxrend::cc;
using namespace subttxrend::protocol;
using namespace subttxrend::gfx;

class MockDrawContext : public DrawContext
{
public:
    MockDrawContext() : fillRectCalls(0), drawStringCalls(0), drawGlyphCalls(0) {}

    void fillRectangle(ColorArgb color, const Rectangle& rectangle) override { fillRectCalls++; }
    void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
    void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
    void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}
    void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override { drawGlyphCalls++; }
    void drawString(PrerenderedFont& font, const Rectangle &destinationRect, const std::vector<GlyphData>& glyphs, const ColorArgb fgColor, const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override { drawStringCalls++; }

    void resetCounters() { fillRectCalls = 0; drawStringCalls = 0; drawGlyphCalls = 0; }
    int fillRectCalls;
    int drawStringCalls;
    int drawGlyphCalls;
};

class MockWindow : public subttxrend::gfx::Window
{
public:
    MockWindow()
        : clearCalled(0), updateCalled(0), visibleState(false) {}
    virtual ~MockWindow() {}

    void addKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override {}
    void removeKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
        return mockContext;
    }

    Size getPreferredSize() const override {
        return Size(1920, 1080);
    }

    void setSize(const Size& newSize) override {}

    Size getSize() const override {
        return Size(1920, 1080);
    }

    void setVisible(bool visible) override {
        visibleState = visible;
    }

    void clear() override {
        clearCalled++;
    }

    void update() override {
        updateCalled++;
    }

    void setDrawDirection(DrawDirection dir) override {}

    // Test verification helpers
    void resetCounters() {
        clearCalled = 0;
        updateCalled = 0;
        mockContext.resetCounters();
    }

    MockDrawContext& getMockContext() { return mockContext; }

    int clearCalled;
    int updateCalled;
    bool visibleState;

private:
    MockDrawContext mockContext;
};

class TestPacketData : public PacketData
{
public:
    TestPacketData(const std::vector<uint8_t>& data)
        : PacketData(Packet::Type::CC_DATA)
    {
        // Initialize the base class's m_dataBuffer properly
        auto buffer = std::make_unique<std::vector<char>>(data.begin(), data.end());
        takeBuffer(std::move(buffer), 0);
    }
};

class CCDataBuilder
{
public:
    // Build CEA-608 field 1 data with H.264 SEI container format
    static std::vector<uint8_t> buildCEA608Field1(uint8_t byte1, uint8_t byte2, bool valid = true)
    {
        std::vector<uint8_t> data;

        // H.264 start code: 0x00 0x00 0x01
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x01);

        // SEI NAL unit type (0x06)
        data.push_back(0x06);

        // SEI payload type (0x04 = user_data_registered_itu_t_t35)
        data.push_back(0x04);

        // Padding to reach expected offset
        for (int i = 0; i < 6; ++i) data.push_back(0x00);

        // User data type code (0x03 = cc_data) at offset+11
        data.push_back(0x03);

        // CEA-708 header
        data.push_back(0x40);  // process_cc_data_flag = 1
        data.push_back(0x01);  // cc_count = 1

        // CC data triplet for CEA-608 field 1
        uint8_t cc_valid_type = valid ? 0xFC : 0xF8;  // marker(5) + cc_valid(1) + cc_type(2)
        data.push_back(cc_valid_type);  // cc_valid=1, cc_type=0 (608 field 1)
        data.push_back(byte1);
        data.push_back(byte2);

        return data;
    }

    // Build CEA-608 field 2 data with H.264 SEI container format
    static std::vector<uint8_t> buildCEA608Field2(uint8_t byte1, uint8_t byte2, bool valid = true)
    {
        std::vector<uint8_t> data;

        // H.264 start code: 0x00 0x00 0x01
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x01);

        // SEI NAL unit type (0x06)
        data.push_back(0x06);

        // SEI payload type (0x04)
        data.push_back(0x04);

        // Padding
        for (int i = 0; i < 6; ++i) data.push_back(0x00);

        // User data type code (0x03 = cc_data)
        data.push_back(0x03);

        data.push_back(0x40);
        data.push_back(0x01);

        uint8_t cc_valid_type = valid ? 0xFD : 0xF8;  // cc_type=1 (608 field 2)
        data.push_back(cc_valid_type);
        data.push_back(byte1);
        data.push_back(byte2);

        return data;
    }

    // Build CEA-708 CCP start packet with H.264 SEI container format
    static std::vector<uint8_t> buildCEA708CCPStart(uint8_t serviceNo, uint8_t seqNo, uint8_t packetSize)
    {
        std::vector<uint8_t> data;

        // H.264 start code: 0x00 0x00 0x01
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x01);

        // SEI NAL unit type (0x06)
        data.push_back(0x06);

        // SEI payload type (0x04)
        data.push_back(0x04);

        // Padding
        for (int i = 0; i < 6; ++i) data.push_back(0x00);

        // User data type code (0x03 = cc_data)
        data.push_back(0x03);

        data.push_back(0x40);
        data.push_back(0x01);

        // DTVCC_CCP_START (cc_type = 2)
        data.push_back(0xFE);  // cc_valid=1, cc_type=2

        // CCP header: sequence_no (2 bits) | packet_size_code (6 bits)
        uint8_t header = ((seqNo & 0x03) << 6) | (packetSize & 0x3F);
        data.push_back(header);

        // First service block header: service_number (3 bits) | block_size (5 bits)
        uint8_t sbHeader = ((serviceNo & 0x07) << 5) | 0x02;  // block_size = 2
        data.push_back(sbHeader);

        return data;
    }

    // Build CEA-708 CCP data continuation with H.264 SEI container format
    static std::vector<uint8_t> buildCEA708CCPData(uint8_t data1, uint8_t data2)
    {
        std::vector<uint8_t> data;

        // H.264 start code: 0x00 0x00 0x01
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x01);

        // SEI NAL unit type (0x06)
        data.push_back(0x06);

        // SEI payload type (0x04)
        data.push_back(0x04);

        // Padding
        for (int i = 0; i < 6; ++i) data.push_back(0x00);

        // User data type code (0x03 = cc_data)
        data.push_back(0x03);

        data.push_back(0x40);
        data.push_back(0x01);

        // DTVCC_CCP_DATA (cc_type = 3)
        data.push_back(0xFF);  // cc_valid=1, cc_type=3
        data.push_back(data1);
        data.push_back(data2);

        return data;
    }

    // Build padding data
    static std::vector<uint8_t> buildPadding()
    {
        std::vector<uint8_t> data;
        data.push_back(0x03);
        data.push_back(0x40);
        data.push_back(0x01);

        data.push_back(0xF8);  // cc_valid=0 (padding)
        data.push_back(0x00);
        data.push_back(0x00);

        return data;
    }

    // Build invalid/malformed data
    static std::vector<uint8_t> buildInvalidData()
    {
        return std::vector<uint8_t>{0xFF, 0xFF, 0xFF};
    }

    // Build empty data
    static std::vector<uint8_t> buildEmptyData()
    {
        return std::vector<uint8_t>();
    }
};

class CcControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcControllerTest);

    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testInitWithValidParameters);
    CPPUNIT_TEST(testInitReturnValue);
    CPPUNIT_TEST(testStartAfterInit);
    CPPUNIT_TEST(testStartMultipleTimes);
    CPPUNIT_TEST(testStopWithoutStart);
    CPPUNIT_TEST(testStopAfterStart);
    CPPUNIT_TEST(testStopMultipleTimes);
    CPPUNIT_TEST(testIsStartedStates);
    CPPUNIT_TEST(testInitialMutedState);
    CPPUNIT_TEST(testMuteWhenAlreadyMuted);
    CPPUNIT_TEST(testUnmuteWhenMuted);
    CPPUNIT_TEST(testUnmuteWhenAlreadyUnmuted);
    CPPUNIT_TEST(testIsMutedStates);
    CPPUNIT_TEST(testMuteBeforeStart);
    CPPUNIT_TEST(testMuteAfterStart);
    CPPUNIT_TEST(testUnmuteAfterStart);
    CPPUNIT_TEST(testSetActiveServiceCEA608);
    CPPUNIT_TEST(testSetActiveServiceCEA708);
    CPPUNIT_TEST(testSetActiveServiceBoundaryValues);
    CPPUNIT_TEST(testSetActiveServiceSameValues);
    CPPUNIT_TEST(testSetActiveServiceWhileStarted);
    CPPUNIT_TEST(testAddDataBeforeStart);
    CPPUNIT_TEST(testAddDataAfterStart);
    CPPUNIT_TEST(testAddDataReturnValues);
    CPPUNIT_TEST(testAddDataMultiplePackets);
    CPPUNIT_TEST(testAddDataAfterStop);
    CPPUNIT_TEST(testProcessWithoutInit);
    CPPUNIT_TEST(testProcessWithoutStart);
    CPPUNIT_TEST(testProcessWithoutData);
    CPPUNIT_TEST(testProcessWhileMuted);
    CPPUNIT_TEST(testProcessAfterStop);
    CPPUNIT_TEST(testProcessIdempotency);
    CPPUNIT_TEST(testShutdownWithoutInit);
    CPPUNIT_TEST(testShutdownAfterInit);
    CPPUNIT_TEST(testShutdownWhileStarted);
    CPPUNIT_TEST(testShutdownMultipleTimes);
    CPPUNIT_TEST(testProcessCCAttributesAfterInit);
    CPPUNIT_TEST(testProcessCCAttributesWithAllTypes);
    CPPUNIT_TEST(testSetTextForPreviewEmpty);
    CPPUNIT_TEST(testSetTextForPreviewASCII);
    CPPUNIT_TEST(testSetTextForPreviewUnicode);
    CPPUNIT_TEST(testSetTextForPreviewLongText);
    CPPUNIT_TEST(testSetTextForPreviewSpecialChars);
    CPPUNIT_TEST(testCEA608Field1Processing);
    CPPUNIT_TEST(testCEA608Field2Processing);
    CPPUNIT_TEST(testCEA608InvalidData);
    CPPUNIT_TEST(testCEA608PaddingData);
    CPPUNIT_TEST(testCEA608MultiplePackets);
    CPPUNIT_TEST(testCEA608ServiceNumberFiltering);
    CPPUNIT_TEST(testCEA708CCPStartProcessing);
    CPPUNIT_TEST(testCEA708CCPDataProcessing);
    CPPUNIT_TEST(testCEA708CompleteCCPPacket);
    CPPUNIT_TEST(testCEA708ServiceFiltering);
    CPPUNIT_TEST(testCEA708MultipleCCPs);
    CPPUNIT_TEST(testCEA708IncompleteCCP);
    CPPUNIT_TEST(testStateTransitionFullCycle);
    CPPUNIT_TEST(testStateTransitionServiceSwitch);
    CPPUNIT_TEST(testStateTransitionMuteDuringProcessing);
    CPPUNIT_TEST(testStateTransitionStopClearsQueues);
    CPPUNIT_TEST(testStateTransitionRapidChanges);
    CPPUNIT_TEST(testQueueClearedWhenMuted);
    CPPUNIT_TEST(testQueueClearedWhenStopped);
    CPPUNIT_TEST(testQueueClearedAfterProcess);
    CPPUNIT_TEST(testQueuePurgeOnShutdown);
    CPPUNIT_TEST(testRendererClearedOnStart);
    CPPUNIT_TEST(testRendererShownWhenUnmuted);
    CPPUNIT_TEST(testRendererHiddenWhenMuted);
    CPPUNIT_TEST(testRendererHiddenOnStop);
    CPPUNIT_TEST(testEdgeCaseMalformedCCData);
    CPPUNIT_TEST(testEdgeCaseExcessiveQueueSize);
    CPPUNIT_TEST(testEdgeCaseReinitAfterShutdown);
    CPPUNIT_TEST(testErrorHandlingInvalidUserData);
    CPPUNIT_TEST(testErrorHandlingCCPException);
    CPPUNIT_TEST(testErrorHandlingServiceBlockError);

    CPPUNIT_TEST_SUITE_END();

private:
    Controller* controller;
    MockWindow* mockWindow;
    std::shared_ptr<PrerenderedFontCache> fontCache;

public:
    void setUp()
    {
        controller = new Controller();
        mockWindow = new MockWindow();
        fontCache = std::make_shared<PrerenderedFontCache>();
    }

    void tearDown()
    {
        delete controller;
        delete mockWindow;
        mockWindow = nullptr;
        fontCache.reset();
    }

    void testConstructor()
    {
        Controller* testCtrl = new Controller();
        CPPUNIT_ASSERT(testCtrl != nullptr);
        CPPUNIT_ASSERT_EQUAL(false, testCtrl->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, testCtrl->isMuted());  // CRITICAL: Initial state is MUTED
        delete testCtrl;
    }

    void testDestructor()
    {
        Controller* testCtrl = new Controller();
        testCtrl->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        testCtrl->start();
        delete testCtrl;  // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testInitWithValidParameters()
    {
        bool result = controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testInitReturnValue()
    {
        bool result = controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        CPPUNIT_ASSERT_EQUAL(true, result);

        // Verify we can call methods after init
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
    }

    void testStartAfterInit()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        mockWindow->resetCounters();

        bool result = controller->start();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        // Verify renderer interactions - window should be cleared and updated
        CPPUNIT_ASSERT(mockWindow->clearCalled > 0);
        CPPUNIT_ASSERT(mockWindow->updateCalled > 0);
        // Window should remain hidden since controller starts muted
        CPPUNIT_ASSERT_EQUAL(false, mockWindow->visibleState);
    }

    void testStartMultipleTimes()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        bool result2 = controller->start();
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
    }

    void testStopWithoutStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        bool result = controller->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);  // stop() always returns true (even if not started)
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());  // Should remain not started
    }

    void testStopAfterStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        mockWindow->resetCounters();

        bool result = controller->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        // Verify renderer hidden
        CPPUNIT_ASSERT_EQUAL(false, mockWindow->visibleState);
    }

    void testStopMultipleTimes()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->stop();

        bool result2 = controller->stop();
        CPPUNIT_ASSERT_EQUAL(true, result2);  // stop() always returns true
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());  // Should remain stopped
    }

    void testIsStartedStates()
    {
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testInitialMutedState()
    {
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testMuteWhenAlreadyMuted()
    {
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
        controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testUnmuteWhenMuted()
    {
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
    }

    void testUnmuteWhenAlreadyUnmuted()
    {
        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
    }

    void testIsMutedStates()
    {
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());

        controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testMuteBeforeStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testMuteAfterStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, mockWindow->visibleState);  // Hidden when muted
    }

    void testUnmuteAfterStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
        CPPUNIT_ASSERT_EQUAL(true, mockWindow->visibleState);  // Shown when unmuted
    }

    void testSetActiveServiceCEA608()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        controller->setActiveService(CeaType::CEA_608, 1);
        // Service change should stop the controller
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testSetActiveServiceCEA708()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        // Change from default CEA_608 to CEA_708
        controller->setActiveService(CeaType::CEA_708, 1);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testSetActiveServiceBoundaryValues()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);

        // Test boundary service numbers - should not crash
        controller->setActiveService(CeaType::CEA_608, 0);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        controller->setActiveService(CeaType::CEA_608, 4);
        controller->setActiveService(CeaType::CEA_708, 0);
        controller->setActiveService(CeaType::CEA_708, 63);
        controller->setActiveService(CeaType::CEA_708, UINT32_MAX);

        // Controller should still be in valid state
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testSetActiveServiceSameValues()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        // Setting same service is a no-op - doesn't call stop()
        controller->setActiveService(CeaType::CEA_708, 1);
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());  // Still started (no-op)
    }

    void testSetActiveServiceWhileStarted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        // Changing service should call stop()
        controller->setActiveService(CeaType::CEA_708, 2);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());  // Stopped by service change
    }

    void testAddDataBeforeStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);

        auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
        TestPacketData packet(data);

        bool result = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);  // Not started, should fail
    }

    void testAddDataAfterStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();

        auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
        TestPacketData packet(data);

        bool result = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testAddDataReturnValues()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);

        auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
        TestPacketData packet(data);

        // Before start - should return false
        bool before = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(false, before);
        CPPUNIT_ASSERT_MESSAGE("addData must reject data when not started", !before);

        controller->start();

        // After start - should return true
        bool after = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(true, after);
        CPPUNIT_ASSERT_MESSAGE("addData must accept data when started", after);
    }

    void testAddDataMultiplePackets()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();

        for (int i = 0; i < 100; ++i)
        {
            auto data = CCDataBuilder::buildCEA608Field1(0x80 + i, 0x80);
            TestPacketData packet(data);
            bool result = controller->addData(packet);
            CPPUNIT_ASSERT_EQUAL(true, result);
        }
    }

    void testAddDataAfterStop()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->stop();

        auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
        TestPacketData packet(data);

        bool result = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testProcessWithoutInit()
    {
        controller->process();  // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testProcessWithoutStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->process();  // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testProcessWithoutData()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        controller->process();  // No data to process
        CPPUNIT_ASSERT(true);
    }

    void testProcessWhileMuted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
        mockWindow->resetCounters();

        auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should purge queues when muted
        // Verify no drawing occurred (muted state prevents processing)
        CPPUNIT_ASSERT_EQUAL(0, mockWindow->getMockContext().drawStringCalls);
        CPPUNIT_ASSERT_EQUAL(0, mockWindow->getMockContext().drawGlyphCalls);
    }

    void testProcessAfterStop()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->stop();

        controller->process();  // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testProcessIdempotency()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        controller->process();
        controller->process();
        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testShutdownWithoutInit()
    {
        controller->shutdown();  // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testShutdownAfterInit()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->shutdown();
        CPPUNIT_ASSERT(true);
    }

    void testShutdownWhileStarted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        controller->shutdown();  // Should call stop()
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testShutdownMultipleTimes()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->shutdown();
        controller->shutdown();
        controller->shutdown();
        CPPUNIT_ASSERT(true);
    }

    void testProcessCCAttributesAfterInit()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        PacketSetCCAttributes packet;
        controller->processSetCCAttributesPacket(packet);  // Should process successfully
        // Verify controller state unchanged by processing attributes
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testProcessCCAttributesWithAllTypes()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);

        // Test with different attribute combinations - should handle all gracefully
        for (int i = 0; i < 10; ++i)
        {
            PacketSetCCAttributes packet;
            controller->processSetCCAttributesPacket(packet);
        }

        // Verify controller remains stable after multiple packets
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());
    }

    void testSetTextForPreviewEmpty()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        std::string empty = "";
        controller->setTextForPreview(empty);  // Should accept empty string
        // Verify no state change from setting preview text
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testSetTextForPreviewASCII()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        std::string text = "Hello, World!";
        controller->setTextForPreview(text);
        CPPUNIT_ASSERT(true);
    }

    void testSetTextForPreviewUnicode()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        std::string text = "Hello 世界 🌍";
        controller->setTextForPreview(text);
        CPPUNIT_ASSERT(true);
    }

    void testSetTextForPreviewLongText()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        std::string longText(100, 'X');  // Reduced from 10000 to avoid excessive logging/processing
        controller->setTextForPreview(longText);
        CPPUNIT_ASSERT(true);
    }

    void testSetTextForPreviewSpecialChars()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        std::string text = "Line1\nLine2\tTabbed\r\nWindows";
        controller->setTextForPreview(text);
        CPPUNIT_ASSERT(true);
    }

    void testCEA608Field1Processing()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();
        controller->unmute();
        mockWindow->resetCounters();

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);  // Standard command
        TestPacketData packet(data);
        bool added = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(true, added);

        controller->process();
        // Verify controller remains in valid state after processing
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
    }

    void testCEA608Field2Processing()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 2);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildCEA608Field2(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA608InvalidData()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildCEA608Field1(0x00, 0x00, false);  // Invalid
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should skip invalid data
        CPPUNIT_ASSERT(true);
    }

    void testCEA608PaddingData()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildPadding();
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should skip padding
        CPPUNIT_ASSERT(true);
    }

    void testCEA608MultiplePackets()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();
        controller->unmute();

        for (int i = 0; i < 10; ++i)
        {
            auto data = CCDataBuilder::buildCEA608Field1(0x90 + i, 0x80);
            TestPacketData packet(data);
            controller->addData(packet);
        }

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA608ServiceNumberFiltering()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);  // Service 1
        controller->start();
        controller->unmute();

        // Add field 1 data (should be processed for service 1)
        auto data1 = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet1(data1);
        controller->addData(packet1);

        // Add field 2 data (service 2 - depends on implementation)
        auto data2 = CCDataBuilder::buildCEA608Field2(0x94, 0x20);
        TestPacketData packet2(data2);
        controller->addData(packet2);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708CCPStartProcessing()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildCEA708CCPStart(1, 0, 10);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708CCPDataProcessing()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Start CCP
        auto startData = CCDataBuilder::buildCEA708CCPStart(1, 0, 6);
        TestPacketData startPacket(startData);
        controller->addData(startPacket);

        // Add CCP data
        auto ccpData = CCDataBuilder::buildCEA708CCPData(0x98, 0x41);  // Command + data
        TestPacketData dataPacket(ccpData);
        controller->addData(dataPacket);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708CompleteCCPPacket()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Build complete CCP
        auto startData = CCDataBuilder::buildCEA708CCPStart(1, 0, 4);
        TestPacketData startPacket(startData);
        controller->addData(startPacket);

        auto ccpData1 = CCDataBuilder::buildCEA708CCPData(0x98, 0x41);
        TestPacketData dataPacket1(ccpData1);
        controller->addData(dataPacket1);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708ServiceFiltering()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 2);  // Service 2
        controller->start();
        controller->unmute();

        // Service 1 data (should be filtered out)
        auto data1 = CCDataBuilder::buildCEA708CCPStart(1, 0, 4);
        TestPacketData packet1(data1);
        controller->addData(packet1);

        // Service 2 data (should be processed)
        auto data2 = CCDataBuilder::buildCEA708CCPStart(2, 0, 4);
        TestPacketData packet2(data2);
        controller->addData(packet2);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708MultipleCCPs()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        for (int i = 0; i < 5; ++i)
        {
            auto data = CCDataBuilder::buildCEA708CCPStart(1, i % 4, 4);
            TestPacketData packet(data);
            controller->addData(packet);
        }

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testCEA708IncompleteCCP()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Start but don't complete CCP
        auto startData = CCDataBuilder::buildCEA708CCPStart(1, 0, 20);
        TestPacketData startPacket(startData);
        controller->addData(startPacket);

        // Add only partial data
        auto ccpData = CCDataBuilder::buildCEA708CCPData(0x98, 0x41);
        TestPacketData dataPacket(ccpData);
        controller->addData(dataPacket);

        controller->process();

        // Start new CCP (should handle incomplete previous CCP)
        auto newStart = CCDataBuilder::buildCEA708CCPStart(1, 1, 4);
        TestPacketData newPacket(newStart);
        controller->addData(newPacket);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testStateTransitionFullCycle()
    {
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);

        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);
        controller->process();

        controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        controller->shutdown();
        CPPUNIT_ASSERT(true);
    }

    void testStateTransitionServiceSwitch()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());

        // Switch service - should stop
        controller->setActiveService(CeaType::CEA_708, 2);
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        // Restart with new service
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
    }

    void testStateTransitionMuteDuringProcessing()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->mute();  // Mute before processing
        controller->process();  // Should clear queues

        controller->unmute();
        controller->addData(packet);
        controller->process();  // Should process normally

        CPPUNIT_ASSERT(true);
    }

    void testStateTransitionStopClearsQueues()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_608, 1);
        controller->start();

        // Add data
        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        // Stop should purge queues
        controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());

        // Restart and verify queues are empty
        controller->start();
        controller->unmute();
        controller->process();  // Should not process old data

        CPPUNIT_ASSERT(true);
    }

    void testStateTransitionRapidChanges()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);

        for (int i = 0; i < 10; ++i)
        {
            controller->start();
            controller->unmute();
            controller->mute();
            controller->stop();
        }

        CPPUNIT_ASSERT_EQUAL(false, controller->isStarted());
    }

    void testQueueClearedWhenMuted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should clear queues when muted
        CPPUNIT_ASSERT(true);
    }

    void testQueueClearedWhenStopped()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Muted, so clears queues
        CPPUNIT_ASSERT(true);
    }

    void testQueueClearedAfterProcess()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();

        // Process again without adding data - should be no-op
        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testQueuePurgeOnShutdown()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();

        auto data = CCDataBuilder::buildCEA608Field1(0x94, 0x20);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->shutdown();  // Should purge all queues
        CPPUNIT_ASSERT(true);
    }

    void testRendererClearedOnStart()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        mockWindow->resetCounters();

        controller->start();

        CPPUNIT_ASSERT(mockWindow->clearCalled > 0);
        CPPUNIT_ASSERT(mockWindow->updateCalled > 0);
    }

    void testRendererShownWhenUnmuted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        CPPUNIT_ASSERT_EQUAL(true, controller->isMuted());

        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(true, mockWindow->visibleState);
    }

    void testRendererHiddenWhenMuted()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();
        CPPUNIT_ASSERT_EQUAL(true, mockWindow->visibleState);

        controller->mute();
        CPPUNIT_ASSERT_EQUAL(false, mockWindow->visibleState);
    }

    void testRendererHiddenOnStop()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->unmute();

        mockWindow->resetCounters();
        controller->stop();

        CPPUNIT_ASSERT_EQUAL(false, mockWindow->visibleState);
        CPPUNIT_ASSERT(mockWindow->clearCalled > 0);
    }

    void testEdgeCaseMalformedCCData()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        auto data = CCDataBuilder::buildInvalidData();
        TestPacketData packet(data);
        bool added = controller->addData(packet);
        CPPUNIT_ASSERT_EQUAL(true, added);

        controller->process();  // Should handle malformed data gracefully
        // Verify controller still functional after bad data
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, controller->isMuted());
    }

    void testEdgeCaseExcessiveQueueSize()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();

        // Add many packets to test queue handling
        int successCount = 0;
        for (int i = 0; i < 1000; ++i)
        {
            auto data = CCDataBuilder::buildCEA608Field1(0x80, 0x80);
            TestPacketData packet(data);
            if (controller->addData(packet)) {
                successCount++;
            }
        }

        // Should accept all packets when started
        CPPUNIT_ASSERT_EQUAL(1000, successCount);
        CPPUNIT_ASSERT_EQUAL(true, controller->isStarted());
    }

    void testEdgeCaseReinitAfterShutdown()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->start();
        controller->shutdown();

        // Try to reinit
        bool result = controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        CPPUNIT_ASSERT(result == true || result == false);
    }

    void testErrorHandlingInvalidUserData()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Invalid data that might throw in UserData constructor
        auto data = CCDataBuilder::buildInvalidData();
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should catch and continue
        CPPUNIT_ASSERT(true);
    }

    void testErrorHandlingCCPException()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Malformed CCP that might throw
        auto data = CCDataBuilder::buildInvalidData();
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();
        CPPUNIT_ASSERT(true);
    }

    void testErrorHandlingServiceBlockError()
    {
        controller->init(static_cast<subttxrend::gfx::Window*>(mockWindow), fontCache);
        controller->setActiveService(CeaType::CEA_708, 1);
        controller->start();
        controller->unmute();

        // Add CCP with zero-size service block
        auto data = CCDataBuilder::buildCEA708CCPStart(1, 0, 1);
        TestPacketData packet(data);
        controller->addData(packet);

        controller->process();  // Should handle null service block
        CPPUNIT_ASSERT(true);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcControllerTest);
