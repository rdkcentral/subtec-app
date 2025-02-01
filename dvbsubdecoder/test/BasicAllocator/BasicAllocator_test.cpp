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
    CPPUNIT_TEST(testTraits);CPPUNIT_TEST_SUITE_END()
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(BasicAllocatorTest);
