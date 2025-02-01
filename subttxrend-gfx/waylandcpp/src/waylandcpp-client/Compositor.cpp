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


#include "Compositor.hpp"

#include <cassert>

#include <wayland-client.h>

#include "ObjectFactory.hpp"

namespace waylandcpp
{

const wl_interface* Compositor::getWlInterface()
{
    return &wl_compositor_interface;
}

Compositor::Compositor(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Compositor::~Compositor()
{
    // noop
}

void Compositor::setUserData(void* userData)
{
    return wl_compositor_set_user_data(getNativeObject(), userData);
}

void* Compositor::getUserData() const
{
    return wl_compositor_get_user_data(getNativeObject());
}

//-------------------------

Compositor1::Compositor1(WaylandObjectType* const wlObject) :
        Compositor(wlObject, wl_compositor_destroy)
{
    // noop
}

SurfacePtr Compositor1::createSurface()
{
    auto wlRegistry = wl_compositor_create_surface(getNativeObject());
    return ObjectFactory::create(wlRegistry);
}

} // namespace waylandcpp
