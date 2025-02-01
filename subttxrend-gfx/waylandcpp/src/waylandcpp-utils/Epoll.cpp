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


#include "Epoll.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("GfxEngine", "Epoll");

} // namespace <anonymous>

namespace waylandcpp
{

Epoll::Epoll() :
        m_epollFd(-1)
{
    // noop
}

Epoll::~Epoll()
{
    close();
}

bool Epoll::add(EpollFdHandler* fdHandler,
                uint32_t events)
{
    g_logger.trace("%s - handler=%p events=%u", __func__, fdHandler, events);

    if (!isOpen())
    {
        g_logger.error("%s - not open", __func__);
        return false;
    }

    if (!fdHandler)
    {
        g_logger.error("%s - null handler", __func__);
        return false;
    }

    if (m_handlers.find(fdHandler) != m_handlers.end())
    {
        g_logger.error("%s - already added", __func__);
        return false;
    }

    const int handlerFd = fdHandler->getFd();
    if (handlerFd == -1)
    {
        g_logger.error("%s - invalid descriptor", __func__);
        return false;
    }

    epoll_event event;

    event.events = events;
    event.data.ptr = fdHandler;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, handlerFd, &event) != 0)
    {
        g_logger.error("%s - epoll error: %d", __func__, errno);
        return false;
    }

    m_handlers.insert(fdHandler);

    return true;
}

bool Epoll::modify(EpollFdHandler* fdHandler,
                   uint32_t events)
{
    g_logger.trace("%s - handler=%p events=%u", __func__, fdHandler, events);

    if (!isOpen())
    {
        g_logger.error("%s - not open", __func__);
        return false;
    }

    if (!fdHandler)
    {
        g_logger.error("%s - null handler", __func__);
        return false;
    }

    if (m_handlers.find(fdHandler) == m_handlers.end())
    {
        g_logger.error("%s - already added", __func__);
        return false;
    }

    const int handlerFd = fdHandler->getFd();
    if (handlerFd == -1)
    {
        g_logger.error("%s - invalid descriptor", __func__);
        return false;
    }

    epoll_event event;

    event.events = events;
    event.data.ptr = fdHandler;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, handlerFd, &event) != 0)
    {
        g_logger.error("%s - epoll error: %d", __func__, errno);
        return false;
    }

    return true;
}

bool Epoll::remove(EpollFdHandler* fdHandler)
{
    g_logger.trace("%s - handler=%p", __func__, fdHandler);

    if (!isOpen())
    {
        g_logger.error("%s - not open", __func__);
        return false;
    }

    if (!fdHandler)
    {
        g_logger.error("%s - null handler", __func__);
        return false;
    }

    if (m_handlers.find(fdHandler) == m_handlers.end())
    {
        g_logger.error("%s - already added", __func__);
        return false;
    }

    const int handlerFd = fdHandler->getFd();
    if (handlerFd == -1)
    {
        g_logger.error("%s - invalid descriptor", __func__);
        return false;
    }

    epoll_event event;

    event.events = 0;
    event.data.ptr = fdHandler;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, handlerFd, &event) != 0)
    {
        g_logger.error("%s - epoll error: %d", __func__, errno);
        return false;
    }

    m_handlers.erase(m_handlers.find(fdHandler));

    return true;
}

bool Epoll::isOpen() const
{
    return m_epollFd != -1;
}

bool Epoll::open()
{
    g_logger.trace("%s", __func__);

   if (isOpen())
    {
        g_logger.error("%s - already open", __func__);
        return false;
    }

    m_epollFd = epollCreateCloexec();

    if (!isOpen())
    {
        g_logger.error("%s - open failed: %d", __func__, errno);
    }

    return isOpen();
}

void Epoll::close()
{
    g_logger.trace("%s", __func__);

    if (isOpen())
    {
        (void)::close(m_epollFd);
        m_epollFd = -1;

        m_handlers.clear();
    }
}

bool Epoll::wait()
{
    g_logger.trace("%s", __func__);

    if (!isOpen())
    {
        g_logger.error("%s - not open", __func__);
        return false;
    }

    static const int MAX_EVENTS = 10;

    epoll_event events[MAX_EVENTS];

    int count = epoll_wait(m_epollFd, &events[0], MAX_EVENTS, -1);
    if (count < 0)
    {
        if (errno == EINTR)
        {
            g_logger.info("%s - interrupted. Ignoring.", __func__);
            return true;
        }
        else
        {
            g_logger.error("%s - wait failed: %d", __func__, errno);
            return false;
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            EpollFdHandler* handler =
                    reinterpret_cast<EpollFdHandler*>(events[i].data.ptr);

            if (!handler->handleEvents(this, events[i].events))
            {
                g_logger.error("%s - handler failed: %p", __func__, handler);
                return false;
            }
        }

        return true;
    }
}

int Epoll::fdSetCloexed(int fd)
{
    if (fd == -1)
    {
        return -1;
    }

    long flags = ::fcntl(fd, F_GETFD);
    if (flags == -1)
    {
        return -1;
    }

    if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
    {
        return -1;
    }

    return 0;
}

int Epoll::setCloexecOrClose(int fd)
{
    if (fdSetCloexed(fd) != 0)
    {
        ::close(fd);
        return -1;
    }
    return fd;
}

int Epoll::epollCreateCloexec(void)
{
    int fd;

#ifdef EPOLL_CLOEXEC
    fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (fd >= 0)
    {
        return fd;
    }
    if (errno != EINVAL)
    {
        return -1;
    }
#endif
    fd = ::epoll_create(1);
    return setCloexecOrClose(fd);
}

} // namespace waylandcpp
