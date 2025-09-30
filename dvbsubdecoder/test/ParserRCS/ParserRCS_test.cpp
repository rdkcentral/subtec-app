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
    CPPUNIT_TEST(testBadUpdate);
    CPPUNIT_TEST(testDepth2BitBackground);
    CPPUNIT_TEST(testDepth4BitBackground);
    CPPUNIT_TEST(testFillDepth2Bit);
    CPPUNIT_TEST(testFillDepth4Bit);
    CPPUNIT_TEST(testDimensionBoundaryValues);
    CPPUNIT_TEST(testZeroSizedRegion);
    CPPUNIT_TEST(testDatabaseCapacityLimit);
    CPPUNIT_TEST(testSameVersionPreservesObjects);
    CPPUNIT_TEST(testObjectListReplacement);
    CPPUNIT_TEST(testParameterChangeWidth);
    CPPUNIT_TEST(testParameterChangeHeight);
    CPPUNIT_TEST(testParameterChangeDepth);
    CPPUNIT_TEST(testParameterChangeCompatibility);
    CPPUNIT_TEST(testPageStateComplete);
    CPPUNIT_TEST(testPageStateTimedOut);
    CPPUNIT_TEST(testTruncatedHeader);
    CPPUNIT_TEST(testTruncatedObjectData);
    CPPUNIT_TEST(testCompositeStringObjectType);
    CPPUNIT_TEST(testUnsupportedProviderFlag);
    CPPUNIT_TEST(testMaxObjectCapacityOverflow);
    CPPUNIT_TEST(testObjectPositionBoundaries);
    CPPUNIT_TEST(testObjectIdBoundaries);
    CPPUNIT_TEST(testVersionNumberBoundaries);
    CPPUNIT_TEST(testMultipleRegionsVersionUpdate);
    CPPUNIT_TEST(testFillFlagToggle);
    CPPUNIT_TEST(testVersionRegression);
    CPPUNIT_TEST(testInvalidDepthBranch);
    CPPUNIT_TEST(testBothInvalidParameters);
    CPPUNIT_TEST(testRegionAdditionFailure);
    CPPUNIT_TEST(testTruncatedCharacterColors);
    CPPUNIT_TEST(testVersionSequenceProgression);
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

    void testDepth2BitBackground()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x01, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, // compatibility
                dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, // depth
                0x10, 0xAA, 0x0C, 0x02); // bg8, bg4, bg2(=2)

        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, reader);
        auto region = m_database->getRegionById(0x01);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(region->getBackgroundIndex() == 0x02);
    }

    void testDepth4BitBackground()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x02, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                0x11, 0xAB, 0x0D, 0x03); // bg4=0xD
        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, reader);
        auto region = m_database->getRegionById(0x02);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(region->getBackgroundIndex() == 0x0D);
    }
    void testFillDepth2Bit()
    {
        // Validate background fill for 2-bit depth separately (isolated scenario)
        prepareParsing();
        auto writer = buildRegionHeader(0x03, 1, true, 8, 4,
                        dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                        0x01, 0x10, 0x05, 0x01); // bg2=1
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x03);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(region->getPixmap().getBuffer()[0] == 0x01);
    }

    void testFillDepth4Bit()
    {
        // Validate background fill for 4-bit depth separately
        prepareParsing();
        auto writer = buildRegionHeader(0x04, 1, true, 8, 4,
                        dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                        0x02, 0x22, 0x07, 0x01); // bg4=0x7
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x04);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(region->getPixmap().getBuffer()[0] == 0x07);
    }

    void testDimensionBoundaryValues()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x05, 2, false, 1, 1,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x05, 0x33, 0x0A, 0x01);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x05);
        CPPUNIT_ASSERT(region);
        CPPUNIT_ASSERT(region->getWidth() == 1);
        CPPUNIT_ASSERT(region->getHeight() == 1);
    }

    void testZeroSizedRegion()
    {
        prepareParsing();
        BitStreamWriter w;
        w.write(0x06, 8); // id
        w.write(0x10, 8); // version=1
        w.write(0, 16); // width invalid
        w.write(50, 16); // height
        w.write(((dvbsubdecoder::RegionDepthBits::DEPTH_8BIT) << 5)
                | (dvbsubdecoder::RegionDepthBits::DEPTH_8BIT << 2), 8);
        w.write(1, 8); // clut
        w.write(0, 8); // bg8
        w.write(0, 8); // bg flags
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        CPPUNIT_ASSERT(m_database->getRegionById(0x06) == nullptr);
    }

    void testDatabaseCapacityLimit()
    {
        prepareParsing();
        for (std::uint8_t id = 0; id < dvbsubdecoder::MAX_SUPPORTED_REGIONS; ++id)
        {
            auto writer = buildRegionHeader(id, 1, false, 2, 2,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    id, 0, 0, 0);
            PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, r);
        }
        CPPUNIT_ASSERT(m_database->getRegionCount() == dvbsubdecoder::MAX_SUPPORTED_REGIONS);
        // attempt one more
        auto writer2 = buildRegionHeader(0xFE, 1, false, 2, 2,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        CPPUNIT_ASSERT(m_database->getRegionById(0xFE) == nullptr);
    }

    void testSameVersionPreservesObjects()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x07, 3, false, 40, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        addBitmapObject(writer, 0x1111, 5, 6);
        {
            PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, r);
        }
        // second parse same version should skip
        PesPacketReader r2(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        auto region = m_database->getRegionById(0x07);
        CPPUNIT_ASSERT(region);
        auto obj = region->getFirstObject();
        CPPUNIT_ASSERT(obj && obj->m_objectId == 0x1111);
        CPPUNIT_ASSERT(region->getVersion() == 3);
    }

    void testObjectListReplacement()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x08, 4, false, 40, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        addBitmapObject(writer, 0x2000, 0, 0);
        addBitmapObject(writer, 0x2001, 1, 1);
        {
            PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, r);
        }
        // new version with different objects
        auto writer2 = buildRegionHeader(0x08, 5, false, 40, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        addBitmapObject(writer2, 0x3000, 2, 2);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        auto region = m_database->getRegionById(0x08);
        CPPUNIT_ASSERT(region->getVersion() == 5);
        auto obj = region->getFirstObject();
        CPPUNIT_ASSERT(obj && obj->m_objectId == 0x3000);
        CPPUNIT_ASSERT(region->getNextObject(obj) == nullptr);
    }

    void testParameterChangeWidth()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x09, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        {
            PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, r);
        }
        auto writer2 = buildRegionHeader(0x09, 2, false, 30, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r2), ParserException);
    }

    void testParameterChangeHeight()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x0A, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto writer2 = buildRegionHeader(0x0A, 2, false, 20, 11,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r2), ParserException);
    }

    void testParameterChangeDepth()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x0B, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                0x01, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto writer2 = buildRegionHeader(0x0B, 2, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r2), ParserException);
    }

    void testParameterChangeCompatibility()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x0C, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto writer2 = buildRegionHeader(0x0C, 2, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r2), ParserException);
    }

    void testPageStateComplete()
    {
        prepareParsing();
        m_database->getPage().finishParsing();
        auto writer = buildRegionHeader(0x0D, 1, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        CPPUNIT_ASSERT(m_database->getRegionById(0x0D) == nullptr);
    }

    void testPageStateTimedOut()
    {
        prepareParsing();
        m_database->getPage().finishParsing();
        m_database->getPage().setTimedOut();
        auto writer = buildRegionHeader(0x0E, 1, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        CPPUNIT_ASSERT(m_database->getRegionById(0x0E) == nullptr);
    }

    void testTruncatedHeader()
    {
        prepareParsing();
        BitStreamWriter w;
        w.write(0x0F, 8); // region id only (insufficient header)
        // Missing remaining bytes -> expect reader exception
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), PesPacketReader::Exception);
    }

    void testTruncatedObjectData()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x10, 1, false, 20, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                0x01, 0, 0, 0);
        // start object but truncate after 4 bytes instead of 6
        writer.write(0x12, 8); // partial object id high byte (corrupt)
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), PesPacketReader::Exception);
    }

    void testCompositeStringObjectType()
    {
        prepareParsing();
        BitStreamWriter w = buildRegionHeader(0x11, 1, false, 30, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        // object with composite string type -> unsupported
        w.write(0x7777, 16);
        std::uint16_t data1 = (dvbsubdecoder::RegionObjectTypeBits::COMPOSITE_STRING << 14)
                | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM << 12)
                | 0; // posX
        w.write(data1, 16);
        w.write(0, 16); // posY
        w.write(0, 8); // fg
        w.write(0, 8); // bg
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), ParserException);
    }

    void testUnsupportedProviderFlag()
    {
        prepareParsing();
        BitStreamWriter w = buildRegionHeader(0x12, 1, false, 30, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        w.write(0x1234, 16);
        std::uint16_t data1 = (dvbsubdecoder::RegionObjectTypeBits::BASIC_BITMAP << 14)
                | (0x02 << 12) // invalid provider flag
                | 0;
        w.write(data1, 16);
        w.write(0, 16);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), ParserException);
    }

    void testMaxObjectCapacityOverflow()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x13, 1, false, 20, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        for (int i = 0; i < static_cast<int>(dvbsubdecoder::MAX_SUPPORTED_OBJECTS) + 1; ++i)
        {
            addBitmapObject(writer, 0x4000 + i, i % 100, i % 50);
        }
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), ParserException);
    }

    void testObjectPositionBoundaries()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x14, 1, false, 50, 50,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        addBitmapObject(writer, 0x5000, 0, 0);
        addBitmapObject(writer, 0x5001, 4095, 4095);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x14);
        auto obj1 = region->getFirstObject();
        auto obj2 = region->getNextObject(obj1);
        CPPUNIT_ASSERT(obj1->m_positionX == 0 && obj1->m_positionY == 0);
        CPPUNIT_ASSERT(obj2->m_positionX == 4095 && obj2->m_positionY == 4095);
    }

    void testObjectIdBoundaries()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x15, 1, false, 20, 20,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        addBitmapObject(writer, 0x0000, 1, 1);
        addBitmapObject(writer, 0xFFFF, 2, 2);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x15);
        auto o1 = region->getFirstObject();
        auto o2 = region->getNextObject(o1);
        CPPUNIT_ASSERT(o1->m_objectId == 0x0000);
        CPPUNIT_ASSERT(o2->m_objectId == 0xFFFF);
    }

    void testVersionNumberBoundaries()
    {
        prepareParsing();
        auto writer = buildRegionHeader(0x16, 0, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r(writer.data(), writer.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto writer2 = buildRegionHeader(0x16, 0x0F, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r2(writer2.data(), writer2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        CPPUNIT_ASSERT(m_database->getRegionById(0x16)->getVersion() == 0x0F);
    }

    void testMultipleRegionsVersionUpdate()
    {
        prepareParsing();
        auto w1 = buildRegionHeader(0x17, 1, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        auto w2 = buildRegionHeader(0x18, 2, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                2, 0, 0, 0);
        PesPacketReader r1(w1.data(), w1.size(), nullptr, 0);
        PesPacketReader r2(w2.data(), w2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r1);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        // update only 0x18
        auto w2u = buildRegionHeader(0x18, 3, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                2, 0, 0, 0);
        PesPacketReader r2u(w2u.data(), w2u.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2u);
        CPPUNIT_ASSERT(m_database->getRegionById(0x17)->getVersion() == 1);
        CPPUNIT_ASSERT(m_database->getRegionById(0x18)->getVersion() == 3);
    }

    void testFillFlagToggle()
    {
        prepareParsing();
        auto w = buildRegionHeader(0x19, 1, false, 8, 4,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0x22, 0, 0);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto region = m_database->getRegionById(0x19);
        // fill region with non-background value
        region->getPixmap().clear(0x7F);
        auto w2 = buildRegionHeader(0x19, 2, true, 8, 4,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0x33, 0, 0); // bg8=0x33
        PesPacketReader r2(w2.data(), w2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        CPPUNIT_ASSERT(region->getPixmap().getBuffer()[0] == 0x33);
    }

    void testVersionRegression()
    {
        prepareParsing();
        auto w = buildRegionHeader(0x1A, 10, false, 8, 8,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                1, 0, 0, 0);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        auto w2 = buildRegionHeader(0x1A, 3, false, 8, 8,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                1, 0, 0, 0);
        PesPacketReader r2(w2.data(), w2.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r2);
        // Rationale: parser explicitly allows lower version numbers (no monotonic constraint enforced), so
        // this verifies behavior rather than spec expectation (documented for clarity).
        CPPUNIT_ASSERT(m_database->getRegionById(0x1A)->getVersion() == 3);
    }

    void testInvalidDepthBranch()
    {
        prepareParsing();
        // depth code 0 invalid
        BitStreamWriter w;
        w.write(0x1B, 8);
        w.write(0x10, 8); // version 1
        w.write(10, 16);
        w.write(10, 16);
        w.write(((dvbsubdecoder::RegionDepthBits::DEPTH_2BIT) << 5) | (0 << 2), 8); // compatibility valid, depth invalid
        w.write(1, 8);
        w.write(0, 8);
        w.write(0, 8);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), ParserException);
    }

    void testBothInvalidParameters()
    {
        prepareParsing();
        BitStreamWriter w;
        w.write(0x1C, 8);
        w.write(0x10, 8);
        w.write(10, 16);
        w.write(10, 16);
        w.write(((0) << 5) | (0 << 2), 8); // invalid compatibility & depth
        w.write(1, 8);
        w.write(0, 8);
        w.write(0, 8);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), ParserException);
    }

    void testRegionAdditionFailure()
    {
        prepareParsing();
        // make database stop allowing new regions
        m_database->commitPage(); // sets m_isEpochStart=false
        auto w = buildRegionHeader(0x1D, 1, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        ParserRCS().parseRegionCompositionSegment(*m_database, r);
        CPPUNIT_ASSERT(m_database->getRegionById(0x1D) == nullptr);
    }

    void testTruncatedCharacterColors()
    {
        prepareParsing();
        BitStreamWriter w = buildRegionHeader(0x1E, 1, false, 10, 10,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                1, 0, 0, 0);
        // BASIC_CHARACTER object (needs 2 extra bytes) but provide only 1
        w.write(0x2222, 16);
        std::uint16_t data1 = (dvbsubdecoder::RegionObjectTypeBits::BASIC_CHARACTER << 14)
                | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM << 12)
                | 5; // posX
        w.write(data1, 16);
        w.write(6, 16); // posY
        w.write(0x01, 8); // only fg color, missing bg -> truncated
        PesPacketReader r(w.data(), w.size(), nullptr, 0);
        CPPUNIT_ASSERT_THROW(ParserRCS().parseRegionCompositionSegment(*m_database, r), PesPacketReader::Exception);
    }

    void testVersionSequenceProgression()
    {
        prepareParsing();
        for (int v = 0; v < 6; ++v)
        {
            auto w = buildRegionHeader(0x1F, v, false, 12, 12,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                    1, 0, 0, 0);
            PesPacketReader r(w.data(), w.size(), nullptr, 0);
            ParserRCS().parseRegionCompositionSegment(*m_database, r);
        }
        CPPUNIT_ASSERT(m_database->getRegionById(0x1F)->getVersion() == 5);
    }

private:
        void prepareParsing()
        {
                m_database->epochReset();
                m_database->getPage().startParsing(0, StcTime(), 0);
        }

        BitStreamWriter buildRegionHeader(std::uint8_t regionId,
                                                std::uint8_t version,
                                                bool fill,
                                                std::uint16_t width,
                                                std::uint16_t height,
                                                std::uint8_t compatibility,
                                                std::uint8_t depth,
                                                std::uint8_t clut,
                                                std::uint8_t bg8,
                                                std::uint8_t bg4,
                                                std::uint8_t bg2) const
        {
                BitStreamWriter w;
                w.write(regionId, 8);
                w.write((version << 4) | (fill ? 0x08 : 0x00), 8);
                w.write(width, 16);
                w.write(height, 16);
                w.write(((compatibility & 0x07) << 5) | ((depth & 0x03) << 2), 8);
                w.write(clut, 8);
                w.write(bg8, 8);
                w.write(((bg4 & 0x0F) << 4) | ((bg2 & 0x03) << 2), 8); // bg4 bits 4..7, bg2 bits 2..3
                return w;
        }

        void addBitmapObject(BitStreamWriter& w,
                                std::uint16_t objectId,
                                std::uint16_t posX,
                                std::uint16_t posY)
        {
                w.write(objectId, 16);
                std::uint16_t data1 = (dvbsubdecoder::RegionObjectTypeBits::BASIC_BITMAP << 14)
                                | (dvbsubdecoder::RegionObjectProviderBits::SUBTITLING_STREAM << 12)
                                | (posX & 0x0FFF);
                std::uint16_t data2 = (posY & 0x0FFF);
                w.write(data1, 16);
                w.write(data2, 16);
        }

    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserRCSTest);
