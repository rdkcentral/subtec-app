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
#include <cstring>

#include "Controller.hpp"
#include <subttxrend/ctrl/Options.hpp>
#include <subttxrend/ctrl/Configuration.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <subttxrend/protocol/Packet.hpp>
#include <subttxrend/common/DataBuffer.hpp>

using namespace subttxrend::app;
using namespace subttxrend::ctrl;
using namespace subttxrend::gfx;
using namespace subttxrend::protocol;
using namespace subttxrend::common;

/**
 * Mock Engine for testing
 */
class MockEngine : public Engine
{
public:
    void init(const std::string& displayName = {}) override {}
    void shutdown() override {}
    void execute() override {}
    WindowPtr createWindow() override { return nullptr; }
    void attach(WindowPtr window) override {}
    void detach(WindowPtr window) override {}
    FontStripPtr createFontStrip(const Size& glyphSize, std::size_t numGlyphs) override { return nullptr; }
};

/**
 * Mock DrawContext for testing
 */
class MockDrawContext : public DrawContext
{
public:
    void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {}
    void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
    void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
    void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}
    void drawGlyph(const FontStripPtr& fontStrip,
                   std::int32_t glyphIndex,
                   const Rectangle& rect,
                   ColorArgb fgColor,
                   ColorArgb bgColor) override {}
    void drawString(PrerenderedFont& font,
                    const Rectangle& destinationRect,
                    const std::vector<GlyphData>& glyphs,
                    const ColorArgb fgColor,
                    const ColorArgb bgColor,
                    int outlineSize = 0,
                    int verticalOffset = 0) override {}
};

/**
 * Mock Window for testing
 */
class MockWindow : public Window
{
public:
    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}
    Rectangle getBounds() const override { return Rectangle{0, 0, 1920, 1080}; }
    DrawContext& getDrawContext() override { return m_drawContext; }
    Size getPreferredSize() const override { return Size{1920, 1080}; }
    void setSize(const Size& newSize) override {}
    Size getSize() const override { return Size{1920, 1080}; }
    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}

private:
    MockDrawContext m_drawContext;
};

class TestOptionsHelper
{
public:
    static std::string getTestConfigPath()
    {
        std::string filePath = __FILE__;
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            return filePath.substr(0, lastSlash + 1) + "test_config.ini";
        }
        return "test_config.ini";
    }

    static Options createValidOptions()
    {
        static std::string configPath = getTestConfigPath();
        static std::string configArg = "--config-file-path=" + configPath;
        static std::vector<std::string> args = {
            "subttxrend-app",
            "--main-socket-path=/tmp/test_socket",
            configArg
        };
        std::vector<char*> argv;
        for (auto& arg : args)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        return Options(static_cast<int>(argv.size()), argv.data());
    }
};

class ControllerTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ControllerTest );
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithNullGfxEngine);
    CPPUNIT_TEST(testConstructorWithNullGfxWindow);
    CPPUNIT_TEST(testStopWithoutStartAsync);
    CPPUNIT_TEST(testOnStreamBrokenNotification);
    CPPUNIT_TEST(testAddBufferWhenNotRendering);
    CPPUNIT_TEST(testOnPacketReceivedBeforeStart);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_mockEngine = std::make_shared<MockEngine>();
        m_mockWindow = std::make_shared<MockWindow>();
    }

    void tearDown()
    {
        m_mockEngine.reset();
        m_mockWindow.reset();
    }

    void testConstructorWithValidParameters()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        CPPUNIT_ASSERT_NO_THROW(Controller controller(config, m_mockEngine, m_mockWindow));
    }

    void testConstructorWithNullGfxEngine()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        EnginePtr nullEngine;
        auto constructWithNullEngine = [&]() {
            Controller controller(config, nullEngine, m_mockWindow);
        };
        CPPUNIT_ASSERT_NO_THROW(constructWithNullEngine());
    }

    void testConstructorWithNullGfxWindow()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        WindowPtr nullWindow;
        auto constructWithNullWindow = [&]() {
            Controller controller(config, m_mockEngine, nullWindow);
        };
        CPPUNIT_ASSERT_NO_THROW(constructWithNullWindow());
    }

    void testStopWithoutStartAsync()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        Controller controller(config, m_mockEngine, m_mockWindow);

        CPPUNIT_ASSERT_NO_THROW(controller.stop());
    }

    void testOnStreamBrokenNotification()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        Controller controller(config, m_mockEngine, m_mockWindow);

        CPPUNIT_ASSERT_NO_THROW(controller.onStreamBroken());
        CPPUNIT_ASSERT_NO_THROW(controller.onStreamBroken());
    }

    void testAddBufferWhenNotRendering()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        Controller controller(config, m_mockEngine, m_mockWindow);

        auto emptyBuffer = std::make_unique<DataBuffer>();
        CPPUNIT_ASSERT_NO_THROW(controller.addBuffer(std::move(emptyBuffer)));

        for (int i = 0; i < 10; ++i)
        {
            auto buffer = std::make_unique<DataBuffer>();
            buffer->resize(100 + i);
            CPPUNIT_ASSERT_NO_THROW(controller.addBuffer(std::move(buffer)));
        }
    }

    void testOnPacketReceivedBeforeStart()
    {
        Options opts = TestOptionsHelper::createValidOptions();
        Configuration config(opts);
        Controller controller(config, m_mockEngine, m_mockWindow);

        PacketResetAll packet;
        CPPUNIT_ASSERT_NO_THROW(controller.onPacketReceived(packet));
    }

private:
    EnginePtr m_mockEngine;
    WindowPtr m_mockWindow;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ControllerTest );
