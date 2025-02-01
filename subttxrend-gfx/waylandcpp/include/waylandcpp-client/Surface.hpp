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


#ifndef WAYLANDCPP_SURFACE_HPP_
#define WAYLANDCPP_SURFACE_HPP_

#include "Types.hpp"

#include <string>
#include <vector>

namespace waylandcpp
{

class SurfaceListenerWrapper;

/**
 * Listener for surface frame events.
 */
class SurfaceFrameListener
{
public:
    /**
     * Constructor.
     */
    SurfaceFrameListener() = default;

    /**
     * Destructor.
     */
    virtual ~SurfaceFrameListener() = default;

    /**
     * Notifies that frame rendering was done.
     *
     * @param frameTime
     *      Wayland frame rendering time.
     */
    virtual void frameDone(uint32_t frameTime) = 0;
};

/**
 * Listener for surface events.
 */
class SurfaceListener
{
public:
    /**
     * Constructor.
     */
    SurfaceListener() = default;

    /**
     * Destructor.
     */
    virtual ~SurfaceListener() = default;

    /**
     * surface enters an output
     *
     * @param object
     *      Object for which event is called.
     * @param output
     *      Output entered by the surface
     */
    virtual void enter(SurfacePtr object,
                       struct wl_output *output) = 0;

    /**
     * surface leaves an output
     *
     * @param object
     *      Object for which event is called.
     * @param output
     *      Output left by the surface
     */
    virtual void leave(SurfacePtr object,
                       struct wl_output *output) = 0;
};

/**
 * Surface (common).
 */
class Surface : public Proxy<Surface, wl_surface>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Surface> Ptr;

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
    virtual ~Surface();

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
    bool setListener(SurfaceListener* listener);

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Surface(WaylandObjectType* const wlObject,
            DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<SurfaceListenerWrapper> m_listenerWrapper;
};

/**
 * Surface (version 1).
 */
class Surface1 : public Surface,
                 private SurfaceFrameListener
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Surface1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Surface1(WaylandObjectType* const wlObject);

    /**
     * Destructor.
     */
    virtual ~Surface1();

    /**
     * Adds frame event listener.
     *
     * @param listener
     *      Listener to add.
     */
    void addFrameListener(SurfaceFrameListener* listener);

    /**
     *
     */
    void damage(int32_t x,
                int32_t y,
                int32_t width,
                int32_t height);

    /**
     *
     */
    void commit();

    /**
     *
     */
    void attach(const BufferPtr& buffer,
                int32_t x,
                int32_t y);

    /**
     * Attaches the buffer at location 0,0
     *
     * @param buffer
     *      Buffer to attach.
     */
    void attach(const BufferPtr& buffer)
    {
        attach(buffer, 0, 0);
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
    Surface1(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction) :
            Surface(wlObject, destructorFunction),
            m_frameCallback(nullptr)
    {
        // noop
    }

private:
    /**
     * Method called by the frame done callback.
     *
     * @param data
     *      User defined data.
     * @param wl_callback
     *      Callback that called the method.
     * @param callback_data
     *      Callback data.
     */
    static void frameCallbackNotify(void *data,
                                    wl_callback *wl_callback,
                                    uint32_t callback_data);

    /**
     * Processes frame done notification.
     *
     * @param frameTime
     *      Frame time received from the callback.
     */
    virtual void frameDone(uint32_t frameTime) override;

    /**
     * (Re)starts frame callback.
     */
    void restartFrameCallback();

    /**
     * Destroys frame callback instance.
     */
    void destroyFrameCallback();

    /** Frame callback instance. */
    wl_callback* m_frameCallback;

    /** Frame listeners. */
    std::vector<SurfaceFrameListener*> m_frameListeners;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SURFACE_HPP_*/
