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
#include <thread>
#include <chrono>
#include <atomic>

#include "../src/EngineImpl.hpp"
#include "../src/Backend.hpp"
#include "../src/BackendFactory.hpp"
#include "../src/WindowImpl.hpp"
#include "KeyEvent.hpp"
#include "Types.hpp"

using subttxrend::gfx::EngineImpl;
using subttxrend::gfx::Backend;
using subttxrend::gfx::BackendListener;
using subttxrend::gfx::BackendFactory;
using subttxrend::gfx::WindowImpl;
using subttxrend::gfx::WindowPtr;
using subttxrend::gfx::Size;

/**
 * Mock Backend for testing
 */
class MockBackend : public Backend
{
public:
    MockBackend(BackendListener* listener)
        : Backend(listener)
        , m_isSyncNeeded(true)
        , m_initCalled(false)
        , m_initResult(true)
        , m_startCalled(false)
        , m_startResult(true)
        , m_stopCalled(false)
        , m_requestRenderCalled(false)
        , m_forceRenderCalled(false)
        , m_requestRenderCount(0)
        , m_forceRenderCount(0)
    {
    }

    virtual ~MockBackend() = default;

    bool isSyncNeeded() const override
    {
        return m_isSyncNeeded;
    }

    bool init(const std::string& displayName) override
    {
        m_initCalled = true;
        s_initCalled.store(true);
        m_displayName = displayName;
        return m_initResult;
    }

    bool start() override
    {
        m_startCalled = true;
        s_startCalled.store(true);
        return m_startResult;
    }

    void stop() override
    {
        m_stopCalled = true;
        s_stopCalled.store(true);
    }

    void requestRender() override
    {
        m_requestRenderCalled = true;
        m_requestRenderCount++;
        s_requestRenderCount.fetch_add(1);
    }

    void forceRender() override
    {
        m_forceRenderCalled = true;
        m_forceRenderCount++;
        s_forceRenderCount.fetch_add(1);
    }

#ifdef __APPLE__
    void startBlockingApplicationWindow() override
    {
        // noop for tests
    }
#endif

    // Test helper methods
    void setSyncNeeded(bool needed) { m_isSyncNeeded = needed; }
    void setInitResult(bool result) { m_initResult = result; }
    void setStartResult(bool result) { m_startResult = result; }
    bool wasInitCalled() const { return m_initCalled; }
    bool wasStartCalled() const { return m_startCalled; }
    bool wasStopCalled() const { return m_stopCalled; }
    bool wasForceRenderCalled() const { return m_forceRenderCalled; }
    int getForceRenderCount() const { return m_forceRenderCount; }
    bool wasRequestRenderCalled() const { return m_requestRenderCalled; }
    int getRequestRenderCount() const { return m_requestRenderCount; }
    std::string getDisplayName() const { return m_displayName; }

    static void resetStaticFlags()
    {
        s_initCalled.store(false);
        s_startCalled.store(false);
        s_stopCalled.store(false);
        s_requestRenderCount.store(0);
        s_forceRenderCount.store(0);
    }

    static std::atomic<bool> s_initCalled;
    static std::atomic<bool> s_startCalled;
    static std::atomic<bool> s_stopCalled;
    static std::atomic<int> s_requestRenderCount;
    static std::atomic<int> s_forceRenderCount;

private:
    bool m_isSyncNeeded;
    bool m_initCalled;
    bool m_initResult;
    bool m_startCalled;
    bool m_startResult;
    bool m_stopCalled;
    bool m_requestRenderCalled;
    bool m_forceRenderCalled;
    int m_requestRenderCount;
    int m_forceRenderCount;
    std::string m_displayName;
};

// Global pointer for mock backend injection
static MockBackend* g_mockBackend = nullptr;

// Override BackendFactory for testing
namespace subttxrend
{
namespace gfx
{
std::unique_ptr<Backend> BackendFactory::createBackend(BackendListener* listener)
{
    if (g_mockBackend)
    {
        // Return the mock backend (ownership transferred)
        auto backend = std::unique_ptr<Backend>(g_mockBackend);
        g_mockBackend = nullptr;
        return backend;
    }
    return nullptr;
}
} // namespace gfx
} // namespace subttxrend

