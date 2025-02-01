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


#include "Registry.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class RegistryListenerWrapper : public ListenerWrapper<Registry,
        RegistryListener, wl_registry, wl_registry_listener>
{
public:
    RegistryListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_registry_add_listener)
    {
        getListenerStruct().global = callbackGlobal;
        getListenerStruct().global_remove = callbackGlobalRemove;
    }

private:
    static void callbackGlobal(void* data,
                               struct wl_registry* /*wlRegistry*/,
                               uint32_t name,
                               const char* interface,
                               uint32_t version)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->global(owner, name, interface, version);
        }
    }

    static void callbackGlobalRemove(void* data,
                                     struct wl_registry* /*wlRegistry*/,
                                     uint32_t name)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->globalRemove(owner, name);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------

Registry::Registry(WaylandObjectType* const wlObject,
                   DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Registry::~Registry()
{
    // noop
}

void Registry::setUserData(void* userData)
{
    return wl_registry_set_user_data(getNativeObject(), userData);
}

void* Registry::getUserData() const
{
    return wl_registry_get_user_data(getNativeObject());
}

bool Registry::setListener(RegistryListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new RegistryListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Registry1::Registry1(WaylandObjectType* const wlObject) :
        Registry(wlObject, wl_registry_destroy)
{
    // noop
}

void* Registry1::nativeBind(uint32_t name,
                            const wl_interface* interface,
                            uint32_t version)
{
    return wl_registry_bind(getNativeObject(), name, interface, version);
}

} // namespace waylandcpp
