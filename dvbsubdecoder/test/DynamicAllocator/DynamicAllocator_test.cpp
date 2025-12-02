/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#include <cppunit/extensions/HelperMacros.h>

#include "DynamicAllocator.hpp"
#include "AllocatorTraits.hpp"

using dvbsubdecoder::DynamicAllocator;
using dvbsubdecoder::AllocatorTraits;

class DynamicAllocatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DynamicAllocatorTest );
    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testBasicAllocation);
    CPPUNIT_TEST(testBasicFree);
    CPPUNIT_TEST(testMultipleAllocations);
    CPPUNIT_TEST(testZeroSizeAllocation);
    CPPUNIT_TEST(testPowerOfTwoAlignments);
    CPPUNIT_TEST(testLargeAlignment);
    CPPUNIT_TEST(testLargeSize);
    CPPUNIT_TEST(testMaxSizeAllocation);
    CPPUNIT_TEST(testFreeNullPointer);
    CPPUNIT_TEST(testFreeInvalidPointer);
    CPPUNIT_TEST(testDoubleFree);
    CPPUNIT_TEST(testFreePointerToMiddleOfAllocation);
    CPPUNIT_TEST(testReturnedPointerAlignment);
    CPPUNIT_TEST(testComplexAlignmentScenarios);
    CPPUNIT_TEST(testDirectAllocationSuccess);
    CPPUNIT_TEST(testExtendedAllocationFallback);
    CPPUNIT_TEST(testAllocationOrderIndependence);
    CPPUNIT_TEST(testMixedSizeAlignmentPatterns);
    CPPUNIT_TEST(testHighFrequencyAllocationDeallocation);
    CPPUNIT_TEST(testMemoryExhaustionHandling);
    CPPUNIT_TEST(testAllocatorTraitsIntegration);
    CPPUNIT_TEST(testExceptionSafetyInAllocation);
    CPPUNIT_TEST(testStateConsistencyAfterErrors);
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

    // Basic API Coverage Tests
    void testConstructorInitialization()
    {
        // Test that constructor creates allocator in clean state
        DynamicAllocator allocator;

        // Should be able to allocate immediately
        void* ptr = allocator.allocate(100, 1);
        CPPUNIT_ASSERT(ptr != nullptr);

        allocator.free(ptr);
    }

    void testBasicAllocation()
    {
        DynamicAllocator allocator;

        void* ptr = allocator.allocate(1024, 8);
        CPPUNIT_ASSERT(ptr != nullptr);

        // Verify alignment
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % 8 == 0);

        allocator.free(ptr);
    }

    void testBasicFree()
    {
        DynamicAllocator allocator;

        void* ptr = allocator.allocate(512, 4);
        CPPUNIT_ASSERT(ptr != nullptr);

        // Free should not throw
        allocator.free(ptr);
    }

    void testMultipleAllocations()
    {
        DynamicAllocator allocator;
        std::vector<void*> pointers;

        // Allocate multiple blocks
        for (int i = 0; i < 10; ++i)
        {
            void* ptr = allocator.allocate(100 + i * 50, 4);
            CPPUNIT_ASSERT(ptr != nullptr);
            pointers.push_back(ptr);
        }

        // Free all blocks
        for (void* ptr : pointers)
        {
            allocator.free(ptr);
        }
    }

    // Edge and Boundary Cases
    void testZeroSizeAllocation()
    {
        DynamicAllocator allocator;

        // Zero size allocation should succeed
        void* ptr = allocator.allocate(0, 1);
        CPPUNIT_ASSERT(ptr != nullptr);

        allocator.free(ptr);
    }

    void testPowerOfTwoAlignments()
    {
        DynamicAllocator allocator;
        std::vector<void*> pointers;

        // Test various power-of-2 alignments
        std::vector<std::size_t> alignments = {1, 2, 4, 8, 16, 32, 64, 128, 256};

        for (std::size_t alignment : alignments)
        {
            void* ptr = allocator.allocate(100, alignment);
            CPPUNIT_ASSERT(ptr != nullptr);

            // Verify alignment
            CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0);

            pointers.push_back(ptr);
        }

        // Free all blocks
        for (void* ptr : pointers)
        {
            allocator.free(ptr);
        }
    }

    void testLargeAlignment()
    {
        DynamicAllocator allocator;

        // Test large alignment (should trigger extended allocation)
        void* ptr = allocator.allocate(100, 4096);
        CPPUNIT_ASSERT(ptr != nullptr);

        // Verify alignment
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % 4096 == 0);

        allocator.free(ptr);
    }

    void testLargeSize()
    {
        DynamicAllocator allocator;

        // Test large size allocation
        const std::size_t largeSize = 1024 * 1024; // 1MB
        void* ptr = allocator.allocate(largeSize, 8);
        CPPUNIT_ASSERT(ptr != nullptr);

        allocator.free(ptr);
    }

    void testMaxSizeAllocation()
    {
        DynamicAllocator allocator;

        // Test very large allocation - should either succeed or throw std::bad_alloc
        const std::size_t veryLargeSize = 1024 * 1024 * 1024; // 1GB

        try
        {
            void* ptr = allocator.allocate(veryLargeSize, 8);
            if (ptr != nullptr)
            {
                allocator.free(ptr);
            }
        }
        catch (const std::bad_alloc&)
        {
            // This is expected for very large allocations
        }
    }

    // Invalid Inputs and Error Handling
    void testFreeNullPointer()
    {
        DynamicAllocator allocator;

        // Free with nullptr should throw std::logic_error
        CPPUNIT_ASSERT_THROW(allocator.free(nullptr), std::logic_error);
    }

    void testFreeInvalidPointer()
    {
        DynamicAllocator allocator;

        // Free with untracked pointer should throw std::logic_error
        int dummyVariable = 42;
        void* invalidPtr = &dummyVariable;

        CPPUNIT_ASSERT_THROW(allocator.free(invalidPtr), std::logic_error);
    }

    void testDoubleFree()
    {
        DynamicAllocator allocator;

        void* ptr = allocator.allocate(100, 4);
        CPPUNIT_ASSERT(ptr != nullptr);

        // First free should succeed
        allocator.free(ptr);

        // Second free should throw std::logic_error
        CPPUNIT_ASSERT_THROW(allocator.free(ptr), std::logic_error);
    }

    void testFreePointerToMiddleOfAllocation()
    {
        DynamicAllocator allocator;

        void* ptr = allocator.allocate(1000, 8);
        CPPUNIT_ASSERT(ptr != nullptr);

        // Pointer to middle of allocation
        void* middlePtr = static_cast<char*>(ptr) + 500;

        // Should throw std::logic_error
        CPPUNIT_ASSERT_THROW(allocator.free(middlePtr), std::logic_error);

        // Original pointer should still be valid
        allocator.free(ptr);
    }

    // Alignment Verification
    void testReturnedPointerAlignment()
    {
        DynamicAllocator allocator;

        // Test various size/alignment combinations
        std::vector<std::pair<std::size_t, std::size_t>> testCases = {
            {100, 1}, {100, 2}, {100, 4}, {100, 8}, {100, 16},
            {1, 32}, {7, 64}, {33, 128}, {129, 256}
        };

        std::vector<void*> pointers;

        for (const auto& testCase : testCases)
        {
            std::size_t size = testCase.first;
            std::size_t alignment = testCase.second;

            void* ptr = allocator.allocate(size, alignment);
            CPPUNIT_ASSERT(ptr != nullptr);

            // Verify alignment
            std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
            CPPUNIT_ASSERT(addr % alignment == 0);

            pointers.push_back(ptr);
        }

        // Free all blocks
        for (void* ptr : pointers)
        {
            allocator.free(ptr);
        }
    }

    void testComplexAlignmentScenarios()
    {
        DynamicAllocator allocator;

        // Allocate with complex alignment requirements
        void* ptr1 = allocator.allocate(17, 32);   // Small size, large alignment
        void* ptr2 = allocator.allocate(1000, 8); // Large size, standard alignment
        void* ptr3 = allocator.allocate(1, 1024);  // Tiny size, huge alignment

        CPPUNIT_ASSERT(ptr1 != nullptr);
        CPPUNIT_ASSERT(ptr2 != nullptr);
        CPPUNIT_ASSERT(ptr3 != nullptr);

        // Verify alignments
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr1) % 32 == 0);
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr2) % 8 == 0);
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr3) % 1024 == 0);

        allocator.free(ptr1);
        allocator.free(ptr2);
        allocator.free(ptr3);
    }

    // Two-phase Allocation Strategy Tests
    void testDirectAllocationSuccess()
    {
        DynamicAllocator allocator;

        // Small alignment should succeed in first phase
        void* ptr = allocator.allocate(100, 4);
        CPPUNIT_ASSERT(ptr != nullptr);
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % 4 == 0);

        allocator.free(ptr);
    }

    void testExtendedAllocationFallback()
    {
        DynamicAllocator allocator;

        // Large alignment likely to trigger extended allocation
        void* ptr = allocator.allocate(10, 8192);
        CPPUNIT_ASSERT(ptr != nullptr);
        CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % 8192 == 0);

        allocator.free(ptr);
    }

    // Resource Management
    void testAllocationOrderIndependence()
    {
        DynamicAllocator allocator;

        // Allocate multiple blocks
        void* ptr1 = allocator.allocate(100, 4);
        void* ptr2 = allocator.allocate(200, 8);
        void* ptr3 = allocator.allocate(300, 16);

        CPPUNIT_ASSERT(ptr1 != nullptr);
        CPPUNIT_ASSERT(ptr2 != nullptr);
        CPPUNIT_ASSERT(ptr3 != nullptr);

        // Free in different order (reverse)
        allocator.free(ptr3);
        allocator.free(ptr1);
        allocator.free(ptr2);
    }

    void testMixedSizeAlignmentPatterns()
    {
        DynamicAllocator allocator;
        std::vector<void*> pointers;

        // Mixed allocation pattern
        std::vector<std::pair<std::size_t, std::size_t>> allocations = {
            {1, 1}, {1000, 2}, {50, 64}, {2048, 4}, {10, 512},
            {500, 8}, {1, 256}, {750, 16}, {25, 32}
        };

        // Allocate all
        for (const auto& alloc : allocations)
        {
            void* ptr = allocator.allocate(alloc.first, alloc.second);
            CPPUNIT_ASSERT(ptr != nullptr);
            CPPUNIT_ASSERT(reinterpret_cast<std::uintptr_t>(ptr) % alloc.second == 0);
            pointers.push_back(ptr);
        }

        // Free all
        for (void* ptr : pointers)
        {
            allocator.free(ptr);
        }
    }

    // Stress and Integration Tests
    void testHighFrequencyAllocationDeallocation()
    {
        DynamicAllocator allocator;

        // Perform many allocation/deallocation cycles
        for (int cycle = 0; cycle < 100; ++cycle)
        {
            std::vector<void*> pointers;

            // Allocate multiple blocks
            for (int i = 0; i < 10; ++i)
            {
                std::size_t size = 50 + (i * 25);
                std::size_t alignment = 1 << (i % 6); // 1, 2, 4, 8, 16, 32

                void* ptr = allocator.allocate(size, alignment);
                CPPUNIT_ASSERT(ptr != nullptr);
                pointers.push_back(ptr);
            }

            // Free all blocks
            for (void* ptr : pointers)
            {
                allocator.free(ptr);
            }
        }
    }

    void testMemoryExhaustionHandling()
    {
        DynamicAllocator allocator;

        // Try to allocate impossibly large block
        try
        {
            void* ptr = allocator.allocate(SIZE_MAX - 1, 1);
            if (ptr != nullptr)
            {
                allocator.free(ptr);
            }
        }
        catch (const std::bad_alloc&)
        {
            // Expected behavior for memory exhaustion
        }

        // Allocator should still work for normal allocations
        void* normalPtr = allocator.allocate(100, 4);
        CPPUNIT_ASSERT(normalPtr != nullptr);
        allocator.free(normalPtr);
    }

    void testAllocatorTraitsIntegration()
    {
        DynamicAllocator allocator;
        AllocatorTraits traits(allocator);

        // Test AllocatorTraits with DynamicAllocator
        auto intPtr = traits.allocUnique<int>(42);
        CPPUNIT_ASSERT(intPtr != nullptr);
        CPPUNIT_ASSERT(*intPtr == 42);

        auto stringPtr = traits.allocUnique<std::string>("test");
        CPPUNIT_ASSERT(stringPtr != nullptr);
        CPPUNIT_ASSERT(*stringPtr == "test");

        auto arrayPtr = traits.allocUnique<std::array<int, 10>>();
        CPPUNIT_ASSERT(arrayPtr != nullptr);
        // Objects should be automatically freed when unique_ptrs go out of scope
    }

    // Exception Safety
    void testExceptionSafetyInAllocation()
    {
        DynamicAllocator allocator;

        // Test that failed allocations don't corrupt state
        void* validPtr = allocator.allocate(100, 4);
        CPPUNIT_ASSERT(validPtr != nullptr);

        try
        {
            // Try to allocate huge block (likely to fail)
            void* hugePtr = allocator.allocate(SIZE_MAX / 2, SIZE_MAX / 4);
            if (hugePtr != nullptr)
            {
                allocator.free(hugePtr);
            }
        }
        catch (const std::bad_alloc&)
        {
            // Expected
        }

        // Original allocation should still be valid
        allocator.free(validPtr);

        // New allocations should still work
        void* newPtr = allocator.allocate(200, 8);
        CPPUNIT_ASSERT(newPtr != nullptr);
        allocator.free(newPtr);
    }

    // State Consistency
    void testStateConsistencyAfterErrors()
    {
        DynamicAllocator allocator;

        // Allocate some blocks
        void* ptr1 = allocator.allocate(100, 4);
        void* ptr2 = allocator.allocate(200, 8);

        CPPUNIT_ASSERT(ptr1 != nullptr);
        CPPUNIT_ASSERT(ptr2 != nullptr);

        // Try various error operations
        try
        {
            allocator.free(nullptr);
        }
        catch (const std::logic_error&)
        {
            // Expected
        }

        try
        {
            int dummy;
            allocator.free(&dummy);
        }
        catch (const std::logic_error&)
        {
            // Expected
        }

        // Original allocations should still be valid
        allocator.free(ptr1);
        allocator.free(ptr2);

        // New allocations should work
        void* ptr3 = allocator.allocate(300, 16);
        CPPUNIT_ASSERT(ptr3 != nullptr);
        allocator.free(ptr3);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DynamicAllocatorTest);
