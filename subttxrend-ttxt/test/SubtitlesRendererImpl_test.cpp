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
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testStartWithMagazine0_ConvertsTo8);
    CPPUNIT_TEST(testStartWithMagazine1_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine2_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine3_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine4_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine5_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine6_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine7_NoConversion);
    CPPUNIT_TEST(testStartWithMagazine8_NoConversion);
    CPPUNIT_TEST(testStartWithPageNumber0);
    CPPUNIT_TEST(testStartWithPageNumberMax);
    CPPUNIT_TEST(testStartWithMagazine0PageNumber0);
    CPPUNIT_TEST(testStartWithMagazine0PageNumberMax);
    CPPUNIT_TEST(testStartWithMagazine8PageNumber0);
    CPPUNIT_TEST(testStartWithMagazine8PageNumberMax);
    CPPUNIT_TEST(testStartPageIdHasAnySubpage);
    CPPUNIT_TEST(testStartMultipleTimes_Magazine0);
    CPPUNIT_TEST(testStartMultipleTimes_Magazine8);
    CPPUNIT_TEST(testStartMultipleTimes_DifferentPages);
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
    void testConstructor()
    {
        std::unique_ptr<TestableSubtitlesRendererImpl> renderer(new TestableSubtitlesRendererImpl());
        CPPUNIT_ASSERT(renderer.get() != nullptr);
    }

    void testDestructor()
    {
        std::unique_ptr<TestableSubtitlesRendererImpl> renderer(new TestableSubtitlesRendererImpl());
        renderer.reset();
        CPPUNIT_ASSERT(true);
    }

    void testStartWithMagazine0_ConvertsTo8()
    {
        m_renderer->start(0, 0x10);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 0 should be converted to 8: 8 * 0x100 + 0x10 = 0x810
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x810), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine1_NoConversion()
    {
        m_renderer->start(1, 0x10);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 1: 1 * 0x100 + 0x10 = 0x110
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x110), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine2_NoConversion()
    {
        m_renderer->start(2, 0x20);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 2: 2 * 0x100 + 0x20 = 0x220
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x220), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine3_NoConversion()
    {
        m_renderer->start(3, 0x30);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 3: 3 * 0x100 + 0x30 = 0x330
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x330), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine4_NoConversion()
    {
        m_renderer->start(4, 0x40);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 4: 4 * 0x100 + 0x40 = 0x440
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x440), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine5_NoConversion()
    {
        m_renderer->start(5, 0x50);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 5: 5 * 0x100 + 0x50 = 0x550
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x550), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine6_NoConversion()
    {
        m_renderer->start(6, 0x60);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 6: 6 * 0x100 + 0x60 = 0x660
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x660), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine7_NoConversion()
    {
        m_renderer->start(7, 0x70);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 7: 7 * 0x100 + 0x70 = 0x770
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x770), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine8_NoConversion()
    {
        m_renderer->start(8, 0x80);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 8: 8 * 0x100 + 0x80 = 0x880
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x880), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithPageNumber0()
    {
        m_renderer->start(1, 0x00);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 1, page 0: 1 * 0x100 + 0x00 = 0x100
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x100), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithPageNumberMax()
    {
        m_renderer->start(1, 0xFF);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 1, page 0xFF: 1 * 0x100 + 0xFF = 0x1FF
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1FF), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine0PageNumber0()
    {
        m_renderer->start(0, 0x00);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 0 -> 8, page 0: 8 * 0x100 + 0x00 = 0x800
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x800), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine0PageNumberMax()
    {
        m_renderer->start(0, 0xFF);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 0 -> 8, page 0xFF: 8 * 0x100 + 0xFF = 0x8FF
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x8FF), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine8PageNumber0()
    {
        m_renderer->start(8, 0x00);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 8, page 0: 8 * 0x100 + 0x00 = 0x800
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x800), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartWithMagazine8PageNumberMax()
    {
        m_renderer->start(8, 0xFF);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        // Magazine 8, page 0xFF: 8 * 0x100 + 0xFF = 0x8FF
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x8FF), pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(1, m_renderer->getSetCurrentPageCalls());
    }

    void testStartPageIdHasAnySubpage()
    {
        m_renderer->start(1, 0x10);

        ttxdecoder::PageId pageId = m_renderer->getLastPageId();
        CPPUNIT_ASSERT_EQUAL(true, pageId.isAnySubpage());
    }

    void testStartMultipleTimes_Magazine0()
    {
        m_renderer->start(0, 0x10);
        ttxdecoder::PageId pageId1 = m_renderer->getLastPageId();

        m_renderer->start(0, 0x20);
        ttxdecoder::PageId pageId2 = m_renderer->getLastPageId();

        // Both calls should convert magazine 0 to 8
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x810), pageId1.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x820), pageId2.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->getSetCurrentPageCalls());
    }

    void testStartMultipleTimes_Magazine8()
    {
        m_renderer->start(8, 0x10);
        ttxdecoder::PageId pageId1 = m_renderer->getLastPageId();

        m_renderer->start(8, 0x20);
        ttxdecoder::PageId pageId2 = m_renderer->getLastPageId();

        // Magazine 8 should remain 8
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x810), pageId1.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x820), pageId2.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(2, m_renderer->getSetCurrentPageCalls());
    }

    void testStartMultipleTimes_DifferentPages()
    {
        m_renderer->start(1, 0x00);
        ttxdecoder::PageId pageId1 = m_renderer->getLastPageId();

        m_renderer->start(3, 0x55);
        ttxdecoder::PageId pageId2 = m_renderer->getLastPageId();

        m_renderer->start(7, 0xFF);
        ttxdecoder::PageId pageId3 = m_renderer->getLastPageId();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x100), pageId1.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x355), pageId2.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x7FF), pageId3.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(3, m_renderer->getSetCurrentPageCalls());
    }

private:
    std::unique_ptr<TestableSubtitlesRendererImpl> m_renderer;
    MockWindow m_window;
    subttxrend::gfx::EnginePtr m_gfxEngine;
    MockConfigProvider m_configProvider;
    MockTimeSource m_timeSource;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SubtitlesRendererImplTest);