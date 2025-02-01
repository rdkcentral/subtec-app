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

#include "Clut.hpp"
#include "Consts.hpp"

using dvbsubdecoder::Clut;
using dvbsubdecoder::INVALID_VERSION;

class ClutTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ClutTest );
    CPPUNIT_TEST(testIdVersion);
    CPPUNIT_TEST(testSetReset);
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

    void testIdVersion()
    {
        Clut clut;

        CPPUNIT_ASSERT(clut.getVersion() == INVALID_VERSION);

        clut.setId(10);
        clut.setVersion(20);

        CPPUNIT_ASSERT(clut.getId() == 10);
        CPPUNIT_ASSERT(clut.getVersion() == 20);

        clut.reset();

        CPPUNIT_ASSERT(clut.getId() == 10);
        CPPUNIT_ASSERT(clut.getVersion() == INVALID_VERSION);

        clut.setId(40);
        clut.setVersion(50);

        CPPUNIT_ASSERT(clut.getId() == 40);
        CPPUNIT_ASSERT(clut.getVersion() == 50);
    }

    void testSetReset()
    {
        const Clut defaultClut;
        Clut testClut;

        // check initial state
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto defaultArray = defaultClut.getArray2bit();
            auto testArray = testClut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto defaultArray = defaultClut.getArray4bit();
            auto testArray = testClut.getArray4bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto defaultArray = defaultClut.getArray8bit();
            auto testArray = testClut.getArray8bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }

        // set
        for (int i = 0; i < (1 << 2); ++i)
        {
            std::uint32_t value = (i << 24) || (i << 16) || i;
            testClut.set2bit(i, value);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            std::uint32_t value = (i << 24) || (i << 16) || i;
            testClut.set4bit(i, value);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            std::uint32_t value = (i << 24) || (i << 16) || i;
            testClut.set8bit(i, value);
        }

        // test set
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto testArray = testClut.getArray2bit();
            std::uint32_t value = (i << 24) || (i << 16) || i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto testArray = testClut.getArray4bit();
            std::uint32_t value = (i << 24) || (i << 16) || i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto testArray = testClut.getArray8bit();
            std::uint32_t value = (i << 24) || (i << 16) || i;
            CPPUNIT_ASSERT_EQUAL(value, testArray[i]);
        }

        testClut.reset();

        // check after reset state
        for (int i = 0; i < (1 << 2); ++i)
        {
            auto defaultArray = defaultClut.getArray2bit();
            auto testArray = testClut.getArray2bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 4); ++i)
        {
            auto defaultArray = defaultClut.getArray4bit();
            auto testArray = testClut.getArray4bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
        for (int i = 0; i < (1 << 8); ++i)
        {
            auto defaultArray = defaultClut.getArray8bit();
            auto testArray = testClut.getArray8bit();
            CPPUNIT_ASSERT_EQUAL(defaultArray[i], testArray[i]);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ClutTest);
