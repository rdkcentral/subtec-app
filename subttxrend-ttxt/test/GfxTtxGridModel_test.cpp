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
#include "GfxTtxGridModel.hpp"
#include "GfxTtxClut.hpp"
#include <ttxdecoder/Engine.hpp>
#include <ttxdecoder/DecodedPage.hpp>
#include <ttxdecoder/PageId.hpp>
#include <ttxdecoder/ControlInfo.hpp>

using subttxrend::ttxt::GfxTtxGridModel;
using subttxrend::ttxt::Size;

// Local constants to avoid linker issues with static class members
namespace {
    const std::uint8_t COLOR_INDEX_BLACK = 0;
    const std::uint8_t COLOR_INDEX_RED = 1;
    const std::uint8_t COLOR_INDEX_GREEN = 2;
    const std::uint8_t COLOR_INDEX_YELLOW = 3;
    const std::uint8_t COLOR_INDEX_CYAN = 5;
    const std::uint8_t COLOR_INDEX_WHITE = 7;
    const std::uint8_t COLOR_INDEX_TRANSPARENT = 8;
}

/**
 * Mock TTX Engine for testing GfxTtxGridModel
 */
class MockTtxEngine : public ttxdecoder::Engine
{
public:
    MockTtxEngine()
        : m_currentPageId(1, 0x100)
    {
        // Initialize default page
        m_decodedPage = ttxdecoder::DecodedPage();
    }

    void setPageControlInfo(std::uint8_t controlInfo)
    {
        m_pageControlInfo = controlInfo;
    }

    void setCurrentPageId(const ttxdecoder::PageId& pageId) override
    {
        m_currentPageId = pageId;
    }

    ttxdecoder::PageId getPageId(ttxdecoder::PageIdType type) const override
    {
        if (type == ttxdecoder::PageIdType::CURRENT_PAGE)
        {
            return m_currentPageId;
        }
        return ttxdecoder::PageId();
    }

    const ttxdecoder::DecodedPage& getPage() const override
    {
        return m_decodedPage;
    }

    // Unused pure virtual implementations - stubbed for compilation
    void resetAcquisition() override {}
    std::uint32_t process() override { return 0; }
    bool addPesPacket(const std::uint8_t* packet, std::uint16_t packetLength) override { return true; }
    ttxdecoder::PageId getNextPageId(const ttxdecoder::PageId& inputPageId) const override { return inputPageId; }
    ttxdecoder::PageId getPrevPageId(const ttxdecoder::PageId& inputPageId) const override { return inputPageId; }
    void setNavigationMode(ttxdecoder::NavigationMode navigationMode) override {}
    ttxdecoder::NavigationState getNavigationState() const override { return ttxdecoder::NavigationState(); }
    void getTopLinkText(std::uint16_t magazine, std::uint32_t page, std::uint16_t* text) const override {}
    std::uint8_t getScreenColorIndex() const override { return 0; }
    std::uint8_t getRowColorIndex(std::uint8_t row) const override { return 0; }
    bool getColors(std::array<std::uint32_t, 16>& colors) const override { return true; }
    const ttxdecoder::CharsetMappingArray& getCharsetMapping(ttxdecoder::Charset charset) const override
    {
        static ttxdecoder::CharsetMappingArray mapping;
        return mapping;
    }
    void setCharsetMapping(ttxdecoder::Charset charset, const ttxdecoder::CharsetMappingArray& mapping) override {}
    void setDefaultPrimaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}
    void setDefaultSecondaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}
    std::uint8_t getPageControlInfo() const override { return m_pageControlInfo; }
    void setIgnorePts(bool ignorePts) override {}

private:
    ttxdecoder::DecodedPage m_decodedPage;
    ttxdecoder::PageId m_currentPageId;
    std::uint8_t m_pageControlInfo = 0;
};

class GfxTtxGridModelTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxTtxGridModelTest);
    CPPUNIT_TEST(testConstructorWithValidSize);
    CPPUNIT_TEST(testConstructorWithZeroWidth);
    CPPUNIT_TEST(testConstructorWithZeroHeight);
    CPPUNIT_TEST(testConstructorWithBothZero);
    CPPUNIT_TEST(testConstructorLargeSize);
    CPPUNIT_TEST(testGetSizeReturnsCorrectDimensions);
    CPPUNIT_TEST(testInitWithValidEngine);
    CPPUNIT_TEST(testInitWithNullptr);
    CPPUNIT_TEST(testInitCalledMultipleTimes);
    CPPUNIT_TEST(testGetConstCellWithValidCoordinates);
    CPPUNIT_TEST(testGetConstCellAtOrigin);
    CPPUNIT_TEST(testGetConstCellAtMaxValidCoordinates);
    CPPUNIT_TEST(testGetConstCellWithNegativeColumn);
    CPPUNIT_TEST(testGetConstCellWithNegativeRow);
    CPPUNIT_TEST(testGetConstCellWithColumnOutOfBounds);
    CPPUNIT_TEST(testGetConstCellWithRowOutOfBounds);
    CPPUNIT_TEST(testGetConstCellWithBothNegative);
    CPPUNIT_TEST(testGetConstCellWithBothOutOfBounds);
    CPPUNIT_TEST(testSetFlashEnabledTrueWhenFalse);
    CPPUNIT_TEST(testSetFlashEnabledFalseWhenTrue);
    CPPUNIT_TEST(testSetFlashEnabledTrueWhenTrue);
    CPPUNIT_TEST(testSetFlashEnabledFalseWhenFalse);
    CPPUNIT_TEST(testToggleRevealEnabledFromFalse);
    CPPUNIT_TEST(testToggleRevealEnabledTwice);
    CPPUNIT_TEST(testSetRevealEnabledTrueWhenFalse);
    CPPUNIT_TEST(testSetRevealEnabledFalseWhenTrue);
    CPPUNIT_TEST(testSetRevealEnabledTrueWhenTrue);
    CPPUNIT_TEST(testSetRevealEnabledFalseWhenFalse);
    CPPUNIT_TEST(testClearAllInSubtitlesMode);
    CPPUNIT_TEST(testClearAllInNormalMode);
    CPPUNIT_TEST(testClearAllOnStandardGrid);
    CPPUNIT_TEST(testClearAllOnZeroSizeGrid);
    CPPUNIT_TEST(testMarkAllAsChangedOnPopulatedGrid);
    CPPUNIT_TEST(testMarkChangedByColorWithValidColor);
    CPPUNIT_TEST(testMarkChangedByColorWithColorZero);
    CPPUNIT_TEST(testMarkChangedByColorWithColorMax);
    CPPUNIT_TEST(testRefreshSelectionNormalWithZeroPage);
    CPPUNIT_TEST(testRefreshSelectionPaused);
    CPPUNIT_TEST(testRefreshSelectionWithNewPageId);
    CPPUNIT_TEST(testRefreshSelectionWithMaxPageId);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockEngine.reset(new MockTtxEngine());
    }

    void tearDown() override
    {
        m_model.reset();
        m_mockEngine.reset();
    }

