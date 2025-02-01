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


#include "Ipv4SocketTarget.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdio>

#include <arpa/inet.h>
namespace subttxrend
{
namespace testapps
{

Ipv4SocketTarget::Ipv4SocketTarget(const std::string& path) :
        DataTarget(path),
        m_socketHandle(-1)
{
    // noop
}

Ipv4SocketTarget::~Ipv4SocketTarget()
{
    close();
}

bool Ipv4SocketTarget::open()
{
    if (m_socketHandle != -1)
    {
        return true;
    }

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

    (void) std::memset(&m_targetAddress, 0, sizeof(m_targetAddress));
    m_targetAddress.sin_family = AF_INET;
    m_targetAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddressStr.c_str(), &m_targetAddress.sin_addr)
            != 1)
    {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }

    m_socketHandle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketHandle == -1)
    {
        std::cerr << "Cannot create socket" << std::endl;
        return false;
    }

    if (::connect(m_socketHandle,
            reinterpret_cast<struct sockaddr*>(&m_targetAddress),
            sizeof(m_targetAddress)) != 0)
    {
        std::cerr << "Cannot connect socket" << std::endl;
        close();
        return false;
    }

    return true;
}

void Ipv4SocketTarget::close()
{
    if (m_socketHandle != -1)
    {
        (void) ::close(m_socketHandle);
        m_socketHandle = -1;
    }
}

bool Ipv4SocketTarget::wantsMorePackets()
{
    // packets always wanted
    return true;
}

bool Ipv4SocketTarget::writePacket(const DataPacket& packet)
{
    auto size = packet.getSize();
    auto written = ::send(m_socketHandle, packet.getBuffer(), size, 0);

    std::cout << "Write operation - requested: " << size << " written: "
            << written << std::endl;

    return written == static_cast<int>(size);
}

} // namespace testapps
} // namespace subttxrend
