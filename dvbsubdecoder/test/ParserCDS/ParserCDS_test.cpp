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

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserCDSTest);
