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
#include "SubtitlesRendererImpl.hpp"
#include <subttxrend/gfx/Window.hpp>
#include "TimeSource.hpp"
#include <memory>
#include <stdexcept>

using namespace subttxrend::dvbsub;
using namespace subttxrend::gfx;

class MockWindow : public Window
{
public:
    MockWindow()
        : m_visible(false)
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
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}

    // Test helper methods
    bool isVisible() const { return m_visible; }

    class MockDrawContext : public DrawContext
    {
    public:
        void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {}
        void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
        void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect,
                      const Rectangle& dstRect) override {}
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
    Size m_size;
    MockDrawContext m_mockDrawContext;
};

class MockTimeSource : public TimeSource
{
public:
    MockTimeSource() : m_stc(0) {}
    virtual ~MockTimeSource() = default;

    std::uint32_t getStc() override
    {
        return m_stc;
    }

private:
    std::uint32_t m_stc;
};

class SubtitlesRendererImplTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SubtitlesRendererImplTest);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testDestructorAfterInit);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testInitWithNullWindow);
    CPPUNIT_TEST(testInitWithNullTimeSource);
    CPPUNIT_TEST(testInitAlreadyInitialized);
    CPPUNIT_TEST(testInitAfterShutdown);
    CPPUNIT_TEST(testShutdown);
    CPPUNIT_TEST(testShutdownNotInitialized);
    CPPUNIT_TEST(testShutdownCalledTwice);
    CPPUNIT_TEST(testStartAfterInit);
    CPPUNIT_TEST(testStartWhenMuted);
    CPPUNIT_TEST(testStopWhenStarted);
    CPPUNIT_TEST(testStopWhenAlreadyStopped);
    CPPUNIT_TEST(testIsStartedReturnsFalseInitially);
    CPPUNIT_TEST(testIsStartedReturnsTrueAfterStart);
    CPPUNIT_TEST(testIsStartedReturnsFalseAfterStop);
    CPPUNIT_TEST(testMuteWhenAlreadyMuted);
    CPPUNIT_TEST(testMuteWhenNotStarted);
    CPPUNIT_TEST(testMuteWhenStarted);
    CPPUNIT_TEST(testUnmuteWhenAlreadyUnmuted);
    CPPUNIT_TEST(testUnmuteWhenNotStarted);
    CPPUNIT_TEST(testUnmuteWhenStarted);
    CPPUNIT_TEST(testIsMutedReturnsFalseInitially);
    CPPUNIT_TEST(testIsMutedReturnsTrueAfterMute);
    CPPUNIT_TEST(testIsMutedReturnsFalseAfterUnmute);
    CPPUNIT_TEST(testAddPesPacketWhenNotStarted);
    CPPUNIT_TEST(testProcessDataWhenNotStarted);
    CPPUNIT_TEST(testFullLifecycle);
    CPPUNIT_TEST(testMuteUnmuteCycle);
    CPPUNIT_TEST(testMultipleStartStopCycles);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_renderer = std::make_unique<SubtitlesRendererImpl>();
        m_mockWindow = std::make_unique<MockWindow>();
        m_mockTimeSource = std::make_unique<MockTimeSource>();
    }

    void tearDown() override
    {
        if (m_renderer)
        {
            m_renderer->shutdown();
        }
        m_renderer.reset();
        m_mockWindow.reset();
        m_mockTimeSource.reset();
    }

    void testConstructor()
    {
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testDestructor()
    {
        auto tempRenderer = std::make_unique<SubtitlesRendererImpl>();
        try {
            tempRenderer.reset();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Destructor threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor threw unknown exception");
        }
    }

    void testDestructorAfterInit()
    {
        auto tempRenderer = std::make_unique<SubtitlesRendererImpl>();
        tempRenderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        try {
            tempRenderer.reset();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Destructor after init threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Destructor after init threw unknown exception");
        }

        // Ensure renderer can be recreated and initialized again
        auto r2 = std::make_unique<SubtitlesRendererImpl>();
        bool ok = r2->init(m_mockWindow.get(), m_mockTimeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, ok);
    }

    void testInit()
    {
        bool result = m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testInitWithNullWindow()
    {
        // gfxRenderer.gfxInit() will throw std::invalid_argument
        CPPUNIT_ASSERT_THROW(
            m_renderer->init(nullptr, m_mockTimeSource.get()),
            std::invalid_argument
        );
    }

    void testInitWithNullTimeSource()
    {
        // TimeSource can be null - DecoderTimeProvider handles it gracefully
        bool result = m_renderer->init(m_mockWindow.get(), nullptr);

        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testInitAlreadyInitialized()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        // Second init should return false
        bool result = m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testInitAfterShutdown()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->shutdown();

        // Should be able to reinitialize
        bool result = m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testShutdown()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        // Perform shutdown and verify it is safe to call
        try {
            m_renderer->shutdown();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Shutdown threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Shutdown threw unknown exception");
        }
    }

    void testShutdownNotInitialized()
    {
        // Shutdown without init should not throw
        try {
            m_renderer->shutdown();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Shutdown (not initialized) threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Shutdown (not initialized) threw unknown exception");
        }
    }

    void testShutdownCalledTwice()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->shutdown();
        // Second shutdown should not throw
        try {
            m_renderer->shutdown();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("Second shutdown threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("Second shutdown threw unknown exception");
        }
    }

    void testStartAfterInit()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        bool result = m_renderer->start(100, 200);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
    }

    void testStartWhenMuted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->mute();

        bool result = m_renderer->start(100, 200);

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        // Should NOT show graphics when muted
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testStopWhenStarted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);

        bool result = m_renderer->stop();

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testStopWhenAlreadyStopped()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        // Stop when not started should return true without error
        bool result = m_renderer->stop();

        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testIsStartedReturnsFalseInitially()
    {
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testIsStartedReturnsTrueAfterStart()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);

        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
    }

    void testIsStartedReturnsFalseAfterStop()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);
        m_renderer->stop();

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

    void testMuteWhenAlreadyMuted()
    {
        m_renderer->mute();
        m_renderer->mute();

        // Should remain muted (idempotent)
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testMuteWhenNotStarted()
    {
        m_renderer->mute();

        // Should only set flag, not affect graphics
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testMuteWhenStarted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());

        m_renderer->mute();

        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        // Should hide graphics
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testUnmuteWhenAlreadyUnmuted()
    {
        m_renderer->unmute();

        // Should remain unmuted (idempotent)
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testUnmuteWhenNotStarted()
    {
        m_renderer->mute();
        m_renderer->unmute();

        // Should only set flag, not affect graphics
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testUnmuteWhenStarted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);
        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());

        m_renderer->unmute();

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
        // Should show graphics and invalidate decoder
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());
    }

    void testIsMutedReturnsFalseInitially()
    {
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testIsMutedReturnsTrueAfterMute()
    {
        m_renderer->mute();

        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
    }

    void testIsMutedReturnsFalseAfterUnmute()
    {
        m_renderer->mute();
        m_renderer->unmute();

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
    }

    void testAddPesPacketWhenNotStarted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        std::uint8_t buffer[] = {0x00, 0x01, 0x02, 0x03};
        bool result = m_renderer->addPesPacket(buffer, sizeof(buffer));

        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testProcessDataWhenNotStarted()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        // Should not throw when called before start and should not show graphics
        try {
            m_renderer->processData();
        } catch (const std::exception& e) {
            CPPUNIT_FAIL(std::string("processData (not started) threw exception: ") + e.what());
        } catch (...) {
            CPPUNIT_FAIL("processData (not started) threw unknown exception");
        }

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testFullLifecycle()
    {
        // Complete lifecycle: init → start → stop
        bool initResult = m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        CPPUNIT_ASSERT_EQUAL(true, initResult);

        bool startResult = m_renderer->start(100, 200);
        CPPUNIT_ASSERT_EQUAL(true, startResult);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());

        bool stopResult = m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(true, stopResult);
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testMuteUnmuteCycle()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());
        m_renderer->start(100, 200);

        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());

        m_renderer->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(true, m_mockWindow->isVisible());

        m_renderer->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isMuted());
        CPPUNIT_ASSERT_EQUAL(false, m_mockWindow->isVisible());
    }

    void testMultipleStartStopCycles()
    {
        m_renderer->init(m_mockWindow.get(), m_mockTimeSource.get());

        // First cycle
        m_renderer->start(100, 200);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());

        // Second cycle
        m_renderer->start(300, 400);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());

        // Third cycle
        m_renderer->start(500, 600);
        CPPUNIT_ASSERT_EQUAL(true, m_renderer->isStarted());
        m_renderer->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_renderer->isStarted());
    }

private:
    std::unique_ptr<SubtitlesRendererImpl> m_renderer;
    std::unique_ptr<MockWindow> m_mockWindow;
    std::unique_ptr<MockTimeSource> m_mockTimeSource;
};

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(SubtitlesRendererImplTest);
