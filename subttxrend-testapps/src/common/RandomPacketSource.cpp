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


#include "RandomPacketSource.hpp"

#include <ctime>
#include <cstdlib>

#include <unistd.h>

namespace subttxrend
{
namespace testapps
{

RandomPacketSource::RandomPacketSource(const std::string& path) :
        DataSource(path),
        m_packetCount(0),
        m_sleepTimeMs(0),
        m_packetNo(0),
        m_nextPacketCounter(0)
{
    std::srand(std::time(nullptr));
}

bool RandomPacketSource::open()
{
    std::string params = getPath();

    unsigned int packetCount = 0;
    unsigned int sleepTimeMs = 0;
    bool paramsValid = false;

    if (params.length() > 0)
    {
        char separator = 0;
        char endmarker = 0;

        if (sscanf(params.c_str(), "%u%c%u%c", &packetCount, &separator,
                &sleepTimeMs, &endmarker) == 3)
        {
            if ((separator == ':') && (separator == ':'))
            {
                paramsValid = true;
            }
        }
    }
    else
    {
        paramsValid = true;
    }

    if (paramsValid)
    {
        m_packetCount = packetCount;
        m_sleepTimeMs = sleepTimeMs;
        return true;
    }
    else
    {
        return false;
    }
}

void RandomPacketSource::close()
{
    // noop
}

bool RandomPacketSource::readPacket(DataPacket& packet)
{
    if (m_packetCount > 0)
    {
        if (m_packetNo < m_packetCount)
        {
            ++m_packetNo;
        }
        else
        {
            packet.setSize(0);
            return true;
        }
    }

    bool rv = generatePacket(packet);

    if (rv && (m_sleepTimeMs > 0))
    {
        struct timespec req;

        req.tv_sec = m_sleepTimeMs / 1000;
        req.tv_nsec = (m_sleepTimeMs % 1000) * 1000000;

        ::nanosleep(&req, NULL);
    }

    return rv;
}

bool RandomPacketSource::generatePacket(DataPacket& packet)
{
    const std::uint32_t type = (rand() % 6) + 1; // 1-6

    switch (type)
    {
    case PACKET_TYPE_PES_DATA:
        return generatePesData(packet);

    case PACKET_TYPE_TIMESTAMP:
        return generateTimestamp(packet);

    case PACKET_TYPE_RESET_ALL:
        return generateResetAll(packet);

    case PACKET_TYPE_RESET_CHANNEL:
        return generateResetChannel(packet);

    case PACKET_TYPE_SUBTITLE_SELECTION:
        return generateSubtitleSelection(packet);

    case PACKET_TYPE_TELETEXT_SELECTION:
        return generateTeletextSelection(packet);

    default:
        /* shall never happen, produce EOF */
        packet.setSize(0);
        return true;
    }
}

bool RandomPacketSource::generatePesData(DataPacket& packet)
{
    const std::uint32_t dataLen = rand() % (packet.getCapacity() - 12 - 8 + 1);

    const std::uint32_t type = PACKET_TYPE_PES_DATA;
    const std::uint32_t counter = m_nextPacketCounter++;
    const std::uint32_t size = 8 + dataLen;
    const std::uint32_t channelId = rand() % 10;
    const std::uint32_t channelType = (channelId == 1) ? 1 : 0;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);
    packet.appendLeUint32(channelId);
    packet.appendLeUint32(channelType);
    packet.appendZeroes(dataLen);

    return true;
}

bool RandomPacketSource::generateTimestamp(DataPacket& packet)
{
    const std::uint32_t type = PACKET_TYPE_TIMESTAMP;
    const std::uint32_t counter = m_nextPacketCounter++;
    const std::uint32_t size = 12;
    const std::uint32_t stc = m_nextPacketCounter;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);
    packet.appendZeroes(8); // timestamp
    packet.appendLeUint32(stc);

    return true;
}

bool RandomPacketSource::generateResetAll(DataPacket& packet)
{
    m_nextPacketCounter = 0;

    const std::uint32_t type = PACKET_TYPE_RESET_ALL;
    const std::uint32_t counter = 0;
    const std::uint32_t size = 0;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);

    return true;
}

bool RandomPacketSource::generateResetChannel(DataPacket& packet)
{
    const std::uint32_t type = PACKET_TYPE_RESET_CHANNEL;
    const std::uint32_t counter = m_nextPacketCounter++;
    const std::uint32_t size = 4;
    const std::uint32_t channelId = rand() % 10;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);
    packet.appendLeUint32(channelId);

    return true;
}

bool RandomPacketSource::generateSubtitleSelection(DataPacket& packet)
{
    const std::uint32_t type = PACKET_TYPE_SUBTITLE_SELECTION;
    const std::uint32_t counter = m_nextPacketCounter++;
    const std::uint32_t size = 16;
    const std::uint32_t channelId = rand() % 10;
    const std::uint32_t subtitlesType = rand() % 1;
    const std::uint32_t auxId1 = rand();
    const std::uint32_t auxId2 = rand();

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);
    packet.appendLeUint32(channelId);
    packet.appendLeUint32(subtitlesType);
    packet.appendLeUint32(auxId1);
    packet.appendLeUint32(auxId2);

    return true;
}

bool RandomPacketSource::generateTeletextSelection(DataPacket& packet)
{
    const std::uint32_t type = PACKET_TYPE_TELETEXT_SELECTION;
    const std::uint32_t counter = m_nextPacketCounter++;
    const std::uint32_t size = 4;
    const std::uint32_t channelId = rand() % 10;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(counter);
    packet.appendLeUint32(size);
    packet.appendLeUint32(channelId);

    return true;
}

} // namespace testapps
} // namespace subttxrend
