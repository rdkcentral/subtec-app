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

#include "IniFile.hpp"

using subttxrend::common::IniFile;

class StreamutilsTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StreamutilsTest );
    CPPUNIT_TEST(testValidFile);
    CPPUNIT_TEST(testValidAppend);
    CPPUNIT_TEST(testBadLine);
    CPPUNIT_TEST(testArray);
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

    void testValidFile()
    {
        IniFile iniFile;

        CPPUNIT_ASSERT(iniFile.parse("test1.ini"));

        for (int i = 1; i <= 3; ++i)
        {
            CPPUNIT_ASSERT(iniFile.get("key1") == "value1");
            CPPUNIT_ASSERT(iniFile.get("key1", "default") == "value1");
            CPPUNIT_ASSERT(iniFile.get("key2") == "ala ma kota");
            CPPUNIT_ASSERT(iniFile.get("key3") == "ala ma kota");

            CPPUNIT_ASSERT(iniFile.getInt("key3") == -1);

            CPPUNIT_ASSERT(iniFile.getInt("keyInt1") == -16);
            CPPUNIT_ASSERT(iniFile.getInt("keyInt1", 5) == -16);
            CPPUNIT_ASSERT(iniFile.getInt("keyInt2") == 254);
            CPPUNIT_ASSERT(iniFile.getInt("keyInt3") == -(7 * 8 + 2 * 1));

            CPPUNIT_ASSERT(iniFile.get("keyOther", "zombie") == "zombie");
            CPPUNIT_ASSERT(iniFile.getInt("keyOther", -5) == -5);
        }
    }

    void testValidAppend()
    {
        IniFile iniFile;

        CPPUNIT_ASSERT(iniFile.parse("test1.ini"));

        CPPUNIT_ASSERT(iniFile.get("key1") == "value1");
        CPPUNIT_ASSERT(iniFile.get("key1", "default") == "value1");
        CPPUNIT_ASSERT(iniFile.get("key2") == "ala ma kota");
        CPPUNIT_ASSERT(iniFile.get("key3") == "ala ma kota");

        CPPUNIT_ASSERT(iniFile.getInt("key3") == -1);

        CPPUNIT_ASSERT(iniFile.getInt("keyInt1") == -16);
        CPPUNIT_ASSERT(iniFile.getInt("keyInt1", 5) == -16);
        CPPUNIT_ASSERT(iniFile.getInt("keyInt2") == 254);
        CPPUNIT_ASSERT(iniFile.getInt("keyInt3") == -(7 * 8 + 2 * 1));

        CPPUNIT_ASSERT(iniFile.get("keyOther", "zombie") == "zombie");
        CPPUNIT_ASSERT(iniFile.getInt("keyOther", -5) == -5);

        CPPUNIT_ASSERT(iniFile.get("key_append_string") == "");
        CPPUNIT_ASSERT(iniFile.getInt("key_append_int", 111) == 111);

        CPPUNIT_ASSERT(iniFile.parseAppend("test1a.ini"));

        CPPUNIT_ASSERT(iniFile.get("key_append_string") == "Dona Ltd");
        CPPUNIT_ASSERT(iniFile.getInt("key_append_int") == 123);
    }

    void testBadLine()
    {
        IniFile iniFile;

        CPPUNIT_ASSERT(!iniFile.parse("test2.ini"));
        CPPUNIT_ASSERT(iniFile.get("key1") == "value1");
        CPPUNIT_ASSERT(iniFile.get("key2") == "value2");
        CPPUNIT_ASSERT(iniFile.get("key3", "") == "");
        CPPUNIT_ASSERT(iniFile.get("key3", "empty") == "empty");
    }

    void testArray()
    {
        IniFile iniFile;

        CPPUNIT_ASSERT(iniFile.parse("test3.ini"));

        auto data = iniFile.getArray("keySpace1", " ");
        CPPUNIT_ASSERT(data.size() == 3);
        CPPUNIT_ASSERT(data[0] == "ala");
        CPPUNIT_ASSERT(data[1] == "ma");
        CPPUNIT_ASSERT(data[2] == "kota");

        data = iniFile.getArray("keySpace2", " ");
        CPPUNIT_ASSERT(data.size() == 4);
        CPPUNIT_ASSERT(data[0] == "key1");
        CPPUNIT_ASSERT(data[1] == "key2");
        CPPUNIT_ASSERT(data[2] == "key3");
        CPPUNIT_ASSERT(data[3] == "key4");

        data = iniFile.getArray("keyComma", ",");
        CPPUNIT_ASSERT(data.size() == 4);
        CPPUNIT_ASSERT(data[0] == "a");
        CPPUNIT_ASSERT(data[1] == "b");
        CPPUNIT_ASSERT(data[2] == "c");
        CPPUNIT_ASSERT(data[3] == "d");

        data = iniFile.getArray("keyAB", "AB");
        CPPUNIT_ASSERT(data.size() == 3);
        CPPUNIT_ASSERT(data[0] == "tomek");
        CPPUNIT_ASSERT(data[1] == "tomekAtomekBmarta");
        CPPUNIT_ASSERT(data[2] == "marta");
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( StreamutilsTest );
