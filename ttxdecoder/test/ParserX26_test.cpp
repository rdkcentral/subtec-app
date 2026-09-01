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
#include "PacketTriplets.hpp"
#include "DecodedPage.hpp"
#include "PresentationLevel.hpp"
#include "Types.hpp"
#include "ControlInfo.hpp"
#include "PageId.hpp"
#include "CharsetMaps.hpp"

using namespace ttxdecoder;

// Mock CharsetConfig for testing
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

class ParserX26Test : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ParserX26Test);
    CPPUNIT_TEST(testPacketTripletsSetAndGetDesignationCode);
    CPPUNIT_TEST(testPacketTripletsSetAndGetTripletValue);
    CPPUNIT_TEST(testPacketTripletsInvalidIndexReturnsMax);
    CPPUNIT_TEST(testPacketTripletsAllThirteenTriplets);
    CPPUNIT_TEST(testEmptyDisplayable);
    CPPUNIT_TEST(testFullModeLevel1);
    CPPUNIT_TEST(testFullModeLevel15);
    CPPUNIT_TEST(testAddress40MapsToRow24);
    CPPUNIT_TEST(testAddressDisplayRow0TargetsHeader);
    CPPUNIT_TEST(testSmoothMosaicWritesCharacter);
    CPPUNIT_TEST(testG2CharacterWritesMappedCharacter);
    CPPUNIT_TEST(testDiacriticPropertyApplied);
    CPPUNIT_TEST(testTerminationMarkerStopsProcessing);
    CPPUNIT_TEST(testTerminationMarkerWithHighBitsStopsProcessing);
    CPPUNIT_TEST(testContinuesAcrossPackets);
    CPPUNIT_TEST(testLaterPacketTerminationStopsFollowingPackets);
    CPPUNIT_TEST(testUnsupportedPrintableModeIgnored);
    CPPUNIT_TEST(testNonPrintableDataIgnored);

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
    Parser* m_parser;
    Database* m_database;
    CharsetConfig* m_charsetConfig;

    // Helper to create valid header packet
    void createHeaderPacket(PacketHeader& header, uint16_t decimalPage, std::uint8_t controlInfo)
    {
        uint16_t magazine = decimalPage / 100;
        uint16_t page1 = (decimalPage / 10) % 10;
        uint16_t page2 = decimalPage % 10;
        uint16_t magazinePage = (magazine << 8) | (page1 << 4) | page2;

        PageId pid(magazinePage, PageId::ANY_SUBPAGE);
        header.setPageInfo(pid, controlInfo, 0);
    }

    std::uint32_t makeTriplet(std::uint8_t address,
                              std::uint8_t mode,
                              std::uint8_t data) const
    {
        return address | (mode << 6) | (data << 11);
    }

    PacketTriplets& configureX26Packet(PageDisplayable& page,
                                       std::int8_t designationCode,
                                       std::initializer_list<std::uint32_t> triplets,
                                       std::uint32_t trailingValue = 0x7FF)
    {
        auto* packet = static_cast<PacketTriplets*>(page.takePacket(26, designationCode));
        CPPUNIT_ASSERT(packet != nullptr);

        packet->setDesignationCode(designationCode);

        for (std::size_t i = 0; i < PacketTriplets::TRIPLET_COUNT; ++i)
        {
            packet->setTripletValue(i, trailingValue);
        }

        std::size_t index = 0;
        for (auto triplet : triplets)
        {
            CPPUNIT_ASSERT(index < PacketTriplets::TRIPLET_COUNT);
            packet->setTripletValue(index++, triplet);
        }

        page.setLastPacketValid(packet);
        return *packet;
    }

    void testPacketTripletsSetAndGetDesignationCode()
    {
        PacketTriplets packet;
        packet.setDesignationCode(5);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(5), packet.getDesignationCode());

        // Test negative values
        packet.setDesignationCode(-1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(-1), packet.getDesignationCode());
    }

    void testPacketTripletsSetAndGetTripletValue()
    {
        PacketTriplets packet;
        packet.setTripletValue(0, 0x12345);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x12345), packet.getTripletValue(0));
    }

    void testPacketTripletsInvalidIndexReturnsMax()
    {
        PacketTriplets packet;

        // Invalid index should return 0xFFFFFFFF (TRIPLET_COUNT is 13)
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF),
                            packet.getTripletValue(13));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0xFFFFFFFF), packet.getTripletValue(999));
    }

    void testPacketTripletsAllThirteenTriplets()
    {
        PacketTriplets packet;

        // PacketTriplets has 13 triplets
        const std::size_t TRIPLET_COUNT = 13;

        // Set all 13 triplets
        for (std::size_t i = 0; i < TRIPLET_COUNT; i++)
        {
            packet.setTripletValue(i, i * 100);
        }

        // Verify all can be retrieved
        for (std::size_t i = 0; i < TRIPLET_COUNT; i++)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(i * 100), packet.getTripletValue(i));
        }
    }

    void testEmptyDisplayable()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;  // Empty page
        DecodedPage decodedPage;

        // parsePage is public and processes X26 internally in FULL_PAGE mode
        bool changed = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                          NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(ControlInfo::ERASE_PAGE),
                             decodedPage.getPageControlInfo());
    }

    void testFullModeLevel1()
    {
        Database db;
        MockCharsetConfig config;
        Parser parser(PresentationLevel::LEVEL_1, db, config);

        PacketHeader header;
        createHeaderPacket(header, 200, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        bool changed = parser.parsePage(page, header, Parser::Mode::FULL_PAGE,
                                       NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(200), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(ControlInfo::ERASE_PAGE),
                             decodedPage.getPageControlInfo());
    }

    void testFullModeLevel15()
    {
        Database db;
        MockCharsetConfig config;
        Parser parser(PresentationLevel::LEVEL_1_5, db, config);

        PacketHeader header;
        createHeaderPacket(header, 300, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        DecodedPage decodedPage;

        bool changed = parser.parsePage(page, header, Parser::Mode::FULL_PAGE,
                                       NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(true, changed);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(300), decodedPage.getPageId().getDecimalMagazinePage());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(ControlInfo::ERASE_PAGE),
                             decodedPage.getPageControlInfo());
    }

    void testAddress40MapsToRow24()
    {
        PacketHeader header;
        createHeaderPacket(header, 400, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(40, 0x4, 39),
                    makeTriplet(39, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(24).m_levelOnePageSegment.m_charArray[39]);
    }

    void testAddressDisplayRow0TargetsHeader()
    {
        PacketHeader header;
        createHeaderPacket(header, 401, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(0x1F, 0x7, 0),
                    makeTriplet(5, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(0).m_levelOnePageSegment.m_charArray[5]);
    }

    void testSmoothMosaicWritesCharacter()
    {
        PacketHeader header;
        createHeaderPacket(header, 402, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(42, 0x4, 0),
                    makeTriplet(3, 0x2, 0x25),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x25),
                             decodedPage.getRow(2).m_levelOnePageSegment.m_charArray[3]);
    }

    void testG2CharacterWritesMappedCharacter()
    {
        PacketHeader header;
        createHeaderPacket(header, 403, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(43, 0x4, 0),
                    makeTriplet(4, 0xF, 0x21),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00A1),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_charArray[4]);
    }

    void testDiacriticPropertyApplied()
    {
        PacketHeader header;
        createHeaderPacket(header, 404, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(44, 0x4, 0),
                    makeTriplet(6, 0x1A, 0x41),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('A'),
                             decodedPage.getRow(4).m_levelOnePageSegment.m_charArray[6]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1400),
                             decodedPage.getRow(4).m_levelOnePageSegment.m_propertiesArray[6]);
    }

    void testTerminationMarkerStopsProcessing()
    {
        PacketHeader header;
        createHeaderPacket(header, 405, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(41, 0x4, 0),
                    makeTriplet(1, 0x10, 0x2A),
                    0x7FF,
                    makeTriplet(42, 0x4, 0),
                    makeTriplet(2, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(' '),
                             decodedPage.getRow(2).m_levelOnePageSegment.m_charArray[2]);
    }

    void testTerminationMarkerWithHighBitsStopsProcessing()
    {
        PacketHeader header;
        createHeaderPacket(header, 406, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(41, 0x4, 0),
                    makeTriplet(1, 0x10, 0x2A),
                    0xF7FF,
                    makeTriplet(42, 0x4, 0),
                    makeTriplet(2, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(' '),
                             decodedPage.getRow(2).m_levelOnePageSegment.m_charArray[2]);
    }

    void testContinuesAcrossPackets()
    {
        PacketHeader header;
        createHeaderPacket(header, 408, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(45, 0x4, 0),
                },
                0);
        configureX26Packet(page, 1,
                {
                    makeTriplet(7, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(5).m_levelOnePageSegment.m_charArray[7]);
    }

    void testLaterPacketTerminationStopsFollowingPackets()
    {
        PacketHeader header;
        createHeaderPacket(header, 409, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(41, 0x4, 0),
                    makeTriplet(1, 0x10, 0x2A),
            },
            0);
        configureX26Packet(page, 1,
                {
                    makeTriplet(42, 0x4, 0),
                    makeTriplet(2, 0x10, 0x2A),
                    0x7FF,
                });
        configureX26Packet(page, 2,
                {
                    makeTriplet(43, 0x4, 0),
                    makeTriplet(3, 0x10, 0x2A),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(1).m_levelOnePageSegment.m_charArray[1]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('`'),
                             decodedPage.getRow(2).m_levelOnePageSegment.m_charArray[2]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(' '),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_charArray[3]);
    }

    void testUnsupportedPrintableModeIgnored()
    {
        PacketHeader header;
        createHeaderPacket(header, 410, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(43, 0x4, 0),
                    makeTriplet(4, 0x3, 0x41),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(' '),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_charArray[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_propertiesArray[4]);
    }

    void testNonPrintableDataIgnored()
    {
        PacketHeader header;
        createHeaderPacket(header, 407, ControlInfo::ERASE_PAGE);

        PageDisplayable page;
        configureX26Packet(page, 0,
                {
                    makeTriplet(43, 0x4, 0),
                    makeTriplet(4, 0xF, 0x1F),
                });

        DecodedPage decodedPage;
        m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                            NavigationMode::DEFAULT, decodedPage);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(' '),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_charArray[4]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0),
                             decodedPage.getRow(3).m_levelOnePageSegment.m_propertiesArray[4]);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserX26Test);
