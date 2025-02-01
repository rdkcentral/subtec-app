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


#ifndef TTXDECODER_ALLOCATOR_HPP_
#define TTXDECODER_ALLOCATOR_HPP_

#include <cstdint>
#include <exception>
#include <memory>
#include <subttxrend/common/NonCopyable.hpp>

namespace ttxdecoder
{

/**
 * Allocator interface.
 */
class Allocator
{
public:
    /**
     * Constructor.
     */
    Allocator() = default;

    /**
     * Destructor.
     */
    virtual ~Allocator() = default;

    /**
     * Allocate block of memory.
     *
     * @param size
     *      Number of bytes to allocate.
     *
     * @return
     *      Allocated block or nullptr on error.
     */
    virtual std::uint8_t* alloc(std::size_t size) = 0;

    /**
     * Returns free memory size.
     *
     * @return
     *      Free memory size in bytes.
     */
    virtual std::size_t getFreeSize() = 0;
};

/**
 * Allocator based on memory block.
 */
class MemoryBlockAllocator : public Allocator,
                             private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param block
     *      Memory block to use.
     * @param size
     *      Size of the given memory block.
     */
    MemoryBlockAllocator(void* block,
                         std::uint32_t size) :
            m_block(reinterpret_cast<std::uint8_t*>(block)),
            m_size(size),
            m_used(0)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~MemoryBlockAllocator() = default;

    /** @copydoc Allocator::alloc */
    virtual std::uint8_t* alloc(std::size_t size) override
    {
        if (getFreeSize() < size)
        {
            throw std::bad_alloc();
        }

        std::uint8_t* buffer = m_block + m_used;

        m_used += size;

        return buffer;
    }

    /** @copydoc Allocator::getFreeSize */
    virtual std::size_t getFreeSize() override
    {
        return m_size - m_used;
    }

private:
    /** Memory block to use. */
    std::uint8_t* m_block;
    /** Size of the memory block in bytes. */
    std::uint32_t m_size;
    /** Number of currnetly used bytes. */
    std::uint32_t m_used;
};

/**
 * Standard allocator.
 *
 * The allocator is used when user does not provide memory block to use.
 */
class StandardAllocator : public Allocator,
                          private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param size
     *      Memory size to use (in bytes).
     */
    StandardAllocator(size_t size) :
            m_buffer(new std::uint8_t[size]),
            m_peer(m_buffer.get(), size)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~StandardAllocator() = default;

    /** @copydoc Allocator::alloc */
    virtual std::uint8_t* alloc(std::size_t size) override
    {
        return m_peer.alloc(size);
    }

    /** @copydoc Allocator::getFreeSize */
    virtual std::size_t getFreeSize() override
    {
        return m_peer.getFreeSize();
    }

private:
    /** Memory block that will be used. */
    std::unique_ptr<std::uint8_t> m_buffer;

    /** Peer allocator created for the buffer allocated. */
    MemoryBlockAllocator m_peer;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_ALLOCATOR_HPP_*/
