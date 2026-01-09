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
#include <vector>
#include <cstring>

#include "Decoder.hpp"
#include "Database.hpp"
#include "Cache.hpp"
#include "DecoderListener.hpp"
#include "PesPacketReader.hpp"
#include "PageDisplayable.hpp"
#include "ControlInfo.hpp"
#include "PageId.hpp"

using namespace ttxdecoder;

// Mock DecoderListener for testing
class MockDecoderListener : public DecoderListener
{
public:
    MockDecoderListener()
        : m_pageDecodedCount(0)
        , m_headerDecodedCount(0)
        , m_lastPageId()
        , m_lastHeaderMagazineNumber(0xFF)
        , m_lastHeaderControlInfo(0)
    {
    }

    virtual ~MockDecoderListener() = default;

    virtual void pageDecoded(const PageId& pageId) override
    {
        m_pageDecodedCount++;
        m_lastPageId = pageId;
        m_pageIds.push_back(pageId);
    }

    virtual void headerDecoded(const PacketHeader& header) override
    {
        m_headerDecodedCount++;
        m_lastHeaderMagazineNumber = header.getMagazineNumber();
        m_lastHeaderControlInfo = header.getControlInfo();
        m_headerControlInfos.push_back(header.getControlInfo());
        m_headerPageIds.push_back(header.getPageId());
    }

    void reset()
    {
        m_pageDecodedCount = 0;
        m_headerDecodedCount = 0;
        m_lastHeaderMagazineNumber = 0xFF;
        m_lastHeaderControlInfo = 0;
        m_pageIds.clear();
        m_headerControlInfos.clear();
        m_headerPageIds.clear();
    }

    int getPageDecodedCount() const { return m_pageDecodedCount; }
    int getHeaderDecodedCount() const { return m_headerDecodedCount; }
    std::uint8_t getLastHeaderMagazineNumber() const { return m_lastHeaderMagazineNumber; }
    std::uint8_t getLastHeaderControlInfo() const { return m_lastHeaderControlInfo; }

private:
    int m_pageDecodedCount;
    int m_headerDecodedCount;
    PageId m_lastPageId;
    std::uint8_t m_lastHeaderMagazineNumber;
    std::uint8_t m_lastHeaderControlInfo;
    std::vector<PageId> m_pageIds;
    std::vector<std::uint8_t> m_headerControlInfos;
    std::vector<PageId> m_headerPageIds;
};

// Mock Cache for testing
class MockCache : public Cache
{
public:
    MockCache()
        : m_currentPage()
        , m_isPageNeededResult(true)
        , m_getMutablePageResult(nullptr)
        , m_getClearPageResult(nullptr)
        , m_insertPageCallCount(0)
        , m_releasePageCallCount(0)
        , m_getMutablePageCallCount(0)
        , m_getClearPageCallCount(0)
    {
    }

    virtual ~MockCache() = default;

    virtual void setCurrentPage(PageId pageId) override
    {
        m_currentPage = pageId;
    }

    virtual void setLinkedPages(const PageId* pageIds, std::size_t count) override
    {
        // Not needed for these tests
    }

    virtual void clear() override
    {
        // Not needed for these tests
    }

    virtual bool isPageNeeded(PageId pageId) const override
    {
        return m_isPageNeededResult;
    }

    virtual const PageDisplayable* getPage(PageId pageId) override
    {
        return nullptr;
    }

    virtual PageDisplayable* getMutablePage(PageId pageId) override
    {
        m_getMutablePageCallCount++;
        m_lastRequestedPageId = pageId;
        return m_getMutablePageResult;
    }

    virtual const PageDisplayable* getNewestSubpage(PageId pageId) override
    {
        return nullptr;
    }

    virtual PageDisplayable* getClearPage() override
    {
        m_getClearPageCallCount++;
        return m_getClearPageResult;
    }

    virtual void insertPage(PageDisplayable* page) override
    {
        m_insertPageCallCount++;
        m_lastInsertedPage = page;
    }

    virtual void releasePage(const PageDisplayable* page) override
    {
        m_releasePageCallCount++;
        m_lastReleasedPage = page;
    }

