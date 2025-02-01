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


#ifndef WAYLANDCPP_EPOLLEVENTHANDLER_HPP_
#define WAYLANDCPP_EPOLLEVENTHANDLER_HPP_

#include "EpollFdHandler.hpp"

namespace waylandcpp
{

/**
 * Listener for Epoll Event Handler events.
 */
class EpollEventHandlerListener
{
public:
    /**
     * Constructor.
     */
    EpollEventHandlerListener() = default;

    /**
     * Destructor.
     */
    virtual ~EpollEventHandlerListener() = default;

    /**
     * Wakup event received.
     */
    virtual void wakeupReceived() = 0;
};

/**
 * Epoll event handler.
 *
 * This handler is used to break the epoll wait by providing
 * wakup events.
 *
 * Uses eventfd internally.
 */
class EpollEventHandler : private EpollFdHandler
{
public:
    /**
     * Constructor.
     */
    EpollEventHandler();

    /**
     * Destructor.
     */
    virtual ~EpollEventHandler();

    /**
     * Installs the handler in epoll.
     *
     * @param epoll
     *      Epoll to use.
     * @param listener
     *      Listener to call on events.
     *
     * @return
     *      True on success, false on error.
     */
    bool install(Epoll* epoll,
                 EpollEventHandlerListener* listener);

    /**
     * Uninstalls the handler.
     *
     * @return
     *      True on success, false on error.
     */
    bool uninstall();

    /**
     * Sends wakeup.
     *
     * @return
     *      True on success, false on error.
     */
    bool sendWakeup();

private:
    /** @copydoc EpollFdHandler::getFd */
    virtual int getFd() const override;

    /** @copydoc EpollFdHandler::handleEvents */
    virtual bool handleEvents(Epoll* epollFd,
                              uint32_t events) override;

    /** Eventfd descriptor. */
    int m_eventFd;

    /** Epoll in which handler is installed. */
    Epoll* m_epoll;

    /** Listener to call on events. */
    EpollEventHandlerListener* m_listener;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_EPOLLEVENTHANDLER_HPP_*/
