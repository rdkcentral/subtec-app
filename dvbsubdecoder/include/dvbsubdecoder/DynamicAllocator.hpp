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


#ifndef DVBSUBDECODER_DYNAMICALLOCATOR_HPP_
#define DVBSUBDECODER_DYNAMICALLOCATOR_HPP_

#include <exception>
#include <map>

#include "Allocator.hpp"

namespace dvbsubdecoder
{

/**
 * Allocator that reserves the blocks dynamically.
 */
class DynamicAllocator : public Allocator
{
    DynamicAllocator(const DynamicAllocator&) = delete;
    DynamicAllocator& operator =(const DynamicAllocator&) = delete;

public:
    /**
     * Constructor.
     */
    DynamicAllocator() = default;

    /**
     * Destructor.
     *
     * Resets the allocator and releases it.
     */
    virtual ~DynamicAllocator();

    /** @copydoc Allocator::allocate */
    virtual void* allocate(std::size_t size,
                           std::size_t alignment) override;

    /** @copydoc Allocator::free */
    virtual void free(void* block) override;

private:
    /** Map with allocated blocks. */
    std::map<void*, void*> m_blocks;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DYNAMICALLOCATOR_HPP_*/
