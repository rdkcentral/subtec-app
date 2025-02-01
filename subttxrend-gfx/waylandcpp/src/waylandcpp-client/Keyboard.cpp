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


#include "Keyboard.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "File.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class KeyboardListenerWrapper : public ListenerWrapper<Keyboard,
        KeyboardListener, wl_keyboard, wl_keyboard_listener>
{
public:
    KeyboardListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_keyboard_add_listener)
    {
        getListenerStruct().keymap = callbackKeymap;
        getListenerStruct().enter = callbackEnter;
        getListenerStruct().leave = callbackLeave;
        getListenerStruct().key = callbackKey;
        getListenerStruct().modifiers = callbackModifiers;
        getListenerStruct().repeat_info = callbackRepeatInfo;
    }

private:
    static void callbackKeymap(void *data,
                               struct wl_keyboard* /*wl_keyboard*/,
                               uint32_t format,
                               int32_t fd,
                               uint32_t size)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->keymap(owner, format, fd, size);
        }
    }

    static void callbackEnter(void *data,
                              struct wl_keyboard* /*wl_keyboard*/,
                              uint32_t serial,
                              struct wl_surface *surface,
                              struct wl_array *keys)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->enter(owner, serial, surface, keys);
        }
    }

    static void callbackLeave(void *data,
                              struct wl_keyboard* /*wl_keyboard*/,
                              uint32_t serial,
                              struct wl_surface *surface)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->leave(owner, serial, surface);
        }
    }

    static void callbackKey(void *data,
                            struct wl_keyboard* /*wl_keyboard*/,
                            uint32_t serial,
                            uint32_t time,
                            uint32_t key,
                            uint32_t state)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->key(owner, serial, time, key, state);
        }
    }

    static void callbackModifiers(void *data,
                                  struct wl_keyboard* /*wl_keyboard*/,
                                  uint32_t serial,
                                  uint32_t mods_depressed,
                                  uint32_t mods_latched,
                                  uint32_t mods_locked,
                                  uint32_t group)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->modifiers(owner, serial, mods_depressed, mods_latched,
                    mods_locked, group);
        }
    }

    static void callbackRepeatInfo(void *data,
                                   struct wl_keyboard* /*wl_keyboard*/,
                                   int32_t rate,
                                   int32_t delay)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->repeatInfo(owner, rate, delay);
        }
    }

};
/** @endcond INTERNALS */

//-------------------------
const wl_interface* Keyboard::getWlInterface()
{
    return &wl_keyboard_interface;
}

Keyboard::Keyboard(WaylandObjectType* const wlObject,
                   DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Keyboard::~Keyboard()
{
    // noop
}

void Keyboard::setUserData(void* userData)
{
    return wl_keyboard_set_user_data(getNativeObject(), userData);
}

void* Keyboard::getUserData() const
{
    return wl_keyboard_get_user_data(getNativeObject());
}

bool Keyboard::setListener(KeyboardListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new KeyboardListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Keyboard1::Keyboard1(WaylandObjectType* const wlObject) :
        Keyboard(wlObject, wl_keyboard_destroy)
{
    // noop
}

} // namespace waylandcpp