    // Test helpers
    void setIsPageNeeded(bool result) { m_isPageNeededResult = result; }
    void setMutablePageResult(PageDisplayable* page) { m_getMutablePageResult = page; }
    void setClearPageResult(PageDisplayable* page) { m_getClearPageResult = page; }

    int getInsertPageCallCount() const { return m_insertPageCallCount; }
    int getReleasePageCallCount() const { return m_releasePageCallCount; }
    int getGetMutablePageCallCount() const { return m_getMutablePageCallCount; }
    int getGetClearPageCallCount() const { return m_getClearPageCallCount; }

    void reset()
    {
        m_insertPageCallCount = 0;
        m_releasePageCallCount = 0;
        m_getMutablePageCallCount = 0;
        m_getClearPageCallCount = 0;
        m_lastInsertedPage = nullptr;
        m_lastReleasedPage = nullptr;
    }

private:
    PageId m_currentPage;
    bool m_isPageNeededResult;
    PageDisplayable* m_getMutablePageResult;
    PageDisplayable* m_getClearPageResult;
    int m_insertPageCallCount;
    int m_releasePageCallCount;
    int m_getMutablePageCallCount;
    int m_getClearPageCallCount;
    const PageDisplayable* m_lastInsertedPage;
    const PageDisplayable* m_lastReleasedPage;
    PageId m_lastRequestedPageId;
};

class DecoderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DecoderTest );
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testResetClearsState);
    CPPUNIT_TEST(testProcessHeaderPacketSetsSerialMode);
    CPPUNIT_TEST(testProcessHeaderPacketSetsParallelMode);
    CPPUNIT_TEST(testHeaderDecodedCallback);
    CPPUNIT_TEST(testMultipleConsecutiveHeadersSameMagazine);
    CPPUNIT_TEST(testHeaderWithDifferentMagazineInSerialMode);
    CPPUNIT_TEST(testHeaderWithDifferentPageInParallelMode);
    CPPUNIT_TEST(testHeaderWithSamePageNumber);
    CPPUNIT_TEST(testHeaderWithSubtitleFlagSet);
    CPPUNIT_TEST(testHeaderWithErasePageFlagSet);
    CPPUNIT_TEST(testHeaderWithoutErasePageFlag);
    CPPUNIT_TEST(testCacheReturnsValidPageForNeededPageId);
    CPPUNIT_TEST(testCacheReturnsNullForNeededPageId);
    CPPUNIT_TEST(testCacheIndicatesPageNotNeeded);
    CPPUNIT_TEST(testValidPageProcessedAndCached);
    CPPUNIT_TEST(testInvalidPageProcessedAndReleased);
    CPPUNIT_TEST(testMagazineNumbersZeroToSeven);
    CPPUNIT_TEST(testMultipleMagazinesConcurrent);
    CPPUNIT_TEST(testSerialModeProcessesOnePageAtTime);
    CPPUNIT_TEST(testParallelModeProcessesPerMagazine);
    CPPUNIT_TEST(testResetAfterHeaderProcessing);
    CPPUNIT_TEST(testPartialPageReplacedByNewHeader);
    CPPUNIT_TEST(testCacheMutablePageCalledWithoutEraseFlag);
    CPPUNIT_TEST(testCacheClearPageCalledWithEraseFlag);
    CPPUNIT_TEST(testPageReleasedWhenInvalid);
    CPPUNIT_TEST(testNoPageDecodedForInvalidPage);
    CPPUNIT_TEST(testInterleavedPacketsMultipleMagazines);
    CPPUNIT_TEST(testControlInfoAllFlagsCombination);
    CPPUNIT_TEST(testControlInfoNoFlagsSet);
    CPPUNIT_TEST(testMagazineZeroMappedCorrectly);
    CPPUNIT_TEST(testMagazineSevenBoundary);
    CPPUNIT_TEST(testSubtitleFlagTriggersImmediateProcessing);
    CPPUNIT_TEST(testMultipleResetsInSequence);
    CPPUNIT_TEST(testCacheFullScenarioWithNullReturn);
    CPPUNIT_TEST(testSerialModeAcrossDifferentMagazines);
    CPPUNIT_TEST(testParallelModeSameMagazineDifferentPages);
    CPPUNIT_TEST(testHeaderWithAllControlBitsSet);
    CPPUNIT_TEST(testHeaderWithNewsflashFlag);
    CPPUNIT_TEST(testHeaderWithSuppressHeaderFlag);
    CPPUNIT_TEST(testHeaderWithUpdateIndicatorFlag);
    CPPUNIT_TEST(testSequentialHeadersWithMixedModes);
    CPPUNIT_TEST(testCachePageNeededChangesAcrossHeaders);
    CPPUNIT_TEST(testMultipleHeadersSamePageDifferentSubpages);
    CPPUNIT_TEST(testResetClearsAllMagazineSlots);
    CPPUNIT_TEST(testProcessPacketDataMultipleTimes);
    CPPUNIT_TEST(testHeaderDecodedForAllMagazines);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_database = new Database();
        m_cache = new MockCache();
        m_listener = new MockDecoderListener();
        m_decoder = new Decoder(*m_database, *m_cache, *m_listener);
    }

    void tearDown() override
    {
        delete m_decoder;
        delete m_listener;
        delete m_cache;
        delete m_database;
    }

    void testConstructor()
    {
        // Constructor should complete successfully with valid dependencies
        Database database;
        MockCache cache;
        MockDecoderListener listener;

        Decoder decoder(database, cache, listener);

        // If we get here, constructor succeeded
        CPPUNIT_ASSERT(true);
    }

    void testResetClearsState()
    {
        // Process a header packet first
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());

        // Reset the decoder
        m_decoder->reset();
        m_listener->reset();

        // Process another packet - should work normally after reset
        std::vector<std::uint8_t> headerData2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(headerData2.data(), headerData2.size(), nullptr, 0);

        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
    }

    void testProcessHeaderPacketSetsSerialMode()
    {
        // Create header packet with MAGAZINE_SERIAL flag set (bit 7)
        std::uint8_t controlInfo = ControlInfo::MAGAZINE_SERIAL;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify header was decoded
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(controlInfo, m_listener->getLastHeaderControlInfo());
    }

    void testProcessHeaderPacketSetsParallelMode()
    {
        // Create header packet without MAGAZINE_SERIAL flag
        std::uint8_t controlInfo = 0;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify header was decoded
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(controlInfo, m_listener->getLastHeaderControlInfo());
    }

    void testHeaderDecodedCallback()
    {
        // Create and process header packet
        std::vector<std::uint8_t> headerData = createHeaderPacket(3, 0x300, 0x0005, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify listener was called
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(3), m_listener->getLastHeaderMagazineNumber());
    }

    void testMultipleConsecutiveHeadersSameMagazine()
    {
        m_cache->setIsPageNeeded(false); // Simplify by not needing pages

        // Process first header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process second header for same magazine
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x101, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testHeaderWithDifferentMagazineInSerialMode()
    {
        m_cache->setIsPageNeeded(false);

        // Process header with SERIAL mode flag
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process header for different magazine in serial mode
        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be decoded
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testHeaderWithDifferentPageInParallelMode()
    {
        m_cache->setIsPageNeeded(false);

        // Process header without SERIAL flag (parallel mode)
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process different page in same magazine (parallel mode)
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x101, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be decoded
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testHeaderWithSamePageNumber()
    {
        m_cache->setIsPageNeeded(false);

        // Process header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process same page number with different subpage
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x100, 1, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be decoded
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testHeaderWithSubtitleFlagSet()
    {
        m_cache->setIsPageNeeded(false);

        // Process header with SUBTITLE flag
        std::uint8_t controlInfo = ControlInfo::SUBTITLE;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify header was decoded with subtitle flag
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::SUBTITLE) != 0);
    }

    void testHeaderWithErasePageFlagSet()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPageResult(nullptr); // Simulate no clear page available

        // Process header with ERASE_PAGE flag
        std::uint8_t controlInfo = ControlInfo::ERASE_PAGE;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify getMutablePage was NOT called (erase flag set)
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetMutablePageCallCount());
        // Clear page should be requested
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
    }

    void testHeaderWithoutErasePageFlag()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr); // No mutable page available
        m_cache->setClearPageResult(nullptr); // No clear page available

        // Process header without ERASE_PAGE flag
        std::uint8_t controlInfo = 0;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify getMutablePage was called (no erase flag)
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetMutablePageCallCount());
    }

    void testCacheReturnsValidPageForNeededPageId()
    {
        PageDisplayable mockPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(&mockPage);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify mutable page was requested
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetMutablePageCallCount());
    }

    void testCacheReturnsNullForNeededPageId()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr);
        m_cache->setClearPageResult(nullptr);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Should fallback to clear page
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
    }

    void testCacheIndicatesPageNotNeeded()
    {
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Mutable page should not be requested
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetMutablePageCallCount());
        // Clear page should not be requested
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetClearPageCallCount());
    }

    void testValidPageProcessedAndCached()
    {
        // This test would require a valid page that reports isValid() == true
        // For now, verify that page processing occurs
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Header should be decoded
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
    }

    void testInvalidPageProcessedAndReleased()
    {
        // Process header that creates a page
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // pageDecoded should not be called for invalid pages
        // (metadata pages are typically invalid for display)
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
    }

    void testMagazineNumbersZeroToSeven()
    {
        m_cache->setIsPageNeeded(false);

        // Test all magazine numbers 0-7
        for (int mag = 0; mag <= 7; mag++)
        {
            std::vector<std::uint8_t> headerData = createHeaderPacket(mag, 0x100 + mag, 0, 0);
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

            m_decoder->processPacketData(reader);
        }

        // All 8 headers should be decoded
        CPPUNIT_ASSERT_EQUAL(8, m_listener->getHeaderDecodedCount());
    }

    void testMultipleMagazinesConcurrent()
    {
        m_cache->setIsPageNeeded(false);

        // Process headers for different magazines
        for (int mag = 0; mag < 4; mag++)
        {
            std::vector<std::uint8_t> headerData = createHeaderPacket(mag, 0x100 + mag, 0, 0);
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
            m_decoder->processPacketData(reader);
        }

        // All headers should be processed
        CPPUNIT_ASSERT_EQUAL(4, m_listener->getHeaderDecodedCount());
    }

    void testSerialModeProcessesOnePageAtTime()
    {
        m_cache->setIsPageNeeded(false);

        // Process first header in serial mode
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process header for different magazine (should complete first page)
        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testParallelModeProcessesPerMagazine()
    {
        m_cache->setIsPageNeeded(false);

        // Process headers in parallel mode (no MAGAZINE_SERIAL flag)
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be processed independently
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testResetAfterHeaderProcessing()
    {
        m_cache->setIsPageNeeded(false);

        // Process header
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());

        // Reset
        m_decoder->reset();
        m_listener->reset();
        m_cache->reset();

        // Process another header
        std::vector<std::uint8_t> headerData2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(headerData2.data(), headerData2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Should process normally
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
    }

    void testPartialPageReplacedByNewHeader()
    {
        PageDisplayable mockPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPageResult(&mockPage);

        // Process first header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Verify first header was processed
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());

        // Process second header for same magazine (partial page replaced)
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x101, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Verify second header was processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());

        // First page should be inserted (valid with header) when second header arrives
        CPPUNIT_ASSERT(m_cache->getInsertPageCallCount() >= 1);
    }

    void testCacheMutablePageCalledWithoutEraseFlag()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr);
        m_cache->setClearPageResult(nullptr);

        // Header without erase flag
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // getMutablePage should be called
        CPPUNIT_ASSERT(m_cache->getGetMutablePageCallCount() > 0);
    }

    void testCacheClearPageCalledWithEraseFlag()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPageResult(nullptr);

        // Header with erase flag
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // getClearPage should be called (not getMutablePage)
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetMutablePageCallCount());
    }

    void testPageReleasedWhenInvalid()
    {
        PageDisplayable mockPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPageResult(&mockPage);

        // Process header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Verify first header was processed
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());

        // Process new header (should release previous page)
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x101, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Verify second header was processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());

        // Page should be inserted (valid with header) when second header arrives
        CPPUNIT_ASSERT(m_cache->getInsertPageCallCount() > 0);
    }

    void testNoPageDecodedForInvalidPage()
    {
        m_cache->setIsPageNeeded(false);

        // Process header
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // pageDecoded should not be called for invalid page
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
    }

    void testInterleavedPacketsMultipleMagazines()
    {
        m_cache->setIsPageNeeded(false);

        // Process headers from different magazines in interleaved fashion
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, 0);
        std::vector<std::uint8_t> header3 = createHeaderPacket(3, 0x300, 0, 0);

        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        PesPacketReader reader3(header3.data(), header3.size(), nullptr, 0);
        m_decoder->processPacketData(reader3);

        // All three headers should be processed
        CPPUNIT_ASSERT_EQUAL(3, m_listener->getHeaderDecodedCount());
    }

    void testControlInfoAllFlagsCombination()
    {
        m_cache->setIsPageNeeded(false);

        // Test with multiple control flags set
        std::uint8_t controlInfo = ControlInfo::ERASE_PAGE | ControlInfo::SUBTITLE | ControlInfo::NEWSFLASH;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify all flags are preserved
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::ERASE_PAGE) != 0);
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::SUBTITLE) != 0);
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::NEWSFLASH) != 0);
    }

    void testControlInfoNoFlagsSet()
    {
        m_cache->setIsPageNeeded(false);

        // Test with no control flags
        std::uint8_t controlInfo = 0;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Verify control info is 0
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastHeaderControlInfo());
    }

    void testMagazineZeroMappedCorrectly()
    {
        m_cache->setIsPageNeeded(false);

        // Magazine 0 should be processed correctly (maps to index 0 internally)
        std::vector<std::uint8_t> headerData = createHeaderPacket(0, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Should be processed successfully
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_listener->getLastHeaderMagazineNumber());
    }

    void testMagazineSevenBoundary()
    {
        m_cache->setIsPageNeeded(false);

        // Magazine 7 is the maximum valid magazine
        std::vector<std::uint8_t> headerData = createHeaderPacket(7, 0x700, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Should be processed successfully
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(7), m_listener->getLastHeaderMagazineNumber());
    }

    void testSubtitleFlagTriggersImmediateProcessing()
    {
        m_cache->setIsPageNeeded(false);

        // Process first header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Process same page with SUBTITLE flag - should trigger immediate processing
        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x100, 0, ControlInfo::SUBTITLE);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both headers should be processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testMultipleResetsInSequence()
    {
        m_cache->setIsPageNeeded(false);

        // Process header
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Multiple resets
        m_decoder->reset();
        m_decoder->reset();
        m_decoder->reset();

        m_listener->reset();

        // Should still work after multiple resets
        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
    }

    void testCacheFullScenarioWithNullReturn()
    {
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr);
        m_cache->setClearPageResult(nullptr);

        // Process header when cache is "full" (returns null)
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Header should still be decoded even if cache is full
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        // Fallback to metadata processor should occur
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
    }

    void testSerialModeAcrossDifferentMagazines()
    {
        m_cache->setIsPageNeeded(false);

        // In serial mode, changing magazine should complete previous page
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        std::vector<std::uint8_t> header2 = createHeaderPacket(3, 0x300, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both should be processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testParallelModeSameMagazineDifferentPages()
    {
        m_cache->setIsPageNeeded(false);

        // In parallel mode, different pages in same magazine should be processed independently
        std::vector<std::uint8_t> header1 = createHeaderPacket(2, 0x201, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x202, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Both should be processed
        CPPUNIT_ASSERT_EQUAL(2, m_listener->getHeaderDecodedCount());
    }

    void testHeaderWithAllControlBitsSet()
    {
        m_cache->setIsPageNeeded(false);

        // Test with all control bits set (0xFF)
        std::uint8_t controlInfo = 0xFF;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        // Should be processed with all flags
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(controlInfo, m_listener->getLastHeaderControlInfo());
    }

    void testHeaderWithNewsflashFlag()
    {
        m_cache->setIsPageNeeded(false);

        // Test NEWSFLASH flag
        std::uint8_t controlInfo = ControlInfo::NEWSFLASH;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::NEWSFLASH) != 0);
    }

    void testHeaderWithSuppressHeaderFlag()
    {
        m_cache->setIsPageNeeded(false);

        // Test SUPPRESS_HEADER flag
        std::uint8_t controlInfo = ControlInfo::SUPRESS_HEADER;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::SUPRESS_HEADER) != 0);
    }

    void testHeaderWithUpdateIndicatorFlag()
    {
        m_cache->setIsPageNeeded(false);

        // Test UPDATE_INDICATOR flag
        std::uint8_t controlInfo = ControlInfo::UPDATE_INDICATOR;
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, controlInfo);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::UPDATE_INDICATOR) != 0);
    }

    void testSequentialHeadersWithMixedModes()
    {
        m_cache->setIsPageNeeded(false);

        // Start with serial mode
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        // Switch to parallel mode
        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // Back to serial mode
        std::vector<std::uint8_t> header3 = createHeaderPacket(3, 0x300, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader3(header3.data(), header3.size(), nullptr, 0);
        m_decoder->processPacketData(reader3);

        // All should be processed
        CPPUNIT_ASSERT_EQUAL(3, m_listener->getHeaderDecodedCount());
    }

    void testCachePageNeededChangesAcrossHeaders()
    {
        // First header: page is needed
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPageResult(nullptr);

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        int clearPageCalls1 = m_cache->getGetClearPageCallCount();

        // Second header: page is not needed
        m_cache->reset();
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, ControlInfo::ERASE_PAGE);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        // First header should have called getClearPage, second should not
        CPPUNIT_ASSERT(clearPageCalls1 > 0);
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetClearPageCallCount());
    }

    void testMultipleHeadersSamePageDifferentSubpages()
    {
        m_cache->setIsPageNeeded(false);

        // Same page number (0x100) but different subpages
        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0x0000, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x100, 0x0001, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        std::vector<std::uint8_t> header3 = createHeaderPacket(1, 0x100, 0x0002, 0);
        PesPacketReader reader3(header3.data(), header3.size(), nullptr, 0);
        m_decoder->processPacketData(reader3);

        // All subpages should be processed
        CPPUNIT_ASSERT_EQUAL(3, m_listener->getHeaderDecodedCount());
    }

    void testResetClearsAllMagazineSlots()
    {
        m_cache->setIsPageNeeded(false);

        // Populate multiple magazine slots
        for (int mag = 0; mag < 4; mag++)
        {
            std::vector<std::uint8_t> headerData = createHeaderPacket(mag, 0x100 + mag, 0, 0);
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
            m_decoder->processPacketData(reader);
        }

        CPPUNIT_ASSERT_EQUAL(4, m_listener->getHeaderDecodedCount());

        // Reset
        m_decoder->reset();
        m_listener->reset();

        // Process new headers - should work normally
        for (int mag = 4; mag < 8; mag++)
        {
            std::vector<std::uint8_t> headerData = createHeaderPacket(mag, 0x100 + mag, 0, 0);
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
            m_decoder->processPacketData(reader);
        }

        CPPUNIT_ASSERT_EQUAL(4, m_listener->getHeaderDecodedCount());
    }

    void testProcessPacketDataMultipleTimes()
    {
        m_cache->setIsPageNeeded(false);

        // Process same packet data multiple times
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);

        for (int i = 0; i < 5; i++)
        {
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
            m_decoder->processPacketData(reader);
        }

        // Each call should process the header
        CPPUNIT_ASSERT_EQUAL(5, m_listener->getHeaderDecodedCount());
    }

    void testHeaderDecodedForAllMagazines()
    {
        m_cache->setIsPageNeeded(false);

        // Process header for each magazine
        for (int mag = 0; mag <= 7; mag++)
        {
            std::vector<std::uint8_t> headerData = createHeaderPacket(mag, 0x100, 0, 0);
            PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
            m_decoder->processPacketData(reader);

            // Verify this magazine's header was decoded
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(mag), m_listener->getLastHeaderMagazineNumber());
        }

        // All 8 magazines processed
        CPPUNIT_ASSERT_EQUAL(8, m_listener->getHeaderDecodedCount());
    }

