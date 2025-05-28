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


#include "WaylandBackendLoopGlib.hpp"

#include <cstdint>
#include <cstring>

#include "wayland-client.h"

namespace subttxrend
{
namespace gfx
{

namespace
{

struct WaylandDisplaySource
{
    /** Source base class. */
    GSource m_source;

    /** PollFD descriptor. */
    GPollFD m_pfd;

    /** Wayland display pointer. */
    waylandcpp::DisplayPtr m_display;

    /** Listener to call with events. */
    WaylandBackendLoopListener* m_listener;

    /** Start callback flag. */
    bool m_startRequested;

    /** Wakeup requested flag. */
    bool m_wakeupRequested;
};

const gint SOURCE_TIMEOUT_MS = 10;

gboolean waylandDisplaySourcePrepare(GSource *base,
                                     gint *timeout)
{
    auto source = reinterpret_cast<WaylandDisplaySource*>(base);

    *timeout = SOURCE_TIMEOUT_MS;

    source->m_pfd.revents = 0;

    source->m_display->dispatchPending();

    /*
     * normally it would be needed to change the mask
     * but it is safe just to flush as many events as possible
     */
    source->m_display->flush();

    if (source->m_wakeupRequested || source->m_startRequested)
    {
        // ready to dispatch
        return TRUE;
    }

    return FALSE;
}

gboolean waylandDisplaySourceCheck(GSource *base)
{
    auto source = reinterpret_cast<WaylandDisplaySource*>(base);

    if ((source->m_pfd.revents != 0) || source->m_wakeupRequested
            || source->m_startRequested)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

gboolean waylandDisplaySourceDispatch(GSource *base,
                                      GSourceFunc /*callback*/,
                                      gpointer /*data*/)
{
    auto source = reinterpret_cast<WaylandDisplaySource*>(base);

    if (source->m_pfd.revents & G_IO_IN)
    {
        source->m_display->dispatch();
    }


    if (source->m_startRequested)
    {
        if (source->m_listener)
        {
            source->m_listener->loopStarted();
        }

        source->m_startRequested = false;
    }

    if (source->m_wakeupRequested)
    {
        if (source->m_listener)
        {
            source->m_listener->loopWakeupReceived();
        }
        source->m_wakeupRequested = false;
    }

    return G_SOURCE_CONTINUE;
}

void waylandDisplaySourceFinalize(GSource* base)
{
    auto source = reinterpret_cast<WaylandDisplaySource*>(base);

    if (source->m_listener)
    {
        source->m_listener->loopFinished();
    }

    source->m_display.reset();
}

void waylandDisplaySourceSetWakeup(GSource* base)
{
    auto source = reinterpret_cast<WaylandDisplaySource*>(base);

    source->m_wakeupRequested = true;
}

// ugly workaround for old GCC versions
bool g_waylandDisplaySourceFuncsInited = false;
GSourceFuncs g_waylandDisplaySourceFuncs;

GSource* waylandDisplaySourceNew(waylandcpp::DisplayPtr display,
                                 WaylandBackendLoopListener* listener)
{
    if (!g_waylandDisplaySourceFuncsInited)
    {
        (void)std::memset(&g_waylandDisplaySourceFuncs, 0,
                sizeof(g_waylandDisplaySourceFuncs));

        g_waylandDisplaySourceFuncs.prepare = waylandDisplaySourcePrepare;
        g_waylandDisplaySourceFuncs.check = waylandDisplaySourceCheck;
        g_waylandDisplaySourceFuncs.dispatch = waylandDisplaySourceDispatch;
        g_waylandDisplaySourceFuncs.finalize = waylandDisplaySourceFinalize;
        g_waylandDisplaySourceFuncsInited = true;
    }

    auto source = reinterpret_cast<WaylandDisplaySource*>(g_source_new(
            &g_waylandDisplaySourceFuncs, sizeof(WaylandDisplaySource)));

    source->m_display = display;
    source->m_listener = listener;
    source->m_startRequested = true;
    source->m_wakeupRequested = false;

    source->m_pfd.fd = display->getFd();
    source->m_pfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;

    g_source_add_poll(&source->m_source, &source->m_pfd);

    return &source->m_source;
}

}

WaylandBackendLoopGlib::WaylandBackendLoopGlib() :
        m_source(nullptr)
{
// noop
}

WaylandBackendLoopGlib::~WaylandBackendLoopGlib()
{
    // Explicit call to specific stop() method from WaylandBackendLoopGlib is a result of static code analyser warning
    // as described here: https://pvs-studio.com/en/blog/posts/cpp/0891/
    WaylandBackendLoopGlib::stop();
}

bool WaylandBackendLoopGlib::start(waylandcpp::DisplayPtr display,
                                   WaylandBackendLoopListener* listener)
{
    m_source = waylandDisplaySourceNew(display, listener);

    g_source_attach(m_source, nullptr);

    return true;
}

void WaylandBackendLoopGlib::stop()
{
    g_source_destroy(m_source);
}

void WaylandBackendLoopGlib::requestWakeup()
{
    if (m_source)
    {
        waylandDisplaySourceSetWakeup(m_source);
    }
}

}
// namespace gfx
}// namespace subttxrend
