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
        , m_lastHeaderMagazineNumber(0xFF)
        , m_lastHeaderControlInfo(0)
    {
    }

    virtual ~MockDecoderListener() = default;

    virtual void pageDecoded(const PageId& pageId) override
    {
        m_pageDecodedCount++;
    }

    virtual void headerDecoded(const PacketHeader& header) override
    {
        m_headerDecodedCount++;
        m_lastHeaderMagazineNumber = header.getMagazineNumber();
        m_lastHeaderControlInfo = header.getControlInfo();
    }

    void reset()
    {
        m_pageDecodedCount = 0;
        m_headerDecodedCount = 0;
        m_lastHeaderMagazineNumber = 0xFF;
        m_lastHeaderControlInfo = 0;
    }

    int getPageDecodedCount() const { return m_pageDecodedCount; }
    int getHeaderDecodedCount() const { return m_headerDecodedCount; }
    std::uint8_t getLastHeaderMagazineNumber() const { return m_lastHeaderMagazineNumber; }
    std::uint8_t getLastHeaderControlInfo() const { return m_lastHeaderControlInfo; }

private:
    int m_pageDecodedCount;
    int m_headerDecodedCount;
    std::uint8_t m_lastHeaderMagazineNumber;
    std::uint8_t m_lastHeaderControlInfo;
};

// Mock Cache for testing
class MockCache : public Cache
{
public:
    MockCache()
        : m_isPageNeededResult(true)
        , m_getMutablePageResult(nullptr)
        , m_getClearPageResult(nullptr)
        , m_clearPageResults()
        , m_clearPageResultIndex(0)
        , m_isPageNeededCallCount(0)
        , m_insertPageCallCount(0)
        , m_releasePageCallCount(0)
        , m_getMutablePageCallCount(0)
        , m_getClearPageCallCount(0)
    {
    }

    virtual ~MockCache() = default;

    virtual void setCurrentPage(PageId) override
    {
        // Not needed for these tests
    }

    virtual void setLinkedPages(const PageId*, std::size_t) override
    {
        // Not needed for these tests
    }

    virtual void clear() override
    {
        // Not needed for these tests
    }

    virtual bool isPageNeeded(PageId pageId) const override
    {
        m_isPageNeededCallCount++;
        m_lastCheckedPageId = pageId;
        return m_isPageNeededResult;
    }

    virtual const PageDisplayable* getPage(PageId) override
    {
        return nullptr;
    }

    virtual PageDisplayable* getMutablePage(PageId pageId) override
    {
        m_getMutablePageCallCount++;
        m_lastRequestedPageId = pageId;
        return m_getMutablePageResult;
    }

    virtual const PageDisplayable* getNewestSubpage(PageId) override
    {
        return nullptr;
    }

    virtual PageDisplayable* getClearPage() override
    {
        m_getClearPageCallCount++;

        if (m_clearPageResultIndex < m_clearPageResults.size())
        {
            return m_clearPageResults[m_clearPageResultIndex++];
        }

        return m_getClearPageResult;
    }

    virtual void insertPage(PageDisplayable*) override
    {
        m_insertPageCallCount++;
    }

    virtual void releasePage(const PageDisplayable*) override
    {
        m_releasePageCallCount++;
    }

    // Test helpers
    void setIsPageNeeded(bool result) { m_isPageNeededResult = result; }
    void setMutablePageResult(PageDisplayable* page) { m_getMutablePageResult = page; }
    void setClearPageResult(PageDisplayable* page)
    {
        m_getClearPageResult = page;
        m_clearPageResults.clear();
        m_clearPageResultIndex = 0;
    }
    void setClearPages(const std::vector<PageDisplayable*>& pages)
    {
        m_clearPageResults = pages;
        m_clearPageResultIndex = 0;
    }

