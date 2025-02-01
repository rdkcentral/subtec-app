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


#include "WaylandBufferManager.hpp"

#include <subttxrend/common/Logger.hpp>

#include "Blitter.hpp"
#include "Pixmap.hpp"
#include "waylandcpp-client/File.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

subttxrend::common::Logger g_logger("Gfx", "WaylandBufferManager");

} // namespace <anonymous>

WaylandBufferManager::WaylandBufferManager(waylandcpp::Shm1::Ptr shm) :
        m_shm(shm)
{
    // noop
}

WaylandBufferManager::~WaylandBufferManager()
{
    // noop
}

WaylandBuffer::Ptr WaylandBufferManager::getBuffer(const Size& size)
{
    g_logger.trace("%s - %d x %d", __func__, size.m_w, size.m_h);

    if ((size.m_w == 0) || (size.m_h == 0))
    {
        return getEmptyBuffer();
    }
    else
    {
        return getRegularBuffer(size);
    }
}

WaylandBuffer::Ptr WaylandBufferManager::getRegularBuffer(const Size& size)
{
    g_logger.trace("%s - %d x %d", __func__, size.m_w, size.m_h);

    waylandcpp::BufferParams params(0, size.m_w, size.m_h,
            waylandcpp::PixelFormat::ARGB8888);

    // try to find matching buffer which is not in use
    auto buffer = findBuffer(params);
    if (buffer)
    {
        g_logger.trace("%s - found buffer: %p", __func__, buffer.get());
        return buffer;
    }

    // try to find pool with enough memory
    auto shmPool = findPool(params);

    // no pools found, allocate new pool and buffer
    if (shmPool)
    {
        g_logger.trace("%s - found pool to reuse: %p", __func__, shmPool.get());
    }
    else
    {
        const waylandcpp::BufferParams minParams(0, 1280, 720,
                waylandcpp::PixelFormat::ARGB8888);

        auto memSize = params.getMemSize();
        if (memSize < minParams.getMemSize())
        {
            memSize = minParams.getMemSize();
        }

        shmPool = createPool(memSize);

        // cannot create pool, return null
        if (!shmPool)
        {
            g_logger.error("%s - cannot create pool", __func__);
            return nullptr;
        }
    }

    g_logger.trace("%s - pool to use: %p", __func__, shmPool.get());

    buffer = createBuffer(shmPool, params, false);
    if (!buffer)
    {
        g_logger.error("%s - cannot create regular buffer", __func__);
        return nullptr;
    }

    m_regularBuffers.push_back(BufferEntry(shmPool, buffer));

    g_logger.info("%s - created regular buffer: %p", __func__, buffer.get());

    return buffer;
}

WaylandBuffer::Ptr WaylandBufferManager::getEmptyBuffer()
{
    for (auto& buffer : m_emptyBuffers)
    {
        if (!buffer->isInUse())
        {
            g_logger.trace("%s - found empty buffer not in use: %p", __func__,
                    buffer.get());
            return buffer;
        }
    }

    auto buffer = createEmptyBuffer();
    if (!buffer)
    {
        return nullptr;
    }

    m_emptyBuffers.push_back(buffer);

    g_logger.trace("%s - created empty buffer: %p", __func__, buffer.get());

    return buffer;
}

waylandcpp::ShmPool1::Ptr WaylandBufferManager::findPool(const waylandcpp::BufferParams& params)
{
    for (auto iter = m_pools.begin(); iter != m_pools.end();)
    {
        if ((*iter)->getSize() < params.getMemSize())
        {
            g_logger.trace("%s - Releasing pool: %p", __func__, iter->get());

            // the pool is too small, discard
            iter = m_pools.erase(iter);

            g_logger.trace("%s - pool released", __func__);
        }
        else
        {
            // found proper pool, take it.
            auto shmPool = *iter;

            g_logger.trace("%s - pool found: %p", __func__, shmPool.get());

            iter = m_pools.erase(iter);

            return shmPool;
        }
    }

    g_logger.trace("%s - no usable pool found", __func__);

    return nullptr;
}

WaylandBuffer::Ptr WaylandBufferManager::findBuffer(const waylandcpp::BufferParams& params)
{
    for (auto iter = m_regularBuffers.begin(); iter != m_regularBuffers.end();)
    {
        if (iter->m_buffer->isInUse())
        {
            g_logger.trace("%s - Buffer in use: %p", __func__,
                    iter->m_buffer.get());

            ++iter;
            continue;
        }

        const auto& iterParams = iter->m_buffer->getParams();
        if ((iterParams.m_width == params.m_width)
                && (iterParams.m_height == params.m_height))
        {
            g_logger.trace("%s - Found matching buffer: %p", __func__,
                    iter->m_buffer.get());

            return iter->m_buffer;
        }
        else
        {
            g_logger.trace("%s - Storing pool for reuse: %p", __func__,
                    iter->m_pool.get());

            // keep the pool (it won't be shared as buffer is not used)
            m_pools.push_back(iter->m_pool);

            g_logger.trace("%s - Releasing buffer: %p", __func__,
                    iter->m_buffer.get());

            // discard the buffer,
            iter = m_regularBuffers.erase(iter);
        }
    }

    return nullptr;
}

WaylandBuffer::Ptr WaylandBufferManager::createEmptyBuffer()
{
    waylandcpp::BufferParams params(0, 1, 1, waylandcpp::PixelFormat::ARGB8888);

    auto shmPool = createPool(params.getMemSize());
    if (!shmPool)
    {
        g_logger.error("%s - cannot create pool", __func__);
        return nullptr;
    }

    auto buffer = createBuffer(shmPool, params, true);
    if (!buffer)
    {
        g_logger.error("%s - cannot create empty buffer", __func__);
        return nullptr;
    }

    g_logger.info("%s - created empty buffer: %p", __func__, buffer.get());

    return buffer;
}

WaylandBuffer::Ptr WaylandBufferManager::createBuffer(const waylandcpp::ShmPool1::Ptr& shmPool,
                                                      const waylandcpp::BufferParams& params,
                                                      bool empty)
{
    auto waylandBuffer = shmPool->createBuffer<waylandcpp::Buffer1>(params);
    if (!waylandBuffer)
    {
        g_logger.error("%s - cannot create buffer using pool", __func__);
        return nullptr;
    }

    /* set the contents to transparent */
    if (empty)
    {
        Pixmap emptyPixmap(
                reinterpret_cast<uint8_t*>(waylandBuffer->getDataPtr()),
                params.m_width, params.m_height, params.m_stride);

        Blitter::clear(emptyPixmap);
    }

    g_logger.info("%s - created wayland buffer: %p (%d x %d)", __func__,
            waylandBuffer.get(), params.m_width, params.m_height);

    return std::make_shared<WaylandBuffer>(waylandBuffer, empty);
}

waylandcpp::ShmPool1::Ptr WaylandBufferManager::createPool(std::size_t size)
{
    g_logger.trace("%s - size=%zu", __func__, size);

    auto bufferFile = waylandcpp::File::createMappedShmFile(size);
    if (!bufferFile)
    {
        g_logger.error("%s - creating a buffer file of size %zu failed",
                __func__, size);
        return nullptr;
    }

    auto shmPool = m_shm->createPool<waylandcpp::ShmPool1>(bufferFile);
    if (!shmPool)
    {
        g_logger.error("%s - cannot create SHM pool", __func__);
        return nullptr;
    }

    g_logger.info("%s - allocated pool %p of size %zu", __func__, shmPool.get(),
            size);

    return shmPool;
}

} // namespace gfx
} // namespace subttxrend
