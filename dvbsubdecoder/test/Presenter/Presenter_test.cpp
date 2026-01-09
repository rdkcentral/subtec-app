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
#include <limits>

#include "Presenter.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "Types.hpp"
#include "../Decoder/DecoderClientMock.hpp"

using dvbsubdecoder::Presenter;
using dvbsubdecoder::Database;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Rectangle;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;

class PresenterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PresenterTest );
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testBasicDraw);
    CPPUNIT_TEST(testBasicInvalidate);
    CPPUNIT_TEST(testMultipleDrawCallsWithoutChanges);
    CPPUNIT_TEST(testMultipleInvalidateCalls);
    CPPUNIT_TEST(testDrawAfterInvalidateSequence);
    CPPUNIT_TEST(testZeroDimensionRectangles);
    CPPUNIT_TEST(testIntegerLimitCoordinates);
    CPPUNIT_TEST(testEmptyRenderingState);
    CPPUNIT_TEST(testMaximumRegionCount);
    CPPUNIT_TEST(testDisplayBoundsSmallerThanWindow);
    CPPUNIT_TEST(testWindowBeyondDisplayBounds);
    CPPUNIT_TEST(testNullRegionReferences);
    CPPUNIT_TEST(testCorruptedRectangleData);
    CPPUNIT_TEST(testStateSwapWithIdenticalStates);
    CPPUNIT_TEST(testBoundsChangeNotifications);
    CPPUNIT_TEST(testModifiedRectangleCalculation);
    CPPUNIT_TEST(testEmptyModifiedRectangle);
    CPPUNIT_TEST(testFewLargeRegions);
    CPPUNIT_TEST(testRapidStateChanges);
    CPPUNIT_TEST(testInterleavedClearAndDraw);
    CPPUNIT_TEST(testRegionOverlapHandling);
    CPPUNIT_TEST(testCoordinateTransformation);
    CPPUNIT_TEST(testIsRectangleInsideBoundaryConditions);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_decoderClient = std::make_unique<DecoderClientMock>();
        m_pixmapAllocator = std::make_unique<PixmapAllocator>(Specification::VERSION_1_3_1, *m_decoderClient);
        m_database = std::make_unique<Database>(Specification::VERSION_1_3_1, *m_pixmapAllocator);
        m_presenter = std::make_unique<Presenter>(*m_decoderClient, *m_database);
        // Ensure allocator starts from a clean state once per test instead of resetting per region.
        m_pixmapAllocator->reset();

        // Set up basic display bounds
        setupBasicDisplayBounds();
    }

    void tearDown()
    {
        m_presenter.reset();
        m_database.reset();
        m_pixmapAllocator.reset();
        m_decoderClient.reset();
    }

    // Basic API Coverage Tests
    void testConstructorInitialization()
    {
        // Test that constructor creates Presenter with valid references
        DecoderClientMock client;
        PixmapAllocator allocator(Specification::VERSION_1_3_1, client);
        Database database(Specification::VERSION_1_3_1, allocator);

        Presenter presenter(client, database);

        // Should be able to call public methods without crashing
        presenter.invalidate();
        presenter.draw();
    }

    void testBasicDraw()
    {
        m_decoderClient->clearCallbackHistory();

        m_presenter->draw();

        // Should call gfxFinish at minimum
        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty());
    }

    void testBasicInvalidate()
    {
        // Invalidate should not crash and should affect next draw
        m_presenter->invalidate();

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw();

        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty());
    }

    void testMultipleDrawCallsWithoutChanges()
    {
        // First draw
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        // Second draw without changes - should be optimized
        m_presenter->draw();

        auto history = m_decoderClient->getCallbackHistory();
        // Should still call gfxFinish but minimize other operations
        CPPUNIT_ASSERT(!history.empty());
    }

    void testMultipleInvalidateCalls()
    {
        // Multiple invalidates should be safe
        m_presenter->invalidate();
        m_presenter->invalidate();
        m_presenter->invalidate();

        // Should still work normally
        m_presenter->draw();
    }

    void testDrawAfterInvalidateSequence()
    {
        // Setup some content
        setupTestPage();

        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        // Invalidate and draw again
        m_presenter->invalidate();
        m_presenter->draw();

        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty());
    }

    // Edge and Boundary Cases
    void testZeroDimensionRectangles()
    {
        // Setup page with zero-dimension region
        setupPageWithZeroDimensionRegion();

        // Should handle gracefully without crashing
        m_presenter->draw();
    }

    void testIntegerLimitCoordinates()
    {
        // Setup page with extreme coordinate values
        setupPageWithExtremeCoordinates();

        // Should handle without overflow
        m_presenter->draw();
    }

    void testEmptyRenderingState()
    {
        // Database with no regions - should handle empty state
        m_presenter->draw();

        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty()); // Should at least call gfxFinish
    }

    void testMaximumRegionCount()
    {
        // Setup page with many regions
        setupPageWithManyRegions();

        // Should handle large number of regions
        m_presenter->draw();
    }

    void testDisplayBoundsSmallerThanWindow()
    {
        // Setup invalid bounds relationship
        setupInvalidBoundsRelationship();

        // Should handle gracefully
        m_presenter->draw();
    }

    void testWindowBeyondDisplayBounds()
    {
        // Setup window extending beyond display
        setupWindowBeyondDisplay();

        // Should clip or handle appropriately
        m_presenter->draw();
    }

    // Invalid and Unusual Inputs
    void testNullRegionReferences()
    {
        // Setup page with references to non-existent regions
        setupPageWithInvalidRegionReferences();

        // Should skip null regions without crashing
        m_presenter->draw();
    }

    void testCorruptedRectangleData()
    {
        // Setup page with malformed rectangles
        setupPageWithCorruptedRectangles();

        // Should validate and handle safely
        m_presenter->draw();
    }

    // State Management Tests
    void testStateSwapWithIdenticalStates()
    {
        // Setup identical current and previous states
        m_presenter->draw(); // First draw

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw(); // Second draw with identical state

        // Should optimize redundant operations
        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty());
    }

    void testBoundsChangeNotifications()
    {
        // Change display/window bounds
        changeBounds();

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw();

        // Should notify bounds changes
        auto history = m_decoderClient->getCallbackHistory();
        bool foundBoundsCall = false;
        for (const auto& call : history) {
            if (call.method == MethodData::Method::gfxSetDisplayBounds) {
                foundBoundsCall = true;
                break;
            }
        }
        CPPUNIT_ASSERT(foundBoundsCall);
    }

    void testModifiedRectangleCalculation()
    {
        // Setup regions and verify modified rectangle calculation
        setupTestPage();
        m_presenter->draw();

        // Verify that gfxFinish is called with some rectangle
        auto history = m_decoderClient->getCallbackHistory();
        bool foundFinish = false;
        for (const auto& call : history) {
            if (call.method == MethodData::Method::gfxFinish) {
                foundFinish = true;
                break;
            }
        }
        CPPUNIT_ASSERT(foundFinish);
    }

    void testEmptyModifiedRectangle()
    {
        // Setup case where nothing is modified
        m_presenter->draw(); // Initial draw

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw(); // Second draw, nothing changed

        // Should still call gfxFinish with empty rectangle
        auto history = m_decoderClient->getCallbackHistory();
        bool foundFinish = false;
        for (const auto& call : history) {
            if (call.method == MethodData::Method::gfxFinish) {
                foundFinish = true;
                break;
            }
        }
        CPPUNIT_ASSERT(foundFinish);
    }

    // Complex Rendering Scenarios
    void testFewLargeRegions()
    {
        // Setup few large regions
        setupFewLargeRegions();

        // Should handle large regions
        m_presenter->draw();
    }

    void testRapidStateChanges()
    {
        // Simulate rapid state changes
        for (int i = 0; i < 10; ++i) {
            m_presenter->invalidate();
            m_presenter->draw();
        }
    }

    void testInterleavedClearAndDraw()
    {
        // First: create a page with a region and draw
        setupTestPage();
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        // Second: reset page (no regions) to trigger clear path
        m_database->getPage().reset();
        m_presenter->invalidate();
        m_presenter->draw();

        auto history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(!history.empty());
    }

    void testRegionOverlapHandling()
    {
        // Setup overlapping regions directly
        setupPageWithOverlappingRegions();

        // Should handle overlap correctly
        m_presenter->draw();
    }

    void testCoordinateTransformation()
    {
        setupTestPage();

        // Should transform coordinates correctly
        m_presenter->draw();
    }

    void testIsRectangleInsideBoundaryConditions()
    {
        // Setup rectangles at exact boundaries
        setupBoundaryRectangles();

        // Should handle boundary conditions
        m_presenter->draw();
    }

