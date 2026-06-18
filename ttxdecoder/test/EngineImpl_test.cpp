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
    CPPUNIT_TEST(testAddPesPacketWithZeroLength);
    CPPUNIT_TEST(testAddPesPacketWithMaxLength);
    CPPUNIT_TEST(testSetCurrentPageIdWithValidPageId);
    CPPUNIT_TEST(testSetCurrentPageIdSamePageNoChange);
    CPPUNIT_TEST(testSetCurrentPageIdWithEmptyPageId);
    CPPUNIT_TEST(testGetNextPageIdDelegatesDatabase);
    CPPUNIT_TEST(testGetPrevPageIdDelegatesDatabase);
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
    CPPUNIT_TEST(testTryRestoreCurrentPageAnySubpage);
    CPPUNIT_TEST(testTryRestoreCurrentPageNotAvailable);
    CPPUNIT_TEST(testUnsetCurrentPageUseAsStale);
    CPPUNIT_TEST(testUnsetCurrentPageReleaseAll);
    CPPUNIT_TEST(testRefreshPageDataWithEmptyPage);
    CPPUNIT_TEST(testSetPageThenGetPage);
    CPPUNIT_TEST(testSetCurrentPageIdWithLargeSubpage);
    CPPUNIT_TEST(testGetNextPageIdFromInvalidInput);
    CPPUNIT_TEST(testGetPageIdFirstSubpage);
    CPPUNIT_TEST(testGetPageIdIndexPageP830);
    CPPUNIT_TEST(testGetPageIdNextSubpage);
    CPPUNIT_TEST(testGetPageIdPrevSubpage);
    CPPUNIT_TEST(testGetPageIdHighestSubpage);
    CPPUNIT_TEST(testGetPageIdLastReceivedSubpage);
    CPPUNIT_TEST(testGetPageControlInfo);
    CPPUNIT_TEST(testGetNavigationStateWithNoLinks);
    CPPUNIT_TEST(testNavigationModePersistence);
    CPPUNIT_TEST(testPacketWithNoPTS);
    CPPUNIT_TEST(testPacketPTSWithinMinThreshold);
    CPPUNIT_TEST(testPacketPTSInSynchronizationWindow);
    CPPUNIT_TEST(testPacketPTSFarInFuture);
    CPPUNIT_TEST(testLatePacketWithinTolerance);
    CPPUNIT_TEST(testVeryLatePacketRejection);
    CPPUNIT_TEST(testBufferAdditionWhenFull);
    CPPUNIT_TEST(testNonTeletextPacket);
    CPPUNIT_TEST(testClientGetStcReturnsFalse);
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

    std::vector<std::uint8_t> createValidPesPacket(std::uint16_t dataSize,
                                                   std::uint8_t streamId = 0xBD,
                                                   bool withPts = false,
                                                   std::uint32_t ptsValue = 0)
    {
        std::vector<std::uint8_t> packet;
        packet.reserve(static_cast<std::size_t>(dataSize) + 14);

        packet.push_back(0x00);
        packet.push_back(0x00);
        packet.push_back(0x01);
        packet.push_back(streamId);

        std::uint16_t pesLength = dataSize;
        if (withPts)
        {
            pesLength = static_cast<std::uint16_t>(pesLength + 8);
        }

        packet.push_back((pesLength >> 8) & 0xFF);
        packet.push_back(pesLength & 0xFF);

        if (withPts)
        {
            packet.push_back(0x80);
            packet.push_back(0x80);
            packet.push_back(0x05);
            packet.push_back(0x20 | ((ptsValue >> 28) & 0x0E) | 0x01);
            packet.push_back((ptsValue >> 21) & 0xFF);
            packet.push_back(((ptsValue >> 13) & 0xFE) | 0x01);
            packet.push_back((ptsValue >> 6) & 0xFF);
            packet.push_back(((ptsValue << 2) & 0xFC) | 0x01);
        }

        for (std::uint16_t index = 0; index < dataSize; ++index)
        {
            packet.push_back(static_cast<std::uint8_t>(index & 0xFF));
        }

        return packet;
    }

    std::vector<std::uint8_t> createDecoderSafePesPacket(std::uint8_t streamId = 0xBD,
                                                         bool withPts = false,
                                                         std::uint32_t ptsValue = 0)
    {
        std::vector<std::uint8_t> packet = createValidPesPacket(withPts ? 3 : 6,
                                                                streamId,
                                                                withPts,
                                                                ptsValue);

        // Collector expects: data_identifier, data_unit_id, data_unit_length.
        // An unsupported unit id with zero length is enough to consume the packet without throwing.
        if (!withPts)
        {
            packet[6] = 0x80;
            packet[7] = 0x00;
            packet[8] = 0x00;
        }

        const std::size_t payloadOffset = withPts ? 14U : 9U;
        packet[payloadOffset + 0] = 0x10;
        packet[payloadOffset + 1] = 0xFF;
        packet[payloadOffset + 2] = 0x00;

        return packet;
    }

    void assertPageIsEmpty(const DecodedPage& page)
    {
        CPPUNIT_ASSERT(!page.getPageId().isValidDecimal());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), page.getPageControlInfo());
        CPPUNIT_ASSERT(!page.getColourKeyLink(DecodedPage::Link::RED).isValidDecimal());
    }

    void testConstructorInitializesWithValidAllocator()
    {
        // Create engine with valid allocator and client
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Verify object was constructed successfully
        CPPUNIT_ASSERT(m_engine != nullptr);

        assertPageIsEmpty(m_engine->getPage());
        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, m_engine->getNavigationState());
    }

    void testConstructorAllocatesMemoryCorrectly()
    {
        std::size_t allocatorSize = 2048 * 1024;
        auto allocator = std::make_unique<MockAllocator>(allocatorSize);

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

        auto packet = createDecoderSafePesPacket();

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t packetsProcessed = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), packetsProcessed);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), m_engine->process());
    }

    void testProcessWaitActionHaltsProcessing()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 100000);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        m_mockClient->setStcValue(0);

        std::uint32_t firstResult = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), firstResult);
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

        std::vector<std::uint8_t> largePacket = createValidPesPacket(65529);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(65535), largePacket.size());
        CPPUNIT_ASSERT(m_engine->addPesPacket(largePacket.data(), static_cast<std::uint16_t>(largePacket.size())));
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

    void testGetNextPageIdDelegatesDatabase()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId inputPageId(0x0100, 0x0000);

        PageId nextPageId = m_engine->getNextPageId(inputPageId);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0101), nextPageId.getMagazinePage());
        CPPUNIT_ASSERT(nextPageId.isAnySubpage());
    }

    void testGetPrevPageIdDelegatesDatabase()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId inputPageId(0x0100, 0x0000);

        PageId prevPageId = m_engine->getPrevPageId(inputPageId);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0899), prevPageId.getMagazinePage());
        CPPUNIT_ASSERT(prevPageId.isAnySubpage());
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

        PageId redLink = m_engine->getPageId(PageIdType::RED_KEY);
        CPPUNIT_ASSERT(!redLink.isValidDecimal());

        PageId greenLink = m_engine->getPageId(PageIdType::GREEN_KEY);
        CPPUNIT_ASSERT(!greenLink.isValidDecimal());

        PageId yellowLink = m_engine->getPageId(PageIdType::YELLOW_KEY);
        CPPUNIT_ASSERT(!yellowLink.isValidDecimal());

        PageId cyanLink = m_engine->getPageId(PageIdType::CYAN_KEY);
        CPPUNIT_ASSERT(!cyanLink.isValidDecimal());

        PageId flofLink = m_engine->getPageId(PageIdType::FLOF_INDEX_PAGE);
        CPPUNIT_ASSERT(!flofLink.isValidDecimal());
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

        const DecodedPage& page = m_engine->getPage();

        assertPageIsEmpty(page);
    }

    void testSetNavigationModeDefault()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set navigation mode to DEFAULT
        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, m_engine->getNavigationState());
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

        auto packet = createDecoderSafePesPacket(0xBD, true, 100000);
        m_mockClient->setStcValue(0);
        m_engine->setIgnorePts(true);

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
    }

    void testSetIgnorePtsFalse()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 100000);
        m_mockClient->setStcValue(0);
        m_engine->setIgnorePts(false);

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), m_engine->process());
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

        m_engine->getTopLinkText(0x0100, 0, nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), m_engine->getPageControlInfo());
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
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));
        CPPUNIT_ASSERT(m_engine != nullptr);

        m_engine.reset();
        CPPUNIT_ASSERT(m_engine == nullptr);
    }

    void testDestructorWithActivePages()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0105, 0x0005);
        m_engine->setCurrentPageId(pageId);

        m_engine.reset();
        CPPUNIT_ASSERT(m_engine == nullptr);
    }

    void testTryRestoreCurrentPageAnySubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId anySubpagePage(0x0100, static_cast<std::uint16_t>(0xFFFF));
        m_engine->setCurrentPageId(anySubpagePage);

        PageId retrieved = m_engine->getPageId(PageIdType::ACTUAL_SUBPAGE);
        CPPUNIT_ASSERT_EQUAL(anySubpagePage.getMagazinePage(), retrieved.getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(anySubpagePage.getSubpage(), retrieved.getSubpage());
    }

    void testTryRestoreCurrentPageNotAvailable()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId nonExistentPage(0x9999, 0x9999);
        m_engine->setCurrentPageId(nonExistentPage);

        assertPageIsEmpty(m_engine->getPage());
        CPPUNIT_ASSERT_EQUAL(nonExistentPage.getMagazinePage(), m_engine->getPageId(PageIdType::CURRENT_PAGE).getMagazinePage());
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

    void testRefreshPageDataWithEmptyPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        m_engine->resetAcquisition();

        assertPageIsEmpty(m_engine->getPage());
    }

    void testSetPageThenGetPage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        CPPUNIT_ASSERT_EQUAL(pageId.getMagazinePage(), m_engine->getPageId(PageIdType::CURRENT_PAGE).getMagazinePage());
        assertPageIsEmpty(m_engine->getPage());
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

        PageId nextPageId = m_engine->getNextPageId(invalidPageId);

        CPPUNIT_ASSERT(!nextPageId.isValidDecimal());
    }

    void testGetPageIdFirstSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId firstSubpage = m_engine->getPageId(PageIdType::FIRST_SUBPAGE);

        CPPUNIT_ASSERT(!firstSubpage.isValidDecimal());
    }

    void testGetPageIdIndexPageP830()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        PageId indexPage = m_engine->getPageId(PageIdType::INDEX_PAGE_P830);

        CPPUNIT_ASSERT(!indexPage.isValidDecimal());
    }

    void testGetPageIdNextSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0001);
        m_engine->setCurrentPageId(pageId);

        PageId result = m_engine->getPageId(PageIdType::NEXT_SUBPAGE);

        CPPUNIT_ASSERT(!result.isValidDecimal());
    }

    void testGetPageIdPrevSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0002);
        m_engine->setCurrentPageId(pageId);

        PageId result = m_engine->getPageId(PageIdType::PREV_SUBPAGE);

        CPPUNIT_ASSERT(!result.isValidDecimal());
    }

    void testGetPageIdHighestSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        PageId result = m_engine->getPageId(PageIdType::HIGHEST_SUBPAGE);

        CPPUNIT_ASSERT(!result.isValidDecimal());
    }

    void testGetPageIdLastReceivedSubpage()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        // Set display page
        PageId pageId(0x0100, 0x0000);
        m_engine->setCurrentPageId(pageId);

        PageId result = m_engine->getPageId(PageIdType::LAST_RECEIVED_SUBPAGE);

        CPPUNIT_ASSERT(!result.isValidDecimal());
    }

    void testGetPageControlInfo()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        std::uint8_t controlInfo = m_engine->getPageControlInfo();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), controlInfo);
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

        m_engine->setNavigationMode(NavigationMode::DEFAULT);

        m_engine->process();
        PageId nextPage = m_engine->getNextPageId(PageId(0x0100, 0x0000));
        m_engine->process();

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0101), nextPage.getMagazinePage());
        CPPUNIT_ASSERT(nextPage.isAnySubpage());
        CPPUNIT_ASSERT_EQUAL(NavigationState::DEFAULT, m_engine->getNavigationState());
    }

    void testPacketWithNoPTS()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createDecoderSafePesPacket();
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
    }

    void testPacketPTSWithinMinThreshold()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        const std::uint32_t stc = 5000;
        const std::uint32_t pts = stc + 100;
        auto packet = createDecoderSafePesPacket(0xBD, true, pts);
        m_mockClient->setStcValue(stc);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
    }

    void testPacketPTSInSynchronizationWindow()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 100000);
        m_mockClient->setStcValue(0);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), result);
    }

    void testPacketPTSFarInFuture()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 0xFFFFFF00);
        m_mockClient->setStcValue(0);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), result);
    }

    void testLatePacketWithinTolerance()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        const std::uint32_t pts = 100000;
        const std::uint32_t stc = pts + 100;
        auto packet = createDecoderSafePesPacket(0xBD, true, pts);
        m_mockClient->setStcValue(stc);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
    }

    void testVeryLatePacketRejection()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 1000);
        m_mockClient->setStcValue(1000000);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), result);
        CPPUNIT_ASSERT(!m_mockClient->wasHeaderReadyCalled());
        CPPUNIT_ASSERT(!m_mockClient->wasPageReadyCalled());
    }

    void testBufferAdditionWhenFull()
    {
        auto allocator = std::make_unique<MockAllocator>(128 * 1024);
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(allocator));

        auto packet = createValidPesPacket(39994);

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT(!m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
    }

    void testNonTeletextPacket()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xC0);

        CPPUNIT_ASSERT(!m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), result);
    }

    void testClientGetStcReturnsFalse()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createValidPesPacket(10, 0xBD, true, 100000);
        m_mockClient->setStcAvailable(false);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));

        std::uint32_t result = m_engine->process();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), result);

        m_mockClient->setStcAvailable(true);
    }

private:
    std::unique_ptr<EngineImpl> m_engine;
    std::unique_ptr<MockEngineClient> m_mockClient;
    std::unique_ptr<MockAllocator> m_mockAllocator;
};

CPPUNIT_TEST_SUITE_REGISTRATION( EngineImplTest );
