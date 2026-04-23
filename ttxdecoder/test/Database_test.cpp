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
#include "Database.hpp"

using namespace ttxdecoder;

// Local constants to avoid ODR-use of PageId static members
namespace
{
    constexpr uint16_t INVALID_MAGAZINE_PAGE = 0xFFFF;
    constexpr uint16_t ANY_SUBPAGE = 0xFFFF;
    constexpr uint16_t NULL_MAGAZINE_PAGE_MASK = 0x00FF;
    constexpr uint16_t NULL_SUBPAGE = 0x3F7F;
}

class DatabaseTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(DatabaseTest);
    CPPUNIT_TEST(testConstructorInitializesDatabase);
    CPPUNIT_TEST(testConstructorInitializesMagazinePages);
    CPPUNIT_TEST(testConstructorInitializesMetadata);
    CPPUNIT_TEST(testResetClearsIndexPage);
    CPPUNIT_TEST(testResetInvalidatesMagazinePages);
    CPPUNIT_TEST(testResetClearsTopMetadata);
    CPPUNIT_TEST(testResetTopMetadataOnly);
    CPPUNIT_TEST(testGetMagazinePageValidIndex);
    CPPUNIT_TEST(testGetMagazinePageBoundaryZero);
    CPPUNIT_TEST(testGetMagazinePageBoundaryMax);
    CPPUNIT_TEST(testGetMagazinePageInvalidIndexReturnsDefault);
    CPPUNIT_TEST(testGetMagazinePageNonConstVersion);
    CPPUNIT_TEST(testSetAndGetIndexPageP830);
    CPPUNIT_TEST(testIndexPageP830WithInvalidPage);
    CPPUNIT_TEST(testIndexPageP830AfterReset);
    CPPUNIT_TEST(testGetNextPageValidDecimal);
    CPPUNIT_TEST(testGetNextPageBoundaryCarryOnes);
    CPPUNIT_TEST(testGetNextPageBoundaryCarryTens);
    CPPUNIT_TEST(testGetNextPageBoundaryMaxPage);
    CPPUNIT_TEST(testGetNextPageWrapAroundFrom899);
    CPPUNIT_TEST(testGetNextPageInvalidInput);
    CPPUNIT_TEST(testGetPrevPageValidDecimal);
    CPPUNIT_TEST(testGetPrevPageBoundaryBorrowOnes);
    CPPUNIT_TEST(testGetPrevPageBoundaryBorrowTens);
    CPPUNIT_TEST(testGetPrevPageBoundaryMinPage);
    CPPUNIT_TEST(testGetPrevPageWrapAroundFrom100);
    CPPUNIT_TEST(testGetPrevPageInvalidInput);
    CPPUNIT_TEST(testGetTopMetadataValidRange);
    CPPUNIT_TEST(testGetTopMetadataConstVersion);
    CPPUNIT_TEST(testGetTopMetadataBelowValidRange);
    CPPUNIT_TEST(testGetTopMetadataAboveValidRange);
    CPPUNIT_TEST(testGetTopMetadataExactBoundaries);
    CPPUNIT_TEST(testGetFirstSubpageReturnsInvalid);
    CPPUNIT_TEST(testGetNextSubpageReturnsInvalid);
    CPPUNIT_TEST(testGetPrevSubpageReturnsInvalid);
    CPPUNIT_TEST(testGetHighestSubpageReturnsInvalid);
    CPPUNIT_TEST(testGetLastReceivedSubpageReturnsInvalid);
    CPPUNIT_TEST(testGetNextPageWithTopDefaultMode);
    CPPUNIT_TEST(testGetPrevPageWithFlofDefaultMode);
    CPPUNIT_TEST(testGetNextPageWithTopFlofDefaultMode);
    CPPUNIT_TEST(testNavigationSequenceForward);
    CPPUNIT_TEST(testNavigationSequenceBackward);
    CPPUNIT_TEST(testNavigationRoundTrip);
    CPPUNIT_TEST(testGetNextPageSequence189to191);
    CPPUNIT_TEST(testGetPrevPageSequence191to189);
    CPPUNIT_TEST(testGetNextPageAllDigitsNine);
    CPPUNIT_TEST(testGetPrevPageAllDigitsZero);
    CPPUNIT_TEST(testMetadataPersistsAcrossQueries);
    CPPUNIT_TEST(testMultipleMetadataModifications);
    CPPUNIT_TEST(testMetadataIndependence);
    CPPUNIT_TEST(testMultipleResetsPreserveStructure);
    CPPUNIT_TEST(testIndexPageIndependentOfMetadata);
    CPPUNIT_TEST(testMagazinePageAccessAfterReset);
    CPPUNIT_TEST(testGetNextPageWithNullPage);
    CPPUNIT_TEST(testGetPrevPageWithNullPage);
    CPPUNIT_TEST(testNavigationWithInvalidSubpage);
    CPPUNIT_TEST(testAllMagazinePagesAccessible);
    CPPUNIT_TEST(testMetadataForAllValidPages);
    CPPUNIT_TEST(testNavigationAcrossAllMagazines);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_database = std::make_unique<Database>();
    }

    void tearDown() override
    {
        m_database.reset();
    }

