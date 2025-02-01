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

#include <list>
#include <vector>

#include "ObjectTablePool.hpp"

using dvbsubdecoder::ObjectTablePool;

class ObjectTablePoolTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ObjectTablePoolTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testIdHandling);
    CPPUNIT_TEST(testErrors);
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

    void testSimple()
    {
        const std::size_t POOL_SIZE = 32;

        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;

        // add all
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(pool.canAdd());

            auto object = pool.add(i);
            CPPUNIT_ASSERT(object);
        }

        // check no more can be added
        CPPUNIT_ASSERT(!pool.canAdd());
        CPPUNIT_ASSERT(!pool.add(POOL_SIZE));

        // reset
        pool.reset();

        // add all
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(pool.canAdd());

            auto object = pool.add(i);
            CPPUNIT_ASSERT(object);
        }

        // check no more can be added
        CPPUNIT_ASSERT(!pool.canAdd());
        CPPUNIT_ASSERT(!pool.add(POOL_SIZE));
    }

    void testIdHandling()
    {
        const std::size_t POOL_SIZE = 32;

        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        std::vector<PoolItem*> objects;

        // add all
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(pool.canAdd());

            auto object = pool.add(i);
            CPPUNIT_ASSERT(object);
            objects.push_back(object);
        }

        // check no more can be added
        CPPUNIT_ASSERT(!pool.canAdd());
        CPPUNIT_ASSERT(!pool.add(POOL_SIZE));

        // test ids
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(objects[i]->getId() == i);
        }

        // reset
        pool.reset();
        objects.clear();

        // add all
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(pool.canAdd());

            auto object = pool.add(i * 2);
            CPPUNIT_ASSERT(object);
            objects.push_back(object);
        }


        // check no more can be added
        CPPUNIT_ASSERT(!pool.canAdd());
        CPPUNIT_ASSERT(!pool.add(POOL_SIZE * 2));

        // test ids
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(objects[i]->getId() == i * 2);
        }

        // test get by id & by index
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            CPPUNIT_ASSERT(pool.getById(i * 2) == objects[i]);
            CPPUNIT_ASSERT(pool.getByIndex(i) == objects[i]);
        }
    }

    void testErrors()
    {
        const std::size_t POOL_SIZE = 32;

        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;

        CPPUNIT_ASSERT_THROW(pool.getByIndex(0), std::range_error);

        pool.add(0);
        pool.add(1);

        CPPUNIT_ASSERT_THROW(pool.getByIndex(10), std::range_error);

        CPPUNIT_ASSERT_THROW(pool.add(0), std::logic_error);
        CPPUNIT_ASSERT_THROW(pool.add(1), std::logic_error);
    }

private:
    typedef std::uint8_t IdType;

    class PoolItem
    {
    public:
        PoolItem() :
                m_id(0),
                m_idValid(false)
        {
            // false
        }

        void reset()
        {
            m_idValid = false;
        }

        void setId(IdType id)
        {
            m_id = id;
            m_idValid = true;
        }

        IdType getId() const
        {
            CPPUNIT_ASSERT(m_idValid);
            return m_id;
        }

    private:
        IdType m_id;
        bool m_idValid;
    };
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectTablePoolTest);