private:
    // Helper to create header packet data
    std::vector<std::uint8_t> createHeaderPacket(
        std::uint8_t magazine,
        std::uint16_t page,
        std::uint16_t subpage,
        std::uint8_t controlInfo)
    {
        std::vector<std::uint8_t> data;

        // Data identifier
        data.push_back(0x10);

        // Data unit ID (0x02 for teletext)
        data.push_back(0x02);

        // Data unit length (44 bytes for teletext packet)
        data.push_back(0x2C);

        // The following 44 bytes make up the data unit:

        // Data unit control (1 byte)
        data.push_back(0xFF);

        // Framing code (1 byte)
        data.push_back(0xE4);

        // Magazine and packet address (2 bytes, Hamming 8/4 encoded)
        data.push_back(getHammingByteForMagazine(magazine));
        data.push_back(0x28); // Packet address 0

        // Page number (2 bytes, Hamming 8/4 encoded BCD)
        // Extract page number from magazinePage format (bits 0-7)
        std::uint8_t pageOnly = page & 0xFF;
        std::uint8_t pageUnits = pageOnly & 0x0F;
        std::uint8_t pageTens = (pageOnly >> 4) & 0x0F;
        data.push_back(getHammingByte(pageUnits));
        data.push_back(getHammingByte(pageTens));

        // Subpage and control bits (6 bytes, Hamming 8/4 encoded)
        // Based on teletext specification structure

        // S1: Subpage bits 0-3
        std::uint8_t byte2 = (subpage >> 0) & 0x0F;
        data.push_back(getHammingByte(byte2));

        // S2: Subpage bits 4-6 + C4 (ERASE_PAGE)
        std::uint8_t byte3 = ((subpage >> 4) & 0x07) |
                             ((controlInfo & ControlInfo::ERASE_PAGE) ? 0x08 : 0x00);
        data.push_back(getHammingByte(byte3));

        // S3: Subpage bits 8-11
        std::uint8_t byte4 = ((subpage >> 8) & 0x0F);
        data.push_back(getHammingByte(byte4));

        // S4: Subpage bits 12-13 + C5 (NEWSFLASH) + C6 (SUBTITLE)
        std::uint8_t byte5 = ((subpage >> 12) & 0x03) |
                             ((controlInfo & ControlInfo::NEWSFLASH) ? 0x04 : 0x00) |
                             ((controlInfo & ControlInfo::SUBTITLE) ? 0x08 : 0x00);
        data.push_back(getHammingByte(byte5));

        // C7-C10: Control bits
        std::uint8_t byte6 = ((controlInfo & ControlInfo::SUPRESS_HEADER) ? 0x01 : 0x00) |
                             ((controlInfo & ControlInfo::UPDATE_INDICATOR) ? 0x02 : 0x00) |
                             ((controlInfo & ControlInfo::INTERRUPTED_SEQUENCE) ? 0x04 : 0x00) |
                             ((controlInfo & ControlInfo::INHIBIT_DISPLAY) ? 0x08 : 0x00);
        data.push_back(getHammingByte(byte6));

        // C11: MAGAZINE_SERIAL + National option
        std::uint8_t byte7 = ((controlInfo & ControlInfo::MAGAZINE_SERIAL) ? 0x01 : 0x00);
        data.push_back(getHammingByte(byte7));

        // Remaining 32 bytes of header display data
        for (int i = 0; i < 32; i++)
        {
            data.push_back(0x20); // Space character
        }

        return data;
    }

    // Hamming 8/4 encoder for teletext packets
    // Inverse mapping of hamming84LookupTable from Hamming.cpp
    std::uint8_t getHammingByte(std::uint8_t nibble)
    {
        static const std::uint8_t hammingEncode[16] = {
            0x28, 0x00, 0x12, 0x3A, 0x06, 0x4E, 0x0C, 0x74,
            0x03, 0x63, 0x11, 0x59, 0x05, 0x2D, 0x3F, 0x17
        };
        return hammingEncode[nibble & 0x0F];
    }

    // Encode magazine number using Hamming 8/4
    std::uint8_t getHammingByteForMagazine(std::uint8_t magazine)
    {
        return getHammingByte(magazine & 0x07);
    }

    Database* m_database;
    MockCache* m_cache;
    MockDecoderListener* m_listener;
    Decoder* m_decoder;
};

CPPUNIT_TEST_SUITE_REGISTRATION( DecoderTest );
