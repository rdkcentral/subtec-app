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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#include "XdgWmBase.hpp"
#include "ListenerWrapper.hpp"
#include "Surface.hpp"
#include "ObjectFactory.hpp"
#include <xdg-shell-client-protocol.h>

namespace waylandcpp
{

/** @cond INTERNALS */
class XdgWmBaseListenerWrapper : public ListenerWrapper<XdgWmBase, XdgWmBaseListener, xdg_wm_base, xdg_wm_base_listener>
{
public:
    XdgWmBaseListenerWrapper(WeakObjectPtr owner) :
                             ListenerWrapper(owner, xdg_wm_base_add_listener)
    {
        getListenerStruct().ping = ping;
    }

private:
    static void ping(void *data,
                     struct xdg_wm_base *xdg_wm_base,
                     uint32_t serial)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->ping(owner, serial);
        }
    }
};
/** @endcond INTERNALS */

const wl_interface* XdgWmBase::getWlInterface()
{
    return &xdg_wm_base_interface;
}

XdgWmBase::XdgWmBase(WaylandObjectType* const wlObject,
                     DestructorFunc const destructorFunction) :
                     Proxy(wlObject, destructorFunction)
{
    // noop
}

XdgWmBase::~XdgWmBase()
{
    // noop
}
/*
uint32_t XdgWmBase::getVersion() const
{
    return xdg_wm_base_get_version(getNativeObject());
}*/

void XdgWmBase::setUserData(void* userData)
{
    return xdg_wm_base_set_user_data(getNativeObject(), userData);
}

void* XdgWmBase::getUserData() const
{
    return xdg_wm_base_get_user_data(getNativeObject());
}

bool XdgWmBase::setListener(XdgWmBaseListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new XdgWmBaseListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

XdgWmBase1::XdgWmBase1(WaylandObjectType* const wlObject) :
        XdgWmBase(wlObject, xdg_wm_base_destroy)
{
    // noop
}

void XdgWmBase1::pong(uint32_t serial)
{
    static_assert(XDG_WM_BASE_PONG_SINCE_VERSION == OBJECT_VERSION, "Wayland API broken");

    xdg_wm_base_pong(getNativeObject(), serial);
}

XdgSurfacePtr XdgWmBase1::getXdgSurface(SurfacePtr surface)
{
    if (!surface)
    {
        return nullptr;
    }

    static_assert(XDG_WM_BASE_GET_XDG_SURFACE_SINCE_VERSION == OBJECT_VERSION, "Wayland API broken");

    auto xdgSurface = xdg_wm_base_get_xdg_surface(getNativeObject(),
                                                  surface->getNativeObject());
    return ObjectFactory::create(xdgSurface);
}

} // namespace waylandcpp
