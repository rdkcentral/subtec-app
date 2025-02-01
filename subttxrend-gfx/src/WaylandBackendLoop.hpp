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


#ifndef SUBTTXREND_GFX_WAYLAND_LOOP_HPP_
#define SUBTTXREND_GFX_WAYLAND_LOOP_HPP_

#include "waylandcpp-client/Display.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Listener for backend loop events.
 */
class WaylandBackendLoopListener
{
public:
    /**
     * Constructor.
     */
    WaylandBackendLoopListener() = default;

    /**
     * Destructor.
     */
    virtual ~WaylandBackendLoopListener() = default;

    /**
     * Display events loop started.
     */
    virtual void loopStarted() = 0;

    /**
     * Display events loop started.
     */
    virtual void loopFinished() = 0;

    /**
     * Wakup event received.
     */
    virtual void loopWakeupReceived() = 0;
};

/**
 * Backend loop.
 */
class WaylandBackendLoop
{
public:
    /**
     * Constructor.
     */
    WaylandBackendLoop() = default;

    /**
     * Destructor.
     */
    virtual ~WaylandBackendLoop() = default;

    /**
     * Checks if synchronization is needed.
     *
     * Synchronization is usually only needed when loop runs its own thread.
     *
     * @return
     *      True if synchronization is needed, false otherwise.
     */
    virtual bool isSyncNeeded() const = 0;

    /**
     * Starts the loop.
     *
     * @param display
     *      Display for which loop is created.
     * @param listener
     *      Listener to be called with events.
     *
     * @return
     *      True on successs, false on error.
     */
    virtual bool start(waylandcpp::DisplayPtr display,
                       WaylandBackendLoopListener* listener) = 0;

    /**
     * Stops the loop.
     */
    virtual void stop() = 0;

    /**
     * Requests loop wakeup.
     */
    virtual void requestWakeup() = 0;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_LOOP_HPP_
