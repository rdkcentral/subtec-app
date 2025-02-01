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


#ifndef DVBSUBDECODER_BASICALLOCATOR_HPP_
#define DVBSUBDECODER_BASICALLOCATOR_HPP_

#include <exception>

#include "Allocator.hpp"

namespace dvbsubdecoder
{

/**
 * Basic allocator allocating memory from the provided memory block.
 *
 * @note    Free does not release memory (it cannot be reused) - it is only
 *          marked as freed until reset.
 * @note    The allocator is not initialized after construction and cannot be
 *          used before initialization.
 * @note    Double initialization is forbidden.
 * @note    Reset is only allowed after all allocated blocks were freed.
 * @note    An exception of type std::logic_error is thrown on invalid usage.
 */
class BasicAllocator : public Allocator
{
    BasicAllocator(const BasicAllocator&) = delete;
    BasicAllocator& operator =(const BasicAllocator&) = delete;

public:
    /**
     * Constructor.
     */
    BasicAllocator();

    /**
     * Destructor.
     *
     * Resets the allocator and releases it.
     */
    virtual ~BasicAllocator();

    /**
     * Initializes the allocator.
     *
     * @param block
     *      Block of memory to be used.
     * @param size
     *      Size of the given memory block.
     */
    void init(void* block,
              std::size_t size);

    /**
     * Resets the allocator.
     *
     * Checks if all allocated blocks were released before reset.
     */
    void reset();

    /** @copydoc Allocator::allocate */
    virtual void* allocate(std::size_t size,
                           std::size_t alignment) override;

    /** @copydoc Allocator::free */
    virtual void free(void* block) override;

private:
    /** Memory block pointer. */
    std::uint8_t* m_blockPtr;

    /** Memory block size. */
    std::size_t m_blockSize;

    /** Current pointer. */
    std::uint8_t* m_currentPtr;

    /** Bytes left. */
    std::size_t m_bytesLeft;

    /** Allocs count. */
    std::size_t m_allocCount;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_BASICALLOCATOR_HPP_*/
