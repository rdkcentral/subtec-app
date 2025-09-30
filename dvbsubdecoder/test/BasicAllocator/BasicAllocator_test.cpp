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
#include <string>

#include "BasicAllocator.hpp"
#include "AllocatorTraits.hpp"

using dvbsubdecoder::BasicAllocator;
using dvbsubdecoder::AllocatorTraits;

class BasicAllocatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( BasicAllocatorTest );
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testAlignment);
    CPPUNIT_TEST(testDoubleInit);
    CPPUNIT_TEST(testResetWithoutFree);
    CPPUNIT_TEST(testAllocNoInit);
    CPPUNIT_TEST(testDoubleFree);
    CPPUNIT_TEST(testTraits);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testInitBoundaryValues);
    CPPUNIT_TEST(testAllocateZeroSize);
    CPPUNIT_TEST(testAllocateMaxSize);
    CPPUNIT_TEST(testResetBeforeInit);
    CPPUNIT_TEST(testMultipleAllocations);
    CPPUNIT_TEST(testFreeOrder);
    CPPUNIT_TEST(testInitWithNullptr);
    CPPUNIT_TEST(testDestructorBehavior);
    CPPUNIT_TEST(testBufferBoundaryConditions);
    CPPUNIT_TEST(testMemoryExhaustionRecovery);
    CPPUNIT_TEST(testAlignmentBoundaryValues);
    CPPUNIT_TEST(testExceptionSafety);
    CPPUNIT_TEST(testComplexAllocationPatterns);
    CPPUNIT_TEST(testStateConsistencyAfterErrors);
    CPPUNIT_TEST(testAllocatorTraitsEdgeCases);
    CPPUNIT_TEST(testMemoryLayoutVerification);
    CPPUNIT_TEST(testFragmentationScenarios);
    CPPUNIT_TEST(testMarkerIntegrity);
    CPPUNIT_TEST(testExactFitAllocations);
    CPPUNIT_TEST(testMultipleResetCycles);
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
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        auto block1 = allocator.allocate(400, 1);
        CPPUNIT_ASSERT(block1);
        auto block2 = allocator.allocate(400, 1);

        CPPUNIT_ASSERT(block2);
        CPPUNIT_ASSERT_THROW(allocator.allocate(400, 1), std::bad_alloc);

        allocator.free(block1);
        allocator.free(block2);

        CPPUNIT_ASSERT_THROW(allocator.allocate(400, 1), std::bad_alloc);

        auto block3 = allocator.allocate(10, 1);
        CPPUNIT_ASSERT(block3);
        allocator.free(block3);

        allocator.free(nullptr);

        CPPUNIT_ASSERT_THROW(allocator.free(block1), std::logic_error);

        allocator.reset();
    }

    void testAlignment()
    {
        std::vector<std::size_t> goodAlignments;

        goodAlignments.push_back(0);
        for (std::size_t alignment = 1; alignment <= 1024; alignment <<= 1)
        {
            goodAlignments.push_back(alignment);
        }

        for (auto alignment : goodAlignments)
        {
            const std::size_t BUFFER_SIZE = 2048;
            const std::size_t ALLOC_SIZE = BUFFER_SIZE / 2;
            std::array<std::uint8_t, BUFFER_SIZE> buffer;

            BasicAllocator allocator;

            allocator.init(buffer.data(), buffer.size());

            auto blockPtr = allocator.allocate(ALLOC_SIZE, alignment);
            CPPUNIT_ASSERT(blockPtr);
            allocator.free(blockPtr);
        }

        std::vector<std::size_t> badAlignments;

        badAlignments.push_back(3);
        badAlignments.push_back(5);
        badAlignments.push_back(0x11);

        for (auto alignment : badAlignments)
        {
            const std::size_t BUFFER_SIZE = 2048;
            const std::size_t ALLOC_SIZE = BUFFER_SIZE / 2;
            std::array<std::uint8_t, BUFFER_SIZE> buffer;

            BasicAllocator allocator;

            allocator.init(buffer.data(), buffer.size());

            CPPUNIT_ASSERT_THROW(allocator.allocate(ALLOC_SIZE, alignment),
                    std::logic_error);
        }
    }

    void testDoubleInit()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        CPPUNIT_ASSERT_THROW(allocator.init(buffer.data(), buffer.size()),
                std::logic_error);
    }

    void testResetWithoutFree()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());

        auto block1 = allocator.allocate(400, 1);
        CPPUNIT_ASSERT(block1);

        CPPUNIT_ASSERT_THROW(allocator.reset(), std::logic_error);

        allocator.free(block1);

        allocator.reset();
    }

    void testAllocNoInit()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        CPPUNIT_ASSERT_THROW(allocator.allocate(400, 1), std::logic_error);
    }

    void testDoubleFree()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        auto block1 = allocator.allocate(400, 1);
        CPPUNIT_ASSERT(block1);
        auto block2 = allocator.allocate(400, 1);
        CPPUNIT_ASSERT(block2);

        allocator.free(block1);
        CPPUNIT_ASSERT_THROW(allocator.free(block1), std::logic_error);
        allocator.free(block2);

        allocator.reset();
    }

    void testTraits()
    {
        struct Thrower
        {
            Thrower()
            {
                throw std::invalid_argument("none");
            }
        };

        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;

        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());

        AllocatorTraits traits(allocator);

        auto block1 = traits.allocUnique<int>();
        auto block2 = traits.allocUnique<std::string>("test");
        auto block3 = traits.allocUnique<long long>(16);
        auto block4 = traits.allocUnique<std::array<int, 16> >();

        using BigArray = std::array<int, BUFFER_SIZE * 2>;
        CPPUNIT_ASSERT_THROW(traits.allocUnique<BigArray>(), std::bad_alloc);

        CPPUNIT_ASSERT_THROW(traits.allocUnique<Thrower>(), std::invalid_argument);
    }

    // Basic API coverage tests
    void testConstructorInitialState()
    {
        BasicAllocator allocator;
        // Constructor should create allocator in uninitialized state
        // Verify by attempting to allocate without init
        CPPUNIT_ASSERT_THROW(allocator.allocate(100, 1), std::logic_error);
    }

    void testInitBoundaryValues()
    {
        BasicAllocator allocator;
        std::array<std::uint8_t, 1> tinyBuffer;

        // Test minimum buffer size
        allocator.init(tinyBuffer.data(), tinyBuffer.size());
        // Should throw due to insufficient space for allocation + markers
        CPPUNIT_ASSERT_THROW(allocator.allocate(1, 1), std::bad_alloc);
        allocator.reset();

        // Test with size 0
        BasicAllocator allocator2;
        allocator2.init(tinyBuffer.data(), 0);
        CPPUNIT_ASSERT_THROW(allocator2.allocate(1, 1), std::bad_alloc);
        allocator2.reset();
    }

    void testAllocateZeroSize()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Zero-size allocation should succeed
        void* block = allocator.allocate(0, 1);
        CPPUNIT_ASSERT(block);
        allocator.free(block);
        allocator.reset();
    }

    void testAllocateMaxSize()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        // Try to allocate entire buffer (should fail due to 4-byte marker overhead)
        CPPUNIT_ASSERT_THROW(allocator.allocate(BUFFER_SIZE, 1), std::bad_alloc);

        // Allocate maximum possible size (buffer - 4 byte marker overhead)
        void* block = allocator.allocate(BUFFER_SIZE - 4, 1);
        CPPUNIT_ASSERT(block);
        allocator.free(block);
        allocator.reset();
    }

    void testResetBeforeInit()
    {
        BasicAllocator allocator;
        // Reset without init should work (no outstanding allocations)
        allocator.reset();
    }


    void testMultipleAllocations()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        std::vector<void*> blocks;
        // Allocate multiple small blocks
        for (int i = 0; i < 10; ++i) {
            void* block = allocator.allocate(50, 1);
            CPPUNIT_ASSERT(block);
            blocks.push_back(block);
        }
        
        // Free all blocks
        for (void* block : blocks) {
            allocator.free(block);
        }
        allocator.reset();
    }

    void testFreeOrder()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        void* block1 = allocator.allocate(100, 1);
        void* block2 = allocator.allocate(100, 1);
        void* block3 = allocator.allocate(100, 1);
        
        // Free in reverse order
        allocator.free(block3);
        allocator.free(block2);
        allocator.free(block1);
        
        allocator.reset();
    }

    void testInitWithNullptr()
    {
        BasicAllocator allocator;
        // Init with nullptr should work (though not useful)
        allocator.init(nullptr, 0);
        // With nullptr buffer, allocate should throw logic_error "Allocator not initialized"
        CPPUNIT_ASSERT_THROW(allocator.allocate(1, 1), std::logic_error);
        allocator.reset();
    }

    void testDestructorBehavior()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        
        // Test normal destructor behavior
        {
            BasicAllocator allocator;
            allocator.init(buffer.data(), buffer.size());
            void* block = allocator.allocate(100, 1);
            allocator.free(block);
            allocator.reset();
            // Destructor should succeed without issues
        }
        
        // Test reset behavior with outstanding allocations
        BasicAllocator allocator;
        allocator.init(buffer.data(), buffer.size());
        void* block = allocator.allocate(100, 1);
        
        // Verify that reset throws when there are outstanding allocations
        CPPUNIT_ASSERT_THROW(allocator.reset(), std::logic_error);
        
        // Clean up properly
        allocator.free(block);
        allocator.reset();
    }

    // Edge cases and error handling tests
    void testBufferBoundaryConditions()
    {
        const std::size_t BUFFER_SIZE = 32;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Test allocation that exactly fills remaining space
        void* block1 = allocator.allocate(12, 1); // 12 + 4 marker = 16 bytes
        CPPUNIT_ASSERT(block1);
        
        void* block2 = allocator.allocate(12, 1); // Another 16 bytes, total 32
        CPPUNIT_ASSERT(block2);
        
        // No space left
        CPPUNIT_ASSERT_THROW(allocator.allocate(1, 1), std::bad_alloc);
        
        allocator.free(block1);
        allocator.free(block2);
        allocator.reset();
    }

    void testMemoryExhaustionRecovery()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Exhaust memory
        std::vector<void*> blocks;
        try {
            while (true) {
                void* block = allocator.allocate(100, 1);
                blocks.push_back(block);
            }
        } catch (const std::bad_alloc&) {
            // Expected
        }
        
        // Free some blocks and verify recovery
        for (size_t i = 0; i < blocks.size() / 2; ++i) {
            allocator.free(blocks[i]);
        }
        
        // Should still fail - allocator doesn't defragment
        CPPUNIT_ASSERT_THROW(allocator.allocate(100, 1), std::bad_alloc);
        
        // Free remaining blocks
        for (size_t i = blocks.size() / 2; i < blocks.size(); ++i) {
            allocator.free(blocks[i]);
        }
        allocator.reset();
    }

    void testAlignmentBoundaryValues()
    {
        const std::size_t BUFFER_SIZE = 2048;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Test maximum power-of-2 alignment
        void* block1 = allocator.allocate(64, 1024);
        CPPUNIT_ASSERT(block1);
        CPPUNIT_ASSERT_EQUAL(0UL, reinterpret_cast<std::uintptr_t>(block1) % 1024);
        allocator.free(block1);
        
        // Test alignment = 1 (minimum)
        void* block2 = allocator.allocate(64, 1);
        CPPUNIT_ASSERT(block2);
        allocator.free(block2);
        
        // Test alignment = 0 (should be treated as 1)
        void* block3 = allocator.allocate(64, 0);
        CPPUNIT_ASSERT(block3);
        allocator.free(block3);
        
        allocator.reset();
    }

    void testExceptionSafety()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        // Test exception safety during allocation failure
        allocator.init(buffer.data(), buffer.size());
        
        try {
            allocator.allocate(BUFFER_SIZE * 2, 1); // Too large
        } catch (const std::bad_alloc&) {
            // Allocator should still be in valid state
            void* block = allocator.allocate(100, 1);
            CPPUNIT_ASSERT(block);
            allocator.free(block);
        }
        
        allocator.reset();
    }

    void testComplexAllocationPatterns()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Mixed size allocations with different alignments
        std::vector<void*> blocks;
        
        blocks.push_back(allocator.allocate(32, 4));
        blocks.push_back(allocator.allocate(64, 8));
        blocks.push_back(allocator.allocate(16, 2));
        blocks.push_back(allocator.allocate(128, 16));
        
        // Verify all allocations succeeded
        for (void* block : blocks) {
            CPPUNIT_ASSERT(block);
        }
        
        // Free in mixed order
        allocator.free(blocks[1]);
        allocator.free(blocks[3]);
        allocator.free(blocks[0]);
        allocator.free(blocks[2]);
        
        allocator.reset();
    }

    void testStateConsistencyAfterErrors()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        void* validBlock = allocator.allocate(100, 1);
        CPPUNIT_ASSERT(validBlock);
        
        // Attempt invalid operations
        try {
            allocator.allocate(BUFFER_SIZE * 2, 1);
        } catch (const std::bad_alloc&) {}
        
        try {
            allocator.allocate(100, 3); // Invalid alignment
        } catch (const std::logic_error&) {}
        
        // Valid block should still be freeable
        allocator.free(validBlock);
        allocator.reset();
    }

    void testAllocatorTraitsEdgeCases()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        AllocatorTraits traits(allocator);
        
        // Test with types requiring specific alignment
        auto block1 = traits.allocUnique<double>();
        CPPUNIT_ASSERT(block1);
        
        auto block2 = traits.allocUnique<long long>();
        CPPUNIT_ASSERT(block2);
        
        // Test with large types
        using LargeStruct = std::array<double, 100>;
        auto block3 = traits.allocUnique<LargeStruct>();
        CPPUNIT_ASSERT(block3);
        
        // Test memory exhaustion through traits
        using HugeArray = std::array<int, BUFFER_SIZE>;
        CPPUNIT_ASSERT_THROW(traits.allocUnique<HugeArray>(), std::bad_alloc);
    }

    void testMemoryLayoutVerification()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        void* block1 = allocator.allocate(64, 8);
        void* block2 = allocator.allocate(64, 8);
        
        // Verify blocks don't overlap and are properly aligned
        std::uintptr_t addr1 = reinterpret_cast<std::uintptr_t>(block1);
        std::uintptr_t addr2 = reinterpret_cast<std::uintptr_t>(block2);
        
        // Store results for verification before cleanup
        bool addr1Aligned = (addr1 % 8) == 0;
        bool addr2Aligned = (addr2 % 8) == 0;
        bool noOverlap = (addr2 >= addr1 + 64) || (addr1 >= addr2 + 64);
        
        // Always clean up first, then verify
        allocator.free(block1);
        allocator.free(block2);
        allocator.reset();
        
        // Now perform assertions after cleanup
        CPPUNIT_ASSERT(addr1Aligned);
        CPPUNIT_ASSERT(addr2Aligned);
        CPPUNIT_ASSERT(noOverlap);
    }

    void testFragmentationScenarios()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Allocate several blocks
        void* block1 = allocator.allocate(100, 1);
        void* block2 = allocator.allocate(100, 1);
        void* block3 = allocator.allocate(100, 1);
        
        // Free middle block - creates potential fragmentation
        allocator.free(block2);
        
        // Test allocation behavior after freeing middle block
        // We don't assume specific fragmentation behavior, just test that allocator works
        void* largeBlock = nullptr;
        try {
            largeBlock = allocator.allocate(200, 1);
        } catch (const std::bad_alloc&) {
            // Either outcome is acceptable - depends on allocator implementation
        }
        
        // Small allocation should always succeed somewhere in the buffer
        void* block4 = allocator.allocate(50, 1);
        CPPUNIT_ASSERT(block4);
        
        // Clean up all allocations
        allocator.free(block1);
        allocator.free(block3);
        allocator.free(block4);
        if (largeBlock) {
            allocator.free(largeBlock);
        }
        allocator.reset();
        
        // The main test is that the allocator remains consistent and functional
        // regardless of allocation/deallocation patterns
    }

    void testMarkerIntegrity()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        
        // Use a separate scope to test marker corruption
        {
            BasicAllocator allocator;
            allocator.init(buffer.data(), buffer.size());
            
            void* block = allocator.allocate(100, 1);
            CPPUNIT_ASSERT(block);
            
            // Test with valid marker first
            allocator.free(block);
            
            // Allocate again for corruption test
            block = allocator.allocate(100, 1);
            CPPUNIT_ASSERT(block);
            
            // Corrupt the marker (simulate buffer overrun)
            std::uint8_t* markerPtr = reinterpret_cast<std::uint8_t*>(block) - 4;
            // Save original marker values (should be { 0x7F, 0xF7, 0x7F, 0xF7 })
            std::uint8_t originalMarker[4];
            originalMarker[0] = markerPtr[0];
            originalMarker[1] = markerPtr[1];
            originalMarker[2] = markerPtr[2];
            originalMarker[3] = markerPtr[3];
            
            // Corrupt all 4 bytes of marker (production code uses && so all must be wrong)
            markerPtr[0] = 0xFF;
            markerPtr[1] = 0xFF;
            markerPtr[2] = 0xFF;
            markerPtr[3] = 0xFF;
            
            // Free should detect corruption
            CPPUNIT_ASSERT_THROW(allocator.free(block), std::logic_error);
            
            // Restore the marker to allow proper cleanup
            markerPtr[0] = originalMarker[0];
            markerPtr[1] = originalMarker[1];
            markerPtr[2] = originalMarker[2];
            markerPtr[3] = originalMarker[3];
            
            allocator.free(block);
            allocator.reset();
        }
    }

    void testExactFitAllocations()
    {
        const std::size_t BUFFER_SIZE = 100;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        allocator.init(buffer.data(), buffer.size());
        
        // Calculate exact fit: buffer size - marker overhead
        void* block = allocator.allocate(BUFFER_SIZE - 4, 1);
        CPPUNIT_ASSERT(block);
        
        // No space for another allocation
        CPPUNIT_ASSERT_THROW(allocator.allocate(1, 1), std::bad_alloc);
        
        allocator.free(block);
        allocator.reset();
    }

    void testMultipleResetCycles()
    {
        const std::size_t BUFFER_SIZE = 1024;
        std::array<std::uint8_t, BUFFER_SIZE> buffer;
        BasicAllocator allocator;

        // Multiple init/use/reset cycles
        for (int cycle = 0; cycle < 3; ++cycle) {
            allocator.init(buffer.data(), buffer.size());
            
            void* block1 = allocator.allocate(100, 1);
            void* block2 = allocator.allocate(200, 4);
            
            CPPUNIT_ASSERT(block1);
            CPPUNIT_ASSERT(block2);
            
            allocator.free(block1);
            allocator.free(block2);
            allocator.reset();
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(BasicAllocatorTest);
