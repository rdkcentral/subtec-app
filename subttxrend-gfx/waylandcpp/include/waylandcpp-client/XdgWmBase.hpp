/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
 */

#ifndef WAYLANDCPP_XDGWMBASE_HPP_
#define WAYLANDCPP_XDGWMBASE_HPP_

#include "Types.hpp"
#include "XdgSurface.hpp"

namespace waylandcpp
{

class XdgWmBaseListenerWrapper;

/**
 * Listener for XdgWmBase events.
 */
class XdgWmBaseListener
{
public:
    /**
     * Constructor.
     */
    XdgWmBaseListener() = default;

    /**
     * Destructor.
     */
    virtual ~XdgWmBaseListener() = default;

    /**
     * ping client
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      Serial number of the ping
     */
    virtual void ping(XdgWmBasePtr object,
                      uint32_t serial) = 0;
};

/**
 * XdgWmBase (common).
 */
class XdgWmBase : public Proxy<XdgWmBase, xdg_wm_base>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgWmBase> Ptr;

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
    virtual ~XdgWmBase();

    /** @copydoc Proxy::getVersion */
    //virtual uint32_t getVersion() const override final;

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Sets object listener.
     *
     * @param listener
     *      Listener to set.
     *
     * @retval true
     *      Success.
     * @retval fSalse
     *      Failure (e.g. listener already set).
     */
    bool setListener(XdgWmBaseListener* listener);

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
    XdgWmBase(WaylandObjectType* const wlObject,
              DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<XdgWmBaseListenerWrapper> m_listenerWrapper;
};

/**
 * XdgWmBase (version 1).
 */
class XdgWmBase1 : public XdgWmBase
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgWmBase1> Ptr;

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
    XdgWmBase1(WaylandObjectType* const wlObject);

    /**
     * A client must respond to a ping event with a pong request or
     * the client may be deemed unresponsive.
     *
     * @param serial
     *      Serial number from the ping request.
     */
    void pong(uint32_t serial);

    /**
     * Creates XDG surface.
     *
     * @param surface
     *      Surface for which XDG surface is created.
     *
     * @return
     *      Created surface or null on error.
     */
    XdgSurfacePtr getXdgSurface(SurfacePtr surface);

    /**
     * Creates XDG surface.
     *
     * @param surface
     *      Surface for which XDG surface is created.
     *
     * @return
     *      Created XDG surface or null on error.
     *
     * @tparam XdgSurfaceInterface
     *      Requested XDG surface interface.
     */
    template <class XdgSurfaceInterface>
    typename XdgSurfaceInterface::Ptr getXdgSurface(SurfacePtr surface)
    {
        XdgSurfacePtr ptr = getXdgSurface(surface);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<XdgSurfaceInterface>();
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
    XdgWmBase1(WaylandObjectType* const wlObject,
               DestructorFunc const destructorFunction) :
               XdgWmBase(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_XDGWMBASE_HPP_*/
