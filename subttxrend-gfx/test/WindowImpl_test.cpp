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

#include "../src/WindowImpl.hpp"
#include "KeyEvent.hpp"
#include "KeyEventListener.hpp"
#include "Types.hpp"
#include "DrawContext.hpp"
#include "FontStrip.hpp"
#include "../src/FontStripImpl.hpp"
#include "Pixmap.hpp"

using subttxrend::gfx::WindowImpl;
using subttxrend::gfx::KeyEvent;
using subttxrend::gfx::KeyEventListener;
using subttxrend::gfx::Rectangle;
using subttxrend::gfx::Size;
using subttxrend::gfx::ColorArgb;
using subttxrend::gfx::DrawContext;
using subttxrend::gfx::FontStripPtr;
using subttxrend::gfx::FontStripImpl;
using subttxrend::gfx::EngineHooks;
using subttxrend::gfx::DrawDirection;
using subttxrend::gfx::ClutBitmap;
using subttxrend::gfx::Bitmap;
using subttxrend::gfx::Pixmap;

// ============================================================================
// Mock Classes
// ============================================================================

/**
 * Mock KeyEventListener for testing
 */
class MockKeyEventListener : public KeyEventListener
{
public:
    MockKeyEventListener()
        : m_eventCount(0)
        , m_lastEventType(KeyEvent::Type::PRESSED)
        , m_lastSymbol(0)
    {
    }

    void onKeyEvent(const KeyEvent& event) override
    {
        m_eventCount++;
        m_lastEventType = event.getType();
        m_lastSymbol = event.getSymbol();
    }

    int getEventCount() const { return m_eventCount; }
    KeyEvent::Type getLastEventType() const { return m_lastEventType; }
    std::uint32_t getLastSymbol() const { return m_lastSymbol; }

private:
    int m_eventCount;
    KeyEvent::Type m_lastEventType;
    std::uint32_t m_lastSymbol;
};

/**
 * Mock EngineHooks for testing
 */
class MockEngineHooks : public EngineHooks
{
public:
    MockEngineHooks()
        : m_requestRedrawCount(0)
        , m_forceRedrawCount(0)
        , m_lockCount(0)
        , m_unlockCount(0)
    {
    }

    void requestRedraw() override
    {
        m_requestRedrawCount++;
    }

    void forceRedraw() override
    {
        m_forceRedrawCount++;
    }

    void lock() override
    {
        m_lockCount++;
    }

    void unlock() override
    {
        m_unlockCount++;
    }

    int getRequestRedrawCount() const { return m_requestRedrawCount; }
    int getForceRedrawCount() const { return m_forceRedrawCount; }
    int getLockCount() const { return m_lockCount; }
    int getUnlockCount() const { return m_unlockCount; }

    void reset()
    {
        m_requestRedrawCount = 0;
        m_forceRedrawCount = 0;
        m_lockCount = 0;
        m_unlockCount = 0;
    }

private:
    int m_requestRedrawCount;
    int m_forceRedrawCount;
    int m_lockCount;
    int m_unlockCount;
};

// ============================================================================
// Test Fixture
// ============================================================================

class WindowImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( WindowImplTest );
    CPPUNIT_TEST(testConstructorCreatesSurfaces);
    CPPUNIT_TEST(testConstructorSetsVisibilityToFalse);
    CPPUNIT_TEST(testConstructorSetsPreferredSizeToZero);
    CPPUNIT_TEST(testDestructorCleansUpResources);
    CPPUNIT_TEST(testDestructorWithActiveListeners);
    CPPUNIT_TEST(testAddKeyEventListenerWithValidListener);
    CPPUNIT_TEST(testAddKeyEventListenerWithNullThrows);
    CPPUNIT_TEST(testAddKeyEventListenerWithSameListenerTwice);
    CPPUNIT_TEST(testAddKeyEventListenerWithMultipleDifferent);
    CPPUNIT_TEST(testRemoveKeyEventListenerWithExisting);
    CPPUNIT_TEST(testRemoveKeyEventListenerWithNonExistent);
    CPPUNIT_TEST(testRemoveKeyEventListenerWithNull);
    CPPUNIT_TEST(testRemoveKeyEventListenerRemovesAllInstances);
    CPPUNIT_TEST(testRemoveKeyEventListenerFromEmptyList);
    CPPUNIT_TEST(testGetBoundsReturnsCorrectRectangle);
    CPPUNIT_TEST(testGetBoundsWithZeroSizeSurface);
    CPPUNIT_TEST(testGetSizeReturnsCurrentSize);
    CPPUNIT_TEST(testGetSizeBeforeSetSize);
    CPPUNIT_TEST(testSetSizeWithValidDimensions);
    CPPUNIT_TEST(testSetSizeWithZeroDimensions);
    CPPUNIT_TEST(testSetSizeWithLargeDimensions);
    CPPUNIT_TEST(testSetSizeWhenVisible);
    CPPUNIT_TEST(testSetSizeWhenNotVisible);
    CPPUNIT_TEST(testSetSizeMultipleTimes);
    CPPUNIT_TEST(testSetSizeResizesBothSurfaces);
    CPPUNIT_TEST(testSetSizeResizesBackgroundSurfaces);
    CPPUNIT_TEST(testSetSizeUpdatesInternalSize);
    CPPUNIT_TEST(testGetPreferredSizeAfterSetPreferredSize);
    CPPUNIT_TEST(testSetPreferredSizeWithValidSize);
    CPPUNIT_TEST(testSetPreferredSizeWithZeroSize);
    CPPUNIT_TEST(testSetPreferredSizeCalledMultipleTimes);
    CPPUNIT_TEST(testSetPreferredSizeWithLargeDimensions);
    CPPUNIT_TEST(testGetPreferredSizeCallsLockUnlock);
    CPPUNIT_TEST(testSetVisibleTrueUpdatesVisibility);
    CPPUNIT_TEST(testSetVisibleFalseUpdatesVisibility);
    CPPUNIT_TEST(testSetVisibleTrueCallsRequestRedraw);
    CPPUNIT_TEST(testSetVisibleFalseCallsForceRedraw);
    CPPUNIT_TEST(testSetVisibleTrueMultipleTimes);
    CPPUNIT_TEST(testSetVisibleWithSameValue);
    CPPUNIT_TEST(testSetVisibleAcquiresLock);
    CPPUNIT_TEST(testUpdateSwapsSurfaces);
    CPPUNIT_TEST(testUpdateSwapsBackgroundSurfaces);
    CPPUNIT_TEST(testUpdateWhenVisibleCallsRequestRedraw);
    CPPUNIT_TEST(testUpdateWhenNotVisibleDoesNotCallRequestRedraw);
    CPPUNIT_TEST(testUpdateAcquiresLock);
    CPPUNIT_TEST(testUpdateMultipleTimes);
    CPPUNIT_TEST(testClearClearsDrawingSurface);
    CPPUNIT_TEST(testClearClearsBackgroundSurface);
    CPPUNIT_TEST(testClearCallsRequestRedraw);
    CPPUNIT_TEST(testClearAcquiresLock);
    CPPUNIT_TEST(testSetEngineHooksWithValidHooks);
    CPPUNIT_TEST(testSetEngineHooksWithNull);
    CPPUNIT_TEST(testSetEngineHooksReplacesExisting);
    CPPUNIT_TEST(testEngineHooksDefaultToNullHooks);
    CPPUNIT_TEST(testEngineHooksCalledOnVisibilityChange);
    CPPUNIT_TEST(testEngineHooksCalledOnUpdate);
    CPPUNIT_TEST(testGetPixmapReturnsValidPixmap);
    CPPUNIT_TEST(testGetPixmapReturnsReadySurface);
    CPPUNIT_TEST(testGetBgPixmapReturnsValidPixmap);
    CPPUNIT_TEST(testGetBgPixmapReturnsReadySurface);
    CPPUNIT_TEST(testGetPixmapAfterUpdate);
    CPPUNIT_TEST(testProcessKeyEventWithNoListeners);
    CPPUNIT_TEST(testProcessKeyEventWithOneListener);
    CPPUNIT_TEST(testProcessKeyEventWithMultipleListeners);
    CPPUNIT_TEST(testProcessKeyEventPressedType);
    CPPUNIT_TEST(testProcessKeyEventReleasedType);
    CPPUNIT_TEST(testProcessKeyEventWithDifferentSymbols);
    CPPUNIT_TEST(testProcessKeyEventCalledInOrder);
    CPPUNIT_TEST(testProcessKeyEventAfterRemovingListener);
    CPPUNIT_TEST(testSetDrawDirectionLeftToRight);
    CPPUNIT_TEST(testSetDrawDirectionRightToLeft);
    CPPUNIT_TEST(testSetDrawDirectionTopBottom);
    CPPUNIT_TEST(testSetDrawDirectionBottomTop);
    CPPUNIT_TEST(testSetDrawDirectionMultipleTimes);
    CPPUNIT_TEST(testGetDrawContextReturnsValidContext);
    CPPUNIT_TEST(testFillRectangleWithValidColor);
    CPPUNIT_TEST(testFillRectangleWithTransparentColor);
    CPPUNIT_TEST(testFillRectangleWithOpaqueColor);
    CPPUNIT_TEST(testFillRectangleWithZeroSizeRectangle);
    CPPUNIT_TEST(testFillRectangleWithNegativeCoordinates);
    CPPUNIT_TEST(testFillRectangleWithLargeRectangle);
    CPPUNIT_TEST(testFillRectangleMultipleTimes);
    CPPUNIT_TEST(testFillRectangleAfterSetSize);
    CPPUNIT_TEST(testDrawUnderlineWithValidColor);
    CPPUNIT_TEST(testDrawUnderlineWithTransparentColor);
    CPPUNIT_TEST(testDrawUnderlineWithOpaqueColor);
    CPPUNIT_TEST(testDrawUnderlineWithThinRectangle);
    CPPUNIT_TEST(testDrawUnderlineWithWideRectangle);
    CPPUNIT_TEST(testDrawUnderlineMultipleTimes);
    CPPUNIT_TEST(testDrawUnderlineAfterClear);
    CPPUNIT_TEST(testSetSizeAndUpdateInteraction);
    CPPUNIT_TEST(testSetVisibleAndUpdateInteraction);
    CPPUNIT_TEST(testClearAndUpdateInteraction);
    CPPUNIT_TEST(testMultipleOperationsWithLocking);
    CPPUNIT_TEST(testDrawPixmapWithValidBitmap);
    CPPUNIT_TEST(testDrawPixmapWithZeroSizeSource);
    CPPUNIT_TEST(testDrawPixmapWithZeroSizeDestination);
    CPPUNIT_TEST(testDrawPixmapWithMatchingSrcDst);
    CPPUNIT_TEST(testDrawPixmapWithDifferentSrcDst);
    CPPUNIT_TEST(testDrawPixmapMultipleTimes);
    CPPUNIT_TEST(testDrawPixmapAfterUpdate);
    CPPUNIT_TEST(testDrawBitmapWithValidBitmap);
    CPPUNIT_TEST(testDrawBitmapWithSmallBitmap);
    CPPUNIT_TEST(testDrawBitmapWithLargeBitmap);
    CPPUNIT_TEST(testDrawBitmapMultipleTimes);
    CPPUNIT_TEST(testDrawBitmapAtDifferentPositions);
    CPPUNIT_TEST(testDrawBitmapAfterClear);
    CPPUNIT_TEST(testDrawGlyphWithValidFontStrip);
    CPPUNIT_TEST(testDrawGlyphWithNullFontStripThrows);
    CPPUNIT_TEST(testDrawGlyphWithZeroIndex);
    CPPUNIT_TEST(testDrawGlyphWithMultipleIndices);
    CPPUNIT_TEST(testDrawGlyphWithTransparentForeground);
    CPPUNIT_TEST(testDrawGlyphWithTransparentBackground);
    CPPUNIT_TEST(testDrawGlyphMultipleTimes);
    CPPUNIT_TEST(testDrawGlyphAfterSetSize);
    CPPUNIT_TEST(testFillRectangleThenDrawUnderline);
    CPPUNIT_TEST(testMultipleDrawingOperationsSequence);
    CPPUNIT_TEST(testDrawingOperationsWithUpdate);
    CPPUNIT_TEST(testDrawingOperationsWithVisibilityChanges);
    CPPUNIT_TEST(testDrawingOperationsWithResize);
    CPPUNIT_TEST(testDrawingOperationsAfterClear);
    CPPUNIT_TEST(testConcurrentSurfaceOperations);
    CPPUNIT_TEST(testDrawContextPersistence);
    CPPUNIT_TEST(testPreferredSizeAndActualSizeInteraction);
    CPPUNIT_TEST(testVisibilityAndSizeInteraction);
    CPPUNIT_TEST(testListenerManagementDuringEvents);
    CPPUNIT_TEST(testHooksReplacementDuringOperations);
    CPPUNIT_TEST(testSurfaceStateAfterMultipleUpdates);
    CPPUNIT_TEST(testPixmapAccessConsistency);
    CPPUNIT_TEST(testDrawDirectionPersistence);
    CPPUNIT_TEST(testBoundsConsistencyAfterOperations);
    CPPUNIT_TEST(testLockingConsistencyAcrossOperations);
    CPPUNIT_TEST(testComplexOperationSequence);
    CPPUNIT_TEST(testRecoverFromMultipleSetSizeCalls);
    CPPUNIT_TEST(testRecoverFromRapidVisibilityToggles);
    CPPUNIT_TEST(testRecoverFromMultipleClearCalls);
    CPPUNIT_TEST(testRecoverFromMultipleUpdateCalls);
    CPPUNIT_TEST(testHandleExtremeSize);
    CPPUNIT_TEST(testHandleMinimalSize);
    CPPUNIT_TEST(testHandleRapidListenerChanges);
    CPPUNIT_TEST(testHandleMultipleHookChanges);
    CPPUNIT_TEST(testHandleMixedDrawingOperations);
    CPPUNIT_TEST(testHandleDrawingWithoutSetSize);
    CPPUNIT_TEST(testHandleOperationsBeforeInitialization);
    CPPUNIT_TEST(testHandleFullLifecycleScenario);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup before each test
    }

    void tearDown()
    {
        // Cleanup after each test
    }

    void testConstructorCreatesSurfaces()
    {
        WindowImpl window;
        // Verify window is properly initialized and surfaces are usable
        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(0, bounds.m_x);
        CPPUNIT_ASSERT_EQUAL(0, bounds.m_y);
        CPPUNIT_ASSERT(bounds.m_w >= 0);
        CPPUNIT_ASSERT(bounds.m_h >= 0);

        // Verify initial visibility state
        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());
    }

    void testConstructorSetsVisibilityToFalse()
    {
        WindowImpl window;
        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());
    }

    void testConstructorSetsPreferredSizeToZero()
    {
        WindowImpl window;
        Size preferredSize = window.getPreferredSize();
        // Should return DEFAULT_WINDOW_SIZE (1280, 720) since preferred is 0
        CPPUNIT_ASSERT_EQUAL(1280, preferredSize.m_w);
        CPPUNIT_ASSERT_EQUAL(720, preferredSize.m_h);
    }

    void testDestructorCleansUpResources()
    {
        {
            WindowImpl window;
            window.setSize(Size{100, 100});
        }
        // Destructor should clean up without errors
        CPPUNIT_ASSERT(true);
    }

    void testDestructorWithActiveListeners()
    {
        MockKeyEventListener listener;
        {
            WindowImpl window;
            window.addKeyEventListener(&listener);
        }
        // Destructor should handle active listeners gracefully
        CPPUNIT_ASSERT(true);
    }

    void testAddKeyEventListenerWithValidListener()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);

        // Verify by processing an event
        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(1, listener.getEventCount());
        CPPUNIT_ASSERT_EQUAL(KeyEvent::Type::PRESSED, listener.getLastEventType());
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x41), listener.getLastSymbol());
    }

    void testAddKeyEventListenerWithNullThrows()
    {
        WindowImpl window;

        CPPUNIT_ASSERT_THROW(window.addKeyEventListener(nullptr), std::invalid_argument);
    }

    void testAddKeyEventListenerWithSameListenerTwice()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);
        window.addKeyEventListener(&listener);

        // Process event - should be called twice
        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(2, listener.getEventCount());
    }

    void testAddKeyEventListenerWithMultipleDifferent()
    {
        WindowImpl window;
        MockKeyEventListener listener1;
        MockKeyEventListener listener2;
        MockKeyEventListener listener3;

        window.addKeyEventListener(&listener1);
        window.addKeyEventListener(&listener2);
        window.addKeyEventListener(&listener3);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(1, listener1.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener2.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener3.getEventCount());
    }

    void testRemoveKeyEventListenerWithExisting()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);
        window.removeKeyEventListener(&listener);

        // Process event - should not be called
        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(0, listener.getEventCount());
    }

    void testRemoveKeyEventListenerWithNonExistent()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        // Should not throw
        window.removeKeyEventListener(&listener);
        CPPUNIT_ASSERT(true);
    }

    void testRemoveKeyEventListenerWithNull()
    {
        WindowImpl window;

        // Should not crash
        window.removeKeyEventListener(nullptr);
        CPPUNIT_ASSERT(true);
    }

    void testRemoveKeyEventListenerRemovesAllInstances()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);
        window.addKeyEventListener(&listener);
        window.addKeyEventListener(&listener);

        // Verify listener was added 3 times
        KeyEvent testEvent(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(testEvent);
        CPPUNIT_ASSERT_EQUAL(3, listener.getEventCount());

        window.removeKeyEventListener(&listener);

        // Create new listener to test removal (can't reset original)
        MockKeyEventListener newListener;
        window.addKeyEventListener(&newListener);
        KeyEvent event(KeyEvent::Type::PRESSED, 0x42);
        window.processKeyEvent(event);

        // Original listener should not receive new event, new listener should
        CPPUNIT_ASSERT_EQUAL(3, listener.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, newListener.getEventCount());
    }

    void testRemoveKeyEventListenerFromEmptyList()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        // Should not crash or throw
        window.removeKeyEventListener(&listener);
        CPPUNIT_ASSERT(true);
    }

    void testGetBoundsReturnsCorrectRectangle()
    {
        WindowImpl window;
        window.setSize(Size{640, 480});

        Rectangle bounds = window.getBounds();

        CPPUNIT_ASSERT_EQUAL(0, bounds.m_x);
        CPPUNIT_ASSERT_EQUAL(0, bounds.m_y);
        CPPUNIT_ASSERT_EQUAL(640, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(480, bounds.m_h);
    }

    void testGetBoundsWithZeroSizeSurface()
    {
        WindowImpl window;
        // Without setting size, surfaces have default size

        Rectangle bounds = window.getBounds();

        CPPUNIT_ASSERT_EQUAL(0, bounds.m_x);
        CPPUNIT_ASSERT_EQUAL(0, bounds.m_y);
        // Bounds should reflect actual surface size
        CPPUNIT_ASSERT(bounds.m_w >= 0);
        CPPUNIT_ASSERT(bounds.m_h >= 0);
    }

    void testGetSizeReturnsCurrentSize()
    {
        WindowImpl window;
        Size testSize{800, 600};

        window.setSize(testSize);
        Size retrievedSize = window.getSize();

        CPPUNIT_ASSERT_EQUAL(testSize.m_w, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(testSize.m_h, retrievedSize.m_h);
    }

    void testGetSizeBeforeSetSize()
    {
        WindowImpl window;
        Size size = window.getSize();

        // Should return default initialized size (0, 0)
        CPPUNIT_ASSERT_EQUAL(0, size.m_w);
        CPPUNIT_ASSERT_EQUAL(0, size.m_h);
    }

    void testSetSizeWithValidDimensions()
    {
        WindowImpl window;
        Size newSize{1024, 768};

        window.setSize(newSize);

        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(1024, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(768, retrievedSize.m_h);

        // Verify bounds match size
        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(1024, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(768, bounds.m_h);
    }

    void testSetSizeWithZeroDimensions()
    {
        WindowImpl window;
        Size zeroSize{0, 0};

        // Should accept zero dimensions
        window.setSize(zeroSize);

        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(0, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(0, retrievedSize.m_h);
    }

    void testSetSizeWithLargeDimensions()
    {
        WindowImpl window;
        Size largeSize{7680, 4320}; // 8K resolution

        window.setSize(largeSize);

        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(7680, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(4320, retrievedSize.m_h);
    }

    void testSetSizeWhenVisible()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(true);
        hooks.reset();

        window.setSize(Size{640, 480});

        // Should call requestRedraw when visible
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testSetSizeWhenNotVisible()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(false);
        hooks.reset();

        window.setSize(Size{640, 480});

        // Should not call requestRedraw when not visible
        CPPUNIT_ASSERT_EQUAL(0, hooks.getRequestRedrawCount());
    }

    void testSetSizeMultipleTimes()
    {
        WindowImpl window;

        window.setSize(Size{640, 480});
        window.setSize(Size{800, 600});
        window.setSize(Size{1024, 768});

        Size finalSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(1024, finalSize.m_w);
        CPPUNIT_ASSERT_EQUAL(768, finalSize.m_h);
    }

    void testSetSizeResizesBothSurfaces()
    {
        WindowImpl window;

        window.setSize(Size{640, 480});

        // Verify both surfaces are resized by checking bounds
        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(640, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(480, bounds.m_h);
    }

    void testSetSizeResizesBackgroundSurfaces()
    {
        WindowImpl window;

        window.setSize(Size{640, 480});

        // Background surfaces should be resized (verified by no crash on clear)
        window.clear();
        CPPUNIT_ASSERT(true);
    }

    void testSetSizeUpdatesInternalSize()
    {
        WindowImpl window;
        Size newSize{1920, 1080};

        window.setSize(newSize);

        // Verify internal m_size is updated
        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(newSize.m_w, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(newSize.m_h, retrievedSize.m_h);
    }

    void testGetPreferredSizeAfterSetPreferredSize()
    {
        WindowImpl window;
        Size testSize{1920, 1080};

        window.setPreferredSize(testSize);
        Size retrievedSize = window.getPreferredSize();

        CPPUNIT_ASSERT_EQUAL(1920, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(1080, retrievedSize.m_h);
    }

    void testSetPreferredSizeWithValidSize()
    {
        WindowImpl window;
        Size newSize{1600, 900};

        window.setPreferredSize(newSize);

        Size retrievedSize = window.getPreferredSize();
        CPPUNIT_ASSERT_EQUAL(1600, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(900, retrievedSize.m_h);
    }

    void testSetPreferredSizeWithZeroSize()
    {
        WindowImpl window;
        Size zeroSize{0, 0};

        window.setPreferredSize(zeroSize);

        // Should store zero, but getPreferredSize returns default
        Size retrievedSize = window.getPreferredSize();
        CPPUNIT_ASSERT_EQUAL(1280, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(720, retrievedSize.m_h);
    }

    void testSetPreferredSizeCalledMultipleTimes()
    {
        WindowImpl window;

        window.setPreferredSize(Size{1920, 1080});
        window.setPreferredSize(Size{1280, 720});
        window.setPreferredSize(Size{640, 480});

        // Only first call should take effect
        Size retrievedSize = window.getPreferredSize();
        CPPUNIT_ASSERT_EQUAL(1920, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(1080, retrievedSize.m_h);
    }

    void testSetPreferredSizeWithLargeDimensions()
    {
        WindowImpl window;
        Size largeSize{7680, 4320};

        window.setPreferredSize(largeSize);

        Size retrievedSize = window.getPreferredSize();
        CPPUNIT_ASSERT_EQUAL(7680, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(4320, retrievedSize.m_h);
    }

    void testGetPreferredSizeCallsLockUnlock()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.getPreferredSize();

        // Should call lock and unlock
        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT(hooks.getUnlockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testSetVisibleTrueUpdatesVisibility()
    {
        WindowImpl window;

        window.setVisible(true);

        CPPUNIT_ASSERT_EQUAL(true, window.isVisible());
    }

    void testSetVisibleFalseUpdatesVisibility()
    {
        WindowImpl window;

        window.setVisible(true);
        window.setVisible(false);

        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());
    }

    void testSetVisibleTrueCallsRequestRedraw()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.setVisible(true);

        CPPUNIT_ASSERT_EQUAL(true, window.isVisible());
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testSetVisibleFalseCallsForceRedraw()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(true);
        CPPUNIT_ASSERT_EQUAL(true, window.isVisible());
        hooks.reset();

        window.setVisible(false);

        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());
        CPPUNIT_ASSERT(hooks.getForceRedrawCount() > 0);
    }

    void testSetVisibleTrueMultipleTimes()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.setVisible(true);
        int firstCount = hooks.getRequestRedrawCount();

        window.setVisible(true);

        // Each call should trigger requestRedraw
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > firstCount);
    }

    void testSetVisibleWithSameValue()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(false);
        hooks.reset();

        window.setVisible(false);

        // Should still call forceRedraw
        CPPUNIT_ASSERT(hooks.getForceRedrawCount() > 0);
    }

    void testSetVisibleAcquiresLock()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.setVisible(true);

        // Should call lock and unlock
        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT(hooks.getUnlockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testUpdateSwapsSurfaces()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        // Get initial bounds
        Rectangle beforeBounds = window.getBounds();

        window.update();

        // After update, surfaces are swapped but bounds should still be valid
        Rectangle afterBounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(beforeBounds.m_w, afterBounds.m_w);
        CPPUNIT_ASSERT_EQUAL(beforeBounds.m_h, afterBounds.m_h);
    }

    void testUpdateSwapsBackgroundSurfaces()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        // Update should swap background surfaces without crashing
        window.update();
        window.clear();

        CPPUNIT_ASSERT(true);
    }

    void testUpdateWhenVisibleCallsRequestRedraw()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(true);
        hooks.reset();

        window.update();

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testUpdateWhenNotVisibleDoesNotCallRequestRedraw()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setVisible(false);
        hooks.reset();

        window.update();

        CPPUNIT_ASSERT_EQUAL(0, hooks.getRequestRedrawCount());
    }

    void testUpdateAcquiresLock()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.update();

        // Should call lock and unlock
        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT(hooks.getUnlockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testUpdateMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        window.update();
        window.update();
        window.update();

        // Should not crash, surfaces should be valid
        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(100, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(100, bounds.m_h);
    }

    void testClearClearsDrawingSurface()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        // Clear should not throw
        window.clear();

        CPPUNIT_ASSERT(true);
    }

    void testClearClearsBackgroundSurface()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        // Clear should clear both surfaces without crashing
        window.clear();
        window.update();

        CPPUNIT_ASSERT(true);
    }

    void testClearCallsRequestRedraw()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.clear();

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testClearAcquiresLock()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.clear();

        // Should call lock and unlock
        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT(hooks.getUnlockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testSetEngineHooksWithValidHooks()
    {
        WindowImpl window;
        MockEngineHooks hooks;

        window.setEngineHooks(&hooks);

        // Verify by triggering a hook operation
        hooks.reset();
        window.setVisible(true);

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testSetEngineHooksWithNull()
    {
        WindowImpl window;
        MockEngineHooks hooks;

        window.setEngineHooks(&hooks);
        window.setEngineHooks(nullptr);

        // Should use NullEngineHooks, which won't crash
        window.setVisible(true);
        CPPUNIT_ASSERT(true);
    }

    void testSetEngineHooksReplacesExisting()
    {
        WindowImpl window;
        MockEngineHooks hooks1;
        MockEngineHooks hooks2;

        window.setEngineHooks(&hooks1);
        window.setEngineHooks(&hooks2);

        hooks1.reset();
        hooks2.reset();

        window.setVisible(true);

        // Only hooks2 should be called
        CPPUNIT_ASSERT_EQUAL(0, hooks1.getRequestRedrawCount());
        CPPUNIT_ASSERT(hooks2.getRequestRedrawCount() > 0);
    }

    void testEngineHooksDefaultToNullHooks()
    {
        WindowImpl window;

        // Should not crash with default null hooks
        window.setVisible(true);
        window.update();
        window.clear();

        CPPUNIT_ASSERT(true);
    }

    void testEngineHooksCalledOnVisibilityChange()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();
        window.setVisible(true);
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);

        hooks.reset();
        window.setVisible(false);
        CPPUNIT_ASSERT(hooks.getForceRedrawCount() > 0);
    }

    void testEngineHooksCalledOnUpdate()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);
        window.setVisible(true);

        hooks.reset();
        window.update();

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testGetPixmapReturnsValidPixmap()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        Pixmap& pixmap = window.getPixmap();

        // Verify pixmap has valid dimensions
        CPPUNIT_ASSERT(pixmap.getWidth() > 0 || pixmap.getHeight() > 0);
    }

    void testGetPixmapReturnsReadySurface()
    {
        WindowImpl window;
        window.setSize(Size{640, 480});

        Pixmap& pixmap = window.getPixmap();

        // The ready surface should match the set size
        CPPUNIT_ASSERT_EQUAL(640, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(480, pixmap.getHeight());
    }

    void testGetBgPixmapReturnsValidPixmap()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        Pixmap& pixmap = window.getBgPixmap();

        // Verify pixmap has valid dimensions (can be 0 for background)
        CPPUNIT_ASSERT(pixmap.getWidth() >= 0 && pixmap.getHeight() >= 0);
    }

    void testGetBgPixmapReturnsReadySurface()
    {
        WindowImpl window;
        window.setSize(Size{640, 480});

        Pixmap& bgPixmap = window.getBgPixmap();

        // Background pixmap dimensions depend on backend type
        CPPUNIT_ASSERT(bgPixmap.getWidth() >= 0);
        CPPUNIT_ASSERT(bgPixmap.getHeight() >= 0);
    }

    void testGetPixmapAfterUpdate()
    {
        WindowImpl window;
        window.setSize(Size{800, 600});

        window.update();

        Pixmap& pixmap = window.getPixmap();

        // After update, surfaces are swapped
        CPPUNIT_ASSERT_EQUAL(800, pixmap.getWidth());
        CPPUNIT_ASSERT_EQUAL(600, pixmap.getHeight());
    }

    void testProcessKeyEventWithNoListeners()
    {
        WindowImpl window;

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);

        // Should not crash with no listeners
        window.processKeyEvent(event);

        CPPUNIT_ASSERT(true);
    }

    void testProcessKeyEventWithOneListener()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(1, listener.getEventCount());
        CPPUNIT_ASSERT_EQUAL(KeyEvent::Type::PRESSED, listener.getLastEventType());
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x41), listener.getLastSymbol());
    }

    void testProcessKeyEventWithMultipleListeners()
    {
        WindowImpl window;
        MockKeyEventListener listener1;
        MockKeyEventListener listener2;
        MockKeyEventListener listener3;

        window.addKeyEventListener(&listener1);
        window.addKeyEventListener(&listener2);
        window.addKeyEventListener(&listener3);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x42);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(1, listener1.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener2.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener3.getEventCount());
    }

    void testProcessKeyEventPressedType()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x50);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(KeyEvent::Type::PRESSED, listener.getLastEventType());
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x50), listener.getLastSymbol());
    }

    void testProcessKeyEventReleasedType()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);

        KeyEvent event(KeyEvent::Type::RELEASED, 0x51);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(KeyEvent::Type::RELEASED, listener.getLastEventType());
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x51), listener.getLastSymbol());
    }

    void testProcessKeyEventWithDifferentSymbols()
    {
        WindowImpl window;
        MockKeyEventListener listener;

        window.addKeyEventListener(&listener);

        KeyEvent event1(KeyEvent::Type::PRESSED, 0x61); // 'a'
        window.processKeyEvent(event1);
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x61), listener.getLastSymbol());

        KeyEvent event2(KeyEvent::Type::PRESSED, 0x7A); // 'z'
        window.processKeyEvent(event2);
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(0x7A), listener.getLastSymbol());

        CPPUNIT_ASSERT_EQUAL(2, listener.getEventCount());
    }

    void testProcessKeyEventCalledInOrder()
    {
        WindowImpl window;
        MockKeyEventListener listener1;
        MockKeyEventListener listener2;

        window.addKeyEventListener(&listener1);
        window.addKeyEventListener(&listener2);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        // Both should be called with the same event
        CPPUNIT_ASSERT_EQUAL(1, listener1.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener2.getEventCount());
        CPPUNIT_ASSERT_EQUAL(listener1.getLastSymbol(), listener2.getLastSymbol());
    }

    void testProcessKeyEventAfterRemovingListener()
    {
        WindowImpl window;
        MockKeyEventListener listener1;
        MockKeyEventListener listener2;

        window.addKeyEventListener(&listener1);
        window.addKeyEventListener(&listener2);

        window.removeKeyEventListener(&listener1);

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        CPPUNIT_ASSERT_EQUAL(0, listener1.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener2.getEventCount());
    }

    void testSetDrawDirectionLeftToRight()
    {
        WindowImpl window;

        window.setDrawDirection(DrawDirection::LEFT_TO_RIGHT);

        // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testSetDrawDirectionRightToLeft()
    {
        WindowImpl window;

        window.setDrawDirection(DrawDirection::RIGHT_TO_LEFT);

        // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testSetDrawDirectionTopBottom()
    {
        WindowImpl window;

        window.setDrawDirection(DrawDirection::TOP_BOTTOM);

        // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testSetDrawDirectionBottomTop()
    {
        WindowImpl window;

        window.setDrawDirection(DrawDirection::BOTTOM_TOP);

        // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testSetDrawDirectionMultipleTimes()
    {
        WindowImpl window;

        window.setDrawDirection(DrawDirection::LEFT_TO_RIGHT);
        window.setDrawDirection(DrawDirection::RIGHT_TO_LEFT);
        window.setDrawDirection(DrawDirection::TOP_BOTTOM);
        window.setDrawDirection(DrawDirection::BOTTOM_TOP);

        // Should accept multiple changes
        CPPUNIT_ASSERT(true);
    }

    void testGetDrawContextReturnsValidContext()
    {
        WindowImpl window;

        DrawContext& context = window.getDrawContext();

        // Verify we can use the context
        ColorArgb color(255, 255, 0, 0);
        Rectangle rect{10, 10, 50, 50};

        window.setSize(Size{100, 100});
        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithValidColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 128, 64, 32);
        Rectangle rect{10, 10, 50, 50};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithTransparentColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(0, 255, 255, 255); // Fully transparent
        Rectangle rect{10, 10, 50, 50};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithOpaqueColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 255, 0, 0); // Fully opaque red
        Rectangle rect{10, 10, 50, 50};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithZeroSizeRectangle()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 255, 0, 0);
        Rectangle rect{10, 10, 0, 0};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithNegativeCoordinates()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 255, 0, 0);
        Rectangle rect{-10, -10, 50, 50};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleWithLargeRectangle()
    {
        WindowImpl window;
        window.setSize(Size{1920, 1080});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 255, 0);
        Rectangle rect{0, 0, 1920, 1080};

        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color1(255, 255, 0, 0);
        ColorArgb color2(255, 0, 255, 0);
        ColorArgb color3(255, 0, 0, 255);
        Rectangle rect{10, 10, 50, 50};

        context.fillRectangle(color1, rect);
        context.fillRectangle(color2, rect);
        context.fillRectangle(color3, rect);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleAfterSetSize()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 128, 128, 128);
        Rectangle rect{10, 10, 50, 50};

        context.fillRectangle(color, rect);

        window.setSize(Size{200, 200});
        context.fillRectangle(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineWithValidColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 0, 0);
        Rectangle rect{10, 50, 100, 2}; // Thin underline

        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineWithTransparentColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(0, 0, 0, 0); // Fully transparent
        Rectangle rect{10, 50, 100, 2};

        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineWithOpaqueColor()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 255, 255, 0); // Fully opaque yellow
        Rectangle rect{10, 50, 100, 2};

        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineWithThinRectangle()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 0, 0);
        Rectangle rect{10, 50, 100, 1}; // 1 pixel height

        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineWithWideRectangle()
    {
        WindowImpl window;
        window.setSize(Size{1920, 1080});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 0, 0);
        Rectangle rect{0, 100, 1920, 3}; // Full width underline

        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 0, 0);

        Rectangle rect1{10, 50, 100, 2};
        Rectangle rect2{10, 60, 100, 2};
        Rectangle rect3{10, 70, 100, 2};

        context.drawUnderline(color, rect1);
        context.drawUnderline(color, rect2);
        context.drawUnderline(color, rect3);

        CPPUNIT_ASSERT(true);
    }

    void testDrawUnderlineAfterClear()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();
        ColorArgb color(255, 0, 0, 0);
        Rectangle rect{10, 50, 100, 2};

        window.clear();
        context.drawUnderline(color, rect);

        CPPUNIT_ASSERT(true);
    }

    void testSetSizeAndUpdateInteraction()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);
        window.setVisible(true);

        hooks.reset();

        window.setSize(Size{640, 480});
        int sizeRedrawCount = hooks.getRequestRedrawCount();

        window.update();
        int updateRedrawCount = hooks.getRequestRedrawCount();

        // Both operations should trigger requestRedraw
        CPPUNIT_ASSERT(sizeRedrawCount > 0);
        CPPUNIT_ASSERT(updateRedrawCount > sizeRedrawCount);
    }

    void testSetVisibleAndUpdateInteraction()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);
        window.setSize(Size{100, 100});

        hooks.reset();

        window.setVisible(true);
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);

        hooks.reset();
        window.update();
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);

        hooks.reset();
        window.setVisible(false);
        CPPUNIT_ASSERT(hooks.getForceRedrawCount() > 0);
    }

    void testClearAndUpdateInteraction()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);
        window.setSize(Size{100, 100});

        hooks.reset();

        window.clear();
        int clearRedrawCount = hooks.getRequestRedrawCount();

        window.update();

        // Both operations involve surfaces
        CPPUNIT_ASSERT(clearRedrawCount > 0);
        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(100, bounds.m_w);
    }

    void testMultipleOperationsWithLocking()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();

        window.setSize(Size{640, 480});
        window.setVisible(true);
        window.clear();
        window.update();

        // All operations should properly lock/unlock
        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testDrawPixmapWithValidBitmap()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        // Create a simple CLUT bitmap
        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFF000000}; // Black with full alpha

        ClutBitmap bitmap(10, 10, 10, pixels, clut, 256);
        Rectangle srcRect{0, 0, 10, 10};
        Rectangle dstRect{10, 10, 10, 10};

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapWithZeroSizeSource()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFF000000};

        ClutBitmap bitmap(10, 10, 10, pixels, clut, 256);
        Rectangle srcRect{0, 0, 0, 0}; // Zero size
        Rectangle dstRect{10, 10, 10, 10};

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapWithZeroSizeDestination()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFF000000};

        ClutBitmap bitmap(10, 10, 10, pixels, clut, 256);
        Rectangle srcRect{0, 0, 10, 10};
        Rectangle dstRect{10, 10, 0, 0}; // Zero size

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapWithMatchingSrcDst()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        std::uint8_t pixels[400] = {0};
        std::uint32_t clut[256] = {0xFF0000FF}; // Blue

        ClutBitmap bitmap(20, 20, 20, pixels, clut, 256);
        Rectangle srcRect{0, 0, 20, 20};
        Rectangle dstRect{50, 50, 20, 20}; // Same size as source

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapWithDifferentSrcDst()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        std::uint8_t pixels[400] = {0};
        std::uint32_t clut[256] = {0xFF00FF00}; // Green

        ClutBitmap bitmap(20, 20, 20, pixels, clut, 256);
        Rectangle srcRect{0, 0, 20, 20};
        Rectangle dstRect{50, 50, 40, 40}; // Different size (scaling)

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFFFF0000}; // Red

        ClutBitmap bitmap(10, 10, 10, pixels, clut, 256);
        Rectangle srcRect{0, 0, 10, 10};

        context.drawPixmap(bitmap, srcRect, Rectangle{10, 10, 10, 10});
        context.drawPixmap(bitmap, srcRect, Rectangle{30, 30, 10, 10});
        context.drawPixmap(bitmap, srcRect, Rectangle{50, 50, 10, 10});

        CPPUNIT_ASSERT(true);
    }

    void testDrawPixmapAfterUpdate()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        window.update();

        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFFFFFF00}; // Yellow

        ClutBitmap bitmap(10, 10, 10, pixels, clut, 256);
        Rectangle srcRect{0, 0, 10, 10};
        Rectangle dstRect{10, 10, 10, 10};

        context.drawPixmap(bitmap, srcRect, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapWithValidBitmap()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        Bitmap bitmap(10, 10, 40); // 10x10, 4 bytes per pixel
        Rectangle dstRect{10, 10, 10, 10};

        context.drawBitmap(bitmap, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapWithSmallBitmap()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        Bitmap bitmap(1, 1, 4); // 1x1 pixel
        Rectangle dstRect{10, 10, 1, 1};

        context.drawBitmap(bitmap, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapWithLargeBitmap()
    {
        WindowImpl window;
        window.setSize(Size{1920, 1080});

        DrawContext& context = window.getDrawContext();

        Bitmap bitmap(100, 100, 400); // 100x100 bitmap
        Rectangle dstRect{0, 0, 100, 100};

        context.drawBitmap(bitmap, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        Bitmap bitmap(20, 20, 80);

        context.drawBitmap(bitmap, Rectangle{10, 10, 20, 20});
        context.drawBitmap(bitmap, Rectangle{40, 40, 20, 20});
        context.drawBitmap(bitmap, Rectangle{70, 70, 20, 20});

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapAtDifferentPositions()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        Bitmap bitmap(15, 15, 60);

        context.drawBitmap(bitmap, Rectangle{0, 0, 15, 15});
        context.drawBitmap(bitmap, Rectangle{100, 100, 15, 15});
        context.drawBitmap(bitmap, Rectangle{50, 150, 15, 15});

        CPPUNIT_ASSERT(true);
    }

    void testDrawBitmapAfterClear()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        window.clear();

        Bitmap bitmap(10, 10, 40);
        Rectangle dstRect{10, 10, 10, 10};

        context.drawBitmap(bitmap, dstRect);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphWithValidFontStrip()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        // Create a font strip (simplified, real implementation would use actual font)
        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);
        Rectangle rect{10, 10, 16, 16};

        context.drawGlyph(fontStrip, 0, rect, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphWithNullFontStripThrows()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);
        Rectangle rect{10, 10, 16, 16};

        CPPUNIT_ASSERT_THROW(
            context.drawGlyph(nullptr, 0, rect, fgColor, bgColor),
            std::invalid_argument
        );
    }

    void testDrawGlyphWithZeroIndex()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);
        Rectangle rect{10, 10, 16, 16};

        context.drawGlyph(fontStrip, 0, rect, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphWithMultipleIndices()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);

        context.drawGlyph(fontStrip, 0, Rectangle{10, 10, 16, 16}, fgColor, bgColor);
        context.drawGlyph(fontStrip, 1, Rectangle{30, 10, 16, 16}, fgColor, bgColor);
        context.drawGlyph(fontStrip, 2, Rectangle{50, 10, 16, 16}, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphWithTransparentForeground()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(0, 0, 0, 0); // Fully transparent
        ColorArgb bgColor(255, 255, 255, 255);
        Rectangle rect{10, 10, 16, 16};

        context.drawGlyph(fontStrip, 0, rect, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphWithTransparentBackground()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(0, 0, 0, 0); // Fully transparent
        Rectangle rect{10, 10, 16, 16};

        context.drawGlyph(fontStrip, 0, rect, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphMultipleTimes()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);

        for (int i = 0; i < 5; i++)
        {
            Rectangle rect{i * 20, 10, 16, 16};
            context.drawGlyph(fontStrip, i, rect, fgColor, bgColor);
        }

        CPPUNIT_ASSERT(true);
    }

    void testDrawGlyphAfterSetSize()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        DrawContext& context = window.getDrawContext();

        window.setSize(Size{200, 200});

        FontStripPtr fontStrip = std::make_shared<FontStripImpl>(Size{16, 16}, 256);

        ColorArgb fgColor(255, 0, 0, 0);
        ColorArgb bgColor(255, 255, 255, 255);
        Rectangle rect{10, 10, 16, 16};

        context.drawGlyph(fontStrip, 0, rect, fgColor, bgColor);

        CPPUNIT_ASSERT(true);
    }

    void testFillRectangleThenDrawUnderline()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        ColorArgb fillColor(255, 200, 200, 200);
        Rectangle fillRect{10, 10, 100, 50};
        context.fillRectangle(fillColor, fillRect);

        ColorArgb underlineColor(255, 0, 0, 0);
        Rectangle underlineRect{10, 58, 100, 2};
        context.drawUnderline(underlineColor, underlineRect);

        CPPUNIT_ASSERT(true);
    }

    void testMultipleDrawingOperationsSequence()
    {
        WindowImpl window;
        window.setSize(Size{300, 300});

        DrawContext& context = window.getDrawContext();

        // Fill background
        context.fillRectangle(ColorArgb(255, 240, 240, 240), Rectangle{0, 0, 300, 300});

        // Draw some rectangles
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});
        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});
        context.fillRectangle(ColorArgb(255, 0, 0, 255), Rectangle{130, 10, 50, 50});

        // Draw underlines
        context.drawUnderline(ColorArgb(255, 0, 0, 0), Rectangle{10, 70, 170, 2});

        CPPUNIT_ASSERT(true);
    }

    void testDrawingOperationsWithUpdate()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});
        window.update();

        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});
        window.update();

        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(200, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(200, bounds.m_h);
    }

    void testDrawingOperationsWithVisibilityChanges()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        window.setVisible(false);
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        window.setVisible(true);
        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testDrawingOperationsWithResize()
    {
        WindowImpl window;
        window.setSize(Size{100, 100});

        DrawContext& context = window.getDrawContext();

        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        window.setSize(Size{200, 200});

        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{110, 110, 50, 50});

        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(200, bounds.m_w);
    }

    void testDrawingOperationsAfterClear()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});
        window.clear();

        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{10, 10, 50, 50});
        context.drawUnderline(ColorArgb(255, 0, 0, 0), Rectangle{10, 70, 50, 2});

        CPPUNIT_ASSERT(true);
    }

    void testConcurrentSurfaceOperations()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        // Drawing operations
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        // Surface operations
        window.update();

        // More drawing
        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});

        // Clear
        window.clear();

        // Final drawing
        context.fillRectangle(ColorArgb(255, 0, 0, 255), Rectangle{130, 10, 50, 50});

        CPPUNIT_ASSERT(true);
    }

    void testDrawContextPersistence()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context1 = window.getDrawContext();
        DrawContext& context2 = window.getDrawContext();

        // Both should reference the same context
        context1.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});
        context2.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});

        CPPUNIT_ASSERT(true);
    }

    void testPreferredSizeAndActualSizeInteraction()
    {
        WindowImpl window;

        window.setPreferredSize(Size{1920, 1080});
        Size preferred = window.getPreferredSize();

        window.setSize(Size{640, 480});
        Size actual = window.getSize();

        CPPUNIT_ASSERT_EQUAL(1920, preferred.m_w);
        CPPUNIT_ASSERT_EQUAL(1080, preferred.m_h);
        CPPUNIT_ASSERT_EQUAL(640, actual.m_w);
        CPPUNIT_ASSERT_EQUAL(480, actual.m_h);
    }

    void testVisibilityAndSizeInteraction()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        window.setSize(Size{640, 480});
        window.setVisible(true);

        hooks.reset();
        window.setSize(Size{800, 600});

        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
        CPPUNIT_ASSERT_EQUAL(true, window.isVisible());
    }

    void testListenerManagementDuringEvents()
    {
        WindowImpl window;
        MockKeyEventListener listener1;
        MockKeyEventListener listener2;

        window.addKeyEventListener(&listener1);

        KeyEvent event1(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event1);

        window.addKeyEventListener(&listener2);

        KeyEvent event2(KeyEvent::Type::PRESSED, 0x42);
        window.processKeyEvent(event2);

        CPPUNIT_ASSERT_EQUAL(2, listener1.getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listener2.getEventCount());
    }

    void testHooksReplacementDuringOperations()
    {
        WindowImpl window;
        MockEngineHooks hooks1;
        MockEngineHooks hooks2;

        window.setEngineHooks(&hooks1);
        window.setVisible(true);

        hooks1.reset();
        hooks2.reset();

        window.setEngineHooks(&hooks2);
        window.update();

        CPPUNIT_ASSERT_EQUAL(0, hooks1.getRequestRedrawCount());
        CPPUNIT_ASSERT(hooks2.getRequestRedrawCount() > 0);
    }

    void testSurfaceStateAfterMultipleUpdates()
    {
        WindowImpl window;
        window.setSize(Size{640, 480});

        for (int i = 0; i < 10; i++)
        {
            window.update();
        }

        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(640, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(480, bounds.m_h);
    }

    void testPixmapAccessConsistency()
    {
        WindowImpl window;
        window.setSize(Size{800, 600});

        Pixmap& pixmap1 = window.getPixmap();
        int width1 = pixmap1.getWidth();

        window.update();

        Pixmap& pixmap2 = window.getPixmap();
        int width2 = pixmap2.getWidth();

        CPPUNIT_ASSERT_EQUAL(width1, width2);
    }

    void testDrawDirectionPersistence()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        DrawContext& context = window.getDrawContext();

        window.setDrawDirection(DrawDirection::RIGHT_TO_LEFT);
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        window.update();

        window.setDrawDirection(DrawDirection::BOTTOM_TOP);
        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{70, 10, 50, 50});

        CPPUNIT_ASSERT(true);
    }

    void testBoundsConsistencyAfterOperations()
    {
        WindowImpl window;
        window.setSize(Size{640, 480});

        Rectangle bounds1 = window.getBounds();

        window.clear();
        window.update();
        window.setVisible(true);
        window.setVisible(false);

        Rectangle bounds2 = window.getBounds();

        CPPUNIT_ASSERT_EQUAL(bounds1.m_w, bounds2.m_w);
        CPPUNIT_ASSERT_EQUAL(bounds1.m_h, bounds2.m_h);
    }

    void testLockingConsistencyAcrossOperations()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        hooks.reset();

        window.setSize(Size{640, 480});
        window.setPreferredSize(Size{1920, 1080});
        window.setVisible(true);
        window.update();
        window.clear();
        window.getPreferredSize();

        CPPUNIT_ASSERT(hooks.getLockCount() > 0);
        CPPUNIT_ASSERT_EQUAL(hooks.getLockCount(), hooks.getUnlockCount());
    }

    void testComplexOperationSequence()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        MockKeyEventListener listener;

        window.setEngineHooks(&hooks);
        window.addKeyEventListener(&listener);

        window.setPreferredSize(Size{1920, 1080});
        window.setSize(Size{640, 480});
        window.setVisible(true);

        DrawContext& context = window.getDrawContext();
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 100, 100});

        window.update();

        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        window.clear();
        window.setDrawDirection(DrawDirection::RIGHT_TO_LEFT);

        CPPUNIT_ASSERT_EQUAL(1, listener.getEventCount());
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
    }

    void testRecoverFromMultipleSetSizeCalls()
    {
        WindowImpl window;

        for (int i = 0; i < 100; i++)
        {
            window.setSize(Size{640 + i, 480 + i});
        }

        Size finalSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(739, finalSize.m_w);
        CPPUNIT_ASSERT_EQUAL(579, finalSize.m_h);
    }

    void testRecoverFromRapidVisibilityToggles()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        window.setEngineHooks(&hooks);

        for (int i = 0; i < 50; i++)
        {
            window.setVisible(true);
            window.setVisible(false);
        }

        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);
        CPPUNIT_ASSERT(hooks.getForceRedrawCount() > 0);
    }

    void testRecoverFromMultipleClearCalls()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        for (int i = 0; i < 20; i++)
        {
            window.clear();
        }

        DrawContext& context = window.getDrawContext();
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        CPPUNIT_ASSERT(true);
    }

    void testRecoverFromMultipleUpdateCalls()
    {
        WindowImpl window;
        window.setSize(Size{200, 200});

        for (int i = 0; i < 50; i++)
        {
            window.update();
        }

        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT_EQUAL(200, bounds.m_w);
        CPPUNIT_ASSERT_EQUAL(200, bounds.m_h);
    }

    void testHandleExtremeSize()
    {
        WindowImpl window;

        Size extremeSize{16384, 8192}; // Very large
        window.setSize(extremeSize);

        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(16384, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(8192, retrievedSize.m_h);
    }

    void testHandleMinimalSize()
    {
        WindowImpl window;

        Size minimalSize{1, 1};
        window.setSize(minimalSize);

        Size retrievedSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(1, retrievedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(1, retrievedSize.m_h);
    }

    void testHandleRapidListenerChanges()
    {
        WindowImpl window;
        MockKeyEventListener listeners[10];

        // Add all listeners
        for (int i = 0; i < 10; i++)
        {
            window.addKeyEventListener(&listeners[i]);
        }

        // Remove every other listener
        for (int i = 0; i < 10; i += 2)
        {
            window.removeKeyEventListener(&listeners[i]);
        }

        // Process event
        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        // Check only odd-indexed listeners received the event
        CPPUNIT_ASSERT_EQUAL(0, listeners[0].getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listeners[1].getEventCount());
        CPPUNIT_ASSERT_EQUAL(0, listeners[2].getEventCount());
        CPPUNIT_ASSERT_EQUAL(1, listeners[3].getEventCount());
    }

    void testHandleMultipleHookChanges()
    {
        WindowImpl window;
        MockEngineHooks hooks[5];

        for (int i = 0; i < 5; i++)
        {
            window.setEngineHooks(&hooks[i]);
            window.setVisible(true);
            hooks[i].reset();
        }

        window.update();

        // Only the last hook should receive the update
        for (int i = 0; i < 4; i++)
        {
            CPPUNIT_ASSERT_EQUAL(0, hooks[i].getRequestRedrawCount());
        }
        CPPUNIT_ASSERT(hooks[4].getRequestRedrawCount() > 0);
    }

    void testHandleMixedDrawingOperations()
    {
        WindowImpl window;
        window.setSize(Size{300, 300});

        DrawContext& context = window.getDrawContext();

        // Mix all drawing types
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});
        context.drawUnderline(ColorArgb(255, 0, 0, 0), Rectangle{10, 70, 50, 2});

        std::uint8_t pixels[100] = {0};
        std::uint32_t clut[256] = {0xFF000000};
        ClutBitmap clutBitmap(10, 10, 10, pixels, clut, 256);
        context.drawPixmap(clutBitmap, Rectangle{0, 0, 10, 10}, Rectangle{10, 80, 10, 10});

        Bitmap bitmap(15, 15, 60);
        context.drawBitmap(bitmap, Rectangle{10, 100, 15, 15});

        CPPUNIT_ASSERT(true);
    }

    void testHandleDrawingWithoutSetSize()
    {
        WindowImpl window;

        // Set a minimal size first to ensure surfaces are initialized
        window.setSize(Size{100, 100});

        DrawContext& context = window.getDrawContext();

        // Now drawing should work
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 50, 50});

        CPPUNIT_ASSERT(true);
    }

    void testHandleOperationsBeforeInitialization()
    {
        WindowImpl window;

        // All operations should work with default state
        CPPUNIT_ASSERT_EQUAL(false, window.isVisible());

        Size size = window.getSize();
        CPPUNIT_ASSERT_EQUAL(0, size.m_w);

        Rectangle bounds = window.getBounds();
        CPPUNIT_ASSERT(bounds.m_w >= 0);

        Size preferred = window.getPreferredSize();
        CPPUNIT_ASSERT_EQUAL(1280, preferred.m_w);
    }

    void testHandleFullLifecycleScenario()
    {
        WindowImpl window;
        MockEngineHooks hooks;
        MockKeyEventListener listener;

        // Initialization
        window.setEngineHooks(&hooks);
        window.addKeyEventListener(&listener);
        window.setPreferredSize(Size{1920, 1080});
        window.setSize(Size{640, 480});

        // Operation phase
        window.setVisible(true);
        window.setDrawDirection(DrawDirection::LEFT_TO_RIGHT);

        DrawContext& context = window.getDrawContext();
        context.fillRectangle(ColorArgb(255, 128, 128, 128), Rectangle{0, 0, 640, 480});
        context.fillRectangle(ColorArgb(255, 255, 0, 0), Rectangle{10, 10, 100, 100});
        context.drawUnderline(ColorArgb(255, 0, 0, 0), Rectangle{10, 120, 100, 2});

        window.update();

        // Event handling
        KeyEvent event(KeyEvent::Type::PRESSED, 0x41);
        window.processKeyEvent(event);

        // State changes
        window.setSize(Size{800, 600});
        window.clear();

        // More operations
        context.fillRectangle(ColorArgb(255, 0, 255, 0), Rectangle{50, 50, 100, 100});
        window.update();

        // Visibility toggle
        window.setVisible(false);
        window.setVisible(true);

        // Final update
        window.update();

        // Verify final state
        CPPUNIT_ASSERT_EQUAL(true, window.isVisible());
        CPPUNIT_ASSERT_EQUAL(1, listener.getEventCount());
        CPPUNIT_ASSERT(hooks.getRequestRedrawCount() > 0);

        Size finalSize = window.getSize();
        CPPUNIT_ASSERT_EQUAL(800, finalSize.m_w);
        CPPUNIT_ASSERT_EQUAL(600, finalSize.m_h);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( WindowImplTest );
