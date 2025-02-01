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


#include "PlainFileSource.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

namespace subttxrend
{
namespace testapps
{

PlainFileSource::PlainFileSource(const std::string& path) :
        DataSource(path),
        m_fileHandle(-1)
{
    // noop
}

PlainFileSource::~PlainFileSource()
{
    close();
}

bool PlainFileSource::open()
{
    if (m_fileHandle != -1)
    {
        std::cerr << "Already open" << std::endl;
        return true;
    }

    m_fileHandle = ::open(getPath().c_str(), O_RDONLY);
    if (m_fileHandle == -1)
    {
        std::cerr << "Cannot open file" << std::endl;
        return false;
    }

    return true;
}

void PlainFileSource::close()
{
    if (m_fileHandle != -1)
    {
        (void) ::close(m_fileHandle);
        m_fileHandle = -1;
    }
}

bool PlainFileSource::readPacket(DataPacket& packet)
{
    static const std::size_t HEADER_SIZE = 12;
    std::uint8_t header[HEADER_SIZE];

    if (m_fileHandle == -1)
    {
        return false;
    }

    if (packet.getCapacity() < HEADER_SIZE)
    {
        std::cerr << "Not enough space for header" << std::endl;
        return false;
    }

    // read packet header
    int headerBytesRead = ::read(m_fileHandle, header, HEADER_SIZE);
    if (headerBytesRead == 0)
    {
        // eof
        packet.setSize(0);
        return true;
    }
    if (headerBytesRead < static_cast<int>(HEADER_SIZE))
    {
        std::cerr << "Header read failed" << std::endl;
        return false;
    }

    std::uint32_t packetDataSize1 = header[8];
    std::uint32_t packetDataSize2 = header[9];
    std::uint32_t packetDataSize3 = header[10];
    std::uint32_t packetDataSize4 = header[11];

    std::uint32_t packetDataSize = 0;
    packetDataSize |= packetDataSize4;
    packetDataSize <<= 8;
    packetDataSize |= packetDataSize3;
    packetDataSize <<= 8;
    packetDataSize |= packetDataSize2;
    packetDataSize <<= 8;
    packetDataSize |= packetDataSize1;

    if ((packet.getCapacity() - HEADER_SIZE) < packetDataSize)
    {
        std::cerr << "Not enough space for data: " << packetDataSize << std::endl;
        return false;
    }

    (void) std::memcpy(packet.getBuffer(), header, HEADER_SIZE);

    int dataBytesRead = ::read(m_fileHandle, packet.getBuffer() + HEADER_SIZE,
            packetDataSize);
    if (dataBytesRead < static_cast<int>(packetDataSize))
    {
        std::cerr << "Header read failed" << std::endl;
        return false;
    }

    packet.setSize(HEADER_SIZE + packetDataSize);

    return true;
}

} // namespace testapps
} // namespace subttxrend
