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


#include "Ipv4SocketSource.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

namespace subttxrend
{
namespace testapps
{

Ipv4SocketSource::Ipv4SocketSource(const std::string& path) :
        DataSource(path),
        m_serverSocketHandle(-1),
        m_clientSocketHandle(-1)
{
    // noop
}

Ipv4SocketSource::~Ipv4SocketSource()
{
    close();
}

bool Ipv4SocketSource::open()
{
    if (m_serverSocketHandle != -1)
    {
        return true;
    }

    struct sockaddr_in bindAddress;

    const std::string& path = getPath();
    std::string::size_type portSep = path.find(':');
    if (portSep == std::string::npos)
    {
        std::cerr << "Port not specified in path" << std::endl;
        return false;
    }

    std::string ipAddressStr = path.substr(0, portSep);
    std::string portStr = path.substr(portSep + 1);

    unsigned short port = 0;

    if (std::sscanf(portStr.c_str(), "%hu", &port) != 1)
    {
        std::cerr << "Invalid port specified" << std::endl;
        return false;
    }

    (void) std::memset(&bindAddress, 0, sizeof(bindAddress));
    bindAddress.sin_family = AF_INET;
    bindAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddressStr.c_str(), &bindAddress.sin_addr) != 1)
    {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }

    m_serverSocketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverSocketHandle == -1)
    {
        std::cerr << "Cannot create socket" << std::endl;
        return false;
    }

    if (::bind(m_serverSocketHandle,
            reinterpret_cast<struct sockaddr*>(&bindAddress),
            sizeof(bindAddress)) != 0)
    {
        close();
        std::cerr << "Cannot bind to address" << std::endl;
        return false;
    }

    if (::listen(m_serverSocketHandle, 1) != 0)
    {
        close();
        std::cerr << "Cannot listen on socket" << std::endl;
        return false;
    }

    return true;
}

void Ipv4SocketSource::close()
{
    if (m_clientSocketHandle != -1)
    {
        (void) ::close(m_clientSocketHandle);
        m_clientSocketHandle = -1;
    }

    if (m_serverSocketHandle != -1)
    {
        (void) ::close(m_serverSocketHandle);
        m_serverSocketHandle = -1;
    }
}

bool Ipv4SocketSource::readPacket(DataPacket& packet)
{
    if (m_clientSocketHandle == -1)
    {
        m_clientSocketHandle = ::accept(m_serverSocketHandle, NULL, NULL);
        if (m_clientSocketHandle == -1)
        {
            std::cerr << "Cannot accept client socket" << std::endl;
            return false;
        }
    }

    std::uint8_t* buffer = reinterpret_cast<std::uint8_t*>(packet.getBuffer());
    auto bufferSize = packet.getCapacity();
    std::size_t bytesRead = 0;

    /* read header fist */
    const int HEADER_SIZE = 12;
    if (bufferSize < HEADER_SIZE)
    {
        std::cerr << "Not enough space for header" << std::endl;
        return false;
    }
    while (bytesRead < HEADER_SIZE)
    {
        auto newBytesRead = ::recv(m_clientSocketHandle, &buffer[bytesRead],
                12 - bytesRead, 0);
        if (newBytesRead < 0)
        {
            std::cerr << "Cannot read header data" << std::endl;
            return false;
        }
        else if (newBytesRead == 0)
        {
            if (bytesRead == 0)
            {
                /* eof */
                packet.setSize(0);
                return true;
            }
            else
            {
                std::cerr << "Cannot read header data" << std::endl;
                return false;
            }
        }
        bytesRead += newBytesRead;
    }

    /* get size */
    std::uint32_t dataSize1 = buffer[8];
    std::uint32_t dataSize2 = buffer[9];
    std::uint32_t dataSize3 = buffer[10];
    std::uint32_t dataSize4 = buffer[11];

    std::uint32_t dataSize = 0;
    dataSize |= dataSize4;
    dataSize <<= 8;
    dataSize |= dataSize3;
    dataSize <<= 8;
    dataSize |= dataSize2;
    dataSize <<= 8;
    dataSize |= dataSize1;

    dataSize += HEADER_SIZE;

    if (bufferSize < dataSize)
    {
        std::cerr << "Not enough space for packet" << std::endl;
        return false;
    }

    while (bytesRead < dataSize)
    {
        auto newBytesRead = ::recv(m_clientSocketHandle, &buffer[bytesRead],
                dataSize - bytesRead, 0);
        if (newBytesRead <= 0)
        {
            std::cerr << "Cannot read packet data" << std::endl;
            return false;
        }
        bytesRead += newBytesRead;
    }

    packet.setSize(dataSize);

    return true;
}

} // namespace testapps
} // namespace subttxrend