protected:
    void testConstructorWithValidSize()
    {
        Size size(40, 25);
        m_model.reset(new GfxTtxGridModel(size));

        CPPUNIT_ASSERT(m_model.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(40, m_model->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(25, m_model->getSize().m_h);
    }

    void testConstructorWithZeroWidth()
    {
        Size size(0, 25);
        m_model.reset(new GfxTtxGridModel(size));

        CPPUNIT_ASSERT(m_model.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(0, m_model->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(25, m_model->getSize().m_h);
    }

    void testConstructorWithZeroHeight()
    {
        Size size(40, 0);
        m_model.reset(new GfxTtxGridModel(size));

        CPPUNIT_ASSERT(m_model.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(40, m_model->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(0, m_model->getSize().m_h);
    }

    void testConstructorWithBothZero()
    {
        Size size(0, 0);
        m_model.reset(new GfxTtxGridModel(size));

        CPPUNIT_ASSERT(m_model.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(0, m_model->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(0, m_model->getSize().m_h);
    }

    void testConstructorLargeSize()
    {
        Size size(100, 100);
        m_model.reset(new GfxTtxGridModel(size));

        CPPUNIT_ASSERT(m_model.get() != nullptr);
        CPPUNIT_ASSERT_EQUAL(100, m_model->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(100, m_model->getSize().m_h);
    }

    void testGetSizeReturnsCorrectDimensions()
    {
        Size size(30, 20);
        m_model.reset(new GfxTtxGridModel(size));

        const Size& returnedSize = m_model->getSize();
        CPPUNIT_ASSERT_EQUAL(30, returnedSize.m_w);
        CPPUNIT_ASSERT_EQUAL(20, returnedSize.m_h);
    }

    void testInitWithValidEngine()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());

        // Verify init doesn't crash and object is still valid
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testInitWithNullptr()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(nullptr);

        // Should not crash
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testInitCalledMultipleTimes()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());
        m_model->init(m_mockEngine.get());

        // Multiple init calls should be safe
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testGetConstCellWithValidCoordinates()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(20, 12);
        CPPUNIT_ASSERT(cell != nullptr);
    }

    void testGetConstCellAtOrigin()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell != nullptr);
    }

    void testGetConstCellAtMaxValidCoordinates()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(39, 24);
        CPPUNIT_ASSERT(cell != nullptr);
    }

    void testGetConstCellWithNegativeColumn()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(-1, 10);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testGetConstCellWithNegativeRow()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(10, -1);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testGetConstCellWithColumnOutOfBounds()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(40, 10);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testGetConstCellWithRowOutOfBounds()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(10, 25);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testGetConstCellWithBothNegative()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(-5, -10);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testGetConstCellWithBothOutOfBounds()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        const auto* cell = m_model->getConstCell(100, 100);
        CPPUNIT_ASSERT(cell == nullptr);
    }

    void testSetFlashEnabledTrueWhenFalse()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        bool changed = m_model->setFlashEnabled(true);
        CPPUNIT_ASSERT_EQUAL(true, changed);
    }

    void testSetFlashEnabledFalseWhenTrue()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->setFlashEnabled(true);
        bool changed = m_model->setFlashEnabled(false);
        CPPUNIT_ASSERT_EQUAL(true, changed);
    }

    void testSetFlashEnabledTrueWhenTrue()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->setFlashEnabled(true);
        bool changed = m_model->setFlashEnabled(true);
        CPPUNIT_ASSERT_EQUAL(false, changed);
    }

    void testSetFlashEnabledFalseWhenFalse()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        bool changed = m_model->setFlashEnabled(false);
        CPPUNIT_ASSERT_EQUAL(false, changed);
    }

    void testToggleRevealEnabledFromFalse()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Initial state is false, after toggle should be true
        // We verify by toggling twice and checking setRevealEnabled behavior
        m_model->toggleRevealEnabled();
        bool changed = m_model->setRevealEnabled(true);
        CPPUNIT_ASSERT_EQUAL(false, changed); // Already true, so no change
    }

    void testToggleRevealEnabledTwice()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Toggle twice should return to original state
        m_model->toggleRevealEnabled();
        m_model->toggleRevealEnabled();
        bool changed = m_model->setRevealEnabled(false);
        CPPUNIT_ASSERT_EQUAL(false, changed); // Already false, so no change
    }

    void testSetRevealEnabledTrueWhenFalse()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        bool changed = m_model->setRevealEnabled(true);
        CPPUNIT_ASSERT_EQUAL(true, changed);
    }

    void testSetRevealEnabledFalseWhenTrue()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->setRevealEnabled(true);
        bool changed = m_model->setRevealEnabled(false);
        CPPUNIT_ASSERT_EQUAL(true, changed);
    }

    void testSetRevealEnabledTrueWhenTrue()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->setRevealEnabled(true);
        bool changed = m_model->setRevealEnabled(true);
        CPPUNIT_ASSERT_EQUAL(false, changed);
    }

    void testSetRevealEnabledFalseWhenFalse()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        bool changed = m_model->setRevealEnabled(false);
        CPPUNIT_ASSERT_EQUAL(false, changed);
    }

    void testClearAllInSubtitlesMode()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->clearAll(true);

        // Verify cells are cleared with transparent background
        const auto* cell = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell != nullptr);
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_TRANSPARENT, cell->getBgColor());
    }

    void testClearAllInNormalMode()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->clearAll(false);

        // Verify cells are cleared with black background
        const auto* cell = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell != nullptr);
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_BLACK, cell->getBgColor());
    }

    void testClearAllOnStandardGrid()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        m_model->clearAll(false);

        // Verify multiple cells are cleared
        const auto* cell1 = m_model->getConstCell(0, 0);
        const auto* cell2 = m_model->getConstCell(39, 24);
        const auto* cell3 = m_model->getConstCell(20, 12);

        CPPUNIT_ASSERT(cell1 != nullptr);
        CPPUNIT_ASSERT(cell2 != nullptr);
        CPPUNIT_ASSERT(cell3 != nullptr);

        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_BLACK, cell1->getBgColor());
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_BLACK, cell2->getBgColor());
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_BLACK, cell3->getBgColor());
    }

    void testClearAllOnZeroSizeGrid()
    {
        m_model.reset(new GfxTtxGridModel(Size(0, 0)));

        // Should not crash on empty grid
        m_model->clearAll(false);
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testMarkAllAsChangedOnPopulatedGrid()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Should not crash
        m_model->markAllAsChanged();
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testMarkChangedByColorWithValidColor()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Should not crash
        m_model->markChangedByColor(COLOR_INDEX_RED);
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testMarkChangedByColorWithColorZero()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Should not crash with boundary value
        m_model->markChangedByColor(0);
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testMarkChangedByColorWithColorMax()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));

        // Should not crash with maximum value
        m_model->markChangedByColor(255);
        CPPUNIT_ASSERT(m_model.get() != nullptr);
    }

    void testRefreshSelectionNormalWithZeroPage()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());

        m_mockEngine->setCurrentPageId(ttxdecoder::PageId(1, 0x100));

        // Should not crash
        m_model->refreshSelection(false, 0);

        // Verify first 4 cells have been updated (page number display)
        const auto* cell0 = m_model->getConstCell(0, 0);
        const auto* cell1 = m_model->getConstCell(1, 0);
        const auto* cell2 = m_model->getConstCell(2, 0);
        const auto* cell3 = m_model->getConstCell(3, 0);

        CPPUNIT_ASSERT(cell0 != nullptr);
        CPPUNIT_ASSERT(cell1 != nullptr);
        CPPUNIT_ASSERT(cell2 != nullptr);
        CPPUNIT_ASSERT(cell3 != nullptr);

        // Verify colors are set correctly for page number
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_WHITE, cell0->getFgColor());
        CPPUNIT_ASSERT_EQUAL(COLOR_INDEX_BLACK, cell0->getBgColor());
    }

    void testRefreshSelectionPaused()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());

        // Should display "PAUS" when paused
        m_model->refreshSelection(true, 0);

        const auto* cell0 = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell0 != nullptr);

        // Verify character is 'P' (ASCII 80)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('P'), cell0->getChar());
    }

    void testRefreshSelectionWithNewPageId()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());

        // Should display new page ID when provided
        m_model->refreshSelection(false, 0x123);

        const auto* cell0 = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell0 != nullptr);

        // Should have 'P' as first character
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('P'), cell0->getChar());
    }

    void testRefreshSelectionWithMaxPageId()
    {
        m_model.reset(new GfxTtxGridModel(Size(40, 25)));
        m_model->init(m_mockEngine.get());

        // Should handle maximum valid page ID
        m_model->refreshSelection(false, 0xFFF);

        const auto* cell0 = m_model->getConstCell(0, 0);
        CPPUNIT_ASSERT(cell0 != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('P'), cell0->getChar());
    }


private:
    std::unique_ptr<GfxTtxGridModel> m_model;
    std::unique_ptr<MockTtxEngine> m_mockEngine;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxTtxGridModelTest);