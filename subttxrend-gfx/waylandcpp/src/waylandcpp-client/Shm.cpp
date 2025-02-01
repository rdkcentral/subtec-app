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


#include "Shm.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "File.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class ShmListenerWrapper : public ListenerWrapper<Shm, ShmListener, wl_shm,
        wl_shm_listener>
{
public:
    ShmListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_shm_add_listener)
    {
        getListenerStruct().format = callbackFormat;
    }

private:
    static void callbackFormat(void* data,
                               struct wl_shm* /*wlShm*/,
                               uint32_t format)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->format(owner, format);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------

const wl_interface* Shm::getWlInterface()
{
    return &wl_shm_interface;
}

Shm::Shm(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Shm::~Shm()
{
    // noop
}

void Shm::setUserData(void* userData)
{
    return wl_shm_set_user_data(getNativeObject(), userData);
}

void* Shm::getUserData() const
{
    return wl_shm_get_user_data(getNativeObject());
}

bool Shm::setListener(ShmListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new ShmListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Shm1::Shm1(WaylandObjectType* const wlObject) :
        Shm(wlObject, wl_shm_destroy)
{
    // noop
}

ShmPoolPtr Shm1::createPool(FilePtr file)
{
    if (!file->isMapped())
    {
        return nullptr;
    }

    auto wlObject = wl_shm_create_pool(getNativeObject(),
            file->getNativeObject(), file->getSize());
    return ObjectFactory::create(wlObject, file);
}

} // namespace waylandcpp
