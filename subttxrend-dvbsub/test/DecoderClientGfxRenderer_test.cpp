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
#include "DecoderClientGfxRenderer.hpp"
#include <subttxrend/gfx/Window.hpp>
#include <dvbsubdecoder/DecoderClient.hpp>
#include <memory>
#include <vector>
#include <stdexcept>

using namespace subttxrend::dvbsub;
using namespace subttxrend::gfx;

class MockWindow : public Window
{
public:
    MockWindow()
        : m_visible(false)
        , m_updateCount(0)
        , m_size(0, 0)
        , m_fillRectangleCount(0)
        , m_drawPixmapCount(0)
    {}

    virtual ~MockWindow() {}

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}

    Rectangle getBounds() const override
    {
        return Rectangle(0, 0, m_size.m_w, m_size.m_h);
    }

    DrawContext& getDrawContext() override
    {
        return m_mockDrawContext;
    }

    Size getPreferredSize() const override { return Size(1920, 1080); }
    void setSize(const Size& newSize) override { m_size = newSize; }
    Size getSize() const override { return m_size; }
    void setVisible(bool visible) override { m_visible = visible; }
    void clear() override {}
    void update() override { m_updateCount++; }
    void setDrawDirection(DrawDirection dir) override {}

    // Test helper methods
    bool isVisible() const { return m_visible; }
    int getUpdateCount() const { return m_updateCount; }
    void resetUpdateCount() { m_updateCount = 0; }
    void resetFillRectangleCount() { m_fillRectangleCount = 0; }
    void resetDrawPixmapCount() { m_drawPixmapCount = 0; }
    int getFillRectangleCount() const { return m_fillRectangleCount; }
    int getDrawPixmapCount() const { return m_drawPixmapCount; }

    class MockDrawContext : public DrawContext
    {
    public:
        MockDrawContext(MockWindow* parent) : m_parent(parent) {}

        void fillRectangle(ColorArgb color, const Rectangle& rectangle) override
        {
            if (m_parent) m_parent->m_fillRectangleCount++;
        }

        void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}

        void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect,
                      const Rectangle& dstRect) override
        {
            if (m_parent) m_parent->m_drawPixmapCount++;
        }

        void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}

        void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex,
                     const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override {}

        void drawString(PrerenderedFont& font, const Rectangle &destinationRect,
                      const std::vector<GlyphData>& glyphs, const ColorArgb fgColor,
                      const ColorArgb bgColor, int outlineSize = 0,
                      int verticalOffset = 0) override {}

    private:
        MockWindow* m_parent;
    };

private:
    bool m_visible;
    int m_updateCount;
    Size m_size;
    int m_fillRectangleCount;
    int m_drawPixmapCount;
    MockDrawContext m_mockDrawContext{this};
};

class DecoderClientGfxRendererTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DecoderClientGfxRendererTest);

    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testDestructorAfterInit);
    CPPUNIT_TEST(testGetDecoderClient);
    CPPUNIT_TEST(testGfxInit);
    CPPUNIT_TEST(testGfxInitNullWindow);
    CPPUNIT_TEST(testGfxInitCalledTwice);
    CPPUNIT_TEST(testGfxShutdown);
    CPPUNIT_TEST(testGfxShutdownCalledTwice);
    CPPUNIT_TEST(testGfxShow);
    CPPUNIT_TEST(testGfxHide);
    CPPUNIT_TEST(testGfxShowWithoutInit);
    CPPUNIT_TEST(testGfxHideWithoutInit);
    CPPUNIT_TEST(testMultipleShowHideCycles);
    CPPUNIT_TEST(testShowWhenAlreadyShown);
    CPPUNIT_TEST(testHideWhenAlreadyHidden);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_renderer = std::make_unique<DecoderClientGfxRenderer>();
        m_mockWindow = std::make_unique<MockWindow>();
    }

    void tearDown() override
    {
        m_renderer.reset();
        m_mockWindow.reset();
    }

    void testConstructor()
    {
        // Should construct without issues
        CPPUNIT_ASSERT(m_renderer != nullptr);
    }

    void testDestructor()
    {
        // Test that destructor doesn't crash without initialization
        auto tempRenderer = std::make_unique<DecoderClientGfxRenderer>();
        tempRenderer.reset();
        CPPUNIT_ASSERT(true);
    }

    void testDestructorAfterInit()
    {
        // Test that destructor doesn't crash after initialization
        auto tempRenderer = std::make_unique<DecoderClientGfxRenderer>();
        tempRenderer->gfxInit(m_mockWindow.get());
        tempRenderer.reset();
        CPPUNIT_ASSERT(true);
    }

    void testGetDecoderClient()
    {
        dvbsubdecoder::DecoderClient& client = m_renderer->getDecoderClient();

        // Should return a valid reference
        CPPUNIT_ASSERT(&client != nullptr);
    }

    void testGfxInit()
    {
        // Should initialize without throwing
        m_renderer->gfxInit(m_mockWindow.get());
        CPPUNIT_ASSERT(true);
    }

    void testGfxInitNullWindow()
    {
        // Should throw std::invalid_argument for null window
        CPPUNIT_ASSERT_THROW(m_renderer->gfxInit(nullptr), std::invalid_argument);
    }

    void testGfxInitCalledTwice()
    {
        m_renderer->gfxInit(m_mockWindow.get());

        // Second init should overwrite (no guard against it)
        auto secondWindow = std::make_unique<MockWindow>();
        m_renderer->gfxInit(secondWindow.get());

        CPPUNIT_ASSERT(true);
    }

    void testGfxShutdown()
    {
        m_renderer->gfxInit(m_mockWindow.get());

        // Should shutdown without issues
        m_renderer->gfxShutdown();

        CPPUNIT_ASSERT(true);
    }

    void testGfxShutdownCalledTwice()
    {
        m_renderer->gfxInit(m_mockWindow.get());
        m_renderer->gfxShutdown();

        // Second shutdown should not crash
        m_renderer->gfxShutdown();

        CPPUNIT_ASSERT(true);
    }

    void testGfxShow()
    {
        m_renderer->gfxInit(m_mockWindow.get());
        m_mockWindow->resetUpdateCount();

        m_renderer->gfxShow();

        CPPUNIT_ASSERT(m_mockWindow->isVisible());
        CPPUNIT_ASSERT(m_mockWindow->getUpdateCount() > 0);
    }

    void testGfxHide()
    {
        m_renderer->gfxInit(m_mockWindow.get());
        m_renderer->gfxShow();
        m_mockWindow->resetUpdateCount();

        m_renderer->gfxHide();

        CPPUNIT_ASSERT(!m_mockWindow->isVisible());
        CPPUNIT_ASSERT(m_mockWindow->getUpdateCount() > 0);
    }

    void testGfxShowWithoutInit()
    {
        // Should not crash when called without init
        m_renderer->gfxShow();
        CPPUNIT_ASSERT(true);
    }

    void testGfxHideWithoutInit()
    {
        // Should not crash when called without init
        m_renderer->gfxHide();
        CPPUNIT_ASSERT(true);
    }

    void testMultipleShowHideCycles()
    {
        m_renderer->gfxInit(m_mockWindow.get());

        // First cycle
        m_renderer->gfxShow();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
        m_renderer->gfxHide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());

        // Second cycle
        m_renderer->gfxShow();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
        m_renderer->gfxHide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());

        // Third cycle
        m_renderer->gfxShow();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
    }

    void testShowWhenAlreadyShown()
    {
        m_renderer->gfxInit(m_mockWindow.get());
        m_renderer->gfxShow();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());

        // Show again - should remain visible
        m_renderer->gfxShow();
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
    }

    void testHideWhenAlreadyHidden()
    {
        m_renderer->gfxInit(m_mockWindow.get());
        m_renderer->gfxHide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());

        // Hide again - should remain hidden
        m_renderer->gfxHide();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

private:
    std::unique_ptr<DecoderClientGfxRenderer> m_renderer;
    std::unique_ptr<MockWindow> m_mockWindow;
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(DecoderClientGfxRendererTest);
