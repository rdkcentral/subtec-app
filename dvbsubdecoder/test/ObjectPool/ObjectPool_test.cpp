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

#include "ObjectPool.hpp"

using dvbsubdecoder::ObjectPool;

class ObjectPoolTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ObjectPoolTest );
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
        const std::size_t POOL_SIZE = 32;

        ObjectPool<PoolItem, POOL_SIZE> pool;

        {
            std::list<PoolItem*> objectsList;

            // alloc all
            for (auto i = 0;; ++i)
            {
                auto object = pool.alloc();
                if (object)
                {
                    objectsList.push_back(object);
                }
                else
                {
                    break;
                }
            }

            // check all allocated
            CPPUNIT_ASSERT(objectsList.size() == POOL_SIZE);

            // free all (in reverse order)
            while (objectsList.size() > 0)
            {
                pool.release(objectsList.back());
                objectsList.pop_back();
            }

            // alloc all again
            for (auto i = 0; i < POOL_SIZE; ++i)
            {
                auto object = pool.alloc();
                CPPUNIT_ASSERT(object);
                objectsList.push_back(object);
            }

            // free all (in alloc order)
            while (objectsList.size() > 0)
            {
                pool.release(objectsList.front());
                objectsList.pop_front();
            }
        }

        {
            std::vector<PoolItem*> objectsVector;

            // alloc half
            for (auto i = 0; i < POOL_SIZE / 2; ++i)
            {
                auto object = pool.alloc();

                CPPUNIT_ASSERT(object);
                objectsVector.push_back(object);
            }

            // free half of allocated
            for (int i = objectsVector.size() - 1; i >= 0; i -= 2)
            {
                auto iter = objectsVector.begin() + i;
                auto object = *iter;
                pool.release(object);

                objectsVector.erase(iter);
            }

            // alloc half
            for (auto i = 0; i < POOL_SIZE / 2; ++i)
            {
                auto object = pool.alloc();

                CPPUNIT_ASSERT(object);
                objectsVector.push_back(object);
            }

            // free all
            while (objectsVector.size() > 0)
            {
                auto object = objectsVector.back();
                pool.release(object);
                objectsVector.pop_back();
            }
        }
    }

private:
    class PoolItem
    {
    public:
        PoolItem() :
                m_id(-1)
        {
            // noop
        }

        int m_id;
    };
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectPoolTest);
