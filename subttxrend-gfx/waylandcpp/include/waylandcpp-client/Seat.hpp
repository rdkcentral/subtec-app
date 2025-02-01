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


#ifndef WAYLANDCPP_SEAT_HPP_
#define WAYLANDCPP_SEAT_HPP_

#include "Types.hpp"

#include <string>

#include "Keyboard.hpp"

namespace waylandcpp
{

class SeatListenerWrapper;

/**
 * Listener for seat events.
 */
class SeatListener
{
public:
    /** Capability flag - pointer. */
    static const uint32_t CAPABILITY_FLAG_POINTER;

    /** Capability flag - pointer. */
    static const uint32_t CAPABILITY_FLAG_KEYBOARD;

    /** Capability flag - pointer. */
    static const uint32_t CAPABILITY_FLAG_TOUCH;

    /**
     * Constructor.
     */
    SeatListener() = default;

    /**
     * Destructor.
     */
    virtual ~SeatListener() = default;

    /**
     * seat capabilities changed
     *
     * @param object
     *      Object for which event is called.
     * @param capabilities
     *      Capabilities of the seat
     */
    virtual void capabilities(SeatPtr object,
                              uint32_t capabilities) = 0;

    /**
     * unique identifier for this seat
     *
     * @param object
     *      Object for which event is called.
     * @param name
     *      seat identifier
     *
     * @since 2
     */
    virtual void name(SeatPtr object,
                      const char *name) = 0;

};

/**
 * Seat (common).
 */
class Seat : public Proxy<Seat, wl_seat>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Seat> Ptr;

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
    virtual ~Seat();

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
    bool setListener(SeatListener* listener);

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Seat(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<SeatListenerWrapper> m_listenerWrapper;
};

/**
 * Seat (version 1).
 */
class Seat1 : public Seat
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Seat1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Seat1(WaylandObjectType* const wlObject);

    /**
     * Return seat keyboard.
     *
     * @param capabilities
     *      Capabilities flags.
     *
     * @return
     *      Keyboard object or null on error.
     */
    KeyboardPtr getKeyboard(uint32_t capabilities);

    /**
     * Return seat keyboard.
     *
     * @param capabilities
     *      Capabilities flags.
     *
     * @return
     *      Keyboard object or null on error.
     *
     * @tparam ShmPoolInterface
     *      Requested pool interface.
     */
    template<class KeyboardInterface>
    typename KeyboardInterface::Ptr getKeyboard(uint32_t capabilities)
    {
        auto ptr = getKeyboard(capabilities);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<KeyboardInterface>();
    }

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Seat1(WaylandObjectType* const wlObject,
          DestructorFunc const destructorFunction) :
            Seat(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SEAT_HPP_*/
