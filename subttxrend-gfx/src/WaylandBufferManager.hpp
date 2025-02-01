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


#ifndef SUBTTXREND_GFX_WAYLAND_BUFFER_MANAGER_HPP_
#define SUBTTXREND_GFX_WAYLAND_BUFFER_MANAGER_HPP_

#include <vector>

#include <subttxrend/common/NonCopyable.hpp>

#include "Types.hpp"
#include "WaylandBuffer.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Wayland buffer manager.
 */
class WaylandBufferManager : public subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param shm
     *      Wayland SHM object.
     */
    WaylandBufferManager(waylandcpp::Shm1::Ptr shm);

    /**
     * Destructor.
     */
    virtual ~WaylandBufferManager();

    /**
     * Requests buffer of a specific size.
     *
     * @param size
     *      Size of the requested buffer.
     *
     * @return
     *      Buffer if available, null pointer otherwise.
     */
    WaylandBuffer::Ptr getBuffer(const Size& size);

private:
    /** Buffer entry. */
    struct BufferEntry
    {
        /**
         * Constructor.
         */
        BufferEntry() = default;

        /**
         * Constructor.
         *
         * @param pool
         *      Pool pointer.
         * @param buffer
         *      Buffer pointer.
         */
        BufferEntry(const waylandcpp::ShmPool1::Ptr& pool,
                    const WaylandBuffer::Ptr& buffer) :
                m_pool(pool),
                m_buffer(buffer)
        {
            // noop
        }

        /** Pool pointer. */
        waylandcpp::ShmPool1::Ptr m_pool;

        /** Buffer pointer. */
        WaylandBuffer::Ptr m_buffer;
    };

    /**
     * Requests regular buffer of a specific size.
     *
     * @param size
     *      Size of the requested buffer.
     *
     * @return
     *      Buffer if available, null pointer otherwise.
     */
    WaylandBuffer::Ptr getRegularBuffer(const Size& size);

    /**
     * Requests empty buffer.
     *
     * The empty buffer is transparent and has minimum size.
     * It is expected to be used when there is no content to be rendered
     * as attaching the NULL surface does not work as expected (forces
     * surface hide but without re-showing when new buffer is attached).
     *
     * @return
     *      Empty buffer if available, null pointer otherwise.
     */
    WaylandBuffer::Ptr getEmptyBuffer();

    /**
     * Creates empty buffer.
     *
     * @return
     *      Created buffer or nullptr on error.
     */
    WaylandBuffer::Ptr createEmptyBuffer();

    /**
     * Creates memory pool.
     *
     * @param size
     *      Size of the memory pool.
     *
     * @return
     *      Creates pool or nullptr on error.
     */
    waylandcpp::ShmPool1::Ptr createPool(std::size_t size);

    /**
     * Finds regular buffer.
     *
     * The buffer must not be currently used and must suit the given
     * params.
     *
     * @param params
     *      Requested parameters.
     *
     * @return
     *      Buffer found or nullptr.
     */
    WaylandBuffer::Ptr findBuffer(const waylandcpp::BufferParams& params);

    /**
     * Finds pool to be reused.
     *
     * The pool is removed from pool list if matches.
     * All pools smaller than requested parameters are also freed.
     *
     * @param params
     *      Requested parameters.
     *
     * @return
     *      Pool found or nullptr.
     */
    waylandcpp::ShmPool1::Ptr findPool(const waylandcpp::BufferParams& params);

    /**
     * Creates buffer.
     *
     * @param shmPool
     *      Pool to use.
     * @param params
     *      Requested params.
     * @param empty
     *      If true the buffer is marked as 'empty' and the
     *      contents are made transparent.
     *
     * @return
     *      Created buffer or nullptr on error.
     */
    WaylandBuffer::Ptr createBuffer(const waylandcpp::ShmPool1::Ptr& shmPool,
                                         const waylandcpp::BufferParams& params,
                                         bool empty);

    /** SHM object to use. */
    waylandcpp::Shm1::Ptr m_shm;

    /** Colection of 'empty' buffers. */
    std::vector<WaylandBuffer::Ptr> m_emptyBuffers;

    /** Colection of regular buffers. */
    std::vector<BufferEntry> m_regularBuffers;

    /** Pools. */
    std::vector<waylandcpp::ShmPool1::Ptr> m_pools;

};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_BUFFER_MANAGER_HPP_
