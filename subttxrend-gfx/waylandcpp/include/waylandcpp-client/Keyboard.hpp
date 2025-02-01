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


#ifndef WAYLANDCPP_KEYBOARD_HPP_
#define WAYLANDCPP_KEYBOARD_HPP_

#include "Types.hpp"

#include <string>

namespace waylandcpp
{

class KeyboardListenerWrapper;

/**
 * Listener for keyboard events.
 */
class KeyboardListener
{
public:
    /**
     * Constructor.
     */
    KeyboardListener() = default;

    /**
     * Destructor.
     */
    virtual ~KeyboardListener() = default;

    /**
     * keyboard mapping
     *
     * This event provides a file descriptor to the client which can
     * be memory-mapped to provide a keyboard mapping description.
     *
     * @param object
     *      Object for which event is called.
     * @param format
     *      keymap format
     * @param fd
     *      keymap file descriptor
     * @param size
     *      keymap size, in bytes
     */
    virtual void keymap(KeyboardPtr object,
                        uint32_t format,
                        int32_t fd,
                        uint32_t size) = 0;

    /**
     * enter event
     *
     * Notification that this seat's keyboard focus is on a certain
     * surface.
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      serial number of the enter event
     * @param surface
     *      surface gaining keyboard focus
     * @param keys
     *      the currently pressed keys
     */
    virtual void enter(KeyboardPtr object,
                       uint32_t serial,
                       struct wl_surface *surface,
                       struct wl_array *keys) = 0;

    /**
     * leave event
     *
     * Notification that this seat's keyboard focus is no longer on a
     * certain surface.
     *
     * The leave notification is sent before the enter notification for
     * the new focus.
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      serial number of the leave event
     * @param surface
     *      surface that lost keyboard focus
     */
    virtual void leave(KeyboardPtr object,
                       uint32_t serial,
                       struct wl_surface *surface) = 0;

    /**
     * key event
     *
     * A key was pressed or released. The time argument is a
     * timestamp with millisecond granularity, with an undefined base.
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      serial number of the key event
     * @param time
     *      timestamp with millisecond granularity
     * @param key
     *      key that produced the event
     * @param state
     *      physical state of the key
     */
    virtual void key(KeyboardPtr object,
                     uint32_t serial,
                     uint32_t time,
                     uint32_t key,
                     uint32_t state) = 0;

    /**
     * modifier and group state
     *
     * Notifies clients that the modifier and/or group state has
     * changed, and it should update its local state.
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      serial number of the modifiers event
     * @param mods_depressed
     *      depressed modifiers
     * @param mods_latched
     *      latched modifiers
     * @param mods_locked
     *      locked modifiers
     * @param group
     *      keyboard layout
     */
    virtual void modifiers(KeyboardPtr object,
                           uint32_t serial,
                           uint32_t mods_depressed,
                           uint32_t mods_latched,
                           uint32_t mods_locked,
                           uint32_t group) = 0;

    /**
     * repeat rate and delay
     *
     * @param object
     *      Object for which event is called.
     * @param rate
     *      the rate of repeating keys in characters per second
     * @param delay
     *      delay in milliseconds since key down until repeating starts
     *
     * @since 4
     */
    virtual void repeatInfo(KeyboardPtr object,
                            int32_t rate,
                            int32_t delay) = 0;

};

/**
 * Keyboard (common).
 */
class Keyboard : public Proxy<Keyboard, wl_keyboard>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Keyboard> Ptr;

    /**
     * Returns object wayland interface.
     *
     * @return
     *      Pointer to wayland interface.
     */
    static const wl_interface* getWlInterface();

    /**
     * Destructor.
     */
    virtual ~Keyboard();

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Sets object listner.
     *
     * @param listener
     *      Listner to set.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure (e.g. listener already set).
     */
    bool setListener(KeyboardListener* listener);

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Keyboard(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<KeyboardListenerWrapper> m_listenerWrapper;
};

/**
 * Keyboard (version 1).
 */
class Keyboard1 : public Keyboard
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Keyboard1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * Uses default destructor function.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Keyboard1(WaylandObjectType* const wlObject);

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Keyboard1(WaylandObjectType* const wlObject,
              DestructorFunc const destructorFunction) :
            Keyboard(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_KEYBOARD_HPP_*/
