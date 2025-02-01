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


#include "Surface.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "Buffer.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class SurfaceListenerWrapper : public ListenerWrapper<Surface, SurfaceListener,
        wl_surface, wl_surface_listener>
{
public:
    SurfaceListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_surface_add_listener)
    {
        getListenerStruct().enter = callbackEnter;
        getListenerStruct().leave = callbackLeave;
    }

private:
    static void callbackEnter(void *data,
                              struct wl_surface */*wl_surface*/,
                              struct wl_output *output)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->enter(owner, output);
        }
    }

    static void callbackLeave(void *data,
                              struct wl_surface */*wl_surface*/,
                              struct wl_output *output)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->leave(owner, output);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------

const wl_interface* Surface::getWlInterface()
{
    return &wl_surface_interface;
}

Surface::Surface(WaylandObjectType* const wlObject,
                 DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Surface::~Surface()
{
    // noop
}

void Surface::setUserData(void* userData)
{
    return wl_surface_set_user_data(getNativeObject(), userData);
}

void* Surface::getUserData() const
{
    return wl_surface_get_user_data(getNativeObject());
}

bool Surface::setListener(SurfaceListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new SurfaceListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Surface1::Surface1(WaylandObjectType* const wlObject) :
        Surface(wlObject, wl_surface_destroy),
        m_frameCallback(nullptr)
{
    // noop
}

Surface1::~Surface1()
{
    destroyFrameCallback();
}

void Surface1::addFrameListener(SurfaceFrameListener* listener)
{
    m_frameListeners.push_back(listener);

    if (!m_frameCallback)
    {
        restartFrameCallback();
    }
}

void Surface1::damage(int32_t x,
                      int32_t y,
                      int32_t width,
                      int32_t height)
{
    wl_surface_damage(getNativeObject(), x, y, width, height);
}

void Surface1::commit()
{
    wl_surface_commit(getNativeObject());
}

void Surface1::attach(const BufferPtr& buffer,
                      int32_t x,
                      int32_t y)
{
    if (buffer)
    {
        wl_surface_attach(getNativeObject(), buffer->getNativeObject(), x, y);
    }
    else
    {
        wl_surface_attach(getNativeObject(), nullptr, x, y);
    }
}

void Surface1::frameDone(uint32_t frameTime)
{
    destroyFrameCallback();
    restartFrameCallback();

    // TODO: a copy-on-write listeners collection should be used but
    //       the implementation was postponed at the moment
    auto size = m_frameListeners.size();
    for (std::size_t i = 0; (i < size) && (i < m_frameListeners.size()); ++i)
    {
        m_frameListeners[i]->frameDone(frameTime);
    }
}

void Surface1::destroyFrameCallback()
{
    if (m_frameCallback)
    {
        wl_callback_destroy(m_frameCallback);
    }
    m_frameCallback = nullptr;
}

void Surface1::restartFrameCallback()
{
    static wl_callback_listener frameCallbackListener =
    { Surface1::frameCallbackNotify };

    m_frameCallback = wl_surface_frame(getNativeObject());
    wl_callback_add_listener(m_frameCallback, &frameCallbackListener, this);
}

void Surface1::frameCallbackNotify(void *data,
                                   wl_callback */*wl_callback*/,
                                   uint32_t callbackTime)
{
    Surface1* surface = reinterpret_cast<Surface1*>(data);

    surface->frameDone(callbackTime);
}

} // namespace waylandcpp