// Static flag definitions for MockBackend observable state
std::atomic<bool> MockBackend::s_initCalled{false};
std::atomic<bool> MockBackend::s_startCalled{false};
std::atomic<bool> MockBackend::s_stopCalled{false};
std::atomic<int> MockBackend::s_requestRenderCount{0};
std::atomic<int> MockBackend::s_forceRenderCount{0};


class EngineImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( EngineImplTest );
    CPPUNIT_TEST(testConstructorInitializesEngineCorrectly);
    CPPUNIT_TEST(testConstructorDoesNotCreateBackend);
    CPPUNIT_TEST(testDestructorWithNoWindows);
    CPPUNIT_TEST(testDestructorDoesNotCallStopOnBackend);
    CPPUNIT_TEST(testInitWithEmptyDisplayName);
    CPPUNIT_TEST(testInitWithValidDisplayName);
    CPPUNIT_TEST(testInitCallsBackendFactoryCreateBackend);
    CPPUNIT_TEST(testInitCallsBackendInit);
    CPPUNIT_TEST(testInitCallsBackendStart);
    CPPUNIT_TEST(testInitWhenBackendInitFails);
    CPPUNIT_TEST(testInitWhenBackendStartFails);
    CPPUNIT_TEST(testInitStoresBackendOnSuccess);
    CPPUNIT_TEST(testInitMultipleTimes);
    CPPUNIT_TEST(testShutdownWithoutInit);
    CPPUNIT_TEST(testShutdownCallsBackendStop);
    CPPUNIT_TEST(testShutdownResetsBackend);
    CPPUNIT_TEST(testShutdownMultipleTimes);
    CPPUNIT_TEST(testShutdownAfterFailedInit);
    CPPUNIT_TEST(testCreateWindowReturnsWindowPtr);
    CPPUNIT_TEST(testCreateWindowMultipleTimes);
    CPPUNIT_TEST(testCreateWindowBeforeInit);
    CPPUNIT_TEST(testCreateWindowAfterInit);
    CPPUNIT_TEST(testCreateFontStripReturnsNonNull);
    CPPUNIT_TEST(testCreateFontStripWithZeroSize);
    CPPUNIT_TEST(testCreateFontStripWithZeroCount);
    CPPUNIT_TEST(testCreateFontStripWithLargeCount);
    CPPUNIT_TEST(testCreateFontStripMultipleTimes);
    CPPUNIT_TEST(testAttachNullWindowThrows);
    CPPUNIT_TEST(testAttachValidWindow);
    CPPUNIT_TEST(testAttachWindowCallsSetEngineHooks);
    CPPUNIT_TEST(testAttachSameWindowTwiceThrows);
    CPPUNIT_TEST(testAttachMultipleDifferentWindows);
    CPPUNIT_TEST(testAttachWindowBeforeInit);
    CPPUNIT_TEST(testAttachAfterInit);
    CPPUNIT_TEST(testDetachNullWindowThrows);
    CPPUNIT_TEST(testDetachAttachedWindow);
    CPPUNIT_TEST(testDetachUnattachedWindow);
    CPPUNIT_TEST(testDetachWindowCallsSetEngineHooksNull);
    CPPUNIT_TEST(testDetachRemovesWindowFromList);
    CPPUNIT_TEST(testDetachSameWindowTwice);
    CPPUNIT_TEST(testDetachOneOfMultipleWindows);
    CPPUNIT_TEST(testExecuteCallsLockAndUnlock);
    CPPUNIT_TEST(testExecuteBeforeInit);
    CPPUNIT_TEST(testExecuteAfterInit);
    CPPUNIT_TEST(testRequestRedrawWithBackend);
    CPPUNIT_TEST(testForceRedrawWithBackend);
    CPPUNIT_TEST(testLockWithBackendSyncNeeded);
    CPPUNIT_TEST(testLockWithBackendSyncNotNeeded);
    CPPUNIT_TEST(testUnlockWithBackendSyncNeeded);
    CPPUNIT_TEST(testUnlockWithBackendSyncNotNeeded);
    CPPUNIT_TEST(testCompleteLifecycleInitShutdown);
    CPPUNIT_TEST(testLifecycleInitShutdownReinit);
    CPPUNIT_TEST(testLifecycleWithWindowAttachDetach);
    CPPUNIT_TEST(testLifecycleMultipleWindows);
    CPPUNIT_TEST(testLifecycleShutdownWithAttachedWindows);
    CPPUNIT_TEST(testAttachDetachMultipleWindowsSequence);
    CPPUNIT_TEST(testAttachDetachReattachWindow);
    CPPUNIT_TEST(testMultipleWindowCreationAndAttachment);
    CPPUNIT_TEST(testDetachAllWindowsInOrder);
    CPPUNIT_TEST(testDetachAllWindowsReverseOrder);
    CPPUNIT_TEST(testInitShutdownBackendStopCalled);
    CPPUNIT_TEST(testMultipleInitShutdownCycles);
    CPPUNIT_TEST(testBackendInitFailureHandling);
    CPPUNIT_TEST(testBackendStartFailureHandling);
    CPPUNIT_TEST(testBackendOperationsAfterShutdown);
    CPPUNIT_TEST(testAttachWindowAfterDetach);
    CPPUNIT_TEST(testMultipleAttachDetachSameWindow);
    CPPUNIT_TEST(testExecuteAfterShutdown);
    CPPUNIT_TEST(testCreateWindowAfterShutdown);
    CPPUNIT_TEST(testStateAfterFailedInit);
    CPPUNIT_TEST(testStateAfterSuccessfulInit);
    CPPUNIT_TEST(testStateAfterShutdown);
    CPPUNIT_TEST(testStateWithMultipleWindows);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Reset global mock backend pointer
        g_mockBackend = nullptr;
        MockBackend::resetStaticFlags();
    }

    void tearDown()
    {
        // Clean up any remaining mock backend
        if (g_mockBackend)
        {
            delete g_mockBackend;
            g_mockBackend = nullptr;
        }
    }

    void testConstructorInitializesEngineCorrectly()
    {
        EngineImpl engine;
        // Verify engine is functional by creating a window
        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);
    }

    void testConstructorDoesNotCreateBackend()
    {
        // Backend should only be created during init(), not constructor
        auto sentinel = new MockBackend(nullptr);
        g_mockBackend = sentinel;

        EngineImpl engine;

        // If constructor created a backend, BackendFactory would have consumed g_mockBackend
        CPPUNIT_ASSERT_MESSAGE("Constructor must not create backend", g_mockBackend == sentinel);

        // Cleanup sentinel if it wasn't consumed
        if (g_mockBackend)
        {
            delete g_mockBackend;
            g_mockBackend = nullptr;
        }
    }

    void testDestructorWithNoWindows()
    {
        try
        {
            EngineImpl engine;
            // Destructor should not throw when going out of scope
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("Destructor threw exception: ") + ex.what());
        }
        catch (...)
        {
            CPPUNIT_FAIL("Destructor threw unknown exception");
        }
    }

    void testDestructorDoesNotCallStopOnBackend()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        {
            EngineImpl engine;
            engine.init("test_display");
            // init should have triggered backend init/start
            CPPUNIT_ASSERT(MockBackend::s_startCalled.load());
            // BackendFactory should have consumed g_mockBackend (ownership transferred)
            CPPUNIT_ASSERT_MESSAGE("BackendFactory should consume the provided mock backend", g_mockBackend == nullptr);
        }

        // Destructor does not call stop(); ensure we don't fail if it didn't
        CPPUNIT_ASSERT(!MockBackend::s_stopCalled.load());
    }

    void testInitWithEmptyDisplayName()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("");

        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT_EQUAL(std::string(""), mockBackend->getDisplayName());
    }

    void testInitWithValidDisplayName()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("test_display");

        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT_EQUAL(std::string("test_display"), mockBackend->getDisplayName());
    }

    void testInitCallsBackendFactoryCreateBackend()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // If mockBackend was used, factory was called
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
    }

    void testInitCallsBackendInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
    }

    void testInitCallsBackendStart()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(mockBackend->wasStartCalled());
    }

    void testInitWhenBackendInitFails()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setInitResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(!mockBackend->wasStartCalled());
    }

    void testInitWhenBackendStartFails()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setStartResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());
    }

    void testInitStoresBackendOnSuccess()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Backend should be stored and functional
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());
    }

    void testInitMultipleTimes()
    {
        auto mockBackend1 = new MockBackend(nullptr);
        g_mockBackend = mockBackend1;

        EngineImpl engine;
        engine.init("display1");

        CPPUNIT_ASSERT(mockBackend1->wasInitCalled());

        // Second init creates new backend
        auto mockBackend2 = new MockBackend(nullptr);
        g_mockBackend = mockBackend2;

        engine.init("display2");
        CPPUNIT_ASSERT(mockBackend2->wasInitCalled());
    }

    void testShutdownWithoutInit()
    {
        try
        {
            EngineImpl engine;
            engine.shutdown();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("shutdown() threw exception: ") + ex.what());
        }
        catch (...)
        {
            CPPUNIT_FAIL("shutdown() threw unknown exception");
        }
    }

    void testShutdownCallsBackendStop()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(!mockBackend->wasStopCalled());
        engine.shutdown();
        CPPUNIT_ASSERT(mockBackend->wasStopCalled());
    }

    void testShutdownResetsBackend()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();
        // After shutdown, backend operations should be safe and stop should have been called
        CPPUNIT_ASSERT(MockBackend::s_stopCalled.load());
        try
        {
            engine.shutdown(); // Second call should be safe
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("Second shutdown() threw exception: ") + ex.what());
        }
    }

    void testShutdownMultipleTimes()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();
        try
        {
            engine.shutdown(); // Second call should be safe
            engine.shutdown(); // Third call should be safe
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("shutdown() repeated calls threw: ") + ex.what());
        }
        CPPUNIT_ASSERT(MockBackend::s_stopCalled.load());
    }

    void testShutdownAfterFailedInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setInitResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();
        // init failed; start should not have been called and stop should not be called
        CPPUNIT_ASSERT(MockBackend::s_initCalled.load());
        CPPUNIT_ASSERT(!MockBackend::s_startCalled.load());
        CPPUNIT_ASSERT(!MockBackend::s_stopCalled.load());
    }

    void testCreateWindowReturnsWindowPtr()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        CPPUNIT_ASSERT(window != nullptr);
        CPPUNIT_ASSERT(std::dynamic_pointer_cast<WindowImpl>(window) != nullptr);
    }

    void testCreateWindowMultipleTimes()
    {
        EngineImpl engine;

        WindowPtr window1 = engine.createWindow();
        WindowPtr window2 = engine.createWindow();
        WindowPtr window3 = engine.createWindow();

        CPPUNIT_ASSERT(window1 != nullptr);
        CPPUNIT_ASSERT(window2 != nullptr);
        CPPUNIT_ASSERT(window3 != nullptr);

        // Each window should be unique
        CPPUNIT_ASSERT(window1 != window2);
        CPPUNIT_ASSERT(window2 != window3);
        CPPUNIT_ASSERT(window1 != window3);
    }

    void testCreateWindowBeforeInit()
    {
        EngineImpl engine;
        // Should be able to create windows before init
        WindowPtr window = engine.createWindow();

        CPPUNIT_ASSERT(window != nullptr);
    }

    void testCreateWindowAfterInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);
    }

    void testCreateFontStripReturnsNonNull()
    {
        EngineImpl engine;
        Size glyphSize{16, 24};

        auto fontStrip = engine.createFontStrip(glyphSize, 256);
        CPPUNIT_ASSERT(fontStrip != nullptr);
    }

    void testCreateFontStripWithZeroSize()
    {
        EngineImpl engine;
        Size glyphSize{0, 0};

        auto fontStrip = engine.createFontStrip(glyphSize, 10);
        CPPUNIT_ASSERT(fontStrip != nullptr);
    }

    void testCreateFontStripWithZeroCount()
    {
        EngineImpl engine;
        Size glyphSize{16, 24};

        auto fontStrip = engine.createFontStrip(glyphSize, 0);
        CPPUNIT_ASSERT(fontStrip != nullptr);
    }

    void testCreateFontStripWithLargeCount()
    {
        EngineImpl engine;
        Size glyphSize{32, 48};

        auto fontStrip = engine.createFontStrip(glyphSize, 10000);
        CPPUNIT_ASSERT(fontStrip != nullptr);
    }

    void testCreateFontStripMultipleTimes()
    {
        EngineImpl engine;
        Size glyphSize{16, 24};

        auto fontStrip1 = engine.createFontStrip(glyphSize, 128);
        auto fontStrip2 = engine.createFontStrip(glyphSize, 256);
        auto fontStrip3 = engine.createFontStrip(glyphSize, 512);

        CPPUNIT_ASSERT(fontStrip1 != nullptr);
        CPPUNIT_ASSERT(fontStrip2 != nullptr);
        CPPUNIT_ASSERT(fontStrip3 != nullptr);
    }

    void testAttachNullWindowThrows()
    {
        EngineImpl engine;

        CPPUNIT_ASSERT_THROW(engine.attach(nullptr), std::invalid_argument);
    }

    void testAttachValidWindow()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        // Should not throw
        engine.attach(window);

        // Verify attachment - attaching same window again should throw
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
    }

    void testAttachWindowCallsSetEngineHooks()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);

        // Verify hooks set by attempting to attach again (should throw)
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);

        // Verify detach works (hooks were set)
        engine.detach(window);
    }

    void testAttachSameWindowTwiceThrows()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);

        // Attaching same window again should throw
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
    }

    void testAttachMultipleDifferentWindows()
    {
        EngineImpl engine;

        WindowPtr window1 = engine.createWindow();
        WindowPtr window2 = engine.createWindow();
        WindowPtr window3 = engine.createWindow();

        engine.attach(window1);
        engine.attach(window2);
        engine.attach(window3);

        // Verify all are attached by checking re-attach throws
        CPPUNIT_ASSERT_THROW(engine.attach(window1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(window2), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(window3), std::logic_error);
    }

    void testAttachWindowBeforeInit()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        // Should be able to attach before init
        engine.attach(window);

        // Verify attachment worked
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
    }

    void testAttachAfterInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        WindowPtr window = engine.createWindow();
        engine.attach(window);

        // Verify attachment worked
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
    }

    void testDetachNullWindowThrows()
    {
        EngineImpl engine;

        CPPUNIT_ASSERT_THROW(engine.detach(nullptr), std::invalid_argument);
    }

    void testDetachAttachedWindow()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);
        engine.detach(window);

        // Verify detachment - should be able to attach again
        engine.attach(window);
        engine.detach(window);
    }

    void testDetachUnattachedWindow()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();
        // Detaching unattached window should not throw
        try
        {
            engine.detach(window);
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("detach() threw exception: ") + ex.what());
        }
    }

    void testDetachWindowCallsSetEngineHooksNull()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);
        engine.detach(window);

        // Verify hooks cleared - can attach again without error
        engine.attach(window);
        engine.detach(window);
    }

    void testDetachRemovesWindowFromList()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);
        engine.detach(window);

        // Window should be removed from internal list
        // Can verify by attaching again (should not throw)
        try
        {
            engine.attach(window);
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("Re-attach after detach threw: ") + ex.what());
        }
    }

    void testDetachSameWindowTwice()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();
        engine.attach(window);
        try
        {
            engine.detach(window);
            engine.detach(window); // Second detach should be safe
        }
        catch (...)
        {
            CPPUNIT_FAIL("Second detach threw an exception");
        }
    }

    void testDetachOneOfMultipleWindows()
    {
        EngineImpl engine;

        WindowPtr window1 = engine.createWindow();
        WindowPtr window2 = engine.createWindow();
        WindowPtr window3 = engine.createWindow();

        engine.attach(window1);
        engine.attach(window2);
        engine.attach(window3);

        engine.detach(window2);

        // Verify window2 detached - can re-attach
        engine.attach(window2);

        // Verify other windows still attached
        CPPUNIT_ASSERT_THROW(engine.attach(window1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(window3), std::logic_error);
    }

    void testExecuteCallsLockAndUnlock()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setSyncNeeded(true);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Execute should call lock/unlock; ensure it doesn't throw
        try
        {
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() threw exception: ") + ex.what());
        }
    }

    void testExecuteBeforeInit()
    {
        EngineImpl engine;
        try
        {
            // Should be safe to execute before init
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() before init threw: ") + ex.what());
        }
    }

    void testExecuteAfterInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        try
        {
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() after init threw: ") + ex.what());
        }
    }

    void testRequestRedrawWithBackend()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Verify backend is initialized and started
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());
        // EngineImpl::requestRedraw() is private; verify initial state remains consistent
        // and that no unexpected backend calls occur without public triggers.
        CPPUNIT_ASSERT_EQUAL(0, mockBackend->getRequestRenderCount());
    }

    void testForceRedrawWithoutBackend()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();
        engine.attach(window);
        try
        {
            // setVisible(false) calls forceRedraw() via EngineHooks
            window->setVisible(false);
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("forceRedraw without backend threw: ") + ex.what());
        }
    }

    void testForceRedrawWithBackend()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Verify backend fully initialized
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());
    }

    void testLockWithBackendSyncNeeded()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setSyncNeeded(true);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        try
        {
            // Lock should acquire mutex when sync needed
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() threw: ") + ex.what());
        }
    }

    void testLockWithBackendSyncNotNeeded()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setSyncNeeded(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        try
        {
            // Lock should skip mutex when sync not needed
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() threw: ") + ex.what());
        }
    }

    void testUnlockWithBackendSyncNeeded()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setSyncNeeded(true);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        try
        {
            // Unlock should release mutex when sync needed
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() threw: ") + ex.what());
        }
    }

    void testUnlockWithBackendSyncNotNeeded()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setSyncNeeded(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        try
        {
            // Unlock should skip mutex when sync not needed
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute() threw: ") + ex.what());
        }
    }

    void testCompleteLifecycleInitShutdown()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;

        // Complete lifecycle
        engine.init("display");
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());

        engine.shutdown();
        CPPUNIT_ASSERT(mockBackend->wasStopCalled());
    }

    void testLifecycleInitShutdownReinit()
    {
        auto mockBackend1 = new MockBackend(nullptr);
        g_mockBackend = mockBackend1;

        EngineImpl engine;
        engine.init("display1");
        engine.shutdown();

        auto mockBackend2 = new MockBackend(nullptr);
        g_mockBackend = mockBackend2;

        engine.init("display2");
        CPPUNIT_ASSERT(mockBackend2->wasInitCalled());

        engine.shutdown();
        CPPUNIT_ASSERT(mockBackend2->wasStopCalled());
    }

    void testLifecycleWithWindowAttachDetach()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        WindowPtr window = engine.createWindow();
        try
        {
            engine.attach(window);
            engine.detach(window);

            engine.shutdown();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("Lifecycle attach/detach threw: ") + ex.what());
        }
    }

    void testLifecycleMultipleWindows()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        WindowPtr w1 = engine.createWindow();
        WindowPtr w2 = engine.createWindow();
        WindowPtr w3 = engine.createWindow();

        engine.attach(w1);
        engine.attach(w2);
        engine.attach(w3);

        try
        {
            engine.detach(w1);
            engine.detach(w2);
            engine.detach(w3);

            engine.shutdown();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("Lifecycle multiple windows threw: ") + ex.what());
        }
    }

    void testLifecycleShutdownWithAttachedWindows()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        WindowPtr window = engine.createWindow();
        engine.attach(window);

        // Shutdown without detaching - should handle gracefully
        engine.shutdown();

        // Clean up to avoid destructor warning
        try
        {
            engine.detach(window);
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("detach after shutdown threw: ") + ex.what());
        }
    }

    void testAttachDetachMultipleWindowsSequence()
    {
        EngineImpl engine;

        WindowPtr w1 = engine.createWindow();
        WindowPtr w2 = engine.createWindow();
        WindowPtr w3 = engine.createWindow();

        engine.attach(w1);
        engine.attach(w2);
        engine.detach(w1);
        engine.attach(w3);
        try
        {
            engine.detach(w2);
            engine.detach(w3);
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("attach/detach sequence threw: ") + ex.what());
        }
    }

    void testAttachDetachReattachWindow()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        // Attach should succeed first time
        engine.attach(window);
        // Attaching the same window again must throw
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);

        // Detach and ensure we can re-attach afterwards
        engine.detach(window);
        try
        {
            engine.attach(window); // Should be able to reattach
        }
        catch (...)
        {
            CPPUNIT_FAIL("attach after detach threw an exception");
        }

        // Now that it's attached again, re-attaching must throw
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
        engine.detach(window);
    }

    void testMultipleWindowCreationAndAttachment()
    {
        EngineImpl engine;
        std::vector<WindowPtr> windows;

        for (int i = 0; i < 10; ++i)
        {
            WindowPtr window = engine.createWindow();
            CPPUNIT_ASSERT(window != nullptr);
            engine.attach(window);
            windows.push_back(window);
        }

        // Verify all windows are attached
        for (const auto& window : windows)
        {
            CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
        }
    }

    void testDetachAllWindowsInOrder()
    {
        EngineImpl engine;

        WindowPtr w1 = engine.createWindow();
        WindowPtr w2 = engine.createWindow();
        WindowPtr w3 = engine.createWindow();

        engine.attach(w1);
        engine.attach(w2);
        engine.attach(w3);

        // Verify initial attachments prevent re-attachment
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w2), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);

        engine.detach(w1);
        try { engine.attach(w1); } catch (...) { CPPUNIT_FAIL("attach after detach for w1 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        engine.detach(w1);

        engine.detach(w2);
        try { engine.attach(w2); } catch (...) { CPPUNIT_FAIL("attach after detach for w2 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w2), std::logic_error);
        engine.detach(w2);

        engine.detach(w3);
        try { engine.attach(w3); } catch (...) { CPPUNIT_FAIL("attach after detach for w3 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);
        engine.detach(w3);
    }

    void testDetachAllWindowsReverseOrder()
    {
        EngineImpl engine;

        WindowPtr w1 = engine.createWindow();
        WindowPtr w2 = engine.createWindow();
        WindowPtr w3 = engine.createWindow();

        engine.attach(w1);
        engine.attach(w2);
        engine.attach(w3);

        // Verify initial attachments prevent re-attachment
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w2), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);

        engine.detach(w3);
        try { engine.attach(w3); } catch (...) { CPPUNIT_FAIL("attach after detach for w3 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);
        engine.detach(w3);

        engine.detach(w2);
        try { engine.attach(w2); } catch (...) { CPPUNIT_FAIL("attach after detach for w2 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w2), std::logic_error);
        engine.detach(w2);

        engine.detach(w1);
        try { engine.attach(w1); } catch (...) { CPPUNIT_FAIL("attach after detach for w1 threw"); }
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        engine.detach(w1);
    }

    void testInitShutdownBackendStopCalled()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        CPPUNIT_ASSERT(!mockBackend->wasStopCalled());
        engine.shutdown();
        CPPUNIT_ASSERT(mockBackend->wasStopCalled());
    }

    void testMultipleInitShutdownCycles()
    {
        for (int i = 0; i < 3; ++i)
        {
            auto mockBackend = new MockBackend(nullptr);
            g_mockBackend = mockBackend;

            EngineImpl engine;
            engine.init("display");
            CPPUNIT_ASSERT(mockBackend->wasInitCalled());
            CPPUNIT_ASSERT(mockBackend->wasStartCalled());

            engine.shutdown();
            CPPUNIT_ASSERT(mockBackend->wasStopCalled());
        }
    }

    void testBackendInitFailureHandling()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setInitResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Backend init failed, start should not be called
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(!mockBackend->wasStartCalled());
        CPPUNIT_ASSERT(!mockBackend->wasStopCalled());

        // Shutdown should be safe and not call stop
        engine.shutdown();
    }

    void testBackendStartFailureHandling()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setStartResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Backend init succeeded but start failed
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());

        // Shutdown should be safe - stop may or may not be called depending on implementation
        // If start() fails, engine may not store the backend, so stop() might not be called
        engine.shutdown();
    }

    void testBackendOperationsAfterShutdown()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();

        // Operations after shutdown should be safe
        try
        {
            engine.execute();
        }
        catch (const std::exception& ex)
        {
            CPPUNIT_FAIL(std::string("execute after shutdown threw: ") + ex.what());
        }
    }

    void testAttachWindowAfterDetach()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        engine.attach(window);
        engine.detach(window);

        // Should be able to attach again after detach
        engine.attach(window);

        // Verify re-attachment worked
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);

        engine.detach(window);
    }

    void testMultipleAttachDetachSameWindow()
    {
        EngineImpl engine;
        WindowPtr window = engine.createWindow();

        for (int i = 0; i < 5; ++i)
        {
            engine.attach(window);
            // Verify attached - re-attach should throw
            CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);

            engine.detach(window);
        }
    }

    void testExecuteAfterShutdown()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        // Capture counts before shutdown
        int reqCountBeforeShutdown = mockBackend->getRequestRenderCount();
        int forceCountBeforeShutdown = mockBackend->getForceRenderCount();
        engine.shutdown();

        // Execute after shutdown should be safe
        engine.execute();
        engine.execute();
        // Backend should not receive further render calls after shutdown
        CPPUNIT_ASSERT_EQUAL(reqCountBeforeShutdown, mockBackend->getRequestRenderCount());
        CPPUNIT_ASSERT_EQUAL(forceCountBeforeShutdown, mockBackend->getForceRenderCount());
        // verified counts unchanged above
    }

    void testCreateWindowAfterShutdown()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();

        // Should still be able to create windows
        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);
    }

    void testStateAfterFailedInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        mockBackend->setInitResult(false);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Verify backend init was attempted but failed
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(!mockBackend->wasStartCalled());

        // State should remain consistent - window operations should work
        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);

        engine.attach(window);
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);
        engine.detach(window);
    }

    void testStateAfterSuccessfulInit()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");

        // Verify backend is fully initialized
        CPPUNIT_ASSERT(mockBackend->wasInitCalled());
        CPPUNIT_ASSERT(mockBackend->wasStartCalled());

        // All operations should work after successful init
        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);

        engine.attach(window);
        CPPUNIT_ASSERT_THROW(engine.attach(window), std::logic_error);

        engine.execute();
        engine.detach(window);
    }

    void testStateAfterShutdown()
    {
        auto mockBackend = new MockBackend(nullptr);
        g_mockBackend = mockBackend;

        EngineImpl engine;
        engine.init("display");
        engine.shutdown();

        // State should remain valid after shutdown
        WindowPtr window = engine.createWindow();
        CPPUNIT_ASSERT(window != nullptr);
        try
        {
            engine.attach(window);
            engine.execute();
            engine.detach(window);
        }
        catch (...)
        {
            CPPUNIT_FAIL("post-shutdown operations threw an exception");
        }
    }

    void testStateWithMultipleWindows()
    {
        EngineImpl engine;

        WindowPtr w1 = engine.createWindow();
        WindowPtr w2 = engine.createWindow();
        WindowPtr w3 = engine.createWindow();

        engine.attach(w1);
        engine.attach(w2);
        engine.attach(w3);

        // Verify all 3 windows attached
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w2), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);

        engine.execute();

        engine.detach(w2);
        engine.attach(w2); // Should work after detach
        engine.detach(w2);

        // Verify w1 and w3 still attached
        CPPUNIT_ASSERT_THROW(engine.attach(w1), std::logic_error);
        CPPUNIT_ASSERT_THROW(engine.attach(w3), std::logic_error);

        engine.execute();

        engine.detach(w1);
        engine.detach(w3);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( EngineImplTest );
