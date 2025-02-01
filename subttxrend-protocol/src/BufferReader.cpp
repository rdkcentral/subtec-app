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


#include "BufferReader.hpp"

namespace subttxrend
{
namespace protocol
{

BufferReader::BufferReader(const Buffer& buffer) :
        m_buffer(buffer),
        m_offset(0)
{
    // noop
}

const Buffer& BufferReader::getBuffer() const
{
    return m_buffer;
}

std::size_t BufferReader::getSize() const
{
    return m_buffer.getSize();
}

std::size_t BufferReader::getOffset() const
{
    return m_offset;
}

bool BufferReader::extractLeUint32(std::uint32_t& value)
{
    if (m_buffer.extractLeUint32(m_offset, value))
    {
        m_offset += 4;
        return true;
    }
    else
    {
        return false;
    }
}

bool BufferReader::extractLeUint64(std::uint64_t& value)
{
    if (m_buffer.extractLeUint64(m_offset, value))
    {
        m_offset += 8;
        return true;
    }
    else
    {
        return false;
    }
}

bool BufferReader::extractLeInt64(std::int64_t& value)
{
    if (m_buffer.extractLeInt64(m_offset, value))
    {
        m_offset += 8;
        return true;
    }
    else
    {
        return false;
    }
}

bool BufferReader::extractBuffer(std::size_t wantedSize,
                                 std::vector<char>& buffer)
{
    if (m_buffer.extractBuffer(m_offset, wantedSize, buffer))
    {
        m_offset += wantedSize;
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace protocol
} // namespace subttxrend
