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
#include <stdexcept>
#include <string>

#include "DecodedPage.hpp"

using namespace ttxdecoder;

// Local constants to avoid ODR-use of static members
namespace
{
    constexpr uint16_t INVALID_MAGAZINE_PAGE = 0xFFFF;
    constexpr uint16_t ANY_SUBPAGE = 0xFFFF;
    constexpr uint16_t NULL_MAGAZINE_PAGE_MASK = 0x00FF;
    constexpr uint16_t NULL_SUBPAGE = 0x3F7F;
    constexpr std::size_t MAX_ROWS = 26;  // DecodedPage::MAX_ROWS = 1 + 25
    constexpr std::size_t LEVEL_ONE_PAGE_WIDTH = 40;  // DecodedPageRow::LEVEL_ONE_PAGE_WIDTH
}

class DecodedPageTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DecodedPageTest );
    CPPUNIT_TEST(testDefaultConstructorInitialization);
    CPPUNIT_TEST(testMultipleObjectInstantiation);
    CPPUNIT_TEST(testClearEmptyPage);
    CPPUNIT_TEST(testClearPageWithData);
    CPPUNIT_TEST(testClearAfterSettingAllFields);
    CPPUNIT_TEST(testMultipleConsecutiveClears);
    CPPUNIT_TEST(testSetAndGetValidPageId);
    CPPUNIT_TEST(testSetPageIdWithBoundaryValuesMin);
    CPPUNIT_TEST(testSetPageIdWithBoundaryValuesMax);
    CPPUNIT_TEST(testSetPageIdWithNullPage);
    CPPUNIT_TEST(testOverwriteExistingPageId);
    CPPUNIT_TEST(testSetPageIdAfterClear);
    CPPUNIT_TEST(testSetAndGetValidControlInfo);
    CPPUNIT_TEST(testSetControlInfoWithBoundaryValueMin);
    CPPUNIT_TEST(testSetControlInfoWithBoundaryValueMax);
    CPPUNIT_TEST(testSetVariousControlBitPatterns);
    CPPUNIT_TEST(testOverwriteExistingControlInfo);
    CPPUNIT_TEST(testSetControlInfoAfterClear);
    CPPUNIT_TEST(testGetRowCountOnNewPage);
    CPPUNIT_TEST(testGetRowCountAfterClear);
    CPPUNIT_TEST(testGetRowCountAfterModifications);
    CPPUNIT_TEST(testGetRowAtIndexZero);
    CPPUNIT_TEST(testGetRowAtIndexMax);
    CPPUNIT_TEST(testGetRowAtMiddleIndex);
    CPPUNIT_TEST(testGetRowNonConstAndModify);
    CPPUNIT_TEST(testGetRowConstVersion);
    CPPUNIT_TEST(testGetAllRowsSequentially);
    CPPUNIT_TEST(testGetRowWithIndexEqualsMaxRows);
    CPPUNIT_TEST(testGetRowWithIndexGreaterThanMaxRows);
    CPPUNIT_TEST(testGetRowWithVeryLargeIndex);
    CPPUNIT_TEST(testGetRowExceptionMessage);
    CPPUNIT_TEST(testGetRowConstVersionWithInvalidIndex);
    CPPUNIT_TEST(testSetAllFiveColorLinksWithDifferentValues);
    CPPUNIT_TEST(testOverwriteExistingColorLink);
    CPPUNIT_TEST(testSetColorLinksAfterClear);
    CPPUNIT_TEST(testSetColorLinkWithNullPageId);
    CPPUNIT_TEST(testSetColorLinkWithBoundaryPageIds);
    CPPUNIT_TEST(testModifyRowDataAndVerifyPageState);
    CPPUNIT_TEST(testClearPageAfterSettingRows);
    CPPUNIT_TEST(testSetMultipleRowsWithDifferentData);
    CPPUNIT_TEST(testAccessRowDataThroughConstAndNonConstGetters);
    CPPUNIT_TEST(testSetAllPageProperties);
    CPPUNIT_TEST(testClearAndRebuildPageState);
    CPPUNIT_TEST(testDumpPageWithDataCharsOnlyTrue);
    CPPUNIT_TEST(testDumpPageWithDataCharsOnlyFalse);
    CPPUNIT_TEST(testDumpPageWithSpecialCharacters);
    CPPUNIT_TEST(testDumpAfterClear);
    CPPUNIT_TEST(testCreateAndDestroyMultipleInstances);
    CPPUNIT_TEST(testLargeNumberOfStateChanges);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_page = std::make_unique<DecodedPage>();
    }

    void tearDown() override
    {
        m_page.reset();
    }

