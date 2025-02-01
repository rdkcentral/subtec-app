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

#include "Array.hpp"

class ArrayTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ArrayTest );
    CPPUNIT_TEST(testSimple);CPPUNIT_TEST_SUITE_END()
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

    void testSimple()
    {
        const std::size_t ARRAY_SIZE = 32;

        dvbsubdecoder::SizedArray<std::uint16_t, ARRAY_SIZE> array;
        dvbsubdecoder::Array<std::uint16_t>& baseArray = array;
        const dvbsubdecoder::SizedArray<std::uint16_t, ARRAY_SIZE>& constArray =
                array;
        const dvbsubdecoder::Array<std::uint16_t>& constBaseArray = array;

        CPPUNIT_ASSERT(array.getData());
        CPPUNIT_ASSERT(array.getData() == baseArray.getData());
        CPPUNIT_ASSERT(array.getData() == constArray.getData());
        CPPUNIT_ASSERT(array.getData() == constBaseArray.getData());

        CPPUNIT_ASSERT(array.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(baseArray.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(constArray.getSize() == ARRAY_SIZE);
        CPPUNIT_ASSERT(constBaseArray.getSize() == ARRAY_SIZE);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ArrayTest);
