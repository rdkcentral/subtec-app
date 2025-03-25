/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
 */

#include "Surface.hpp"
#include "XdgSurface.hpp"
#include "ListenerWrapper.hpp"

#include <cassert>

#include <xdg-shell-client-protocol.h>

#include "ObjectFactory.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class XdgSurfaceListenerWrapper : public ListenerWrapper<XdgSurface, XdgSurfaceListener, xdg_surface, xdg_surface_listener>
{
public:
    XdgSurfaceListenerWrapper(WeakObjectPtr owner) :
                              ListenerWrapper(owner, xdg_surface_add_listener)
    {
        getListenerStruct().configure = configure;
    }

private:
    static void configure(void *data,
                          struct xdg_surface *xdg_surface,
                          uint32_t serial)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->configure(owner, serial);
        }
    }
};
/** @endcond INTERNALS */

const wl_interface* XdgSurface::getWlInterface()
{
    return &xdg_surface_interface;
}

XdgSurface::XdgSurface(WaylandObjectType* const wlObject,
                       DestructorFunc const destructorFunction) :
                       Proxy(wlObject, destructorFunction)
{
    // noop
}

XdgSurface::~XdgSurface()
{
    // noop
}
/*
uint32_t XdgSurface::getVersion() const
{
    return xdg_surface_get_version(getNativeObject());
}*/

void XdgSurface::setUserData(void* userData)
{
    return xdg_surface_set_user_data(getNativeObject(), userData);
}

void* XdgSurface::getUserData() const
{
    return xdg_surface_get_user_data(getNativeObject());
}

bool XdgSurface::setListener(XdgSurfaceListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new XdgSurfaceListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

XdgSurface1::XdgSurface1(WaylandObjectType* const wlObject) :
                         XdgSurface(wlObject, xdg_surface_destroy)
{
    // noop
}

void XdgSurface1::ackConfigure(uint32_t serial)
{
    static_assert(XDG_SURFACE_ACK_CONFIGURE_SINCE_VERSION == OBJECT_VERSION, "Wayland API broken");

    xdg_surface_ack_configure(getNativeObject(), serial);
}

XdgToplevelPtr XdgSurface1::getToplevel()
{
    static_assert(XDG_SURFACE_GET_TOPLEVEL_SINCE_VERSION == OBJECT_VERSION, "Wayland API broken");

    auto xdgToplevel = xdg_surface_get_toplevel(getNativeObject());
    return ObjectFactory::create(xdgToplevel);
}

} // namespace waylandcpp
