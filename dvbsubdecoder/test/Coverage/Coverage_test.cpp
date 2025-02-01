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

#include <cassert>

#include "DynamicAllocator.hpp"
#include "DecoderFactory.hpp"
#include "Decoder.hpp"
#include "DecoderClientMock.hpp"

class CoverageTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( CoverageTest );
    CPPUNIT_TEST(testFactory);CPPUNIT_TEST_SUITE_END()
    ;

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testFactory()
    {
        dvbsubdecoder::DynamicAllocator allocator;
        DecoderClientMock client;

        auto decoder = dvbsubdecoder::DecoderFactory::create(
                dvbsubdecoder::Specification::VERSION_1_3_1, allocator, client,
                client);

        // verify it does not crash
        decoder->setPageIds(0, 1);
        decoder->start();
        decoder->stop();
        decoder->invalidate();
        CPPUNIT_ASSERT(decoder->addPesPacket(nullptr, 0) == false);
        decoder->process();
        decoder->draw();
        decoder->draw();

        decoder->reset();

        decoder->setPageIds(0, 1);
        decoder->start();
        decoder->stop();
        decoder->invalidate();
        CPPUNIT_ASSERT(decoder->addPesPacket(nullptr, 0) == false);
        decoder->process();
        decoder->draw();
        decoder->draw();

        decoder->reset();
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(CoverageTest);
