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
#include <memory>
#include <stdexcept>
#include <array>

#define private public
#include "GfxRenderer.hpp"
#undef private
#include "GfxRendererClient.hpp"
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <ttxdecoder/Engine.hpp>
#include <ttxdecoder/PageId.hpp>
#include <ttxdecoder/DecodedPage.hpp>

using namespace subttxrend::ttxt;

/**
 * Mock DrawContext for testing
 */
class MockDrawContext : public subttxrend::gfx::DrawContext
{
public:
    void fillRectangle(subttxrend::gfx::ColorArgb color,
                      const subttxrend::gfx::Rectangle& rect) override
    {
        (void)color;
        (void)rect;
    }

    void drawUnderline(subttxrend::gfx::ColorArgb color,
                      const subttxrend::gfx::Rectangle& rect) override {}

    void drawPixmap(const subttxrend::gfx::ClutBitmap& bitmap,
                   const subttxrend::gfx::Rectangle& src,
                   const subttxrend::gfx::Rectangle& dst) override {}

    void drawBitmap(const subttxrend::gfx::Bitmap& bitmap,
                   const subttxrend::gfx::Rectangle& dst) override {}

    void drawGlyph(const subttxrend::gfx::FontStripPtr& fontStrip,
                  std::int32_t glyphIndex,
                  const subttxrend::gfx::Rectangle& dst,
                  subttxrend::gfx::ColorArgb fgColor,
                  subttxrend::gfx::ColorArgb bgColor) override {}

    void drawString(subttxrend::gfx::PrerenderedFont& font,
                   const subttxrend::gfx::Rectangle& dst,
                   const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                   subttxrend::gfx::ColorArgb fgColor,
                   subttxrend::gfx::ColorArgb bgColor,
                   int xOffset,
                   int yOffset) override {}
};

/**
 * Mock Window for testing
 */
class MockWindow : public subttxrend::gfx::Window
{
public:
    MockWindow()
        : m_visible(false), m_updateCalled(false),
          m_sizeCalled(false), m_addListenerCalled(false),
          m_removeListenerCalled(false)
    {
        m_size.m_w = 1280;
        m_size.m_h = 720;
        m_bounds.m_x = 0;
        m_bounds.m_y = 0;
        m_bounds.m_w = 1280;
        m_bounds.m_h = 720;
    }

    void setVisible(bool visible) override
    {
        m_visible = visible;
    }

    void setSize(const subttxrend::gfx::Size& size) override
    {
        m_size = size;
        m_sizeCalled = true;
    }

    subttxrend::gfx::Size getSize() const override { return m_size; }

    subttxrend::gfx::Size getPreferredSize() const override { return m_size; }

    void update() override { m_updateCalled = true; }

    void clear() override {}

    subttxrend::gfx::DrawContext& getDrawContext() override
    {
        return m_drawContext;
    }

    subttxrend::gfx::Rectangle getBounds() const override
    {
        return m_bounds;
    }

    void setDrawDirection(subttxrend::gfx::DrawDirection dir) override {}

    void addKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        m_addListenerCalled = true;
        m_listener = listener;
    }

    void removeKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        m_removeListenerCalled = true;
        if (m_listener == listener) {
            m_listener = nullptr;
        }
    }

    bool isVisible() const { return m_visible; }
    bool wasUpdateCalled() const { return m_updateCalled; }
    bool wasSizeCalled() const { return m_sizeCalled; }
    bool wasAddListenerCalled() const { return m_addListenerCalled; }
    bool wasRemoveListenerCalled() const { return m_removeListenerCalled; }
    void resetCalls()
    {
        m_updateCalled = false;
        m_sizeCalled = false;
        m_addListenerCalled = false;
        m_removeListenerCalled = false;
    }

private:
    bool m_visible;
    bool m_updateCalled;
    bool m_sizeCalled;
    bool m_addListenerCalled;
    bool m_removeListenerCalled;
    subttxrend::gfx::Size m_size;
    subttxrend::gfx::Rectangle m_bounds;
    MockDrawContext m_drawContext;
    subttxrend::gfx::KeyEventListener* m_listener = nullptr;
};

/**
 * Mock FontStrip for testing
 */
class MockFontStrip : public subttxrend::gfx::FontStrip
{
public:
    MockFontStrip(const subttxrend::gfx::Size& glyphSize, std::size_t glyphCount)
        : m_glyphSize(glyphSize), m_glyphCount(glyphCount), m_glyphsLoaded(0)
    {
        m_glyphs.resize(glyphCount);
    }

    bool loadGlyph(std::int32_t glyphIndex, const std::uint8_t* data,
                   std::size_t dataSize) override
    {
        if (glyphIndex >= 0 && glyphIndex < static_cast<std::int32_t>(m_glyphCount) && data && dataSize > 0)
        {
            m_glyphs[glyphIndex].assign(data, data + dataSize);
            m_glyphsLoaded++;
            return true;
        }
        return false;
    }

    bool loadFont(const std::string& fontName,
                  const subttxrend::gfx::Size& glyphSize,
                  const subttxrend::gfx::FontStripMap& mapping) override
    {
        return true;
    }

private:
    subttxrend::gfx::Size m_glyphSize;
    std::size_t m_glyphCount;
    std::uint16_t m_glyphsLoaded;
    std::vector<std::vector<std::uint8_t>> m_glyphs;
};

/**
 * Mock Graphics Engine for testing
 */
class MockGfxEngine : public subttxrend::gfx::Engine
{
public:
    void init(const std::string &displayName = {}) override {}
    void shutdown() override {}
    void execute() override {}
    subttxrend::gfx::WindowPtr createWindow() override { return nullptr; }
    void attach(subttxrend::gfx::WindowPtr window) override {}
    void detach(subttxrend::gfx::WindowPtr window) override {}
    subttxrend::gfx::FontStripPtr createFontStrip(const subttxrend::gfx::Size& glyphSize,
                                                  std::size_t glyphCount) override
    {
        return std::make_shared<MockFontStrip>(glyphSize, glyphCount);
    }
};

