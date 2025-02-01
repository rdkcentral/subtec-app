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


#include "DataPacket.hpp"

#include <stdexcept>

namespace subttxrend
{
namespace testapps
{

DataPacket::DataPacket(const std::size_t capacity) :
        m_capacity(capacity),
        m_size(0)
{
    m_buffer.reset(new char[capacity]);
}

char* DataPacket::getBuffer()
{
    return m_buffer.get();
}

const char* DataPacket::getBuffer() const
{
    return m_buffer.get();
}

size_t DataPacket::getCapacity() const
{
    return m_capacity;
}

size_t DataPacket::getSize() const
{
    return m_size;
}

void DataPacket::setSize(std::size_t newSize)
{
    if (newSize > m_capacity)
    {
        throw std::length_error("Given size larger than capacity");
    }

    m_size = newSize;
}

void DataPacket::reset()
{
    setSize(0);
}

void DataPacket::appendLeUint32(std::uint32_t value)
{
    if (getCapacity() - getSize() < 4)
    {
        throw std::length_error("Not enough space");
    }

    m_buffer.get()[m_size++] = static_cast<char>((value >> 0) & 0xFF);
    m_buffer.get()[m_size++] = static_cast<char>((value >> 8) & 0xFF);
    m_buffer.get()[m_size++] = static_cast<char>((value >> 16) & 0xFF);
    m_buffer.get()[m_size++] = static_cast<char>((value >> 24) & 0xFF);
}

void DataPacket::appendLeUint64(std::uint64_t value)
{
    if (getCapacity() - getSize() < 8)
    {
        throw std::length_error("Not enough space");
    }

    appendLeUint32((static_cast<std::uint32_t>((value >> 0)) & 0xFFFFFFFF));
    appendLeUint32((static_cast<std::uint32_t>((value >> 32)) & 0xFFFFFFFF));
}

void DataPacket::appendZeroes(std::size_t count)
{
    if (getCapacity() - getSize() < count)
    {
        throw std::length_error("Not enough space");
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        m_buffer.get()[m_size++] = 0;
    }
}

} // namespace testapps
} // namespace subttxrend
