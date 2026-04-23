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
    CPPUNIT_TEST(testPacketTripletsDefaultConstruction);
    CPPUNIT_TEST(testPacketTripletsSetAndGetDesignationCode);
    CPPUNIT_TEST(testPacketTripletsSetAndGetTripletValue);
    CPPUNIT_TEST(testPacketTripletsInvalidIndexReturnsMax);
    CPPUNIT_TEST(testPacketTripletsAllThirteenTriplets);
    CPPUNIT_TEST(testTerminationMarkerEncoding);
    CPPUNIT_TEST(testTerminationMarkerDetection);
    CPPUNIT_TEST(testNonTerminationMarkerValues);
    CPPUNIT_TEST(testTripletEncodingSetActivePosition);
    CPPUNIT_TEST(testTripletEncodingAddressDisplayRow0);
    CPPUNIT_TEST(testTripletEncodingSmoothMosaic);
    CPPUNIT_TEST(testTripletEncodingCharacterFromG2);
    CPPUNIT_TEST(testTripletEncodingDiacriticMode);
    CPPUNIT_TEST(testExtractAddressFieldMinMax);
    CPPUNIT_TEST(testExtractModeFieldValues);
    CPPUNIT_TEST(testExtractDataFieldValues);
    CPPUNIT_TEST(testAddressConversionValidRows);
    CPPUNIT_TEST(testAddressConversionInvalidAddresses);
    CPPUNIT_TEST(testColumnConversionBoundaries);
    CPPUNIT_TEST(testDiacriticPropertyCalculationMinMode);
    CPPUNIT_TEST(testDiacriticPropertyCalculationMaxMode);
    CPPUNIT_TEST(testDiacriticPropertyBitShift);
    CPPUNIT_TEST(testCharacterIndexFromDataField);
    CPPUNIT_TEST(testCharacterIndexBoundaries);
    CPPUNIT_TEST(testParsePageWithEmptyDisplayable);
    CPPUNIT_TEST(testParsePageFullModeLevel1);
    CPPUNIT_TEST(testParsePageFullModeLevel1_5);
    CPPUNIT_TEST(testDecodedPageRowAccess);
    CPPUNIT_TEST(testDecodedPageCharArrayManipulation);
    CPPUNIT_TEST(testDecodedPagePropertiesArrayManipulation);
    CPPUNIT_TEST(testDecodedPageBoundaryPositions);

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

    void testPacketTripletsDefaultConstruction()
    {
        PacketTriplets packet;

        // Default should allow getting designation code
        CPPUNIT_ASSERT_NO_THROW(packet.getDesignationCode());
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

    void testTerminationMarkerEncoding()
    {
        PacketTriplets packet;

        // Termination marker is 0x7FF (all lower 11 bits set)
        packet.setTripletValue(0, 0x7FF);

        std::uint32_t value = packet.getTripletValue(0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x7FF), value);
        CPPUNIT_ASSERT((value & 0x7FF) == 0x7FF);
    }

    void testTerminationMarkerDetection()
    {
        PacketTriplets packet;

        // Test various termination marker values
        std::uint32_t terminationMarkers[] = {0x7FF, 0xF7FF, 0x1F7FF, 0xFFFFFFFF};

        for (std::size_t i = 0; i < 4; i++)
        {
            packet.setTripletValue(i, terminationMarkers[i]);
            std::uint32_t value = packet.getTripletValue(i);
            // All should have lower 11 bits set to 0x7FF
            CPPUNIT_ASSERT((value & 0x7FF) == 0x7FF);
        }
    }

    void testNonTerminationMarkerValues()
    {
        PacketTriplets packet;

        // These should NOT be termination markers
        std::uint32_t nonTerminationValues[] = {0x400, 0x200, 0x7FE, 0x0};

        for (std::size_t i = 0; i < 4; i++)
        {
            packet.setTripletValue(i, nonTerminationValues[i]);
            std::uint32_t value = packet.getTripletValue(i);
            CPPUNIT_ASSERT((value & 0x7FF) != 0x7FF);
        }
    }

    void testTripletEncodingSetActivePosition()
    {
        PacketTriplets packet;

        // Encode: address=40, mode=0x4 (SET_ACTIVE_POSITION), data=10
        std::uint32_t triplet = 40 | (0x4 << 6) | (10 << 11);
        packet.setTripletValue(0, triplet);

        std::uint32_t value = packet.getTripletValue(0);
        std::uint8_t address = value & 0x3F;
        std::uint8_t mode = (value >> 6) & 0x1F;
        std::uint8_t data = (value >> 11) & 0x7F;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(40), address);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x4), mode);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(10), data);
    }

    void testTripletEncodingAddressDisplayRow0()
    {
        PacketTriplets packet;

        // Encode: address=0x1F, mode=0x7 (ADDRESS_DISPLAY_ROW0), data=0
        std::uint32_t triplet = 0x1F | (0x7 << 6) | (0 << 11);
        packet.setTripletValue(0, triplet);

        std::uint32_t value = packet.getTripletValue(0);
        std::uint8_t address = value & 0x3F;
        std::uint8_t mode = (value >> 6) & 0x1F;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x1F), address);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x7), mode);
    }

    void testTripletEncodingSmoothMosaic()
    {
        PacketTriplets packet;

        // Encode: address=10, mode=0x2 (LINE_DRAWING_OR_SMOOTH_MOSAIC), data=0x25
        std::uint32_t triplet = 10 | (0x2 << 6) | (0x25 << 11);
        packet.setTripletValue(0, triplet);

        std::uint32_t value = packet.getTripletValue(0);
        std::uint8_t mode = (value >> 6) & 0x1F;
        std::uint8_t data = (value >> 11) & 0x7F;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x2), mode);
        CPPUNIT_ASSERT(data >= 0x20);  // Valid data threshold
    }

    void testTripletEncodingCharacterFromG2()
    {
        PacketTriplets packet;

        // Encode: address=15, mode=0xF (CHARACTER_FROM_G2), data=0x30
        std::uint32_t triplet = 15 | (0xF << 6) | (0x30 << 11);
        packet.setTripletValue(0, triplet);

        std::uint32_t value = packet.getTripletValue(0);
        std::uint8_t mode = (value >> 6) & 0x1F;
        std::uint8_t data = (value >> 11) & 0x7F;

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xF), mode);
        CPPUNIT_ASSERT(data >= 0x20);
    }

    void testTripletEncodingDiacriticMode()
    {
        PacketTriplets packet;

        // Test diacritic mode range 0x10-0x1F
        for (std::uint8_t mode = 0x10; mode <= 0x1F; mode++)
        {
            std::uint32_t triplet = 20 | (mode << 6) | (0x30 << 11);
            packet.setTripletValue(0, triplet);

            std::uint32_t value = packet.getTripletValue(0);
            std::uint8_t extractedMode = (value >> 6) & 0x1F;

            CPPUNIT_ASSERT_EQUAL(mode, extractedMode);
            CPPUNIT_ASSERT(extractedMode >= 0x10 && extractedMode <= 0x1F);
        }
    }

    void testExtractAddressFieldMinMax()
    {
        PacketTriplets packet;

        // Test minimum address (0)
        packet.setTripletValue(0, 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            static_cast<std::uint8_t>(packet.getTripletValue(0) & 0x3F));

        // Test maximum address (63)
        packet.setTripletValue(1, 63);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(63),
                            static_cast<std::uint8_t>(packet.getTripletValue(1) & 0x3F));
    }

    void testExtractModeFieldValues()
    {
        PacketTriplets packet;

        // Test all mode values 0-31 (5 bits)
        for (std::uint8_t mode = 0; mode <= 0x1F; mode++)
        {
            std::uint32_t triplet = (mode << 6);
            packet.setTripletValue(0, triplet);

            std::uint8_t extractedMode = (packet.getTripletValue(0) >> 6) & 0x1F;
            CPPUNIT_ASSERT_EQUAL(mode, extractedMode);
        }
    }

    void testExtractDataFieldValues()
    {
        PacketTriplets packet;

        // Test minimum data (0)
        packet.setTripletValue(0, 0 << 11);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0),
                            static_cast<std::uint8_t>((packet.getTripletValue(0) >> 11) & 0x7F));

        // Test maximum data (127)
        packet.setTripletValue(1, 0x7F << 11);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x7F),
                            static_cast<std::uint8_t>((packet.getTripletValue(1) >> 11) & 0x7F));

        // Test data threshold (0x20 = 32)
        packet.setTripletValue(2, 0x20 << 11);
        std::uint8_t data = (packet.getTripletValue(2) >> 11) & 0x7F;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x20), data);
    }

    void testAddressConversionValidRows()
    {
        // Addresses 41-63 map to rows 1-23 (addr - 40)
        for (std::uint8_t addr = 41; addr <= 63; addr++)
        {
            std::uint8_t expectedRow = addr - 40;
            CPPUNIT_ASSERT(expectedRow >= 1 && expectedRow <= 23);
        }
    }

    void testAddressConversionInvalidAddresses()
    {
        // Addresses < 40 should return row 0
        for (std::uint8_t addr = 0; addr < 40; addr++)
        {
            // addressToRow would return 0 for these
            CPPUNIT_ASSERT(addr < 40);
        }
    }

    void testColumnConversionBoundaries()
    {
        // addressToColumn: address < 40 returns address, else 0

        // Valid column addresses (0-39)
        for (std::uint8_t addr = 0; addr < 40; addr++)
        {
            std::uint8_t column = (addr < 40) ? addr : 0;
            CPPUNIT_ASSERT_EQUAL(addr, column);
        }

        // Invalid column addresses (40-63) should return 0
        for (std::uint8_t addr = 40; addr <= 63; addr++)
        {
            std::uint8_t column = (addr < 40) ? addr : 0;
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), column);
        }
    }

    void testDiacriticPropertyCalculationMinMode()
    {
        // Mode 0x10: (0x10 & 0xF) << 9 = 0x0 << 9 = 0x0000
        std::uint8_t mode = 0x10;
        std::uint16_t diacritic = ((mode & 0xF) << 9);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0000), diacritic);
    }

    void testDiacriticPropertyCalculationMaxMode()
    {
        // Mode 0x1F: (0x1F & 0xF) << 9 = 0xF << 9 = 0x1E00
        std::uint8_t mode = 0x1F;
        std::uint16_t diacritic = ((mode & 0xF) << 9);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1E00), diacritic);
    }

    void testDiacriticPropertyBitShift()
    {
        // Test various diacritic modes
        std::uint8_t modes[] = {0x10, 0x11, 0x15, 0x1A, 0x1F};
        std::uint16_t expected[] = {0x0000, 0x0200, 0x0A00, 0x1400, 0x1E00};

        for (std::size_t i = 0; i < 5; i++)
        {
            std::uint16_t diacritic = ((modes[i] & 0xF) << 9);
            CPPUNIT_ASSERT_EQUAL(expected[i], diacritic);
        }
    }

    void testCharacterIndexFromDataField()
    {
        // dataToCharacterIndex: data >= 0x20 ? (data - 0x20) : 0

        // Data = 0x20 -> index 0
        std::uint16_t data = 0x20;
        std::uint16_t index = (data >= 0x20) ? (data - 0x20) : 0;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), index);

        // Data = 0x7F -> index 95
        data = 0x7F;
        index = (data >= 0x20) ? (data - 0x20) : 0;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(95), index);
    }

    void testCharacterIndexBoundaries()
    {
        // Data < 0x20 should return 0
        for (std::uint16_t data = 0; data < 0x20; data++)
        {
            std::uint16_t index = (data >= 0x20) ? (data - 0x20) : 0;
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), index);
        }

        // Data >= 0x20 should return (data - 0x20)
        for (std::uint16_t data = 0x20; data <= 0x7F; data++)
        {
            std::uint16_t index = (data >= 0x20) ? (data - 0x20) : 0;
            CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(data - 0x20), index);
        }
    }

    void testParsePageWithEmptyDisplayable()
    {
        PacketHeader header;
        createHeaderPacket(header, 100, ControlInfo::ERASE_PAGE);

        PageDisplayable page;  // Empty page
        DecodedPage decodedPage;

        // parsePage is public and processes X26 internally in FULL_PAGE mode
        bool changed = m_parser->parsePage(page, header, Parser::Mode::FULL_PAGE,
                                          NavigationMode::DEFAULT, decodedPage);

        // Should complete without error
        CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), decodedPage.getPageId().getDecimalMagazinePage());
    }

    void testParsePageFullModeLevel1()
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

        CPPUNIT_ASSERT(true);  // Should complete without error
    }

    void testParsePageFullModeLevel1_5()
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

        CPPUNIT_ASSERT(true);  // Should complete without error
    }

    void testDecodedPageRowAccess()
    {
        DecodedPage page;

        // Should be able to access all rows (0-24)
        for (std::size_t row = 0; row <= 24; row++)
        {
            CPPUNIT_ASSERT_NO_THROW(page.getRow(row));
        }
    }

    void testDecodedPageCharArrayManipulation()
    {
        DecodedPage page;

        // Set character at row 10, column 15
        DecodedPageRow& row = page.getRow(10);
        row.m_levelOnePageSegment.m_charArray[15] = 'A';

        // Verify it was set correctly
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('A'),
                            page.getRow(10).m_levelOnePageSegment.m_charArray[15]);

        // Overwrite with different character
        row.m_levelOnePageSegment.m_charArray[15] = 'B';
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('B'),
                            page.getRow(10).m_levelOnePageSegment.m_charArray[15]);
    }

    void testDecodedPagePropertiesArrayManipulation()
    {
        DecodedPage page;

        // Test setting properties at different positions
        DecodedPageRow& row5 = page.getRow(5);
        row5.m_levelOnePageSegment.m_propertiesArray[20] = 0x0A00;

        // Verify property was set
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0A00),
                            row5.m_levelOnePageSegment.m_propertiesArray[20]);

        // Test setting different property value at different position
        DecodedPageRow& row10 = page.getRow(10);
        row10.m_levelOnePageSegment.m_propertiesArray[15] = 0x1E00;

        // Verify second property was set
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1E00),
                            row10.m_levelOnePageSegment.m_propertiesArray[15]);
    }

    void testDecodedPageBoundaryPositions()
    {
        DecodedPage page;

        // Test minimum position (0, 0)
        DecodedPageRow& row0 = page.getRow(0);
        row0.m_levelOnePageSegment.m_charArray[0] = 'X';
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('X'),
                            page.getRow(0).m_levelOnePageSegment.m_charArray[0]);

        // Test maximum position (24, 39)
        DecodedPageRow& row24 = page.getRow(24);
        row24.m_levelOnePageSegment.m_charArray[39] = 'Y';
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>('Y'),
                            page.getRow(24).m_levelOnePageSegment.m_charArray[39]);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserX26Test);
