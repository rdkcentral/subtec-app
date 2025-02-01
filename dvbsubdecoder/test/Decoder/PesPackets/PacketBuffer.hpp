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


#ifndef DVBSUBDECODER_TEST_PESPACKETS_PACKETBUFFER_HPP
#define DVBSUBDECODER_TEST_PESPACKETS_PACKETBUFFER_HPP

#include <cstdint>
#include <vector>

constexpr std::uint8_t HIGH_BYTE(std::uint16_t word)
{
    return (word >> 8);
}

constexpr std::uint8_t LOW_BYTE(std::uint16_t word)
{
    return (word & 0xFF);
}

class PacketBuffer
{
public:

    PacketBuffer() = default;

    virtual ~PacketBuffer();

    void append(std::uint16_t word)
    {
        m_data.push_back(HIGH_BYTE(word));
        m_data.push_back(LOW_BYTE(word));
    }

    void append(std::uint8_t byte)
    {
        m_data.push_back(byte);
    }

    void append(const std::uint8_t* buffer,
                std::size_t size)
    {
        m_data.insert(m_data.end(), buffer, buffer + size);
    }

    void append(const PacketBuffer& other)
    {
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
    }

    virtual std::size_t getSize() const
    {
        return m_data.size();
    }

    virtual const std::uint8_t* getData() const
    {
        return m_data.data();
    }

protected:

    std::vector<std::uint8_t> m_data;
};

inline PacketBuffer::~PacketBuffer()
{
    // noop
}

#endif /* DVBSUBDECODER_TEST_PESPACKETS_PACKETBUFFER_HPP */
