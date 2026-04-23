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
#include "ScteController.hpp"
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/common/StcProvider.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <vector>
#include <cstring>

using namespace subttxrend::scte;
using namespace subttxrend;

class MockDrawContext : public gfx::DrawContext
{
public:
    MockDrawContext() = default;
    virtual ~MockDrawContext() = default;

    virtual void fillRectangle(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override {}
    virtual void drawUnderline(gfx::ColorArgb color, const gfx::Rectangle& rectangle) override {}
    virtual void drawPixmap(const gfx::ClutBitmap& bitmap, const gfx::Rectangle& srcRect, const gfx::Rectangle& dstRect) override {}
    virtual void drawBitmap(const gfx::Bitmap& bitmap, const gfx::Rectangle& dstRect) override {}
    virtual void drawGlyph(const gfx::FontStripPtr& fontStrip, std::int32_t glyphIndex, const gfx::Rectangle& rect, gfx::ColorArgb fgColor, gfx::ColorArgb bgColor) override {}
    virtual void drawString(gfx::PrerenderedFont& font, const gfx::Rectangle& destinationRect, const std::vector<gfx::GlyphData>& glyphs, const gfx::ColorArgb fgColor, const gfx::ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {}
};

class MockWindow : public gfx::Window
{
public:
    MockWindow() : m_drawContext() {}
    virtual ~MockWindow() = default;

    virtual void addKeyEventListener(gfx::KeyEventListener* listener) override {}
    virtual void removeKeyEventListener(gfx::KeyEventListener* listener) override {}
    virtual gfx::Rectangle getBounds() const override { return gfx::Rectangle(0, 0, 1920, 1080); }
    virtual gfx::DrawContext& getDrawContext() override { return m_drawContext; }
    virtual gfx::Size getPreferredSize() const override { return gfx::Size(1920, 1080); }
    virtual void setSize(const gfx::Size& newSize) override {}
    virtual gfx::Size getSize() const override { return gfx::Size(1920, 1080); }
    virtual void setVisible(bool visible) override {}
    virtual void clear() override {}
    virtual void update() override {}
    virtual void setDrawDirection(gfx::DrawDirection dir) override {}

private:
    MockDrawContext m_drawContext;
};

class MockStcProvider : public common::StcProvider
{
public:
    MockStcProvider() : m_stc(0) {}
    virtual ~MockStcProvider() = default;

    virtual std::uint32_t getStc() const override
    {
        return m_stc;
    }

    void setStc(std::uint32_t stc)
    {
        m_stc = stc;
    }

private:
    std::uint32_t m_stc;
};

class ScteControllerTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ScteControllerTest );
    CPPUNIT_TEST(testConstructorInitializesState);
    CPPUNIT_TEST(testInitWithValidParameters);
    CPPUNIT_TEST(testShutdownWithoutInit);
    CPPUNIT_TEST(testShutdownAfterInit);
    CPPUNIT_TEST(testShutdownMultipleTimes);
    CPPUNIT_TEST(testStartWhenNotStarted);
    CPPUNIT_TEST(testStartWhenAlreadyStarted);
    CPPUNIT_TEST(testStopWhenStarted);
    CPPUNIT_TEST(testStopWhenNotStarted);
    CPPUNIT_TEST(testStopReturnsTrue);
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
    CPPUNIT_TEST(testProcessWhenNotStarted);
    CPPUNIT_TEST(testProcessWhenMuted);
    CPPUNIT_TEST(testStartStopStart);
    CPPUNIT_TEST(testMuteUnmuteMute);
    CPPUNIT_TEST(testFullLifecycle);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_mockWindow = new MockWindow();
        m_mockStcProvider = new MockStcProvider();
        m_controller = new Controller();
    }

    void tearDown()
    {
        delete m_controller;
        delete m_mockStcProvider;
        delete m_mockWindow;
    }

protected:
    void testConstructorInitializesState()
    {
        Controller controller;
        CPPUNIT_ASSERT_EQUAL(false, controller.isStarted());
        CPPUNIT_ASSERT_EQUAL(false, controller.isMuted());
    }

    void testInitWithValidParameters()
    {
        bool result = m_controller->init(m_mockWindow, m_mockStcProvider);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testShutdownWithoutInit()
    {
        // Should not crash
        m_controller->shutdown();
        // Verify we can still check state
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testShutdownAfterInit()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->shutdown();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testShutdownMultipleTimes()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->shutdown();
        m_controller->shutdown(); // Should be safe
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testStartWhenNotStarted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        bool result = m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());
    }

    void testStartWhenAlreadyStarted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        bool wasStarted = m_controller->isStarted();
        m_controller->start(); // Should call stop() first
        CPPUNIT_ASSERT_EQUAL(true, wasStarted);
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());
    }

    void testStopWhenStarted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        bool result = m_controller->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testStopWhenNotStarted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        bool result = m_controller->stop();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testStopReturnsTrue()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->stop());
    }

    void testIsStartedAfterConstruction()
    {
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testIsStartedAfterStart()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());
    }

    void testIsStartedAfterStop()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        m_controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testMuteWhenNotMuted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());
    }

    void testMuteWhenAlreadyMuted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->mute();
        m_controller->mute(); // Should be idempotent
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());
    }

    void testUnmuteWhenMuted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->mute();
        m_controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isMuted());
    }

    void testUnmuteWhenNotMuted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->unmute(); // Should be idempotent
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isMuted());
    }

    void testIsMutedAfterConstruction()
    {
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isMuted());
    }

    void testIsMutedAfterMute()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());
    }

    void testIsMutedAfterUnmute()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->mute();
        m_controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isMuted());
    }

    void testProcessWhenNotStarted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        // Should not crash when not started
        m_controller->process();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

    void testProcessWhenMuted()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);
        m_controller->start();
        m_controller->mute();

        // Should not crash when muted
        m_controller->process();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());
    }

    void testStartStopStart()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);

        m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());

        m_controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());

        m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());
    }

    void testMuteUnmuteMute()
    {
        m_controller->init(m_mockWindow, m_mockStcProvider);

        m_controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());

        m_controller->unmute();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isMuted());

        m_controller->mute();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isMuted());
    }

    void testFullLifecycle()
    {
        // Test complete lifecycle without addData (PacketData not mockable)
        m_controller->init(m_mockWindow, m_mockStcProvider);
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());

        m_controller->start();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());

        m_controller->process();
        CPPUNIT_ASSERT_EQUAL(true, m_controller->isStarted());

        m_controller->stop();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());

        m_controller->shutdown();
        CPPUNIT_ASSERT_EQUAL(false, m_controller->isStarted());
    }

private:
    Controller* m_controller;
    MockWindow* m_mockWindow;
    MockStcProvider* m_mockStcProvider;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ScteControllerTest );
