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
#include <vector>
#include <string>
#include "SubtitlesRendererImpl.hpp"
#include "TimeSource.hpp"
#include <ttxdecoder/PageId.hpp>

#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/common/ConfigProvider.hpp>

using namespace subttxrend::ttxt;

/**
 * Minimal mocks required to safely call RendererImpl::init() and start().
 * Mirrors the approach used in RendererImpl_test.cpp.
 */
class MockDrawContext : public subttxrend::gfx::DrawContext
{
public:
    void fillRectangle(subttxrend::gfx::ColorArgb,
                       const subttxrend::gfx::Rectangle&) override {}
    void drawUnderline(subttxrend::gfx::ColorArgb,
                       const subttxrend::gfx::Rectangle&) override {}
    void drawPixmap(const subttxrend::gfx::ClutBitmap&,
                    const subttxrend::gfx::Rectangle&,
                    const subttxrend::gfx::Rectangle&) override {}
    void drawBitmap(const subttxrend::gfx::Bitmap&,
                    const subttxrend::gfx::Rectangle&) override {}
    void drawGlyph(const subttxrend::gfx::FontStripPtr&,
                   std::int32_t,
                   const subttxrend::gfx::Rectangle&,
                   subttxrend::gfx::ColorArgb,
                   subttxrend::gfx::ColorArgb) override {}
    void drawString(subttxrend::gfx::PrerenderedFont&,
                    const subttxrend::gfx::Rectangle&,
                    const std::vector<subttxrend::gfx::GlyphData>&,
                    subttxrend::gfx::ColorArgb,
                    subttxrend::gfx::ColorArgb,
                    int,
                    int) override {}
};

class MockWindow : public subttxrend::gfx::Window
{
public:
    MockWindow()
    {
        m_size.m_w = 1280;
        m_size.m_h = 720;
        m_bounds.m_x = 0;
        m_bounds.m_y = 0;
        m_bounds.m_w = 1280;
        m_bounds.m_h = 720;
    }

    void setVisible(bool) override {}
    void setSize(const subttxrend::gfx::Size& size) override { m_size = size; }
    subttxrend::gfx::Size getSize() const override { return m_size; }
    subttxrend::gfx::Size getPreferredSize() const override { return m_size; }
    void update() override {}
    void clear() override {}
    subttxrend::gfx::DrawContext& getDrawContext() override { return m_drawContext; }
    subttxrend::gfx::Rectangle getBounds() const override { return m_bounds; }
    void setDrawDirection(subttxrend::gfx::DrawDirection) override {}
    void addKeyEventListener(subttxrend::gfx::KeyEventListener*) override {}
    void removeKeyEventListener(subttxrend::gfx::KeyEventListener*) override {}

private:
    subttxrend::gfx::Size m_size;
    subttxrend::gfx::Rectangle m_bounds;
    MockDrawContext m_drawContext;
};

class MockFontStrip : public subttxrend::gfx::FontStrip
{
public:
    MockFontStrip(const subttxrend::gfx::Size&, std::size_t) {}

    bool loadGlyph(std::int32_t,
                   const std::uint8_t*,
                   std::size_t) override
    {
        return true;
    }

    bool loadFont(const std::string&,
                  const subttxrend::gfx::Size&,
                  const subttxrend::gfx::FontStripMap&) override
    {
        return true;
    }
};

class MockGfxEngine : public subttxrend::gfx::Engine
{
public:
    void init(const std::string& = {}) override {}
    void shutdown() override {}
    void execute() override {}
    subttxrend::gfx::WindowPtr createWindow() override { return nullptr; }
    void attach(subttxrend::gfx::WindowPtr) override {}
    void detach(subttxrend::gfx::WindowPtr) override {}
    subttxrend::gfx::FontStripPtr createFontStrip(const subttxrend::gfx::Size& glyphSize,
                                                  std::size_t glyphCount) override
    {
        return std::make_shared<MockFontStrip>(glyphSize, glyphCount);
    }
};

class MockConfigProvider : public subttxrend::common::ConfigProvider
{
protected:
    const char* getValue(const std::string&) const override { return nullptr; }
};

class MockTimeSource : public TimeSource
{
public:
    std::uint32_t getStc() override { return 0; }
};

/**
 * Testable SubtitlesRendererImpl that captures method calls for verification
 */
