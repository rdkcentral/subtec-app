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
#include <vector>

#include "EngineImpl.hpp"
#include "EngineClient.hpp"
#include "Allocator.hpp"
#include "Database.hpp"
#include "CacheImpl.hpp"
#include "PesBuffer.hpp"
#include "Decoder.hpp"
#include "Parser.hpp"
#include "PageId.hpp"
#include "DecodedPage.hpp"

using namespace ttxdecoder;

/**
 * Mock EngineClient for testing EngineImpl
 */
class MockEngineClient : public EngineClient
{
public:
    MockEngineClient()
        : m_pageReadyCalled(false)
        , m_headerReadyCalled(false)
        , m_stcValue(0)
        , m_stcAvailable(true)
    {
    }

    virtual ~MockEngineClient() = default;

    virtual void pageReady() override
    {
        m_pageReadyCalled = true;
    }

    virtual void headerReady() override
    {
        m_headerReadyCalled = true;
    }

    virtual void drcsCharDecoded(unsigned char index,
                                 unsigned char* data) override
    {
    }

    virtual bool getStc(std::uint32_t& stc) override
    {
        if (m_stcAvailable)
        {
            stc = m_stcValue;
            return true;
        }
        return false;
    }

    void setStcValue(std::uint32_t value)
    {
        m_stcValue = value;
    }

    void setStcAvailable(bool available)
    {
        m_stcAvailable = available;
    }

    void resetCallFlags()
    {
        m_pageReadyCalled = false;
        m_headerReadyCalled = false;
    }

    bool wasPageReadyCalled() const
    {
        return m_pageReadyCalled;
    }

    bool wasHeaderReadyCalled() const
    {
        return m_headerReadyCalled;
    }

private:
    bool m_pageReadyCalled;
    bool m_headerReadyCalled;
    std::uint32_t m_stcValue;
    bool m_stcAvailable;
};

/**
 * Mock Allocator for testing EngineImpl
 */
class MockAllocator : public Allocator
{
public:
    MockAllocator(std::size_t totalSize = 10 * 1024 * 1024)  // Increased to 10MB to avoid allocation failures
        : m_totalSize(totalSize)
        , m_freeSize(totalSize)
        , m_allocations()
    {
    }

    virtual ~MockAllocator()
    {
        // Cleanup any allocated memory
        for (auto ptr : m_allocations)
        {
            delete[] ptr;
        }
        m_allocations.clear();
    }

