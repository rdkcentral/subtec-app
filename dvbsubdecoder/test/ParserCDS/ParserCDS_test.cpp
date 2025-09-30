/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
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
*****************************************************************************/


#include <cppunit/extensions/HelperMacros.h>

#include "ParserCDS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "ParserException.hpp"
#include "ColorCalculator.hpp"

#include "DecoderClientMock.hpp"
#include "BitStreamWriter.hpp"
#include "Misc.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserCDS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::ParserException;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;
using dvbsubdecoder::ColorYCrCbT;
using dvbsubdecoder::ColorARGB;
using dvbsubdecoder::ColorCalculator;

class ParserCDSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserCDSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testBadState);
    CPPUNIT_TEST(testEmptySegment);
    CPPUNIT_TEST(testBoundaryEntryIds);
    CPPUNIT_TEST(testBoundaryColorValues);
    CPPUNIT_TEST(testTruncatedHeader);
    CPPUNIT_TEST(testTruncatedVersionByte);
    CPPUNIT_TEST(testTruncatedFullRangeEntry);
    CPPUNIT_TEST(testTruncatedLimitedRangeEntry);
    CPPUNIT_TEST(testInvalidFlagCombinations);
    CPPUNIT_TEST(testEntryIdExceedingBitDepthRange);
    CPPUNIT_TEST(testSameVersionNumber);
    CPPUNIT_TEST(testClutNotFound);
    CPPUNIT_TEST(testMixedRangeEntries);
    CPPUNIT_TEST(testMultipleEntriesSameId);
    CPPUNIT_TEST(testLargeSegmentManyEntries);
    CPPUNIT_TEST(testVersionRollover);
    CPPUNIT_TEST(testAllPageStates);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_client.reset(new DecoderClientMock());
        m_client->setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);
        m_pixmapAllocator.reset(new PixmapAllocator(SPEC_VERSION, *m_client));
        m_database.reset(new Database(SPEC_VERSION, *m_pixmapAllocator));
    }

    void tearDown()
    {
        m_database.reset();
        m_pixmapAllocator.reset();
        m_client.reset();
    }

    void testSimple()
    {
        const std::int32_t Y_FULL_TRANSPARENT = 0;
        const std::int32_t Y_RANGE_MIN = 16;
        const std::int32_t Y_RANGE_MAX = 235;
        const std::int32_t CX_RANGE_MIN = 16;
        const std::int32_t CX_RANGE_MAX = 240;

        ColorYCrCbT colors[] =
        {
        { 0, 0, 0, 0 },
        { Y_RANGE_MIN, 0, 0, 0 },
        { 0xFF, 0xFF, 0xFF, 0xFF },
        { Y_RANGE_MIN, CX_RANGE_MIN, CX_RANGE_MIN, 0 },
        { Y_RANGE_MAX, CX_RANGE_MAX, CX_RANGE_MAX, 0xFF },
        { 128, 192, 160, 120 },
        { 64, 65, 66, 67 },
        { 30, 32, 34, 36 },
        { 200, 199, 201, 198 },
        { 2, 254, 3, 11 },
        };


        const std::uint8_t CLUT_ID = 5;

        BitStreamWriter writer;

        writer.write(CLUT_ID, 8); // clut_id
        writer.write((0xC << 4), 8); // version

        std::uint8_t entryId = 0;

        for (auto iter = std::begin(colors); iter != std::end(colors);
                ++iter, ++entryId)
        {
            std::uint8_t flags = 1; // full range

            if (entryId < (1 << 2))
            {
                flags |= (1 << 7);
            }
            else if (entryId < (1 << 4))
            {
                flags |= (1 << 6);
            }
            else
            {
                flags |= (1 << 5);
            }

            writer.write(entryId, 8); // entry_id
            writer.write(flags, 8); // flags
            writer.write(iter->m_y, 8);
            writer.write(iter->m_cr, 8);
            writer.write(iter->m_cb, 8);
            writer.write(iter->m_t, 8);
        }

        for (auto iter = std::begin(colors); iter != std::end(colors);
                ++iter, ++entryId)
        {
            std::uint8_t flags = 0;

            if (entryId < (1 << 2))
            {
                flags |= (1 << 7);
            }
            else if (entryId < (1 << 4))
            {
                flags |= (1 << 6);
            }
            else
            {
                flags |= (1 << 5);
            }

            // limited range
            std::uint16_t y = (iter->m_y >> 2) & 0x3F;
            std::uint16_t cr = (iter->m_cr >> 4) & 0x0F;
            std::uint16_t cb = (iter->m_cb >> 4) & 0x0F;
            std::uint16_t t = (iter->m_t >> 6) & 0x03;
            std::uint16_t value = (y << 10) | (cr << 6) | (cb << 2) | t;

            writer.write(entryId, 8); // entry_id
            writer.write(flags, 8); // flags
            writer.write(value, 16);
        }

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        entryId = 0;
        for (auto iter = std::begin(colors); iter != std::end(colors);
                ++iter, ++entryId)
        {
            std::uint32_t argb = 0;

            if (entryId < (1 << 2))
            {
                argb = clut->getArray2bit()[entryId];
            }
            else if (entryId < (1 << 4))
            {
                argb = clut->getArray4bit()[entryId];
            }
            else
            {
                argb = clut->getArray8bit()[entryId];
            }

            auto argbRef = ColorCalculator().toARGB(*iter).toUint32();

            CPPUNIT_ASSERT(argb == argbRef);
        }

        for (auto iter = std::begin(colors); iter != std::end(colors);
                ++iter, ++entryId)
        {
            std::uint32_t argb = 0;

            if (entryId < (1 << 2))
            {
                argb = clut->getArray2bit()[entryId];
            }
            else if (entryId < (1 << 4))
            {
                argb = clut->getArray4bit()[entryId];
            }
            else
            {
                argb = clut->getArray8bit()[entryId];
            }

            ColorYCrCbT refColor = *iter;

            refColor.m_y &= 0xFC;
            refColor.m_cr &= 0xF0;
            refColor.m_cb &= 0xF0;
            refColor.m_t &= 0xC0;

            auto argbRef = ColorCalculator().toARGB(refColor).toUint32();

            CPPUNIT_ASSERT(argb == argbRef);
        }
    }

    void testBadState()
    {
        const std::uint8_t CLUT_ID = 5;

        BitStreamWriter writer;

        writer.write(CLUT_ID, 8); // clut_id
        writer.write((0xC << 4), 8); // version

        m_database->epochReset();

        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getClutById(CLUT_ID) == nullptr);
        }

        m_database->getPage().startParsing(0, StcTime(), 0);

        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getClutById(CLUT_ID) == nullptr);
        }
    }

    // Test empty segment (header only)
    void testEmptySegment()
    {
        const std::uint8_t CLUT_ID = 10;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8); // clut_id
        writer.write((0x5 << 4), 8); // version

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);
        CPPUNIT_ASSERT(clut->getVersion() == 0x5);
    }

    // Test boundary entry IDs (4, 16, 255)
    void testBoundaryEntryIds()
    {
        const std::uint8_t CLUT_ID = 25;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x8 << 4), 8);

        // Test entry at 2-bit/4-bit boundary (entry ID 4)
        writer.write(4, 8); // entry_id
        writer.write((1 << 6) | (1 << 0), 8); // 4-bit flag + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        // Test entry at 4-bit/8-bit boundary (entry ID 16)
        writer.write(16, 8); // entry_id
        writer.write((1 << 5) | (1 << 0), 8); // 8-bit flag + full range
        writer.write(150, 8); // Y
        writer.write(160, 8); // Cr
        writer.write(170, 8); // Cb
        writer.write(180, 8); // T

        // Test maximum entry ID (255)
        writer.write(255, 8); // entry_id
        writer.write((1 << 5) | (1 << 0), 8); // 8-bit flag + full range
        writer.write(200, 8); // Y
        writer.write(210, 8); // Cr
        writer.write(220, 8); // Cb
        writer.write(230, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        // Verify 4-bit entry
        ColorYCrCbT expectedColor4 = {100, 110, 120, 130};
        auto expectedArgb4 = ColorCalculator().toARGB(expectedColor4).toUint32();
        CPPUNIT_ASSERT(clut->getArray4bit()[4] == expectedArgb4);

        // Verify 8-bit entries
        ColorYCrCbT expectedColor16 = {150, 160, 170, 180};
        auto expectedArgb16 = ColorCalculator().toARGB(expectedColor16).toUint32();
        CPPUNIT_ASSERT(clut->getArray8bit()[16] == expectedArgb16);

        ColorYCrCbT expectedColor255 = {200, 210, 220, 230};
        auto expectedArgb255 = ColorCalculator().toARGB(expectedColor255).toUint32();
        CPPUNIT_ASSERT(clut->getArray8bit()[255] == expectedArgb255);
    }

    // Test boundary color values
    void testBoundaryColorValues()
    {
        const std::uint8_t CLUT_ID = 30;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x9 << 4), 8);

        // Test with Y=16, Cr/Cb=16 (range minimum)
        writer.write(0, 8); // entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(16, 8); // Y
        writer.write(16, 8); // Cr
        writer.write(16, 8); // Cb
        writer.write(0, 8); // T

        // Test with Y=235, Cr/Cb=240 (range maximum)
        writer.write(1, 8); // entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(235, 8); // Y
        writer.write(240, 8); // Cr
        writer.write(240, 8); // Cb
        writer.write(255, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        ColorYCrCbT expectedMin = {16, 16, 16, 0};
        auto expectedArgbMin = ColorCalculator().toARGB(expectedMin).toUint32();
        CPPUNIT_ASSERT(clut->getArray2bit()[0] == expectedArgbMin);

        ColorYCrCbT expectedMax = {235, 240, 240, 255};
        auto expectedArgbMax = ColorCalculator().toARGB(expectedMax).toUint32();
        CPPUNIT_ASSERT(clut->getArray2bit()[1] == expectedArgbMax);
    }

    // Test truncated header (insufficient data for CLUT ID)
    void testTruncatedHeader()
    {
        BitStreamWriter writer;
        // Don't write anything - empty data

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        bool exceptionThrown = false;
        try {
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
        } catch (const PesPacketReader::Exception&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test truncated version byte
    void testTruncatedVersionByte()
    {
        BitStreamWriter writer;
        writer.write(15, 8); // clut_id only, no version byte

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        bool exceptionThrown = false;
        try {
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
        } catch (const PesPacketReader::Exception&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test truncated full-range entry
    void testTruncatedFullRangeEntry()
    {
        const std::uint8_t CLUT_ID = 35;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0xA << 4), 8);
        writer.write(0, 8); // entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        // Missing Cb and T bytes

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        bool exceptionThrown = false;
        try {
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
        } catch (const PesPacketReader::Exception&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test truncated limited-range entry
    void testTruncatedLimitedRangeEntry()
    {
        const std::uint8_t CLUT_ID = 40;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0xB << 4), 8);
        writer.write(0, 8); // entry_id
        writer.write((1 << 7), 8); // 2-bit flag, no full range
        writer.write(0x12, 8); // Only first byte of 16-bit value

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        bool exceptionThrown = false;
        try {
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
        } catch (const PesPacketReader::Exception&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test invalid flag combinations (no bit depth flags)
    void testInvalidFlagCombinations()
    {
        const std::uint8_t CLUT_ID = 45;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0xC << 4), 8);
        writer.write(0, 8); // entry_id
        writer.write((1 << 0), 8); // Only full range flag, no bit depth flags
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        // Snapshot default arrays before parse
        auto clutPre = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clutPre);
        std::uint32_t pre2[4];
        std::uint32_t pre4[16];
        // For performance we only sample some 8-bit indices (0,5,10) though none should change.
        std::uint32_t pre8_0 = clutPre->getArray8bit()[0];
        std::uint32_t pre8_5 = clutPre->getArray8bit()[5];
        std::uint32_t pre8_10 = clutPre->getArray8bit()[10];
        for (int i=0;i<4;++i) pre2[i]=clutPre->getArray2bit()[i];
        for (int i=0;i<16;++i) pre4[i]=clutPre->getArray4bit()[i];

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);
        // Assert nothing changed in 2-bit and 4-bit arrays
        for (int i=0;i<4;++i) { CPPUNIT_ASSERT(clut->getArray2bit()[i] == pre2[i]); }
        for (int i=0;i<16;++i) { CPPUNIT_ASSERT(clut->getArray4bit()[i] == pre4[i]); }
        // Sample 8-bit indices unchanged
        CPPUNIT_ASSERT(clut->getArray8bit()[0] == pre8_0);
        CPPUNIT_ASSERT(clut->getArray8bit()[5] == pre8_5);
        CPPUNIT_ASSERT(clut->getArray8bit()[10] == pre8_10);
    }

    // Test entry IDs exceeding bit depth ranges
    void testEntryIdExceedingBitDepthRange()
    {
        const std::uint8_t CLUT_ID = 50;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0xD << 4), 8);

        // Test 2-bit entry with ID > 3 (should be ignored/warned)
        writer.write(5, 8); // entry_id exceeds 2-bit range
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        // Test 4-bit entry with ID > 15 (should be ignored/warned)
        writer.write(20, 8); // entry_id exceeds 4-bit range
        writer.write((1 << 6) | (1 << 0), 8); // 4-bit + full range
        writer.write(140, 8); // Y
        writer.write(150, 8); // Cr
        writer.write(160, 8); // Cb
        writer.write(170, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        // Snapshot default arrays (2-bit indices 0..3, 4-bit indices 0..15)
        auto clutPre = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clutPre);
        std::uint32_t pre2[4];
        std::uint32_t pre4[16];
        for (int i=0;i<4;++i) pre2[i]=clutPre->getArray2bit()[i];
        for (int i=0;i<16;++i) pre4[i]=clutPre->getArray4bit()[i];

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        // Should not throw exception but should log warnings
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);
        // Entries should be skipped due to range violations -> arrays must be identical
        for (int i=0;i<4;++i) { CPPUNIT_ASSERT(clut->getArray2bit()[i] == pre2[i]); }
        for (int i=0;i<16;++i) { CPPUNIT_ASSERT(clut->getArray4bit()[i] == pre4[i]); }
    }

    // Test same version number handling
    void testSameVersionNumber()
    {
        const std::uint8_t CLUT_ID = 55;
        const std::uint8_t VERSION = 0x6;

        // First parse with version 6
        {
            BitStreamWriter writer;
            writer.write(CLUT_ID, 8);
            writer.write((VERSION << 4), 8);
            writer.write(0, 8); // entry_id
            writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
            writer.write(100, 8); // Y
            writer.write(110, 8); // Cr
            writer.write(120, 8); // Cb
            writer.write(130, 8); // T

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);
            m_database->addRegionAndClut(0, 10, 10,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            CPPUNIT_ASSERT(clut->getVersion() == VERSION);
        }

        // Second parse with same version 6 (should be skipped)
        {
            BitStreamWriter writer;
            writer.write(CLUT_ID, 8);
            writer.write((VERSION << 4), 8);
            writer.write(0, 8); // entry_id
            writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
            writer.write(200, 8); // Different Y value
            writer.write(210, 8); // Different Cr value
            writer.write(220, 8); // Different Cb value
            writer.write(230, 8); // Different T value

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            
            // Should still have old values since same version was skipped
            ColorYCrCbT expectedOld = {100, 110, 120, 130};
            auto expectedArgbOld = ColorCalculator().toARGB(expectedOld).toUint32();
            CPPUNIT_ASSERT(clut->getArray2bit()[0] == expectedArgbOld);
        }
    }

    // Test CLUT not found in database
    void testClutNotFound()
    {
        const std::uint8_t CLUT_ID = 99; // CLUT not added to database

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x7 << 4), 8);
        writer.write(0, 8); // entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        // Note: NOT adding CLUT to database

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        
        // Should exit early without exception
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);
        
        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut == nullptr);
    }

    // Test mixed full-range and limited-range entries
    void testMixedRangeEntries()
    {
        const std::uint8_t CLUT_ID = 60;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x8 << 4), 8);

        // Full-range entry
        writer.write(0, 8); // entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        // Limited-range entry
        writer.write(1, 8); // entry_id
        writer.write((1 << 7), 8); // 2-bit, no full range
        std::uint16_t limitedValue = (25 << 10) | (7 << 6) | (8 << 2) | 2;
        writer.write(limitedValue, 16);

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        // Verify full-range entry
        ColorYCrCbT expectedFull = {100, 110, 120, 130};
        auto expectedArgbFull = ColorCalculator().toARGB(expectedFull).toUint32();
        CPPUNIT_ASSERT(clut->getArray2bit()[0] == expectedArgbFull);

        // Verify limited-range entry (converted to full range)
        ColorYCrCbT expectedLimited = {25 << 2, 7 << 4, 8 << 4, 2 << 6};
        auto expectedArgbLimited = ColorCalculator().toARGB(expectedLimited).toUint32();
        CPPUNIT_ASSERT(clut->getArray2bit()[1] == expectedArgbLimited);
    }

    // Test multiple entries with same ID but different flags
    void testMultipleEntriesSameId()
    {
        const std::uint8_t CLUT_ID = 65;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x9 << 4), 8);

        // First entry for ID 5 with 4-bit and 8-bit flags
        writer.write(5, 8); // entry_id
        writer.write((1 << 6) | (1 << 5) | (1 << 0), 8); // 4-bit + 8-bit + full range
        writer.write(100, 8); // Y
        writer.write(110, 8); // Cr
        writer.write(120, 8); // Cb
        writer.write(130, 8); // T

        // Second entry for same ID 5 with different values and 2-bit flag
        writer.write(5, 8); // same entry_id
        writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
        writer.write(200, 8); // Y
        writer.write(210, 8); // Cr
        writer.write(220, 8); // Cb
        writer.write(230, 8); // T

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        // Snapshot default 2-bit entries (indices 0..3) before parsing to prove they remain unchanged.
        auto clutBefore = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clutBefore); // Should exist right after addRegionAndClut
        std::uint32_t default2bit[4];
        for (int i = 0; i < 4; ++i)
        {
            default2bit[i] = clutBefore->getArray2bit()[i];
        }

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);
        // Version should be the one in the segment header (0x9)
        CPPUNIT_ASSERT(clut->getVersion() == 0x9);

        // Verify first entry set both 4-bit and 8-bit
        ColorYCrCbT expectedFirst = {100, 110, 120, 130};
        auto expectedArgbFirst = ColorCalculator().toARGB(expectedFirst).toUint32();
        CPPUNIT_ASSERT(clut->getArray4bit()[5] == expectedArgbFirst);
        CPPUNIT_ASSERT(clut->getArray8bit()[5] == expectedArgbFirst);

        // Verify second entry overwrote only the valid range (2-bit entry ID 5 is out of range)
        // 2-bit range is 0-3, so entry ID 5 should be ignored for 2-bit
        for (int i = 0; i < 4; ++i)
        {
            CPPUNIT_ASSERT(clut->getArray2bit()[i] == default2bit[i]);
        }

        // Re-assert 4-bit / 8-bit entries unchanged after the second (ignored for these arrays) entry.
        CPPUNIT_ASSERT(clut->getArray4bit()[5] == expectedArgbFirst);
        CPPUNIT_ASSERT(clut->getArray8bit()[5] == expectedArgbFirst);
    }

    // Test large segment with many entries
    void testLargeSegmentManyEntries()
    {
        const std::uint8_t CLUT_ID = 70;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0xA << 4), 8);

        // Add 50 entries to test performance
        for (int i = 0; i < 50; ++i) {
            writer.write(i, 8); // entry_id
            writer.write((1 << 5) | (1 << 0), 8); // 8-bit + full range
            writer.write(i + 10, 8); // Y
            writer.write(i + 20, 8); // Cr
            writer.write(i + 30, 8); // Cb
            writer.write(i + 40, 8); // T
        }

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->addRegionAndClut(0, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserCDS().parseClutDefinitionSegment(*m_database, reader);

        auto clut = m_database->getClutById(CLUT_ID);
        CPPUNIT_ASSERT(clut);

        // Verify some of the entries
        for (int i = 0; i < 10; ++i) {
            ColorYCrCbT expected = {
                static_cast<std::uint8_t>(i + 10),
                static_cast<std::uint8_t>(i + 20),
                static_cast<std::uint8_t>(i + 30),
                static_cast<std::uint8_t>(i + 40)
            };
            auto expectedArgb = ColorCalculator().toARGB(expected).toUint32();
            CPPUNIT_ASSERT(clut->getArray8bit()[i] == expectedArgb);
        }
    }

    // Test version rollover (15 to 0)
    void testVersionRollover()
    {
        const std::uint8_t CLUT_ID = 75;

        // First parse with version 15
        {
            BitStreamWriter writer;
            writer.write(CLUT_ID, 8);
            writer.write((0xF << 4), 8); // version 15
            writer.write(0, 8); // entry_id
            writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
            writer.write(100, 8); // Y
            writer.write(110, 8); // Cr
            writer.write(120, 8); // Cb
            writer.write(130, 8); // T

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);
            m_database->addRegionAndClut(0, 10, 10,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            CPPUNIT_ASSERT(clut->getVersion() == 0xF);
        }

        // Second parse with version 0 (rollover)
        {
            BitStreamWriter writer;
            writer.write(CLUT_ID, 8);
            writer.write((0x0 << 4), 8); // version 0
            writer.write(0, 8); // entry_id
            writer.write((1 << 7) | (1 << 0), 8); // 2-bit + full range
            writer.write(200, 8); // Y
            writer.write(210, 8); // Cr
            writer.write(220, 8); // Cb
            writer.write(230, 8); // T

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);

            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            CPPUNIT_ASSERT(clut->getVersion() == 0x0);

            // Should have new values
            ColorYCrCbT expectedNew = {200, 210, 220, 230};
            auto expectedArgbNew = ColorCalculator().toARGB(expectedNew).toUint32();
            CPPUNIT_ASSERT(clut->getArray2bit()[0] == expectedArgbNew);
        }
    }

    // Test all page states
    void testAllPageStates()
    {
        const std::uint8_t CLUT_ID = 80;

        BitStreamWriter writer;
        writer.write(CLUT_ID, 8);
        writer.write((0x5 << 4), 8);

        m_database->epochReset();

        // Test INVALID state (should exit early)
        {
            // Page starts in INVALID state after epochReset
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
            // Should exit early, no processing
        }

        // Test COMPLETE state (should exit early)
        {
            m_database->getPage().startParsing(0, StcTime(), 0);
            m_database->addRegionAndClut(0, 10, 10,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);
            m_database->getPage().finishParsing();
            // Page is now in COMPLETE state

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
            
            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            // Version should not have been updated since parsing was skipped
            CPPUNIT_ASSERT(clut->getVersion() != 0x5);
        }

        // Test TIMEDOUT state (should exit early)
        {
            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);
            m_database->addRegionAndClut(0, 10, 10,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, CLUT_ID);
            // Complete parsing then mark timed out
            m_database->getPage().finishParsing();
            m_database->getPage().setTimedOut();
            // Page is now in TIMEDOUT state

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserCDS().parseClutDefinitionSegment(*m_database, reader);
            
            auto clut = m_database->getClutById(CLUT_ID);
            CPPUNIT_ASSERT(clut);
            // Version should not have been updated since parsing was skipped
            CPPUNIT_ASSERT(clut->getVersion() != 0x5);
        }
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserCDSTest);
