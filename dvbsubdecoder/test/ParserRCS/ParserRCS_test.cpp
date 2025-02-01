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

#include "ParserRCS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "ParserException.hpp"

#include "DecoderClientMock.hpp"
#include "BitStreamWriter.hpp"
#include "Misc.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserRCS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::ParserException;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserRCSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserRCSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testBadData);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testBadUpdate);CPPUNIT_TEST_SUITE_END()
    ;

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
        const std::uint16_t REGION_ID = 0xDC;

        BitStreamWriter writer;

        writer.write(REGION_ID, 8); // region_id
        writer.write(0xC << 4, 8); // version=C, fill_flag=0
        writer.write(300, 16); // width
        writer.write(200, 16); // height
        writer.write(
                (dvbsubdecoder::RegionDepthBits::DEPTH_2BIT << 5)
                        | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2), 8); // depth=8bit, compatibility=2bit
        writer.write(5, 8); // clut_id
        writer.write(3, 8); // background 8bit
        writer.write(0, 8); // other backgrounds, reserved (unused)

        // object 1/2
        writer.write(1000, 16); // object_id
        writer.write(
                (dvbsubdecoder::RegionObjectTypeBits::BASIC_BITMAP << 6)
                        | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM
                                << 4), 8);
        writer.write(0, 8);
        writer.write(0, 16);

        // object 2/2
        writer.write(1005, 16); // object_id
        writer.write(
                (dvbsubdecoder::RegionObjectTypeBits::BASIC_BITMAP << 6)
                        | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM
                                << 4), 8);
        writer.write(10, 8);
        writer.write(100, 16);

        // cannot parse, page not started
        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getRegionCount() == 0);
        }

        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        // parse, must succeed
        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, reader);

            CPPUNIT_ASSERT(m_database->getRegionCount() == 1);
            auto region = m_database->getRegionById(REGION_ID);
            CPPUNIT_ASSERT(region);
            CPPUNIT_ASSERT(region->getVersion() == 0xC);
            CPPUNIT_ASSERT(region->getWidth() == 300);
            CPPUNIT_ASSERT(region->getHeight() == 200);
            CPPUNIT_ASSERT(
                    region->getDepth()
                            == dvbsubdecoder::RegionDepthBits::DEPTH_8BIT);
            CPPUNIT_ASSERT(
                    region->getCompatibilityLevel()
                            == dvbsubdecoder::RegionDepthBits::DEPTH_2BIT);
            CPPUNIT_ASSERT(region->getClutId() == 5);
            CPPUNIT_ASSERT(region->getBackgroundIndex() == 3);

            auto object1 = region->getFirstObject();
            CPPUNIT_ASSERT(object1->m_objectId == 1000);
            CPPUNIT_ASSERT(object1->m_positionX == 0);
            CPPUNIT_ASSERT(object1->m_positionY == 0);

            auto object2 = region->getNextObject(object1);
            CPPUNIT_ASSERT(object2->m_objectId == 1005);
            CPPUNIT_ASSERT(object2->m_positionX == 10);
            CPPUNIT_ASSERT(object2->m_positionY == 100);

            CPPUNIT_ASSERT(region->getNextObject(object2) == nullptr);
        }

        // update, must succeed
        {
            m_database->getPage().finishParsing();
            m_database->getRegionByIndex(0)->setVersion(0xB);

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, reader);
        }

        // same version, must succeed
        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, reader);
        }

        // m_finish parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);
        m_database->getPage().finishParsing();

        // cannot add, parsing is complete and the region is not in DB
        {
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, reader);
        }
    }

    void testBadData()
    {
        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write(0xC << 4, 8); // version=C, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (0 << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2),
                    8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // cannot parse, page not started
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                CPPUNIT_ASSERT_THROW(
                        ParserRCS().parseRegionCompositionSegment(*m_database,
                                reader), ParserException);
            }
        }

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write(0xC << 4, 8); // version=C, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_4BIT << 5) | (0), 8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // cannot parse, page not started
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                CPPUNIT_ASSERT_THROW(
                        ParserRCS().parseRegionCompositionSegment(*m_database,
                                reader), ParserException);
            }
        }

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write(0xC << 4, 8); // version=C, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_4BIT << 2),
                    8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // object
            writer.write(1000, 16); // object_id
            writer.write(
                    (dvbsubdecoder::RegionObjectTypeBits::BASIC_CHARACTER << 6)
                            | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM
                                    << 4), 8);
            writer.write(0, 8);
            writer.write(0, 16);
            writer.write(0, 8); // fg
            writer.write(0, 8); // bg

            // cannot parse, page not started
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                CPPUNIT_ASSERT_THROW(
                        ParserRCS().parseRegionCompositionSegment(*m_database,
                                reader), ParserException);
            }
        }

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write(0xC << 4, 8); // version=C, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_4BIT << 2),
                    8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // object
            writer.write(1000, 16); // object_id
            writer.write(
                    (dvbsubdecoder::RegionObjectTypeBits::BASIC_BITMAP << 6)
                            | (1 << 4), 8);
            writer.write(0, 8);
            writer.write(0, 16);

            // cannot parse, page not started
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                CPPUNIT_ASSERT_THROW(
                        ParserRCS().parseRegionCompositionSegment(*m_database,
                                reader), ParserException);
            }
        }
    }

    void testFill()
    {
        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write((0xC << 4) | (1 << 3), 8); // version=C, fill_flag=1
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2),
                    8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // parse (add region)
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                ParserRCS().parseRegionCompositionSegment(*m_database, reader);
            }

            // clear pixmap with 0xFF
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(region);
            region->getPixmap().clear(0xFF);

            // parse again (update)
            {
                region->setVersion(0xB);

                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                ParserRCS().parseRegionCompositionSegment(*m_database, reader);
            }

            CPPUNIT_ASSERT(region->getPixmap().getBuffer()[0] == 0x3); // bg index
        }
    }

    void testBadUpdate()
    {
        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write((0xC << 4), 8); // version=C, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2),
                    8); // depth & compatibility
            writer.write(5, 8); // clut_id
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // parse (add region)
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                ParserRCS().parseRegionCompositionSegment(*m_database, reader);
            }
        }

        m_database->getPage().finishParsing();
        m_database->getPage().startParsing(0, StcTime(), 0);

        {
            BitStreamWriter writer;

            writer.write(2, 8); // region_id
            writer.write((0xD << 4), 8); // version=D, fill_flag=0
            writer.write(300, 16); // width
            writer.write(200, 16); // height
            writer.write(
                    (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 5)
                            | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2),
                    8); // depth & compatibility
            writer.write(7, 8); // clut_id (DIFFERENT)
            writer.write(3, 8); // background 8bit
            writer.write(0, 8); // other backgrounds, reserved (unused)

            // parse (add region)
            {
                PesPacketReader reader(writer.data(), writer.size(), nullptr,
                        0);
                CPPUNIT_ASSERT_THROW(
                        ParserRCS().parseRegionCompositionSegment(*m_database,
                                reader), ParserException);
            }
        }
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserRCSTest);