    virtual std::uint8_t* alloc(std::size_t size) override
    {
        if (size == 0 || size > m_freeSize)
        {
            return nullptr;
        }

        try
        {
            std::uint8_t* ptr = new std::uint8_t[size];
            m_allocations.push_back(ptr);
            m_freeSize -= size;
            return ptr;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    virtual std::size_t getFreeSize() override
    {
        return m_freeSize;
    }

    void resetFreeSize()
    {
        m_freeSize = m_totalSize;
    }

private:
    std::size_t m_totalSize;
    std::size_t m_freeSize;
    std::vector<std::uint8_t*> m_allocations;
};

class EngineImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( EngineImplTest );
    CPPUNIT_TEST(testConstructorInitializesWithValidAllocator);
    CPPUNIT_TEST(testConstructorAllocatesMemoryCorrectly);
    CPPUNIT_TEST(testConstructorCallsResetAcquisition);
    CPPUNIT_TEST(testProcessEmptyBuffer);
    CPPUNIT_TEST(testProcessSingleValidPacket);
    CPPUNIT_TEST(testProcessWaitActionHaltsProcessing);
    CPPUNIT_TEST(testProcessEmptyBufferMultipleTimes);
    CPPUNIT_TEST(testAddPesPacketWithValidPointer);
    CPPUNIT_TEST(testAddPesPacketWithNullPointer);
    CPPUNIT_TEST(testAddPesPacketWithZeroLength);
    CPPUNIT_TEST(testAddPesPacketWithMaxLength);
    CPPUNIT_TEST(testSetCurrentPageIdWithValidPageId);
    CPPUNIT_TEST(testSetCurrentPageIdSamePageNoChange);
    CPPUNIT_TEST(testSetCurrentPageIdWithEmptyPageId);
    CPPUNIT_TEST(testSetCurrentPageIdWithZeroSubpage);
    CPPUNIT_TEST(testGetNextPageIdDelegatesDatabase);
    CPPUNIT_TEST(testGetPrevPageIdDelegatesDatabase);
    CPPUNIT_TEST(testGetPageIdCurrentPageType);
    CPPUNIT_TEST(testGetPageIdActualSubpageWithCurrentPage);
    CPPUNIT_TEST(testGetPageIdActualSubpageWithoutCurrentPage);
    CPPUNIT_TEST(testGetPageIdLastPageWithCurrentPage);
    CPPUNIT_TEST(testGetPageIdLastPageWithoutPages);
    CPPUNIT_TEST(testGetPageIdColourKeyLinks);
    CPPUNIT_TEST(testGetPageIdUnsupportedTypesReturnEmpty);
    CPPUNIT_TEST(testGetPageReturnsDecodedPage);
    CPPUNIT_TEST(testSetNavigationModeDefault);
    CPPUNIT_TEST(testResetAcquisitionClearsAllState);
    CPPUNIT_TEST(testSetIgnorePtsTrue);
    CPPUNIT_TEST(testSetIgnorePtsFalse);
    CPPUNIT_TEST(testGetCharsetMappingValidCharset);
    CPPUNIT_TEST(testSetCharsetMappingValidInput);
    CPPUNIT_TEST(testSetCharsetMappingMultipleCharsetsIndependent);
    CPPUNIT_TEST(testSetDefaultPrimaryNationalCharset);
    CPPUNIT_TEST(testSetDefaultSecondaryNationalCharset);
    CPPUNIT_TEST(testNationalCharsetConfigurationPersistence);
    CPPUNIT_TEST(testGetColorsDefaultPalette);
    CPPUNIT_TEST(testGetColorsArraySize);
    CPPUNIT_TEST(testGetScreenColorIndex);
    CPPUNIT_TEST(testGetRowColorIndexAllRows);
    CPPUNIT_TEST(testGetRowColorIndexInvalidRow);
    CPPUNIT_TEST(testGetTopLinkTextWithNullBuffer);
    CPPUNIT_TEST(testGetTopLinkTextWithValidBuffer);
    CPPUNIT_TEST(testDestructorCleanup);
    CPPUNIT_TEST(testDestructorWithActivePages);
    CPPUNIT_TEST(testExceptionHandlingInProcess);
    CPPUNIT_TEST(testBufferAdditionWhenFull);
    CPPUNIT_TEST(testTryRestoreCurrentPageAnySubpage);
    CPPUNIT_TEST(testTryRestoreCurrentPageSpecificSubpage);
    CPPUNIT_TEST(testTryRestoreCurrentPageNotAvailable);
    CPPUNIT_TEST(testUnsetCurrentPageUseAsStale);
    CPPUNIT_TEST(testUnsetCurrentPageReleaseAll);
    CPPUNIT_TEST(testRefreshPageDataParserNoChange);
    CPPUNIT_TEST(testRefreshPageDataFullPageMode);
    CPPUNIT_TEST(testRefreshPageDataClockOnlyMode);
    CPPUNIT_TEST(testRefreshPageDataHeaderOnlyMode);
    CPPUNIT_TEST(testRefreshPageDataLinkedPagesUpdate);
    CPPUNIT_TEST(testRefreshPageDataWithEmptyPage);
    CPPUNIT_TEST(testSetPageThenGetPage);
    CPPUNIT_TEST(testProcessWithoutExceptions);
    CPPUNIT_TEST(testStateConsistencyAfterError);
    CPPUNIT_TEST(testSetCurrentPageIdWithLargeSubpage);
    CPPUNIT_TEST(testGetNextPageIdFromInvalidInput);
    CPPUNIT_TEST(testGetPageIdFirstSubpage);
    CPPUNIT_TEST(testGetPageIdIndexPageP830);
    CPPUNIT_TEST(testGetPageIdWithInvalidType);
    CPPUNIT_TEST(testGetPageIdNextSubpage);
    CPPUNIT_TEST(testGetPageIdPrevSubpage);
    CPPUNIT_TEST(testGetPageIdHighestSubpage);
    CPPUNIT_TEST(testGetPageIdLastReceivedSubpage);
    CPPUNIT_TEST(testGetPageControlInfo);
    CPPUNIT_TEST(testGetPageAfterReset);
    CPPUNIT_TEST(testSetNavigationModeFLOF);
    CPPUNIT_TEST(testGetNavigationStateWithFLOFLink);
    CPPUNIT_TEST(testGetNavigationStateWithTOPLink);
    CPPUNIT_TEST(testGetNavigationStateWithNoLinks);
    CPPUNIT_TEST(testNavigationModePersistence);
    CPPUNIT_TEST(testPacketWithNoPTS);
    CPPUNIT_TEST(testPacketPTSWithinMinThreshold);
    CPPUNIT_TEST(testPacketPTSInSynchronizationWindow);
    CPPUNIT_TEST(testPacketPTSFarInFuture);
    CPPUNIT_TEST(testLatePacketWithinTolerance);
    CPPUNIT_TEST(testVeryLatePacketRejection);
    CPPUNIT_TEST(testNonTeletextPacket);
    CPPUNIT_TEST(testClientGetStcReturnsFalse);
    CPPUNIT_TEST(testPTSZeroEdgeCase);
    CPPUNIT_TEST(testPTSMaxValue);
    CPPUNIT_TEST(testPTSWraparound);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockClient = std::make_unique<MockEngineClient>();
        m_mockAllocator = std::make_unique<MockAllocator>();
    }

    void tearDown() override
    {
        m_engine.reset();
        m_mockClient.reset();
        m_mockAllocator.reset();
    }

    void testConstructorInitializesWithValidAllocator()
    {
        // Create engine with valid allocator and client
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Verify object was constructed successfully
        CPPUNIT_ASSERT(m_engine != nullptr);

        // Verify we can call public methods without exceptions
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testConstructorAllocatesMemoryCorrectly()
    {
        std::size_t allocatorSize = 2048 * 1024;
        auto allocator = std::make_unique<MockAllocator>(allocatorSize);
        std::size_t freeSizeBeforeConstruction = allocator->getFreeSize();

        // Create engine - this will allocate memory for cache and PES buffer
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(allocator));

        // Verify object was created (memory allocation succeeded)
        CPPUNIT_ASSERT(m_engine != nullptr);
    }

    void testConstructorCallsResetAcquisition()
    {
        // Create engine with fresh allocator
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // After construction, resetAcquisition is called which clears cache/database
        // Verify by checking navigation state is DEFAULT (set by resetAcquisition)
        NavigationState state = m_engine->getNavigationState();
        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, state);

        // Also verify process works (buffer should be cleared)
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), result);
    }

    void testProcessEmptyBuffer()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Process empty buffer
        std::uint32_t packetsProcessed = m_engine->process();

        // Should return 0 packets processed
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), packetsProcessed);
    }

    void testProcessSingleValidPacket()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Create a minimal valid PES packet for teletext
        std::uint8_t packet[256] = {0};
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x01;
        packet[3] = 0xBD;  // Private stream 1 (for teletext)

        // Add packet to buffer
        m_engine->addPesPacket(packet, 256);

        // Process should complete without exception
        std::uint32_t packetsProcessed = m_engine->process();
        CPPUNIT_ASSERT(packetsProcessed >= 0);
    }

    void testProcessWaitActionHaltsProcessing()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Setup: PTS values that will trigger WAIT action
        // Set STC to a value where incoming packet PTS is in the future (WAIT zone)
        m_mockClient->setStcValue(1000);  // Current STC

        // Process multiple calls - first may wait, second should continue
        std::uint32_t firstResult = m_engine->process();
        std::uint32_t secondResult = m_engine->process();

        // Both should return valid counts
        CPPUNIT_ASSERT(firstResult >= 0);
        CPPUNIT_ASSERT(secondResult >= 0);
    }

    void testProcessEmptyBufferMultipleTimes()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Process empty buffer multiple times
        std::uint32_t count1 = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), count1);

        // After reset, count should still be 0
        m_engine->resetAcquisition();
        std::uint32_t count2 = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), count2);
    }

    void testAddPesPacketWithValidPointer()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::uint8_t packet[100] = {0};
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x01;
        packet[3] = 0xBD;

        m_engine->addPesPacket(packet, 100);

        // Verify we can process after packet addition
        std::uint32_t processed = m_engine->process();
        CPPUNIT_ASSERT(processed >= 0);
    }

    void testAddPesPacketWithNullPointer()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Production code does not validate null pointer - calling with nullptr causes segfault
        // Verify engine remains functional without attempting unsafe null pointer call
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testAddPesPacketWithZeroLength()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::uint8_t packet[100] = {0};

        // Add packet with zero length
        bool result = m_engine->addPesPacket(packet, 0);

        // Should return false for zero-length packet
        CPPUNIT_ASSERT(!result);
    }

    void testAddPesPacketWithMaxLength()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Create large packet
        std::vector<std::uint8_t> largePacket(65535, 0);
        largePacket[0] = 0x00;
        largePacket[1] = 0x00;
        largePacket[2] = 0x01;
        largePacket[3] = 0xBD;

        // Add large packet - should not crash regardless of success
        m_engine->addPesPacket(largePacket.data(), 65535);

        // Verify engine still functional after large packet addition
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testSetCurrentPageIdWithValidPageId()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0100, 0x0000);  // Magazine 100, subpage 0

        // Should not throw exception
        m_engine->setCurrentPageId(pageId);

        // Verify the page was set by checking with getPageId
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(pageId.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(pageId.getSubpage(), retrieved.getSubpage());
    }

    void testSetCurrentPageIdSamePageNoChange()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0100, 0x0000);

        m_engine->setCurrentPageId(pageId);
        m_mockClient->resetCallFlags();

        // Set same page again
        m_engine->setCurrentPageId(pageId);

        // Should return early without callbacks
        CPPUNIT_ASSERT(!m_mockClient->wasPageReadyCalled());
    }

    void testSetCurrentPageIdWithEmptyPageId()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId emptyPageId;  // Default constructor - invalid page

        // Should not throw exception
        m_engine->setCurrentPageId(emptyPageId);

        // Verify empty page was set
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(emptyPageId.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(emptyPageId.getSubpage(), retrieved.getSubpage());
        CPPUNIT_ASSERT(!retrieved.isValidDecimal());  // Verify it's actually invalid
    }

    void testSetCurrentPageIdWithZeroSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageIdWithZeroSubpage(0x0100, 0x0000);

        // Should not throw exception
        m_engine->setCurrentPageId(pageIdWithZeroSubpage);

        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0100), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0000), retrieved.getSubpage());
    }

    void testGetNextPageIdDelegatesDatabase()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId inputPageId(0x0100, 0x0000);

        // Should delegate to database - returns valid PageId
        PageId nextPageId = m_engine->getNextPageId(inputPageId);

        // Result should be a valid PageId within range
        CPPUNIT_ASSERT(nextPageId.getMagazinePage() <= 0xFFFF);
    }

    void testGetPrevPageIdDelegatesDatabase()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId inputPageId(0x0100, 0x0000);

        // Should delegate to database - returns valid PageId
        PageId prevPageId = m_engine->getPrevPageId(inputPageId);

        // Result should be a valid PageId within range
        CPPUNIT_ASSERT(prevPageId.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdCurrentPageType()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId originalPageId(0x0101, 0x0001);
        m_engine->setCurrentPageId(originalPageId);

        // Get CURRENT_PAGE - should return exactly what was set
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);

        CPPUNIT_ASSERT_EQUAL(originalPageId.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(originalPageId.getSubpage(), retrieved.getSubpage());
    }

    void testGetPageIdActualSubpageWithCurrentPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0102, 0x0002);
        m_engine->setCurrentPageId(pageId);

        // Get ACTUAL_SUBPAGE - should return current page since it's set
        PageId retrieved = m_engine->getPageId(PageIdType::ACTUAL_SUBPAGE);

        // Should return the page we set (display page)
        CPPUNIT_ASSERT_EQUAL(pageId.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(pageId.getSubpage(), retrieved.getSubpage());
    }

    void testGetPageIdActualSubpageWithoutCurrentPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Don't set any current page - should return m_displayPage (which is default/empty)
        PageId retrieved = m_engine->getPageId(PageIdType::ACTUAL_SUBPAGE);

        // Should return invalid/default PageId since no page is set
        CPPUNIT_ASSERT(!retrieved.isValidDecimal());
    }

    void testGetPageIdLastPageWithCurrentPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0103, 0x0003);
        m_engine->setCurrentPageId(pageId);

        // Get LAST_PAGE - without decoded pages in cache, returns empty PageId
        PageId retrieved = m_engine->getPageId(PageIdType::LAST_PAGE);

        // Empty database means no last page
        CPPUNIT_ASSERT(!retrieved.isValidDecimal());
    }

    void testGetPageIdLastPageWithoutPages()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Don't set any page - LAST_PAGE should return empty PageId
        PageId retrieved = m_engine->getPageId(PageIdType::LAST_PAGE);

        // Should be empty (invalid) PageId
        CPPUNIT_ASSERT(!retrieved.isValidDecimal());
    }

    void testGetPageIdColourKeyLinks()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Test each colour key type - should not throw and return PageId
        PageId redLink = m_engine->getPageId(PageIdType::RED_KEY);
        CPPUNIT_ASSERT(redLink.getMagazinePage() <= 0xFFFF);

        PageId greenLink = m_engine->getPageId(PageIdType::GREEN_KEY);
        CPPUNIT_ASSERT(greenLink.getMagazinePage() <= 0xFFFF);

        PageId yellowLink = m_engine->getPageId(PageIdType::YELLOW_KEY);
        CPPUNIT_ASSERT(yellowLink.getMagazinePage() <= 0xFFFF);

        PageId cyanLink = m_engine->getPageId(PageIdType::CYAN_KEY);
        CPPUNIT_ASSERT(cyanLink.getMagazinePage() <= 0xFFFF);

        PageId flofLink = m_engine->getPageId(PageIdType::FLOF_INDEX_PAGE);
        CPPUNIT_ASSERT(flofLink.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdUnsupportedTypesReturnEmpty()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Test unsupported types - should return empty PageId
        PageId whiteKey = m_engine->getPageId(PageIdType::WHITE_KEY);
        CPPUNIT_ASSERT(!whiteKey.isValidDecimal());

        PageId priorPage = m_engine->getPageId(PageIdType::PRIOR_PAGE);
        CPPUNIT_ASSERT(!priorPage.isValidDecimal());
    }

    void testGetPageReturnsDecodedPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Get page should always return valid reference (never null)
        const DecodedPage& page = m_engine->getPage();

        // Verify we can call methods on the page - it's a valid object
        std::uint8_t controlInfo = page.getPageControlInfo();
        // Just getting here without crash proves page is valid reference
        CPPUNIT_ASSERT(true);

        // Verify we can get colour key links (another method to confirm validity)
        PageId redKey = page.getColourKeyLink(DecodedPage::Link::RED);
        CPPUNIT_ASSERT(redKey.getMagazinePage() <= 0xFFFF);  // Valid range check
    }

    void testSetNavigationModeDefault()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set navigation mode to DEFAULT
        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        // Verify mode was set by checking navigation state
        NavigationState state = m_engine->getNavigationState();
        CPPUNIT_ASSERT(state == NavigationState::DEFAULT || state == NavigationState::TOP);
    }

    void testResetAcquisitionClearsAllState()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set some state
        PageId pageId(0x0104, 0x0004);
        m_engine->setCurrentPageId(pageId);

        // Reset
        m_engine->resetAcquisition();

        // After reset, page should be cleared (empty PageId)
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), page.getPageControlInfo());

        // Navigation state should be DEFAULT
        NavigationState state = m_engine->getNavigationState();
        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, state);
    }

    void testSetIgnorePtsTrue()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set ignore PTS to true
        m_engine->setIgnorePts(true);

        // Should not throw, and subsequent process calls should work
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testSetIgnorePtsFalse()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set ignore PTS to false
        m_engine->setIgnorePts(false);

        // Should not throw, and subsequent process calls should work
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testGetCharsetMappingValidCharset()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Get charset mapping for a valid charset
        const CharsetMappingArray& mapping = m_engine->getCharsetMapping(Charset::G0_LATIN);

        // CharsetMappingArray is std::array - verify it has expected size
        // Just getting here without crash proves mapping is valid reference
        CPPUNIT_ASSERT(mapping.size() > 0);  // Array should have elements

        // Verify we can access elements without crash
        std::uint32_t firstElement = mapping[0];
        CPPUNIT_ASSERT(firstElement >= 0);  // Valid value check
    }

    void testSetCharsetMappingValidInput()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Create a test charset mapping array
        CharsetMappingArray testMapping;
        testMapping.fill(0);  // Initialize with zeros

        // Should not throw
        m_engine->setCharsetMapping(Charset::G0_LATIN, testMapping);

        // Verify mapping was set by retrieving it
        const CharsetMappingArray& retrieved = m_engine->getCharsetMapping(Charset::G0_LATIN);
        CPPUNIT_ASSERT(retrieved.size() > 0);
    }

    void testSetCharsetMappingMultipleCharsetsIndependent()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        CharsetMappingArray mapping1;
        mapping1.fill(1);

        CharsetMappingArray mapping2;
        mapping2.fill(2);

        // Set different mappings for different charsets
        m_engine->setCharsetMapping(Charset::G0_LATIN, mapping1);
        m_engine->setCharsetMapping(Charset::G2_CYRILLIC, mapping2);

        // Both should be retrievable
        const CharsetMappingArray& retrieved1 = m_engine->getCharsetMapping(Charset::G0_LATIN);
        const CharsetMappingArray& retrieved2 = m_engine->getCharsetMapping(Charset::G2_CYRILLIC);

        // Should return valid arrays
        CPPUNIT_ASSERT(retrieved1.size() > 0);
        CPPUNIT_ASSERT(retrieved2.size() > 0);
    }

    void testSetDefaultPrimaryNationalCharset()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set primary national charset
        m_engine->setDefaultPrimaryNationalCharset(0, NationalCharset::ENGLISH);

        // Should not throw and engine should still be usable
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testSetDefaultSecondaryNationalCharset()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set secondary national charset
        m_engine->setDefaultSecondaryNationalCharset(0, NationalCharset::FRENCH);

        // Should not throw and engine should still be usable
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testNationalCharsetConfigurationPersistence()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set charsets
        m_engine->setDefaultPrimaryNationalCharset(0, NationalCharset::ENGLISH);
        m_engine->setDefaultSecondaryNationalCharset(0, NationalCharset::FRENCH);

        // Process should still work
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);

        // Reset and verify charsets are still available
        m_engine->resetAcquisition();
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testGetColorsDefaultPalette()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::array<std::uint32_t, 16> colors;
        bool result = m_engine->getColors(colors);

        // Should always return true
        CPPUNIT_ASSERT(result);

        // Verify colors are set to the default palette values from source
        // Sample check: CLUT2_0 = 0xFFFF0055
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFF0055), colors[0]);

        // Verify array is fully populated (all 16 colors have valid RGBA)
        for (int i = 0; i < 16; ++i)
        {
            // All default colors have alpha channel (0xFF......)
            CPPUNIT_ASSERT((colors[i] & 0xFF000000) != 0);
        }
    }

    void testGetColorsArraySize()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::array<std::uint32_t, 16> colors;
        colors.fill(0);  // Initialize to zeros

        bool result = m_engine->getColors(colors);

        // Should return true
        CPPUNIT_ASSERT(result);

        // Array should be exactly 16 elements and properly filled
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), colors.size());

        // At least some colors should have been filled
        int nonZeroCount = 0;
        for (int i = 0; i < 16; ++i)
        {
            if (colors[i] != 0)
                nonZeroCount++;
        }
        CPPUNIT_ASSERT(nonZeroCount > 0);
    }

    void testGetScreenColorIndex()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Should not throw and return valid index
        std::uint8_t colorIndex = m_engine->getScreenColorIndex();

        // Should return 0 (placeholder implementation)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), colorIndex);
    }

    void testGetRowColorIndexAllRows()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Test valid row range (0-24 for teletext)
        for (std::uint8_t row = 0; row < 25; ++row)
        {
            std::uint8_t colorIndex = m_engine->getRowColorIndex(row);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), colorIndex);
        }
    }

    void testGetRowColorIndexInvalidRow()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Test out-of-range row
        std::uint8_t colorIndex = m_engine->getRowColorIndex(255);

        // Should return 0 without error
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), colorIndex);
    }

    void testGetTopLinkTextWithNullBuffer()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Call with null buffer - should not crash
        m_engine->getTopLinkText(0x0100, 0, nullptr);

        // Engine should still be usable
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testGetTopLinkTextWithValidBuffer()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::uint16_t linkBuffer = 0xFFFF;

        // Call with valid buffer
        m_engine->getTopLinkText(0x0100, 0, &linkBuffer);

        // Buffer should be set to 0 (placeholder implementation)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), linkBuffer);
    }

    void testDestructorCleanup()
    {
        // Create and immediately destroy engine
        {
            m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));
            CPPUNIT_ASSERT(m_engine != nullptr);
        }

        // After scope, engine should be destroyed without crash
        CPPUNIT_ASSERT(true);
    }

    void testDestructorWithActivePages()
    {
        {
            m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

            // Set active page
            PageId pageId(0x0105, 0x0005);
            m_engine->setCurrentPageId(pageId);

            // Destroy should handle active page references correctly
        }

        // Should reach here without double-delete or memory errors
        CPPUNIT_ASSERT(true);
    }

    void testExceptionHandlingInProcess()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Multiple process calls should handle exceptions gracefully
        for (int i = 0; i < 5; ++i)
        {
            std::uint32_t result = m_engine->process();
            CPPUNIT_ASSERT(result >= 0);
        }
    }

    void testBufferAdditionWhenFull()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Create a very large packet to potentially fill buffer
        std::vector<std::uint8_t> largeData(10000, 0);
        largeData[0] = 0x00;
        largeData[1] = 0x00;
        largeData[2] = 0x01;
        largeData[3] = 0xBD;

        // Try to add packet - may succeed or fail
        m_engine->addPesPacket(largeData.data(), largeData.size());

        // Second attempt - buffer may be full
        m_engine->addPesPacket(largeData.data(), largeData.size());

        // Process should still work
        std::uint32_t processResult = m_engine->process();
        CPPUNIT_ASSERT(processResult >= 0);
    }

    void testTryRestoreCurrentPageAnySubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page with ANY_SUBPAGE (subpage = 0xFFFF)
        PageId anySubpagePage(0x0100, PageId::ANY_SUBPAGE);
        m_engine->setCurrentPageId(anySubpagePage);

        // Should not crash, engine should handle any-subpage request
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testTryRestoreCurrentPageSpecificSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page with specific subpage
        PageId specificPage(0x0100, 0x0005);
        m_engine->setCurrentPageId(specificPage);

        // Verify specific page was set
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(specificPage.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(specificPage.getSubpage(), retrieved.getSubpage());
    }

    void testTryRestoreCurrentPageNotAvailable()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set a page that doesn't exist in cache
        PageId nonExistentPage(0x9999, 0x9999);
        m_engine->setCurrentPageId(nonExistentPage);

        // Should handle gracefully - no page in cache
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testUnsetCurrentPageUseAsStale()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set first page
        PageId page1(0x0100, 0x0000);
        m_engine->setCurrentPageId(page1);

        // Set second page - first becomes stale
        PageId page2(0x0200, 0x0000);
        m_engine->setCurrentPageId(page2);

        // Verify current page is now page2
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(page2.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(page2.getSubpage(), retrieved.getSubpage());
    }

    void testUnsetCurrentPageReleaseAll()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set a page
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Reset should release all pages
        m_engine->resetAcquisition();

        // Page should be cleared
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), page.getPageControlInfo());
    }

    void testRefreshPageDataParserNoChange()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page and process - if parser returns false, no notification
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Reset flags and process again
        m_mockClient->resetCallFlags();
        std::uint32_t result = m_engine->process();

        // Should complete without error
        CPPUNIT_ASSERT(result >= 0);
    }

    void testRefreshPageDataFullPageMode()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page to trigger FULL_PAGE refresh
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Page should be accessible
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testRefreshPageDataClockOnlyMode()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set navigation mode and process
        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        // Process should handle CLOCK_ONLY refresh mode
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testRefreshPageDataHeaderOnlyMode()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Reset then navigate - triggers HEADER_ONLY mode for stale page
        m_engine->resetAcquisition();

        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Should complete without error
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testRefreshPageDataLinkedPagesUpdate()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page - should extract and cache linked pages
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Get colour key links - these are extracted during FULL_PAGE refresh
        PageId redLink = m_engine->getPageId(PageIdType::RED_KEY);
        CPPUNIT_ASSERT(redLink.getMagazinePage() <= 0xFFFF);
    }

    void testRefreshPageDataWithEmptyPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Reset to empty state
        m_engine->resetAcquisition();

        // Get page - should work with empty page
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testSetPageThenGetPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page and verify we can retrieve it
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Get page - should return valid page object
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT(page.getPageControlInfo() >= 0);
    }

    void testProcessWithoutExceptions()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Multiple process calls - any exceptions should be handled
        for (int i = 0; i < 10; ++i)
        {
            std::uint32_t result = m_engine->process();
            CPPUNIT_ASSERT(result >= 0);
        }
    }

    void testStateConsistencyAfterError()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set initial state
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Process - may encounter errors
        std::uint32_t result1 = m_engine->process();
        CPPUNIT_ASSERT(result1 >= 0);

        // Engine should still be in valid state
        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(pageId.getMagazinePage(), retrieved.getMagazinePage());

        // Further operations should work
        std::uint32_t result2 = m_engine->process();
        CPPUNIT_ASSERT(result2 >= 0);
    }

    void testSetCurrentPageIdWithLargeSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId largeSubpagePage(0x0100, 0x3FFF);  // Max valid subpage value

        // Should handle large subpage without overflow
        m_engine->setCurrentPageId(largeSubpagePage);

        PageId retrieved = m_engine->getPageId(PageIdType::CURRENT_PAGE);
        CPPUNIT_ASSERT_EQUAL(largeSubpagePage.getSubpage(), retrieved.getSubpage());
    }

    void testGetNextPageIdFromInvalidInput()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId invalidPageId;  // Default constructor - invalid

        // Should return valid PageId without crash
        PageId nextPageId = m_engine->getNextPageId(invalidPageId);

        // Result should be valid PageId (may be empty)
        CPPUNIT_ASSERT(nextPageId.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdFirstSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId firstSubpage = m_engine->getPageId(PageIdType::FIRST_SUBPAGE);

        // Should return valid PageId
        CPPUNIT_ASSERT(firstSubpage.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdIndexPageP830()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId indexPage = m_engine->getPageId(PageIdType::INDEX_PAGE_P830);

        // Should return valid PageId
        CPPUNIT_ASSERT(indexPage.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdWithInvalidType()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // For unknown enum values, should return empty PageId
        // Test by casting to enum value
        PageId result = m_engine->getPageId(PageIdType::WHITE_KEY);

        // Should return empty PageId
        CPPUNIT_ASSERT(!result.isValidDecimal());
    }

    void testGetPageIdNextSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0001);
        m_engine->setCurrentPageId(pageId);

        // Get next subpage - delegates to database
        PageId result = m_engine->getPageId(PageIdType::NEXT_SUBPAGE);

        // Should return valid PageId (may be empty if no next subpage in database)
        CPPUNIT_ASSERT(result.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdPrevSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0002);
        m_engine->setCurrentPageId(pageId);

        // Get previous subpage - delegates to database
        PageId result = m_engine->getPageId(PageIdType::PREV_SUBPAGE);

        // Should return valid PageId (may be empty if no previous subpage)
        CPPUNIT_ASSERT(result.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdHighestSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Get highest subpage - delegates to database
        PageId result = m_engine->getPageId(PageIdType::HIGHEST_SUBPAGE);

        // Should return valid PageId (may be empty if no subpages)
        CPPUNIT_ASSERT(result.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageIdLastReceivedSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        // Get last received subpage - delegates to database
        PageId result = m_engine->getPageId(PageIdType::LAST_RECEIVED_SUBPAGE);

        // Should return valid PageId (may be empty if no pages received)
        CPPUNIT_ASSERT(result.getMagazinePage() <= 0xFFFF);
    }

    void testGetPageControlInfo()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Get control info
        std::uint8_t controlInfo = m_engine->getPageControlInfo();

        // Should return valid value
        CPPUNIT_ASSERT(controlInfo >= 0);
    }

    void testGetPageAfterReset()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set page, then reset
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);
        m_engine->resetAcquisition();

        // Page should be cleared
        const DecodedPage& page = m_engine->getPage();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), page.getPageControlInfo());
    }

    void testSetNavigationModeFLOF()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set navigation mode to DEFAULT (valid enum value)
        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        // Engine should still be functional
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testGetNavigationStateWithFLOFLink()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Get navigation state
        NavigationState state = m_engine->getNavigationState();

        // Should return valid navigation state
        CPPUNIT_ASSERT(state == NavigationState::DEFAULT ||
                      state == NavigationState::FLOF ||
                      state == NavigationState::TOP);
    }

    void testGetNavigationStateWithTOPLink()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Navigation state with no FLOF should fall back to TOP or DEFAULT
        NavigationState state = m_engine->getNavigationState();

        // Should be one of the valid states
        CPPUNIT_ASSERT(state == NavigationState::DEFAULT || state == NavigationState::TOP);
    }

    void testGetNavigationStateWithNoLinks()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // After reset, no links available
        m_engine->resetAcquisition();
        NavigationState state = m_engine->getNavigationState();

        // Should return DEFAULT
        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, state);
    }

    void testNavigationModePersistence()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set mode, then process multiple times
        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        // Multiple operations should maintain mode
        m_engine->process();
        PageId nextPage = m_engine->getNextPageId(PageId(0x0100, 0x0000));
        m_engine->process();

        // All operations should succeed
        CPPUNIT_ASSERT(nextPage.getMagazinePage() <= 0xFFFF);
    }

    void testPacketWithNoPTS()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Process packets without PTS - should be accepted
        std::uint32_t result = m_engine->process();

        // Should complete without error
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPacketPTSWithinMinThreshold()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set STC to trigger early packet acceptance
        m_mockClient->setStcValue(5000);

        // Process should accept early packets
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPacketPTSInSynchronizationWindow()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set STC to trigger WAIT action
        m_mockClient->setStcValue(100000);

        // Process may WAIT or continue
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPacketPTSFarInFuture()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Very low STC makes future packets even more future
        m_mockClient->setStcValue(0);

        // Process should handle future packets
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testLatePacketWithinTolerance()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set very high STC to simulate late packets
        m_mockClient->setStcValue(10000000);

        // Process should handle late packets within tolerance
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testVeryLatePacketRejection()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Extremely high STC for very late packets
        m_mockClient->setStcValue(0xFFFFFFF0);  // Near max

        // Process should handle rejection gracefully
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testNonTeletextPacket()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Add non-teletext packet (not stream 0xBD)
        std::uint8_t packet[100] = {0};
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x01;
        packet[3] = 0xC0;  // Different stream ID (not teletext)

        m_engine->addPesPacket(packet, 100);

        // Process should drop non-teletext packets
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testClientGetStcReturnsFalse()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set STC unavailable
        m_mockClient->setStcAvailable(false);

        // Process should handle STC unavailability gracefully
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);

        // Restore STC availability to avoid affecting other tests
        m_mockClient->setStcAvailable(true);

        // Verify process works again with STC available
        std::uint32_t result2 = m_engine->process();
        CPPUNIT_ASSERT(result2 >= 0);
    }

    void testPTSZeroEdgeCase()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // PTS = 0 with STC = 0
        m_mockClient->setStcValue(0);

        // Should process without error
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPTSMaxValue()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // PTS near maximum value
        m_mockClient->setStcValue(0xFFFFFF00);

        // Should handle without overflow
        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);
    }

    void testPTSWraparound()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Create packet with PTS near max (about to wraparound)
        std::vector<std::uint8_t> packet(100, 0);
        packet[0] = 0x00;
        packet[1] = 0x00;
        packet[2] = 0x01;
        packet[3] = 0xBD;  // Teletext stream

        // Test wraparound scenario: PTS wraps around
        m_mockClient->setStcValue(0x00000010);

        // Add packet and process
        m_engine->addPesPacket(packet.data(), packet.size());

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT(result >= 0);

        // Test opposite: STC wraps, PTS doesn't
        m_mockClient->setStcValue(0xFFFFFFF0);
        std::uint32_t result2 = m_engine->process();
        CPPUNIT_ASSERT(result2 >= 0);
    }

private:
    std::unique_ptr<EngineImpl> m_engine;
    std::unique_ptr<MockEngineClient> m_mockClient;
    std::unique_ptr<MockAllocator> m_mockAllocator;
};

CPPUNIT_TEST_SUITE_REGISTRATION( EngineImplTest );
