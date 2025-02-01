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

#pragma once

#include <subttxrend/common/DataBuffer.hpp>

#include <cassert>
#include <exception>
#include <memory>
#include <string>

#include <unistd.h>

namespace subttxrend {
namespace socksrc {

class UnixSocket
{
public:

    class SocketException : public std::exception
    {
    public:
        SocketException(std::string const& _msg) : msg(_msg) {
        }

        char const* what() const noexcept override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    UnixSocket(std::string const& path);

    /**
     * Read socket buffer size.
     *
     * @return
     *      Socket buffer size.
     */
    std::size_t getSocketBufferSize() const;

    /**
     * Blocking call waiting for socket data.
     *
     * @return
     *      Number of bytes read into the buffer.
     */
    std::size_t peekBytes(common::DataBuffer& buffer);

    /**
     * Read bytes from socket.
     *
     * @return
     *      Number of bytes read into the buffer.
     */
    std::size_t readBytes(common::DataBuffer& buffer);

    /**
     * Signals socket to shutdown - it will cause pending listener to return.
     */
    void shutdown();

private:

    std::size_t read(common::DataBuffer& buffer, bool peekOnly);

    struct Handle
    {
        Handle(int _h) : handle(_h)
        {
            if (handle == INVALID_HANDLE) {
                throw SocketException("Cannot create socket");
            }
        };
        ~Handle()
        {
            assert(handle != INVALID_HANDLE);
            ::close(handle);
        }
        operator int() const { return handle; }
        static constexpr int INVALID_HANDLE = -1;
        int handle;
    };
    using SocketHandlePtr = std::unique_ptr<Handle>;

    SocketHandlePtr m_socketHandlePtr;
};

using UnixSocketPtr = std::unique_ptr<UnixSocket>;

} // namespace subttxrend
} // namespace socksrc
