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


#include "UnixSocketTarget.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace subttxrend
{
namespace testapps
{

UnixSocketTarget::UnixSocketTarget(const std::string& path) :
        DataTarget(path),
        m_socketHandle(-1)
{
    // noop
}

UnixSocketTarget::~UnixSocketTarget()
{
    close();
}

bool UnixSocketTarget::open()
{
    if (m_socketHandle != -1)
    {
        return true;
    }

    m_socketHandle = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (m_socketHandle == -1)
    {
        std::cerr << "Cannot create socket" << std::endl;
        return false;
    }

    struct sockaddr_un addr;

    if (getPath().size() >= sizeof(addr.sun_path))
    {
        close();
        std::cerr << "Given socket address is too long" << std::endl;
        return false;
    }

    (void) std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    (void) std::strncpy(addr.sun_path, getPath().c_str(),
            sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = 0;

    if (::connect(m_socketHandle, reinterpret_cast<struct sockaddr*>(&addr),
            sizeof(addr)) != 0)
    {
        close();
        std::cerr << "Cannot connect to address" << std::endl;
        return false;
    }

    return true;
}

void UnixSocketTarget::close()
{
    if (m_socketHandle != -1)
    {
        (void) ::close(m_socketHandle);
        m_socketHandle = -1;
    }
}

bool UnixSocketTarget::wantsMorePackets()
{
    // packets always wanted
    return true;
}

bool UnixSocketTarget::writePacket(const DataPacket& packet)
{
    auto size = packet.getSize();
    auto written = ::write(m_socketHandle, packet.getBuffer(), size);

    std::cout << "Write operation - requested: " << size << " written: "
            << written << std::endl;

    return written == static_cast<int>(size);
}

} // namespace testapps
} // namespace subttxrend
