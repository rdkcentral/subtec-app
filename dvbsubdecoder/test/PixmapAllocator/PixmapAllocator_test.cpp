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

#include "PixmapAllocator.hpp"
#include "Misc.hpp"
#include "DecoderClientMock.hpp"
#include <limits>

using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;

class PixmapAllocatorTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( PixmapAllocatorTest );
    CPPUNIT_TEST(test121);
    CPPUNIT_TEST(test131);
    CPPUNIT_TEST(testUnsupportedSpecVersion);
    CPPUNIT_TEST(testConstructorWithInsufficientMemory);
    CPPUNIT_TEST(testConstructorStepDownAllocation);
    CPPUNIT_TEST(testAllocationBeforeReset);
    CPPUNIT_TEST(testCanAllocateBeforeReset);
    CPPUNIT_TEST(testExactBlockSizeAllocation);
    CPPUNIT_TEST(testOversizeAllocation);
    CPPUNIT_TEST(testZeroSizeAllocation);
    CPPUNIT_TEST(testResetBehavior);
    CPPUNIT_TEST(testMultipleResets);
    CPPUNIT_TEST(testReuseAfterExhaustion);
    CPPUNIT_TEST(testPointerMonotonicity);
    CPPUNIT_TEST(testProgressiveExhaustion);
    CPPUNIT_TEST(testPartialAllocationRemainder);
    CPPUNIT_TEST(testFailureDoesNotMutateState);
    CPPUNIT_TEST(testMixedSizeAllocations);
    CPPUNIT_TEST(testCanAllocateVsAllocateConsistency);
    CPPUNIT_TEST(testLargeNumberSmallAllocations);
    CPPUNIT_TEST(testVeryLargeSizeRequest);
    CPPUNIT_TEST(testPartialCapacityStepDown);
    CPPUNIT_TEST(testAllocationFailsBelowMinimum);
    CPPUNIT_TEST(testSpecVersionSizeLimits);
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

    void test131()
    {
        DecoderClientMock client;

        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);

        PixmapAllocator allocator131(Specification::VERSION_1_3_1, client);

        std::vector<std::uint8_t*> chunks;

        std::size_t totalSize = 0;

        const auto ALLOC_STEP = 1024;

        CPPUNIT_ASSERT(!allocator131.canAllocate(ALLOC_STEP));

        allocator131.reset();

        CPPUNIT_ASSERT(allocator131.canAllocate(ALLOC_STEP));

        for (;;)
        {
            if (allocator131.canAllocate(ALLOC_STEP))
            {
                auto chunk = allocator131.allocate(ALLOC_STEP);
                CPPUNIT_ASSERT(chunk);

                chunks.push_back(chunk);

                totalSize += ALLOC_STEP;
            }
            else
            {
                break;
            }
        }

        CPPUNIT_ASSERT(totalSize <= client.getAllocTotal());

        allocator131.reset();
        chunks.clear();

        for (;;)
        {
            auto chunk = allocator131.allocate(ALLOC_STEP);
            if (chunk)
            {
                chunks.push_back(chunk);

                totalSize += ALLOC_STEP;
            }
            else
            {
                break;
            }
        }

        allocator131.reset();
    }

    void test121()
    {
        DecoderClientMock client;

        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator121(Specification::VERSION_1_2_1, client);

        std::vector<std::uint8_t*> chunks;

        std::size_t totalSize = 0;

        const auto ALLOC_STEP = 1024;

        CPPUNIT_ASSERT(!allocator121.canAllocate(ALLOC_STEP));

        allocator121.reset();

        CPPUNIT_ASSERT(allocator121.canAllocate(ALLOC_STEP));

        for (;;)
        {
            if (allocator121.canAllocate(ALLOC_STEP))
            {
                auto chunk = allocator121.allocate(ALLOC_STEP);
                CPPUNIT_ASSERT(chunk);

                chunks.push_back(chunk);

                totalSize += ALLOC_STEP;
            }
            else
            {
                break;
            }
        }

        CPPUNIT_ASSERT(totalSize <= client.getAllocTotal());

        allocator121.reset();
        chunks.clear();

        for (;;)
        {
            auto chunk = allocator121.allocate(ALLOC_STEP);
            if (chunk)
            {
                chunks.push_back(chunk);

                totalSize += ALLOC_STEP;
            }
            else
            {
                break;
            }
        }

        allocator121.reset();
    }

    void testUnsupportedSpecVersion()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);

        // Cast to invalid enum value to test default case
        auto invalidSpec = static_cast<Specification>(999);
        
        CPPUNIT_ASSERT_THROW(PixmapAllocator(invalidSpec, client), std::logic_error);
    }

    void testConstructorWithInsufficientMemory()
    {
        DecoderClientMock client;
        
        // Set alloc limit to 0 to force all allocations to fail
        client.setAllocLimit(0);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);

        // After construction with failed allocation, nothing should be allocatable
        allocator.reset();
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testConstructorStepDownAllocation()
    {
        DecoderClientMock client;
        
        // Set limit to force step-down but still allow minimum size
        std::size_t limitSize = PixmapAllocator::BUFFER_SIZE_MIN + PixmapAllocator::BUFFER_SIZE_STEP;
        client.setAllocLimit(limitSize);

        PixmapAllocator allocator(Specification::VERSION_1_3_1, client);

        allocator.reset();
        
        // Should be able to allocate something less than the original max
        CPPUNIT_ASSERT(allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_MIN));
        
        // But not the full 1.3.1 buffer size
        CPPUNIT_ASSERT(!allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_131));
    }

    void testAllocationBeforeReset()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);

        // Before reset, allocation should return nullptr
        CPPUNIT_ASSERT(allocator.allocate(1024) == nullptr);
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testCanAllocateBeforeReset()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);

        // Before reset, canAllocate should return false
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(!allocator.canAllocate(1024));
    }

    void testExactBlockSizeAllocation()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Allocate exact block size
        auto blockSize = PixmapAllocator::BUFFER_SIZE_121;
        CPPUNIT_ASSERT(allocator.canAllocate(blockSize));
        
        auto ptr = allocator.allocate(blockSize);
        CPPUNIT_ASSERT(ptr != nullptr);

        // After exact allocation, no more should be available
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testOversizeAllocation()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Request larger than block size should fail
        auto oversizeRequest = PixmapAllocator::BUFFER_SIZE_121 + 1;
        CPPUNIT_ASSERT(!allocator.canAllocate(oversizeRequest));
        CPPUNIT_ASSERT(allocator.allocate(oversizeRequest) == nullptr);

        // State should remain unchanged - smaller allocation should still work
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        CPPUNIT_ASSERT(allocator.allocate(1024) != nullptr);
    }

    void testZeroSizeAllocation()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Zero-size allocation should succeed without advancing pointer
        CPPUNIT_ASSERT(allocator.canAllocate(0));
        auto ptr1 = allocator.allocate(0);
        auto ptr2 = allocator.allocate(0);
        
        // Both should return same pointer (no advancement)
        CPPUNIT_ASSERT(ptr1 == ptr2);
        
        // Regular allocation should still work normally
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        auto ptr3 = allocator.allocate(1024);
        CPPUNIT_ASSERT(ptr3 == ptr1); // Should start from same base
    }

    void testResetBehavior()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        
        // Initial state
        CPPUNIT_ASSERT(!allocator.canAllocate(1024));
        
        allocator.reset();
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        
        // Use some memory
        auto ptr1 = allocator.allocate(1024);
        CPPUNIT_ASSERT(ptr1 != nullptr);
        
        // Reset should restore full capacity
        allocator.reset();
        CPPUNIT_ASSERT(allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_121));
        
        // New allocation should start from beginning again
        auto ptr2 = allocator.allocate(1024);
        CPPUNIT_ASSERT(ptr2 == ptr1); // Same base pointer
    }

    void testMultipleResets()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        
        // Multiple resets should be safe
        allocator.reset();
        allocator.reset();
        allocator.reset();
        
        // Should still work normally
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        CPPUNIT_ASSERT(allocator.allocate(1024) != nullptr);
    }

    void testReuseAfterExhaustion()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Exhaust all memory
        while (allocator.canAllocate(1024))
        {
            allocator.allocate(1024);
        }
        
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        
        // Reset and verify full capacity restored
        allocator.reset();
        CPPUNIT_ASSERT(allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_121));
        CPPUNIT_ASSERT(allocator.allocate(1024) != nullptr);
    }

    void testPointerMonotonicity()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        std::vector<std::uint8_t*> pointers;
        
        // Allocate several chunks
        for (int i = 0; i < 10; ++i)
        {
            auto ptr = allocator.allocate(1024);
            CPPUNIT_ASSERT(ptr != nullptr);
            pointers.push_back(ptr);
        }

        // Verify pointers are monotonically increasing and non-overlapping
        for (size_t i = 1; i < pointers.size(); ++i)
        {
            CPPUNIT_ASSERT(pointers[i] > pointers[i-1]);
            CPPUNIT_ASSERT(pointers[i] >= pointers[i-1] + 1024); // No overlap
        }
    }

    void testProgressiveExhaustion()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        std::size_t totalAllocated = 0;
        const std::size_t chunkSize = 1024;

        // Allocate until exhaustion
        while (allocator.canAllocate(chunkSize))
        {
            auto ptr = allocator.allocate(chunkSize);
            CPPUNIT_ASSERT(ptr != nullptr);
            totalAllocated += chunkSize;
        }

        // Should have allocated close to but not exceeding block size
        CPPUNIT_ASSERT(totalAllocated <= PixmapAllocator::BUFFER_SIZE_121);
        CPPUNIT_ASSERT(totalAllocated > PixmapAllocator::BUFFER_SIZE_121 - chunkSize);

        // No further allocation should be possible
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testPartialAllocationRemainder()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Allocate most of the block, leaving a small remainder
        std::size_t largeSize = PixmapAllocator::BUFFER_SIZE_121 - 100;
        auto ptr1 = allocator.allocate(largeSize);
        CPPUNIT_ASSERT(ptr1 != nullptr);

        // Should be able to allocate the exact remainder
        CPPUNIT_ASSERT(allocator.canAllocate(100));
        auto ptr2 = allocator.allocate(100);
        CPPUNIT_ASSERT(ptr2 != nullptr);

        // Now nothing should be left
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testFailureDoesNotMutateState()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Allocate some memory
        auto ptr1 = allocator.allocate(1024);
        CPPUNIT_ASSERT(ptr1 != nullptr);

        // Verify remaining capacity
        CPPUNIT_ASSERT(allocator.canAllocate(1024));

        // Try to allocate too much - should fail
        auto oversizeRequest = PixmapAllocator::BUFFER_SIZE_121;
        CPPUNIT_ASSERT(!allocator.canAllocate(oversizeRequest));
        CPPUNIT_ASSERT(allocator.allocate(oversizeRequest) == nullptr);

        // State should be unchanged - normal allocation should still work
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        auto ptr2 = allocator.allocate(1024);
        CPPUNIT_ASSERT(ptr2 != nullptr);
        CPPUNIT_ASSERT(ptr2 > ptr1);
    }

    void testMixedSizeAllocations()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        std::vector<std::size_t> sizes = {512, 2048, 256, 4096, 128};
        std::size_t totalAllocated = 0;

        for (auto size : sizes)
        {
            if (allocator.canAllocate(size))
            {
                auto ptr = allocator.allocate(size);
                CPPUNIT_ASSERT(ptr != nullptr);
                totalAllocated += size;
            }
        }

        // Verify total doesn't exceed block size
        CPPUNIT_ASSERT(totalAllocated <= PixmapAllocator::BUFFER_SIZE_121);
    }

    void testCanAllocateVsAllocateConsistency()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        std::vector<std::size_t> testSizes = {1, 512, 1024, 2048, 8192};

        for (auto size : testSizes)
        {
            bool canAlloc = allocator.canAllocate(size);
            auto ptr = allocator.allocate(size);

            if (canAlloc)
            {
                CPPUNIT_ASSERT(ptr != nullptr);
            }
            else
            {
                CPPUNIT_ASSERT(ptr == nullptr);
                break; // Stop testing once we hit failure
            }
        }
    }

    void testLargeNumberSmallAllocations()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        std::size_t count = 0;
        std::size_t totalSize = 0;
        const std::size_t smallSize = 64;

        // Allocate many small chunks
        while (allocator.canAllocate(smallSize))
        {
            auto ptr = allocator.allocate(smallSize);
            CPPUNIT_ASSERT(ptr != nullptr);
            count++;
            totalSize += smallSize;
        }

        // Verify arithmetic consistency
        CPPUNIT_ASSERT(totalSize == count * smallSize);
        CPPUNIT_ASSERT(totalSize <= PixmapAllocator::BUFFER_SIZE_121);
        
        // Should have allocated a significant number
        CPPUNIT_ASSERT(count > 100);
    }

    void testVeryLargeSizeRequest()
    {
        DecoderClientMock client;
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Request SIZE_MAX - should fail gracefully
        auto hugeSize = std::numeric_limits<std::size_t>::max();
        CPPUNIT_ASSERT(!allocator.canAllocate(hugeSize));
        CPPUNIT_ASSERT(allocator.allocate(hugeSize) == nullptr);

        // Normal allocation should still work
        CPPUNIT_ASSERT(allocator.canAllocate(1024));
        CPPUNIT_ASSERT(allocator.allocate(1024) != nullptr);
    }

    void testPartialCapacityStepDown()
    {
        DecoderClientMock client;
        
        // Set limit between min and step to force exactly one step-down
        std::size_t limitSize = PixmapAllocator::BUFFER_SIZE_MIN + (PixmapAllocator::BUFFER_SIZE_STEP / 2);
        client.setAllocLimit(limitSize);

        PixmapAllocator allocator(Specification::VERSION_1_3_1, client);
        allocator.reset();

        // Should have stepped down from BUFFER_SIZE_131 to something smaller
        CPPUNIT_ASSERT(!allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_131));
        CPPUNIT_ASSERT(allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_MIN));
        
        // Should be able to allocate at least the minimum size
        auto ptr = allocator.allocate(PixmapAllocator::BUFFER_SIZE_MIN);
        CPPUNIT_ASSERT(ptr != nullptr);
    }

    void testAllocationFailsBelowMinimum()
    {
        DecoderClientMock client;
        
        // Set limit below minimum to force complete failure
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_MIN - 1);

        PixmapAllocator allocator(Specification::VERSION_1_2_1, client);
        allocator.reset();

        // Nothing should be allocatable
        CPPUNIT_ASSERT(!allocator.canAllocate(1));
        CPPUNIT_ASSERT(!allocator.canAllocate(PixmapAllocator::BUFFER_SIZE_MIN));
        CPPUNIT_ASSERT(allocator.allocate(1) == nullptr);
    }

    void testSpecVersionSizeLimits()
    {
        DecoderClientMock client;

        // Test 1.2.1 specification limits
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_121);
        {
            PixmapAllocator allocator121(Specification::VERSION_1_2_1, client);
            allocator121.reset();
            
            // Should be able to allocate up to 1.2.1 size
            CPPUNIT_ASSERT(allocator121.canAllocate(PixmapAllocator::BUFFER_SIZE_121));
            
            // But not 1.3.1 size (if limit allows)
            if (PixmapAllocator::BUFFER_SIZE_131 > PixmapAllocator::BUFFER_SIZE_121)
            {
                CPPUNIT_ASSERT(!allocator121.canAllocate(PixmapAllocator::BUFFER_SIZE_131));
            }
        }

        // Test 1.3.1 specification limits
        client.setAllocLimit(PixmapAllocator::BUFFER_SIZE_131);
        {
            PixmapAllocator allocator131(Specification::VERSION_1_3_1, client);
            allocator131.reset();
            
            // Should be able to allocate up to 1.3.1 size
            CPPUNIT_ASSERT(allocator131.canAllocate(PixmapAllocator::BUFFER_SIZE_131));
            CPPUNIT_ASSERT(allocator131.canAllocate(PixmapAllocator::BUFFER_SIZE_121));
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PixmapAllocatorTest);