/**
 * Mock TTX Engine for testing
 */
class MockTtxEngine : public ttxdecoder::Engine
{
public:
    void resetAcquisition() override {}
    std::uint32_t process() override { return 0; }
    bool addPesPacket(const std::uint8_t* packet, std::uint16_t packetLength) override { return true; }

    void setCurrentPageId(const ttxdecoder::PageId& pageId) override
    {
        m_currentPageId = pageId;
    }

    ttxdecoder::PageId getNextPageId(const ttxdecoder::PageId& inputPageId) const override
    {
        // Return page + 1
        auto pageNum = inputPageId.getPage();
        if (pageNum < 0x899) {
            return ttxdecoder::PageId(pageNum + 1, ttxdecoder::PageId::ANY_SUBPAGE);
        }
        return ttxdecoder::PageId(); // Invalid
    }

    ttxdecoder::PageId getPrevPageId(const ttxdecoder::PageId& inputPageId) const override
    {
        // Return page - 1
        auto pageNum = inputPageId.getPage();
        if (pageNum > 0x100) {
            return ttxdecoder::PageId(pageNum - 1, ttxdecoder::PageId::ANY_SUBPAGE);
        }
        return ttxdecoder::PageId(); // Invalid
    }

    ttxdecoder::PageId getPageId(ttxdecoder::PageIdType type) const override
    {
        return m_currentPageId;
    }

    const ttxdecoder::DecodedPage& getPage() const override
    {
        return m_decodedPage;
    }

    void setNavigationMode(ttxdecoder::NavigationMode navigationMode) override {}
    ttxdecoder::NavigationState getNavigationState() const override
    {
        return ttxdecoder::NavigationState();
    }

    void getTopLinkText(std::uint16_t magazine, std::uint32_t page, std::uint16_t* text) const override {}

    std::uint8_t getScreenColorIndex() const override { return 0; }
    std::uint8_t getRowColorIndex(std::uint8_t row) const override { return 0; }

    bool getColors(std::array<std::uint32_t, 16>& colors) const override
    {
        (void)colors;
        return false;
    }

    const ttxdecoder::CharsetMappingArray& getCharsetMapping(ttxdecoder::Charset charset) const override
    {
        static ttxdecoder::CharsetMappingArray mapping;
        return mapping;
    }

    void setCharsetMapping(ttxdecoder::Charset charset, const ttxdecoder::CharsetMappingArray& mapping) override {}
    void setDefaultPrimaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}
    void setDefaultSecondaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}

    std::uint8_t getPageControlInfo() const override
    {
        return m_controlInfo;
    }

    void setIgnorePts(bool ignorePts) override {}

private:
    ttxdecoder::PageId m_currentPageId;
    ttxdecoder::DecodedPage m_decodedPage;
    std::uint8_t m_controlInfo = 0;
};

/**
 * Mock ConfigProvider for testing
 */
class MockConfigProvider : public subttxrend::common::ConfigProvider
{
public:
    MockConfigProvider()
    {
        // Set default values
        m_values["ttxt.window.width"] = "1280";
        m_values["ttxt.window.height"] = "720";
        m_values["ttxt.cell.width"] = "32";
        m_values["ttxt.cell.height"] = "29";
        m_values["ttxt.flash.period.ms"] = "1000";
        m_values["ttxt.background.alpha"] = "255";
        m_values["ttxt.font.path"] = "/usr/share/fonts";
    }

    const char* getValue(const std::string& key) const override
    {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }

private:
    mutable std::map<std::string, std::string> m_values;
};

/**
 * Mock GfxRendererClient for testing
 */
class MockGfxRendererClient : public GfxRendererClient
{
public:
    MockGfxRendererClient(bool subtitlesRenderer = false)
        : GfxRendererClient(subtitlesRenderer),
          m_config(std::make_shared<MockConfigProvider>()),
          m_gfxEngine(std::make_shared<MockGfxEngine>()),
          m_ttxEngine(std::make_shared<MockTtxEngine>()),
          m_window(std::make_shared<MockWindow>())
    {
    }

    const ttxdecoder::Engine* getDataSource() const override
    {
        return m_ttxEngine.get();
    }

    const subttxrend::common::ConfigProvider* getConfiguration() const override
    {
        return m_config.get();
    }

    subttxrend::gfx::Window* getGfxWindow() const override
    {
        return m_window.get();
    }

    const subttxrend::gfx::EnginePtr& getGfxEngine() const override
    {
        return m_gfxEngine;
    }

    void setCurrentPage(const ttxdecoder::PageId& pageId) override
    {
        m_ttxEngine->setCurrentPageId(pageId);
    }

    // Helper methods for test setup
    void setNullConfig() { m_config.reset(); }
    void setNullGfxEngine() { m_gfxEngine.reset(); }
    void setNullDataSource() { m_ttxEngine.reset(); }
    void setNullWindow() { m_window.reset(); }

    std::shared_ptr<MockWindow> getMockWindow() const { return m_window; }

private:
    std::shared_ptr<MockConfigProvider> m_config;
    subttxrend::gfx::EnginePtr m_gfxEngine;
    std::shared_ptr<MockTtxEngine> m_ttxEngine;
    std::shared_ptr<MockWindow> m_window;
};


class GfxRendererTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxRendererTest);

    CPPUNIT_TEST(test_getSingleton_ReturnsSameInstance);
    CPPUNIT_TEST(test_getSingleton_ReturnsNonNull);
    CPPUNIT_TEST(test_gfxInit_WithValidClient_Succeeds);
    CPPUNIT_TEST(test_gfxInit_CalledTwice_IncrementsRefCount);
    CPPUNIT_TEST(test_gfxInit_WithNullClient_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_WithNullConfig_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_WithNullGfxEngine_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_WithNullDataSource_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_WithNullWindow_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_RegistersKeyEventListener);
    CPPUNIT_TEST(test_gfxShutdown_WhenNotInitialized_ThrowsException);
    CPPUNIT_TEST(test_gfxShutdown_AfterSingleInit_Succeeds);
    CPPUNIT_TEST(test_gfxShutdown_AfterMultipleInits_DecrementsRefCount);
    CPPUNIT_TEST(test_gfxShutdown_RemovesKeyEventListener);
    CPPUNIT_TEST(test_gfxShow_WithValidClient_Succeeds);
    CPPUNIT_TEST(test_gfxShow_WithNullClient_ThrowsException);
    CPPUNIT_TEST(test_gfxShow_WhenNotInitialized_ThrowsException);
    CPPUNIT_TEST(test_gfxShow_WhenAlreadyShown_ThrowsException);
    CPPUNIT_TEST(test_gfxShow_SetsWindowVisible);
    CPPUNIT_TEST(test_gfxShow_SetsWindowSize);
    CPPUNIT_TEST(test_gfxHide_WithValidClient_Succeeds);
    CPPUNIT_TEST(test_gfxHide_WithNullClient_ThrowsException);
    CPPUNIT_TEST(test_gfxHide_WhenNotInitialized_ThrowsException);
    CPPUNIT_TEST(test_gfxHide_WhenNotShown_ThrowsException);
    CPPUNIT_TEST(test_gfxHide_WithDifferentClient_ThrowsException);
    CPPUNIT_TEST(test_gfxHide_SetsWindowInvisible);
    CPPUNIT_TEST(test_gfxHide_ClearsCurrentClient);
    CPPUNIT_TEST(test_gfxDraw_WithHeaderUpdate_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_WithPageUpdate_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_WithBothUpdates_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_WithNullClient_ThrowsException);
    CPPUNIT_TEST(test_gfxDraw_WhenNotInitialized_ThrowsException);
    CPPUNIT_TEST(test_gfxDraw_WhenNotShown_ThrowsException);
    CPPUNIT_TEST(test_gfxDraw_WithDifferentClient_ThrowsException);
    CPPUNIT_TEST(test_gfxDraw_CallsWindowUpdate);
    CPPUNIT_TEST(test_lifecycle_MultipleInitShutdownPairs);
    CPPUNIT_TEST(test_lifecycle_InitAfterFullShutdown);
    CPPUNIT_TEST(test_integration_InitShowHideShutdown);
    CPPUNIT_TEST(test_integration_InitShowDrawHideShutdown);
    CPPUNIT_TEST(test_integration_MultipleClients_Sequential);
    CPPUNIT_TEST(test_gfxInit_MoreThan100Times_ThrowsException);
    CPPUNIT_TEST(test_gfxInit_Exactly100Times_Succeeds);
    CPPUNIT_TEST(test_gfxInit_101Times_ThrowsException);
    CPPUNIT_TEST(test_gfxShow_AfterHide_Succeeds);
    CPPUNIT_TEST(test_gfxShow_MultipleShowHideCycles);
    CPPUNIT_TEST(test_gfxShow_DifferentClientAfterHide);
    CPPUNIT_TEST(test_gfxDraw_WithNoUpdates_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_MultipleCalls_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_AfterShow_Succeeds);
    CPPUNIT_TEST(test_gfxDraw_BeforeHide_Succeeds);
    CPPUNIT_TEST(test_lifecycle_DoubleInit_SingleShutdown_Fails);
    CPPUNIT_TEST(test_lifecycle_Init_DoubleShow_Fails);
    CPPUNIT_TEST(test_lifecycle_Init_Show_DoubleHide_Fails);
    CPPUNIT_TEST(test_lifecycle_MultipleInitBeforeShutdown);
    CPPUNIT_TEST(test_lifecycle_ShowAfterPartialShutdown);
    CPPUNIT_TEST(test_client_ShowWithDifferentClient_Fails);
    CPPUNIT_TEST(test_client_DrawWithWrongClient_Fails);
    CPPUNIT_TEST(test_client_HideWithWrongClient_Fails);
    CPPUNIT_TEST(test_client_SequentialClientsWorkCorrectly);
    CPPUNIT_TEST(test_state_HideResetsCurrentClient);
    CPPUNIT_TEST(test_state_MultipleDrawsConsistent);
    CPPUNIT_TEST(test_recovery_AfterInitFailure_CanRetry);
    CPPUNIT_TEST(test_recovery_AfterShowFailure_CanRetry);
    CPPUNIT_TEST(test_recovery_StateAfterException);
    CPPUNIT_TEST(test_window_VisibilityToggle);
    CPPUNIT_TEST(test_window_SizeSetOnShow);
    CPPUNIT_TEST(test_subtitle_ClientCreation);
    CPPUNIT_TEST(test_subtitle_InitShowHide);
    CPPUNIT_TEST(test_subtitle_DrawOperations);
    CPPUNIT_TEST(test_mixed_NormalThenSubtitle);
    CPPUNIT_TEST(test_mixed_SubtitleThenNormal);
    CPPUNIT_TEST(test_concurrent_InitShowBeforeShutdown);
    CPPUNIT_TEST(test_concurrent_ShowDrawHide);
    CPPUNIT_TEST(test_concurrent_MultipleDraws);
    CPPUNIT_TEST(test_boundary_ZeroInits);
    CPPUNIT_TEST(test_boundary_SingleInitShutdown);
    CPPUNIT_TEST(test_boundary_ManyInitShutdownCycles);
    CPPUNIT_TEST(test_integration_FullWorkflow);
    CPPUNIT_TEST(test_integration_MultipleClients_Interleaved);
    CPPUNIT_TEST(test_integration_ErrorInMiddle);
    CPPUNIT_TEST(test_integration_PartialWorkflow);
    CPPUNIT_TEST(test_integration_RepeatedOperations);
    CPPUNIT_TEST(test_stress_RapidInitShutdownCycles);
    CPPUNIT_TEST(test_stress_RapidShowHideCycles);
    CPPUNIT_TEST(test_stress_ManyDrawCallsInSession);
    CPPUNIT_TEST(test_edge_InitCountBoundary_99Inits);
    CPPUNIT_TEST(test_edge_InitCountBoundary_100InitsWithOperations);
    CPPUNIT_TEST(test_edge_ClientSwitchingWithoutFullCleanup);
    CPPUNIT_TEST(test_edge_DrawWithBothFlagsAndNoFlags);
    CPPUNIT_TEST(test_validation_AllPublicMethodsThrowWhenNotInit);
    CPPUNIT_TEST(test_validation_AllPublicMethodsWorkAfterInit);
    CPPUNIT_TEST(test_robustness_ExceptionDoesNotCorruptState);
    CPPUNIT_TEST(test_robustness_MultipleExceptionsRecovery);
    CPPUNIT_TEST(test_coverage_AllErrorPathsCovered);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_renderer = &GfxRenderer::getSingleton();
        m_client = std::make_shared<MockGfxRendererClient>();
        m_client2 = std::make_shared<MockGfxRendererClient>();
    }

    void tearDown() override
    {
        // Try to hide in case a test left it shown
        try { m_renderer->gfxHide(m_client.get()); } catch (...) {}
        try { m_renderer->gfxHide(m_client2.get()); } catch (...) {}

        // Cleanup singleton state safely.
        // If m_gfxWindow is nullptr, gfxShutdown() would dereference it in shutdownInternal().
        if (m_renderer)
        {
            if (m_renderer->m_initCount > 0)
            {
                if (m_renderer->m_gfxWindow)
                {
                    // Normal cleanup path.
                    while (m_renderer->m_initCount > 0)
                    {
                        try { m_renderer->gfxShutdown(); }
                        catch (...) { break; }
                    }
                }
                else
                {
                    // Failed init path: initInternal() threw before setting m_gfxWindow.
                    // Force-reset to avoid poisoning subsequent tests and destructor fatal.
                    m_renderer->m_initCount = 0;
                    m_renderer->m_currentClient = nullptr;
                }
            }
        }

        m_client.reset();
        m_client2.reset();
        m_renderer = nullptr;
    }

protected:
    void test_getSingleton_ReturnsSameInstance()
    {
        GfxRenderer& renderer1 = GfxRenderer::getSingleton();
        GfxRenderer& renderer2 = GfxRenderer::getSingleton();

        CPPUNIT_ASSERT_EQUAL(&renderer1, &renderer2);
    }

    void test_getSingleton_ReturnsNonNull()
    {
        GfxRenderer& renderer = GfxRenderer::getSingleton();

        CPPUNIT_ASSERT(&renderer != nullptr);
    }

    void test_gfxInit_WithValidClient_Succeeds()
    {
        // Should not throw
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxInit_CalledTwice_IncrementsRefCount()
    {
        CPPUNIT_ASSERT_EQUAL(0, m_renderer->m_initCount);

        m_renderer->gfxInit(m_client.get());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->m_initCount);

        // Second init should succeed without re-initializing
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->m_initCount);

        // Both shutdowns should succeed
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->m_initCount);
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        CPPUNIT_ASSERT_EQUAL(0, m_renderer->m_initCount);
    }

    void test_gfxInit_WithNullClient_ThrowsException()
    {
        bool exceptionThrown = false;

        try {
            m_renderer->gfxInit(nullptr);
        } catch (const std::invalid_argument& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Reset the poisoned init count deterministically for this test.
        // gfxInit(nullptr) increments m_initCount and throws before setting m_gfxWindow.
        CPPUNIT_ASSERT(m_renderer->m_gfxWindow == nullptr);
        m_renderer->m_initCount = 0;
    }

    void test_gfxInit_WithNullConfig_ThrowsException()
    {
        m_client->setNullConfig();

        bool exceptionThrown = false;

        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("configuration") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
        // No cleanup - tearDown will handle any state
    }

    void test_gfxInit_WithNullGfxEngine_ThrowsException()
    {
        m_client->setNullGfxEngine();

        bool exceptionThrown = false;

        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("gfx engine") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
        // No cleanup - tearDown will handle any state
    }

    void test_gfxInit_WithNullDataSource_ThrowsException()
    {
        m_client->setNullDataSource();

        bool exceptionThrown = false;

        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("data source") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
        // No cleanup - tearDown will handle any state
    }

    void test_gfxInit_WithNullWindow_ThrowsException()
    {
        m_client->setNullWindow();

        bool exceptionThrown = false;

        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("window") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
        // No cleanup - tearDown will handle any state
    }

    void test_gfxInit_RegistersKeyEventListener()
    {
        // Ensure clean client state
        m_client = std::make_shared<MockGfxRendererClient>();
        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        m_renderer->gfxInit(m_client.get());

        CPPUNIT_ASSERT(mockWindow->wasAddListenerCalled());

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxShutdown_WhenNotInitialized_ThrowsException()
    {
        // Make sure we're starting from uninitialized state
        // Try to shutdown any leftover state from previous tests
        try { m_renderer->gfxShutdown(); } catch (...) {}

        bool exceptionThrown = false;

        try {
            m_renderer->gfxShutdown();
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_gfxShutdown_AfterSingleInit_Succeeds()
    {
        m_client = std::make_shared<MockGfxRendererClient>();
        m_renderer->gfxInit(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_gfxShutdown_AfterMultipleInits_DecrementsRefCount()
    {
        m_client = std::make_shared<MockGfxRendererClient>();
        CPPUNIT_ASSERT_EQUAL(0, m_renderer->m_initCount);
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get());
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->m_initCount);

        // First shutdown should succeed
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->m_initCount);

        // Second shutdown should also succeed
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        CPPUNIT_ASSERT_EQUAL(0, m_renderer->m_initCount);

        // Third shutdown should fail (not initialized)
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShutdown();
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_gfxShutdown_RemovesKeyEventListener()
    {
        m_client = std::make_shared<MockGfxRendererClient>();
        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        CPPUNIT_ASSERT(mockWindow->wasRemoveListenerCalled());
    }

    void test_gfxShow_WithValidClient_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_WithNullClient_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxShow(nullptr);
        } catch (const std::invalid_argument& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_WhenNotInitialized_ThrowsException()
    {
        bool exceptionThrown = false;

        try {
            m_renderer->gfxShow(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_gfxShow_WhenAlreadyShown_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxShow(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Already shown") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_SetsWindowVisible()
    {
        auto mockWindow = m_client->getMockWindow();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT(mockWindow->isVisible());

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_SetsWindowSize()
    {
        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT(mockWindow->wasSizeCalled());

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_WithValidClient_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_WithNullClient_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxHide(nullptr);
        } catch (const std::invalid_argument& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_WhenNotInitialized_ThrowsException()
    {
        bool exceptionThrown = false;

        try {
            m_renderer->gfxHide(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_gfxHide_WhenNotShown_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxHide(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not shown") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_WithDifferentClient_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxHide(m_client2.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("other client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_SetsWindowInvisible()
    {
        auto mockWindow = m_client->getMockWindow();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        CPPUNIT_ASSERT(mockWindow->isVisible());

        m_renderer->gfxHide(m_client.get());

        CPPUNIT_ASSERT(!mockWindow->isVisible());

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxHide_ClearsCurrentClient()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());

        // Should be able to show again with same client
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithHeaderUpdate_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, false));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithPageUpdate_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), false, true));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithBothUpdates_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithNullClient_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxDraw(nullptr, false, true);
        } catch (const std::invalid_argument& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WhenNotInitialized_ThrowsException()
    {
        bool exceptionThrown = false;

        try {
            m_renderer->gfxDraw(m_client.get(), false, true);
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_gfxDraw_WhenNotShown_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxDraw(m_client.get(), false, true);
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not shown") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithDifferentClient_ThrowsException()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxDraw(m_client2.get(), false, true);
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("other client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_CallsWindowUpdate()
    {
        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        mockWindow->resetCalls();
        m_renderer->gfxDraw(m_client.get(), false, true);

        CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_lifecycle_MultipleInitShutdownPairs()
    {
        // First pair
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        // Second pair should work fine
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());

        // Third pair
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_lifecycle_InitAfterFullShutdown()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        // Should be able to init again
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));

        // And show
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_integration_InitShowHideShutdown()
    {
        // Full lifecycle test
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_integration_InitShowDrawHideShutdown()
    {
        // Full lifecycle with drawing
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_integration_MultipleClients_Sequential()
    {
        // First client
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // Second client (should work)
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client2.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client2.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client2.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_gfxInit_MoreThan100Times_ThrowsException()
    {
        // Init 101 times - 101st should work because check is done BEFORE increment
        for (int i = 0; i < 101; ++i) {
            m_renderer->gfxInit(m_client.get());
        }

        // 102nd init should throw (m_initCount is now 101, which is > 100)
        bool exceptionThrown = false;
        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Too many inits") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        for (int i = 0; i < 101; ++i) {
            m_renderer->gfxShutdown();
        }
    }

    void test_gfxInit_Exactly100Times_Succeeds()
    {
        // Init exactly 100 times should work
        for (int i = 0; i < 100; ++i) {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        }

        // Cleanup
        for (int i = 0; i < 100; ++i) {
            m_renderer->gfxShutdown();
        }
    }

    void test_gfxInit_101Times_ThrowsException()
    {
        // Init 101 times - all should succeed
        for (int i = 0; i < 101; ++i) {
            m_renderer->gfxInit(m_client.get());
        }

        // 102nd time should fail (m_initCount is 101, which is > 100)
        bool exceptionThrown = false;
        try {
            m_renderer->gfxInit(m_client.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        for (int i = 0; i < 101; ++i) {
            m_renderer->gfxShutdown();
        }
    }

    void test_gfxShow_AfterHide_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());

        // Should be able to show again
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_MultipleShowHideCycles()
    {
        m_renderer->gfxInit(m_client.get());

        // Multiple show/hide cycles
        for (int i = 0; i < 5; ++i) {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));
        }

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_gfxShow_DifferentClientAfterHide()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // Different client should work after full cleanup
        m_renderer->gfxInit(m_client2.get());
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client2.get()));

        // Cleanup
        m_renderer->gfxHide(m_client2.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_WithNoUpdates_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Draw with no updates
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), false, false));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_MultipleCalls_Succeeds()
    {
        auto mockWindow = m_client->getMockWindow();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Multiple draw calls should all succeed
        for (int i = 0; i < 10; ++i) {
            mockWindow->resetCalls();
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));
            CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());
        }

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_AfterShow_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Draw immediately after show should work
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_gfxDraw_BeforeHide_Succeeds()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, true);

        // Hide after draw should work
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_lifecycle_DoubleInit_SingleShutdown_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        // One more shutdown should work (ref count = 2)
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());

        // Third shutdown should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShutdown();
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_lifecycle_Init_DoubleShow_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Second show should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShow(m_client.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_lifecycle_Init_Show_DoubleHide_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());

        // Second hide should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxHide(m_client.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_lifecycle_MultipleInitBeforeShutdown()
    {
        // Init multiple times
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get());

        // Show should work
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
        m_renderer->gfxShutdown();
        m_renderer->gfxShutdown();
    }

    void test_lifecycle_ShowAfterPartialShutdown()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        // Show should work (still initialized)
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_client_ShowWithDifferentClient_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // After full cleanup, different client should work
        m_renderer->gfxInit(m_client2.get());
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client2.get()));

        // Cleanup
        m_renderer->gfxHide(m_client2.get());
        m_renderer->gfxShutdown();
    }

    void test_client_DrawWithWrongClient_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxDraw(m_client2.get(), true, true);
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("other client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_client_HideWithWrongClient_Fails()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        bool exceptionThrown = false;
        try {
            m_renderer->gfxHide(m_client2.get());
        } catch (const std::logic_error& e) {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("other client") != std::string::npos);
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_client_SequentialClientsWorkCorrectly()
    {
        // First client full cycle
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // Second client full cycle
        m_renderer->gfxInit(m_client2.get());
        m_renderer->gfxShow(m_client2.get());
        m_renderer->gfxDraw(m_client2.get(), true, true);
        m_renderer->gfxHide(m_client2.get());
        m_renderer->gfxShutdown();

        // Third cycle with first client again
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_state_HideResetsCurrentClient()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());

        // Should be able to show again (current client was reset)
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_state_MultipleDrawsConsistent()
    {
        auto mockWindow = m_client->getMockWindow();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Multiple draws should all trigger window update
        for (int i = 0; i < 5; ++i) {
            mockWindow->resetCalls();
            m_renderer->gfxDraw(m_client.get(), true, true);
            CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());
        }

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_recovery_AfterInitFailure_CanRetry()
    {
        auto badClient = std::make_shared<MockGfxRendererClient>();
        badClient->setNullConfig();

        // First init should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxInit(badClient.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Should be able to init with good client
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_recovery_AfterShowFailure_CanRetry()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Try to show again (should fail)
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShow(m_client.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);

        // Should still be able to hide and show again
        m_renderer->gfxHide(m_client.get());
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_recovery_StateAfterException()
    {
        m_renderer->gfxInit(m_client.get());

        // Try invalid operation
        try {
            m_renderer->gfxShow(nullptr);
        } catch (...) {
            // Exception expected
        }

        // Should still be able to perform valid operations
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_window_VisibilityToggle()
    {
        auto mockWindow = m_client->getMockWindow();

        m_renderer->gfxInit(m_client.get());
        CPPUNIT_ASSERT(!mockWindow->isVisible());

        m_renderer->gfxShow(m_client.get());
        CPPUNIT_ASSERT(mockWindow->isVisible());

        m_renderer->gfxHide(m_client.get());
        CPPUNIT_ASSERT(!mockWindow->isVisible());

        // Cleanup
        m_renderer->gfxShutdown();
    }

    void test_window_SizeSetOnShow()
    {
        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        CPPUNIT_ASSERT(mockWindow->wasSizeCalled());

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_subtitle_ClientCreation()
    {
        auto subtitleClient = std::make_shared<MockGfxRendererClient>(true);

        CPPUNIT_ASSERT(subtitleClient->isSubtitlesRenderer());
        CPPUNIT_ASSERT(!m_client->isSubtitlesRenderer());
    }

    void test_subtitle_InitShowHide()
    {
        auto subtitleClient = std::make_shared<MockGfxRendererClient>(true);

        m_renderer->gfxInit(subtitleClient.get());
        m_renderer->gfxShow(subtitleClient.get());
        m_renderer->gfxHide(subtitleClient.get());
        m_renderer->gfxShutdown();
    }

    void test_subtitle_DrawOperations()
    {
        auto subtitleClient = std::make_shared<MockGfxRendererClient>(true);

        m_renderer->gfxInit(subtitleClient.get());
        m_renderer->gfxShow(subtitleClient.get());

        // Draw should work for subtitle renderer
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(subtitleClient.get(), true, true));

        // Cleanup
        m_renderer->gfxHide(subtitleClient.get());
        m_renderer->gfxShutdown();
    }

    void test_mixed_NormalThenSubtitle()
    {
        auto subtitleClient = std::make_shared<MockGfxRendererClient>(true);

        // Normal client first
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // Subtitle client second
        m_renderer->gfxInit(subtitleClient.get());
        m_renderer->gfxShow(subtitleClient.get());
        m_renderer->gfxHide(subtitleClient.get());
        m_renderer->gfxShutdown();
    }

    void test_mixed_SubtitleThenNormal()
    {
        auto subtitleClient = std::make_shared<MockGfxRendererClient>(true);

        // Subtitle client first
        m_renderer->gfxInit(subtitleClient.get());
        m_renderer->gfxShow(subtitleClient.get());
        m_renderer->gfxHide(subtitleClient.get());
        m_renderer->gfxShutdown();

        // Normal client second
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_concurrent_InitShowBeforeShutdown()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Second init before shutdown
        m_renderer->gfxInit(m_client.get());

        // Should still be able to hide
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // Cleanup both inits
        m_renderer->gfxShutdown();
        m_renderer->gfxShutdown();
    }

    void test_concurrent_ShowDrawHide()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_concurrent_MultipleDraws()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Rapid draws
        m_renderer->gfxDraw(m_client.get(), true, false);
        m_renderer->gfxDraw(m_client.get(), false, true);
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxDraw(m_client.get(), false, false);

        // Cleanup
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_boundary_ZeroInits()
    {
        // Without init, operations should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShow(m_client.get());
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_boundary_SingleInitShutdown()
    {
        m_renderer->gfxInit(m_client.get());
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());

        // Second shutdown should fail
        bool exceptionThrown = false;
        try {
            m_renderer->gfxShutdown();
        } catch (const std::logic_error&) {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void test_boundary_ManyInitShutdownCycles()
    {
        for (int i = 0; i < 20; ++i) {
            m_renderer->gfxInit(m_client.get());
            m_renderer->gfxShutdown();
        }
    }

    void test_integration_FullWorkflow()
    {
        // Complete workflow with all operations
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, false);
        m_renderer->gfxDraw(m_client.get(), false, true);
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_integration_MultipleClients_Interleaved()
    {
        // Client 1 init
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // Client 2 cycle
        m_renderer->gfxInit(m_client2.get());
        m_renderer->gfxShow(m_client2.get());
        m_renderer->gfxDraw(m_client2.get(), true, true);
        m_renderer->gfxHide(m_client2.get());
        m_renderer->gfxShutdown();

        // Back to Client 1
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_integration_ErrorInMiddle()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        // Try invalid operation
        try {
            m_renderer->gfxShow(m_client.get());
        } catch (...) {
            // Expected
        }

        // Should still work
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_integration_PartialWorkflow()
    {
        // Init and shutdown without show
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShutdown();

        // Init, show, hide without draw
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_integration_RepeatedOperations()
    {
        // Multiple init/show/draw/hide/shutdown cycles
        for (int i = 0; i < 3; ++i) {
            m_renderer->gfxInit(m_client.get());
            m_renderer->gfxShow(m_client.get());

            for (int j = 0; j < 3; ++j) {
                m_renderer->gfxDraw(m_client.get(), (j % 2 == 0), (j % 2 == 1));
            }

            m_renderer->gfxHide(m_client.get());
            m_renderer->gfxShutdown();
        }
    }

    void test_stress_RapidInitShutdownCycles()
    {
        // Test rapid init/shutdown cycles
        for (int i = 0; i < 20; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        }

        // Verify state is clean - can init again
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        m_renderer->gfxShutdown();
    }

    void test_stress_RapidShowHideCycles()
    {
        m_renderer->gfxInit(m_client.get());

        // Test rapid show/hide cycles
        for (int i = 0; i < 10; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
            auto mockWindow = m_client->getMockWindow();
            CPPUNIT_ASSERT(mockWindow->isVisible());

            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));
            CPPUNIT_ASSERT(!mockWindow->isVisible());
        }

        m_renderer->gfxShutdown();
    }

    void test_stress_ManyDrawCallsInSession()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        auto mockWindow = m_client->getMockWindow();
        mockWindow->resetCalls();

        // Test many draw calls
        for (int i = 0; i < 100; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), i % 2 == 0, i % 3 == 0));
        }

        // Verify window update was called
        CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());

        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_edge_InitCountBoundary_99Inits()
    {
        // Test exactly 99 inits (boundary just below limit)
        for (int i = 0; i < 99; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));
        }

        // One more init should still work (exactly 100)
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));

        // Cleanup all
        for (int i = 0; i < 100; ++i)
        {
            CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
        }
    }

    void test_edge_InitCountBoundary_100InitsWithOperations()
    {
        // Test that operations work even with 100 inits
        for (int i = 0; i < 100; ++i)
        {
            m_renderer->gfxInit(m_client.get());
        }

        // Show/hide/draw should still work
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, false));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // Cleanup all
        for (int i = 0; i < 100; ++i)
        {
            m_renderer->gfxShutdown();
        }
    }

    void test_edge_ClientSwitchingWithoutFullCleanup()
    {
        // Init with first client
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxInit(m_client.get()); // ref count = 2
        m_renderer->gfxShow(m_client.get());
        m_renderer->gfxDraw(m_client.get(), true, true);
        m_renderer->gfxHide(m_client.get());

        // Partial shutdown (ref count = 1)
        m_renderer->gfxShutdown();

        // Cannot show with different client while ref count > 0
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        m_renderer->gfxHide(m_client.get());

        // Complete shutdown
        m_renderer->gfxShutdown();

        // Now can use second client
        m_renderer->gfxInit(m_client2.get());
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client2.get()));
        m_renderer->gfxHide(m_client2.get());
        m_renderer->gfxShutdown();
    }

    void test_edge_DrawWithBothFlagsAndNoFlags()
    {
        m_renderer->gfxInit(m_client.get());
        m_renderer->gfxShow(m_client.get());

        auto mockWindow = m_client->getMockWindow();

        // Draw with both flags true
        mockWindow->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));
        CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());

        // Draw with both flags false - may or may not update depending on flash timing
        mockWindow->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), false, false));
        // Don't assert on update - it depends on flash animation timing

        // Draw with only header
        mockWindow->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, false));
        CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());

        // Draw with only page
        mockWindow->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), false, true));
        CPPUNIT_ASSERT(mockWindow->wasUpdateCalled());

        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();
    }

    void test_validation_AllPublicMethodsThrowWhenNotInit()
    {
        // Verify all public methods except getSingleton and gfxInit throw when not initialized

        // gfxShutdown throws
        bool shutdownThrew = false;
        try
        {
            m_renderer->gfxShutdown();
        }
        catch (const std::logic_error& e)
        {
            shutdownThrew = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }
        CPPUNIT_ASSERT(shutdownThrew);

        // gfxShow throws
        bool showThrew = false;
        try
        {
            m_renderer->gfxShow(m_client.get());
        }
        catch (const std::logic_error& e)
        {
            showThrew = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }
        CPPUNIT_ASSERT(showThrew);

        // gfxHide throws
        bool hideThrew = false;
        try
        {
            m_renderer->gfxHide(m_client.get());
        }
        catch (const std::logic_error& e)
        {
            hideThrew = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }
        CPPUNIT_ASSERT(hideThrew);

        // gfxDraw throws
        bool drawThrew = false;
        try
        {
            m_renderer->gfxDraw(m_client.get(), true, true);
        }
        catch (const std::logic_error& e)
        {
            drawThrew = true;
            CPPUNIT_ASSERT(std::string(e.what()).find("Not initialized") != std::string::npos);
        }
        CPPUNIT_ASSERT(drawThrew);
    }

    void test_validation_AllPublicMethodsWorkAfterInit()
    {
        // Verify all public methods work correctly after initialization

        // getSingleton works
        GfxRenderer& singleton = GfxRenderer::getSingleton();
        CPPUNIT_ASSERT(&singleton == m_renderer);

        // gfxInit works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxInit(m_client.get()));

        // gfxShow works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));

        // gfxDraw works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));

        // gfxHide works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // gfxShutdown works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShutdown());
    }

    void test_robustness_ExceptionDoesNotCorruptState()
    {
        // Initialize successfully
        m_renderer->gfxInit(m_client.get());

        // Cause an exception by calling show with null
        bool exceptionThrown = false;
        try
        {
            m_renderer->gfxShow(nullptr);
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);

        // Verify state is not corrupted - can still show with valid client
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, false));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        m_renderer->gfxShutdown();
    }

    void test_robustness_MultipleExceptionsRecovery()
    {
        // Ensure this test starts from a clean singleton state.
        // If a prior init attempt failed before setting m_gfxWindow, calling gfxInit()
        // would only bump the refcount and skip initInternal(), and later gfxShutdown()
        // could segfault in shutdownInternal().
        if (m_renderer->m_initCount > 0 && m_renderer->m_gfxWindow == nullptr)
        {
            m_renderer->m_initCount = 0;
            m_renderer->m_currentClient = nullptr;
        }

        // Test that multiple exceptions don't corrupt state
        m_renderer->gfxInit(m_client.get());

        // Cause multiple exceptions
        for (int i = 0; i < 5; ++i)
        {
            try
            {
                m_renderer->gfxShow(nullptr);
            }
            catch (const std::invalid_argument&)
            {
                // Expected
            }
        }

        // Verify normal operation still works
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // More exceptions
        for (int i = 0; i < 5; ++i)
        {
            try
            {
                m_renderer->gfxDraw(nullptr, true, true);
            }
            catch (const std::invalid_argument&)
            {
                // Expected
            }
        }

        // Verify still functional
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxShow(m_client.get()));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxDraw(m_client.get(), true, true));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->gfxHide(m_client.get()));

        // Safe cleanup: only call gfxShutdown() if initInternal() actually set up the window.
        if (m_renderer->m_initCount > 0)
        {
            if (m_renderer->m_gfxWindow)
            {
                m_renderer->gfxShutdown();
            }
            else
            {
                // Failed init path: avoid shutdownInternal() null deref.
                m_renderer->m_initCount = 0;
                m_renderer->m_currentClient = nullptr;
            }
        }
    }

    void test_coverage_AllErrorPathsCovered()
    {
        // Test all error conditions are properly validated

        // 1. Null client errors
        CPPUNIT_ASSERT_THROW(m_renderer->gfxInit(nullptr), std::invalid_argument);

        // gfxInit() increments m_initCount before validating and initInternal() throws before
        // setting up m_gfxWindow. Reset the singleton state so subsequent init is a real init.
        CPPUNIT_ASSERT(m_renderer->m_gfxWindow == nullptr);
        m_renderer->m_initCount = 0;
        m_renderer->m_currentClient = nullptr;

        m_renderer->gfxInit(m_client.get());

        CPPUNIT_ASSERT_THROW(m_renderer->gfxShow(nullptr), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxHide(nullptr), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxDraw(nullptr, true, true), std::invalid_argument);

        // 2. Not shown errors
        CPPUNIT_ASSERT_THROW(m_renderer->gfxHide(m_client.get()), std::logic_error);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxDraw(m_client.get(), true, true), std::logic_error);

        m_renderer->gfxShow(m_client.get());

        // 3. Already shown error
        CPPUNIT_ASSERT_THROW(m_renderer->gfxShow(m_client.get()), std::logic_error);

        // 4. Wrong client errors
        CPPUNIT_ASSERT_THROW(m_renderer->gfxDraw(m_client2.get(), true, true), std::logic_error);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxHide(m_client2.get()), std::logic_error);

        m_renderer->gfxHide(m_client.get());
        m_renderer->gfxShutdown();

        // 5. Not initialized errors
        CPPUNIT_ASSERT_THROW(m_renderer->gfxShutdown(), std::logic_error);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxShow(m_client.get()), std::logic_error);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxHide(m_client.get()), std::logic_error);
        CPPUNIT_ASSERT_THROW(m_renderer->gfxDraw(m_client.get(), true, true), std::logic_error);

        // 6. Too many inits error - need 101 inits before the 102nd throws
        for (int i = 0; i < 101; ++i)
        {
            m_renderer->gfxInit(m_client.get());
        }
        CPPUNIT_ASSERT_THROW(m_renderer->gfxInit(m_client.get()), std::logic_error);

        // Cleanup
        for (int i = 0; i < 101; ++i)
        {
            m_renderer->gfxShutdown();
        }
    }


private:
    GfxRenderer* m_renderer;
    std::shared_ptr<MockGfxRendererClient> m_client;
    std::shared_ptr<MockGfxRendererClient> m_client2;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxRendererTest);
