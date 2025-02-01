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


#ifndef WAYLANDCPP_EPOLLDISPLAYHANDLER_HPP_
#define WAYLANDCPP_EPOLLDISPLAYHANDLER_HPP_

#include <cstdint>
#include <sys/epoll.h>

#include "EpollFdHandler.hpp"
#include <waylandcpp-client/Types.hpp>

namespace waylandcpp
{

/**
 * Epoll handler for Wayland Display.
 */
class EpollDisplayHandler : private EpollFdHandler
{
public:
    /**
     * Constructor.
     */
    EpollDisplayHandler();

    /**
     * Destructor.
     */
    virtual ~EpollDisplayHandler() = default;

    /**
     * Installs the handler.
     *
     * @param epoll
     *      Epoll to use.
     * @param display
     *      Display to use.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool install(Epoll* epoll,
                 DisplayPtr display);

    /**
     * Uninstalls the handler.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool uninstall();

    /**
     * Dispatches pending events.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool dispatchPending();

    /**
     * Flushes the display.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool flush();

private:
    /** @copydoc EpollFdHandler::getFd */
    virtual int getFd() const override;

    /** @copydoc EpollFdHandler::handleEvents */
    virtual bool handleEvents(Epoll* epollFd,
                              uint32_t events) override;

    /** Display to use. */
    DisplayPtr m_display;

    /** Epoll in which handler is installed. */
    Epoll* m_epoll;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_EPOLLDISPLAYHANDLER_HPP_*/