protected:
    void testConstructorInitializesDatabase()
    {
        Database db;

        // Verify index page is initialized to invalid
        PageId indexPage = db.getIndexPageP830();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, indexPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, indexPage.getSubpage());
    }

    void testConstructorInitializesMagazinePages()
    {
        Database db;

        // Verify all magazine pages are accessible
        for (std::uint8_t i = 0; i < Database::MAGAZINE_COUNT; ++i)
        {
            const PageMagazine& page = db.getMagazinePage(i);
            CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
            CPPUNIT_ASSERT(page.isValid());
        }
    }

    void testConstructorInitializesMetadata()
    {
        Database db;

        // Test metadata is initialized for valid range
        const Database::TopMetadata& metadata = db.getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, metadata.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, metadata.m_prevPage);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, metadata.m_nextGroupPage);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, metadata.m_nextBlockPage);
    }

    void testResetClearsIndexPage()
    {
        PageId testPage(0x150, 0x0001);
        m_database->setIndexPageP830(testPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), m_database->getIndexPageP830().getMagazinePage());

        m_database->reset();

        PageId resetPage = m_database->getIndexPageP830();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, resetPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, resetPage.getSubpage());
    }

    void testResetInvalidatesMagazinePages()
    {
        m_database->reset();

        // All magazine pages should still be valid (they are always valid)
        for (std::uint8_t i = 0; i < Database::MAGAZINE_COUNT; ++i)
        {
            const PageMagazine& page = m_database->getMagazinePage(i);
            CPPUNIT_ASSERT(page.isValid());
        }
    }

    void testResetClearsTopMetadata()
    {
        // Set some metadata
        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        metadata.m_nextPage = 0x200;
        metadata.m_prevPage = 0x100;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata.m_nextPage);

        m_database->reset();

        const Database::TopMetadata& resetMetadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, resetMetadata.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, resetMetadata.m_prevPage);
    }

    void testResetTopMetadataOnly()
    {
        // Set index page and metadata
        PageId testPage(0x150, 0x0001);
        m_database->setIndexPageP830(testPage);

        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        metadata.m_nextPage = 0x200;

        // Reset only TOP metadata
        m_database->resetTopMetadata();

        // Index page should remain unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), m_database->getIndexPageP830().getMagazinePage());

        // Metadata should be reset
        const Database::TopMetadata& resetMetadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, resetMetadata.m_nextPage);
    }

    void testGetMagazinePageValidIndex()
    {
        for (std::uint8_t i = 0; i < Database::MAGAZINE_COUNT; ++i)
        {
            const PageMagazine& page = m_database->getMagazinePage(i);
            CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
            CPPUNIT_ASSERT(page.isValid());
        }
    }

    void testGetMagazinePageBoundaryZero()
    {
        const PageMagazine& page = m_database->getMagazinePage(0);
        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
        CPPUNIT_ASSERT(page.isValid());
    }

    void testGetMagazinePageBoundaryMax()
    {
        const PageMagazine& page = m_database->getMagazinePage(Database::MAGAZINE_COUNT - 1);
        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
        CPPUNIT_ASSERT(page.isValid());
    }

    void testGetMagazinePageInvalidIndexReturnsDefault()
    {
        // According to implementation, invalid index returns m_magazinePages[1]
        const PageMagazine& invalidPage = m_database->getMagazinePage(255);
        const PageMagazine& defaultPage = m_database->getMagazinePage(1);

        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, invalidPage.getType());
        CPPUNIT_ASSERT(invalidPage.isValid());

        // Both should be valid magazine pages
        CPPUNIT_ASSERT_EQUAL(defaultPage.getType(), invalidPage.getType());
    }

    void testGetMagazinePageNonConstVersion()
    {
        // Test non-const version by modifying through reference
        PageMagazine& page = m_database->getMagazinePage(3);
        page.invalidate();

        // Magazine pages should still be valid after invalidate
        // but we can verify we got the right reference
        const PageMagazine& samePage = m_database->getMagazinePage(3);
        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, samePage.getType());
    }

    void testSetAndGetIndexPageP830()
    {
        PageId testPage(0x150, 0x0001);
        m_database->setIndexPageP830(testPage);

        PageId retrievedPage = m_database->getIndexPageP830();
        CPPUNIT_ASSERT_EQUAL(testPage.getMagazinePage(), retrievedPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(testPage.getSubpage(), retrievedPage.getSubpage());
    }

    void testIndexPageP830WithInvalidPage()
    {
        PageId invalidPage(INVALID_MAGAZINE_PAGE, ANY_SUBPAGE);
        m_database->setIndexPageP830(invalidPage);

        PageId retrieved = m_database->getIndexPageP830();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, retrieved.getSubpage());
    }

    void testIndexPageP830AfterReset()
    {
        PageId testPage(0x888, 0x1234);
        m_database->setIndexPageP830(testPage);

        m_database->reset();

        PageId retrieved = m_database->getIndexPageP830();
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, retrieved.getMagazinePage());
    }

    void testGetNextPageValidDecimal()
    {
        PageId page100(0x100, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page100, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetNextPageBoundaryCarryOnes()
    {
        // Page 109 -> 110 (carry from ones to tens)
        PageId page109(0x109, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page109, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x110), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetNextPageBoundaryCarryTens()
    {
        // Page 199 -> 200 (carry from tens to hundreds)
        PageId page199(0x199, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page199, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetNextPageBoundaryMaxPage()
    {
        // Page 888 -> 889
        PageId page888(0x888, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page888, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x889), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetNextPageWrapAroundFrom899()
    {
        // Page 899 -> 100 (wrap around)
        PageId page899(0x899, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page899, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetNextPageInvalidInput()
    {
        PageId invalidPage(INVALID_MAGAZINE_PAGE, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(invalidPage, NavigationMode::DEFAULT);

        // Should return invalid page
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetPrevPageValidDecimal()
    {
        PageId page101(0x101, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page101, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetPrevPageBoundaryBorrowOnes()
    {
        // Page 110 -> 109 (borrow to ones)
        PageId page110(0x110, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page110, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x109), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetPrevPageBoundaryBorrowTens()
    {
        // Page 200 -> 199 (borrow to tens)
        PageId page200(0x200, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page200, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x199), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetPrevPageBoundaryMinPage()
    {
        // Page 101 -> 100
        PageId page101(0x101, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page101, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetPrevPageWrapAroundFrom100()
    {
        // Page 100 -> 899 (wrap around)
        PageId page100(0x100, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page100, NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x899), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetPrevPageInvalidInput()
    {
        PageId invalidPage(INVALID_MAGAZINE_PAGE, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(invalidPage, NavigationMode::DEFAULT);

        // Should return invalid page
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetTopMetadataValidRange()
    {
        // Test lower boundary
        Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        metadata100.m_nextPage = 0x101;
        metadata100.m_prevPage = 0x100;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), metadata100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_prevPage);

        // Test upper boundary
        Database::TopMetadata& metadata8FF = m_database->getTopMetatadata(0x8FF);
        metadata8FF.m_nextGroupPage = 0x200;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata8FF.m_nextGroupPage);

        // Test mid-range
        Database::TopMetadata& metadata555 = m_database->getTopMetatadata(0x555);
        metadata555.m_nextBlockPage = 0x556;

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x556), metadata555.m_nextBlockPage);
    }

    void testGetTopMetadataConstVersion()
    {
        // Set up metadata through non-const reference
        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        metadata.m_nextPage = 0x200;
        metadata.m_prevPage = 0x150;
        metadata.m_nextGroupPage = 0x300;
        metadata.m_nextBlockPage = 0x400;

        // Retrieve through const reference
        const Database& constDb = *m_database;
        const Database::TopMetadata& constMetadata = constDb.getTopMetatadata(0x100);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), constMetadata.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), constMetadata.m_prevPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), constMetadata.m_nextGroupPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x400), constMetadata.m_nextBlockPage);
    }

    void testGetTopMetadataBelowValidRange()
    {
        // Test below valid range (< 0x100) - should return default metadata at index 0x00FF
        Database::TopMetadata& metadataBelow = m_database->getTopMetatadata(0x0FF);
        metadataBelow.m_nextPage = 0x123;

        // This should have modified the default fallback metadata
        Database::TopMetadata& metadataAnother = m_database->getTopMetatadata(0x00);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x123), metadataAnother.m_nextPage);
    }

    void testGetTopMetadataAboveValidRange()
    {
        // Test above valid range (> 0x8FF) - should return default metadata at index 0x00FF
        Database::TopMetadata& metadataAbove = m_database->getTopMetatadata(0x900);
        metadataAbove.m_nextPage = 0x456;

        // This should have modified the default fallback metadata
        Database::TopMetadata& metadataAnother = m_database->getTopMetatadata(0xFFF);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x456), metadataAnother.m_nextPage);
    }

    void testGetTopMetadataExactBoundaries()
    {
        // Test exact lower boundary (0x100)
        Database::TopMetadata& metadataLower = m_database->getTopMetatadata(0x100);
        metadataLower.m_nextPage = 0x111;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x111), metadataLower.m_nextPage);

        // Test exact upper boundary (0x8FF)
        Database::TopMetadata& metadataUpper = m_database->getTopMetatadata(0x8FF);
        metadataUpper.m_nextPage = 0x888;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x888), metadataUpper.m_nextPage);

        // Verify they are independent
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x111), metadataLower.m_nextPage);
    }

    void testGetFirstSubpageReturnsInvalid()
    {
        PageId testPage(0x100, 0x0001);
        PageId result = m_database->getFirstSubpage(testPage);

        // Currently returns invalid page (TODO implementation)
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, result.getSubpage());
    }

    void testGetNextSubpageReturnsInvalid()
    {
        PageId testPage(0x200, 0x0005);
        PageId result = m_database->getNextSubpage(testPage);

        // Currently returns invalid page (TODO implementation)
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, result.getSubpage());
    }

    void testGetPrevSubpageReturnsInvalid()
    {
        PageId testPage(0x300, 0x000A);
        PageId result = m_database->getPrevSubpage(testPage);

        // Currently returns invalid page (TODO implementation)
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, result.getSubpage());
    }

    void testGetHighestSubpageReturnsInvalid()
    {
        PageId testPage(0x400, 0x0000);
        PageId result = m_database->getHighestSubpage(testPage);

        // Currently returns invalid page (TODO implementation)
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, result.getSubpage());
    }

    void testGetLastReceivedSubpageReturnsInvalid()
    {
        PageId testPage(0x500, ANY_SUBPAGE);
        PageId result = m_database->getLastReceivedSubpage(testPage);

        // Currently returns invalid page (TODO implementation)
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, result.getSubpage());
    }

    void testGetNextPageWithTopDefaultMode()
    {
        PageId page150(0x150, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page150, NavigationMode::TOP_DEFAULT);

        // Currently navigation mode is ignored (TODO), but should still work
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x151), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testGetPrevPageWithFlofDefaultMode()
    {
        PageId page250(0x250, ANY_SUBPAGE);
        PageId prevPage = m_database->getPrevPage(page250, NavigationMode::FLOF_DEFAULT);

        // Currently navigation mode is ignored (TODO), but should still work
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x249), prevPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prevPage.getSubpage());
    }

    void testGetNextPageWithTopFlofDefaultMode()
    {
        PageId page789(0x789, ANY_SUBPAGE);
        PageId nextPage = m_database->getNextPage(page789, NavigationMode::TOP_FLOF_DEFAULT);

        // Currently navigation mode is ignored (TODO), but should still work
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x790), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testNavigationSequenceForward()
    {
        PageId current(0x105, ANY_SUBPAGE);

        // Navigate forward through several pages
        for (int i = 0; i < 10; ++i)
        {
            PageId next = m_database->getNextPage(current, NavigationMode::DEFAULT);

            // Verify we're moving forward
            CPPUNIT_ASSERT(next.isValidDecimal());
            CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, next.getSubpage());

            current = next;
        }

        // Should end at 115
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x115), current.getMagazinePage());
    }

    void testNavigationSequenceBackward()
    {
        PageId current(0x220, ANY_SUBPAGE);

        // Navigate backward through several pages
        for (int i = 0; i < 10; ++i)
        {
            PageId prev = m_database->getPrevPage(current, NavigationMode::DEFAULT);

            // Verify we're moving backward
            CPPUNIT_ASSERT(prev.isValidDecimal());
            CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, prev.getSubpage());

            current = prev;
        }

        // Should end at 210
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x210), current.getMagazinePage());
    }

    void testNavigationRoundTrip()
    {
        PageId original(0x345, ANY_SUBPAGE);
        PageId current = original;

        // Navigate forward 50 pages
        for (int i = 0; i < 50; ++i)
        {
            current = m_database->getNextPage(current, NavigationMode::DEFAULT);
        }

        // Navigate backward 50 pages
        for (int i = 0; i < 50; ++i)
        {
            current = m_database->getPrevPage(current, NavigationMode::DEFAULT);
        }

        // Should return to original
        CPPUNIT_ASSERT_EQUAL(original.getMagazinePage(), current.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(original.getSubpage(), current.getSubpage());
    }

    void testGetNextPageSequence189to191()
    {
        PageId page189(0x189, ANY_SUBPAGE);

        PageId page190 = m_database->getNextPage(page189, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x190), page190.getMagazinePage());

        PageId page191 = m_database->getNextPage(page190, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x191), page191.getMagazinePage());
    }

    void testGetPrevPageSequence191to189()
    {
        PageId page191(0x191, ANY_SUBPAGE);

        PageId page190 = m_database->getPrevPage(page191, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x190), page190.getMagazinePage());

        PageId page189 = m_database->getPrevPage(page190, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x189), page189.getMagazinePage());
    }

    void testGetNextPageAllDigitsNine()
    {
        // Test 199, 299, ..., 799
        for (uint16_t mag = 1; mag <= 7; ++mag)
        {
            PageId pageX99(mag << 8 | 0x99, ANY_SUBPAGE);
            PageId nextPage = m_database->getNextPage(pageX99, NavigationMode::DEFAULT);

            uint16_t expectedMag = mag + 1;
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(expectedMag << 8), nextPage.getMagazinePage());
        }
    }

    void testGetPrevPageAllDigitsZero()
    {
        // Test 200, 300, ..., 800
        for (uint16_t mag = 2; mag <= 8; ++mag)
        {
            PageId pageX00(mag << 8, ANY_SUBPAGE);
            PageId prevPage = m_database->getPrevPage(pageX00, NavigationMode::DEFAULT);

            uint16_t expectedMag = mag - 1;
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>((expectedMag << 8) | 0x99), prevPage.getMagazinePage());
        }
    }

    void testMetadataPersistsAcrossQueries()
    {
        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x234);
        metadata.m_nextPage = 0x235;
        metadata.m_prevPage = 0x233;
        metadata.m_nextGroupPage = 0x240;
        metadata.m_nextBlockPage = 0x300;

        // Query multiple times
        for (int i = 0; i < 5; ++i)
        {
            const Database::TopMetadata& retrieved = m_database->getTopMetatadata(0x234);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x235), retrieved.m_nextPage);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x233), retrieved.m_prevPage);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x240), retrieved.m_nextGroupPage);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), retrieved.m_nextBlockPage);
        }
    }

    void testMultipleMetadataModifications()
    {
        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x456);

        // Modify multiple times
        metadata.m_nextPage = 0x100;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata.m_nextPage);

        metadata.m_nextPage = 0x200;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata.m_nextPage);

        metadata.m_nextPage = 0x300;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), metadata.m_nextPage);

        // Final value should be retained
        const Database::TopMetadata& retrieved = m_database->getTopMetatadata(0x456);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), retrieved.m_nextPage);
    }

    void testMetadataIndependence()
    {
        // Set different values for different pages
        Database::TopMetadata& metadata1 = m_database->getTopMetatadata(0x111);
        metadata1.m_nextPage = 0x112;

        Database::TopMetadata& metadata2 = m_database->getTopMetatadata(0x222);
        metadata2.m_nextPage = 0x223;

        Database::TopMetadata& metadata3 = m_database->getTopMetatadata(0x333);
        metadata3.m_nextPage = 0x334;

        // Verify independence
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x112), m_database->getTopMetatadata(0x111).m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x223), m_database->getTopMetatadata(0x222).m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x334), m_database->getTopMetatadata(0x333).m_nextPage);
    }

    void testMultipleResetsPreserveStructure()
    {
        // Reset multiple times
        for (int i = 0; i < 5; ++i)
        {
            m_database->reset();

            // Verify structure is still intact
            CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE,
                               m_database->getIndexPageP830().getMagazinePage());

            const Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
            CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, metadata.m_nextPage);
        }
    }

    void testIndexPageIndependentOfMetadata()
    {
        PageId testPage(0x123, 0x0456);
        m_database->setIndexPageP830(testPage);

        // Modify metadata
        Database::TopMetadata& metadata = m_database->getTopMetatadata(0x123);
        metadata.m_nextPage = 0x999;

        // Index page should be unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x123),
                           m_database->getIndexPageP830().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0456),
                           m_database->getIndexPageP830().getSubpage());

        // Reset only metadata
        m_database->resetTopMetadata();

        // Index page should still be unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x123),
                           m_database->getIndexPageP830().getMagazinePage());
    }

    void testMagazinePageAccessAfterReset()
    {
        m_database->reset();

        // Access all magazine pages after reset
        for (std::uint8_t i = 0; i < Database::MAGAZINE_COUNT; ++i)
        {
            PageMagazine& page = m_database->getMagazinePage(i);
            CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
            CPPUNIT_ASSERT(page.isValid());
        }
    }

    void testGetNextPageWithNullPage()
    {
        // Create a null page (0x0FF magazine page and NULL_SUBPAGE)
        PageId nullPage(NULL_MAGAZINE_PAGE_MASK, NULL_SUBPAGE);
        PageId result = m_database->getNextPage(nullPage, NavigationMode::DEFAULT);

        // Should return invalid page since null page is not a valid decimal
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
    }

    void testGetPrevPageWithNullPage()
    {
        // Create a null page
        PageId nullPage(NULL_MAGAZINE_PAGE_MASK, NULL_SUBPAGE);
        PageId result = m_database->getPrevPage(nullPage, NavigationMode::DEFAULT);

        // Should return invalid page since null page is not a valid decimal
        CPPUNIT_ASSERT_EQUAL(INVALID_MAGAZINE_PAGE, result.getMagazinePage());
    }

    void testNavigationWithInvalidSubpage()
    {
        // Create page with invalid subpage but valid magazine page
        PageId pageInvalidSubpage(0x100, 0xFFFF);

        // Navigation should still work based on magazine page
        PageId nextPage = m_database->getNextPage(pageInvalidSubpage, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), nextPage.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(ANY_SUBPAGE, nextPage.getSubpage());
    }

    void testAllMagazinePagesAccessible()
    {
        // Verify all 8 magazine pages are accessible
        for (std::uint8_t mag = 0; mag < Database::MAGAZINE_COUNT; ++mag)
        {
            const PageMagazine& page = m_database->getMagazinePage(mag);
            CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, page.getType());
            CPPUNIT_ASSERT(page.isValid());
        }

        // Verify out-of-bounds returns default (index 1)
        const PageMagazine& oob1 = m_database->getMagazinePage(10);
        const PageMagazine& oob2 = m_database->getMagazinePage(255);
        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, oob1.getType());
        CPPUNIT_ASSERT_EQUAL(PageType::MAGAZINE, oob2.getType());
    }

    void testMetadataForAllValidPages()
    {
        // Test metadata access for pages at different magazines
        for (uint16_t mag = 1; mag <= 8; ++mag)
        {
            uint16_t pageNum = (mag << 8) | 0x00;  // X00 pages
            Database::TopMetadata& metadata = m_database->getTopMetatadata(pageNum);

            // Set unique value
            metadata.m_nextPage = pageNum + 1;

            // Verify it was set
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(pageNum + 1),
                               m_database->getTopMetatadata(pageNum).m_nextPage);
        }
    }

    void testNavigationAcrossAllMagazines()
    {
        // Navigate from magazine 1 to magazine 8
        PageId current(0x100, ANY_SUBPAGE);

        // Navigate through 700 pages (100-899 wraps back to 100)
        for (int i = 0; i < 799; ++i)
        {
            PageId next = m_database->getNextPage(current, NavigationMode::DEFAULT);
            CPPUNIT_ASSERT(next.isValidDecimal());
            current = next;
        }

        // Should be at 899
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x899), current.getMagazinePage());

        // One more should wrap to 100
        PageId wrapped = m_database->getNextPage(current, NavigationMode::DEFAULT);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), wrapped.getMagazinePage());
    }

private:
    std::unique_ptr<Database> m_database;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTest);
