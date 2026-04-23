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

#include "TopNavProcessor.hpp"
#include "Database.hpp"
#include "PageBtt.hpp"
#include "PageId.hpp"
#include "Page.hpp"

using namespace ttxdecoder;

namespace
{

// Mock Page for testing non-BTT page types
class MockNonBttPage : public Page
{
public:
    MockNonBttPage() = default;

    virtual PageType getType() const override
    {
        return PageType::DISPLAYABLE;
    }

    virtual bool isValid() const override
    {
        return true;
    }

    virtual void invalidate() override
    {
    }

protected:
    virtual PagePacket* getPagePacket(std::uint8_t packetAddress,
                                      std::int8_t designationCode) override
    {
        return nullptr;
    }

    virtual const PagePacket* getPagePacket(std::uint8_t packetAddress,
                                            std::int8_t designationCode) const override
    {
        return nullptr;
    }
};

// Test-friendly PageBtt that exposes protected methods for testing
class TestPageBtt : public PageBtt
{
public:
    TestPageBtt() = default;

    // Expose protected method for testing
    PagePacket* getPagePacketForTest(std::uint8_t packetAddress, std::int8_t designationCode)
    {
        return getPagePacket(packetAddress, designationCode);
    }

    // Helper to get typed packet
    PacketBttPageType* getBttPacket(std::uint8_t row)
    {
        if (row >= 1 && row <= 20)
        {
            auto* pagePacket = getPagePacketForTest(row, -1);
            if (pagePacket && pagePacket->isValid())
            {
                // Get the typed packet from the TypedPagePacket
                const PacketBttPageType* constPacket = getPageTypePacket(row);
                // We need mutable access, so const_cast is necessary for test setup
                return const_cast<PacketBttPageType*>(constPacket);
            }
        }
        return nullptr;
    }
};

// Helper class to build test PageBtt data
class PageBttBuilder
{
public:
    PageBttBuilder()
    {
        // Initialize all page types to 0 (BTT_NO_PAGE)
        for (int i = 0; i < 800; ++i)
        {
            m_pageTypes[i] = 0;
        }
    }

    // Set a specific page type
    void setPageType(int pageIndex, std::int8_t pageType)
    {
        if (pageIndex >= 0 && pageIndex < 800)
        {
            m_pageTypes[pageIndex] = pageType;
        }
    }

    // Set all page types
    void setAllPageTypes(std::int8_t pageType)
    {
        for (int i = 0; i < 800; ++i)
        {
            m_pageTypes[i] = pageType;
        }
    }

    // Set a range of page types
    void setPageTypeRange(int startIndex, int endIndex, std::int8_t pageType)
    {
        for (int i = startIndex; i <= endIndex && i < 800; ++i)
        {
            m_pageTypes[i] = pageType;
        }
    }

    // Build and populate a TestPageBtt object
    void build(TestPageBtt& page)
    {
        page.invalidate();

        // Get page buffer and populate header
        auto* header = page.getPagePacketForTest(0, -1);
        if (header)
        {
            header->setValid(true);
        }

        // Populate all 20 rows with page type data
        for (std::uint8_t row = 1; row <= 20; ++row)
        {
            auto* packet = page.getPagePacketForTest(row, -1);
            if (packet)
            {
                packet->setValid(true);
                auto* bttPacket = page.getBttPacket(row);
                if (bttPacket)
                {
                    std::int8_t* buffer = bttPacket->getBuffer();
                    int baseIndex = (row - 1) * 40;

                    // Fill 40 bytes (4 tens * 10 units)
                    for (int i = 0; i < 40; ++i)
                    {
                        buffer[i] = m_pageTypes[baseIndex + i];
                    }
                }
            }
        }
    }

    // Build a page with invalid data (negative value)
    void buildWithInvalidData(TestPageBtt& page, int invalidAtIndex)
    {
        build(page);

        // Set one value to -1 to trigger failure
        if (invalidAtIndex >= 0 && invalidAtIndex < 800)
        {
            int row = (invalidAtIndex / 40) + 1;
            int offset = invalidAtIndex % 40;

            auto* bttPacket = page.getBttPacket(row);
            if (bttPacket)
            {
                std::int8_t* buffer = bttPacket->getBuffer();
                buffer[offset] = -1;
            }
        }
    }

    // Build a page with missing packet
    void buildWithMissingPacket(TestPageBtt& page, int missingRow)
    {
        build(page);

        // Invalidate specific row
        if (missingRow >= 1 && missingRow <= 20)
        {
            auto* packet = page.getPagePacketForTest(missingRow, -1);
            if (packet)
            {
                packet->setValid(false);
            }
        }
    }

private:
    std::int8_t m_pageTypes[800];
};

} // anonymous namespace

class TopNavProcessorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(TopNavProcessorTest);
    CPPUNIT_TEST(testConstructorInitialized);
    CPPUNIT_TEST(testResetCallSucceeds);
    CPPUNIT_TEST(testMultipleResetCalls);
    CPPUNIT_TEST(testGetPageBufferWithBttPageId);
    CPPUNIT_TEST(testGetPageBufferWithNonBttPageId);
    CPPUNIT_TEST(testGetPageBufferMultipleTimes);
    CPPUNIT_TEST(testGetPageBufferBoundaryMinPageId);
    CPPUNIT_TEST(testGetPageBufferBoundaryMaxPageId);
    CPPUNIT_TEST(testGetPageBufferAfterReset);
    CPPUNIT_TEST(testProcessPageWithNonBttType);
    CPPUNIT_TEST(testProcessPageWithValidBttData);
    CPPUNIT_TEST(testProcessPageWithInvalidBttData);
    CPPUNIT_TEST(testProcessPageWithMissingPacket);
    CPPUNIT_TEST(testNavigationWithAllNavigablePages);
    CPPUNIT_TEST(testNavigationWithNoNavigablePages);
    CPPUNIT_TEST(testNavigationWithSingleNavigablePage);
    CPPUNIT_TEST(testPrevPageNavigation);
    CPPUNIT_TEST(testNextPageNavigation);
    CPPUNIT_TEST(testNavigationWraparound);
    CPPUNIT_TEST(testGroupNavigationWithGroupPages);
    CPPUNIT_TEST(testGroupNavigationWithNoGroupPages);
    CPPUNIT_TEST(testGroupNavigationWithSingleGroupPage);
    CPPUNIT_TEST(testGroupNavigationMixedTypes);
    CPPUNIT_TEST(testBlockNavigationWithBlockPages);
    CPPUNIT_TEST(testBlockNavigationWithNoBlockPages);
    CPPUNIT_TEST(testBlockNavigationWithSingleBlockPage);
    CPPUNIT_TEST(testBlockNavigationMixedTypes);
    CPPUNIT_TEST(testFirstPageBttNoPageConversion);
    CPPUNIT_TEST(testAllPageTypesHandled);
    CPPUNIT_TEST(testSubtitlePageNavigation);
    CPPUNIT_TEST(testMixedPageTypeNavigation);
    CPPUNIT_TEST(testInvalidDataResetsDatabase);
    CPPUNIT_TEST(testMissingPacketResetsDatabase);
    CPPUNIT_TEST(testCompleteWorkflow);
    CPPUNIT_TEST(testReprocessAfterReset);
    CPPUNIT_TEST(testDatabaseMetadataCorrectness);
    CPPUNIT_TEST(testNavigationChainIntegrity);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_database = std::make_unique<Database>();
        m_processor = std::make_unique<TopNavProcessor>(*m_database);
    }

    void tearDown() override
    {
        m_processor.reset();
        m_database.reset();
    }

