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


#ifndef SUBTTXREND_GFX_WAYLAND_LOOP_EPOLL_HPP_
#define SUBTTXREND_GFX_WAYLAND_LOOP_EPOLL_HPP_

#include "WaylandBackendLoop.hpp"

#include "waylandcpp-utils/EpollDisplayLoop.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Backend loop - thread will epoll based.
 */
class WaylandBackendLoopEpoll : public WaylandBackendLoop,
                                private waylandcpp::EpollDisplayLoopListener
{
public:
    /**
     * Constructor.
     */
    WaylandBackendLoopEpoll() :
            m_listener(nullptr)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~WaylandBackendLoopEpoll()
    {
        internalStop();
    }

    bool isSyncNeeded() const override
    {
        return true;
    }

    bool start(waylandcpp::DisplayPtr display,
                       WaylandBackendLoopListener* listener) override
    {
        m_listener = listener;
        return m_displayLoop.start(display, this);
    }

    void stop() override
    {
        internalStop();
    }

    void requestWakeup() override
    {
        m_displayLoop.wakeup();
    }

private:
    void internalStop()
    {
        m_displayLoop.stop();
    }
protected:
    void started(waylandcpp::EpollDisplayLoop* /*loop*/) override
    {
        if (m_listener)
        {
            m_listener->loopStarted();
        }
    }

    void finished(waylandcpp::EpollDisplayLoop* /*loop*/) override
    {
        if (m_listener)
        {
            m_listener->loopFinished();
        }
    }

    void wakeupReceived(waylandcpp::EpollDisplayLoop* /*loop*/) override
    {
        if (m_listener)
        {
            m_listener->loopWakeupReceived();
        }
    }

    /** Epool loop instance. */
    waylandcpp::EpollDisplayLoop m_displayLoop;

    /** Listener to call with events. */
    WaylandBackendLoopListener* m_listener;
};

}
// namespace gfx
}// namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_LOOP_EPOLL_HPP_
