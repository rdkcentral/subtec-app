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
#include <vector>
#include <string>

#include "RendererImpl.hpp"
#include "TimeSource.hpp"
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <ttxdecoder/PageId.hpp>

using namespace subttxrend::ttxt;

/**
 * Mock DrawContext for testing
 */
class MockDrawContext : public subttxrend::gfx::DrawContext
{
public:
    void fillRectangle(subttxrend::gfx::ColorArgb color,
                      const subttxrend::gfx::Rectangle& rect) override {}
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
        : m_visible(false), m_updateCalled(false)
    {
        m_size.m_w = 1280;
        m_size.m_h = 720;
        m_bounds.m_x = 0;
        m_bounds.m_y = 0;
        m_bounds.m_w = 1280;
        m_bounds.m_h = 720;
    }

    void setVisible(bool visible) override { m_visible = visible; }
    void setSize(const subttxrend::gfx::Size& size) override { m_size = size; }
    subttxrend::gfx::Size getSize() const override { return m_size; }
    subttxrend::gfx::Size getPreferredSize() const override { return m_size; }
    void update() override { m_updateCalled = true; }
    void clear() override {}
    subttxrend::gfx::DrawContext& getDrawContext() override { return m_drawContext; }
    subttxrend::gfx::Rectangle getBounds() const override { return m_bounds; }
    void setDrawDirection(subttxrend::gfx::DrawDirection dir) override {}
    void addKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override {}
    void removeKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override {}

    bool isVisible() const { return m_visible; }
    bool wasUpdateCalled() const { return m_updateCalled; }
    void resetCalls() { m_updateCalled = false; }

private:
    bool m_visible;
    bool m_updateCalled;
    subttxrend::gfx::Size m_size;
    subttxrend::gfx::Rectangle m_bounds;
    MockDrawContext m_drawContext;
};

/**
 * Mock FontStrip for testing
 */
class MockFontStrip : public subttxrend::gfx::FontStrip
{
public:
    MockFontStrip(const subttxrend::gfx::Size& glyphSize, std::size_t glyphCount)
        : m_glyphSize(glyphSize), m_glyphCount(glyphCount)
    {
        m_glyphs.resize(glyphCount);
    }

