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

using dvbsubdecoder::PixmapAllocator;
using dvbsubdecoder::Specification;

class RenderingStateTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( RenderingStateTest );
    CPPUNIT_TEST(test121);
    CPPUNIT_TEST(test131);CPPUNIT_TEST_SUITE_END()
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

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(RenderingStateTest);
