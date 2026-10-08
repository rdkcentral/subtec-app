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
    MockDrawContext() : fillRectangleCalled(false), fillRectangleCallCount(0), drawPixmapCallCount(0),
                        lastFillColor(0), lastFillRect(0,0,0,0),
                        lastPixmapWidth(0), lastPixmapHeight(0), lastPixmapStride(0),
                        lastClutSize(0) {}
    virtual ~MockDrawContext() = default;

    virtual void fillRectangle(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override
    {
        fillRectangleCalled = true;
        fillRectangleCallCount++;
        lastFillColor = color;
        lastFillRect = rectangle;
    }

    virtual void drawPixmap(const gfx::ClutBitmap& bitmap, const gfx::Rectangle& srcRect, const gfx::Rectangle& dstRect) override
    {
        drawPixmapCallCount++;
        lastSrcRect = srcRect;
        lastDstRect = dstRect;
        lastPixmapWidth = bitmap.m_width;
        lastPixmapHeight = bitmap.m_height;
        lastPixmapStride = bitmap.m_stride;
        lastClutSize = bitmap.m_clutSize;
        capturedPixels.assign(bitmap.m_pixels, bitmap.m_pixels + (bitmap.m_width * bitmap.m_height));
        capturedClut.assign(bitmap.m_clut, bitmap.m_clut + bitmap.m_clutSize);
    }

    virtual void drawUnderline(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override {}
    virtual void drawBitmap(const gfx::Bitmap& bitmap, const gfx::Rectangle& dstRect) override {}
    virtual void drawGlyph(const gfx::FontStripPtr& fontStrip, std::int32_t glyphIndex, const gfx::Rectangle& rect, gfx::ColorArgb fgColor, gfx::ColorArgb bgColor) override {}
    virtual void drawString(gfx::PrerenderedFont& font, const gfx::Rectangle& destinationRect, const std::vector<gfx::GlyphData>& glyphs, const gfx::ColorArgb fgColor, const gfx::ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}

    // Test helpers
    void reset()
    {
        fillRectangleCalled = false;
        fillRectangleCallCount = 0;
        drawPixmapCallCount = 0;
        lastFillColor = gfx::ColorArgb();
        lastFillRect = gfx::Rectangle(0, 0, 0, 0);
        lastSrcRect = gfx::Rectangle(0, 0, 0, 0);
        lastDstRect = gfx::Rectangle(0, 0, 0, 0);
        lastPixmapWidth = 0;
        lastPixmapHeight = 0;
        lastPixmapStride = 0;
        lastClutSize = 0;
        capturedPixels.clear();
        capturedClut.clear();
    }

    bool fillRectangleCalled;
    int fillRectangleCallCount;
    int drawPixmapCallCount;
    gfx::ColorArgb lastFillColor;
    gfx::Rectangle lastFillRect;
    gfx::Rectangle lastSrcRect;
    gfx::Rectangle lastDstRect;
    std::uint32_t lastPixmapWidth;
    std::uint32_t lastPixmapHeight;
    std::uint32_t lastPixmapStride;
    std::size_t lastClutSize;
    std::vector<std::uint8_t> capturedPixels;
    std::vector<std::uint32_t> capturedClut;
};

class MockWindow : public gfx::Window
{
public:
    MockWindow() : m_drawContext(), updateCallCount(0), setSizeCallCount(0),
                   setVisibleCallCount(0), lastVisibleValue(false),
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
        lastVisibleValue = false;
        lastSize = gfx::Size(0, 0);
        m_drawContext.reset();
    }

    MockDrawContext& getMockDrawContext() { return m_drawContext; }

    int updateCallCount;
    int setSizeCallCount;
    int setVisibleCallCount;
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
    CPPUNIT_TEST(testRenderDrawsPixmap);
    CPPUNIT_TEST(testRenderShadow);
    CPPUNIT_TEST(testRenderOutline);
    CPPUNIT_TEST(testRenderOutOfBounds);

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
    void appendCoords(std::vector<uint8_t>& data, uint16_t x, uint16_t y)
    {
        data.push_back(static_cast<uint8_t>(x >> 4));
        data.push_back(static_cast<uint8_t>(((x & 0x0F) << 4) | ((y >> 8) & 0x0F)));
        data.push_back(static_cast<uint8_t>(y & 0xFF));
    }

    std::vector<uint8_t> createBitmapData(BackgroundStyle bgStyle, OutlineStyle outlineStyle,
                                          Coords charTop, Coords charBottom,
                                          const std::vector<uint8_t>& pixels,
                                          uint8_t outlineThickness = 1,
                                          uint8_t shadowRight = 1,
                                          uint8_t shadowBottom = 1)
    {
        std::vector<uint8_t> data;
        uint8_t style = static_cast<uint8_t>(outlineStyle) & 0x03;
        if (bgStyle == BackgroundStyle::FRAMED)
        {
            style |= 0x04;
        }

        data.push_back(style);
        data.push_back(0x00);
        data.push_back(0x00);

        appendCoords(data, charTop.x, charTop.y);
        appendCoords(data, charBottom.x, charBottom.y);

        if (outlineStyle == OutlineStyle::OUTLINE)
        {
            data.push_back(outlineThickness & 0x0F);
            data.push_back(0x00);
            data.push_back(0x00);
        }
        else if (outlineStyle == OutlineStyle::DROP_SHADOW)
        {
            data.push_back(static_cast<uint8_t>((shadowRight & 0x0F) << 4));
            data.push_back(static_cast<uint8_t>(shadowBottom & 0x0F));
            data.push_back(0x00);
        }
        else if (outlineStyle == OutlineStyle::RESERVED)
        {
            data.push_back(0x00);
            data.push_back(0x00);
            data.push_back(0x00);
        }

        data.push_back(static_cast<uint8_t>(pixels.size() >> 8));
        data.push_back(static_cast<uint8_t>(pixels.size() & 0xFF));
        data.insert(data.end(), pixels.begin(), pixels.end());
        return data;
    }

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

        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->getMockDrawContext().fillRectangleCallCount);
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

        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
    }

    void testShowCallSequence()
    {
        m_mockWindow->reset();
        m_renderer->show();

        // Verify all operations occurred
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setSizeCallCount);
        CPPUNIT_ASSERT(m_mockWindow->getMockDrawContext().fillRectangleCalled);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->lastVisibleValue);
    }

    void testShowMultipleCalls()
    {
        m_mockWindow->reset();
        m_renderer->show();
        m_renderer->show();

        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->setSizeCallCount);
        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->updateCallCount);
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
        m_renderer->hide();

        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->setVisibleCallCount);
        CPPUNIT_ASSERT_EQUAL(2, m_mockWindow->updateCallCount);
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

    void testRenderDrawsPixmap()
    {
        std::vector<uint8_t> data = createBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                     Coords{0, 0}, Coords{2, 2},
                                                     std::vector<uint8_t>{1, 0, 0, 1});
        SimpleBitmap bitmap(data.data(), data.size());

        m_mockWindow->reset();
        m_renderer->render(bitmap, 2, 2);

        MockDrawContext& drawContext = m_mockWindow->getMockDrawContext();
        CPPUNIT_ASSERT_EQUAL(1, drawContext.drawPixmapCallCount);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
        CPPUNIT_ASSERT_EQUAL(0, drawContext.lastSrcRect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, drawContext.lastSrcRect.m_y);
        CPPUNIT_ASSERT_EQUAL(2, drawContext.lastSrcRect.m_w);
        CPPUNIT_ASSERT_EQUAL(2, drawContext.lastSrcRect.m_h);
        CPPUNIT_ASSERT_EQUAL(0, drawContext.lastDstRect.m_x);
        CPPUNIT_ASSERT_EQUAL(0, drawContext.lastDstRect.m_y);
        CPPUNIT_ASSERT_EQUAL(1920, drawContext.lastDstRect.m_w);
        CPPUNIT_ASSERT_EQUAL(1080, drawContext.lastDstRect.m_h);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), drawContext.lastPixmapWidth);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), drawContext.lastPixmapHeight);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(2), drawContext.lastPixmapStride);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(COLOR_LAST), drawContext.lastClutSize);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), drawContext.capturedPixels.size());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), drawContext.capturedPixels[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_TRANSPARENT), drawContext.capturedPixels[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_TRANSPARENT), drawContext.capturedPixels[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), drawContext.capturedPixels[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), drawContext.capturedClut[COLOR_TRANSPARENT]);
    }

    void testRenderShadow()
    {
        std::vector<uint8_t> data = createBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::DROP_SHADOW,
                                                     Coords{0, 0}, Coords{1, 0}, std::vector<uint8_t>{1},
                                                     1, 1, 1);
        SimpleBitmap bitmap(data.data(), data.size());

        m_mockWindow->reset();
        m_renderer->render(bitmap, 4, 2);

        const std::vector<std::uint8_t> expected{COLOR_CHARACTER, COLOR_TRANSPARENT, COLOR_TRANSPARENT, COLOR_TRANSPARENT,
                                                 COLOR_TRANSPARENT, COLOR_SHADOW, COLOR_TRANSPARENT, COLOR_TRANSPARENT};
        CPPUNIT_ASSERT(expected == m_mockWindow->getMockDrawContext().capturedPixels);
    }

    void testRenderOutline()
    {
        std::vector<uint8_t> data = createBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::OUTLINE,
                                                     Coords{1, 1}, Coords{2, 2}, std::vector<uint8_t>{1});
        SimpleBitmap bitmap(data.data(), data.size());

        m_mockWindow->reset();
        m_renderer->render(bitmap, 3, 3);

        MockDrawContext& drawContext = m_mockWindow->getMockDrawContext();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), drawContext.capturedPixels[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), drawContext.capturedPixels[3]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_CHARACTER), drawContext.capturedPixels[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), drawContext.capturedPixels[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(COLOR_OUTLINE), drawContext.capturedPixels[7]);
    }

    void testRenderOutOfBounds()
    {
        std::vector<uint8_t> data = createBitmapData(BackgroundStyle::TRANSPARENT, OutlineStyle::NONE,
                                                     Coords{5, 5}, Coords{6, 5}, std::vector<uint8_t>{1});
        SimpleBitmap bitmap(data.data(), data.size());

        m_mockWindow->reset();
        m_renderer->render(bitmap, 2, 2);

        const std::vector<std::uint8_t> expected(4, COLOR_TRANSPARENT);
        CPPUNIT_ASSERT(expected == m_mockWindow->getMockDrawContext().capturedPixels);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->getMockDrawContext().drawPixmapCallCount);
        CPPUNIT_ASSERT_EQUAL(1, m_mockWindow->updateCallCount);
    }

private:
    Renderer* m_renderer;
    MockWindow* m_mockWindow;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteRendererTest );
