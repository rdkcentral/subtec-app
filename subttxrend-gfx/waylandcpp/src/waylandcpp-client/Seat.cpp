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


#include "Seat.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "File.hpp"

namespace waylandcpp
{

const uint32_t SeatListener::CAPABILITY_FLAG_POINTER =
        WL_SEAT_CAPABILITY_POINTER;
const uint32_t SeatListener::CAPABILITY_FLAG_KEYBOARD =
        WL_SEAT_CAPABILITY_KEYBOARD;
const uint32_t SeatListener::CAPABILITY_FLAG_TOUCH = WL_SEAT_CAPABILITY_TOUCH;

/** @cond INTERNALS */
class SeatListenerWrapper : public ListenerWrapper<Seat, SeatListener, wl_seat,
        wl_seat_listener>
{
public:
    SeatListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_seat_add_listener)
    {
        getListenerStruct().capabilities = callbackCapabilities;
        getListenerStruct().name = callbackName;
    }

private:
    static void callbackCapabilities(void *data,
                                     struct wl_seat */*wl_seat*/,
                                     uint32_t capabilities)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->capabilities(owner, capabilities);
        }
    }

    static void callbackName(void *data,
                             struct wl_seat */*wl_seat*/,
                             const char *name)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->name(owner, name);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------
const wl_interface* Seat::getWlInterface()
{
    return &wl_seat_interface;
}

Seat::Seat(WaylandObjectType* const wlObject,
           DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Seat::~Seat()
{
    // noop
}

void Seat::setUserData(void* userData)
{
    return wl_seat_set_user_data(getNativeObject(), userData);
}

void* Seat::getUserData() const
{
    return wl_seat_get_user_data(getNativeObject());
}

bool Seat::setListener(SeatListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new SeatListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Seat1::Seat1(WaylandObjectType* const wlObject) :
        Seat(wlObject, wl_seat_destroy)
{
    // noop
}

KeyboardPtr Seat1::getKeyboard(uint32_t capabilities)
{
    // if ((capabilities & SeatListener::CAPABILITY_FLAG_KEYBOARD) == 0)
    // {
        return nullptr;
    // }

    auto wlKeyboard = wl_seat_get_keyboard(getNativeObject());
    return ObjectFactory::create(wlKeyboard);
}

} // namespace waylandcpp
