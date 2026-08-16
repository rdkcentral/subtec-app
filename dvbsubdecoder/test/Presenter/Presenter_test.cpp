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
#include <cstddef>
#include <cstdint>
#include <memory>

#include "Presenter.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "Types.hpp"
#include "Config.hpp"
#include "Consts.hpp"
#include "../Decoder/DecoderClientMock.hpp"

using dvbsubdecoder::Presenter;
using dvbsubdecoder::Database;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Rectangle;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::MAX_SUPPORTED_REGIONS;

class PresenterTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PresenterTest );
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testBasicDraw);
    CPPUNIT_TEST(testBasicInvalidate);
    CPPUNIT_TEST(testMultipleDrawCallsWithoutChanges);
    CPPUNIT_TEST(testMultipleInvalidateCalls);
    CPPUNIT_TEST(testDrawAfterInvalidateSequence);
    CPPUNIT_TEST(testZeroDimensionRegionRejected);
    CPPUNIT_TEST(testIntegerLimitCoordinates);
    CPPUNIT_TEST(testEmptyRenderingState);
    CPPUNIT_TEST(testMaximumRegionCount);
    CPPUNIT_TEST(testDisplayBoundsSmallerThanWindow);
    CPPUNIT_TEST(testWindowBeyondDisplayBounds);
    CPPUNIT_TEST(testNullRegionReferences);
    CPPUNIT_TEST(testInvalidRegionDimensionsRejected);
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
    CPPUNIT_TEST(testComposedRegionCorrectness);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_decoderClient = std::make_unique<DecoderClientMock>();
        m_pixmapAllocator = std::make_unique<PixmapAllocator>(Specification::VERSION_1_3_1, *m_decoderClient);
        m_database = std::make_unique<Database>(Specification::VERSION_1_3_1, *m_pixmapAllocator);
        m_database->epochReset();
        m_presenter = std::make_unique<Presenter>(*m_decoderClient, *m_database);

        // Set up basic display bounds
        setupBasicDisplayBounds();
        m_decoderClient->clearCallbackHistory();
    }

    void tearDown()
    {
        if (m_presenter) m_presenter->invalidate();
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
        client.clearCallbackHistory();

        Presenter presenter(client, database);

        // Should be able to call public methods without crashing
        presenter.invalidate();
        presenter.draw();

        const auto& history = client.getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxFinish);
    }

    void testBasicDraw()
    {
        m_decoderClient->clearCallbackHistory();

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({0, 0, 720, 576}));
    }

    void testBasicInvalidate()
    {
        // Invalidate should not crash and should affect next draw.
        m_presenter->invalidate();

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxFinish);
    }

    void testMultipleDrawCallsWithoutChanges()
    {
        setupTestPage();
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        // Second draw without changes - should be optimized
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[0].gfxFinishArgs.rect == Rectangle({0, 0, 0, 0}));
    }

    void testMultipleInvalidateCalls()
    {
        setupTestPage();
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        m_presenter->invalidate();
        m_presenter->invalidate();
        m_presenter->invalidate();

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxDraw);
        CPPUNIT_ASSERT(history[2].method == MethodData::Method::gfxFinish);
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

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxDraw);
        CPPUNIT_ASSERT(history[2].method == MethodData::Method::gfxFinish);
    }

    // Edge and Boundary Cases
    void testZeroDimensionRegionRejected()
    {
        // Setup page with a zero-dimension region request.
        auto* rejectedRegion = setupPageWithZeroDimensionRegion();

        // Database rejects the invalid region request, leaving only the page reference behind.
        CPPUNIT_ASSERT(rejectedRegion == nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), m_database->getRegionCount());
        m_presenter->draw();
    }

    void testIntegerLimitCoordinates()
    {
        // Setup page with the largest coordinate accepted by the page API.
        setupPageWithExtremeCoordinates();

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), history.size());
        CPPUNIT_ASSERT(history[2].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[2].gfxFinishArgs.rect == Rectangle({0, 0, 65535, 65535}));
    }

    void testEmptyRenderingState()
    {
        // Database with no regions - should handle empty state
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({0, 0, 720, 576}));
    }

    void testMaximumRegionCount()
    {
        // Setup page with many regions
        setupPageWithManyRegions();

        m_presenter->draw();

        CPPUNIT_ASSERT_EQUAL(MAX_SUPPORTED_REGIONS, m_database->getRegionCount());
        CPPUNIT_ASSERT_EQUAL(MAX_SUPPORTED_REGIONS, m_database->getPage().getRegionCount());
        CPPUNIT_ASSERT_EQUAL(MAX_SUPPORTED_REGIONS, countCalls(MethodData::Method::gfxDraw));
    }

    void testDisplayBoundsSmallerThanWindow()
    {
        // Setup invalid bounds relationship
        setupInvalidBoundsRelationship();

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(history[0].gfxSetDisplayBoundsArgs.displayBounds == Rectangle({0, 0, 400, 300}));
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({0, 0, 400, 300}));
    }

    void testWindowBeyondDisplayBounds()
    {
        // Setup window extending beyond display
        setupWindowBeyondDisplay();

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(history[0].gfxSetDisplayBoundsArgs.windowBounds == Rectangle({100, 100, 820, 676}));
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({0, 0, 720, 576}));
    }

    // Invalid and Unusual Inputs
    void testNullRegionReferences()
    {
        // Setup page with references to non-existent regions
        setupPageWithInvalidRegionReferences();

        m_presenter->draw();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), countCalls(MethodData::Method::gfxDraw));
    }

    void testInvalidRegionDimensionsRejected()
    {
        // Setup page with invalid (negative) region dimensions.
        auto* rejectedRegion = setupPageWithNegativeDimensionRegion();

        // Database rejects the invalid region request, and presenter remains stable.
        CPPUNIT_ASSERT(rejectedRegion == nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), m_database->getRegionCount());
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[1].method == MethodData::Method::gfxFinish);
    }

    // State Management Tests
    void testStateSwapWithIdenticalStates()
    {
        // Setup identical current and previous states
        m_presenter->draw(); // First draw

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw(); // Second draw with identical state

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[0].gfxFinishArgs.rect == Rectangle({0, 0, 0, 0}));
    }

    void testBoundsChangeNotifications()
    {
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        changeBounds();

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxSetDisplayBounds);
        CPPUNIT_ASSERT(history[0].gfxSetDisplayBoundsArgs.displayBounds == Rectangle({0, 0, 800, 600}));
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({0, 0, 800, 600}));
    }

    void testModifiedRectangleCalculation()
    {
        // Setup regions and verify modified rectangle calculation
        setupTestPage();
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(history[history.size() - 1].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[history.size() - 1].gfxFinishArgs.rect == Rectangle({0, 0, 720, 576}));
    }

    void testEmptyModifiedRectangle()
    {
        // Setup case where nothing is modified
        m_presenter->draw(); // Initial draw

        m_decoderClient->clearCallbackHistory();
        m_presenter->draw(); // Second draw, nothing changed

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxFinish);
        CPPUNIT_ASSERT(history[0].gfxFinishArgs.rect == Rectangle({0, 0, 0, 0}));
    }

    // Complex Rendering Scenarios
    void testFewLargeRegions()
    {
        // Setup few large regions
        setupFewLargeRegions();

        m_presenter->draw();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), countCalls(MethodData::Method::gfxDraw));
    }

    void testRapidStateChanges()
    {
        setupTestPage();
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        for (int i = 0; i < 10; ++i) {
            m_presenter->invalidate();
            m_presenter->draw();
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(10), countCalls(MethodData::Method::gfxDraw));
    }

    void testInterleavedClearAndDraw()
    {
        // First: create a page with a region and draw
        setupTestPage();
        m_presenter->draw();
        m_decoderClient->clearCallbackHistory();

        // Second: reset page (no regions) to trigger clear path.
        m_database->getPage().reset();
        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), history.size());
        CPPUNIT_ASSERT(history[0].method == MethodData::Method::gfxClear);
        CPPUNIT_ASSERT(history[0].gfxClearArgs.rect == Rectangle({100, 100, 300, 250}));
        CPPUNIT_ASSERT(history[1].gfxFinishArgs.rect == Rectangle({100, 100, 300, 250}));
    }

    void testRegionOverlapHandling()
    {
        // Setup overlapping regions directly
        setupPageWithOverlappingRegions();

        m_presenter->draw();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), countCalls(MethodData::Method::gfxDraw));
    }

    void testCoordinateTransformation()
    {
        setupTestPage();

        auto& display = m_database->getCurrentDisplay();
        display.set(0, Rectangle({0, 0, 720, 576}), Rectangle({50, 60, 720, 576}));

        m_presenter->draw();

        const auto& history = m_decoderClient->getCallbackHistory();
        CPPUNIT_ASSERT(history[1].gfxDrawArgs.dstRect == Rectangle({150, 160, 350, 310}));
    }

    void testIsRectangleInsideBoundaryConditions()
    {
        // Setup rectangles at exact boundaries
        setupBoundaryRectangles();

        m_presenter->draw();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), countCalls(MethodData::Method::gfxDraw));
    }

    void testComposedRegionCorrectness()
    {
        // Setup a page with a known region
        setupTestPage();
        m_presenter->draw();
        // After draw, verify region exists and has expected dimensions
        auto& page = m_database->getPage();
        // Ensure region index is valid
        CPPUNIT_ASSERT(page.getRegionCount() > 0);
        const auto& region = page.getRegion(0);
        CPPUNIT_ASSERT(m_database->getRegionById(1) != nullptr);
        CPPUNIT_ASSERT_EQUAL(200, m_database->getRegionById(1)->getWidth());
        CPPUNIT_ASSERT_EQUAL(150, m_database->getRegionById(1)->getHeight());
        CPPUNIT_ASSERT_EQUAL(100, region.m_positionX);
        CPPUNIT_ASSERT_EQUAL(100, region.m_positionY);
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
        createTestRegion(1, 200, 150);
        page.finishParsing();
        m_decoderClient->clearCallbackHistory();
    }

    dvbsubdecoder::Region* createTestRegion(uint16_t id, int width, int height)
    {
        auto region = m_database->addRegionAndClut(static_cast<std::uint8_t>(id), width, height,
            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0);
        if (region) {
            region->setVersion(1);
        }
        return region;
    }

    dvbsubdecoder::Region* setupPageWithZeroDimensionRegion()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 100, 100);
        auto* region = createTestRegion(1, 0, 0);
        page.finishParsing();
        return region;
    }

    void setupPageWithExtremeCoordinates()
    {
        // Clamp to uint16 range as per API.
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 60000, 60000);
        createTestRegion(1, 100, 100);
        page.finishParsing();
        m_database->getCurrentDisplay().set(0, Rectangle({0, 0, 65535, 65535}), Rectangle({0, 0, 65535, 65535}));
        m_decoderClient->clearCallbackHistory();
    }

    void setupPageWithManyRegions()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        for (uint16_t i = 1; i <= MAX_SUPPORTED_REGIONS; ++i) {
            page.addRegion(static_cast<std::uint8_t>(i), (i % 10) * 50, (i / 10) * 50);
            createTestRegion(i, 40, 40);
        }
        page.finishParsing();
        m_decoderClient->clearCallbackHistory();
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
        m_decoderClient->clearCallbackHistory();
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
        page.addRegion(99, 100, 100);
        page.finishParsing();
    }

    dvbsubdecoder::Region* setupPageWithNegativeDimensionRegion()
    {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 200, 200);
        auto* region = createTestRegion(1, -50, -50);
        page.finishParsing();
        return region;
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
        createTestRegion(1, 500, 400);
        page.finishParsing();
        m_decoderClient->clearCallbackHistory();
    }

    void setupBoundaryRectangles() {
        auto& page = m_database->getPage();
        page.startParsing(0, StcTime(), 5);
        page.addRegion(1, 0, 0);
        page.addRegion(2, 719, 575);
        createTestRegion(1, 720, 576);
        createTestRegion(2, 2, 2);
        page.finishParsing();
        m_decoderClient->clearCallbackHistory();
    }

    std::size_t countCalls(MethodData::Method method) const
    {
        std::size_t count = 0;
        for (const auto& call : m_decoderClient->getCallbackHistory()) {
            if (call.method == method) ++count;
        }
        return count;
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PresenterTest);
