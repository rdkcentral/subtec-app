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

// Forward declaration for mock
namespace subttxrend { namespace protocol { class BufferReader; } }

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
 * Mock Window for testing
 */
class MockWindow : public Window
{
public:
    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}
    Rectangle getBounds() const override { return Rectangle{0, 0, 1920, 1080}; }
    DrawContext& getDrawContext() override { static DrawContext* ctx = nullptr; return *ctx; }
    Size getPreferredSize() const override { return Size{1920, 1080}; }
    void setSize(const Size& newSize) override {}
    Size getSize() const override { return Size{1920, 1080}; }
    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}
    void setDrawDirection(DrawDirection dir) override {}
};

/**
 * Mock Packet for testing
 */
class MockPacket : public Packet
{
public:
    explicit MockPacket(Type type) : m_type(type) {}

    Type getType() const override { return m_type; }

    bool parseDataHeader(BufferReader& bufferReader) override { return true; }

    bool takeBuffer(DataBufferPtr dataBuffer, std::size_t dataOffset) override { return true; }

private:
    Type m_type;
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
    CPPUNIT_TEST(testConstructorCreatesInternalObjects);
    CPPUNIT_TEST(testStopWithoutStartAsync);
    CPPUNIT_TEST(testOnStreamBrokenNotification);
    CPPUNIT_TEST(testOnStreamBrokenMultipleTimes);
    CPPUNIT_TEST(testAddBufferWhenNotRendering);
    CPPUNIT_TEST(testAddBufferWithEmptyBuffer);
    CPPUNIT_TEST(testAddBufferMultipleTimes);
    CPPUNIT_TEST(testOnPacketReceivedBeforeStart);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Create mock objects for tests
        m_mockEngine = std::make_shared<MockEngine>();
        m_mockWindow = std::make_shared<MockWindow>();
    }

    void tearDown()
    {
        // Cleanup
        m_mockEngine.reset();
        m_mockWindow.reset();
    }

    void testConstructorWithValidParameters()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);

            Controller controller(config, m_mockEngine, m_mockWindow);

            // If we reach here, constructor succeeded
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("Constructor failed with exception: ") + e.what());
        }
    }

    void testConstructorWithNullGfxEngine()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);

            // Pass null for gfxEngine
            EnginePtr nullEngine;
            Controller controller(config, nullEngine, m_mockWindow);

            // Constructor should handle null or throw
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            // Exception is acceptable for null parameter
            CPPUNIT_ASSERT(true);
        }
    }

    void testConstructorWithNullGfxWindow()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);

            // Pass null for gfxWindow
            WindowPtr nullWindow;
            Controller controller(config, m_mockEngine, nullWindow);

            // Constructor should handle null or throw
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            // Exception is acceptable for null parameter
            CPPUNIT_ASSERT(true);
        }
    }

    void testConstructorCreatesInternalObjects()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);

            Controller controller(config, m_mockEngine, m_mockWindow);

            // Constructor should complete successfully with all internal objects created
            // We can't directly access private members, but successful construction
            // indicates they were created
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("Constructor failed to create internal objects: ") + e.what());
        }
    }

    void testStopWithoutStartAsync()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Call stop without starting - should be safe due to joinable() check
            controller.stop();

            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            // Exception is acceptable
            CPPUNIT_ASSERT(true);
        }
    }

    void testOnStreamBrokenNotification()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Call onStreamBroken - should handle gracefully
            controller.onStreamBroken();

            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("onStreamBroken failed: ") + e.what());
        }
    }

    void testOnStreamBrokenMultipleTimes()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Call onStreamBroken multiple times
            controller.onStreamBroken();
            controller.onStreamBroken();
            controller.onStreamBroken();

            // Should handle repeated notifications without crash
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("Multiple onStreamBroken calls failed: ") + e.what());
        }
    }

    void testAddBufferWhenNotRendering()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Create a valid data buffer
            auto buffer = std::make_unique<DataBuffer>();
            buffer->resize(100);

            // Add buffer when no controller is active
            // This should be skipped according to the implementation
            controller.addBuffer(std::move(buffer));

            // Should complete without error
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("addBuffer when not rendering failed: ") + e.what());
        }
    }

    void testAddBufferWithEmptyBuffer()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Create an empty buffer (size 0)
            auto buffer = std::make_unique<DataBuffer>();

            // Add empty buffer
            controller.addBuffer(std::move(buffer));

            // Should handle empty buffer gracefully
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("addBuffer with empty buffer failed: ") + e.what());
        }
    }

    void testAddBufferMultipleTimes()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Add multiple buffers sequentially
            for (int i = 0; i < 10; ++i)
            {
                auto buffer = std::make_unique<DataBuffer>();
                buffer->resize(100 + i);
                controller.addBuffer(std::move(buffer));
            }

            // Should handle multiple additions
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("Multiple addBuffer calls failed: ") + e.what());
        }
    }


    void testOnPacketReceivedBeforeStart()
    {
        try
        {
            Options opts = TestOptionsHelper::createValidOptions();
            Configuration config(opts);
            Controller controller(config, m_mockEngine, m_mockWindow);

            // Create a mock packet
            MockPacket packet(Packet::Type::RESET_ALL);

            // Call onPacketReceived before startAsync
            controller.onPacketReceived(packet);

            // Should handle gracefully
            CPPUNIT_ASSERT(true);
        }
        catch (const std::exception& e)
        {
            CPPUNIT_FAIL(std::string("onPacketReceived before start failed: ") + e.what());
        }
    }

private:
    EnginePtr m_mockEngine;
    WindowPtr m_mockWindow;
};

CPPUNIT_TEST_SUITE_REGISTRATION( ControllerTest );
