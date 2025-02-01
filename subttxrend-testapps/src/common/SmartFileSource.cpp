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


#include "SmartFileSource.hpp"

#include <sys/time.h>
#include <iostream>

namespace subttxrend
{
namespace testapps
{

SmartFileSource::SmartFileSource(const std::string& path) :
        DataSource(path),
        m_peerSource(path),
        m_hasPreviousTimestamp(false),
        m_previousClockTimestampMs(0),
        m_previousPacketTimstampsMs(0)
{
    // noop
}

SmartFileSource::~SmartFileSource()
{
    close();
}

bool SmartFileSource::open()
{
    if (m_peerSource.open())
    {
        m_hasPreviousTimestamp = false;

        return true;
    }
    else
    {
        return false;
    }
}

void SmartFileSource::close()
{
    m_peerSource.close();
}

bool SmartFileSource::readPacket(DataPacket& packet)
{
    if (m_peerSource.readPacket(packet))
    {
        static const std::size_t TIMESTAMP_PACKET_SIZE = 24;
        static const std::uint8_t TIMESTAMP_PACKET_TYPE = 2;

        if (packet.getSize() == TIMESTAMP_PACKET_SIZE)
        {
            const uint8_t* data = reinterpret_cast<uint8_t*>(packet.getBuffer());

            if ((data[0] == TIMESTAMP_PACKET_TYPE) && (data[1] == 0)
                    && (data[2] == 0) && (data[3] == 0))
            {
                std::uint64_t packetTimestampMs1 = data[12];
                std::uint64_t packetTimestampMs2 = data[13];
                std::uint64_t packetTimestampMs3 = data[14];
                std::uint64_t packetTimestampMs4 = data[15];
                std::uint64_t packetTimestampMs5 = data[16];
                std::uint64_t packetTimestampMs6 = data[17];
                std::uint64_t packetTimestampMs7 = data[18];
                std::uint64_t packetTimestampMs8 = data[19];

                std::uint64_t packetTimestampMs = 0;

                packetTimestampMs |= packetTimestampMs8;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs7;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs6;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs5;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs4;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs3;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs2;
                packetTimestampMs <<= 8;
                packetTimestampMs |= packetTimestampMs1;

                struct timeval tv;
                gettimeofday(&tv, NULL);

                const std::uint64_t clockSec = tv.tv_sec;
                const std::uint64_t clockUsec = tv.tv_usec;

                const std::uint64_t clockTimestampMs = (clockSec * 1000)
                        + (clockUsec / 1000);

                if (m_hasPreviousTimestamp)
                {
                    std::uint64_t clockDiffMs = clockTimestampMs
                            - m_previousClockTimestampMs;
                    std::uint64_t packetDiffMs = packetTimestampMs
                            - m_previousPacketTimstampsMs;

                    if (clockDiffMs < packetDiffMs)
                    {
                        std::uint64_t sleepTimeMs = packetDiffMs - clockDiffMs;

                        struct timespec req;

                        req.tv_sec = sleepTimeMs / 1000;
                        req.tv_nsec = (sleepTimeMs % 1000) * 1000000;

                        std::cout << "Sleeping for " << sleepTimeMs << std::endl;

                        ::nanosleep(&req, NULL);
                    }
                }

                m_previousPacketTimstampsMs = packetTimestampMs;
                m_previousClockTimestampMs = clockTimestampMs;
                m_hasPreviousTimestamp = true;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace testapps
} // namespace subttxrend
