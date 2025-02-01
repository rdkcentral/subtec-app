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


#include "Shell.hpp"

#include <cassert>

#include <wayland-client.h>

#include "Surface.hpp"
#include "ObjectFactory.hpp"

namespace waylandcpp
{

const wl_interface* Shell::getWlInterface()
{
    return &wl_shell_interface;
}

Shell::Shell(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction) :
        Proxy(wlObject, destructorFunction)
{
    // noop
}

Shell::~Shell()
{
    // noop
}

void Shell::setUserData(void* userData)
{
    return wl_shell_set_user_data(getNativeObject(), userData);
}

void* Shell::getUserData() const
{
    return wl_shell_get_user_data(getNativeObject());
}

//-------------------------

Shell1::Shell1(WaylandObjectType* const wlObject) :
        Shell(wlObject, wl_shell_destroy)
{
    // noop
}

ShellSurfacePtr Shell1::getShellSurface(SurfacePtr surface)
{
    if (!surface)
    {
        return nullptr;
    }

    auto wlShellSurface = wl_shell_get_shell_surface(getNativeObject(),
            surface->getNativeObject());
    return ObjectFactory::create(wlShellSurface);
}

} // namespace waylandcpp
