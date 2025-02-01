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

#include "StringUtils.hpp"

using subttxrend::common::StringUtils;

class StreamutilsTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StreamutilsTest );
    CPPUNIT_TEST(testTrimBegin);
    CPPUNIT_TEST(testTrimEnd);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST(testFormat);
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

    void testTrimBegin()
    {
        CPPUNIT_ASSERT(StringUtils::trimBegin("") == "");
        CPPUNIT_ASSERT(StringUtils::trimBegin("TestString") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trimBegin("TestString   ") == "TestString   ");
        CPPUNIT_ASSERT(StringUtils::trimBegin("   TestString") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trimBegin("   TestString   ") == "TestString   ");
        CPPUNIT_ASSERT(StringUtils::trimBegin("Test String") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trimBegin("Test String   ") == "Test String   ");
        CPPUNIT_ASSERT(StringUtils::trimBegin("   Test String") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trimBegin("   Test String   ") == "Test String   ");
    }

    void testTrimEnd()
    {
        CPPUNIT_ASSERT(StringUtils::trimEnd("") == "");
        CPPUNIT_ASSERT(StringUtils::trimEnd("TestString") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trimEnd("TestString   ") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trimEnd("   TestString") == "   TestString");
        CPPUNIT_ASSERT(StringUtils::trimEnd("   TestString   ") == "   TestString");
        CPPUNIT_ASSERT(StringUtils::trimEnd("Test String") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trimEnd("Test String   ") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trimEnd("   Test String") == "   Test String");
        CPPUNIT_ASSERT(StringUtils::trimEnd("   Test String   ") == "   Test String");
    }

    void testTrim()
    {
        CPPUNIT_ASSERT(StringUtils::trim("") == "");
        CPPUNIT_ASSERT(StringUtils::trim("TestString") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trim("TestString   ") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trim("   TestString") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trim("   TestString   ") == "TestString");
        CPPUNIT_ASSERT(StringUtils::trim("Test String") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trim("Test String   ") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trim("   Test String") == "Test String");
        CPPUNIT_ASSERT(StringUtils::trim("   Test String   ") == "Test String");
    }

    void testFormat()
    {
        CPPUNIT_ASSERT(StringUtils::format("%s", "Test") == "Test");
        CPPUNIT_ASSERT(StringUtils::format("AA %s AA", "Test") == "AA Test AA");
        CPPUNIT_ASSERT(StringUtils::format("%s - %d - %c", "Test", 123, 'z') == "Test - 123 - z");
        CPPUNIT_ASSERT(StringUtils::format("%04d", 56) == "0056");
        CPPUNIT_ASSERT(StringUtils::format("0x%03X", 129) == "0x081");
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( StreamutilsTest );
