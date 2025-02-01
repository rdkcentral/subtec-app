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


#include "SmartFileTarget.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

namespace subttxrend
{
namespace testapps
{

SmartFileTarget::SmartFileTarget(const std::string& path) :
        DataTarget(path),
        m_peerTarget(path),
        m_wantsMorePackets(false),
        m_recordingStarted(false)
{
    // noop
}

SmartFileTarget::~SmartFileTarget()
{
    close();
}

bool SmartFileTarget::open()
{
    if (!m_peerTarget.open())
    {
        return false;
    }

    m_wantsMorePackets = true;
    m_recordingStarted = false;

    return true;
}

void SmartFileTarget::close()
{
    m_peerTarget.close();
}

bool SmartFileTarget::wantsMorePackets()
{
    return m_wantsMorePackets;
}

bool SmartFileTarget::writePacket(const DataPacket& packet)
{
    bool isResetAllPacket = false;

    if (packet.getSize() == 12)
    {
        const char* data = packet.getBuffer();
        if (data[0] == 3)
        {
            std::size_t i = 0;

            for (i = 1; i < 12; ++i)
            {
                if (data[i] != 0)
                {
                    break;
                }
            }

            if (i == 12)
            {
                isResetAllPacket = true;
            }
        }
    }

    if (!m_recordingStarted)
    {
        if (isResetAllPacket)
        {
            std::cout << "RESET ALL found - Recording started" << std::endl;
            m_recordingStarted = true;
        }
        else
        {
            std::cout << "Recording not started, skipping packet" << std::endl;
            return true;
        }
    }
    else if (m_wantsMorePackets)
    {
        if (isResetAllPacket)
        {
            std::cout << "RESET ALL found - Recording stopped" << std::endl;
            m_wantsMorePackets = false;
            return true;
        }
    }
    else
    {
        return false;
    }

    return m_peerTarget.writePacket(packet);
}

} // namespace testapps
} // namespace subttxrend