class TestableSubtitlesRendererImpl : public SubtitlesRendererImpl
{
public:
    TestableSubtitlesRendererImpl()
        : m_setCurrentPageCalls(0),
          m_lastPageId(0, 0)
    {
    }

    int getSetCurrentPageCalls() const { return m_setCurrentPageCalls; }
    ttxdecoder::PageId getLastPageId() const { return m_lastPageId; }

protected:
    void setCurrentPage(const ttxdecoder::PageId& pageId) override
    {
        m_lastPageId = pageId;
        ++m_setCurrentPageCalls;

        // Keep base renderer state consistent (startInternal() uses decoder engine).
        RendererImpl::setCurrentPage(pageId);
    }

private:
    int m_setCurrentPageCalls;
    ttxdecoder::PageId m_lastPageId;
};

class SubtitlesRendererImplTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SubtitlesRendererImplTest);
    CPPUNIT_TEST(testStartWithMagazine0_ConvertsTo8);
    CPPUNIT_TEST(testStartWithMagazine1_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine8_NoConversion);
    CPPUNIT_TEST(testStartForwardsRawPage);
    CPPUNIT_TEST(testStartForwardsRawPageWithMagazine0);
    CPPUNIT_TEST(testStartPageIdHasAnySubpage);
    CPPUNIT_TEST(testStartMultipleTimes_UpdatesSelectedPage);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_renderer.reset(new TestableSubtitlesRendererImpl());

        m_gfxEngine = std::make_shared<MockGfxEngine>();
        m_renderer->init(&m_window, m_gfxEngine, &m_configProvider, &m_timeSource);
    }

    void tearDown() override
    {
        if (m_renderer)
        {
            // Clean shutdown to avoid cross-test contamination via GfxRenderer singleton.
            m_renderer->stop();
            m_renderer->shutdown();
        }
        m_renderer.reset();
        m_gfxEngine.reset();
    }

protected:
    void assertStartSetsPage(std::uint32_t magazineNumber,
                             std::uint32_t pageNumber,
                             std::uint16_t expectedMagazinePage,
                             int expectedSetCurrentPageCalls,
                             bool verifyStartedState = false)
    {
        bool result = m_renderer->start(magazineNumber, pageNumber);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(expectedMagazinePage, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(expectedSetCurrentPageCalls, m_renderer->getSetCurrentPageCalls());

        if (verifyStartedState)
        {
            CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        }
    }

    void testStartWithMagazine0_ConvertsTo8()
    {
        assertStartSetsPage(0, 0x10, static_cast<std::uint16_t>(0x810), 1, true);
    }

    void testStartWithMagazine1_NoConversion()
    {
        assertStartSetsPage(1, 0x10, static_cast<std::uint16_t>(0x110), 1);
    }

    void testStartWithMagazine8_NoConversion()
    {
        assertStartSetsPage(8, 0x80, static_cast<std::uint16_t>(0x880), 1);
    }

    void testStartForwardsRawPage()
    {
        assertStartSetsPage(1, 0xAA, static_cast<std::uint16_t>(0x1AA), 1);
    }

    void testStartForwardsRawPageWithMagazine0()
    {
        assertStartSetsPage(0, 0xFF, static_cast<std::uint16_t>(0x8FF), 1);
    }

    void testStartPageIdHasAnySubpage()
    {
        assertStartSetsPage(1, 0x10, static_cast<std::uint16_t>(0x110), 1);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        CPPUNIT_ASSERT_EQUAL(true, pageId.isAnySubpage());
    }

    void testStartMultipleTimes_UpdatesSelectedPage()
    {
        assertStartSetsPage(0, 0x10, static_cast<std::uint16_t>(0x810), 1, true);
        ttxdecoder::PageId pageId1 = m_renderer->getLastPageId();

        assertStartSetsPage(8, 0x20, static_cast<std::uint16_t>(0x820), 2, true);
        ttxdecoder::PageId pageId2 = m_renderer->getLastPageId();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x810), pageId1.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x820), pageId2.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->getSetCurrentPageCalls());
    }

private:
    std::unique_ptr<TestableSubtitlesRendererImpl> m_renderer;
    MockWindow m_window;
    subttxrend::gfx::EnginePtr m_gfxEngine;
    MockConfigProvider m_configProvider;
    MockTimeSource m_timeSource;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SubtitlesRendererImplTest);