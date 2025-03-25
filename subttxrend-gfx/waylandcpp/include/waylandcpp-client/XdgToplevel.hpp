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

#ifndef WAYLANDCPP_XTGTOPLEVEL_HPP_
#define WAYLANDCPP_XTGTOPLEVEL_HPP_

#include "Types.hpp"
#include <vector>

namespace waylandcpp
{

class XdgToplevelListenerWrapper;

/** Configure state. */
enum class XdgToplevelState
{
    UNDEF = 0,
    MAXIMIZED,
    FULLSCREEN,
    RESIZING,
    ACTIVATED,
    TILED_LEFT,
    TILED_RIGHT,
    TILED_TOP,
    TILED_BOTTOM
};

/**
 * Listener for XDG toplevel events.
 */
class XdgToplevelListener
{
public:
    /**
     * Constructor.
     */
    XdgToplevelListener() = default;

    /**
     * Destructor.
     */
    virtual ~XdgToplevelListener() = default;

    /**
     * Configure the XDG toplevel.
     *
     * If the width or height parameter is zero, then the client can decide
     * what size to use.
     *
     * @param object
     *      Object for which event is called.
     * @param width
     *      Requested display width.
     * @param height
     *      Requested display height.
     * @param states
     *      Requested state flags.
     */
    virtual void configure(XdgToplevelPtr object,
                           int32_t width,
                           int32_t height,
                           std::vector<XdgToplevelState> &states) = 0;
};

/**
 * XdgTopLevel (common).
 */
class XdgToplevel : public Proxy<XdgToplevel, xdg_toplevel>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgToplevel> Ptr;

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
    virtual ~XdgToplevel();

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
    bool setListener(XdgToplevelListener* listener);

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
    XdgToplevel(WaylandObjectType* const wlObject,
        DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<XdgToplevelListenerWrapper> m_listenerWrapper;
};

/**
 * XdgTopLevel (version 1).
 */
class XdgToplevel1 : public XdgToplevel
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<XdgToplevel1> Ptr;

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
    XdgToplevel1(WaylandObjectType* const wlObject);

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    XdgToplevel1(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
            XdgToplevel(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_XTGTOPLEVEL_HPP_*/