protected:
    void testDefaultConstructorInitialization()
    {
        DecodedPage page;

        // Verify page ID is initialized to invalid
        PageId pageId = page.getPageId();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, pageId.getSubpage());

        // Verify control info is initialized to 0
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), page.getPageControlInfo());

        // Verify row count is MAX_ROWS
        CPPUNIT_ASSERT_EQUAL(MAX_ROWS, page.getRowCount());

        // Verify color links are initialized to default
        for (int i = 0; i <= static_cast<int>(DecodedPage::Link::LAST); ++i)
        {
            DecodedPage::Link link = static_cast<DecodedPage::Link>(i);
            const PageId& linkValue = page.getColourKeyLink(link);
            CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, linkValue.getMagazinePage());
            CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, linkValue.getSubpage());
        }

        // Verify rows are initialized to cleared values
        for (std::size_t rowIndex = 0; rowIndex < page.getRowCount(); ++rowIndex)
        {
            const DecodedPageRow& row = page.getRow(rowIndex);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '),
                               row.m_levelOnePageSegment.m_charArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                               row.m_levelOnePageSegment.m_bgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                               row.m_levelOnePageSegment.m_fgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0),
                               row.m_levelOnePageSegment.m_propertiesArray[0]);
        }
    }

    void testMultipleObjectInstantiation()
    {
        DecodedPage page1;
        DecodedPage page2;

        // Set different values in each page
        PageId id1(0x100, 0x0001);
        PageId id2(0x200, 0x0002);

        page1.setPageId(id1);
        page2.setPageId(id2);

        // Verify each instance maintains its own state
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), page1.getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), page2.getPageId().getMagazinePage());
    }

    void testClearEmptyPage()
    {
        DecodedPage page;

        // Clear should work without errors on a new page
        page.clear();

        // Verify state remains default
        PageId pageId = page.getPageId();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, pageId.getSubpage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), page.getPageControlInfo());
    }

    void testClearPageWithData()
    {
        PageId testId(0x150, 0x0005);
        m_page->setPageId(testId);
        m_page->setPageControlInfo(0xAB);

        // Clear the page
        m_page->clear();

        // Verify all fields are reset
        PageId pageId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, pageId.getSubpage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), m_page->getPageControlInfo());
    }

    void testClearAfterSettingAllFields()
    {
        PageId testId(0x180, 0x0010);
        m_page->setPageId(testId);
        m_page->setPageControlInfo(0xFF);

        DecodedPageRow& row = m_page->getRow(0);
        row.m_levelOnePageSegment.m_charArray[0] = 0x1234;
        row.m_levelOnePageSegment.m_bgColorIndexArray[0] = 0xAA;
        row.m_levelOnePageSegment.m_fgColorIndexArray[0] = 0xBB;
        row.m_levelOnePageSegment.m_propertiesArray[0] = 0x5678;

        // Set all color links
        PageId linkId1(0x101, 0x0001);
        PageId linkId2(0x102, 0x0002);
        PageId linkId3(0x103, 0x0003);
        PageId linkId4(0x104, 0x0004);
        PageId linkId5(0x105, 0x0005);

        m_page->setColourKeyLink(DecodedPage::Link::RED, linkId1);
        m_page->setColourKeyLink(DecodedPage::Link::GREEN, linkId2);
        m_page->setColourKeyLink(DecodedPage::Link::YELLOW, linkId3);
        m_page->setColourKeyLink(DecodedPage::Link::CYAN, linkId4);
        m_page->setColourKeyLink(DecodedPage::Link::FLOF_INDEX, linkId5);

        // Clear the page
        m_page->clear();

        // Verify all fields are reset
        PageId pageId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, pageId.getSubpage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), m_page->getPageControlInfo());

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '),
                           m_page->getRow(0).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                           m_page->getRow(0).m_levelOnePageSegment.m_bgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                           m_page->getRow(0).m_levelOnePageSegment.m_fgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0),
                           m_page->getRow(0).m_levelOnePageSegment.m_propertiesArray[0]);

        // Verify all color links are reset
        for (int i = 0; i <= static_cast<int>(DecodedPage::Link::LAST); ++i)
        {
            DecodedPage::Link link = static_cast<DecodedPage::Link>(i);
            const PageId& linkValue = m_page->getColourKeyLink(link);
            CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, linkValue.getMagazinePage());
            CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, linkValue.getSubpage());
        }
    }

    void testMultipleConsecutiveClears()
    {
        m_page->clear();
        m_page->clear();
        m_page->clear();

        // Should remain in cleared state
        PageId pageId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, pageId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), m_page->getPageControlInfo());
    }

    void testSetAndGetValidPageId()
    {
        PageId testId(0x150, 0x0001);
        m_page->setPageId(testId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0001), retrievedId.getSubpage());
    }

    void testSetPageIdWithBoundaryValuesMin()
    {
        PageId testId(0x0000, 0x0000);
        m_page->setPageId(testId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0000), retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0000), retrievedId.getSubpage());
    }

    void testSetPageIdWithBoundaryValuesMax()
    {
        PageId testId(0xFFFF, 0xFFFF);
        m_page->setPageId(testId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), retrievedId.getSubpage());
    }

    void testSetPageIdWithNullPage()
    {
        PageId testId(NULL_MAGAZINE_PAGE_MASK, NULL_SUBPAGE);
        m_page->setPageId(testId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(NULL_MAGAZINE_PAGE_MASK, retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(NULL_SUBPAGE, retrievedId.getSubpage());
        CPPUNIT_ASSERT(retrievedId.isNull());
        CPPUNIT_ASSERT(retrievedId.isAnySubpage());
    }

    void testOverwriteExistingPageId()
    {
        PageId firstId(0x100, 0x0001);
        PageId secondId(0x200, 0x0002);

        m_page->setPageId(firstId);
        m_page->setPageId(secondId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0002), retrievedId.getSubpage());
    }

    void testSetPageIdAfterClear()
    {
        PageId firstId(0x100, 0x0001);
        m_page->setPageId(firstId);
        m_page->clear();

        PageId secondId(0x200, 0x0002);
        m_page->setPageId(secondId);

        PageId retrievedId = m_page->getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), retrievedId.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0002), retrievedId.getSubpage());
    }

    void testSetAndGetValidControlInfo()
    {
        m_page->setPageControlInfo(0x42);

        uint8_t retrievedInfo = m_page->getPageControlInfo();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x42), retrievedInfo);
    }

    void testSetControlInfoWithBoundaryValueMin()
    {
        m_page->setPageControlInfo(0x00);

        uint8_t retrievedInfo = m_page->getPageControlInfo();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x00), retrievedInfo);
    }

    void testSetControlInfoWithBoundaryValueMax()
    {
        m_page->setPageControlInfo(0xFF);

        uint8_t retrievedInfo = m_page->getPageControlInfo();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xFF), retrievedInfo);
    }

    void testSetVariousControlBitPatterns()
    {
        uint8_t testPatterns[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
                                  0xAA, 0x55, 0xF0, 0x0F, 0xCC, 0x33};

        for (uint8_t pattern : testPatterns)
        {
            m_page->setPageControlInfo(pattern);
            uint8_t retrievedInfo = m_page->getPageControlInfo();
            CPPUNIT_ASSERT_EQUAL(pattern, retrievedInfo);
        }
    }

    void testOverwriteExistingControlInfo()
    {
        m_page->setPageControlInfo(0x11);
        m_page->setPageControlInfo(0x22);

        uint8_t retrievedInfo = m_page->getPageControlInfo();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0x22), retrievedInfo);
    }

    void testSetControlInfoAfterClear()
    {
        m_page->setPageControlInfo(0xAA);
        m_page->clear();

        m_page->setPageControlInfo(0xBB);

        uint8_t retrievedInfo = m_page->getPageControlInfo();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xBB), retrievedInfo);
    }

    void testGetRowCountOnNewPage()
    {
        // A new page exposes rows 0..25, including the header row.
        std::size_t rowCount = m_page->getRowCount();
        CPPUNIT_ASSERT_EQUAL(MAX_ROWS, rowCount);
    }

    void testGetRowCountAfterClear()
    {
        m_page->clear();

        std::size_t rowCount = m_page->getRowCount();
        CPPUNIT_ASSERT_EQUAL(MAX_ROWS, rowCount);
    }

    void testGetRowCountAfterModifications()
    {
        PageId testId(0x100, 0x0001);
        m_page->setPageId(testId);
        m_page->setPageControlInfo(0xFF);

        // Get and modify a row
        DecodedPageRow& row = m_page->getRow(0);
        row.m_levelOnePageSegment.m_charArray[0] = 'A';

        std::size_t rowCount = m_page->getRowCount();
        CPPUNIT_ASSERT_EQUAL(MAX_ROWS, rowCount);
    }

    void testGetRowAtIndexZero()
    {
        DecodedPageRow& row = m_page->getRow(0);

        // Verify we got a valid reference
        CPPUNIT_ASSERT_EQUAL(LEVEL_ONE_PAGE_WIDTH,
                           row.m_levelOnePageSegment.getWidth());
    }

    void testGetRowAtIndexMax()
    {
        DecodedPageRow& row = m_page->getRow(MAX_ROWS - 1);

        // Verify we got a valid reference
        CPPUNIT_ASSERT_EQUAL(LEVEL_ONE_PAGE_WIDTH,
                           row.m_levelOnePageSegment.getWidth());
    }

    void testGetRowAtMiddleIndex()
    {
        DecodedPageRow& row = m_page->getRow(12);

        // Verify we got a valid reference
        CPPUNIT_ASSERT_EQUAL(LEVEL_ONE_PAGE_WIDTH,
                           row.m_levelOnePageSegment.getWidth());
    }

    void testGetRowNonConstAndModify()
    {
        DecodedPageRow& row = m_page->getRow(5);

        // Modify the row
        row.m_levelOnePageSegment.m_charArray[0] = 0x1234;
        row.m_levelOnePageSegment.m_bgColorIndexArray[0] = 0xAB;
        row.m_levelOnePageSegment.m_fgColorIndexArray[0] = 0xCD;
        row.m_levelOnePageSegment.m_propertiesArray[0] = 0x5678;

        // Retrieve the same row again and verify modifications persisted
        DecodedPageRow& rowAgain = m_page->getRow(5);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x1234),
                           rowAgain.m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xAB),
                           rowAgain.m_levelOnePageSegment.m_bgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xCD),
                           rowAgain.m_levelOnePageSegment.m_fgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x5678),
                           rowAgain.m_levelOnePageSegment.m_propertiesArray[0]);
    }

    void testGetRowConstVersion()
    {
        // Set some data first
        DecodedPageRow& row = m_page->getRow(3);
        row.m_levelOnePageSegment.m_charArray[10] = 0xABCD;

        // Access through const reference
        const DecodedPage& constPage = *m_page;
        const DecodedPageRow& constRow = constPage.getRow(3);

        // Verify we can read the data
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xABCD),
                           constRow.m_levelOnePageSegment.m_charArray[10]);
    }

    void testGetAllRowsSequentially()
    {
        // Access all rows from 0 to MAX_ROWS-1
        for (std::size_t i = 0; i < MAX_ROWS; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);

            // Verify each row is valid
            CPPUNIT_ASSERT_EQUAL(LEVEL_ONE_PAGE_WIDTH,
                               row.m_levelOnePageSegment.getWidth());

            // Set unique data in each row
            row.m_levelOnePageSegment.m_charArray[0] = static_cast<uint16_t>(i);
        }

        // Verify all rows have their unique data
        for (std::size_t i = 0; i < MAX_ROWS; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(i),
                               row.m_levelOnePageSegment.m_charArray[0]);
        }
    }

    void testGetRowWithIndexEqualsMaxRows()
    {
        bool exceptionThrown = false;
        try
        {
            m_page->getRow(MAX_ROWS);
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testGetRowWithIndexGreaterThanMaxRows()
    {
        bool exceptionThrown = false;
        try
        {
            m_page->getRow(100);
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testGetRowWithVeryLargeIndex()
    {
        bool exceptionThrown = false;
        try
        {
            m_page->getRow(SIZE_MAX);
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testGetRowExceptionMessage()
    {
        bool correctMessage = false;
        try
        {
            m_page->getRow(MAX_ROWS);
        }
        catch (const std::invalid_argument& e)
        {
            std::string message(e.what());
            correctMessage = (message.find("Invalid row index") != std::string::npos);
        }

        CPPUNIT_ASSERT(correctMessage);
    }

    void testGetRowConstVersionWithInvalidIndex()
    {
        const DecodedPage& constPage = *m_page;

        bool exceptionThrown = false;
        try
        {
            constPage.getRow(MAX_ROWS);
        }
        catch (const std::invalid_argument& e)
        {
            exceptionThrown = true;
        }

        CPPUNIT_ASSERT(exceptionThrown);
    }

    void testSetAllFiveColorLinksWithDifferentValues()
    {
        PageId redId(0x100, 0x0001);
        PageId greenId(0x200, 0x0002);
        PageId yellowId(0x300, 0x0003);
        PageId cyanId(0x400, 0x0004);
        PageId flofId(0x500, 0x0005);

        m_page->setColourKeyLink(DecodedPage::Link::RED, redId);
        m_page->setColourKeyLink(DecodedPage::Link::GREEN, greenId);
        m_page->setColourKeyLink(DecodedPage::Link::YELLOW, yellowId);
        m_page->setColourKeyLink(DecodedPage::Link::CYAN, cyanId);
        m_page->setColourKeyLink(DecodedPage::Link::FLOF_INDEX, flofId);

        // Verify each link maintains its own value
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100),
                           m_page->getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200),
                           m_page->getColourKeyLink(DecodedPage::Link::GREEN).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300),
                           m_page->getColourKeyLink(DecodedPage::Link::YELLOW).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x400),
                           m_page->getColourKeyLink(DecodedPage::Link::CYAN).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x500),
                           m_page->getColourKeyLink(DecodedPage::Link::FLOF_INDEX).getMagazinePage());
    }

    void testOverwriteExistingColorLink()
    {
        PageId firstId(0x100, 0x0001);
        PageId secondId(0x200, 0x0002);

        m_page->setColourKeyLink(DecodedPage::Link::GREEN, firstId);
        m_page->setColourKeyLink(DecodedPage::Link::GREEN, secondId);

        const PageId& retrievedLink = m_page->getColourKeyLink(DecodedPage::Link::GREEN);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), retrievedLink.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0002), retrievedLink.getSubpage());
    }

    void testSetColorLinksAfterClear()
    {
        PageId linkId(0x123, 0x0456);
        m_page->setColourKeyLink(DecodedPage::Link::YELLOW, linkId);

        m_page->clear();

        // Verify link is reset after clear
        const PageId& clearedLink = m_page->getColourKeyLink(DecodedPage::Link::YELLOW);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, clearedLink.getMagazinePage());

        // Set new link after clear
        PageId newLinkId(0x789, 0x0ABC);
        m_page->setColourKeyLink(DecodedPage::Link::YELLOW, newLinkId);

        const PageId& retrievedLink = m_page->getColourKeyLink(DecodedPage::Link::YELLOW);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x789), retrievedLink.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0ABC), retrievedLink.getSubpage());
    }

    void testSetColorLinkWithNullPageId()
    {
        PageId nullId(NULL_MAGAZINE_PAGE_MASK, NULL_SUBPAGE);
        m_page->setColourKeyLink(DecodedPage::Link::CYAN, nullId);

        const PageId& retrievedLink = m_page->getColourKeyLink(DecodedPage::Link::CYAN);
        CPPUNIT_ASSERT_EQUAL(NULL_MAGAZINE_PAGE_MASK, retrievedLink.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(NULL_SUBPAGE, retrievedLink.getSubpage());
        CPPUNIT_ASSERT(retrievedLink.isNull());
        CPPUNIT_ASSERT(retrievedLink.isAnySubpage());
    }

    void testSetColorLinkWithBoundaryPageIds()
    {
        PageId minId(0x0000, 0x0000);
        PageId maxId(0xFFFF, 0xFFFF);

        m_page->setColourKeyLink(DecodedPage::Link::RED, minId);
        m_page->setColourKeyLink(DecodedPage::Link::GREEN, maxId);

        const PageId& minLink = m_page->getColourKeyLink(DecodedPage::Link::RED);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0000), minLink.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0000), minLink.getSubpage());

        const PageId& maxLink = m_page->getColourKeyLink(DecodedPage::Link::GREEN);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), maxLink.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), maxLink.getSubpage());
    }

    void testModifyRowDataAndVerifyPageState()
    {
        DecodedPageRow& row3 = m_page->getRow(3);
        DecodedPageRow& row7 = m_page->getRow(7);

        // Modify different rows
        row3.m_levelOnePageSegment.m_charArray[5] = 0xABCD;
        row7.m_levelOnePageSegment.m_charArray[10] = 0x1234;

        // Verify modifications are independent
        DecodedPageRow& row3Again = m_page->getRow(3);
        DecodedPageRow& row7Again = m_page->getRow(7);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xABCD),
                           row3Again.m_levelOnePageSegment.m_charArray[5]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x1234),
                           row7Again.m_levelOnePageSegment.m_charArray[10]);

        // Verify row 3 doesn't have row 7's data
        CPPUNIT_ASSERT(row3Again.m_levelOnePageSegment.m_charArray[10] != 0x1234);
    }

    void testClearPageAfterSettingRows()
    {
        // Set data in multiple rows
        for (std::size_t i = 0; i < 5; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);
            row.m_levelOnePageSegment.m_charArray[0] = 0xFFFF;
            row.m_levelOnePageSegment.m_bgColorIndexArray[0] = 0xAA;
            row.m_levelOnePageSegment.m_fgColorIndexArray[0] = 0xBB;
            row.m_levelOnePageSegment.m_propertiesArray[0] = 0x1234;
        }

        m_page->clear();

        // Verify all rows are cleared
        for (std::size_t i = 0; i < 5; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '),
                               row.m_levelOnePageSegment.m_charArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                               row.m_levelOnePageSegment.m_bgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0),
                               row.m_levelOnePageSegment.m_fgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0),
                               row.m_levelOnePageSegment.m_propertiesArray[0]);
        }
    }

    void testSetMultipleRowsWithDifferentData()
    {
        // Set unique data in each row
        for (std::size_t i = 0; i < MAX_ROWS; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);
            row.m_levelOnePageSegment.m_charArray[0] = static_cast<uint16_t>(i + 100);
            row.m_levelOnePageSegment.m_bgColorIndexArray[0] = static_cast<uint8_t>(i);
            row.m_levelOnePageSegment.m_fgColorIndexArray[0] = static_cast<uint8_t>(i + 50);
            row.m_levelOnePageSegment.m_propertiesArray[0] = static_cast<uint16_t>(i * 10);
        }

        // Verify each row has independent data
        for (std::size_t i = 0; i < MAX_ROWS; ++i)
        {
            DecodedPageRow& row = m_page->getRow(i);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(i + 100),
                               row.m_levelOnePageSegment.m_charArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(i),
                               row.m_levelOnePageSegment.m_bgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(i + 50),
                               row.m_levelOnePageSegment.m_fgColorIndexArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(i * 10),
                               row.m_levelOnePageSegment.m_propertiesArray[0]);
        }
    }

    void testAccessRowDataThroughConstAndNonConstGetters()
    {
        // Set data via non-const getter
        DecodedPageRow& row = m_page->getRow(8);
        row.m_levelOnePageSegment.m_charArray[15] = 0xDEAD;

        // Read data via const getter
        const DecodedPage& constPage = *m_page;
        const DecodedPageRow& constRow = constPage.getRow(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xDEAD),
                           constRow.m_levelOnePageSegment.m_charArray[15]);

        // Verify non-const getter still returns same data
        DecodedPageRow& nonConstRow = m_page->getRow(8);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xDEAD),
                           nonConstRow.m_levelOnePageSegment.m_charArray[15]);
    }

    void testSetAllPageProperties()
    {
        PageId pageId(0x180, 0x0020);
        m_page->setPageId(pageId);
        m_page->setPageControlInfo(0xEE);

        // Set color links
        for (int i = 0; i <= static_cast<int>(DecodedPage::Link::LAST); ++i)
        {
            DecodedPage::Link link = static_cast<DecodedPage::Link>(i);
            PageId linkId(0x100 + i, 0x0010 + i);
            m_page->setColourKeyLink(link, linkId);
        }

        // Set row data
        DecodedPageRow& row = m_page->getRow(5);
        row.m_levelOnePageSegment.m_charArray[5] = 0xBEEF;

        // Verify all properties are independently retrievable
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x180), m_page->getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xEE), m_page->getPageControlInfo());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100),
                           m_page->getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xBEEF),
                           m_page->getRow(5).m_levelOnePageSegment.m_charArray[5]);
    }

    void testClearAndRebuildPageState()
    {
        // Set initial state
        PageId pageId1(0x100, 0x0001);
        m_page->setPageId(pageId1);
        m_page->setPageControlInfo(0xAA);
        m_page->getRow(0).m_levelOnePageSegment.m_charArray[0] = 0x1111;

        // Clear
        m_page->clear();

        // Rebuild with different state
        PageId pageId2(0x200, 0x0002);
        m_page->setPageId(pageId2);
        m_page->setPageControlInfo(0xBB);
        m_page->getRow(0).m_levelOnePageSegment.m_charArray[0] = 0x2222;

        // Verify new state
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), m_page->getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0xBB), m_page->getPageControlInfo());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x2222),
                           m_page->getRow(0).m_levelOnePageSegment.m_charArray[0]);
    }

    void testDumpPageWithDataCharsOnlyTrue()
    {
        PageId pageId(0x180, 0x0025);
        m_page->setPageId(pageId);
        m_page->setPageControlInfo(0x42);

        DecodedPageRow& row = m_page->getRow(0);
        row.m_levelOnePageSegment.m_charArray[0] = 'T';
        row.m_levelOnePageSegment.m_charArray[1] = 'E';
        row.m_levelOnePageSegment.m_charArray[2] = 'S';
        row.m_levelOnePageSegment.m_charArray[3] = 'T';

        PageId beforeId = m_page->getPageId();
        std::uint8_t beforeControlInfo = m_page->getPageControlInfo();
        std::uint16_t beforeFirstChar = row.m_levelOnePageSegment.m_charArray[0];

        m_page->dump(true);

        CPPUNIT_ASSERT_EQUAL(beforeId.getMagazinePage(), m_page->getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(beforeId.getSubpage(), m_page->getPageId().getSubpage());
        CPPUNIT_ASSERT_EQUAL(beforeControlInfo, m_page->getPageControlInfo());
        CPPUNIT_ASSERT_EQUAL(beforeFirstChar, m_page->getRow(0).m_levelOnePageSegment.m_charArray[0]);
    }

    void testDumpPageWithDataCharsOnlyFalse()
    {
        PageId pageId(0x180, 0x0025);
        m_page->setPageId(pageId);
        m_page->setPageControlInfo(0x42);

        DecodedPageRow& row = m_page->getRow(0);
        row.m_levelOnePageSegment.m_charArray[0] = 'T';
        row.m_levelOnePageSegment.m_bgColorIndexArray[0] = 0x01;
        row.m_levelOnePageSegment.m_fgColorIndexArray[0] = 0x07;
        row.m_levelOnePageSegment.m_propertiesArray[0] = 0x0010;

        PageId linkId(0x100, 0x0001);
        m_page->setColourKeyLink(DecodedPage::Link::RED, linkId);

        PageId beforeId = m_page->getPageId();
        std::uint8_t beforeControlInfo = m_page->getPageControlInfo();
        std::uint16_t beforeChar = row.m_levelOnePageSegment.m_charArray[0];
        std::uint8_t beforeBg = row.m_levelOnePageSegment.m_bgColorIndexArray[0];
        std::uint8_t beforeFg = row.m_levelOnePageSegment.m_fgColorIndexArray[0];
        std::uint16_t beforeProperties = row.m_levelOnePageSegment.m_propertiesArray[0];
        PageId beforeLink = m_page->getColourKeyLink(DecodedPage::Link::RED);

        m_page->dump(false);

        CPPUNIT_ASSERT_EQUAL(beforeId.getMagazinePage(), m_page->getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(beforeId.getSubpage(), m_page->getPageId().getSubpage());
        CPPUNIT_ASSERT_EQUAL(beforeControlInfo, m_page->getPageControlInfo());
        CPPUNIT_ASSERT_EQUAL(beforeChar, m_page->getRow(0).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(beforeBg, m_page->getRow(0).m_levelOnePageSegment.m_bgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(beforeFg, m_page->getRow(0).m_levelOnePageSegment.m_fgColorIndexArray[0]);
        CPPUNIT_ASSERT_EQUAL(beforeProperties, m_page->getRow(0).m_levelOnePageSegment.m_propertiesArray[0]);
        CPPUNIT_ASSERT_EQUAL(beforeLink.getMagazinePage(),
                           m_page->getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(beforeLink.getSubpage(),
                           m_page->getColourKeyLink(DecodedPage::Link::RED).getSubpage());
    }

    void testDumpPageWithSpecialCharacters()
    {
        DecodedPageRow& row = m_page->getRow(1);

        // Set special characters (non-printable)
        row.m_levelOnePageSegment.m_charArray[0] = 0x0000; // NULL
        row.m_levelOnePageSegment.m_charArray[1] = 0x001F; // Below printable range
        row.m_levelOnePageSegment.m_charArray[2] = 0x0080; // Above printable range
        row.m_levelOnePageSegment.m_charArray[3] = 0xFFFF; // Max value
        row.m_levelOnePageSegment.m_charArray[4] = 0x0041; // 'A' - printable

        std::uint16_t beforeChar0 = row.m_levelOnePageSegment.m_charArray[0];
        std::uint16_t beforeChar1 = row.m_levelOnePageSegment.m_charArray[1];
        std::uint16_t beforeChar2 = row.m_levelOnePageSegment.m_charArray[2];
        std::uint16_t beforeChar3 = row.m_levelOnePageSegment.m_charArray[3];
        std::uint16_t beforeChar4 = row.m_levelOnePageSegment.m_charArray[4];

        m_page->dump(true);
        m_page->dump(false);

        CPPUNIT_ASSERT_EQUAL(beforeChar0, m_page->getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(beforeChar1, m_page->getRow(1).m_levelOnePageSegment.m_charArray[1]);
        CPPUNIT_ASSERT_EQUAL(beforeChar2, m_page->getRow(1).m_levelOnePageSegment.m_charArray[2]);
        CPPUNIT_ASSERT_EQUAL(beforeChar3, m_page->getRow(1).m_levelOnePageSegment.m_charArray[3]);
        CPPUNIT_ASSERT_EQUAL(beforeChar4, m_page->getRow(1).m_levelOnePageSegment.m_charArray[4]);
    }

    void testDumpAfterClear()
    {
        // Set some data
        PageId pageId(0x180, 0x0025);
        m_page->setPageId(pageId);
        m_page->setPageControlInfo(0x42);
        m_page->getRow(0).m_levelOnePageSegment.m_charArray[0] = 'X';

        // Clear and dump
        m_page->clear();
        PageId beforeId = m_page->getPageId();
        std::uint8_t beforeControlInfo = m_page->getPageControlInfo();
        std::uint16_t beforeChar = m_page->getRow(0).m_levelOnePageSegment.m_charArray[0];
        m_page->dump(true);
        m_page->dump(false);

        CPPUNIT_ASSERT_EQUAL(beforeId.getMagazinePage(), m_page->getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(beforeId.getSubpage(), m_page->getPageId().getSubpage());
        CPPUNIT_ASSERT_EQUAL(beforeControlInfo, m_page->getPageControlInfo());
        CPPUNIT_ASSERT_EQUAL(beforeChar, m_page->getRow(0).m_levelOnePageSegment.m_charArray[0]);
    }

    void testCreateAndDestroyMultipleInstances()
    {
        const int NUM_INSTANCES = 100;

        for (int i = 0; i < NUM_INSTANCES; ++i)
        {
            std::unique_ptr<DecodedPage> page = std::make_unique<DecodedPage>();

            // Set some data
            PageId pageId(0x100 + i, 0x0001);
            page->setPageId(pageId);
            page->setPageControlInfo(static_cast<uint8_t>(i));

            // Verify data
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100 + i),
                               page->getPageId().getMagazinePage());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0001),
                               page->getPageId().getSubpage());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(i),
                               page->getPageControlInfo());
        }
    }

    void testLargeNumberOfStateChanges()
    {
        const int NUM_ITERATIONS = 1000;

        for (int i = 0; i < NUM_ITERATIONS; ++i)
        {
            PageId pageId(i % 0xFFFF, (i * 2) % 0xFFFF);
            m_page->setPageId(pageId);
            m_page->setPageControlInfo(static_cast<uint8_t>(i % 256));

            // Modify a row
            DecodedPageRow& row = m_page->getRow(i % MAX_ROWS);
            row.m_levelOnePageSegment.m_charArray[0] = static_cast<uint16_t>(i);

            // Set a color link
            DecodedPage::Link link = static_cast<DecodedPage::Link>(i % 5);
            PageId linkId(i % 0x800, i % 0x4000);
            m_page->setColourKeyLink(link, linkId);

            CPPUNIT_ASSERT_EQUAL(pageId.getMagazinePage(), m_page->getPageId().getMagazinePage());
            CPPUNIT_ASSERT_EQUAL(pageId.getSubpage(), m_page->getPageId().getSubpage());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(i % 256), m_page->getPageControlInfo());
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(i),
                               m_page->getRow(i % MAX_ROWS).m_levelOnePageSegment.m_charArray[0]);
            CPPUNIT_ASSERT_EQUAL(linkId.getMagazinePage(),
                               m_page->getColourKeyLink(link).getMagazinePage());
            CPPUNIT_ASSERT_EQUAL(linkId.getSubpage(),
                               m_page->getColourKeyLink(link).getSubpage());

            // Occasionally clear
            if (i % 100 == 0)
            {
                m_page->clear();

                CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE,
                                   m_page->getPageId().getMagazinePage());
                CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE,
                                   m_page->getPageId().getSubpage());
                CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), m_page->getPageControlInfo());
                CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '),
                                   m_page->getRow(i % MAX_ROWS).m_levelOnePageSegment.m_charArray[0]);
                CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE,
                                   m_page->getColourKeyLink(link).getMagazinePage());
                CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE,
                                   m_page->getColourKeyLink(link).getSubpage());
            }
        }
    }

private:
    std::unique_ptr<DecodedPage> m_page;
};

CPPUNIT_TEST_SUITE_REGISTRATION( DecodedPageTest );
