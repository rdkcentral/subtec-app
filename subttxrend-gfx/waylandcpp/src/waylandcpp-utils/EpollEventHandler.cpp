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


#include "EpollEventHandler.hpp"

#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>

#include "Epoll.hpp"

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("GfxEngine", "EpollEventHandler");

} // namespace <anonymous>

namespace waylandcpp
{

EpollEventHandler::EpollEventHandler() :
        m_eventFd(-1),
        m_epoll(nullptr),
        m_listener(nullptr)
{
    // noop
}

EpollEventHandler::~EpollEventHandler()
{
    if (m_eventFd != -1)
    {
        (void) ::close(m_eventFd);
    }
}

bool EpollEventHandler::install(Epoll* epoll,
                                EpollEventHandlerListener* listener)
{
    g_logger.trace("%s", __func__);

    if (m_eventFd != -1)
    {
        g_logger.error("%s - already installed", __func__);
        return false;
    }

    m_eventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (m_eventFd == -1)
    {
        g_logger.error("%s - cannot create eventfd", __func__);
        return false;
    }

    if (!epoll->add(this, EPOLLIN))
    {
        g_logger.error("%s - cannot add to epool", __func__);

        (void) ::close(m_eventFd);
        m_eventFd = -1;
        return false;
    }

    m_epoll = epoll;
    m_listener = listener;

    return true;
}

bool EpollEventHandler::uninstall()
{
    g_logger.trace("%s", __func__);

    if (m_eventFd == -1)
    {
        g_logger.error("%s - not installed", __func__);
        return false;
    }

    bool result = m_epoll->remove(this);
    if (!result)
    {
        g_logger.error("%s - cannot remove from epoll", __func__);
    }

    (void) ::close(m_eventFd);

    m_eventFd = -1;
    m_epoll = nullptr;
    m_listener = nullptr;

    return true;
}

int EpollEventHandler::getFd() const
{
    return m_eventFd;
}

bool EpollEventHandler::handleEvents(Epoll* epollFd,
                                     uint32_t events)
{
    if ((events & EPOLLIN) != 0)
    {
        uint64_t value = 0;

        int result = ::read(m_eventFd, &value, sizeof(value));
        if (result == -1)
        {
            if (errno != EAGAIN)
            {
                g_logger.error("%s - error received", __func__);
                return false;
            }

            // shall never happen, ignore

            return true;
        }
        else if (result != sizeof(value))
        {
            g_logger.error("%s - read failed: %d", __func__, result);
            return false;
        }

        m_listener->wakeupReceived();
    }

    return true;
}

bool EpollEventHandler::sendWakeup()
{
    if (m_eventFd == -1)
    {
        g_logger.error("%s - not started", __func__);
        return false;
    }

    uint64_t value = 1;

    int result = ::write(m_eventFd, &value, sizeof(value));
    if (result == -1)
    {
        if (errno != EAGAIN)
        {
            g_logger.error("%s - error received", __func__);
            return false;
        }

        // already some wakeups in progress, ignore
    }
    else if (result != sizeof(value))
    {
        g_logger.error("%s - write failed: %d", __func__, result);
        return false;
    }

    return true;
}

} // namespace waylandcpp
