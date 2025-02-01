/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <unistd.h>

#include "WebvttFileNoHeaderSource.hpp"

namespace subttxrend
{
namespace testapps
{

WebvttFileNoHeaderSource::WebvttFileNoHeaderSource(const std::string& path) :
        PlainFileSource(path)
{
    // noop
}

WebvttFileNoHeaderSource::~WebvttFileNoHeaderSource()
{
    // noop
}

bool WebvttFileNoHeaderSource::readPacket(DataPacket& packet)
{
    static const std::size_t TTML_DATA_HEADER_SIZE = 24;

    //static std::uint32_t counter = 0;

    auto fileHandle = getFileHandle();
    if (fileHandle == -1)
    {
        return false;
    }

    struct stat st;
    stat(getPath().c_str(), &st);

    std::uint32_t packetDataSize = static_cast<std::uint32_t>(st.st_size);

    std::cerr << "Read file size: " << packetDataSize << std::endl;

    if (packet.getCapacity() <  (TTML_DATA_HEADER_SIZE + packetDataSize))
    {
        std::cerr << "Not enough space for packet: " << packetDataSize << std::endl;
        return false;
    }

    const std::uint32_t type = PACKET_TYPE_WVTT_DATA;
    const std::uint32_t size = packetDataSize + 4 /* for channelId */ + 8 /* for dataOffset */;
    const std::uint32_t channelId = 0;

    packet.reset();
    packet.appendLeUint32(type);
    packet.appendLeUint32(4);
    packet.appendLeUint32(size);
    packet.appendLeUint32(channelId);

    int dataBytesRead = ::read(fileHandle, packet.getBuffer() + TTML_DATA_HEADER_SIZE,
            packetDataSize);
    if (dataBytesRead == 0)
    {
        // eof
        packet.setSize(0);
        return true;
    }

    if (dataBytesRead < static_cast<int>(packetDataSize))
    {
        std::cerr << "Header read failed" << std::endl;
        return false;
    }

    packet.setSize(TTML_DATA_HEADER_SIZE + packetDataSize);

    return true;

}

} // namespace subttxrend
} // namespace testapps

