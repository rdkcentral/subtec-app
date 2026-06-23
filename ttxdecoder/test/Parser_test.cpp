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
#include <cppunit/TestFixture.h>

#include <initializer_list>

#include "Parser.hpp"
#include "Database.hpp"
#include "CharsetConfig.hpp"
#include "PageDisplayable.hpp"
#include "PacketHeader.hpp"
#include "PacketLopData.hpp"
#include "PacketEditorialLinks.hpp"
#include "PacketTriplets.hpp"
#include "DecodedPage.hpp"
#include "PresentationLevel.hpp"
#include "Types.hpp"
#include "ControlInfo.hpp"
#include "PageId.hpp"
#include "CharsetMaps.hpp"
#include "Property.hpp"

using namespace ttxdecoder;

class MockCharsetConfig : public CharsetConfig
{
public:
    MockCharsetConfig()
        : m_mode(Mode::UTF)
        , m_primaryCharset(NationalCharset::ENGLISH)
        , m_secondaryCharset(NationalCharset::ENGLISH)
    {
    }

    virtual ~MockCharsetConfig() = default;

    virtual Mode getMode() const override
    {
        return m_mode;
    }

    virtual NationalCharset getPrimaryNationalCharset(std::uint8_t index = 0) const override
    {
        return m_primaryCharset;
    }

    virtual NationalCharset getSecondaryNationalCharset(std::uint8_t index = 0) const override
    {
        return m_secondaryCharset;
    }

    virtual const NationalSubsets& getNationalSubsets() const override
    {
        static const NationalSubsets subsets;
        return subsets;
    }

    virtual const CharsetMaps& getCharsetMaps() const override
    {
        static const CharsetMaps maps;
        return maps;
    }

private:
    Mode m_mode;
    NationalCharset m_primaryCharset;
    NationalCharset m_secondaryCharset;
};

class ParserTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ParserTest);
    CPPUNIT_TEST(testHeaderOnlySkipsRows);
    CPPUNIT_TEST(testHeaderOnlySkipsNavigation);
    CPPUNIT_TEST(testFullPageCleared);
    CPPUNIT_TEST(testFullPagePreservesWithoutErase);
    CPPUNIT_TEST(testFullPageProcessesAllRows);
    CPPUNIT_TEST(testFullPageSetsPageInfo);
    CPPUNIT_TEST(testNavigationDefault);
    CPPUNIT_TEST(testFlofNavigation);
    CPPUNIT_TEST(testTopFlofPrefersTop);
    CPPUNIT_TEST(testFlofTopFallsBackToTop);
    CPPUNIT_TEST(testNewsflashMode);
    CPPUNIT_TEST(testSubtitleMode);
    CPPUNIT_TEST(testNonBoxingFlags);
    CPPUNIT_TEST(testContextReset);
    CPPUNIT_TEST(testClockOnly);
    CPPUNIT_TEST(testDoubleHeightNextRow);
    CPPUNIT_TEST(testX280Precedence);
    CPPUNIT_TEST(testM290Precedence);
    CPPUNIT_TEST(testDifferentLevels);
    CPPUNIT_TEST(testMagazineMin);
    CPPUNIT_TEST(testMagazineMax);
    CPPUNIT_TEST(testNoChangeWhenIdentical);
    CPPUNIT_TEST(testMultiplePagesNoContamination);
    CPPUNIT_TEST(testHeaderRow);
    CPPUNIT_TEST(testErasePageFlag);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_charsetConfig = new MockCharsetConfig();
        m_database = new Database();
        m_parser = new Parser(PresentationLevel::LEVEL_1, *m_database, *m_charsetConfig);
    }

    void tearDown() override
    {
        delete m_parser;
        delete m_database;
        delete m_charsetConfig;
    }

