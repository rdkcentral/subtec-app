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


#include "EpollDisplayHandler.hpp"

#include "Epoll.hpp"
#include <cerrno>
#include <waylandcpp-client/Display.hpp>

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("GfxEngine", "EpollDisplayHandler");

} // namespace <anonymous>

namespace waylandcpp
{

EpollDisplayHandler::EpollDisplayHandler() :
        EpollFdHandler(),
        m_display(nullptr),
        m_epoll(nullptr)
{
    // nothing to do
}

bool EpollDisplayHandler::install(Epoll* epoll,
                                  DisplayPtr display)
{
    g_logger.trace("%s", __func__);

    if (m_epoll)
    {
        g_logger.error("%s - already installed", __func__);
        return false;
    }

    m_display = display;

    if (!epoll->add(this, EPOLLIN | EPOLLERR | EPOLLHUP))
    {
        g_logger.error("%s - cannot add to epoll", __func__);

        m_display.reset();
        return false;
    }

    m_epoll = epoll;

    return true;
}

bool EpollDisplayHandler::uninstall()
{
    g_logger.trace("%s", __func__);

    if (!m_epoll)
    {
        g_logger.error("%s - not installed", __func__);
        return false;
    }

    bool result = m_epoll->remove(this);
    if (!result)
    {
        g_logger.error("%s - cannot remove from epoll", __func__);
    }
    (void)m_display->flush();

    m_epoll = nullptr;

    return result;
}

int EpollDisplayHandler::getFd() const
{
    return m_display->getFd();
}

bool EpollDisplayHandler::dispatchPending()
{
    g_logger.trace("%s", __func__);

    if (!m_epoll)
    {
        g_logger.error("%s - not installed", __func__);
        return false;
    }

    m_display->dispatchPending();

    return true;
}

bool EpollDisplayHandler::flush()
{
    g_logger.trace("%s", __func__);

    if (!m_epoll)
    {
        g_logger.error("%s - not installed", __func__);
        return false;
    }

    int flushResult = m_display->flush();
    if ((flushResult < 0) && (errno == EAGAIN))
    {
        if (!m_epoll->modify(this, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP))
        {
            g_logger.error("%s - cannot modify in epoll", __func__);
            return false;
        }
    }
    else if (flushResult < 0)
    {
        g_logger.error("%s - flush failed", __func__);
        return false;
    }

    return true;
}

bool EpollDisplayHandler::handleEvents(Epoll* epollFd,
                                       uint32_t events)
{
    g_logger.trace("%s events=%u", __func__, events);

    if ((events & EPOLLERR) || (events & EPOLLHUP))
    {
        g_logger.error("%s - error received", __func__);
        return false;
    }

    if (events & EPOLLIN)
    {
        g_logger.trace("%s - input ready received", __func__);

        int ret = m_display->dispatch();
        if (ret == -1)
        {
            g_logger.error("%s - dispatch failed", __func__);
            return false;
        }
    }

    if (events & EPOLLOUT)
    {
        g_logger.trace("%s - output ready received", __func__);

        int flushResult = m_display->flush();
        if (flushResult == 0)
        {
            if (!m_epoll->modify(this,  EPOLLIN | EPOLLERR | EPOLLHUP))
            {
                g_logger.error("%s - cannot modify in epoll", __func__);
            }
        }
        else if ((flushResult == -1) && (errno != EAGAIN))
        {
            g_logger.error("%s - flush failed", __func__);
            return false;
        }
    }

    return true;
}

}
 // namespace waylandcpp
