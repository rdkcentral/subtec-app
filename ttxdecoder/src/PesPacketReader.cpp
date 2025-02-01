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


#include "PesPacketReader.hpp"

namespace ttxdecoder
{

std::uint8_t PesPacketReader::m_fakeByte = 0;

PesPacketReader::PesPacketReader() :
        m_chunkData1(&m_fakeByte),
        m_chunkLen1(0),
        m_chunkData2(&m_fakeByte),
        m_chunkLen2(0)
{
    // noop
}

PesPacketReader::PesPacketReader(const std::uint8_t* chunkData1,
                                 std::size_t chunkLen1,
                                 const std::uint8_t* chunkData2,
                                 std::size_t chunkLen2) :
        m_chunkData1(chunkData1),
        m_chunkLen1(chunkLen1),
        m_chunkData2(chunkData2),
        m_chunkLen2(chunkLen2)
{
    // noop
}

PesPacketReader::PesPacketReader(const PesPacketReader& reader,
                                 std::size_t count) :
        m_chunkData1(reader.m_chunkData1),
        m_chunkLen1(reader.m_chunkLen1),
        m_chunkData2(reader.m_chunkData2),
        m_chunkLen2(reader.m_chunkLen2)
{
    m_chunkLen1 = std::min(m_chunkLen1, count);
    count -= m_chunkLen1;

    m_chunkLen2 = std::min(m_chunkLen2, count);
    count -= m_chunkLen2;

    if (count > 0)
    {
        throw Exception(
                "More bytes requested than available in original reader");
    }
}

PesPacketReader::~PesPacketReader()
{
    // noop
}

std::uint8_t PesPacketReader::peekUint8() const
{
    if (m_chunkLen1 > 0)
    {
        return *(m_chunkData1);
    }
    if (m_chunkLen2 > 0)
    {
        return *(m_chunkData2);
    }

    throw Exception("No more bytes available");
}

std::uint8_t PesPacketReader::readUint8()
{
    if (m_chunkLen1 > 0)
    {
        --m_chunkLen1;
        return *(m_chunkData1++);
    }
    if (m_chunkLen2 > 0)
    {
        --m_chunkLen2;
        return *(m_chunkData2++);
    }

    throw Exception("No more bytes available");
}

void PesPacketReader::skip(std::size_t count)
{
    std::size_t skip1 = std::min(m_chunkLen1, count);

    m_chunkData1 += skip1;
    m_chunkLen1 -= skip1;
    count -= skip1;

    std::size_t skip2 = std::min(m_chunkLen2, count);

    m_chunkData2 += skip2;
    m_chunkLen2 -= skip2;
    count -= skip2;

    if (count > 0)
    {
        throw Exception("Cannot skip requested number of bytes");
    }
}

} // namespace ttxdecoder