protected:
    void testConstructorInitialized()
    {
        Database testDb;
        TopNavProcessor processor(testDb);

        // Verify processor can be used immediately
        PageId pageId(0x01F0, 0x0000);
        Page* buffer = processor.getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer != nullptr);
    }

    void testResetCallSucceeds()
    {
        // Should not crash or throw
        m_processor->reset();

        // Processor should still be functional
        PageId pageId(0x01F0, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer != nullptr);
    }

    void testMultipleResetCalls()
    {
        m_processor->reset();
        m_processor->reset();
        m_processor->reset();

        // Should still work after multiple resets
        PageId pageId(0x01F0, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer != nullptr);
    }

    void testGetPageBufferWithBttPageId()
    {
        PageId pageId(0x01F0, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);

        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PageType::BTT, buffer->getType());
    }

    void testGetPageBufferWithNonBttPageId()
    {
        PageId pageId(0x0100, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);

        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testGetPageBufferMultipleTimes()
    {
        PageId pageId(0x01F0, 0x0000);

        Page* buffer1 = m_processor->getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer1 != nullptr);

        Page* buffer2 = m_processor->getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer2 != nullptr);

        // Should return same buffer instance
        CPPUNIT_ASSERT_EQUAL(buffer1, buffer2);
    }

    void testGetPageBufferBoundaryMinPageId()
    {
        PageId pageId(0x0000, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);

        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testGetPageBufferBoundaryMaxPageId()
    {
        PageId pageId(0xFFFF, 0xFFFF);
        Page* buffer = m_processor->getPageBuffer(pageId);

        CPPUNIT_ASSERT(buffer == nullptr);
    }

    void testGetPageBufferAfterReset()
    {
        m_processor->reset();

        PageId pageId(0x01F0, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);

        CPPUNIT_ASSERT(buffer != nullptr);
        CPPUNIT_ASSERT_EQUAL(PageType::BTT, buffer->getType());
    }

    void testProcessPageWithNonBttType()
    {
        MockNonBttPage nonBttPage;

        // Should not crash, just ignore the page
        m_processor->processPage(nonBttPage);

        // Database should remain in default state
        const Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadata.m_nextPage);
    }

    void testProcessPageWithValidBttData()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Set all pages as normal navigable pages
        builder.setAllPageTypes(8); // BTT_NORMAL_S
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Verify database was updated
        const Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT(metadata.m_nextPage != 0xFFFF);
        CPPUNIT_ASSERT(metadata.m_prevPage != 0xFFFF);
    }

    void testProcessPageWithInvalidBttData()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8);
        builder.buildWithInvalidData(bttPage, 100); // Set negative value at index 100

        m_processor->processPage(bttPage);

        // Database should be reset due to invalid data
        const Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadata.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadata.m_prevPage);
    }

    void testProcessPageWithMissingPacket()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8);
        builder.buildWithMissingPacket(bttPage, 10); // Invalidate row 10

        m_processor->processPage(bttPage);

        // Database should be reset due to missing packet
        const Database::TopMetadata& metadata = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadata.m_nextPage);
    }

    void testNavigationWithAllNavigablePages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8); // BTT_NORMAL_S - all navigable
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Check first page navigation
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), metadata100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x899), metadata100.m_prevPage);

        // Check middle page navigation
        const Database::TopMetadata& metadata500 = m_database->getTopMetatadata(0x500);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x501), metadata500.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x499), metadata500.m_prevPage);

        // Check last page navigation
        const Database::TopMetadata& metadata899 = m_database->getTopMetatadata(0x899);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata899.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x898), metadata899.m_prevPage);
    }

    void testNavigationWithNoNavigablePages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0); // BTT_NO_PAGE - not navigable
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // All pages should point to page 0 (index 0 = page 100)
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_prevPage);

        const Database::TopMetadata& metadata500 = m_database->getTopMetatadata(0x500);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata500.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata500.m_prevPage);
    }

    void testNavigationWithSingleNavigablePage()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0); // All non-navigable (BTT_NO_PAGE)
        // Note: first page (index 0) will be converted from BTT_NO_PAGE to BTT_BLOCK_M
        // So it becomes the single navigable page
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // All pages should point to page 100 (index 0) which was converted to BTT_BLOCK_M
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_prevPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata150.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata150.m_prevPage);
    }

    void testPrevPageNavigation()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Set specific pages as navigable
        builder.setAllPageTypes(0);
        builder.setPageType(0, 8);   // page 100
        builder.setPageType(50, 8);  // page 150
        builder.setPageType(100, 8); // page 200
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata100.m_prevPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata150.m_prevPage);

        const Database::TopMetadata& metadata200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata200.m_prevPage);
    }

    void testNextPageNavigation()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Set specific pages as navigable
        builder.setAllPageTypes(0);
        builder.setPageType(0, 8);   // page 100
        builder.setPageType(50, 8);  // page 150
        builder.setPageType(100, 8); // page 200
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata100.m_nextPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata150.m_nextPage);

        const Database::TopMetadata& metadata200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata200.m_nextPage);
    }

    void testNavigationWraparound()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8); // All navigable
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Last page should wrap to first page
        const Database::TopMetadata& metadata899 = m_database->getTopMetatadata(0x899);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata899.m_nextPage);

        // First page should wrap to last page
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x899), metadata100.m_prevPage);
    }

    void testGroupNavigationWithGroupPages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0);
        builder.setPageType(0, 6);   // BTT_GROUP_S at page 100
        builder.setPageType(50, 7);  // BTT_GROUP_M at page 150
        builder.setPageType(100, 6); // BTT_GROUP_S at page 200
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata100.m_nextGroupPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata150.m_nextGroupPage);

        const Database::TopMetadata& metadata200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata200.m_nextGroupPage);
    }

    void testGroupNavigationWithNoGroupPages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8); // BTT_NORMAL_S - not group pages
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Should point to first page (no groups found)
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextGroupPage);
    }

    void testGroupNavigationWithSingleGroupPage()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0);
        builder.setPageType(50, 6); // BTT_GROUP_S at page 150
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // All pages should point to the single group page
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata100.m_nextGroupPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata150.m_nextGroupPage);
    }

    void testGroupNavigationMixedTypes()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8);  // Normal pages
        builder.setPageType(10, 6);  // Group at page 110
        builder.setPageType(20, 7);  // Group at page 120
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Non-group pages should point to next group
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x110), metadata100.m_nextGroupPage);

        // Group pages should point to next group
        const Database::TopMetadata& metadata110 = m_database->getTopMetatadata(0x110);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x120), metadata110.m_nextGroupPage);
    }

    void testBlockNavigationWithBlockPages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0);
        builder.setPageType(0, 4);   // BTT_BLOCK_S at page 100
        builder.setPageType(50, 5);  // BTT_BLOCK_M at page 150
        builder.setPageType(100, 4); // BTT_BLOCK_S at page 200
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x150), metadata100.m_nextBlockPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), metadata150.m_nextBlockPage);

        const Database::TopMetadata& metadata200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata200.m_nextBlockPage);
    }

    void testBlockNavigationWithNoBlockPages()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8); // BTT_NORMAL_S - not block pages
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Should point to first page (no blocks found)
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextBlockPage);
    }

    void testBlockNavigationWithSingleBlockPage()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0); // All BTT_NO_PAGE
        // Note: first page (index 0) will be converted from BTT_NO_PAGE to BTT_BLOCK_M
        // So it becomes the single block page
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // All pages should point to page 100 (index 0) which was converted to BTT_BLOCK_M
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextBlockPage);

        const Database::TopMetadata& metadata150 = m_database->getTopMetatadata(0x150);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata150.m_nextBlockPage);
    }

    void testBlockNavigationMixedTypes()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(8);  // Normal pages
        builder.setPageType(10, 4);  // Block at page 110
        builder.setPageType(20, 5);  // Block at page 120
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Non-block pages should point to next block
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x110), metadata100.m_nextBlockPage);

        // Block pages should point to next block
        const Database::TopMetadata& metadata110 = m_database->getTopMetatadata(0x110);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x120), metadata110.m_nextBlockPage);
    }

    void testFirstPageBttNoPageConversion()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(0); // All BTT_NO_PAGE
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // First page (100) should be converted to BTT_BLOCK_M and become navigable
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        // Since first page is now a block type, it should have block navigation
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), metadata100.m_nextBlockPage);
    }

    void testAllPageTypesHandled()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Test with all valid page type values (0-15)
        for (std::int8_t type = 0; type <= 15; ++type)
        {
            builder.setAllPageTypes(type);
            builder.build(bttPage);

            // Should process without crashing
            m_processor->processPage(bttPage);
        }

        // If we get here, all types were handled
        CPPUNIT_ASSERT(true);
    }

    void testSubtitlePageNavigation()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        builder.setAllPageTypes(1); // BTT_SUBTITLE
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Subtitle pages are navigable (based on isNavigableType implementation)
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT(metadata100.m_nextPage != 0xFFFF);
    }

    void testMixedPageTypeNavigation()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Mix of different types
        builder.setPageTypeRange(0, 99, 8);    // Normal pages
        builder.setPageTypeRange(100, 199, 0); // No pages
        builder.setPageTypeRange(200, 299, 4); // Block pages
        builder.setPageTypeRange(300, 399, 6); // Group pages
        builder.setPageTypeRange(400, 799, 8); // Normal pages
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Verify navigation skips non-navigable pages
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), metadata100.m_nextPage);

        // Page 200 (first in non-navigable range) should skip to navigable page
        const Database::TopMetadata& metadata200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT(metadata200.m_nextPage != 0x201); // Should skip non-navigable
    }

    void testInvalidDataResetsDatabase()
    {
        // First populate with valid data
        TestPageBtt validPage;
        PageBttBuilder validBuilder;
        validBuilder.setAllPageTypes(8);
        validBuilder.build(validPage);
        m_processor->processPage(validPage);

        // Verify database was populated
        const Database::TopMetadata& metadataBefore = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT(metadataBefore.m_nextPage != 0xFFFF);

        // Now process invalid data
        TestPageBtt invalidPage;
        PageBttBuilder invalidBuilder;
        invalidBuilder.setAllPageTypes(8);
        invalidBuilder.buildWithInvalidData(invalidPage, 50);
        m_processor->processPage(invalidPage);

        // Database should be reset
        const Database::TopMetadata& metadataAfter = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadataAfter.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadataAfter.m_prevPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadataAfter.m_nextGroupPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadataAfter.m_nextBlockPage);
    }

    void testMissingPacketResetsDatabase()
    {
        // First populate with valid data
        TestPageBtt validPage;
        PageBttBuilder validBuilder;
        validBuilder.setAllPageTypes(8);
        validBuilder.build(validPage);
        m_processor->processPage(validPage);

        // Verify database was populated
        const Database::TopMetadata& metadataBefore = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT(metadataBefore.m_nextPage != 0xFFFF);

        // Now process page with missing packet
        TestPageBtt invalidPage;
        PageBttBuilder invalidBuilder;
        invalidBuilder.setAllPageTypes(8);
        invalidBuilder.buildWithMissingPacket(invalidPage, 5);
        m_processor->processPage(invalidPage);

        // Database should be reset
        const Database::TopMetadata& metadataAfter = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), metadataAfter.m_nextPage);
    }

    void testCompleteWorkflow()
    {
        // Get buffer
        PageId pageId(0x01F0, 0x0000);
        Page* buffer = m_processor->getPageBuffer(pageId);
        CPPUNIT_ASSERT(buffer != nullptr);

        // Populate buffer
        TestPageBtt* bttPage = static_cast<TestPageBtt*>(buffer);
        PageBttBuilder builder;
        builder.setPageTypeRange(0, 199, 8);    // Normal pages 100-299
        builder.setPageTypeRange(200, 399, 4);  // Block pages 300-499
        builder.setPageTypeRange(400, 599, 6);  // Group pages 500-699
        builder.setPageTypeRange(600, 799, 10); // Normal multi pages 700-899
        builder.build(*bttPage);

        // Process page
        m_processor->processPage(*bttPage);

        // Verify all metadata types are populated
        const Database::TopMetadata& metadata100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT(metadata100.m_nextPage != 0xFFFF);
        CPPUNIT_ASSERT(metadata100.m_prevPage != 0xFFFF);
        CPPUNIT_ASSERT(metadata100.m_nextGroupPage != 0xFFFF);
        CPPUNIT_ASSERT(metadata100.m_nextBlockPage != 0xFFFF);

        // Verify specific navigation
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x101), metadata100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), metadata100.m_nextBlockPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x500), metadata100.m_nextGroupPage);
    }

    void testReprocessAfterReset()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;
        builder.setAllPageTypes(8);
        builder.build(bttPage);

        // Process first time
        m_processor->processPage(bttPage);
        const Database::TopMetadata& metadata1 = m_database->getTopMetatadata(0x100);
        uint16_t nextPage1 = metadata1.m_nextPage;

        // Reset and process again
        m_processor->reset();
        m_processor->processPage(bttPage);
        const Database::TopMetadata& metadata2 = m_database->getTopMetatadata(0x100);
        uint16_t nextPage2 = metadata2.m_nextPage;

        // Results should be the same
        CPPUNIT_ASSERT_EQUAL(nextPage1, nextPage2);
    }

    void testDatabaseMetadataCorrectness()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;

        // Create a specific pattern
        builder.setAllPageTypes(0);
        builder.setPageType(0, 8);    // page 100
        builder.setPageType(100, 8);  // page 200
        builder.setPageType(200, 8);  // page 300
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Verify page 100 navigation
        const Database::TopMetadata& meta100 = m_database->getTopMetatadata(0x100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), meta100.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), meta100.m_prevPage);

        // Verify page 200 navigation
        const Database::TopMetadata& meta200 = m_database->getTopMetatadata(0x200);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x300), meta200.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), meta200.m_prevPage);

        // Verify page 300 navigation
        const Database::TopMetadata& meta300 = m_database->getTopMetatadata(0x300);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x100), meta300.m_nextPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x200), meta300.m_prevPage);
    }

    void testNavigationChainIntegrity()
    {
        TestPageBtt bttPage;
        PageBttBuilder builder;
        builder.setAllPageTypes(8); // All navigable
        builder.build(bttPage);

        m_processor->processPage(bttPage);

        // Follow next chain - should visit all pages and return to start
        uint16_t currentPage = 0x100;
        int visitCount = 0;
        const int maxPages = 800;

        do
        {
            const Database::TopMetadata& metadata = m_database->getTopMetatadata(currentPage);
            currentPage = metadata.m_nextPage;
            visitCount++;

            // Prevent infinite loop
            CPPUNIT_ASSERT(visitCount <= maxPages);
        } while (currentPage != 0x100 && visitCount < maxPages);

        // Should have visited all 800 pages
        CPPUNIT_ASSERT_EQUAL(800, visitCount);

        // Follow prev chain
        currentPage = 0x100;
        visitCount = 0;

        do
        {
            const Database::TopMetadata& metadata = m_database->getTopMetatadata(currentPage);
            currentPage = metadata.m_prevPage;
            visitCount++;

            CPPUNIT_ASSERT(visitCount <= maxPages);
        } while (currentPage != 0x100 && visitCount < maxPages);

        // Should have visited all 800 pages in reverse
        CPPUNIT_ASSERT_EQUAL(800, visitCount);
    }

private:
    std::unique_ptr<Database> m_database;
    std::unique_ptr<TopNavProcessor> m_processor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TopNavProcessorTest);
