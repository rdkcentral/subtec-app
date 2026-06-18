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
    CPPUNIT_TEST(testNewsflashMode);
    CPPUNIT_TEST(testSubtitleMode);
    CPPUNIT_TEST(testIgnoresOtherFlags);
    CPPUNIT_TEST(testContextReset);
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

    void testIgnoresOtherFlags()
    {
        PacketHeader boxedHeader;
        createHeaderPacket(boxedHeader, 100, ControlInfo::NEWSFLASH | ControlInfo::SUBTITLE);
        setHeaderBytes(boxedHeader, {0x0B, 'X', 0x0A});

        PacketHeader noisyHeader;
        createHeaderPacket(noisyHeader, 100, 0xFF);
        setHeaderBytes(noisyHeader, {0x0B, 'X', 0x0A});

        PageDisplayable page;
        DecodedPage boxedPage;
        DecodedPage noisyPage;

        m_parser->parsePage(page, boxedHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, boxedPage);
        m_parser->parsePage(page, noisyHeader, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, noisyPage);

        static const std::size_t visibleColumn = 9;
        CPPUNIT_ASSERT_EQUAL(boxedPage.getRow(0).m_levelOnePageSegment.m_charArray[visibleColumn],
                             noisyPage.getRow(0).m_levelOnePageSegment.m_charArray[visibleColumn]);
        CPPUNIT_ASSERT_EQUAL(boxedPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[visibleColumn],
                             noisyPage.getRow(0).m_levelOnePageSegment.m_propertiesArray[visibleColumn]);
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
