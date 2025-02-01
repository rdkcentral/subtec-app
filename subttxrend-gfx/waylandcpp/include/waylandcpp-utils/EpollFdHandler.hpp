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


#ifndef WAYLANDCPP_EPOLLFDHANDLER_HPP_
#define WAYLANDCPP_EPOLLFDHANDLER_HPP_

#include <cstdint>
#include <sys/epoll.h>

namespace waylandcpp
{

class Epoll;

/**
 * Epoll File Descriptor Handler interface.
 */
class EpollFdHandler
{
    EpollFdHandler(const EpollFdHandler&) = delete;
    EpollFdHandler& operator=(const EpollFdHandler&) = delete;

public:
    /**
     * Constructor.
     */
    EpollFdHandler() = default;

    /**
     * Destructor.
     */
    virtual ~EpollFdHandler() = default;

    /**
     * Returns file descriptor.
     *
     * @return
     *      File descriptor (-1 if not available).
     */
    virtual int getFd() const = 0;

    /**
     * Processes the events.
     *
     * @param epollFd
     *      Epoll that called the method.
     * @param events
     *      Events (mask) to process.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure, terminate epoll processing.
     */
    virtual bool handleEvents(Epoll* epollFd,
                              uint32_t events) = 0;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_EPOLLFDHANDLER_HPP_*/
