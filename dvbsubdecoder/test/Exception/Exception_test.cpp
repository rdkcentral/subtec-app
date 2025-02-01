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

#include "ParserException.hpp"
#include "PesPacketReader.hpp"

using dvbsubdecoder::ParserException;
using dvbsubdecoder::PesPacketReader;

class ExceptionTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ExceptionTest );
    CPPUNIT_TEST(testParserException);
    CPPUNIT_TEST(testPesPacketReaderException);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void testParserException()
    {
        const std::string message = "exception message";

        try
        {
            throw ParserException(message.c_str());
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }

        try
        {
            throw ParserException(nullptr);
        }
        catch (const ParserException& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string(), std::string(e.what()));
        }
    }

    void testPesPacketReaderException()
    {
        const std::string message = "exception message";

        try
        {
            throw PesPacketReader::Exception(message.c_str());
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(message, std::string(e.what()));
        }

        try
        {
            throw PesPacketReader::Exception(nullptr);
        }
        catch (const PesPacketReader::Exception& e)
        {
            CPPUNIT_ASSERT_EQUAL(std::string(), std::string(e.what()));
        }
    }
}
;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ExceptionTest);
