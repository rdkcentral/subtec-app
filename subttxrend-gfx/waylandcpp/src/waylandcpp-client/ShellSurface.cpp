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


#include "ShellSurface.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "Output.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class ShellSurfaceListenerWrapper : public ListenerWrapper<ShellSurface,
        ShellSurfaceListener, wl_shell_surface, wl_shell_surface_listener>
{
public:
    ShellSurfaceListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_shell_surface_add_listener)
    {
        getListenerStruct().ping = callbackPing;
        getListenerStruct().configure = callbackConfigure;
        getListenerStruct().popup_done = callbackPopupDone;
    }

private:
    static void callbackPing(void *data,
                             struct wl_shell_surface */*wl_shell_surface*/,
                             uint32_t serial)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->ping(owner, serial);
        }
    }

    static void callbackConfigure(void *data,
                                  struct wl_shell_surface */*wl_shell_surface*/,
                                  uint32_t edges,
                                  int32_t width,
                                  int32_t height)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->configure(owner, edges, width, height);
        }
    }

    static void callbackPopupDone(void *data,
                                  struct wl_shell_surface */*wl_shell_surface*/)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->popupDone(owner);
        }
    }

};
/** @endcond INTERNALS */

//-------------------------

const wl_interface* ShellSurface::getWlInterface()
{
    return &wl_shell_surface_interface;
}

ShellSurface::ShellSurface(WaylandObjectType* const wlObject,
                           DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

ShellSurface::~ShellSurface()
{
    // noop
}

void ShellSurface::setUserData(void* userData)
{
    return wl_shell_surface_set_user_data(getNativeObject(), userData);
}

void* ShellSurface::getUserData() const
{
    return wl_shell_surface_get_user_data(getNativeObject());
}

bool ShellSurface::setListener(ShellSurfaceListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new ShellSurfaceListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

ShellSurface1::ShellSurface1(WaylandObjectType* const wlObject) :
        ShellSurface(wlObject, wl_shell_surface_destroy)
{
    // noop
}

void ShellSurface1::pong(uint32_t serial)
{
    wl_shell_surface_pong(getNativeObject(), serial);
}

void ShellSurface1::setTopLevel()
{
    wl_shell_surface_set_toplevel(getNativeObject());
}

void ShellSurface1::setFullScreen(FullScreenMethod method,
                                  uint32_t framerate,
                                  OutputPtr output)
{
    uint32_t nativeMethod = WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT;

    switch(method)
    {
    case FullScreenMethod::DEFAULT:
        nativeMethod = WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT;
        break;
    case FullScreenMethod::SCALE:
        nativeMethod = WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE;
        break;
    case FullScreenMethod::DRIVER:
        nativeMethod = WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER;
        break;
    case FullScreenMethod::FILL:
        nativeMethod = WL_SHELL_SURFACE_FULLSCREEN_METHOD_FILL;
        break;
    }

    wl_shell_surface_set_fullscreen(getNativeObject(), nativeMethod, framerate,
            output ? output->getNativeObject() : nullptr);
}

void ShellSurface1::setMaximized(OutputPtr output)
{
    wl_shell_surface_set_maximized(getNativeObject(),
            output ? output->getNativeObject() : nullptr);
}

} // namespace waylandcpp
