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

#include <array>

#include "ObjectInstance.hpp"

using dvbsubdecoder::ObjectInstance;
using dvbsubdecoder::ObjectInstanceList;

class ObjectInstanceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ObjectInstanceTest );
    CPPUNIT_TEST(testFunctionality);
    CPPUNIT_TEST(testErrors);CPPUNIT_TEST_SUITE_END()
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

    void testFunctionality()
    {
        const int OBJECT_COUNT = 64;

        std::array<ObjectInstance, OBJECT_COUNT> pool;

        ObjectInstanceList list;

        // add all
        for (auto i = 0; i < OBJECT_COUNT; ++i)
        {
            list.add(&pool[i]);
        }

        // remove half
        for (auto i = 0; i < OBJECT_COUNT / 2; ++i)
        {
            CPPUNIT_ASSERT(list.removeFirst() == &pool[i]);
        }

        // iterate
        auto item = list.getFirst();
        for (auto i = OBJECT_COUNT / 2; i < OBJECT_COUNT; ++i)
        {
            CPPUNIT_ASSERT(item == &pool[i]);
            item = list.getNext(item);
        }
        CPPUNIT_ASSERT(!item);

        // add the half back
        for (auto i = 0; i < OBJECT_COUNT / 2; ++i)
        {
            list.add(&pool[i]);
        }

        // iterate all
        for (item = list.getFirst(); item; item = list.getNext(item))
        {
            // noop
        }

        // remove all
        while (list.getFirst())
        {
            CPPUNIT_ASSERT(list.removeFirst());
        }
        CPPUNIT_ASSERT(list.removeFirst() == nullptr);
    }

    void testErrors()
    {
        const int OBJECT_COUNT = 64;

        std::array<ObjectInstance, OBJECT_COUNT> pool;

        ObjectInstanceList list;

        // add all
        for (auto i = 0; i < OBJECT_COUNT; ++i)
        {
            list.add(&pool[i]);
        }

        CPPUNIT_ASSERT_THROW(list.add(nullptr), std::logic_error);
        CPPUNIT_ASSERT_THROW(list.add(&pool[0]), std::logic_error);

        list.removeFirst();
        CPPUNIT_ASSERT_THROW(list.getNext(&pool[0]), std::logic_error);

        CPPUNIT_ASSERT_THROW(list.getNext(nullptr), std::logic_error);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectInstanceTest);
