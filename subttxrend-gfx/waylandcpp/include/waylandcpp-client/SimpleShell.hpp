/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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
******************************************************************************/

#ifndef WAYLANDCPP_SIMPLE_SHELL_HPP_
#define WAYLANDCPP_SIMPLE_SHELL_HPP_

#include "Types.hpp"

#include <string>

#include "simpleshell-client-protocol.h"

namespace waylandcpp
{

class SimpleShellListenerWrapper;

/**
 * Listener for surface events.
 */
class SimpleShellListener
{
public:
    /**
     * Constructor.
     */
    SimpleShellListener() = default;

    /**
     * Destructor.
     */
    virtual ~SimpleShellListener() = default;

    virtual void surfaceId(SimpleShellPtr object, struct wl_surface *surface, uint32_t surfaceId) = 0;

    virtual void surfaceCreated(SimpleShellPtr object, uint32_t surfaceId, const char *name) = 0;

    virtual void surfaceDestroyed(SimpleShellPtr object, uint32_t surfaceId, const char *name) = 0;

    virtual void surfaceStatus(SimpleShellPtr object,
                               uint32_t surfaceId,
                               const char *name,
                               uint32_t visible,
                               int32_t x,
                               int32_t y,
                               int32_t width,
                               int32_t height,
                               wl_fixed_t opacity,
                               wl_fixed_t zorder) = 0;

    virtual void surfacesDone(SimpleShellPtr object) = 0;
};

/**
 * Shell (common).
 */
class SimpleShell : public Proxy<SimpleShell, wl_simple_shell>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<SimpleShell> Ptr;

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
    virtual ~SimpleShell();

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
    bool setListener(SimpleShellListener* listener);

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
    SimpleShell(WaylandObjectType* const wlObject,
        DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<SimpleShellListenerWrapper> m_listenerWrapper;
};

/**
 * SimpleShell (version 1).
 */
class SimpleShell1 : public SimpleShell
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<SimpleShell1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    SimpleShell1(WaylandObjectType* const wlObject);

    /**
     * Destructor.
     */
    virtual ~SimpleShell1();

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    SimpleShell1(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
            SimpleShell(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SIMPLE_SHELL_HPP_*/
