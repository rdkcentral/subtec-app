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

#include "Buffer.hpp"

namespace subttxrend
{
namespace protocol
{

Buffer::Buffer(const char* data,
               std::size_t size) :
        m_data(data),
        m_size(size)
{
    // noop
}

std::size_t Buffer::getSize() const
{
    return m_size;
}

bool Buffer::extractLeUint32(std::size_t offset,
                             std::uint32_t& value) const
{
    if (offset >= m_size)
    {
        return false;
    }

    if (m_size - offset < 4)
    {
        return false;
    }

    const std::uint32_t byte0 = static_cast<std::uint32_t>(m_data[offset + 0]) & 0xFF;
    const std::uint32_t byte1 = static_cast<std::uint32_t>(m_data[offset + 1]) & 0xFF;
    const std::uint32_t byte2 = static_cast<std::uint32_t>(m_data[offset + 2]) & 0xFF;
    const std::uint32_t byte3 = static_cast<std::uint32_t>(m_data[offset + 3]) & 0xFF;

    value = byte3;
    value <<= 8;
    value |= byte2;
    value <<= 8;
    value |= byte1;
    value <<= 8;
    value |= byte0;

    return true;
}

bool Buffer::extractLeUint64(std::size_t offset,
                             std::uint64_t& value) const
{
    if (offset >= m_size)
    {
        return false;
    }

    if (m_size - offset < 8)
    {
        return false;
    }

    const std::uint64_t byte0 = static_cast<std::uint64_t>(m_data[offset + 0]) & 0xFF;
    const std::uint64_t byte1 = static_cast<std::uint64_t>(m_data[offset + 1]) & 0xFF;
    const std::uint64_t byte2 = static_cast<std::uint64_t>(m_data[offset + 2]) & 0xFF;
    const std::uint64_t byte3 = static_cast<std::uint64_t>(m_data[offset + 3]) & 0xFF;
    const std::uint64_t byte4 = static_cast<std::uint64_t>(m_data[offset + 4]) & 0xFF;
    const std::uint64_t byte5 = static_cast<std::uint64_t>(m_data[offset + 5]) & 0xFF;
    const std::uint64_t byte6 = static_cast<std::uint64_t>(m_data[offset + 6]) & 0xFF;
    const std::uint64_t byte7 = static_cast<std::uint64_t>(m_data[offset + 7]) & 0xFF;

    value = byte7;
    value <<= 8;
    value |= byte6;
    value <<= 8;
    value |= byte5;
    value <<= 8;
    value |= byte4;
    value <<= 8;
    value |= byte3;
    value <<= 8;
    value |= byte2;
    value <<= 8;
    value |= byte1;
    value <<= 8;
    value |= byte0;

    return true;
}

bool Buffer::extractLeInt64(std::size_t offset,
                             std::int64_t& value) const
{
    if (offset >= m_size)
    {
        return false;
    }

    if (m_size - offset < 8)
    {
        return false;
    }

    const std::int64_t byte0 = static_cast<std::int64_t>(m_data[offset + 0]) & 0xFF;
    const std::int64_t byte1 = static_cast<std::int64_t>(m_data[offset + 1]) & 0xFF;
    const std::int64_t byte2 = static_cast<std::int64_t>(m_data[offset + 2]) & 0xFF;
    const std::int64_t byte3 = static_cast<std::int64_t>(m_data[offset + 3]) & 0xFF;
    const std::int64_t byte4 = static_cast<std::int64_t>(m_data[offset + 4]) & 0xFF;
    const std::int64_t byte5 = static_cast<std::int64_t>(m_data[offset + 5]) & 0xFF;
    const std::int64_t byte6 = static_cast<std::int64_t>(m_data[offset + 6]) & 0xFF;
    const std::int64_t byte7 = static_cast<std::int64_t>(m_data[offset + 7]) & 0xFF;

    value = byte7;
    value <<= 8;
    value |= byte6;
    value <<= 8;
    value |= byte5;
    value <<= 8;
    value |= byte4;
    value <<= 8;
    value |= byte3;
    value <<= 8;
    value |= byte2;
    value <<= 8;
    value |= byte1;
    value <<= 8;
    value |= byte0;

    return true;
}

bool Buffer::extractBuffer(std::size_t offset,
                           std::size_t wantedSize,
                           std::vector<char>& buffer) const
{
    if (offset >= m_size)
    {
        return false;
    }

    if (m_size - offset < wantedSize)
    {
        return false;
    }

    buffer.assign(&m_data[offset], &m_data[offset] + wantedSize);

    return true;
}

} // namespace protocol
} // namespace subttxrend
