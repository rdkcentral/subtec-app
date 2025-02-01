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


#ifndef WAYLANDCPP_DISPLAY_HPP_
#define WAYLANDCPP_DISPLAY_HPP_

#include "Types.hpp"
#include "Registry.hpp"

namespace waylandcpp
{

/**
 * Display (common).
 */
class Display : public Proxy<Display, wl_display>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Display> Ptr;

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Returns display file descriptor.
     *
     * @return
     *      File descriptor.
     */
    int getFd();

    /**
     * Processes incoming events.
     *
     * @return
     *      The number of dispatched events on success or -1 on failure
     */
    int dispatch();

    /**
     * Blocks until all pending request are processed by the server.
     *
     * @return
     *      The number of dispatched events on success or -1 on failure.
     */
    int roundtrip();

    /**
     * Dispatches all pending events.
     *
     * @return
     *      The number of dispatched events or -1 on failure.
     */
    int dispatchPending();

    /**
     * Send all buffered requests on the display to the server.
     *
     * @return
     *      On success, the number of bytes sent to the server is returned.
     *      On failure, this function returns -1 and errno is set appropriately.
     */
    int flush();

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Display(WaylandObjectType* const wlObject,
            DestructorFunc const destructorFunction) :
            Proxy(wlObject, destructorFunction)
    {
        // noop
    }
};

/**
 * Display (version 1).
 */
class Display1 : public Display
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Display1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 0;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Display1(WaylandObjectType* const wlObject);

    /**
     * Creates registry.
     *
     * @return
     *      Created registry or null on error.
     */
    RegistryPtr getRegistry();

    /**
     * Creates registry.
     *
     * @return
     *      Created registry or null on error.
     *
     * @tparam RegistryInterface
     *      Requested registry interface.
     */
    template <class RegistryInterface>
    typename RegistryInterface::Ptr getRegistry()
    {
        RegistryPtr ptr = getRegistry();

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<RegistryInterface>();
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
    Display1(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction) :
            Display(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_DISPLAY_HPP_*/
