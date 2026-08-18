/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
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
*****************************************************************************/


#include <cppunit/extensions/HelperMacros.h>
#include <vector>

#include "CacheImpl.hpp"

using ttxdecoder::CacheImpl;
using ttxdecoder::PageId;

class CacheImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( CacheImplTest );
    CPPUNIT_TEST(testPageReleasedDuringUpdate);
    CPPUNIT_TEST(testGetPageNotFound);
    CPPUNIT_TEST(testSetCurrentPageInvalid);
    CPPUNIT_TEST(testInsertDuplicatePage);
    CPPUNIT_TEST(testGetClearPageBasic);
    CPPUNIT_TEST(testGetPageBasic);
    CPPUNIT_TEST(testConstructorMinimumBuffer);
    CPPUNIT_TEST(testGetMutablePageBasic);
    CPPUNIT_TEST(testGetNewestSubpageBasic);
    CPPUNIT_TEST(testReleasePageBasic);
    CPPUNIT_TEST(testSetLinkedPagesBasic);
    CPPUNIT_TEST(testSetLinkedPagesEmpty);
    CPPUNIT_TEST(testPageIdBoundaries);
    CPPUNIT_TEST(testGetNewestSubpageNotFound);
    CPPUNIT_TEST(testMultipleClearPageCalls);
    CPPUNIT_TEST(testSetLinkedPagesOverflow);
    CPPUNIT_TEST(testIsPageNeededCurrent);
    CPPUNIT_TEST(testIsPageNeededBasic);
    CPPUNIT_TEST(testIsPageNeededCacheRange);
    CPPUNIT_TEST(testIsPageNeededWrapRange);
    CPPUNIT_TEST(testIsPageNeededLinkedPages);
    CPPUNIT_TEST(testLinkedPagesGate);
    CPPUNIT_TEST(testIsPageNeededInvalidDecimal);
    CPPUNIT_TEST(testIsPageNeededBoundaryRange);
    CPPUNIT_TEST(testConstructorZeroSize);
    CPPUNIT_TEST(testPageIdNull);
    CPPUNIT_TEST(testPageIdInvalid);
    CPPUNIT_TEST(testPageIdMaxValues);
    CPPUNIT_TEST(testGetClearPageUntilExhaustion);
    CPPUNIT_TEST(testPageReuseAfterRelease);
    CPPUNIT_TEST(testSetLinkedPagesNull);
    CPPUNIT_TEST(testInsertInvalidPage);
    CPPUNIT_TEST(testInsertPageForeign);
    CPPUNIT_TEST(testGetPageAfterClear);
    CPPUNIT_TEST(testPageLifecycleFull);
    CPPUNIT_TEST(testClearDuringActivePages);
    CPPUNIT_TEST(testSetCurrentPageWithActiveReferences);
    CPPUNIT_TEST(testGetNewestSubpageMultipleSubpages);
    CPPUNIT_TEST(testGetPageVsGetNewestSubpage);
    CPPUNIT_TEST(testMultipleReferencesToSamePage);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testPageReleasedDuringUpdate()
    {
        PageId pageId{100, 0};

        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        cache.setCurrentPage(pageId);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);

        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);

        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        auto mutablePage = cache.getMutablePage(pageId);

        CPPUNIT_ASSERT(clearPage == mutablePage);

        auto readyPage = cache.getPage(pageId);
        cache.releasePage(readyPage);

        cache.insertPage(mutablePage);

        auto updatedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(updatedPage != nullptr);
        CPPUNIT_ASSERT(updatedPage == mutablePage);
        cache.releasePage(updatedPage);
    }

    // Test getPage for a pageId that was never inserted (should return nullptr)
    void testGetPageNotFound()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{999, 0};
        auto page = cache.getPage(pageId);
        CPPUNIT_ASSERT(page == nullptr);
    }

    // Test setCurrentPage with an invalid PageId (boundary/invalid input)
    void testSetCurrentPageInvalid()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId invalidPageId{0, 0};

        cache.setCurrentPage(invalidPageId);

        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0, 5}));
        CPPUNIT_ASSERT(!cache.isPageNeeded(PageId{0x100, 0}));
    }

    // Test inserting duplicate page (should not crash, may replace or ignore)
    void testInsertDuplicatePage()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{101, 0};

        cache.setCurrentPage(pageId);

        auto page1 = cache.getClearPage();
        CPPUNIT_ASSERT(page1 != nullptr);

        auto header1 = page1->takeHeader();
        CPPUNIT_ASSERT(header1 != nullptr);
        header1->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        page1->setLastPacketValid(header1);
        cache.insertPage(page1);

        auto page2 = cache.getClearPage();
        CPPUNIT_ASSERT(page2 != nullptr);

        auto header2 = page2->takeHeader();
        CPPUNIT_ASSERT(header2 != nullptr);
        header2->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        page2->setLastPacketValid(header2);
        cache.insertPage(page2);

        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage != nullptr);
        CPPUNIT_ASSERT(retrievedPage == page2);
        CPPUNIT_ASSERT(retrievedPage != page1);
        cache.releasePage(retrievedPage);
    }

    // Test basic getClearPage functionality
    void testGetClearPageBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);
        // Page should be invalidated/clear
        CPPUNIT_ASSERT(!clearPage->isValid());
    }

    // Test basic getPage functionality
    void testGetPageBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{201, 0};

        // First insert a page
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Now retrieve it
        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage != nullptr);
        CPPUNIT_ASSERT(retrievedPage->getPageId() == pageId);

        cache.releasePage(retrievedPage);
    }

    // Test constructor with minimum valid buffer size
    void testConstructorMinimumBuffer()
    {
        auto constexpr MIN_PAGES = 1 + 1 + 4 + 3 + 2;
        auto constexpr MIN_BUFFER_SIZE = sizeof(ttxdecoder::CachePage) * MIN_PAGES;
        auto constexpr TOO_SMALL_BUFFER_SIZE = MIN_BUFFER_SIZE - 1;

        std::uint8_t validBuffer[MIN_BUFFER_SIZE];
        std::uint8_t invalidBuffer[MIN_BUFFER_SIZE];

        CPPUNIT_ASSERT_NO_THROW(CacheImpl cache(validBuffer, MIN_BUFFER_SIZE));
        CPPUNIT_ASSERT_THROW(CacheImpl cache(invalidBuffer, TOO_SMALL_BUFFER_SIZE),
                             std::invalid_argument);
    }

    // Test basic getMutablePage functionality
    void testGetMutablePageBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{203, 0};

        // Insert a page first
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Get mutable version
        auto mutablePage = cache.getMutablePage(pageId);
        CPPUNIT_ASSERT(mutablePage != nullptr);
        CPPUNIT_ASSERT(mutablePage->getPageId() == pageId);

        cache.releasePage(mutablePage);
    }

    // Test basic getNewestSubpage functionality
    void testGetNewestSubpageBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{204, 0};

        // Insert a page first
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Get newest subpage
        auto newestPage = cache.getNewestSubpage(pageId);
        CPPUNIT_ASSERT(newestPage != nullptr);
        CPPUNIT_ASSERT(newestPage->getPageId() == pageId);

        cache.releasePage(newestPage);
    }

    // Test basic releasePage functionality
    void testReleasePageBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{205, 0};

        // Insert and get a page
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage != nullptr);

        // Release should not throw
        CPPUNIT_ASSERT_NO_THROW(cache.releasePage(retrievedPage));
    }

    // Test setLinkedPages with valid input
    void testSetLinkedPagesBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        PageId linkedPages[] = {{0x300, 0}, {0x301, 0}, {0x302, 0}};

        cache.setCurrentPage(currentPage);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        cache.setLinkedPages(linkedPages, 3);

        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[1]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[2]));
    }

    // Test setLinkedPages with zero count
    void testSetLinkedPagesEmpty()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        PageId linkedPages[] = {{0x300, 0}};

        cache.setCurrentPage(currentPage);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        cache.setLinkedPages(linkedPages, 1);
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));

        cache.setLinkedPages(linkedPages, 0);
        CPPUNIT_ASSERT(!cache.isPageNeeded(linkedPages[0]));
    }

    // Test PageId boundaries
    void testPageIdBoundaries()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Test with maximum valid PageId values
        PageId maxPageId{0x8FF, 0x3F7F}; // Maximum valid values
        cache.setCurrentPage(maxPageId);
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x8FF, 0}));

        // Test with minimum valid PageId values
        PageId minPageId{0x100, 0x0000}; // Minimum valid values
        cache.setCurrentPage(minPageId);
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x100, 1}));
    }

    // Test getNewestSubpage for non-existing page
    void testGetNewestSubpageNotFound()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId nonExistentPageId{888, 0};
        auto page = cache.getNewestSubpage(nonExistentPageId);
        CPPUNIT_ASSERT(page == nullptr);
    }

    // Test multiple getClearPage calls
    void testMultipleClearPageCalls()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        auto page1 = cache.getClearPage();
        auto page2 = cache.getClearPage();
        auto page3 = cache.getClearPage();

        CPPUNIT_ASSERT(page1 != nullptr);
        CPPUNIT_ASSERT(page2 != nullptr);
        CPPUNIT_ASSERT(page3 != nullptr);

        // All pages should be different
        CPPUNIT_ASSERT(page1 != page2);
        CPPUNIT_ASSERT(page2 != page3);
        CPPUNIT_ASSERT(page1 != page3);

        // Clean up
        cache.releasePage(page1);
        cache.releasePage(page2);
        cache.releasePage(page3);
    }

    // Test setLinkedPages with count overflow
    void testSetLinkedPagesOverflow()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        // Create array with many pages (more than LINK_PAGES_COUNT)
        PageId linkedPages[50];
        for (int i = 0; i < 50; ++i) {
            linkedPages[i] = PageId{
                    static_cast<std::uint16_t>(0x300 | (((i / 10) & 0x0F) << 4) | (i % 10)),
                    0};
        }

        cache.setCurrentPage(currentPage);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        cache.setLinkedPages(linkedPages, 50);

        for (int i = 0; i < 4; ++i) {
            CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[i]));
        }
        CPPUNIT_ASSERT(!cache.isPageNeeded(linkedPages[4]));
    }

    // Test isPageNeeded with current page (indirect test via behavior)
    void testIsPageNeededCurrent()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{207, 0};

        // Set current page
        cache.setCurrentPage(pageId);

        // Insert the current page - should be accepted since it's needed
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Should be able to retrieve it since it's the current page
        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage != nullptr);

        cache.releasePage(retrievedPage);
    }

    void testIsPageNeededBasic()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId pageId{300, 0};
        cache.setCurrentPage(pageId);

        // Current page should always be needed
        CPPUNIT_ASSERT(cache.isPageNeeded(pageId) == true);

        // Same magazine page should be needed
        PageId sameMagazine{300, 5};
        CPPUNIT_ASSERT(cache.isPageNeeded(sameMagazine) == true);

        // Different magazine page should not be needed (unless in range)
        PageId differentMagazine{400, 0};
        CPPUNIT_ASSERT(!cache.isPageNeeded(differentMagazine));
    }

    // Test isPageNeeded with cache range logic
    void testIsPageNeededCacheRange()
    {
        auto constexpr BUFFER_SIZE = 1024 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x150, 0}; // Page 150 in decimal
        cache.setCurrentPage(currentPage);

        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x100, 0}));
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x150, 0}));
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x200, 0}));
        CPPUNIT_ASSERT(!cache.isPageNeeded(PageId{0x201, 0}));
    }

    void testIsPageNeededWrapRange()
    {
        auto constexpr BUFFER_SIZE = 1024 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        cache.setCurrentPage(PageId{0x149, 0});
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x899, 0}));
        CPPUNIT_ASSERT(!cache.isPageNeeded(PageId{0x898, 0}));

        cache.setCurrentPage(PageId{0x850, 0});
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x100, 0}));
        CPPUNIT_ASSERT(!cache.isPageNeeded(PageId{0x101, 0}));
    }

    // Test isPageNeeded with linked pages
    void testIsPageNeededLinkedPages()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        PageId linkedPages[] = {{0x300, 0}, {0x500, 0}, {0x600, 0}};

        cache.setCurrentPage(currentPage);
        cache.setLinkedPages(linkedPages, 3);

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        CPPUNIT_ASSERT(cache.isPageNeeded(currentPage));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[1]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[2]));
    }

    void testLinkedPagesGate()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        PageId nextCurrentPage{0x500, 0};
        PageId linkedPage{0x300, 0};

        cache.setCurrentPage(currentPage);
        cache.setLinkedPages(&linkedPage, 1);

        CPPUNIT_ASSERT(!cache.isPageNeeded(linkedPage));

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPage));

        cache.setCurrentPage(nextCurrentPage);
        CPPUNIT_ASSERT(!cache.isPageNeeded(linkedPage));
    }

    // Test isPageNeeded with invalid decimal pages
    void testIsPageNeededInvalidDecimal()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Set current page to invalid decimal
        PageId invalidCurrent{0xAAA, 0}; // Invalid decimal
        cache.setCurrentPage(invalidCurrent);

        // Same magazine page should still be needed
        CPPUNIT_ASSERT(cache.isPageNeeded(invalidCurrent) == true);

        // Other invalid decimal pages
        PageId otherInvalid{0xBBB, 0};
        CPPUNIT_ASSERT(!cache.isPageNeeded(otherInvalid));
    }

    // Test isPageNeeded with boundary range values
    void testIsPageNeededBoundaryRange()
    {
        auto constexpr BUFFER_SIZE = 1024 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Test with edge values
        PageId edgePage{0x100, 0}; // Minimum valid decimal (100)
        cache.setCurrentPage(edgePage);

        CPPUNIT_ASSERT(cache.isPageNeeded(edgePage) == true);
        CPPUNIT_ASSERT(cache.isPageNeeded(PageId{0x150, 0}));
        CPPUNIT_ASSERT(!cache.isPageNeeded(PageId{0x151, 0}));
    }

    // Test constructor with zero size
    void testConstructorZeroSize()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];

        bool exceptionThrown = false;
        try {
            CacheImpl cache{buffer, 0};
        } catch (const std::invalid_argument& e) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test with NULL PageId
    void testPageIdNull()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Create NULL page (using NULL constants)
        PageId nullPage{0x00FF, 0x3F7F}; // NULL_MAGAZINE_PAGE_MASK and NULL_SUBPAGE

        cache.setCurrentPage(nullPage);
        CPPUNIT_ASSERT(cache.isPageNeeded(nullPage));

        auto page = cache.getPage(nullPage);
        CPPUNIT_ASSERT(page == nullptr); // Should not find NULL page
    }

    // Test with INVALID PageId
    void testPageIdInvalid()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Create invalid page
        PageId invalidPage{0xFFFF, 0}; // INVALID_MAGAZINE_PAGE

        cache.setCurrentPage(invalidPage);
        CPPUNIT_ASSERT(cache.isPageNeeded(invalidPage));

        auto page = cache.getPage(invalidPage);
        CPPUNIT_ASSERT(page == nullptr); // Should not find invalid page
    }

    // Test with maximum valid PageId values
    void testPageIdMaxValues()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Maximum valid decimal values
        PageId maxValid{0x899, 0x3F7E}; // Max valid decimal page and subpage

        cache.setCurrentPage(maxValid);
        CPPUNIT_ASSERT(cache.isPageNeeded(maxValid));

        auto page = cache.getPage(maxValid);
        CPPUNIT_ASSERT(page == nullptr); // Should not find page that wasn't inserted
    }

    // Test getClearPage until complete exhaustion
    void testGetClearPageUntilExhaustion()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        std::vector<ttxdecoder::PageDisplayable*> pages;

        // Get pages until exhaustion
        for (int i = 0; i < 200; ++i) { // Try to get many pages
            auto page = cache.getClearPage();
            if (page == nullptr) {
                break; // Exhausted
            }
            pages.push_back(page);
        }

        // Should have gotten some pages
        CPPUNIT_ASSERT(pages.size() > 0);

        // Next attempt should return nullptr
        auto nextPage = cache.getClearPage();
        CPPUNIT_ASSERT(nextPage == nullptr);

        // Clean up
        for (auto page : pages) {
            cache.releasePage(page);
        }
    }

    // Test page reuse after release
    void testPageReuseAfterRelease()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        // Get a clear page
        auto page1 = cache.getClearPage();
        CPPUNIT_ASSERT(page1 != nullptr);

        // Release it
        cache.releasePage(page1);

        // Get another clear page - should be able to reuse
        auto page2 = cache.getClearPage();
        CPPUNIT_ASSERT(page2 != nullptr);

        // Clean up
        cache.releasePage(page2);
    }

    // Test setLinkedPages with null pointer
    void testSetLinkedPagesNull()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};

        PageId currentPage{0x200, 0};
        PageId linkedPages[] = {{0x300, 0}, {0x301, 0}};
        PageId unrelatedPage{0x400, 0};

        cache.setCurrentPage(currentPage);
        cache.setLinkedPages(nullptr, 2);
        CPPUNIT_ASSERT(!cache.isPageNeeded(unrelatedPage));

        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(currentPage, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        cache.setLinkedPages(linkedPages, 2);
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[1]));

        cache.setLinkedPages(nullptr, 0);
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[1]));

        cache.setLinkedPages(nullptr, 5);

        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[0]));
        CPPUNIT_ASSERT(cache.isPageNeeded(linkedPages[1]));
    }

    // Test inserting invalid page
    void testInsertInvalidPage()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 0};

        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);

        // Don't set up the page properly (so isValid() returns false)
        // Just insert it directly without making it valid
        cache.insertPage(clearPage);

        // Should not be findable since it's invalid
        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage == nullptr);
    }

    void testInsertPageForeign()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer1[BUFFER_SIZE];
        std::uint8_t buffer2[BUFFER_SIZE];
        CacheImpl cache1{buffer1, BUFFER_SIZE};
        CacheImpl cache2{buffer2, BUFFER_SIZE};

        auto foreignPage = cache1.getClearPage();
        CPPUNIT_ASSERT(foreignPage != nullptr);

        CPPUNIT_ASSERT_THROW(cache2.insertPage(foreignPage), std::invalid_argument);

        cache1.releasePage(foreignPage);
    }

    // Test getPage after clear
    void testGetPageAfterClear()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 0};

        // Insert a page
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Verify page exists
        auto page1 = cache.getPage(pageId);
        CPPUNIT_ASSERT(page1 != nullptr);
        cache.releasePage(page1);

        // Clear cache
        cache.clear();

        // Page should no longer exist
        auto page2 = cache.getPage(pageId);
        CPPUNIT_ASSERT(page2 == nullptr);
    }

    // Test complete page lifecycle
    void testPageLifecycleFull()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 0};

        // 1. Set current page
        cache.setCurrentPage(pageId);

        // 2. Get clear page
        auto clearPage = cache.getClearPage();
        CPPUNIT_ASSERT(clearPage != nullptr);
        CPPUNIT_ASSERT(!clearPage->isValid());

        // 3. Set up page
        auto headerPacket = clearPage->takeHeader();
        CPPUNIT_ASSERT(headerPacket != nullptr);
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        CPPUNIT_ASSERT(clearPage->isValid());

        // 4. Insert page
        cache.insertPage(clearPage);

        // 5. Retrieve page
        auto retrievedPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(retrievedPage != nullptr);
        CPPUNIT_ASSERT(retrievedPage->getPageId() == pageId);
        CPPUNIT_ASSERT(retrievedPage->isValid());

        // 6. Release page
        cache.releasePage(retrievedPage);
    }

    // Test clear during active page references
    void testClearDuringActivePages()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 0};

        // Insert and get a page
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        auto activePage = cache.getPage(pageId);
        CPPUNIT_ASSERT(activePage != nullptr);

        // Clear while page is still referenced
        cache.clear();

        // Should still be able to use the active page
        CPPUNIT_ASSERT(activePage->isValid());
        CPPUNIT_ASSERT(activePage->getPageId() == pageId);

        // Release the page
        cache.releasePage(activePage);

        // New attempts to get the page should fail
        auto newPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(newPage == nullptr);
    }

    void testSetCurrentPageWithActiveReferences()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId1{300, 0};
        PageId pageId2{400, 0};

        // Insert and get first page
        cache.setCurrentPage(pageId1);
        auto clearPage1 = cache.getClearPage();
        auto headerPacket1 = clearPage1->takeHeader();
        headerPacket1->setPageInfo(pageId1, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage1->setLastPacketValid(headerPacket1);
        cache.insertPage(clearPage1);

        auto activePage1 = cache.getPage(pageId1);
        CPPUNIT_ASSERT(activePage1 != nullptr);

        // Change current page while first page is still referenced
        cache.setCurrentPage(pageId2);

        // First page should still be usable
        CPPUNIT_ASSERT(activePage1->isValid());
        CPPUNIT_ASSERT(activePage1->getPageId() == pageId1);

        // Release first page
        cache.releasePage(activePage1);

        auto oldPage = cache.getPage(pageId1);
        CPPUNIT_ASSERT(oldPage == nullptr);
    }

    void testGetNewestSubpageMultipleSubpages()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId1{300, 1};
        PageId pageId2{300, 2}; // Same magazine page, different subpage

        // Insert first subpage
        cache.setCurrentPage(pageId1);
        auto clearPage1 = cache.getClearPage();
        auto headerPacket1 = clearPage1->takeHeader();
        headerPacket1->setPageInfo(pageId1, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage1->setLastPacketValid(headerPacket1);
        cache.insertPage(clearPage1);

        // Insert second subpage (should replace first for same magazine page)
        cache.setCurrentPage(pageId2);
        auto clearPage2 = cache.getClearPage();
        auto headerPacket2 = clearPage2->takeHeader();
        headerPacket2->setPageInfo(pageId2, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage2->setLastPacketValid(headerPacket2);
        cache.insertPage(clearPage2);

        auto oldPage = cache.getPage(pageId1);
        CPPUNIT_ASSERT(oldPage == nullptr);

        // getNewestSubpage should return the most recent one
        auto newestPage = cache.getNewestSubpage(PageId{300, 0xFFFF}); // ANY_SUBPAGE

        CPPUNIT_ASSERT(newestPage != nullptr);
        CPPUNIT_ASSERT(newestPage->getPageId() == pageId2);
        cache.releasePage(newestPage);
    }

    // Test difference between getPage and getNewestSubpage
    void testGetPageVsGetNewestSubpage()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 5};

        // Insert a specific subpage
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // getPage with exact PageId should find it
        auto exactPage = cache.getPage(pageId);
        CPPUNIT_ASSERT(exactPage != nullptr);
        cache.releasePage(exactPage);

        // getNewestSubpage with magazine page should find it
        auto newestPage = cache.getNewestSubpage(PageId{300, 0});
        CPPUNIT_ASSERT(newestPage != nullptr);
        CPPUNIT_ASSERT(newestPage->getPageId() == pageId);
        cache.releasePage(newestPage);

        // getPage with different subpage should not find it
        auto wrongSubpage = cache.getPage(PageId{300, 7});
        CPPUNIT_ASSERT(wrongSubpage == nullptr);
    }

    // Test multiple references to same page
    void testMultipleReferencesToSamePage()
    {
        auto constexpr BUFFER_SIZE = 64 * 1024;
        std::uint8_t buffer[BUFFER_SIZE];
        CacheImpl cache{buffer, BUFFER_SIZE};
        PageId pageId{300, 0};

        // Insert a page
        cache.setCurrentPage(pageId);
        auto clearPage = cache.getClearPage();
        auto headerPacket = clearPage->takeHeader();
        headerPacket->setPageInfo(pageId, ttxdecoder::ControlInfo::MAGAZINE_SERIAL, 0);
        clearPage->setLastPacketValid(headerPacket);
        cache.insertPage(clearPage);

        // Get multiple references to same page
        auto page1 = cache.getPage(pageId);
        auto page2 = cache.getPage(pageId);
        auto page3 = cache.getPage(pageId);

        CPPUNIT_ASSERT(page1 != nullptr);
        CPPUNIT_ASSERT(page2 != nullptr);
        CPPUNIT_ASSERT(page3 != nullptr);

        // All should refer to the same page
        CPPUNIT_ASSERT(page1->getPageId() == pageId);
        CPPUNIT_ASSERT(page2->getPageId() == pageId);
        CPPUNIT_ASSERT(page3->getPageId() == pageId);

        // Release all references
        cache.releasePage(page1);
        cache.releasePage(page2);
        cache.releasePage(page3);
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CacheImplTest );
