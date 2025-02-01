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


#include "EpollDisplayLoop.hpp"

#include "Display.hpp"

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("GfxEngine", "EpollDisplayLoop");

} // namespace <anonymous>

namespace waylandcpp
{

EpollDisplayLoop::EpollDisplayLoop() :
        m_epoll(new Epoll()),
        m_displayHandler(new EpollDisplayHandler()),
        m_eventHandler(new EpollEventHandler()),
        m_running(false),
        m_stopRequested(false),
        m_listener(nullptr)
{
    // noop
}

EpollDisplayLoop::~EpollDisplayLoop()
{
    if (m_running)
    {
        stop();
    }
}

bool EpollDisplayLoop::start(DisplayPtr display,
                             EpollDisplayLoopListener* listener)
{
    MutexGuard guard(m_mutex);

    g_logger.info("%s", __func__);

    if (m_running.exchange(true))
    {
        g_logger.error("%s - already started", __func__);
        return false;
    }

    do
    {
        m_stopRequested = false;
        m_display = display;
        m_listener = listener;

        if (!m_epoll->open())
        {
            g_logger.error("%s - cannot open epool", __func__);
            break;
        }
        if (!m_eventHandler->install(m_epoll.get(), this))
        {
            g_logger.error("%s - cannot install event handler", __func__);
            break;
        }
        if (!m_displayHandler->install(m_epoll.get(), m_display))
        {
            g_logger.error("%s - cannot install display handler", __func__);
            break;
        }

        m_thread.reset(new std::thread(staticThreadLoop, this));

        g_logger.info("%s - started", __func__);

        return true;
    } while (0);

    /* cleanup on error */
    m_eventHandler->uninstall();
    m_displayHandler->uninstall();
    m_epoll->close();
    m_listener = nullptr;
    m_display.reset();

    m_running = false;

    g_logger.error("%s - start failed", __func__);

    return false;
}

void EpollDisplayLoop::wakeup()
{
    MutexGuard guard(m_mutex);

    g_logger.debug("%s", __func__);

    if (!m_running || m_stopRequested)
    {
        return;
    }

    m_eventHandler->sendWakeup();
}

void EpollDisplayLoop::stop()
{
    MutexGuard guard(m_mutex);

    g_logger.info("%s", __func__);

    if (!m_running)
    {
        g_logger.error("%s - not running", __func__);
        return;
    }

    /* send event to wakup the epoll */
    m_stopRequested = true;
    m_eventHandler->sendWakeup();

    /* wait until the thread finishes */
    m_thread->join();
    m_thread.reset();

    /* cleanup */
    m_eventHandler->uninstall();
    m_displayHandler->uninstall();
    m_epoll->close();
    m_listener = nullptr;
    m_display.reset();

    m_running = false;

    g_logger.info("%s - stopped", __func__);
}

void EpollDisplayLoop::wakeupReceived()
{
    g_logger.debug("%s", __func__);

    if (!m_running)
    {
        g_logger.error("%s - not running", __func__);
        return;
    }

    if (m_listener)
    {
        m_listener->wakeupReceived(this);
    }
}

void EpollDisplayLoop::threadLoop()
{
    g_logger.info("%s", __func__);

    if (m_listener)
    {
        m_listener->started(this);
    }

    for (;;)
    {
        g_logger.trace("%s - loop", __func__);

        if (!m_displayHandler->dispatchPending())
        {
            g_logger.error("%s - dispatch failed", __func__);
            break;
        }

        if (m_stopRequested)
        {
            g_logger.info("%s - stop requested", __func__);
            break;
        }

        if (!m_displayHandler->flush())
        {
            g_logger.error("%s - flush failed", __func__);
            break;
        }

        if (!m_epoll->wait())
        {
            g_logger.error("%s - epoll wait failed", __func__);
            break;
        }
    }

    if (m_listener)
    {
        m_listener->finished(this);
    }

    g_logger.info("%s - finished", __func__);
}

void EpollDisplayLoop::staticThreadLoop(EpollDisplayLoop* thiz)
{
    thiz->threadLoop();
}

} // namespace waylandcpp
