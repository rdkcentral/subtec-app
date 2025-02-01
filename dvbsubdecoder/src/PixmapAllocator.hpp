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


#ifndef DVBSUBDECODER_PIXMAPALLOCATOR_HPP_
#define DVBSUBDECODER_PIXMAPALLOCATOR_HPP_

#include <memory>
#include <stdexcept>
#include "DecoderClient.hpp"

namespace dvbsubdecoder
{

/**
 * Allocator for pixmap memory.
 */
class PixmapAllocator
{
    /**
     * Graphics memory pointer deleter.
     */
    class GfxMemoryDeleter
    {
    public:
        /**
         * Constructor.
         *
         * Deleter not associates with any client.
         */
        GfxMemoryDeleter() :
                m_client(nullptr)
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param client
         *      Client interface to release the memory.
         */
        GfxMemoryDeleter(DecoderClient* client) :
                m_client(client)
        {
            // noop
        }

        /**
         * Operator () - release memory.
         *
         * @param memory
         *      Block of memory to be released.
         */
        void operator ()(void* memory)
        {
            if (memory)
            {
                if (m_client)
                {
                    m_client->gfxFree(memory);
                }
                else
                {
                    throw std::logic_error("Uninitialized deleter");
                }
            }
        }

    private:
        /** Pointer to client. */
        DecoderClient* m_client;
    };

    /** Unique pointer to gfx memory. */
    using GfxUniquePtr = std::unique_ptr<void, GfxMemoryDeleter>;

public:
    /**
     * Minimum buffer size.
     */
    static const std::size_t BUFFER_SIZE_MIN = 80 * 1024;

    /**
     * Buffer allocation size step.
     */
    static const std::size_t BUFFER_SIZE_STEP = 80 * 1024;

    /**
     * Maximum buffer for 1.2.1 specification.
     *
     * Spec defines it as mode independent, but this decoder
     * stores 2 bit as 8 bit so must be multiplied by 4.
     */
    static const std::size_t BUFFER_SIZE_121 = 80 * 1024 * 4;

    /**
     * Maximum buffer for 1.3.1 specification.
     *
     * Spec defines it as mode independent, but this decoder
     * stores 2 bit as 8 bit so must be multiplied by 4.
     */
    static const std::size_t BUFFER_SIZE_131 = 320 * 1024 * 4;

    /**
     * Constructor.
     *
     * @param specVersion
     *      Selected specification version.
     * @param client
     *      Decoder client interface.
     */
    PixmapAllocator(Specification specVersion,
                    DecoderClient& client);

    /**
     * Destructor.
     */
    ~PixmapAllocator();

    /**
     * Resets the allocator.
     *
     * @warning Any memory allocated before call to reset is invalidated
     *          and must not be used anomore.
     */
    void reset();

    /**
     * Checks if pixmap of requested size could be allocated.
     *
     * @param size
     *      Requested size.
     *
     * @return
     *      True if pixmap could be allocated, false otherwise.
     */
    bool canAllocate(std::size_t size) const;

    /**
     * Allocates pixmap of requested size.
     *
     * @param size
     *      Requested size.
     *
     * @return
     *      Pointer to memory allocted for pixmap on success,
     *      nullptr if there is no memory available.
     */
    std::uint8_t* allocate(std::size_t size);

private:
    /**
     * Decoder client interface.
     *
     * The interface includes graphics memory allocation methods.
     */
    DecoderClient& m_client;

    /**
     * Pointer to allocated memory block.
     */
    GfxUniquePtr m_blockPtr;

    /**
     * Size of the allocated memory block.
     */
    std::size_t m_blockSize;

    /**
     * Current pointer.
     *
     * Points on memory location that will be returned on next allocation.
     */
    std::uint8_t* m_currentPtr;

    /**
     * Number of bytes left in block.
     */
    std::size_t m_bytesLeft;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PIXMAPALLOCATOR_HPP_*/
