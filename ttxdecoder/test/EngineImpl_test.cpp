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
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "EngineImpl.hpp"
#include "EngineClient.hpp"
#include "Allocator.hpp"
#include "ControlInfo.hpp"

using namespace ttxdecoder;

/**
 * Mock EngineClient for testing EngineImpl
 */
class MockEngineClient : public EngineClient
{
public:
    MockEngineClient()
        : m_pageReadyCount(0)
        , m_headerReadyCount(0)
        , m_getStcCallCount(0)
        , m_stcValue(0)
        , m_stcAvailable(true)
    {
    }

    virtual ~MockEngineClient() = default;

    virtual void pageReady() override
    {
        ++m_pageReadyCount;
    }

    virtual void headerReady() override
    {
        ++m_headerReadyCount;
    }

    virtual void drcsCharDecoded(unsigned char index,
                                 unsigned char* data) override
    {
    }

    virtual bool getStc(std::uint32_t& stc) override
    {
        ++m_getStcCallCount;
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
        m_pageReadyCount = 0;
        m_headerReadyCount = 0;
        m_getStcCallCount = 0;
    }

    bool wasPageReadyCalled() const
    {
        return m_pageReadyCount > 0;
    }

    bool wasHeaderReadyCalled() const
    {
        return m_headerReadyCount > 0;
    }

    int getPageReadyCount() const
    {
        return m_pageReadyCount;
    }

    int getHeaderReadyCount() const
    {
        return m_headerReadyCount;
    }

    int getStcCallCount() const
    {
        return m_getStcCallCount;
    }

private:
    int m_pageReadyCount;
    int m_headerReadyCount;
    int m_getStcCallCount;
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
        , m_buffer(new std::uint8_t[totalSize])
        , m_usedSize(0)
    {
    }

    virtual ~MockAllocator() = default;

    virtual std::uint8_t* alloc(std::size_t size) override
    {
        if (size == 0 || size > getFreeSize())
        {
            throw std::bad_alloc();
        }

        std::uint8_t* ptr = m_buffer.get() + m_usedSize;
        m_usedSize += size;
        return ptr;
    }

    virtual std::size_t getFreeSize() override
    {
        return m_totalSize - m_usedSize;
    }

private:
    std::size_t m_totalSize;
    std::unique_ptr<std::uint8_t[]> m_buffer;
    std::size_t m_usedSize;
};

class EngineImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( EngineImplTest );
    CPPUNIT_TEST(testConstructorInitializesWithValidAllocator);
    CPPUNIT_TEST(testConstructorCallsResetAcquisition);
    CPPUNIT_TEST(testProcessEmptyBuffer);
    CPPUNIT_TEST(testProcessSingleValidPacket);
    CPPUNIT_TEST(testHeaderPacketNotifiesClient);
    CPPUNIT_TEST(testPtsHeaderWaitsThenProcesses);
    CPPUNIT_TEST(testSubtitleHeaderSkipsNotification);
    CPPUNIT_TEST(testProcessWaitActionHaltsProcessing);
    CPPUNIT_TEST(testProcessEmptyBufferMultipleTimes);
    CPPUNIT_TEST(testSetCurrentPageIdWithValidPageId);
    CPPUNIT_TEST(testSetCurrentPageIdSamePageNoChange);
    CPPUNIT_TEST(testSetCurrentPageIdWithEmptyPageId);
    CPPUNIT_TEST(testGetPageIdActualSubpageWithCurrentPage);
    CPPUNIT_TEST(testGetPageIdActualSubpageWithoutCurrentPage);
    CPPUNIT_TEST(testGetPageIdLastPageWithCurrentPage);
    CPPUNIT_TEST(testGetPageIdLastPageWithoutPages);
    CPPUNIT_TEST(testGetPageIdColourKeyLinks);
    CPPUNIT_TEST(testGetPageIdUnsupportedTypesReturnEmpty);
    CPPUNIT_TEST(testGetPageReturnsDecodedPage);
    CPPUNIT_TEST(testSetNavigationModeDefault);
    CPPUNIT_TEST(testSetIgnorePtsTrue);
    CPPUNIT_TEST(testSetIgnorePtsFalse);
    CPPUNIT_TEST(testSetCharsetMappingValidInput);
    CPPUNIT_TEST(testSetCharsetMappingMultipleCharsetsIndependent);
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
    CPPUNIT_TEST(testGetPageControlInfo);
    CPPUNIT_TEST(testGetNavigationStateWithNoLinks);
    CPPUNIT_TEST(testNavigationModePersistence);
    CPPUNIT_TEST(testPacketWithNoPTS);
    CPPUNIT_TEST(testPacketPTSWithinMinThreshold);
    CPPUNIT_TEST(testPacketPTSInSynchronizationWindow);
    CPPUNIT_TEST(testPacketPTSFarInFuture);
    CPPUNIT_TEST(testLatePacketWithinTolerance);
    CPPUNIT_TEST(testVeryLatePacketRejection);
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

    std::uint8_t getHammingByte(std::uint8_t nibble)
    {
        static const std::uint8_t hammingEncode[16] = {
            0x28, 0x00, 0x12, 0x3A, 0x06, 0x4E, 0x0C, 0x74,
            0x03, 0x63, 0x11, 0x59, 0x05, 0x2D, 0x3F, 0x17
        };
        return hammingEncode[nibble & 0x0F];
    }

    std::uint8_t getHammingByteForMagazine(std::uint8_t magazine)
    {
        return getHammingByte(magazine & 0x07);
    }

    std::vector<std::uint8_t> createHeaderDataUnit(std::uint8_t magazine,
                                                   std::uint16_t page,
                                                   std::uint16_t subpage,
                                                   std::uint8_t controlInfo)
    {
        std::vector<std::uint8_t> data;
        data.reserve(47);

        data.push_back(0x10);
        data.push_back(0x02);
        data.push_back(0x2C);
        data.push_back(0xFF);
        data.push_back(0xE4);
        data.push_back(getHammingByteForMagazine(magazine));
        data.push_back(0x28);

        const std::uint8_t pageOnly = page & 0xFF;
        const std::uint8_t pageUnits = pageOnly & 0x0F;
        const std::uint8_t pageTens = (pageOnly >> 4) & 0x0F;
        data.push_back(getHammingByte(pageUnits));
        data.push_back(getHammingByte(pageTens));

        const std::uint8_t byte2 = (subpage >> 0) & 0x0F;
        const std::uint8_t byte3 = ((subpage >> 4) & 0x07)
                | ((controlInfo & ControlInfo::ERASE_PAGE) ? 0x08 : 0x00);
        const std::uint8_t byte4 = (subpage >> 8) & 0x0F;
        const std::uint8_t byte5 = ((subpage >> 12) & 0x03)
                | ((controlInfo & ControlInfo::NEWSFLASH) ? 0x04 : 0x00)
                | ((controlInfo & ControlInfo::SUBTITLE) ? 0x08 : 0x00);
        const std::uint8_t byte6 = ((controlInfo & ControlInfo::SUPRESS_HEADER) ? 0x01 : 0x00)
                | ((controlInfo & ControlInfo::UPDATE_INDICATOR) ? 0x02 : 0x00)
                | ((controlInfo & ControlInfo::INTERRUPTED_SEQUENCE) ? 0x04 : 0x00)
                | ((controlInfo & ControlInfo::INHIBIT_DISPLAY) ? 0x08 : 0x00);
        const std::uint8_t byte7 = (controlInfo & ControlInfo::MAGAZINE_SERIAL) ? 0x01 : 0x00;

        data.push_back(getHammingByte(byte2));
        data.push_back(getHammingByte(byte3));
        data.push_back(getHammingByte(byte4));
        data.push_back(getHammingByte(byte5));
        data.push_back(getHammingByte(byte6));
        data.push_back(getHammingByte(byte7));

        for (int index = 0; index < 32; ++index)
        {
            data.push_back(0x20);
        }

        return data;
    }

    std::vector<std::uint8_t> createHeaderPesPacket(std::uint8_t magazine,
                                                    std::uint16_t page,
                                                    std::uint16_t subpage,
                                                    std::uint8_t controlInfo,
                                                    bool withPts = false,
                                                    std::uint32_t ptsValue = 0)
    {
        std::vector<std::uint8_t> payload = createHeaderDataUnit(magazine,
                                                                 page,
                                                                 subpage,
                                                                 controlInfo);
        const std::uint16_t payloadSize = static_cast<std::uint16_t>(payload.size());
        std::vector<std::uint8_t> packet = createValidPesPacket(
                withPts ? payloadSize : static_cast<std::uint16_t>(payloadSize + 3),
                0xBD,
                withPts,
                ptsValue);

        std::size_t payloadOffset = 14U;
        if (!withPts)
        {
            packet[6] = 0x80;
            packet[7] = 0x00;
            packet[8] = 0x00;
            payloadOffset = 9U;
        }

        std::copy(payload.begin(), payload.end(), packet.begin() + payloadOffset);

        return packet;
    }

    void assertCharsetMappingEquals(const CharsetMappingArray& expected,
                                    const CharsetMappingArray& actual)
    {
        CPPUNIT_ASSERT(std::equal(expected.begin(), expected.end(), actual.begin()));
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

    void testHeaderPacketNotifiesClient()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createHeaderPesPacket(1, 0x0100, 0x0000, 0);

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
        CPPUNIT_ASSERT_EQUAL(1, m_mockClient->getHeaderReadyCount());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getPageReadyCount());
    }

    void testPtsHeaderWaitsThenProcesses()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        const std::uint32_t pts = 100000;
        auto packet = createHeaderPesPacket(1, 0x0100, 0x0000, 0, true, pts);

        m_mockClient->setStcValue(0);
        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), m_engine->process());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getHeaderReadyCount());

        m_mockClient->setStcValue(pts);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
        CPPUNIT_ASSERT_EQUAL(1, m_mockClient->getHeaderReadyCount());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getPageReadyCount());
        CPPUNIT_ASSERT(m_mockClient->getStcCallCount() >= 2);
    }

    void testSubtitleHeaderSkipsNotification()
    {
        m_engine = std::make_unique<EngineImpl>(*m_mockClient, std::move(m_mockAllocator));

        auto packet = createHeaderPesPacket(1, 0x0100, 0x0000, ControlInfo::SUBTITLE);

        CPPUNIT_ASSERT(m_engine->addPesPacket(packet.data(), static_cast<std::uint16_t>(packet.size())));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(1), m_engine->process());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getHeaderReadyCount());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getPageReadyCount());
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
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getPageReadyCount());
        CPPUNIT_ASSERT_EQUAL(0, m_mockClient->getHeaderReadyCount());
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
        assertCharsetMappingEquals(testMapping, retrieved);
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

        assertCharsetMappingEquals(mapping1, retrieved1);
        assertCharsetMappingEquals(mapping2, retrieved2);
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
