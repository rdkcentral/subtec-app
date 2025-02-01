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

#include "UnixSocket.hpp"

#include <cerrno>
#include <cstring>

#include <grp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <sys/ioctl.h>

namespace subttxrend {
namespace socksrc {

UnixSocket::UnixSocket(std::string const& path)
{
    m_socketHandlePtr = std::make_unique<Handle>(::socket(AF_UNIX, SOCK_DGRAM, 0));

    struct sockaddr_un addr{};
    if (path.size() >= sizeof(addr.sun_path)) {
        throw SocketException("Given socket address is too long");
    }

    addr.sun_family = AF_UNIX;
    (void) std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = 0;

    (void) ::unlink(path.c_str());

    if (::bind(*m_socketHandlePtr, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
        throw SocketException("Cannot bind to address");
    }

#ifndef PC_BUILD
#ifdef SUBTTXACCESS_GROUP
    using namespace std::string_literals;
    struct group* grp = getgrnam(SUBTTXACCESS_GROUP);
    if (grp == NULL) {
        auto errorMsg = strerror(errno);
        throw SocketException("getgrnam() failed with error: "s + errorMsg);
    }

    if (chown(path.c_str(), getuid(), grp->gr_gid) == -1) {
        auto errorMsg = strerror(errno);
        throw SocketException("chown() failed with error: "s + errorMsg);
    }

    if (chmod(path.c_str(), S_IRWXU|S_IRWXG) < 0) {
        auto errorMsg = strerror(errno);
        throw SocketException("chmod() failed with error: "s + errorMsg);
    }
#endif
#endif
}

std::size_t UnixSocket::getSocketBufferSize() const
{
    auto constexpr MAX_BUFFER_SIZE = 128 * 1024;
    int bufferSize{0};
    unsigned int bufferSizeSize = sizeof(bufferSize);
    if (::getsockopt(*m_socketHandlePtr, SOL_SOCKET, SO_RCVBUF, (void *)&bufferSize, &bufferSizeSize) == -1) {
        bufferSize = MAX_BUFFER_SIZE;
    }
    return static_cast<std::size_t >(bufferSize);
}

std::size_t UnixSocket::peekBytes(common::DataBuffer& buffer)
{
    return read(buffer, true);
}

std::size_t UnixSocket::readBytes(common::DataBuffer& buffer)
{
    return read(buffer, false);
}

void UnixSocket::shutdown()
{
    ::shutdown(*m_socketHandlePtr, SHUT_RDWR);
}

std::size_t UnixSocket::read(common::DataBuffer& buffer, bool peekOnly)
{
    std::size_t result{};
    int flags = (peekOnly ? MSG_PEEK : 0);
    auto bytesRead = ::recv(*m_socketHandlePtr, buffer.data(), buffer.capacity(), flags);
    if (bytesRead >= 0) {
        buffer.resize(bytesRead);
        result = static_cast<std::size_t>(bytesRead);
    }
    else {
        auto errorMsg = strerror(errno);
        throw SocketException(std::string("Error reading socket: ") + errorMsg);
    }
    return result;
}

} // namespace subttxrend
} // namespace socksrc