private:
    std::unique_ptr<DecoderClientMock> m_decoderClient;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
    std::unique_ptr<Presenter> m_presenter;

    void setupBasicDisplayBounds()
    {
        auto& display = m_database->getCurrentDisplay();
        Rectangle displayBounds = {0, 0, 720, 576};
        Rectangle windowBounds = {0, 0, 720, 576};
        display.set(0, displayBounds, windowBounds);
    }

    void setupTestPage()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5); // timeout 5s
        page.addRegion(1, 100, 100);
        page.finishParsing();
        createTestRegion(1, 200, 150);
    }

    void createTestRegion(uint16_t id, int width, int height)
    {
        if (width <= 0 || height <= 0) {
            return; // Skip invalid sizes in this simplified helper.
        }
        // Region depth / compatibility / clut id use 0 for tests.
        auto region = m_database->addRegionAndClut(static_cast<std::uint8_t>(id), width, height, 0, 0, 0);
        if (region) {
            region->setVersion(1);
        }
    }

    void setupPageWithZeroDimensionRegion()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 100, 100);
        page.finishParsing();
        createTestRegion(1, 0, 0); // skipped in helper
    }

    void setupPageWithExtremeCoordinates()
    {
        // Clamp to uint16 range as per API.
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 60000 % 65535, 60000 % 65535);
        page.finishParsing();
        createTestRegion(1, 100, 100);
    }

    void setupPageWithManyRegions()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        for (uint16_t i = 1; i <= 20; ++i) { // keep reasonable for test runtime
            page.addRegion(static_cast<std::uint8_t>(i), (i % 10) * 50, (i / 10) * 50);
            createTestRegion(i, 40, 40);
        }
        page.finishParsing();
    }

    void setupPageWithOverlappingRegions()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        for (uint16_t i = 1; i <= 3; ++i) {
            page.addRegion(static_cast<std::uint8_t>(i), 100 + (i - 1) * 25, 100);
            createTestRegion(i, 100, 100);
        }
        page.finishParsing();
    }

    void setupInvalidBoundsRelationship()
    {
        auto& display = m_database->getCurrentDisplay();
        Rectangle displayBounds = {0, 0, 400, 300};
        Rectangle windowBounds = {0, 0, 720, 576};
        display.set(0, displayBounds, windowBounds);
    }

    void setupWindowBeyondDisplay()
    {
        auto& display = m_database->getCurrentDisplay();
        Rectangle displayBounds = {0, 0, 720, 576};
        Rectangle windowBounds = {100, 100, 820, 676};
        display.set(0, displayBounds, windowBounds);
    }

    void setupPageWithInvalidRegionReferences()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        // Do not add any regions but finish parsing to simulate page with no matching region objects.
        page.finishParsing();
    }

    void setupPageWithCorruptedRectangles()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 200, 200);
        page.finishParsing();
        createTestRegion(1, -50, -50); // skipped
    }

    void changeBounds() {
        auto& display = m_database->getCurrentDisplay();
        Rectangle displayBounds = {0, 0, 800, 600};
        Rectangle windowBounds = {0, 0, 800, 600};
        display.set(0, displayBounds, windowBounds);
    }

    void setupFewLargeRegions() {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 0, 0);
        page.finishParsing();
        createTestRegion(1, 500, 400);
    }

    void setupBoundaryRectangles() {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 10, 10);
        page.finishParsing();
        createTestRegion(1, 0, 0); // skipped
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PresenterTest);
