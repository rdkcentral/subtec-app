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


#include "DynamicAllocator.hpp"
#include "Memory.hpp"

#include <memory>
#include <cassert>

namespace dvbsubdecoder
{

DynamicAllocator::~DynamicAllocator()
{
    assert(m_blocks.empty());
}

void* DynamicAllocator::allocate(std::size_t size,
                                 std::size_t alignment)
{
    // try to allocate with default alignment
    {
        std::size_t blockSize = size;
        void* blockPtr = ::operator new(blockSize); // may throw
        void* alignedPtr = blockPtr;
        if (align(alignment, size, alignedPtr, blockSize))
        {
            // success - store & return
            try
            {
                m_blocks.insert(std::make_pair(alignedPtr, blockPtr));
                return alignedPtr;
            }
            catch (...)
            {
                ::operator delete(blockPtr);

                // rethrow
                throw;
            }
        }
        else
        {
            ::operator delete(blockPtr);
        }
    }

    {
        // try to allocate with extended size
        std::size_t extBlockSize = size + alignment;
        void* extBlockPtr = ::operator new(extBlockSize); // may throw
        void* extAlignedPtr = extBlockPtr;
        if (align(alignment, size, extAlignedPtr, extBlockSize))
        {
            // success - store & return
            try
            {
                m_blocks.insert(std::make_pair(extAlignedPtr, extBlockPtr));
                return extAlignedPtr;
            }
            catch (...)
            {
                ::operator delete(extBlockPtr);

                // rethrow
                throw;
            }
        }
        else
        {
            ::operator delete(extBlockPtr);
        }
    }

    throw std::bad_alloc();
}

void DynamicAllocator::free(void* block)
{
    auto iter = m_blocks.find(block);
    if (iter != m_blocks.end())
    {
        ::operator delete(iter->second);
        m_blocks.erase(iter);
    }
    else
    {
        throw std::logic_error("Invalid block pointer passed to free");
    }
}

} // namespace dvbsubdecoder
