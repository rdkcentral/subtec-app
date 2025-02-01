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


#include "BasicAllocator.hpp"
#include "Memory.hpp"

#include <stdexcept>
#include <memory>
#include <subttxrend/common/Logger.hpp>

namespace dvbsubdecoder
{

namespace
{

const std::size_t MARKER_SIZE = 4;
const std::uint8_t ALLOC_MARKER[MARKER_SIZE] =
{ 0x7F, 0xF7, 0x7F, 0xF7 };
const std::uint8_t FREE_MARKER[MARKER_SIZE] =
{ 0x38, 0x38, 0xDB, 0xDB };

subttxrend::common::Logger g_logger("DvbSubDecoder", "BasicAllocator");

template<typename T>
bool checkOnlyOneBitIsSet(T b)
{
    return b && !(b & (b - 1));
}

} // namespace <anonmymous>

BasicAllocator::BasicAllocator()
{
    g_logger.trace("%s", __func__);

    m_allocCount = 0;
    reset();
}

BasicAllocator::~BasicAllocator()
{
    reset();
}

void BasicAllocator::init(void* block,
                          std::size_t size)
{
    g_logger.trace("%s - block=%p size=%zu", __func__, block, size);

    if (m_blockPtr)
    {
        throw std::logic_error("Allocator already initialized");
    }

    m_blockPtr = reinterpret_cast<std::uint8_t*>(block);
    m_blockSize = size;

    m_currentPtr = m_blockPtr;
    m_bytesLeft = size;

    m_allocCount = 0;
}

void BasicAllocator::reset()
{
    g_logger.trace("%s", __func__);

    if (m_allocCount > 0)
    {
        throw std::logic_error("Not all blocks freed");
    }

    m_blockPtr = nullptr;
    m_blockSize = 0U;
    m_currentPtr = nullptr;
    m_bytesLeft = 0U;
}

void* BasicAllocator::allocate(std::size_t size,
                               std::size_t alignment)
{
    g_logger.trace("%s - size=%zu align=%zu", __func__, size, alignment);

    if (!m_currentPtr)
    {
        throw std::logic_error("Allocator not initialized");
    }

    if (m_bytesLeft < size + 4)
    {
        g_logger.info("%s - not enough memory precheck", __func__);

        throw std::bad_alloc();
    }

    void* startPtr = m_currentPtr + 4;
    auto bytesLeft = m_bytesLeft - 4;

    if (alignment == 0)
    {
        alignment = 1;
    }

    if (!checkOnlyOneBitIsSet(alignment))
    {
        throw std::logic_error("Unsupported alignment");
    }

    if (!align(alignment, size, startPtr, bytesLeft))
    {
        g_logger.info("%s - not enough memory after align", __func__);

        throw std::bad_alloc();
    }

    // cast block pointer (after alignment) back to bytes
    auto startBytePtr = reinterpret_cast<std::uint8_t*>(startPtr);

    // find marker position and write the marker
    std::uint8_t* markerPtr = startBytePtr - 4;

    markerPtr[0] = ALLOC_MARKER[0];
    markerPtr[1] = ALLOC_MARKER[1];
    markerPtr[2] = ALLOC_MARKER[2];
    markerPtr[3] = ALLOC_MARKER[3];

    // update current position and update number of bytes left
    m_currentPtr = startBytePtr + size;
    m_bytesLeft = m_blockPtr + m_blockSize - m_currentPtr;

    ++m_allocCount;

    g_logger.trace("%s - size=%zu align=%zu => block=%p", __func__, size,
            alignment, startPtr);

    return startPtr;
}

void BasicAllocator::free(void* block)
{
    g_logger.trace("%s - %p", __func__, block);

    if (!block)
    {
        return;
    }

    if (m_allocCount == 0)
    {
        throw std::logic_error("Unexpected free");
    }

    std::uint8_t* blockPtr = reinterpret_cast<std::uint8_t*>(block);
    std::uint8_t* markerPtr = blockPtr - 4;

    if ((markerPtr[0] != ALLOC_MARKER[0]) && (markerPtr[1] != ALLOC_MARKER[1])
            && (markerPtr[2] != ALLOC_MARKER[2])
            && (markerPtr[3] != ALLOC_MARKER[3]))
    {
        throw std::logic_error("Invalid alloc marker");
    }

    markerPtr[0] = FREE_MARKER[0];
    markerPtr[1] = FREE_MARKER[1];
    markerPtr[2] = FREE_MARKER[2];
    markerPtr[3] = FREE_MARKER[3];

    --m_allocCount;
}

} // namespace dvbsubdecoder
