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

#include "XdgToplevel.hpp"
#include "ListenerWrapper.hpp"
#include "ObjectFactory.hpp"
#include <xdg-shell-client-protocol.h>

namespace waylandcpp
{

/** @cond INTERNALS */
class XdgToplevelListenerWrapper : public ListenerWrapper<XdgToplevel, XdgToplevelListener, xdg_toplevel, xdg_toplevel_listener>
{
public:
    XdgToplevelListenerWrapper(WeakObjectPtr owner) :
                               ListenerWrapper(owner, xdg_toplevel_add_listener)
    {
        getListenerStruct().configure = configure;
    }

private:
    static void configure(void *data,
                          struct xdg_toplevel *xdg_toplevel,
                          int32_t width,
                          int32_t height,
                          wl_array *states)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            std::vector<XdgToplevelState> stateVector;
            if (states)
            {
                uint32_t* pos = static_cast<uint32_t*>(states->data);
                uint32_t* end = pos + (states->size/sizeof(uint32_t));

                // wl_array_for_each(pos, states)
                for (; pos < end; pos++)
                {
                    switch (*pos)
                    {
                        case XDG_TOPLEVEL_STATE_MAXIMIZED:
                            stateVector.push_back(XdgToplevelState::MAXIMIZED);
                            break;
                        case XDG_TOPLEVEL_STATE_FULLSCREEN:
                            stateVector.push_back(XdgToplevelState::FULLSCREEN);
                            break;
                        case XDG_TOPLEVEL_STATE_RESIZING:
                            stateVector.push_back(XdgToplevelState::RESIZING);
                            break;
                        case XDG_TOPLEVEL_STATE_ACTIVATED:
                            stateVector.push_back(XdgToplevelState::ACTIVATED);
                            break;
                        case XDG_TOPLEVEL_STATE_TILED_LEFT:
                            stateVector.push_back(XdgToplevelState::TILED_LEFT);
                            break;
                        case XDG_TOPLEVEL_STATE_TILED_RIGHT:
                            stateVector.push_back(XdgToplevelState::TILED_RIGHT);
                            break;
                        case XDG_TOPLEVEL_STATE_TILED_TOP:
                            stateVector.push_back(XdgToplevelState::TILED_TOP);
                            break;
                        case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
                            stateVector.push_back(XdgToplevelState::TILED_BOTTOM);
                            break;
                        default:
                            break;
                    }
                }
            }
            listener->configure(owner, width, height, stateVector);
        }
    }
};
/** @endcond INTERNALS */

const wl_interface* XdgToplevel::getWlInterface()
{
    return &xdg_toplevel_interface;
}

XdgToplevel::XdgToplevel(WaylandObjectType* const wlObject,
                         DestructorFunc const destructorFunction) :
                         Proxy(wlObject, destructorFunction)
{
    // noop
}

XdgToplevel::~XdgToplevel()
{
    // noop
}
/*
uint32_t XdgToplevel::getVersion() const
{
    return xdg_toplevel_get_version(getNativeObject());
}*/

void XdgToplevel::setUserData(void* userData)
{
    return xdg_toplevel_set_user_data(getNativeObject(), userData);
}

void* XdgToplevel::getUserData() const
{
    return xdg_toplevel_get_user_data(getNativeObject());
}

bool XdgToplevel::setListener(XdgToplevelListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new XdgToplevelListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

XdgToplevel1::XdgToplevel1(WaylandObjectType* const wlObject) :
        XdgToplevel(wlObject, xdg_toplevel_destroy)
{
    // noop
}

} // namespace waylandcpp