    int getInsertPageCallCount() const { return m_insertPageCallCount; }
    int getReleasePageCallCount() const { return m_releasePageCallCount; }
    int getGetMutablePageCallCount() const { return m_getMutablePageCallCount; }
    int getGetClearPageCallCount() const { return m_getClearPageCallCount; }
    int getIsPageNeededCallCount() const { return m_isPageNeededCallCount; }
    PageId getLastCheckedPageId() const { return m_lastCheckedPageId; }
    PageId getLastRequestedPageId() const { return m_lastRequestedPageId; }

    void reset()
    {
        m_isPageNeededCallCount = 0;
        m_insertPageCallCount = 0;
        m_releasePageCallCount = 0;
        m_getMutablePageCallCount = 0;
        m_getClearPageCallCount = 0;
        m_lastCheckedPageId = PageId();
        m_lastRequestedPageId = PageId();
    }

private:
    bool m_isPageNeededResult;
    PageDisplayable* m_getMutablePageResult;
    PageDisplayable* m_getClearPageResult;
    std::vector<PageDisplayable*> m_clearPageResults;
    std::size_t m_clearPageResultIndex;
    mutable int m_isPageNeededCallCount;
    mutable PageId m_lastCheckedPageId;
    int m_insertPageCallCount;
    int m_releasePageCallCount;
    int m_getMutablePageCallCount;
    int m_getClearPageCallCount;
    PageId m_lastRequestedPageId;
};

class DecoderTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DecoderTest );
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testHeaderDecodedCallback);
    CPPUNIT_TEST(testHeaderWithSubtitleFlagSet);
    CPPUNIT_TEST(testCacheReturnsValidPageForNeededPageId);
    CPPUNIT_TEST(testCacheReturnsNullForNeededPageId);
    CPPUNIT_TEST(testCacheIndicatesPageNotNeeded);
    CPPUNIT_TEST(testResetAfterHeaderProcessing);
    CPPUNIT_TEST(testMultipleResetsInSequence);
    CPPUNIT_TEST(testPartialPageReplacedByNewHeader);
    CPPUNIT_TEST(testCacheMutablePageCalledWithoutEraseFlag);
    CPPUNIT_TEST(testCacheClearPageCalledWithEraseFlag);
    CPPUNIT_TEST(testNoPageDecodedForHeaderOnly);
    CPPUNIT_TEST(testInterleavedPacketsMultipleMagazines);
    CPPUNIT_TEST(testControlInfoAllFlagsCombination);
    CPPUNIT_TEST(testControlInfoNoFlagsSet);
    CPPUNIT_TEST(testMagazineZeroMappedCorrectly);
    CPPUNIT_TEST(testMagazineSevenBoundary);
    CPPUNIT_TEST(testSubtitleFlagProcessesSecondHeader);
    CPPUNIT_TEST(testSerialModeAcrossDifferentMagazines);
    CPPUNIT_TEST(testParallelModeSameMagazineDifferentPages);
    CPPUNIT_TEST(testHeaderWithAllControlBitsSet);
    CPPUNIT_TEST(testHeaderWithNewsflashFlag);
    CPPUNIT_TEST(testHeaderWithSuppressHeaderFlag);
    CPPUNIT_TEST(testHeaderWithUpdateIndicatorFlag);
    CPPUNIT_TEST(testCachePageNeededChangesAcrossHeaders);
    CPPUNIT_TEST(testDifferentSubpageReleasesPartialPage);
    CPPUNIT_TEST(testProcessPacketDataMultipleTimes);
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
        Database database;
        MockCache cache;
        MockDecoderListener listener;

        Decoder decoder(database, cache, listener);
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        decoder.processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, listener.getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(1), listener.getLastHeaderMagazineNumber());
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

    void testCacheReturnsValidPageForNeededPageId()
    {
        PageDisplayable mockPage;
        PageId expectedPageId(0x100, 0);
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(&mockPage);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_cache->getIsPageNeededCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastCheckedPageId());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetMutablePageCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastRequestedPageId());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetClearPageCallCount());
    }

    void testCacheReturnsNullForNeededPageId()
    {
        PageId expectedPageId(0x100, 0);
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr);
        m_cache->setClearPageResult(nullptr);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_cache->getIsPageNeededCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastCheckedPageId());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetMutablePageCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastRequestedPageId());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
    }

    void testCacheIndicatesPageNotNeeded()
    {
        PageId expectedPageId(0x100, 0);
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_cache->getIsPageNeededCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastCheckedPageId());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetMutablePageCallCount());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getGetClearPageCallCount());
    }

    void testResetAfterHeaderProcessing()
    {
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);
        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());

        m_decoder->reset();
        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> headerData2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(headerData2.data(), headerData2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
    }

    void testMultipleResetsInSequence()
    {
        m_cache->setIsPageNeeded(false);

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        m_decoder->reset();
        m_decoder->reset();
        m_decoder->reset();
        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
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
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getPageDecodedCount());

        // First page should be inserted (valid with header) when second header arrives
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getInsertPageCallCount());
    }

    void testCacheMutablePageCalledWithoutEraseFlag()
    {
        PageId expectedPageId(0x100, 0);
        m_cache->setIsPageNeeded(true);
        m_cache->setMutablePageResult(nullptr);
        m_cache->setClearPageResult(nullptr);

        // Header without erase flag
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

        CPPUNIT_ASSERT_EQUAL(1, m_cache->getIsPageNeededCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastCheckedPageId());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetMutablePageCallCount());
        CPPUNIT_ASSERT(expectedPageId == m_cache->getLastRequestedPageId());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getGetClearPageCallCount());
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

    void testNoPageDecodedForHeaderOnly()
    {
        m_cache->setIsPageNeeded(false);

        // Process header
        std::vector<std::uint8_t> headerData = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader(headerData.data(), headerData.size(), nullptr, 0);

        m_decoder->processPacketData(reader);

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

    void testSubtitleFlagProcessesSecondHeader()
    {
        PageDisplayable firstPage;
        PageDisplayable secondPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPages({&firstPage, &secondPage});

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::SUBTITLE);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
        CPPUNIT_ASSERT((m_listener->getLastHeaderControlInfo() & ControlInfo::SUBTITLE) != 0);

        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getPageDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getInsertPageCallCount());
    }

    void testSerialModeAcrossDifferentMagazines()
    {
        PageDisplayable firstPage;
        PageDisplayable secondPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPages({&firstPage, &secondPage});

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> header2 = createHeaderPacket(2, 0x200, 0, ControlInfo::MAGAZINE_SERIAL);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getPageDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getInsertPageCallCount());
    }

    void testParallelModeSameMagazineDifferentPages()
    {
        PageDisplayable firstPage;
        PageDisplayable secondPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPages({&firstPage, &secondPage});

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x101, 0, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_listener->getPageDecodedCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getInsertPageCallCount());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getReleasePageCallCount());
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

    void testDifferentSubpageReleasesPartialPage()
    {
        PageDisplayable firstPage;
        PageDisplayable secondPage;
        m_cache->setIsPageNeeded(true);
        m_cache->setClearPages({&firstPage, &secondPage});

        std::vector<std::uint8_t> header1 = createHeaderPacket(1, 0x100, 0x0000, 0);
        PesPacketReader reader1(header1.data(), header1.size(), nullptr, 0);
        m_decoder->processPacketData(reader1);

        m_listener->reset();
        m_cache->reset();

        std::vector<std::uint8_t> header2 = createHeaderPacket(1, 0x100, 0x0001, 0);
        PesPacketReader reader2(header2.data(), header2.size(), nullptr, 0);
        m_decoder->processPacketData(reader2);

        CPPUNIT_ASSERT_EQUAL(1, m_listener->getHeaderDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_listener->getPageDecodedCount());
        CPPUNIT_ASSERT_EQUAL(0, m_cache->getInsertPageCallCount());
        CPPUNIT_ASSERT_EQUAL(1, m_cache->getReleasePageCallCount());
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
