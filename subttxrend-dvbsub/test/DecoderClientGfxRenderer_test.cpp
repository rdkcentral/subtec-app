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

    class MockDrawContext : public DrawContext
    {
    public:
           MockDrawContext() = default;

        void fillRectangle(ColorArgb color, const Rectangle& rectangle) override
        {}

        void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}

        void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect,
                      const Rectangle& dstRect) override
        {}

        void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}

        void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex,
                     const Rectangle& rect, ColorArgb fgColor, ColorArgb bgColor) override {}

        void drawString(PrerenderedFont& font, const Rectangle &destinationRect,
                      const std::vector<GlyphData>& glyphs, const ColorArgb fgColor,
                      const ColorArgb bgColor, int outlineSize = 0,
                      int verticalOffset = 0) override {}
    };

private:
    bool m_visible;
    int m_updateCount;
    Size m_size;
    MockDrawContext m_mockDrawContext;
};

class DecoderClientGfxRendererTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DecoderClientGfxRendererTest);

    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testDestructorAfterInit);
    CPPUNIT_TEST(testGfxInit);
    CPPUNIT_TEST(testGfxInitNullWindow);
    CPPUNIT_TEST(testGfxInitCalledTwice);
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
        try {
            tempRenderer.reset();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Destructor threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor threw unknown exception");
        }

        // Ensure we can still create a renderer afterwards
        auto renderer2 = std::make_unique<DecoderClientGfxRenderer>();
        CPPUNIT_ASSERT(renderer2 != nullptr);
    }

    void testDestructorAfterInit()
    {
        // Test that destructor doesn't crash after initialization
        auto tempRenderer = std::make_unique<DecoderClientGfxRenderer>();
        tempRenderer->gfxInit(m_mockWindow.get());
        try {
            tempRenderer.reset();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Destructor threw exception after init: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor threw unknown exception after init");
        }

        // Ensure a new renderer can be created and initialized after destruction
        auto renderer2 = std::make_unique<DecoderClientGfxRenderer>();
        CPPUNIT_ASSERT(renderer2 != nullptr);
        try {
            renderer2->gfxInit(m_mockWindow.get());
            renderer2->gfxShow();
            CPPUNIT_ASSERT(m_mockWindow->isVisible());
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Post-destruct init/show failed: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Post-destruct init/show threw unknown exception");
        }
    }

    void testGfxInit()
    {
        // Should initialize without throwing and allow showing to affect window
        try {
            m_renderer->gfxInit(m_mockWindow.get());
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("gfxInit threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("gfxInit threw unknown exception");
        }

        // After init, calling show should set window visible and produce an update
        m_mockWindow->resetUpdateCount();
        m_renderer->gfxShow();
        CPPUNIT_ASSERT(m_mockWindow->isVisible());
        CPPUNIT_ASSERT(m_mockWindow->getUpdateCount() > 0);
    }

    void testGfxInitNullWindow()
    {
        // Should throw std::invalid_argument for null window
        CPPUNIT_ASSERT_THROW(m_renderer->gfxInit(nullptr), std::invalid_argument);
    }

    void testGfxInitCalledTwice()
    {
        m_renderer->gfxInit(m_mockWindow.get());

        // Second init should overwrite (no guard against it) and subsequent show should affect the new window
        auto secondWindow = std::make_unique<MockWindow>();
        try {
            m_renderer->gfxInit(secondWindow.get());
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Second gfxInit threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Second gfxInit threw unknown exception");
        }

        secondWindow->resetUpdateCount();
        m_renderer->gfxShow();
        CPPUNIT_ASSERT(secondWindow->isVisible());
        CPPUNIT_ASSERT(secondWindow->getUpdateCount() > 0);
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
        // Should not crash when called without init and should allow later init
        try {
            m_renderer->gfxShow();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("gfxShow threw when uninitialized: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("gfxShow threw unknown exception when uninitialized");
        }

        // Ensure init later still works
        try {
            m_renderer->gfxInit(m_mockWindow.get());
            m_mockWindow->resetUpdateCount();
            m_renderer->gfxShow();
            CPPUNIT_ASSERT(m_mockWindow->isVisible());
            CPPUNIT_ASSERT(m_mockWindow->getUpdateCount() > 0);
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Init/show after uninitialized show failed: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Init/show after uninitialized show threw unknown exception");
        }
    }

    void testGfxHideWithoutInit()
    {
        // Should not crash when called without init and should allow later init
        try {
            m_renderer->gfxHide();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("gfxHide threw when uninitialized: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("gfxHide threw unknown exception when uninitialized");
        }

        // Ensure init later still works
        try {
            m_renderer->gfxInit(m_mockWindow.get());
            m_mockWindow->resetUpdateCount();
            m_renderer->gfxShow();
            CPPUNIT_ASSERT(m_mockWindow->isVisible());
            CPPUNIT_ASSERT(m_mockWindow->getUpdateCount() > 0);
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Init/show after uninitialized hide failed: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Init/show after uninitialized hide threw unknown exception");
        }
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
