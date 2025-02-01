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


#include "Buffer.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "ListenerWrapper.hpp"
#include "File.hpp"

namespace waylandcpp
{

/** @cond INTERNALS */
class BufferListenerWrapper : public ListenerWrapper<Buffer, BufferListener,
        wl_buffer, wl_buffer_listener>
{
public:
    BufferListenerWrapper(WeakObjectPtr owner) :
            ListenerWrapper(owner, wl_buffer_add_listener)
    {
        getListenerStruct().release = callbackRelease;
    }

private:
    static void callbackRelease(void *data,
                                struct wl_buffer* /*wl_buffer*/)
    {
        auto owner = getOwner(data);
        if (owner)
        {
            auto listener = getListener(data);
            listener->release(owner);
        }
    }
};
/** @endcond INTERNALS */

//-------------------------

const wl_interface* Buffer::getWlInterface()
{
    return &wl_buffer_interface;
}

Buffer::Buffer(WaylandObjectType* const wlObject,
               const FilePtr& file,
               const BufferParams& params,
               DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction),
        m_file(file),
        m_params(params)
{
// noop
}

Buffer::~Buffer()
{
// noop
}

const BufferParams& Buffer::getParams() const
{
    return m_params;
}

void Buffer::setUserData(void* userData)
{
    return wl_buffer_set_user_data(getNativeObject(), userData);
}

void* Buffer::getUserData() const
{
    return wl_buffer_get_user_data(getNativeObject());
}

bool Buffer::setListener(BufferListener* listener)
{
    if (!m_listenerWrapper)
    {
        m_listenerWrapper.reset(new BufferListenerWrapper(makeShared()));
    }
    return m_listenerWrapper->setListener(getNativeObject(), listener);
}

void* Buffer::getDataPtr()
{
    uint8_t* ptr = m_file->getMapping();
    ptr += m_params.m_offset;
    return ptr;
}

//-------------------------

Buffer1::Buffer1(WaylandObjectType* const wlObject,
                 const FilePtr& file,
                 const BufferParams& params) :
        Buffer(wlObject, file, params, wl_buffer_destroy)
{
// noop
}

} // namespace waylandcpp
