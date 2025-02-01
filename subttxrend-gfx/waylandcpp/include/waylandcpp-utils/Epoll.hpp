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


#ifndef WAYLANDCPP_EPOLL_HPP_
#define WAYLANDCPP_EPOLL_HPP_

#include "EpollFdHandler.hpp"

#include <set>

namespace waylandcpp
{

/**
 * Epoll wrapper.
 */
class Epoll
{
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

public:
    /**
     * Constructor.
     */
    Epoll();

    /**
     * Destructor.
     */
    ~Epoll();

    /**
     * Adds entry.
     *
     * @param fdHandler
     *      Handler with file descriptor.
     * @param events
     *      Requested events to process.
     *
     * @return
     *      True on success, false on error.
     */
    bool add(EpollFdHandler* fdHandler,
             uint32_t events);

    /**
     * Modifies entry.
     *
     * @param fdHandler
     *      Handler with file descriptor.
     * @param events
     *      Requested events to process.
     *
     * @return
     *      True on success, false on error.
     */
    bool modify(EpollFdHandler* fdHandler,
                uint32_t events);

    /**
     * Removes entry.
     *
     * @param fdHandler
     *      Handler with file descriptor.
     *
     * @return
     *      True on success, false on error.
     */
    bool remove(EpollFdHandler* fdHandler);

    /**
     * Checks if epoll is open.
     *
     * @return
     *      True if open, false if closed.
     */
    bool isOpen() const;

    /**
     * Opens the epoll.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool open();

    /**
     * Closes the epoll.
     */
    void close();

    /**
     * Waits for events.
     *
     * @retval true
     *      True on success (events received and successfully processed).
     * @retval false
     *      Wait error or event processing failed.
     */
    bool wait();

private:
    /**
     * Sets CLOEXEC flag.
     *
     * @param fd
     *      File descriptor.
     *
     * @return
     *      File descriptor on success, -1 on error.
     */
    int fdSetCloexed(int fd);

    /**
     * Sets CLOEXEC flag or closes descriptor on error.
     *
     * @param fd
     *      File descriptor.
     *
     * @return
     *      File descriptor on success, -1 on error.
     */
    int setCloexecOrClose(int fd);

    /**
     * Opens the epoll with CLOEXEC flag set.
     *
     * @return
     *      File descriptor on success, -1 on error.
     */
    int epollCreateCloexec(void);

    /** Epoll file descriptor. */
    int m_epollFd;

    /** Set of added entries. */
    std::set<EpollFdHandler*> m_handlers;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_EPOLL_HPP_*/
