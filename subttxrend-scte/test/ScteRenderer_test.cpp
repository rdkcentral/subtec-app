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
#include "ScteRenderer.hpp"
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>

using namespace subttxrend::scte;
using namespace subttxrend;

class MockDrawContext : public gfx::DrawContext
{
public:
    MockDrawContext() : fillRectangleCalled(false), drawPixmapCalled(false),
                        lastFillColor(0), lastFillRect(0,0,0,0) {}
    virtual ~MockDrawContext() = default;

    virtual void fillRectangle(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override
    {
        fillRectangleCalled = true;
        lastFillColor = color;
        lastFillRect = rectangle;
    }

    virtual void drawPixmap(const gfx::ClutBitmap& bitmap, const gfx::Rectangle& srcRect, const gfx::Rectangle& dstRect) override
    {
        drawPixmapCalled = true;
        lastSrcRect = srcRect;
        lastDstRect = dstRect;
    }

    virtual void drawUnderline(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override {}
    virtual void drawBitmap(const gfx::Bitmap& bitmap, const gfx::Rectangle& dstRect) override {}
    virtual void drawGlyph(const gfx::FontStripPtr& fontStrip, std::int32_t glyphIndex, const gfx::Rectangle& rect, gfx::ColorArgb fgColor, gfx::ColorArgb bgColor) override {}
    virtual void drawString(gfx::PrerenderedFont& font, const gfx::Rectangle& destinationRect, const std::vector<gfx::GlyphData>& glyphs, const gfx::ColorArgb fgColor, const gfx::ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}

    // Test helpers
    void reset()
    {
        fillRectangleCalled = false;
        drawPixmapCalled = false;
        updateCallCount = 0;
        setSizeCallCount = 0;
        setVisibleCallCount = 0;
        lastVisibleValue = false;
    }

    bool fillRectangleCalled;
    bool drawPixmapCalled;
    gfx::ColorArgb lastFillColor;
    gfx::Rectangle lastFillRect;
    gfx::Rectangle lastSrcRect;
    gfx::Rectangle lastDstRect;
    int updateCallCount;
    int setSizeCallCount;
    int setVisibleCallCount;
    bool lastVisibleValue;
};

class MockWindow : public gfx::Window
{
public:
    MockWindow() : m_drawContext(), updateCallCount(0), setSizeCallCount(0),
                   setVisibleCallCount(0), clearCallCount(0), lastVisibleValue(false),
                   lastSize(0, 0) {}
    virtual ~MockWindow() = default;

    virtual void addKeyEventListener(gfx::KeyEventListener* listener) override {}
    virtual void removeKeyEventListener(gfx::KeyEventListener* listener) override {}
    virtual gfx::Rectangle getBounds() const override { return gfx::Rectangle(0, 0, 1920, 1080); }
    virtual gfx::DrawContext& getDrawContext() override { return m_drawContext; }
    virtual gfx::Size getPreferredSize() const override { return gfx::Size(1920, 1080); }

    virtual void setSize(const gfx::Size& newSize) override
    {
        setSizeCallCount++;
        lastSize = newSize;
    }

    virtual gfx::Size getSize() const override { return lastSize; }

    virtual void setVisible(bool visible) override
    {
        setVisibleCallCount++;
        lastVisibleValue = visible;
    }

    virtual void clear() override
    {
        clearCallCount++;
    }

    virtual void update() override
    {
        updateCallCount++;
    }

    virtual void setDrawDirection(gfx::DrawDirection dir) override {}

    // Test helpers
    void reset()
    {
        updateCallCount = 0;
        setSizeCallCount = 0;
        setVisibleCallCount = 0;
        clearCallCount = 0;
        lastVisibleValue = false;
        lastSize = gfx::Size(0, 0);
        m_drawContext.reset();
    }

    MockDrawContext& getMockDrawContext() { return m_drawContext; }

    int updateCallCount;
    int setSizeCallCount;
    int setVisibleCallCount;
    int clearCallCount;
    bool lastVisibleValue;
    gfx::Size lastSize;

private:
    MockDrawContext m_drawContext;
};

class ScteRendererTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteRendererTest );
    CPPUNIT_TEST(testConstructorWithValidWindow);
    CPPUNIT_TEST(testUpdateCallsWindowUpdate);
    CPPUNIT_TEST(testUpdateMultipleCalls);
    CPPUNIT_TEST(testClearscreenFillsWithTransparent);
    CPPUNIT_TEST(testClearscreenUsesWindowBounds);
    CPPUNIT_TEST(testClearscreenMultipleCalls);
    CPPUNIT_TEST(testShowSetsWindowSize);
    CPPUNIT_TEST(testShowClearsScreen);
    CPPUNIT_TEST(testShowMakesVisible);
    CPPUNIT_TEST(testShowCallsUpdate);
    CPPUNIT_TEST(testShowCallSequence);
    CPPUNIT_TEST(testShowMultipleCalls);
    CPPUNIT_TEST(testHideMakesInvisible);
    CPPUNIT_TEST(testHideCallsUpdate);
    CPPUNIT_TEST(testHideMultipleCalls);
    CPPUNIT_TEST(testShowThenHide);
    CPPUNIT_TEST(testHideThenShow);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_mockWindow = new MockWindow();
        m_renderer = new Renderer(m_mockWindow);
    }

    void tearDown()
    {
        delete m_renderer;
        delete m_mockWindow;
    }