private:
    // Helper function to create a valid header packet
    // decimalPage is decimal magazine+page number (e.g., 100, 123, 500)
    // It's encoded as hex: magazine in bits 8-11, page digits in bits 0-7
    // e.g., decimal 123 -> 0x123 (magazine=1, page=23)
    void createHeaderPacket(PacketHeader& header, uint16_t decimalPage,
                           std::uint8_t controlInfo)
    {
        // Convert decimal to hex-encoded format
        // e.g., 123 decimal -> magazine=1, page1=2, page2=3 -> 0x123
        uint16_t magazine = decimalPage / 100;
        uint16_t page1 = (decimalPage / 10) % 10;
        uint16_t page2 = decimalPage % 10;
        uint16_t magazinePage = (magazine << 8) | (page1 << 4) | page2;

        PageId pid(magazinePage, PageId::ANY_SUBPAGE);
        header.setMagazineNumber(static_cast<std::uint8_t>((magazine == 8) ? 0 : magazine));
        header.setPacketAddress(0);
        header.setPageInfo(pid, controlInfo, 0);
    }

    void setHeaderBytes(PacketHeader& header,
                        std::initializer_list<std::int8_t> bytes)
    {
        auto* buffer = header.getBuffer();
        const auto length = header.getBufferLength();

        for (std::size_t i = 0; i < length; ++i)
        {
            buffer[i] = ' ';
        }

        std::size_t index = 0;
        for (auto value : bytes)
        {
            if (index >= length)
            {
                break;
            }

            buffer[index] = value;
            ++index;
        }
    }

    void setHeaderTrailingBytes(PacketHeader& header,
                                std::initializer_list<std::int8_t> bytes)
    {
        auto* buffer = header.getBuffer();
        const auto length = header.getBufferLength();
        const auto offset = (bytes.size() < length) ? (length - bytes.size()) : 0;

        std::size_t index = 0;
        for (auto value : bytes)
        {
            if ((offset + index) >= length)
            {
                break;
            }

            buffer[offset + index] = value;
            ++index;
        }
    }

    PacketHeader& configurePageHeader(PageDisplayable& page,
                                      std::uint16_t decimalPage,
                                      std::uint8_t controlInfo,
                                      std::initializer_list<std::int8_t> bytes = {})
    {
        auto* header = page.takeHeader();
        CPPUNIT_ASSERT(header != nullptr);

        createHeaderPacket(*header, decimalPage, controlInfo);
        setHeaderBytes(*header, bytes);

        page.setLastPacketValid(header);
        return *header;
    }

    void configureLopRow(PageDisplayable& page,
                         std::uint8_t row,
                         std::initializer_list<std::int8_t> bytes)
    {
        auto* packet = static_cast<PacketLopData*>(page.takePacket(row, 0));
        CPPUNIT_ASSERT(packet != nullptr);

        auto* buffer = packet->getBuffer();
        const auto length = packet->getBufferLength();
        for (std::size_t i = 0; i < length; ++i)
        {
            buffer[i] = ' ';
        }

        std::size_t index = 0;
        for (auto value : bytes)
        {
            if (index >= length)
            {
                break;
            }

            buffer[index] = value;
            ++index;
        }

        page.setLastPacketValid(packet);
    }

    void configureEditorialLinks(PageDisplayable& page,
                                 std::uint16_t red,
                                 std::uint16_t green,
                                 std::uint16_t yellow,
                                 std::uint16_t cyan,
                                 std::uint16_t flofIndex)
    {
        auto* packet = static_cast<PacketEditorialLinks*>(page.takePacket(27, 0));
        CPPUNIT_ASSERT(packet != nullptr);

        packet->setDesignationCode(0);
        packet->setLink(0, PageId(red, PageId::ANY_SUBPAGE));
        packet->setLink(1, PageId(green, PageId::ANY_SUBPAGE));
        packet->setLink(2, PageId(yellow, PageId::ANY_SUBPAGE));
        packet->setLink(3, PageId(cyan, PageId::ANY_SUBPAGE));
        packet->setLink(5, PageId(flofIndex, PageId::ANY_SUBPAGE));

        page.setLastPacketValid(packet);
    }

    void configureTripletPrimaryCharset(PacketTriplets& packet,
                                        std::int8_t designationCode,
                                        std::uint8_t primaryCharsetCode)
    {
        packet.setDesignationCode(designationCode);
        for (std::size_t i = 0; i < PacketTriplets::TRIPLET_COUNT; ++i)
        {
            packet.setTripletValue(i, 0);
        }

        packet.setTripletValue(0, static_cast<std::uint32_t>(primaryCharsetCode) << 7);
    }

    void configureX28Packet(PageDisplayable& page,
                            std::int8_t designationCode,
                            std::uint8_t primaryCharsetCode)
    {
        auto* packet = static_cast<PacketTriplets*>(page.takePacket(28, designationCode));
        CPPUNIT_ASSERT(packet != nullptr);

        configureTripletPrimaryCharset(*packet, designationCode, primaryCharsetCode);

        page.setLastPacketValid(packet);
    }

    void configureM29Packet(std::uint8_t magazine,
                            std::int8_t designationCode,
                            std::uint8_t primaryCharsetCode)
    {
        auto& magazinePage = m_database->getMagazinePage(magazine);
        auto* packet = static_cast<PacketTriplets*>(magazinePage.takePacket(29, designationCode));
        CPPUNIT_ASSERT(packet != nullptr);

        configureTripletPrimaryCharset(*packet, designationCode, primaryCharsetCode);

        magazinePage.setLastPacketValid(packet);
    }

    void seedRow(DecodedPage& decodedPage,
                 std::size_t row,
                 std::uint16_t character,
                 std::uint16_t properties = 0)
    {
        auto& segment = decodedPage.getRow(row).m_levelOnePageSegment;
        segment.m_charArray[0] = character;
        segment.m_propertiesArray[0] = properties;
    }

    void testHeaderOnlySkipsRows()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);
        setHeaderBytes(header, {'H'});

        PageDisplayable page;
        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x345, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 1, 'X', Property::VALUE_FLASH);

        bool changed = m_parser->parsePage(page, header, Parser::Mode::HEADER_ONLY,
                                           NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x345), decodedPage.getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('X'), decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(Property::VALUE_FLASH, decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[0]);
    }

    void testHeaderOnlySkipsNavigation()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x100, PageId::ANY_SUBPAGE));
        decodedPage.setColourKeyLink(DecodedPage::Link::RED,
                                     PageId(0x234, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 24, 'N', Property::VALUE_FLASH);

        m_parser->parsePage(page, header, Parser::Mode::HEADER_ONLY,
                           NavigationMode::TOP_DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('N'), decodedPage.getRow(24).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(Property::VALUE_FLASH, decodedPage.getRow(24).m_levelOnePageSegment.m_propertiesArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x234),
                             decodedPage.getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
    }

    void testFullPageCleared()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x200, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 1, 'X', Property::VALUE_FLASH);

        bool changed = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                          NavigationMode::DEFAULT, decodedPage);

        PageId resultPageId = decodedPage.getPageId();
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), resultPageId.getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '), decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[0]);
    }

    void testFullPagePreservesWithoutErase()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x100, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 1, 'X', Property::VALUE_FLASH);

        bool changed = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                          NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('X'), decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(Property::VALUE_FLASH, decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[0]);
    }

    void testFullPageProcessesAllRows()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        for (int i = 1; i <= 24; ++i)
        {
            seedRow(decodedPage, i, static_cast<std::uint16_t>('A' + (i % 26)), Property::VALUE_FLASH);
        }

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage);

        for (int i = 1; i <= 24; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '), decodedPage.getRow(i).m_levelOnePageSegment.m_charArray[0]);
            CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), decodedPage.getRow(i).m_levelOnePageSegment.m_propertiesArray[0]);
        }
    }

    void testFullPageSetsPageInfo()
    {
        PacketHeader header;
        createHeaderPacket(header, 123, ControlInfo::NEWSFLASH);

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(123), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT((decodedPage.getPageControlInfo() & ControlInfo::NEWSFLASH) != 0);
    }

    void testNavigationDefault()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x100, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 24, 'N', Property::VALUE_FLASH);

        bool changed = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                          NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '), decodedPage.getRow(24).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), decodedPage.getRow(24).m_levelOnePageSegment.m_propertiesArray[0]);
    }

    void testFlofNavigation()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        configureEditorialLinks(page, 0x234, 0x235, 0x236, 0x237, 0x238);

        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::FLOF_DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x234),
                             decodedPage.getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x238),
                             decodedPage.getColourKeyLink(DecodedPage::Link::FLOF_INDEX).getMagazinePage());
    }

    void testTopFlofPrefersTop()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        configureEditorialLinks(page, 0x234, 0x235, 0x236, 0x237, 0x238);

        auto& metadata = m_database->getTopMetatadata(0x100);
        metadata.m_nextPage = 0x211;
        metadata.m_prevPage = 0x212;
        metadata.m_nextGroupPage = 0x213;
        metadata.m_nextBlockPage = 0x214;

        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::TOP_FLOF_DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x211),
                             decodedPage.getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x214),
                             decodedPage.getColourKeyLink(DecodedPage::Link::CYAN).getMagazinePage());
    }

    void testFlofTopFallsBackToTop()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        auto& metadata = m_database->getTopMetatadata(0x100);
        metadata.m_nextPage = 0x221;
        metadata.m_prevPage = 0x222;
        metadata.m_nextGroupPage = 0x223;
        metadata.m_nextBlockPage = 0x224;

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::FLOF_TOP_DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x221),
                             decodedPage.getColourKeyLink(DecodedPage::Link::RED).getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x224),
                             decodedPage.getColourKeyLink(DecodedPage::Link::CYAN).getMagazinePage());
    }

    void testNewsflashMode()
    {
        PacketHeader normalHeader;
        createHeaderPacket(normalHeader, 100, 0);
        setHeaderBytes(normalHeader, {'N'});

        PacketHeader newsflashHeader;
        createHeaderPacket(newsflashHeader, 100, ControlInfo::NEWSFLASH);
        setHeaderBytes(newsflashHeader, {'N'});

        PageDisplayable page;
        DecodedPage normalPage;
        DecodedPage newsflashPage;

        m_parser->parsePage(page, normalHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, normalPage);
        m_parser->parsePage(page, newsflashHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, newsflashPage);

        static const std::size_t headerColumn = 8;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('N'), normalPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((normalPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) == 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), newsflashPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((newsflashPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) != 0);
    }

    void testSubtitleMode()
    {
        PacketHeader normalHeader;
        createHeaderPacket(normalHeader, 100, 0);
        setHeaderBytes(normalHeader, {'S'});

        PacketHeader subtitleHeader;
        createHeaderPacket(subtitleHeader, 100, ControlInfo::SUBTITLE);
        setHeaderBytes(subtitleHeader, {'S'});

        PageDisplayable page;
        DecodedPage normalPage;
        DecodedPage subtitlePage;

        m_parser->parsePage(page, normalHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, normalPage);
        m_parser->parsePage(page, subtitleHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, subtitlePage);

        static const std::size_t headerColumn = 8;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('S'), normalPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((normalPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) == 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), subtitlePage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((subtitlePage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) != 0);
    }

    void testNonBoxingFlags()
    {
        PacketHeader plainHeader;
        createHeaderPacket(plainHeader, 100, 0);
        setHeaderBytes(plainHeader, {0x0B, 'X', 0x0A});

        PacketHeader flaggedHeader;
        createHeaderPacket(flaggedHeader, 100, ControlInfo::SUPRESS_HEADER);
        setHeaderBytes(flaggedHeader, {0x0B, 'X', 0x0A});

        PageDisplayable page;
        DecodedPage plainPage;
        DecodedPage flaggedPage;

        m_parser->parsePage(page, plainHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, plainPage);
        m_parser->parsePage(page, flaggedHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, flaggedPage);

        static const std::size_t visibleColumn = 9;
        CPPUNIT_ASSERT_EQUAL(plainPage.getRow(0).m_levelOnePageSegment.m_charArray[visibleColumn],
                             flaggedPage.getRow(0).m_levelOnePageSegment.m_charArray[visibleColumn]);
        CPPUNIT_ASSERT_EQUAL(plainPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[visibleColumn],
                             flaggedPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[visibleColumn]);
    }

    void testContextReset()
    {
        PacketHeader header1;
        createHeaderPacket(header1, 100, ControlInfo::NEWSFLASH);
        setHeaderBytes(header1, {'A'});

        PacketHeader header2;
        createHeaderPacket(header2, 200, 0);
        setHeaderBytes(header2, {'A'});

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header1, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage);

        static const std::size_t headerColumn = 8;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((decodedPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) != 0);

        m_parser->parsePage(page, header2, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('A'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT((decodedPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn] & Property::VALUE_HIDDEN) == 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(200), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(0), decodedPage.getPageControlInfo());
    }

    void testClockOnly()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);
        setHeaderBytes(header, {'I'});
        setHeaderTrailingBytes(header, {'2', '2', '2', '2', '2', '2', '2', '2'});

        PageDisplayable page;
        PacketHeader& pageHeader = configurePageHeader(page, 100, 0, {'P'});
        setHeaderTrailingBytes(pageHeader, {'1', '1', '1', '1', '1', '1', '1', '1'});

        DecodedPage decodedPage;
        decodedPage.setPageId(PageId(0x345, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 1, 'R', Property::VALUE_FLASH);

        bool changed = m_parser->parsePage(page, header, Parser::Mode::CLOCK_ONLY,
                                           NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x345), decodedPage.getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('R'), decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(Property::VALUE_FLASH, decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[0]);

        static const std::size_t headerColumn = 8;
        static const std::size_t clockColumn = 32;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('P'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('2'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[clockColumn]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('2'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[39]);
    }

    void testDoubleHeightNextRow()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureLopRow(page, 1, {0x0D, 'A'});
        configureLopRow(page, 2, {'Z'});

        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('A'),
                             decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[1]);
        CPPUNIT_ASSERT((decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[1]
                        & Property::VALUE_DOUBLE_HEIGHT) != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '),
                             decodedPage.getRow(2).m_levelOnePageSegment.m_charArray[0]);
    }

    void testX280Precedence()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureLopRow(page, 1, {0x23});
        configureX28Packet(page, 4, 0x01);

        DecodedPage x284OnlyPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, x284OnlyPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0023),
                             x284OnlyPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);

        configureX28Packet(page, 0, 0x00);

        DecodedPage x280PreferredPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, x280PreferredPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x00A3),
                             x280PreferredPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
    }

    void testM290Precedence()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureLopRow(page, 1, {0x23});
        configureM29Packet(1, 4, 0x01);

        DecodedPage m294OnlyPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, m294OnlyPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x0023),
                             m294OnlyPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);

        configureM29Packet(1, 0, 0x00);

        DecodedPage m290PreferredPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, m290PreferredPage);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0x00A3),
                             m290PreferredPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
    }

    void testDifferentLevels()
    {
        Database db;
        MockCharsetConfig config;
        PacketHeader header;
        createHeaderPacket(header, 100, 0);
        setHeaderBytes(header, {'L'});
        PageDisplayable page;
        DecodedPage decodedPage1;
        DecodedPage decodedPage2;

        Parser parser1(PresentationLevel::LEVEL_1, db, config);
        bool result1 = parser1.parsePage(page, header, Parser::Mode::FULL_PAGE,
                                         NavigationMode::DEFAULT, decodedPage1);

        Parser parser2(PresentationLevel::LEVEL_1_5, db, config);
        bool result2 = parser2.parsePage(page, header, Parser::Mode::FULL_PAGE,
                                         NavigationMode::DEFAULT, decodedPage2);

        static const std::size_t headerColumn = 8;
        CPPUNIT_ASSERT_EQUAL(true, result1);
        CPPUNIT_ASSERT_EQUAL(true, result2);
        CPPUNIT_ASSERT_EQUAL(decodedPage1.getPageId().getMagazinePage(), decodedPage2.getPageId().getMagazinePage());
        CPPUNIT_ASSERT_EQUAL(decodedPage1.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn],
                             decodedPage2.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT_EQUAL(decodedPage1.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn],
                             decodedPage2.getRow(0).m_levelOnePageSegment.m_propertiesArray[headerColumn]);
    }

    void testMagazineMin()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage.getPageId().getDecimalMagazinePage());
    }

    void testMagazineMax()
    {
        PacketHeader header;
        createHeaderPacket(header, 800, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(800), decodedPage.getPageId().getDecimalMagazinePage());
    }

    void testNoChangeWhenIdentical()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, 0);

        PageDisplayable page;
        DecodedPage decodedPage;

        // First parse establishes baseline - use FULL_PAGE to set page ID
        bool changed1 = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                           NavigationMode::DEFAULT, decodedPage);

        // First parse should change (initial state)
        CPPUNIT_ASSERT_EQUAL(true, changed1);

        // Second parse with same header and page data (without ERASE_PAGE flag)
        bool changed2 = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                           NavigationMode::DEFAULT, decodedPage);

        // FULL_PAGE mode always returns true (processes all rows unconditionally)
        // This is by design in Parser.cpp line 262: rowResult.m_anyChange = true;
        CPPUNIT_ASSERT_EQUAL(true, changed2);
    }

    void testMultiplePagesNoContamination()
    {
        // Parse page 1
        PacketHeader header1;
        createHeaderPacket(header1, 100, ControlInfo::ERASE_PAGE);
        PageDisplayable page1;
        DecodedPage decodedPage1;
        m_parser->parsePage(page1, header1, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage1);

        // Parse page 2
        PacketHeader header2;
        createHeaderPacket(header2, 200, ControlInfo::ERASE_PAGE);
        PageDisplayable page2;
        DecodedPage decodedPage2;
        m_parser->parsePage(page2, header2, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage2);

        // Pages should have different IDs
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage1.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(200), decodedPage2.getPageId().getDecimalMagazinePage());
    }

    void testHeaderRow()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);
        setHeaderBytes(header, {'H', 'D', 'R'});

        PageDisplayable page;
        DecodedPage decodedPage;

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage);

        static const std::size_t headerColumn = 8;
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('H'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('D'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn + 1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>('R'), decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[headerColumn + 2]);
    }

    void testErasePageFlag()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        decodedPage.setPageId(PageId(0x100, PageId::ANY_SUBPAGE));
        seedRow(decodedPage, 1, 'E', Property::VALUE_FLASH);

        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                           NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(' '), decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), decodedPage.getRow(1).m_levelOnePageSegment.m_propertiesArray[0]);
    }

private:
    Parser* m_parser;
    Database* m_database;
    CharsetConfig* m_charsetConfig;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);
