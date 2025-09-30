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
    CPPUNIT_TEST(testVersionNumberBoundaries);
    CPPUNIT_TEST(testNonModifyingColorFlag);
    CPPUNIT_TEST(testNoMatchingObjects);
    CPPUNIT_TEST(testZeroLengthFields);
    CPPUNIT_TEST(testLargeFieldLengths);
    CPPUNIT_TEST(testAllCodingMethods);
    CPPUNIT_TEST(testInsufficientData);
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

    void testVersionNumberBoundaries()
    {
        const std::uint16_t OBJECT_ID = 1000;

        // Test minimum version (0)
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write(0x0 << 4, 8); // version=0, coding=0, nmcf=0, reserved=0
            writer.write(1, 16);
            writer.write(0, 16);
            writer.write(0xF0, 8);

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }

        // Test maximum version (15)
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write(0xF << 4, 8); // version=F, coding=0, nmcf=0, reserved=0
            writer.write(1, 16);
            writer.write(0, 16);
            writer.write(0xF0, 8);

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }
    }

    void testNonModifyingColorFlag()
    {
        const std::uint16_t OBJECT_ID = 1000;

        // Test NMCF = false (0)
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write(0xC << 4, 8); // version=C, coding=0, nmcf=0, reserved=0
            writer.write(1, 16);
            writer.write(0, 16);
            writer.write(0xF0, 8);

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }

        // Test NMCF = true (1)
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write((0xC << 4) | 0x02, 8); // version=C, coding=0, nmcf=1, reserved=0
            writer.write(1, 16);
            writer.write(0, 16);
            writer.write(0xF0, 8);

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }
    }

    void testNoMatchingObjects()
    {
        const std::uint16_t OBJECT_ID = 1000;
        const std::uint16_t DIFFERENT_OBJECT_ID = 2000;

        BitStreamWriter writer;
        writer.write(OBJECT_ID, 16);
        writer.write(0xC << 4, 8);
        writer.write(1, 16);
        writer.write(0, 16);
        writer.write(0xF0, 8);

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        CPPUNIT_ASSERT(
                m_database->addRegionAndClut(0, 100, 100,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
        auto region = m_database->getRegionByIndex(0);
        // Add object with different ID
        CPPUNIT_ASSERT(m_database->addRegionObject(region, DIFFERENT_OBJECT_ID, 0, 0));

        ObjectParserStub::resetCallCounter();
        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserODS().parseObjectDataSegment(*m_database, reader);

        CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
    }

    void testZeroLengthFields()
    {
        const std::uint16_t OBJECT_ID = 1000;

        // Test zero top field length
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write(0xC << 4, 8);
            writer.write(0, 16); // zero top field length
            writer.write(1, 16); // non-zero bottom field length
            writer.write(0xF0, 8); // bottom field data

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }

        // Test both fields zero length
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write(0xC << 4, 8);
            writer.write(0, 16); // zero top field length
            writer.write(0, 16); // zero bottom field length

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
        }
    }

    void testLargeFieldLengths()
    {
        const std::uint16_t OBJECT_ID = 1000;

        BitStreamWriter writer;
        writer.write(OBJECT_ID, 16);
        writer.write(0xC << 4, 8);
        writer.write(100, 16); // moderate top field length (not max to avoid excessive test data)
        writer.write(0, 16); // zero bottom field length

        // Add moderate amount of data
        for (int i = 0; i < 100; ++i)
        {
            writer.write(0xF0, 8);
        }

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        CPPUNIT_ASSERT(
                m_database->addRegionAndClut(0, 100, 100,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
        auto region = m_database->getRegionByIndex(0);
        CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

        ObjectParserStub::resetCallCounter();
        PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
        ParserODS().parseObjectDataSegment(*m_database, reader);

        CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
    }

    void testAllCodingMethods()
    {
        const std::uint16_t OBJECT_ID = 1000;

        m_database->epochReset();
        m_database->getPage().startParsing(0, StcTime(), 0);

        CPPUNIT_ASSERT(
                m_database->addRegionAndClut(0, 100, 100,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                        dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
        auto region = m_database->getRegionByIndex(0);
        CPPUNIT_ASSERT(m_database->addRegionObject(region, OBJECT_ID, 0, 0));

        // Test all possible coding method values (0-3)
        for (std::uint8_t coding = 0; coding <= 3; ++coding)
        {
            BitStreamWriter writer;
            writer.write(OBJECT_ID, 16);
            writer.write((0xC << 4) | (coding << 2), 8); // version=C, coding=coding, nmcf=0, reserved=0
            writer.write(1, 16);
            writer.write(0, 16);
            writer.write(0xF0, 8);

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            ParserODS().parseObjectDataSegment(*m_database, reader);

            if (coding == 0) // PIXELS coding method
            {
                CPPUNIT_ASSERT(ObjectParserStub::wasCalled());
            }
            else
            {
                CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
            }
        }
    }

    void testInsufficientData()
    {
        // Test with incomplete header data
        {
            BitStreamWriter writer;
            writer.write(0x1000, 16); // object id only, missing other fields

            m_database->epochReset();
            m_database->getPage().startParsing(0, StcTime(), 0);

            CPPUNIT_ASSERT(
                    m_database->addRegionAndClut(0, 100, 100,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT,
                            dvbsubdecoder::RegionDepthBits::DEPTH_8BIT, 0));
            auto region = m_database->getRegionByIndex(0);
            CPPUNIT_ASSERT(m_database->addRegionObject(region, 0x1000, 0, 0));

            ObjectParserStub::resetCallCounter();
            PesPacketReader reader(writer.data(), writer.size(), nullptr, 0);
            
            // This should handle insufficient data gracefully
            try
            {
                ParserODS().parseObjectDataSegment(*m_database, reader);
                CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
            }
            catch (...)
            {
                // Expected to throw on insufficient data
                CPPUNIT_ASSERT(!ObjectParserStub::wasCalled());
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
