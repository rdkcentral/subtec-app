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

#ifndef WAYLANDCPP_XDGSURFACE_HPP_
#define WAYLANDCPP_XDGSURFACE_HPP_

#include "Types.hpp"
#include "XdgToplevel.hpp"

namespace waylandcpp
{

class XdgSurfaceListenerWrapper;

/**
 * Listener for XDG surface events.
 */
class XdgSurfaceListener
{
public:
    /**
     * Constructor.
     */
    XdgSurfaceListener() = default;

    /**
     * Destructor.
     */
    virtual ~XdgSurfaceListener() = default;

    /**
     * Configure the XDG surface.
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      Serial number of the configure request.
     */
    virtual void configure(XdgSurfacePtr object,
                           uint32_t serial) = 0;
};

/**
 * XdgSurface (common).
 */
class XdgSurface : public Proxy<XdgSurface, xdg_surface>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgSurface> Ptr;

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
    virtual ~XdgSurface();

    /** @copydoc Proxy::getVersion */
  //  virtual uint32_t getVersion() const override final;

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
     * @retval false
     *      Failure (e.g. listener already set).
     */
    bool setListener(XdgSurfaceListener* listener);

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
    XdgSurface(WaylandObjectType* const wlObject,
               DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<XdgSurfaceListenerWrapper> m_listenerWrapper;
};

/**
 * XdgSurface (version 1).
 */
class XdgSurface1 : public XdgSurface
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgSurface1> Ptr;

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
    XdgSurface1(WaylandObjectType* const wlObject);

    /**
     * Get XDG toplevel.
     *
     * @return
     *      Toplevel object or null on error.
     */
    XdgToplevelPtr getToplevel();

    /**
     * Get XDG toplevel.
     *
     * @tparam XdgToplevelInterface
     *      Requested XDG toplevel interface.
     */
    template <class XdgToplevelInterface>
    typename XdgToplevelInterface::Ptr getToplevel()
    {
        XdgToplevelPtr ptr = getToplevel();

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<XdgToplevelInterface>();
    }

    /**
     * A client must respond to a configure event with an ack.
     *
     * @param serial
     *      Serial number from the configure request.
     */
    void ackConfigure(uint32_t serial);

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    XdgSurface1(WaylandObjectType* const wlObject,
                DestructorFunc const destructorFunction) :
                XdgSurface(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_XDGSURFACE_HPP_*/
