/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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
******************************************************************************/

#include "SimpleShell.hpp"

#include <cassert>

#include <simpleshell-client-protocol.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class SimpleShellListenerWrapper : public ListenerWrapper<SimpleShell, SimpleShellListener,
        wl_simple_shell, wl_simple_shell_listener>
{
public:
    SimpleShellListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_simple_shell_add_listener)
    {
        getListenerStruct().surface_id        = callbackSurfaceId;
        getListenerStruct().surface_created   = callbackSurfaceCreated;
        getListenerStruct().surface_destroyed = callbackSurfaceDestroyed;
        getListenerStruct().surface_status    = callbackSurfaceStatus;
        getListenerStruct().get_surfaces_done = callbackGetSurfacesDone;
    }

private:
    static void callbackSurfaceId(void *data,
                                  struct wl_simple_shell * /*wl_simple_shell*/,
                                  struct wl_surface *surface,
                                  uint32_t surfaceId)
    {
        auto owner = getOwner(data);

        if(owner)
        {
            auto listener = getListener(data);
            listener->surfaceId(owner, surface, surfaceId);
        }
    }

    static void callbackSurfaceCreated(void *data,
                                       struct wl_simple_shell * /*wl_simple_shell*/,
                                       uint32_t surfaceId,
                                       const char *name)
    {
        auto owner = getOwner(data);

        if(owner)
        {
            auto listener = getListener(data);
            listener->surfaceCreated(owner, surfaceId, name);
        }
    }

    static void callbackSurfaceDestroyed(void *data,
                                         struct wl_simple_shell * /*wl_simple_shell*/,
                                         uint32_t surfaceId,
                                         const char* name)
    {
        auto owner = getOwner(data);

        if(owner)
        {
            auto listener = getListener(data);
            listener->surfaceDestroyed(owner, surfaceId, name);
        }
    }

    static void callbackSurfaceStatus(void *data,
                                      struct wl_simple_shell * /*wl_simple_shell*/,
                                      uint32_t surfaceId,
                                      const char *name,
                                      uint32_t visible,
                                      int32_t x,
                                      int32_t y,
                                      int32_t width,
                                      int32_t height,
                                      wl_fixed_t opacity,
                                      wl_fixed_t zorder)
    {
        auto owner = getOwner(data);

        if(owner)
        {
            auto listener = getListener(data);
            listener->surfaceStatus(owner,
                                    surfaceId,
                                    name,
                                    visible,
                                    x,
                                    y,
                                    width,
                                    height,
                                    opacity,
                                    zorder);
        }
    }

    static void callbackGetSurfacesDone(void *data,
                                       struct wl_simple_shell * /*wl_simple_shell*/)
    {
        auto owner = getOwner(data);

        if(owner)
        {
            auto listener = getListener(data);
            listener->surfacesDone(owner);
        }
    }
};
/** @endcond INTERNALS */

const wl_interface* SimpleShell::getWlInterface()
{
    return &wl_simple_shell_interface;
}

SimpleShell::SimpleShell(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

SimpleShell::~SimpleShell()
{
    // noop
}

void SimpleShell::setUserData(void* userData)
{
    return wl_simple_shell_set_user_data(getNativeObject(), userData);
}

void* SimpleShell::getUserData() const
{
    return wl_simple_shell_get_user_data(getNativeObject());
}

bool SimpleShell::setListener(SimpleShellListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new SimpleShellListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

SimpleShell1::SimpleShell1(WaylandObjectType* const wlObject) :
        SimpleShell(wlObject, wl_simple_shell_destroy)
{
    // noop
}

SimpleShell1::~SimpleShell1()
{
}

} // namespace waylandcpp
