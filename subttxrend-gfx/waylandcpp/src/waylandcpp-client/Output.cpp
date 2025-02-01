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


#include "Output.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "File.hpp"

namespace waylandcpp
{

const uint32_t OutputListener::OUTPUT_MODE_FLAG_CURRENT = WL_OUTPUT_MODE_CURRENT;
const uint32_t OutputListener::OUTPUT_MODE_FLAG_PREFERRED =
        WL_OUTPUT_MODE_PREFERRED;

/** @cond INTERNALS */
class OutputListenerWrapper : public ListenerWrapper<Output, OutputListener,
        wl_output, wl_output_listener>
{
public:
    OutputListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_output_add_listener)
    {
        getListenerStruct().geometry = callbackGeometry;
        getListenerStruct().mode = callbackMode;
        getListenerStruct().done = callbackDone;
        getListenerStruct().scale = callbackScale;
    }

private:
    static void callbackGeometry(void *data,
                                 struct wl_output */*wl_output*/,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subpixel,
                                 const char *make,
                                 const char *model,
                                 int32_t transform)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->geometry(owner, x, y, physicalWidth, physicalHeight,
                    subpixel, make, model, transform);
        }
    }

    static void callbackMode(void *data,
                             struct wl_output */*wl_output*/,
                             uint32_t flags,
                             int32_t width,
                             int32_t height,
                             int32_t refresh)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->mode(owner, flags, width, height, refresh);
        }
    }

    static void callbackDone(void *data,
                             struct wl_output */*wl_output*/)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->outputEventsDone(owner);
        }
    }

    static void callbackScale(void *data,
                              struct wl_output */*wl_output*/,
                              int32_t factor)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->scale(owner, factor);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------
const wl_interface* Output::getWlInterface()
{
    return &wl_output_interface;
}

Output::Output(WaylandObjectType* const wlObject,
               DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Output::~Output()
{
    // noop
}

void Output::setUserData(void* userData)
{
    return wl_output_set_user_data(getNativeObject(), userData);
}

void* Output::getUserData() const
{
    return wl_output_get_user_data(getNativeObject());
}

bool Output::setListener(OutputListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new OutputListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

//-------------------------

Output1::Output1(WaylandObjectType* const wlObject) :
        Output(wlObject, wl_output_destroy)
{
    // noop
}

} // namespace waylandcpp
