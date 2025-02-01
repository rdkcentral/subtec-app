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


#ifndef WAYLANDCPP_SHELLSURFACE_HPP_
#define WAYLANDCPP_SHELLSURFACE_HPP_

#include "Types.hpp"

#include <string>

namespace waylandcpp
{

class ShellSurfaceListenerWrapper;

/**
 * Listener for shell surface events.
 */
class ShellSurfaceListener
{
public:
    /**
     * Constructor.
     */
    ShellSurfaceListener() = default;

    /**
     * Destructor.
     */
    virtual ~ShellSurfaceListener() = default;

    /**
     * ping client
     *
     * @param object
     *      Object for which event is called.
     * @param serial
     *      Serial number of the ping
     */
    virtual void ping(ShellSurfacePtr object,
                      uint32_t serial) = 0;

    /**
     * suggest resize
     *
     * @param object
     *      Object for which event is called.
     * @param edges
     *      How the surface was resized
     * @param width
     *      New width of the surface
     * @param height
     *      New height of the surface
     */
    virtual void configure(ShellSurfacePtr object,
                           uint32_t edges,
                           int32_t width,
                           int32_t height) = 0;

    /**
     * popup interaction is done
     *
     * @param object
     *      Object for which event is called.
     */
    virtual void popupDone(ShellSurfacePtr object) = 0;

};

/**
 * Shell Surface (common).
 */
class ShellSurface : public Proxy<ShellSurface, wl_shell_surface>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<ShellSurface> Ptr;

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
    virtual ~ShellSurface();

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
    bool setListener(ShellSurfaceListener* listener);

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    ShellSurface(WaylandObjectType* const wlObject,
                 DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<ShellSurfaceListenerWrapper> m_listenerWrapper;
};

/**
 * Shell Surface (version 1).
 */
class ShellSurface1 : public ShellSurface
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<ShellSurface1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /** Fullscreen method. */
    enum class FullScreenMethod
    {
        DEFAULT,
        SCALE,
        DRIVER,
        FILL
    };

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    ShellSurface1(WaylandObjectType* const wlObject);

    /**
     * A client must respond to a ping event with a pong request or
     * the client may be deemed unresponsive.
     *
     * @param serial
     *      Serial number from the ping request.
     */
    void pong(uint32_t serial);

    /**
     * Sets surface as top-level.
     */
    void setTopLevel();

    /**
     * Sets surface as full screen.
     *
     * @param method
     *      Full screen rendering method.
     * @param framerate
     *      Requested frame rate.
     * @param output
     *      Output to use (null for default).
     */
    void setFullScreen(FullScreenMethod method,
                       uint32_t framerate,
                       OutputPtr output);

    /**
     * Sets surface as maximized.
     *
     * @param output
     *      Output to use (null for default).
     */
    void setMaximized(OutputPtr output);

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
    ShellSurface1(WaylandObjectType* const wlObject,
                  DestructorFunc const destructorFunction) :
            ShellSurface(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SHELLSURFACE_HPP_*/
