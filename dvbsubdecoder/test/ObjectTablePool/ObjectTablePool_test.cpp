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
    CPPUNIT_TEST(testConstructorAndInitialState);
    CPPUNIT_TEST(testGetCountMethod);
    CPPUNIT_TEST(testGetByIdBoundaries);
    CPPUNIT_TEST(testCanAddStates);
    CPPUNIT_TEST(testBoundaryPoolSizes);
    CPPUNIT_TEST(testIdTypeBoundaries);
    CPPUNIT_TEST(testObjectStateLifecycle);
    CPPUNIT_TEST(testExceptionSafety);
    CPPUNIT_TEST(testMixedOperationPatterns);
    CPPUNIT_TEST(testDifferentTemplateTypes);
    CPPUNIT_TEST(testComplexScenarios);
    CPPUNIT_TEST(testHighFrequencyOperations);
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

    void testConstructorAndInitialState()
    {
        // Test constructor with different pool sizes
        {
            ObjectTablePool<PoolItem, IdType, 1> smallPool;
            CPPUNIT_ASSERT(smallPool.getCount() == 0);
            CPPUNIT_ASSERT(smallPool.canAdd() == true);
            CPPUNIT_ASSERT(smallPool.getById(0) == nullptr);
        }
        
        {
            ObjectTablePool<PoolItem, IdType, 5> mediumPool;
            CPPUNIT_ASSERT(mediumPool.getCount() == 0);
            CPPUNIT_ASSERT(mediumPool.canAdd() == true);
            CPPUNIT_ASSERT(mediumPool.getById(100) == nullptr);
        }
        
        {
            ObjectTablePool<PoolItem, IdType, 1000> largePool;
            CPPUNIT_ASSERT(largePool.getCount() == 0);
            CPPUNIT_ASSERT(largePool.canAdd() == true);
            CPPUNIT_ASSERT(largePool.getById(255) == nullptr);
        }
    }

    void testGetCountMethod()
    {
        const std::size_t POOL_SIZE = 5;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Initial count should be 0
        CPPUNIT_ASSERT(pool.getCount() == 0);
        
        // Add objects one by one and verify count
        for (std::size_t i = 0; i < POOL_SIZE; ++i)
        {
            auto obj = pool.add(static_cast<IdType>(i));
            CPPUNIT_ASSERT(obj != nullptr);
            CPPUNIT_ASSERT(pool.getCount() == i + 1);
        }
        
        // Count should remain at POOL_SIZE when add fails
        auto failedObj = pool.add(static_cast<IdType>(POOL_SIZE));
        CPPUNIT_ASSERT(failedObj == nullptr);
        CPPUNIT_ASSERT(pool.getCount() == POOL_SIZE);
        
        // After reset, count should be 0
        pool.reset();
        CPPUNIT_ASSERT(pool.getCount() == 0);
        
        // Add some objects again
        pool.add(10);
        pool.add(20);
        CPPUNIT_ASSERT(pool.getCount() == 2);
    }

    void testGetByIdBoundaries()
    {
        const std::size_t POOL_SIZE = 3;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Test with empty pool - all IDs should return nullptr
        CPPUNIT_ASSERT(pool.getById(0) == nullptr);
        CPPUNIT_ASSERT(pool.getById(1) == nullptr);
        CPPUNIT_ASSERT(pool.getById(255) == nullptr); // Max uint8_t value
        
        // Add some objects
        auto obj1 = pool.add(0);     // Min ID
        auto obj2 = pool.add(255);   // Max ID
        auto obj3 = pool.add(128);   // Mid-range ID
        
        CPPUNIT_ASSERT(obj1 != nullptr);
        CPPUNIT_ASSERT(obj2 != nullptr);
        CPPUNIT_ASSERT(obj3 != nullptr);
        
        // Verify retrieval by ID
        CPPUNIT_ASSERT(pool.getById(0) == obj1);
        CPPUNIT_ASSERT(pool.getById(255) == obj2);
        CPPUNIT_ASSERT(pool.getById(128) == obj3);
        
        // Test non-existent IDs
        CPPUNIT_ASSERT(pool.getById(1) == nullptr);
        CPPUNIT_ASSERT(pool.getById(127) == nullptr);
        CPPUNIT_ASSERT(pool.getById(129) == nullptr);
        CPPUNIT_ASSERT(pool.getById(254) == nullptr);
        
        // After reset, previously valid IDs should return nullptr
        pool.reset();
        CPPUNIT_ASSERT(pool.getById(0) == nullptr);
        CPPUNIT_ASSERT(pool.getById(255) == nullptr);
        CPPUNIT_ASSERT(pool.getById(128) == nullptr);
    }

    void testCanAddStates()
    {
        const std::size_t POOL_SIZE = 3;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Initial state - should be able to add
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        // Add objects and verify canAdd at each step
        auto obj1 = pool.add(1);
        CPPUNIT_ASSERT(obj1 != nullptr);
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        auto obj2 = pool.add(2);
        CPPUNIT_ASSERT(obj2 != nullptr);
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        auto obj3 = pool.add(3);
        CPPUNIT_ASSERT(obj3 != nullptr);
        CPPUNIT_ASSERT(pool.canAdd() == false); // Pool should be full
        
        // Multiple checks when full
        CPPUNIT_ASSERT(pool.canAdd() == false);
        CPPUNIT_ASSERT(pool.canAdd() == false);
        
        // After reset, should be able to add again
        pool.reset();
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        // Partial fill and verify
        pool.add(10);
        CPPUNIT_ASSERT(pool.canAdd() == true);
    }

    void testBoundaryPoolSizes()
    {
        // Test single object pool
        {
            ObjectTablePool<PoolItem, IdType, 1> singlePool;
            CPPUNIT_ASSERT(singlePool.getCount() == 0);
            CPPUNIT_ASSERT(singlePool.canAdd() == true);
            
            auto obj = singlePool.add(42);
            CPPUNIT_ASSERT(obj != nullptr);
            CPPUNIT_ASSERT(singlePool.getCount() == 1);
            CPPUNIT_ASSERT(singlePool.canAdd() == false);
            
            // Should not be able to add more
            CPPUNIT_ASSERT(singlePool.add(43) == nullptr);
            CPPUNIT_ASSERT(singlePool.getCount() == 1);
            
            // Verify retrieval
            CPPUNIT_ASSERT(singlePool.getById(42) == obj);
            CPPUNIT_ASSERT(singlePool.getByIndex(0) == obj);
            
            // Reset and verify
            singlePool.reset();
            CPPUNIT_ASSERT(singlePool.getCount() == 0);
            CPPUNIT_ASSERT(singlePool.canAdd() == true);
            CPPUNIT_ASSERT(singlePool.getById(42) == nullptr);
        }
        
        // Test larger pool
        {
            const std::size_t LARGE_SIZE = 100;
            ObjectTablePool<PoolItem, IdType, LARGE_SIZE> largePool;
            
            // Fill completely
            for (std::size_t i = 0; i < LARGE_SIZE; ++i)
            {
                auto obj = largePool.add(static_cast<IdType>(i));
                CPPUNIT_ASSERT(obj != nullptr);
                CPPUNIT_ASSERT(largePool.getCount() == i + 1);
            }
            
            CPPUNIT_ASSERT(largePool.canAdd() == false);
            CPPUNIT_ASSERT(largePool.getCount() == LARGE_SIZE);
            
            // Verify all objects can be retrieved
            for (std::size_t i = 0; i < LARGE_SIZE; ++i)
            {
                CPPUNIT_ASSERT(largePool.getById(static_cast<IdType>(i)) != nullptr);
                CPPUNIT_ASSERT(largePool.getByIndex(i) != nullptr);
            }
            
            largePool.reset();
            CPPUNIT_ASSERT(largePool.getCount() == 0);
        }
    }

    void testIdTypeBoundaries()
    {
        const std::size_t POOL_SIZE = 10;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Test boundary ID values for uint8_t
        const IdType MIN_ID = 0;
        const IdType MAX_ID = 255;
        const IdType MID_ID = 128;
        
        // Add objects with boundary IDs
        auto objMin = pool.add(MIN_ID);
        auto objMax = pool.add(MAX_ID);
        auto objMid = pool.add(MID_ID);
        
        CPPUNIT_ASSERT(objMin != nullptr);
        CPPUNIT_ASSERT(objMax != nullptr);
        CPPUNIT_ASSERT(objMid != nullptr);
        
        // Verify ID assignment
        CPPUNIT_ASSERT(objMin->getId() == MIN_ID);
        CPPUNIT_ASSERT(objMax->getId() == MAX_ID);
        CPPUNIT_ASSERT(objMid->getId() == MID_ID);
        
        // Verify retrieval by boundary IDs
        CPPUNIT_ASSERT(pool.getById(MIN_ID) == objMin);
        CPPUNIT_ASSERT(pool.getById(MAX_ID) == objMax);
        CPPUNIT_ASSERT(pool.getById(MID_ID) == objMid);
        
        // Test duplicate ID handling with boundary values
        CPPUNIT_ASSERT_THROW(pool.add(MIN_ID), std::logic_error);
        CPPUNIT_ASSERT_THROW(pool.add(MAX_ID), std::logic_error);
        CPPUNIT_ASSERT_THROW(pool.add(MID_ID), std::logic_error);
        
        // After reset, should be able to reuse same IDs
        pool.reset();
        auto objMin2 = pool.add(MIN_ID);
        auto objMax2 = pool.add(MAX_ID);
        
        CPPUNIT_ASSERT(objMin2 != nullptr);
        CPPUNIT_ASSERT(objMax2 != nullptr);
        CPPUNIT_ASSERT(objMin2->getId() == MIN_ID);
        CPPUNIT_ASSERT(objMax2->getId() == MAX_ID);
    }

    void testObjectStateLifecycle()
    {
        const std::size_t POOL_SIZE = 3;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Add objects and verify initial state
        auto obj1 = pool.add(10);
        auto obj2 = pool.add(20);
        auto obj3 = pool.add(30);
        
        CPPUNIT_ASSERT(obj1 != nullptr);
        CPPUNIT_ASSERT(obj2 != nullptr);
        CPPUNIT_ASSERT(obj3 != nullptr);
        
        // Verify IDs are set correctly
        CPPUNIT_ASSERT(obj1->getId() == 10);
        CPPUNIT_ASSERT(obj2->getId() == 20);
        CPPUNIT_ASSERT(obj3->getId() == 30);
        
        // Reset should call reset() on all objects and clear pool
        pool.reset();
        CPPUNIT_ASSERT(pool.getCount() == 0);
        CPPUNIT_ASSERT(pool.getById(10) == nullptr);
        CPPUNIT_ASSERT(pool.getById(20) == nullptr);
        CPPUNIT_ASSERT(pool.getById(30) == nullptr);
        
        // Add new objects and verify they get reset objects but with new IDs
        auto newObj1 = pool.add(100);
        auto newObj2 = pool.add(200);
        
        CPPUNIT_ASSERT(newObj1 != nullptr);
        CPPUNIT_ASSERT(newObj2 != nullptr);
        CPPUNIT_ASSERT(newObj1->getId() == 100);
        CPPUNIT_ASSERT(newObj2->getId() == 200);
        
        // Verify pointers might be reused (objects from same pool)
        CPPUNIT_ASSERT(newObj1 == obj1 || newObj1 == obj2 || newObj1 == obj3);
        CPPUNIT_ASSERT(newObj2 == obj1 || newObj2 == obj2 || newObj2 == obj3);
        CPPUNIT_ASSERT(newObj1 != newObj2);
    }

    void testExceptionSafety()
    {
        const std::size_t POOL_SIZE = 3;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Test getByIndex with various invalid indices
        CPPUNIT_ASSERT_THROW(pool.getByIndex(0), std::range_error);
        CPPUNIT_ASSERT_THROW(pool.getByIndex(1), std::range_error);
        CPPUNIT_ASSERT_THROW(pool.getByIndex(100), std::range_error);
        CPPUNIT_ASSERT_THROW(pool.getByIndex(SIZE_MAX), std::range_error);
        
        // Add one object
        auto obj1 = pool.add(42);
        CPPUNIT_ASSERT(obj1 != nullptr);
        
        // Valid index should work
        CPPUNIT_ASSERT(pool.getByIndex(0) == obj1);
        
        // Invalid indices should still throw
        CPPUNIT_ASSERT_THROW(pool.getByIndex(1), std::range_error);
        CPPUNIT_ASSERT_THROW(pool.getByIndex(2), std::range_error);
        CPPUNIT_ASSERT_THROW(pool.getByIndex(100), std::range_error);
        
        // Add more objects
        auto obj2 = pool.add(43);
        auto obj3 = pool.add(44);
        
        // Valid indices
        CPPUNIT_ASSERT(pool.getByIndex(0) == obj1);
        CPPUNIT_ASSERT(pool.getByIndex(1) == obj2);
        CPPUNIT_ASSERT(pool.getByIndex(2) == obj3);
        
        // Boundary - exactly at count should throw
        CPPUNIT_ASSERT_THROW(pool.getByIndex(3), std::range_error);
        
        // Test duplicate ID exceptions
        CPPUNIT_ASSERT_THROW(pool.add(42), std::logic_error);
        CPPUNIT_ASSERT_THROW(pool.add(43), std::logic_error);
        CPPUNIT_ASSERT_THROW(pool.add(44), std::logic_error);
        
        // Pool state should remain consistent after exceptions
        CPPUNIT_ASSERT(pool.getCount() == 3);
        CPPUNIT_ASSERT(pool.canAdd() == false);
        CPPUNIT_ASSERT(pool.getById(42) == obj1);
        CPPUNIT_ASSERT(pool.getById(43) == obj2);
        CPPUNIT_ASSERT(pool.getById(44) == obj3);
    }

    void testMixedOperationPatterns()
    {
        const std::size_t POOL_SIZE = 5;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Pattern 1: Add in non-sequential order
        auto obj3 = pool.add(30);
        auto obj1 = pool.add(10);
        auto obj5 = pool.add(50);
        auto obj2 = pool.add(20);
        auto obj4 = pool.add(40);
        
        CPPUNIT_ASSERT(pool.getCount() == 5);
        CPPUNIT_ASSERT(pool.canAdd() == false);
        
        // Verify IDs are correct
        CPPUNIT_ASSERT(obj1->getId() == 10);
        CPPUNIT_ASSERT(obj2->getId() == 20);
        CPPUNIT_ASSERT(obj3->getId() == 30);
        CPPUNIT_ASSERT(obj4->getId() == 40);
        CPPUNIT_ASSERT(obj5->getId() == 50);
        
        // Verify retrieval by ID works regardless of add order
        CPPUNIT_ASSERT(pool.getById(10) == obj1);
        CPPUNIT_ASSERT(pool.getById(20) == obj2);
        CPPUNIT_ASSERT(pool.getById(30) == obj3);
        CPPUNIT_ASSERT(pool.getById(40) == obj4);
        CPPUNIT_ASSERT(pool.getById(50) == obj5);
        
        // Verify retrieval by index reflects add order
        CPPUNIT_ASSERT(pool.getByIndex(0) == obj3); // First added
        CPPUNIT_ASSERT(pool.getByIndex(1) == obj1); // Second added
        CPPUNIT_ASSERT(pool.getByIndex(2) == obj5); // Third added
        CPPUNIT_ASSERT(pool.getByIndex(3) == obj2); // Fourth added
        CPPUNIT_ASSERT(pool.getByIndex(4) == obj4); // Fifth added
        
        // Pattern 2: Interleaved operations
        pool.reset();
        CPPUNIT_ASSERT(pool.getCount() == 0);
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        auto newObj1 = pool.add(100);
        CPPUNIT_ASSERT(pool.getCount() == 1);
        CPPUNIT_ASSERT(pool.getById(100) == newObj1);
        
        auto newObj2 = pool.add(200);
        CPPUNIT_ASSERT(pool.getCount() == 2);
        CPPUNIT_ASSERT(pool.getByIndex(1) == newObj2);
        
        CPPUNIT_ASSERT(pool.getById(150) == nullptr); // Non-existent
        
        auto newObj3 = pool.add(150);
        CPPUNIT_ASSERT(pool.getCount() == 3);
        CPPUNIT_ASSERT(pool.getById(150) == newObj3);
    }

    void testDifferentTemplateTypes()
    {
        // Test with different ID type and object
        {
            typedef std::uint16_t WideIdType;
            ObjectTablePool<ComplexItem, WideIdType, 3> widePool;
            
            CPPUNIT_ASSERT(widePool.getCount() == 0);
            CPPUNIT_ASSERT(widePool.canAdd() == true);
            
            auto obj1 = widePool.add(1000);
            auto obj2 = widePool.add(2000);
            auto obj3 = widePool.add(65535); // Max uint16_t
            
            CPPUNIT_ASSERT(obj1 != nullptr);
            CPPUNIT_ASSERT(obj2 != nullptr);
            CPPUNIT_ASSERT(obj3 != nullptr);
            
            CPPUNIT_ASSERT(obj1->getId() == 1000);
            CPPUNIT_ASSERT(obj2->getId() == 2000);
            CPPUNIT_ASSERT(obj3->getId() == 65535);
            
            CPPUNIT_ASSERT(widePool.getCount() == 3);
            CPPUNIT_ASSERT(widePool.canAdd() == false);
            
            CPPUNIT_ASSERT(widePool.getById(1000) == obj1);
            CPPUNIT_ASSERT(widePool.getById(2000) == obj2);
            CPPUNIT_ASSERT(widePool.getById(65535) == obj3);
            CPPUNIT_ASSERT(widePool.getById(500) == nullptr);
        }
        
        // Test with simple POD type
        {
            ObjectTablePool<SimpleItem, IdType, 2> simplePool;
            
            auto obj1 = simplePool.add(10);
            auto obj2 = simplePool.add(20);
            
            CPPUNIT_ASSERT(obj1 != nullptr);
            CPPUNIT_ASSERT(obj2 != nullptr);
            CPPUNIT_ASSERT(obj1->getId() == 10);
            CPPUNIT_ASSERT(obj2->getId() == 20);
            CPPUNIT_ASSERT(obj1->data == 42); // Default value
            CPPUNIT_ASSERT(obj2->data == 42);
            
            simplePool.reset();
            CPPUNIT_ASSERT(simplePool.getCount() == 0);
        }
    }

    void testComplexScenarios()
    {
        const std::size_t POOL_SIZE = 10;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Scenario 1: Fill completely, reset, fill with different IDs
        std::vector<PoolItem*> firstRun;
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            auto obj = pool.add(i);
            CPPUNIT_ASSERT(obj != nullptr);
            CPPUNIT_ASSERT(obj->getId() == i);
            firstRun.push_back(obj);
        }
        CPPUNIT_ASSERT(pool.getCount() == POOL_SIZE);
        CPPUNIT_ASSERT(pool.canAdd() == false);
        
        pool.reset();
        CPPUNIT_ASSERT(pool.getCount() == 0);
        
        std::vector<PoolItem*> secondRun;
        for (IdType i = 0; i < POOL_SIZE; ++i)
        {
            IdType newId = static_cast<IdType>(100 + i);
            auto obj = pool.add(newId);
            CPPUNIT_ASSERT(obj != nullptr);
            CPPUNIT_ASSERT(obj->getId() == newId);
            secondRun.push_back(obj);
        }
        
        // Verify object pointers are reused but IDs are different
        for (std::size_t i = 0; i < POOL_SIZE; ++i)
        {
            bool found = false;
            for (std::size_t j = 0; j < POOL_SIZE; ++j)
            {
                if (firstRun[i] == secondRun[j])
                {
                    found = true;
                    break;
                }
            }
            CPPUNIT_ASSERT(found); // Object pointer should be reused
        }
        
        // Scenario 2: Partial operations and verification
        pool.reset();
        
        // Add partial set
        auto obj1 = pool.add(255);
        auto obj2 = pool.add(0);
        auto obj3 = pool.add(128);
        
        CPPUNIT_ASSERT(pool.getCount() == 3);
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        // Verify specific retrievals
        CPPUNIT_ASSERT(pool.getByIndex(0) == obj1);
        CPPUNIT_ASSERT(pool.getByIndex(1) == obj2);
        CPPUNIT_ASSERT(pool.getByIndex(2) == obj3);
        
        CPPUNIT_ASSERT(pool.getById(255) == obj1);
        CPPUNIT_ASSERT(pool.getById(0) == obj2);
        CPPUNIT_ASSERT(pool.getById(128) == obj3);
        CPPUNIT_ASSERT(pool.getById(64) == nullptr);
    }

    void testHighFrequencyOperations()
    {
        const std::size_t POOL_SIZE = 50;
        ObjectTablePool<PoolItem, IdType, POOL_SIZE> pool;
        
        // Perform many rapid add/reset cycles
        for (int cycle = 0; cycle < 20; ++cycle)
        {
            CPPUNIT_ASSERT(pool.getCount() == 0);
            CPPUNIT_ASSERT(pool.canAdd() == true);
            
            // Fill pool partially or completely
            std::size_t addCount = (cycle % 2 == 0) ? POOL_SIZE : POOL_SIZE / 2;
            
            for (std::size_t i = 0; i < addCount; ++i)
            {
                IdType id = static_cast<IdType>((cycle * 10 + i) % 256);
                auto obj = pool.add(id);
                CPPUNIT_ASSERT(obj != nullptr);
                CPPUNIT_ASSERT(obj->getId() == id);
                CPPUNIT_ASSERT(pool.getCount() == i + 1);
            }
            
            // Verify state
            CPPUNIT_ASSERT(pool.getCount() == addCount);
            CPPUNIT_ASSERT(pool.canAdd() == (addCount < POOL_SIZE));
            
            // Verify some retrievals
            for (std::size_t i = 0; i < addCount; ++i)
            {
                CPPUNIT_ASSERT(pool.getByIndex(i) != nullptr);
            }
            
            // Reset for next cycle
            pool.reset();
        }
        
        // Final verification
        CPPUNIT_ASSERT(pool.getCount() == 0);
        CPPUNIT_ASSERT(pool.canAdd() == true);
        
        // One more full cycle to ensure consistency
        for (std::size_t i = 0; i < POOL_SIZE; ++i)
        {
            auto obj = pool.add(static_cast<IdType>(i));
            CPPUNIT_ASSERT(obj != nullptr);
        }
        CPPUNIT_ASSERT(pool.getCount() == POOL_SIZE);
        CPPUNIT_ASSERT(pool.canAdd() == false);
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

    class ComplexItem
    {
    public:
        ComplexItem() :
                m_id(0),
                m_idValid(false),
                value(123),
                name("default")
        {
            // noop
        }

        void reset()
        {
            m_idValid = false;
            value = 123;
            name = "default";
        }

        void setId(std::uint16_t id)
        {
            m_id = id;
            m_idValid = true;
        }

        std::uint16_t getId() const
        {
            CPPUNIT_ASSERT(m_idValid);
            return m_id;
        }

        int value;
        std::string name;

    private:
        std::uint16_t m_id;
        bool m_idValid;
    };

    class SimpleItem
    {
    public:
        SimpleItem() :
                m_id(0),
                m_idValid(false),
                data(42)
        {
            // noop
        }

        void reset()
        {
            m_idValid = false;
            data = 42;
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

        int data;

    private:
        IdType m_id;
        bool m_idValid;
    };
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectTablePoolTest);
