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

#include "ParserEDS.hpp"
#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "PixmapAllocator.hpp"

#include "DecoderClientMock.hpp"

using dvbsubdecoder::Database;
using dvbsubdecoder::Page;
using dvbsubdecoder::ParserEDS;
using dvbsubdecoder::PesPacketReader;
using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;
using dvbsubdecoder::StcTime;
using dvbsubdecoder::StcTimeType;

class ParserEDSTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ParserEDSTest );
    CPPUNIT_TEST(testSimple);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        m_client.reset(new DecoderClientMock());
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
        // no bytes are read from the reader
        PesPacketReader reader;

        ParserEDS parser;

        // verify page state
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        // invalid state, do nothing
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INVALID);

        // valid state, finish parsing
        m_database->getPage().startParsing(0, StcTime(), 0);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::INCOMPLETE);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // complete state, do nothing
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::COMPLETE);

        // timed out state, do nothing
        m_database->getPage().setTimedOut();
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::TIMEDOUT);
        parser.parseEndOfDisplaySetSegment(*m_database, reader);
        CPPUNIT_ASSERT(m_database->getPage().getState() == Page::State::TIMEDOUT);
    }

private:
    const Specification SPEC_VERSION = Specification::VERSION_1_3_1;

    std::unique_ptr<DecoderClientMock> m_client;
    std::unique_ptr<PixmapAllocator> m_pixmapAllocator;
    std::unique_ptr<Database> m_database;
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ParserEDSTest);