protected:
    void testConstructorWithValidWindow()
    {
        // Constructor should not crash with valid window
        Renderer* renderer = new Renderer(m_mockWindow);
        CPPUNIT_ASSERT(renderer != nullptr);
        delete renderer;
    }

    void testUpdateCallsWindowUpdate()
    {
        m_mockWindow->reset();
        m_renderer->update();
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
    }

    void testUpdateMultipleCalls()
    {
        m_mockWindow->reset();
        m_renderer->update();
        m_renderer->update();
        m_renderer->update();
        CPPUNIT_ASSERT_EQUAL(3, m_mockWindow->updateCallCount);
    }

    void testClearscreenFillsWithTransparent()
    {
        m_mockWindow->reset();
        m_renderer->clearscreen();

        CPPUNIT_ASSERT(m_mockWindow->getMockDrawContext().fillRectangleCalled);
        CPPUNIT_ASSERT_EQUAL(static_cast<gfx::ColorArgb>(gfx::ColorArgb::TRANSPARENT),
                            m_mockWindow->getMockDrawContext().lastFillColor);
    }

    void testClearscreenUsesWindowBounds()
    {
        m_mockWindow->reset();
        m_renderer->clearscreen();

        gfx::Rectangle expectedBounds = m_mockWindow->getBounds();
        gfx::Rectangle actualRect = m_mockWindow->getMockDrawContext().lastFillRect;

        CPPUNIT_ASSERT_EQUAL(expectedBounds.m_x, actualRect.m_x);
        CPPUNIT_ASSERT_EQUAL(expectedBounds.m_y, actualRect.m_y);
        CPPUNIT_ASSERT_EQUAL(expectedBounds.m_w, actualRect.m_w);
        CPPUNIT_ASSERT_EQUAL(expectedBounds.m_h, actualRect.m_h);
    }

    void testClearscreenMultipleCalls()
    {
        m_mockWindow->reset();
        m_renderer->clearscreen();
        m_renderer->clearscreen();

        // Both calls should have worked
        CPPUNIT_ASSERT(m_mockWindow->getMockDrawContext().fillRectangleCalled);
    }

    void testShowSetsWindowSize()
    {
        m_mockWindow->reset();
        m_renderer->show();

        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setSizeCallCount);
        CPPUNIT_ASSERT_EQUAL(1920, m_mockWindow->lastSize.m_w);
        CPPUNIT_ASSERT_EQUAL(1080, m_mockWindow->lastSize.m_h);
    }

    void testShowClearsScreen()
    {
        m_mockWindow->reset();
        m_renderer->show();

        // clearscreen() calls fillRectangle
        CPPUNIT_ASSERT(m_mockWindow->getMockDrawContext().fillRectangleCalled);
    }

    void testShowMakesVisible()
    {
        m_mockWindow->reset();
        m_renderer->show();

        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);
    }

    void testShowCallsUpdate()
    {
        m_mockWindow->reset();
        m_renderer->show();

        CPPUNIT_ASSERT(m_mockWindow->updateCallCount >= 1);
    }

    void testShowCallSequence()
    {
        m_mockWindow->reset();
        m_renderer->show();

        // Verify all operations occurred
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setSizeCallCount);
        CPPUNIT_ASSERT(m_mockWindow->getMockDrawContext().fillRectangleCalled);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT(m_mockWindow->updateCallCount >= 1);
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);
    }

    void testShowMultipleCalls()
    {
        m_mockWindow->reset();
        m_renderer->show();
        m_mockWindow->reset();
        m_renderer->show();

        // Second call should work the same
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setSizeCallCount);
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);
    }

    void testHideMakesInvisible()
    {
        m_mockWindow->reset();
        m_renderer->hide();

        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->lastVisibleValue);
    }

    void testHideCallsUpdate()
    {
        m_mockWindow->reset();
        m_renderer->hide();

        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
    }

    void testHideMultipleCalls()
    {
        m_mockWindow->reset();
        m_renderer->hide();
        m_mockWindow->reset();
        m_renderer->hide();

        // Second call should work the same
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->lastVisibleValue);
    }

    void testShowThenHide()
    {
        m_mockWindow->reset();
        m_renderer->show();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);

        m_mockWindow->reset();
        m_renderer->hide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->lastVisibleValue);
    }

    void testHideThenShow()
    {
        m_mockWindow->reset();
        m_renderer->hide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->lastVisibleValue);

        m_mockWindow->reset();
        m_renderer->show();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);
    }

private:
    Renderer* m_renderer;
    MockWindow* m_mockWindow;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteRendererTest );
