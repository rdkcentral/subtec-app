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

#include "ParserODS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"
#include "ParserException.hpp"

#include "DecoderClientMock.hpp"
#include "BitStreamWriter.hpp"
#include "Misc.hpp"
#include "ObjectParserStub.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserODS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::ParserException;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserODSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserODSTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testBadCoding);
    CPPUNIT_TEST(testDepth);
    CPPUNIT_TEST_SUITE_END()
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
        const std::uint16_t OBJECT_ID = 1000;

        BitStreamWriter writer;

        writer.write(OBJECT_ID, 16); // object id
        writer.write(0xC << 4, 8); // version=C, coding=0, nmcf=0, reserved=0
        writer.write(1, 16); // top field length
        writer.write(0, 16); // bottom field length
        writer.write(0xF0, 8); // end of line

        // cannot parse, page not started
        {
            ObjectParserStub::resetCallCounter();

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
        }

        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        // add region
        CPPUNIT_ASSERT(
                m_database->addRegionAndClut(0, 100, 100,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
        auto region = m_database->getRegionByIndex(0);
        CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

        // parse, must succeed
        {
            ObjectParserStub::resetCallCounter();

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }

        // restart parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        // cannot parse, no region refers to object
        {
            ObjectParserStub::resetCallCounter();

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
        }
    }

    void testBadCoding()
    {
        const std::uint16_t REGION_ID = 1000;

        BitStreamWriter writer;

        writer.write(REGION_ID, 16); // object id
        writer.write((0xC << 4) | (0x1 << 2), 8); // version=C, coding=1, nmcf=0, reserved=0
        writer.write(1, 16); // top field length
        writer.write(0, 16); // bottom field length
        writer.write(0xF0, 8); // end of line

        // start parsing
        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        // add region
        CPPUNIT_ASSERT(
                m_database->addRegionAndClut(0, 100, 100,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
        auto region = m_database->getRegionByIndex(0);
        CPPUNIT_ASSERT(m_database->addRegionObject(region, REGION_ID, 0, 0));

        // parse, failure on coding expected
        {
            ObjectParserStub::resetCallCounter();

            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
        }
    }

    void testDepth()
    {
        const std::uint16_t REGION_ID = 1000;

        BitStreamWriter writer;

        writer.write(REGION_ID, 16); // object id
        writer.write((0xC << 4) | (0x1 << 1), 8); // version=C, coding=0, nmcf=1, reserved=0
        writer.write(1, 16); // top field length
        writer.write(1, 16); // bottom field length
        writer.write(0xF0, 8); // end of line (top)
        writer.write(0xF0, 8); // end of line (bottom)

        {
            // start parsing
            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            // add region
            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, REGION_ID, 0, 0));

            // parse, failure on coding expected
            {
                ObjectParserStub::resetCallCounter();

                PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
                ParserODS().parseObjectDataSegment(*m_database, reader);

                CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
            }
        }

        {
            // start parsing
            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            // add region
            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_4BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_4BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, REGION_ID, 0, 0));

            // parse, failure on coding expected
            {
                ObjectParserStub::resetCallCounter();

                PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
                ParserODS().parseObjectDataSegment(*m_database, reader);

                CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
            }
        }

        {
            // start parsing
            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            // add region
            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_2BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_2BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, REGION_ID, 0, 0));

            // parse, failure on coding expected
            {
                ObjectParserStub::resetCallCounter();

                PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
                ParserODS().parseObjectDataSegment(*m_database, reader);

                CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
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
CPPUNIT_TEST_SUITE_REGISTRATION(ParserODSTest);