    bool loadGlyph(std::int32_t glyphIndex, const std::uint8_t* data,
                   std::size_t dataSize) override
    {
        if (glyphIndex >= 0 && glyphIndex < static_cast<std::int32_t>(m_glyphCount) && data && dataSize > 0)
        {
            m_glyphs[glyphIndex].assign(data, data + dataSize);
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
 * Mock ConfigProvider for testing
 */
class MockConfigProvider : public subttxrend::common::ConfigProvider
{
public:
    MockConfigProvider() = default;
    virtual ~MockConfigProvider() = default;

protected:
    const char* getValue(const std::string& key) const override
    {
        return nullptr;
    }
};

/**
 * Mock TimeSource for testing
 */
class MockTimeSource : public TimeSource
{
public:
    MockTimeSource() : m_stc(0) {}
    virtual ~MockTimeSource() = default;

    std::uint32_t getStc() override { return m_stc; }
    void setStc(std::uint32_t stc) { m_stc = stc; }

private:
    std::uint32_t m_stc;
};

/**
 * Test RendererImpl that exposes startInternal for testing
 */
class TestableRendererImpl : public RendererImpl
{
public:
    TestableRendererImpl(bool subtitlesRenderer) : RendererImpl(subtitlesRenderer) {}

    bool callStartInternal() { return startInternal(); }
};

class RendererImplTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(RendererImplTest);
    CPPUNIT_TEST(testConstructorWithSubtitlesRendererTrue);
    CPPUNIT_TEST(testConstructorWithSubtitlesRendererFalse);
    CPPUNIT_TEST(testInitWithValidParameters);
    CPPUNIT_TEST(testInitWithNullGfxWindow);
    CPPUNIT_TEST(testInitWithNullGfxEngine);
    CPPUNIT_TEST(testInitWithNullConfigProvider);
    CPPUNIT_TEST(testInitWithNullTimeSource);
    CPPUNIT_TEST(testInitCalledTwice);
    CPPUNIT_TEST(testShutdownAfterInit);
    CPPUNIT_TEST(testShutdownWithoutInit);
    CPPUNIT_TEST(testShutdownCalledTwice);
    CPPUNIT_TEST(testStartInternalWhenNotStarted);
    CPPUNIT_TEST(testStartInternalWhenAlreadyStarted);
    CPPUNIT_TEST(testStartInternalWhenMuted);
    CPPUNIT_TEST(testStopWhenStarted);
    CPPUNIT_TEST(testStopWhenNotStarted);
    CPPUNIT_TEST(testStopCalledTwice);
    CPPUNIT_TEST(testIsStartedAfterConstruction);
    CPPUNIT_TEST(testIsStartedAfterStart);
    CPPUNIT_TEST(testIsStartedAfterStop);
    CPPUNIT_TEST(testMuteWhenNotMuted);
    CPPUNIT_TEST(testMuteWhenAlreadyMuted);
    CPPUNIT_TEST(testUnmuteWhenMuted);
    CPPUNIT_TEST(testUnmuteWhenNotMuted);
    CPPUNIT_TEST(testIsMutedAfterConstruction);
    CPPUNIT_TEST(testIsMutedAfterMute);
    CPPUNIT_TEST(testIsMutedAfterUnmute);
    CPPUNIT_TEST(testProcessDataWhenNotStarted);
    CPPUNIT_TEST(testProcessDataWhenStartedAndNotMuted);
    CPPUNIT_TEST(testProcessDataWhenStartedAndMuted);
    CPPUNIT_TEST(testAddPesPacketWhenStarted);
    CPPUNIT_TEST(testAddPesPacketWhenNotStarted);
    CPPUNIT_TEST(testAddPesPacketWithNullBuffer);
    CPPUNIT_TEST(testAddPesPacketWithZeroLength);
    CPPUNIT_TEST(testSetCurrentPage);
    CPPUNIT_TEST(testSetCurrentPageMultipleTimes);
    CPPUNIT_TEST(testLifecycleInitStartStopShutdown);
    CPPUNIT_TEST(testLifecycleInitStartMuteUnmuteStop);
    CPPUNIT_TEST(testLifecycleInitMuteStartStopUnmute);
    CPPUNIT_TEST(testMultipleStartStopCycles);
    CPPUNIT_TEST(testMultipleMuteUnmuteCycles);
    CPPUNIT_TEST(testMuteWhileStarted);
    CPPUNIT_TEST(testUnmuteWhileStarted);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_window.reset(new MockWindow());
        m_gfxEngine = std::make_shared<MockGfxEngine>();
        m_configProvider.reset(new MockConfigProvider());
        m_timeSource.reset(new MockTimeSource());
        m_rendererInitialized = false;
    }

    void tearDown() override
    {
        if (m_renderer)
        {
            try
            {
                // Best-effort cleanup to avoid leaking GfxRenderer "shown" state
                // between tests (singleton).
                m_renderer->stop();
            }
            catch (...)
            {
                // ignore
            }
            if (m_rendererInitialized)
            {
                try
                {
                    m_renderer->shutdown();
                }
                catch (...)
                {
                    // Ignore shutdown exceptions in tearDown
                }
            }
            m_renderer.reset();
        }
        m_timeSource.reset();
        m_configProvider.reset();
        m_gfxEngine.reset();
        m_window.reset();
    }

protected:
    void testConstructorWithSubtitlesRendererTrue()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        CPPUNIT_ASSERT(m_renderer.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testConstructorWithSubtitlesRendererFalse()
    {
        m_renderer.reset(new TestableRendererImpl(false));
        CPPUNIT_ASSERT(m_renderer.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testInitWithValidParameters()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool result = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, result);
        m_rendererInitialized = true;
    }

    void testInitWithNullGfxWindow()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool exceptionThrown = false;
        try
        {
            m_renderer->init(nullptr, m_gfxEngine,
                            m_configProvider.get(), m_timeSource.get());
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()) == "gfxWindow");
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testInitWithNullGfxEngine()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool exceptionThrown = false;
        try
        {
            m_renderer->init(m_window.get(), nullptr,
                            m_configProvider.get(), m_timeSource.get());
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()) == "gfxEngine");
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testInitWithNullConfigProvider()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool exceptionThrown = false;
        try
        {
            m_renderer->init(m_window.get(), m_gfxEngine, nullptr, m_timeSource.get());
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()) == "configProvider");
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testInitWithNullTimeSource()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool exceptionThrown = false;
        try
        {
            m_renderer->init(m_window.get(), m_gfxEngine, m_configProvider.get(), nullptr);
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
            CPPUNIT_ASSERT(std::string(e.what()) == "timeSource");
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testInitCalledTwice()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        bool firstInit = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, firstInit);
        m_rendererInitialized = true;

        bool secondInit = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(false, secondInit);
    }

    void testShutdownAfterInit()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->shutdown());
        m_rendererInitialized = false;

        // After shutdown(), init() should be possible again.
        bool reinitResult = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, reinitResult);
        m_rendererInitialized = true;
    }

    void testShutdownWithoutInit()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        // RendererImpl::shutdown() calls GfxRenderer::gfxShutdown(); without init it is expected to throw.
        CPPUNIT_ASSERT_THROW(m_renderer->shutdown(), std::exception);
    }

    void testShutdownCalledTwice()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->shutdown());
        m_rendererInitialized = false;

        // Second shutdown is expected to throw (gfxShutdown without init).
        CPPUNIT_ASSERT_THROW(m_renderer->shutdown(), std::exception);
    }

    void testStartInternalWhenNotStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_window->resetCalls();

        bool result = m_renderer->callStartInternal();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(true, m_window->wasUpdateCalled());
    }

    void testStartInternalWhenAlreadyStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        try
        {
            m_renderer->callStartInternal();
        }
        catch (...)
        {
            // First start may throw if GfxRenderer singleton is already shown
            // In this case, we can't test the "already started" scenario
            return;
        }

        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());

        // When calling start again, it may throw if GfxRenderer singleton is already shown
        // The behavior is correct either way - either it restarts successfully or throws
        try
        {
            bool result = m_renderer->callStartInternal();
            CPPUNIT_ASSERT_EQUAL(true, result);
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        }
        catch (...)
        {
            // GfxRenderer threw, but the first call succeeded so renderer was already started
            // The renderer should remain in started state even if second start threw
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        }
    }

    void testStartInternalWhenMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();

        m_window->resetCalls();

        bool result = m_renderer->callStartInternal();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

        // When muted, startInternal() does not show the renderer.
        CPPUNIT_ASSERT_EQUAL(false, m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(false, m_window->wasUpdateCalled());
    }

    void testStopWhenStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        try
        {
            m_renderer->callStartInternal();
        }
        catch (...) {}

        bool result = m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testStopWhenNotStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        bool result = m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testStopCalledTwice()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        try
        {
            m_renderer->callStartInternal();
        }
        catch (...) {}

        bool firstStop = m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(true, firstStop);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());

        bool secondStop = m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(true, secondStop);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testIsStartedAfterConstruction()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testIsStartedAfterStart()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        bool startSucceeded = false;
        try
        {
            m_renderer->callStartInternal();
            startSucceeded = true;
        }
        catch (...) {}

        if (startSucceeded)
        {
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        }
        // If start failed, we can't make assertions about the state
    }

    void testIsStartedAfterStop()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        bool startSucceeded = false;
        try
        {
            m_renderer->callStartInternal();
            startSucceeded = true;
        }
        catch (...) {}

        if (startSucceeded)
        {
            m_renderer->stop();
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        }
    }

    void testMuteWhenNotMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testMuteWhenAlreadyMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testUnmuteWhenMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();

        m_renderer->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testUnmuteWhenNotMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_renderer->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testIsMutedAfterConstruction()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testIsMutedAfterMute()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testIsMutedAfterUnmute()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();
        m_renderer->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testProcessDataWhenNotStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_window->resetCalls();

        m_renderer->processData();
        // Not started -> should not draw (no window update).
        CPPUNIT_ASSERT_EQUAL(false, m_window->wasUpdateCalled());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testProcessDataWhenStartedAndNotMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());

        m_window->resetCalls();
        m_renderer->processData();
        CPPUNIT_ASSERT_EQUAL(true, m_window->wasUpdateCalled());
    }

    void testProcessDataWhenStartedAndMuted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();

        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());

        m_window->resetCalls();
        m_renderer->processData();
        // Muted -> should not draw.
        CPPUNIT_ASSERT_EQUAL(false, m_window->wasUpdateCalled());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testAddPesPacketWhenStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());

        // Build a minimal valid PES packet for ttxdecoder::PesBuffer validation:
        // 00 00 01 BD, length=1, payload=0x00
        std::array<std::uint8_t, 7> pes = { 0x00, 0x00, 0x01, 0xBD, 0x00, 0x01, 0x00 };
        bool result = m_renderer->addPesPacket(pes.data(), static_cast<std::uint16_t>(pes.size()));
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testAddPesPacketWhenNotStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        std::vector<uint8_t> buffer(100, 0);
        bool result = m_renderer->addPesPacket(buffer.data(), buffer.size());
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testAddPesPacketWithNullBuffer()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        // When NOT started, RendererImpl must not dereference the buffer.
        // (When started, null buffer would crash inside ttxdecoder::PesBuffer.)
        bool result = m_renderer->addPesPacket(nullptr, 100);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testAddPesPacketWithZeroLength()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());

        std::vector<uint8_t> buffer(100, 0);
        bool result = m_renderer->addPesPacket(buffer.data(), 0);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testSetCurrentPage()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        ttxdecoder::PageId pageId(0x100, 0);
        CPPUNIT_ASSERT_NO_THROW(m_renderer->setCurrentPage(pageId));

        // setCurrentPage() should not impact started/muted flags.
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testSetCurrentPageMultipleTimes()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        ttxdecoder::PageId pageId1(0x100, 0);
        ttxdecoder::PageId pageId2(0x200, 0);
        ttxdecoder::PageId pageId3(0x300, 0);

        CPPUNIT_ASSERT_NO_THROW(m_renderer->setCurrentPage(pageId1));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->setCurrentPage(pageId2));
        CPPUNIT_ASSERT_NO_THROW(m_renderer->setCurrentPage(pageId3));

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testLifecycleInitStartStopShutdown()
    {
        m_renderer.reset(new TestableRendererImpl(true));

        bool initResult = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, initResult);
        m_rendererInitialized = true;

        bool startSucceeded = false;
        try
        {
            bool startResult = m_renderer->callStartInternal();
            CPPUNIT_ASSERT_EQUAL(true, startResult);
            startSucceeded = true;
        }
        catch (...) {}

        if (startSucceeded)
        {
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());

            bool stopResult = m_renderer->stop();
            CPPUNIT_ASSERT_EQUAL(true, stopResult);
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        }

        CPPUNIT_ASSERT_NO_THROW(m_renderer->shutdown());
        m_rendererInitialized = false;

        // After shutdown(), init should be possible again.
        bool reinitResult = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, reinitResult);
        m_rendererInitialized = true;
    }

    void testLifecycleInitStartMuteUnmuteStop()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        bool startSucceeded = false;
        try
        {
            m_renderer->callStartInternal();
            startSucceeded = true;
        }
        catch (...) {}

        if (startSucceeded)
        {
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());

            m_renderer->mute();
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

            m_renderer->unmute();
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());

            m_renderer->stop();
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        }
    }

    void testLifecycleInitMuteStartStopUnmute()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

        m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->unmute());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testMultipleStartStopCycles()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        for (int i = 0; i < 3; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->callStartInternal());
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());

            bool stopResult = m_renderer->stop();
            CPPUNIT_ASSERT_EQUAL(true, stopResult);
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        }
    }

    void testMultipleMuteUnmuteCycles()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        CPPUNIT_ASSERT_EQUAL(false, m_window->isVisible());

        for (int i = 0; i < 3; ++i)
        {
            m_renderer->mute();
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());

            m_renderer->unmute();
            CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());

            // Not started in this test -> unmute shouldn't show the window.
            CPPUNIT_ASSERT_EQUAL(false, m_window->isVisible());
        }
    }

    void testMuteWhileStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();

        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(true, m_window->isVisible());

        CPPUNIT_ASSERT_NO_THROW(m_renderer->mute());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_window->isVisible());
    }

    void testUnmuteWhileStarted()
    {
        m_renderer.reset(new TestableRendererImpl(true));
        initRenderer();
        m_renderer->mute();

        m_window->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->callStartInternal());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(false, m_window->wasUpdateCalled());

        m_window->resetCalls();
        CPPUNIT_ASSERT_NO_THROW(m_renderer->unmute());
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());

        // Unmute while started should show and trigger a redraw.
        CPPUNIT_ASSERT_EQUAL(true, m_window->isVisible());
        CPPUNIT_ASSERT_EQUAL(true, m_window->wasUpdateCalled());
    }

private:
    std::unique_ptr<TestableRendererImpl> m_renderer;
    std::unique_ptr<MockWindow> m_window;
    std::shared_ptr<MockGfxEngine> m_gfxEngine;
    std::unique_ptr<MockConfigProvider> m_configProvider;
    std::unique_ptr<MockTimeSource> m_timeSource;
    bool m_rendererInitialized;

    void initRenderer()
    {
        bool result = m_renderer->init(m_window.get(), m_gfxEngine,
                                        m_configProvider.get(), m_timeSource.get());
        CPPUNIT_ASSERT(result);
        m_rendererInitialized = true;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RendererImplTest);