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


#include "ShmPool.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"
#include "File.hpp"

namespace waylandcpp
{

const wl_interface* ShmPool::getWlInterface()
{
    return &wl_shm_pool_interface;
}

ShmPool::ShmPool(WaylandObjectType* const wlObject,
                 FilePtr file,
                 DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction),
        m_file(file)
{
    // noop
}

ShmPool::~ShmPool()
{
    // noop
}

void ShmPool::setUserData(void* userData)
{
    return wl_shm_pool_set_user_data(getNativeObject(), userData);
}

void* ShmPool::getUserData() const
{
    return wl_shm_pool_get_user_data(getNativeObject());
}

std::size_t ShmPool::getSize() const
{
    return m_file->getSize();
}

//-------------------------

ShmPool1::ShmPool1(WaylandObjectType* const wlObject,
                   FilePtr file) :
        ShmPool(wlObject, file, wl_shm_pool_destroy)
{
    // noop
}

BufferPtr ShmPool1::createBuffer(const BufferParams& params)
{
    auto wlObject = wl_shm_pool_create_buffer(getNativeObject(),
            params.m_offset, params.m_width, params.m_height, params.m_stride,
            PixelFormatUtils::toNativeFormat(params.m_format));
    return ObjectFactory::create(wlObject, m_file, params);
}

} // namespace waylandcpp
