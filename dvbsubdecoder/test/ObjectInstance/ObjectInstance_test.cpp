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
    CPPUNIT_TEST(testErrors);
    CPPUNIT_TEST(testObjectInstanceConstruction);
    CPPUNIT_TEST(testObjectInstanceMemberValues);
    CPPUNIT_TEST(testEmptyListBehavior);
    CPPUNIT_TEST(testSingleObjectOperations);
    CPPUNIT_TEST(testListStateConsistency);
    CPPUNIT_TEST(testObjectDataPreservation);
    CPPUNIT_TEST(testComplexOperationSequences);
    CPPUNIT_TEST(testConstObjectInstanceList);
    CPPUNIT_TEST(testExceptionSafety);
    CPPUNIT_TEST(testObjectReuse);
    CPPUNIT_TEST(testBoundaryValues);
    CPPUNIT_TEST(testIterationEdgeCases);
    CPPUNIT_TEST(testLargeScaleOperations);
    CPPUNIT_TEST(testObjectOwnership);
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

        // Adding a null pointer should be treated as invalid argument (implementation throws std::invalid_argument)
        CPPUNIT_ASSERT_THROW(list.add(nullptr), std::invalid_argument);
        CPPUNIT_ASSERT_THROW(list.add(&pool[0]), std::logic_error);

        list.removeFirst();
        CPPUNIT_ASSERT_THROW(list.getNext(&pool[0]), std::logic_error);

        // Requesting next for a null previous element is an invalid argument (implementation throws std::invalid_argument)
        CPPUNIT_ASSERT_THROW(list.getNext(nullptr), std::invalid_argument);
    }

    void testObjectInstanceConstruction()
    {
        // Test default construction
        ObjectInstance obj;
        
        // ObjectInstance inherits from ObjectInstanceListNode
        // Verify object can be constructed and is not initially in any list
        ObjectInstanceList list;
        
        // Should be able to add newly constructed object
        list.add(&obj);
        CPPUNIT_ASSERT(list.getFirst() == &obj);
        
        // Remove and verify object is reusable
        ObjectInstance* removed = list.removeFirst();
        CPPUNIT_ASSERT(removed == &obj);
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
        
        // Should be able to add again
        list.add(&obj);
        CPPUNIT_ASSERT(list.getFirst() == &obj);
    }

    void testObjectInstanceMemberValues()
    {
        ObjectInstance obj1, obj2, obj3;
        
        // Test boundary values for m_objectId (uint16_t)
        obj1.m_objectId = 0;
        obj2.m_objectId = 65535; // UINT16_MAX
        obj3.m_objectId = 12345; // typical value
        
        // Test boundary values for positions (int32_t)
        obj1.m_positionX = INT32_MIN;
        obj1.m_positionY = INT32_MAX;
        
        obj2.m_positionX = 0;
        obj2.m_positionY = 0;
        
        obj3.m_positionX = -1000;
        obj3.m_positionY = 1000;
        
        ObjectInstanceList list;
        
        // Add objects with different member values
        list.add(&obj1);
        list.add(&obj2);
        list.add(&obj3);
        
        // Verify values are preserved in list
        const ObjectInstance* first = list.getFirst();
        CPPUNIT_ASSERT(first == &obj1);
        CPPUNIT_ASSERT(first->m_objectId == 0);
        CPPUNIT_ASSERT(first->m_positionX == INT32_MIN);
        CPPUNIT_ASSERT(first->m_positionY == INT32_MAX);
        
        const ObjectInstance* second = list.getNext(first);
        CPPUNIT_ASSERT(second == &obj2);
        CPPUNIT_ASSERT(second->m_objectId == 65535);
        CPPUNIT_ASSERT(second->m_positionX == 0);
        CPPUNIT_ASSERT(second->m_positionY == 0);
        
        const ObjectInstance* third = list.getNext(second);
        CPPUNIT_ASSERT(third == &obj3);
        CPPUNIT_ASSERT(third->m_objectId == 12345);
        CPPUNIT_ASSERT(third->m_positionX == -1000);
        CPPUNIT_ASSERT(third->m_positionY == 1000);
    }

    void testEmptyListBehavior()
    {
        ObjectInstanceList emptyList;
        
        // Test getFirst() on empty list
        CPPUNIT_ASSERT(emptyList.getFirst() == nullptr);
        
        // Test removeFirst() on empty list
        CPPUNIT_ASSERT(emptyList.removeFirst() == nullptr);
        
        // Test that multiple removeFirst() calls on empty list are safe
        CPPUNIT_ASSERT(emptyList.removeFirst() == nullptr);
        CPPUNIT_ASSERT(emptyList.removeFirst() == nullptr);
        
        // Test that getFirst() remains nullptr after failed removes
        CPPUNIT_ASSERT(emptyList.getFirst() == nullptr);
        
        // Test adding to empty list
        ObjectInstance obj;
        emptyList.add(&obj);
        CPPUNIT_ASSERT(emptyList.getFirst() == &obj);
        CPPUNIT_ASSERT(emptyList.getNext(&obj) == nullptr);
    }

    void testSingleObjectOperations()
    {
        ObjectInstance obj;
        obj.m_objectId = 42;
        obj.m_positionX = 100;
        obj.m_positionY = 200;
        
        ObjectInstanceList list;
        
        // Add single object
        list.add(&obj);
        
        // Verify list state with single object
        const ObjectInstance* first = list.getFirst();
        CPPUNIT_ASSERT(first == &obj);
        CPPUNIT_ASSERT(first->m_objectId == 42);
        
        // Test getNext() on single object (should return nullptr)
        CPPUNIT_ASSERT(list.getNext(first) == nullptr);
        
        // Remove single object
        ObjectInstance* removed = list.removeFirst();
        CPPUNIT_ASSERT(removed == &obj);
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
        
        // Verify object data is preserved after removal
        CPPUNIT_ASSERT(obj.m_objectId == 42);
        CPPUNIT_ASSERT(obj.m_positionX == 100);
        CPPUNIT_ASSERT(obj.m_positionY == 200);
        
        // Test re-adding same object
        list.add(&obj);
        CPPUNIT_ASSERT(list.getFirst() == &obj);
    }

    void testListStateConsistency()
    {
        const int COUNT = 5;
        std::array<ObjectInstance, COUNT> objects;
        ObjectInstanceList list;
        
        // Initialize objects with unique values
        for (int i = 0; i < COUNT; ++i)
        {
            objects[i].m_objectId = static_cast<std::uint16_t>(i);
            objects[i].m_positionX = i * 10;
            objects[i].m_positionY = i * 20;
        }
        
        // Add objects and verify state after each addition
        for (int i = 0; i < COUNT; ++i)
        {
            list.add(&objects[i]);
            
            // Verify first object is always objects[0]
            CPPUNIT_ASSERT(list.getFirst() == &objects[0]);
            
            // Verify we can iterate through all added objects
            const ObjectInstance* current = list.getFirst();
            for (int j = 0; j <= i; ++j)
            {
                CPPUNIT_ASSERT(current == &objects[j]);
                CPPUNIT_ASSERT(current->m_objectId == static_cast<std::uint16_t>(j));
                if (j < i)
                {
                    current = list.getNext(current);
                }
                else
                {
                    CPPUNIT_ASSERT(list.getNext(current) == nullptr);
                }
            }
        }
        
        // Remove objects and verify state after each removal
        for (int i = 0; i < COUNT; ++i)
        {
            ObjectInstance* removed = list.removeFirst();
            CPPUNIT_ASSERT(removed == &objects[i]);
            
            if (i < COUNT - 1)
            {
                // Verify next object becomes first
                CPPUNIT_ASSERT(list.getFirst() == &objects[i + 1]);
            }
            else
            {
                // Last object removed, list should be empty
                CPPUNIT_ASSERT(list.getFirst() == nullptr);
            }
        }
    }

    void testObjectDataPreservation()
    {
        ObjectInstance obj1, obj2;
        
        // Set unique data
        obj1.m_objectId = 111;
        obj1.m_positionX = -500;
        obj1.m_positionY = 750;
        
        obj2.m_objectId = 222;
        obj2.m_positionX = 1500;
        obj2.m_positionY = -250;
        
        ObjectInstanceList list;
        
        // Add objects
        list.add(&obj1);
        list.add(&obj2);
        
        // Verify data through list iteration
        const ObjectInstance* current = list.getFirst();
        CPPUNIT_ASSERT(current->m_objectId == 111);
        CPPUNIT_ASSERT(current->m_positionX == -500);
        CPPUNIT_ASSERT(current->m_positionY == 750);
        
        current = list.getNext(current);
        CPPUNIT_ASSERT(current->m_objectId == 222);
        CPPUNIT_ASSERT(current->m_positionX == 1500);
        CPPUNIT_ASSERT(current->m_positionY == -250);
        
        // Remove first object and verify data preservation
        ObjectInstance* removed = list.removeFirst();
        CPPUNIT_ASSERT(removed->m_objectId == 111);
        CPPUNIT_ASSERT(removed->m_positionX == -500);
        CPPUNIT_ASSERT(removed->m_positionY == 750);
        
        // Verify remaining object data
        current = list.getFirst();
        CPPUNIT_ASSERT(current->m_objectId == 222);
        CPPUNIT_ASSERT(current->m_positionX == 1500);
        CPPUNIT_ASSERT(current->m_positionY == -250);
        
        // Modify data and verify it persists
        obj2.m_objectId = 333;
        CPPUNIT_ASSERT(current->m_objectId == 333);
    }

    void testComplexOperationSequences()
    {
        const int COUNT = 10;
        std::array<ObjectInstance, COUNT> objects;
        ObjectInstanceList list;
        
        // Initialize objects
        for (int i = 0; i < COUNT; ++i)
        {
            objects[i].m_objectId = static_cast<std::uint16_t>(i);
        }
        
        // Complex sequence: add some, remove some, add more
        
        // Add first 5 objects
        for (int i = 0; i < 5; ++i)
        {
            list.add(&objects[i]);
        }
        
        // Remove first 2
        ObjectInstance* removed1 = list.removeFirst();
        ObjectInstance* removed2 = list.removeFirst();
        CPPUNIT_ASSERT(removed1 == &objects[0]);
        CPPUNIT_ASSERT(removed2 == &objects[1]);
        
        // Add next 5 objects
        for (int i = 5; i < COUNT; ++i)
        {
            list.add(&objects[i]);
        }
        
        // Verify current state: should have objects[2,3,4,5,6,7,8,9]
        const ObjectInstance* current = list.getFirst();
        for (int expected = 2; expected < COUNT; ++expected)
        {
            CPPUNIT_ASSERT(current == &objects[expected]);
            CPPUNIT_ASSERT(current->m_objectId == static_cast<std::uint16_t>(expected));
            if (expected < COUNT - 1)
            {
                current = list.getNext(current);
            }
        }
        
        // Re-add the removed objects
        list.add(&objects[0]);
        list.add(&objects[1]);
        
        // Verify objects[0] and objects[1] are now at the end
        current = list.getFirst();
        // Skip to the end
        while (list.getNext(current) != nullptr)
        {
            current = list.getNext(current);
        }
        CPPUNIT_ASSERT(current == &objects[1]); // Last added
    }

    void testConstObjectInstanceList()
    {
        ObjectInstance obj1, obj2;
        obj1.m_objectId = 100;
        obj2.m_objectId = 200;
        
        ObjectInstanceList list;
        list.add(&obj1);
        list.add(&obj2);
        
        // Test const list operations
        const ObjectInstanceList& constList = list;
        
        // Test const getFirst()
        const ObjectInstance* constFirst = constList.getFirst();
        CPPUNIT_ASSERT(constFirst == &obj1);
        CPPUNIT_ASSERT(constFirst->m_objectId == 100);
        
        // Test const getNext()
        const ObjectInstance* constSecond = constList.getNext(constFirst);
        CPPUNIT_ASSERT(constSecond == &obj2);
        CPPUNIT_ASSERT(constSecond->m_objectId == 200);
        
        // Test const iteration
        const ObjectInstance* constCurrent = constList.getFirst();
        int count = 0;
        while (constCurrent)
        {
            count++;
            constCurrent = (constCurrent->m_objectId == 200) ? nullptr : constList.getNext(constCurrent);
        }
        CPPUNIT_ASSERT(count == 2);
    }

    void testExceptionSafety()
    {
        ObjectInstance obj1, obj2;
        ObjectInstanceList list;
        
        list.add(&obj1);
        list.add(&obj2);
        
        // Test that exceptions don't corrupt list state
        try
        {
            list.add(nullptr);
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const std::invalid_argument&)
        {
            // Verify list state is unchanged
            CPPUNIT_ASSERT(list.getFirst() == &obj1);
            const ObjectInstance* second = list.getNext(&obj1);
            CPPUNIT_ASSERT(second == &obj2);
            CPPUNIT_ASSERT(list.getNext(second) == nullptr);
        }
        
        // Test duplicate add exception safety
        try
        {
            list.add(&obj1);
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const std::logic_error&)
        {
            // Verify list state is unchanged
            CPPUNIT_ASSERT(list.getFirst() == &obj1);
            CPPUNIT_ASSERT(list.getNext(&obj1) == &obj2);
        }
        
        // Test getNext with nullptr exception safety
        try
        {
            list.getNext(nullptr);
            CPPUNIT_FAIL("Should have thrown exception");
        }
        catch (const std::invalid_argument&)
        {
            // Verify list state is unchanged
            CPPUNIT_ASSERT(list.getFirst() == &obj1);
            CPPUNIT_ASSERT(list.getNext(&obj1) == &obj2);
        }
    }

    void testObjectReuse()
    {
        ObjectInstance obj;
        obj.m_objectId = 999;
        
        ObjectInstanceList list1, list2;
        
        // Add to first list
        list1.add(&obj);
        CPPUNIT_ASSERT(list1.getFirst() == &obj);
        
        // Remove from first list
        ObjectInstance* removed = list1.removeFirst();
        CPPUNIT_ASSERT(removed == &obj);
        CPPUNIT_ASSERT(list1.getFirst() == nullptr);
        
        // Add to second list
        list2.add(&obj);
        CPPUNIT_ASSERT(list2.getFirst() == &obj);
        CPPUNIT_ASSERT(obj.m_objectId == 999); // Data preserved
        
        // Verify first list is still empty
        CPPUNIT_ASSERT(list1.getFirst() == nullptr);
        
        // Remove from second list
        removed = list2.removeFirst();
        CPPUNIT_ASSERT(removed == &obj);
        CPPUNIT_ASSERT(list2.getFirst() == nullptr);
        
        // Add back to first list
        list1.add(&obj);
        CPPUNIT_ASSERT(list1.getFirst() == &obj);
        CPPUNIT_ASSERT(obj.m_objectId == 999); // Data still preserved
    }

    void testBoundaryValues()
    {
        ObjectInstance minObj, maxObj, zeroObj;
        
        // Test minimum values
        minObj.m_objectId = 0;
        minObj.m_positionX = INT32_MIN;
        minObj.m_positionY = INT32_MIN;
        
        // Test maximum values
        maxObj.m_objectId = UINT16_MAX;
        maxObj.m_positionX = INT32_MAX;
        maxObj.m_positionY = INT32_MAX;
        
        // Test zero values
        zeroObj.m_objectId = 0;
        zeroObj.m_positionX = 0;
        zeroObj.m_positionY = 0;
        
        ObjectInstanceList list;
        
        // Add boundary value objects
        list.add(&minObj);
        list.add(&maxObj);
        list.add(&zeroObj);
        
        // Verify all values are preserved correctly
        const ObjectInstance* current = list.getFirst();
        CPPUNIT_ASSERT(current == &minObj);
        CPPUNIT_ASSERT(current->m_objectId == 0);
        CPPUNIT_ASSERT(current->m_positionX == INT32_MIN);
        CPPUNIT_ASSERT(current->m_positionY == INT32_MIN);
        
        current = list.getNext(current);
        CPPUNIT_ASSERT(current == &maxObj);
        CPPUNIT_ASSERT(current->m_objectId == UINT16_MAX);
        CPPUNIT_ASSERT(current->m_positionX == INT32_MAX);
        CPPUNIT_ASSERT(current->m_positionY == INT32_MAX);
        
        current = list.getNext(current);
        CPPUNIT_ASSERT(current == &zeroObj);
        CPPUNIT_ASSERT(current->m_objectId == 0);
        CPPUNIT_ASSERT(current->m_positionX == 0);
        CPPUNIT_ASSERT(current->m_positionY == 0);
        
        CPPUNIT_ASSERT(list.getNext(current) == nullptr);
    }

    void testIterationEdgeCases()
    {
        ObjectInstance obj;
        ObjectInstanceList list;
        
        // Test iteration on empty list
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
        
        // Test single object iteration
        list.add(&obj);
        const ObjectInstance* current = list.getFirst();
        CPPUNIT_ASSERT(current == &obj);
        CPPUNIT_ASSERT(list.getNext(current) == nullptr);
        
        // Test iteration after object removal
        ObjectInstance* removed = list.removeFirst();
        CPPUNIT_ASSERT(removed == &obj);
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
        
        // Test iteration with multiple add/remove cycles
        ObjectInstance obj2, obj3;
        
        list.add(&obj);
        list.add(&obj2);
        list.add(&obj3);
        
        // Remove middle object by removing first twice, then re-adding first
        ObjectInstance* first = list.removeFirst();  // Remove obj
        ObjectInstance* second = list.removeFirst(); // Remove obj2
        CPPUNIT_ASSERT(first == &obj);
        CPPUNIT_ASSERT(second == &obj2);
        
        list.add(&obj); // Add obj back (now obj3, obj are in list)
        
        // Verify iteration order
        current = list.getFirst();
        CPPUNIT_ASSERT(current == &obj3);
        current = list.getNext(current);
        CPPUNIT_ASSERT(current == &obj);
        CPPUNIT_ASSERT(list.getNext(current) == nullptr);
    }

    void testLargeScaleOperations()
    {
        const int LARGE_COUNT = 500;
        std::vector<ObjectInstance> objects(LARGE_COUNT);
        ObjectInstanceList list;
        
        // Initialize objects with unique IDs
        for (int i = 0; i < LARGE_COUNT; ++i)
        {
            objects[i].m_objectId = static_cast<std::uint16_t>(i % UINT16_MAX);
            objects[i].m_positionX = i;
            objects[i].m_positionY = -i;
        }
        
        // Add all objects
        for (int i = 0; i < LARGE_COUNT; ++i)
        {
            list.add(&objects[i]);
        }
        
        // Verify all objects are in list in correct order
        const ObjectInstance* current = list.getFirst();
        for (int i = 0; i < LARGE_COUNT; ++i)
        {
            CPPUNIT_ASSERT(current == &objects[i]);
            CPPUNIT_ASSERT(current->m_positionX == i);
            CPPUNIT_ASSERT(current->m_positionY == -i);
            
            if (i < LARGE_COUNT - 1)
            {
                current = list.getNext(current);
            }
            else
            {
                CPPUNIT_ASSERT(list.getNext(current) == nullptr);
            }
        }
        
        // Remove half the objects
        for (int i = 0; i < LARGE_COUNT / 2; ++i)
        {
            ObjectInstance* removed = list.removeFirst();
            CPPUNIT_ASSERT(removed == &objects[i]);
        }
        
        // Verify remaining objects
        current = list.getFirst();
        for (int i = LARGE_COUNT / 2; i < LARGE_COUNT; ++i)
        {
            CPPUNIT_ASSERT(current == &objects[i]);
            if (i < LARGE_COUNT - 1)
            {
                current = list.getNext(current);
            }
        }
        
        // Remove all remaining objects
        while (list.getFirst())
        {
            list.removeFirst();
        }
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
    }

    void testObjectOwnership()
    {
        ObjectInstance* dynamicObj = new ObjectInstance();
        dynamicObj->m_objectId = 777;
        dynamicObj->m_positionX = 888;
        dynamicObj->m_positionY = 999;
        
        ObjectInstanceList list;
        
        // Add dynamic object to list
        list.add(dynamicObj);
        CPPUNIT_ASSERT(list.getFirst() == dynamicObj);
        
        // Remove object from list
        ObjectInstance* removed = list.removeFirst();
        CPPUNIT_ASSERT(removed == dynamicObj);
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
        
        // Verify object data is still valid (list doesn't own objects)
        CPPUNIT_ASSERT(dynamicObj->m_objectId == 777);
        CPPUNIT_ASSERT(dynamicObj->m_positionX == 888);
        CPPUNIT_ASSERT(dynamicObj->m_positionY == 999);
        
        // Object can be re-added after removal
        list.add(dynamicObj);
        CPPUNIT_ASSERT(list.getFirst() == dynamicObj);
        
        // Clean up (object lifetime is caller's responsibility)
        list.removeFirst();
        delete dynamicObj;
        
        // Test with stack objects going out of scope
        {
            ObjectInstance localObj;
            localObj.m_objectId = 555;
            list.add(&localObj);
            CPPUNIT_ASSERT(list.getFirst() == &localObj);
            
            // Remove before going out of scope
            ObjectInstance* localRemoved = list.removeFirst();
            CPPUNIT_ASSERT(localRemoved == &localObj);
            CPPUNIT_ASSERT(list.getFirst() == nullptr);
        } // localObj goes out of scope here, but list should be unaffected
        
        CPPUNIT_ASSERT(list.getFirst() == nullptr);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectInstanceTest);
