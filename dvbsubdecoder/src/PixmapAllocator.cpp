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


#include "PixmapAllocator.hpp"

#include <subttxrend/common/Logger.hpp>

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "PixmapAllocator");

} // namespace <anonmymous>

PixmapAllocator::PixmapAllocator(Specification specVersion,
                                 DecoderClient& client) :
        m_client(client)
{
    switch (specVersion)
    {
    case Specification::VERSION_1_2_1:
        m_blockSize = BUFFER_SIZE_121;
        break;
    case Specification::VERSION_1_3_1:
        m_blockSize = BUFFER_SIZE_131;
        break;
    default:
        throw std::logic_error("Unsupported specVersion");
    }

    g_logger.trace("%s - blocksize=%zu", __func__, m_blockSize);

    while (m_blockSize >= BUFFER_SIZE_MIN)
    {
        m_blockPtr = GfxUniquePtr(m_client.gfxAllocate(m_blockSize),
                GfxMemoryDeleter(&client));
        if (m_blockPtr)
        {
            g_logger.trace("%s - success for blocksize=%zu", __func__,
                    m_blockSize);

            break;
        }
        m_blockSize -= BUFFER_SIZE_STEP;
    }

    if (!m_blockPtr)
    {
        g_logger.trace("%s - all allocations failed", __func__);

        m_blockSize = 0;
    }

    g_logger.trace("%s - blocksize=%zu, blockptr=%p", __func__, m_blockSize,
            m_blockPtr.get());

    m_currentPtr = nullptr;
    m_bytesLeft = 0;
}

PixmapAllocator::~PixmapAllocator()
{
    g_logger.trace("%s", __func__);
}

void PixmapAllocator::reset()
{
    g_logger.trace("%s", __func__);

    m_currentPtr = reinterpret_cast<std::uint8_t*>(m_blockPtr.get());
    m_bytesLeft = m_blockSize;
}

bool PixmapAllocator::canAllocate(std::size_t size) const
{
    bool result = m_bytesLeft >= size;

    g_logger.trace("%s %zu -> %d", __func__, size, result);

    return result;
}

std::uint8_t* PixmapAllocator::allocate(std::size_t size)
{
    if (canAllocate(size))
    {
        auto ptr = m_currentPtr;

        m_currentPtr += size;
        m_bytesLeft -= size;

        g_logger.trace("%s %zu -> %p", __func__, size, ptr);

        return ptr;
    }
    else
    {
        g_logger.trace("%s %zu -> null", __func__, size);

        return nullptr;
    }
}

} // namespace dvbsubdecoder
