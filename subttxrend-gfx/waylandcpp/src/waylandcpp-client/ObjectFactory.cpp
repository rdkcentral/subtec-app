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


#include "ObjectFactory.hpp"

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include "Display.hpp"
#include "Registry.hpp"
#include "Shm.hpp"
#include "Compositor.hpp"
#include "Shell.hpp"
#include "Surface.hpp"
#include "Buffer.hpp"
#include "Output.hpp"
#include "Seat.hpp"
#include "Keyboard.hpp"
#if defined(WESTEROS)
#include "SimpleShell.hpp"
#endif
#include "XdgWmBase.hpp"
#include "XdgSurface.hpp"
#include "XdgToplevel.hpp"

namespace waylandcpp
{

namespace
{

template<class BaseType>
class VersionAwareCreator
{
public:
    typedef typename BaseType::WaylandObjectType WaylandObjectType;
    typedef typename BaseType::Ptr PointerType;

    VersionAwareCreator(WaylandObjectType* wlObject) :
            m_wlObject(wlObject)
    {
        // noop
    }

    template<class ObjectType, typename ... Values>
    void processType(Values ... values)
    {
        // no object, cannot do anything
        if (!m_wlObject)
        {
            return;
        }

        // already done, nothing more to do
        if (m_pointer)
        {
            return;
        }

        m_pointer = std::make_shared<ObjectType>(m_wlObject, values...);
    }

    PointerType getPointer()
    {
        return std::move(m_pointer);
    }

private:
    template<class VersionCheckFunc>
    uint32_t getVersion(VersionCheckFunc versionChecker)
    {
        if (!m_wlObject)
        {
            return 0;
        }

        return (*versionChecker)(m_wlObject);
    }

private:
    /** Object for which creator was made. */
    WaylandObjectType* const m_wlObject;

    /** Created object pointer. */
    PointerType m_pointer;
};

} // namespace <anonymous>

DisplayPtr ObjectFactory::create(wl_display* wlObject)
{
    VersionAwareCreator<Display> creator(wlObject);

    creator.processType<Display1>();

    return creator.getPointer();
}

RegistryPtr ObjectFactory::create(wl_registry* wlObject)
{
    VersionAwareCreator<Registry> creator(wlObject);

    creator.processType<Registry1>();

    return creator.getPointer();
}

ShmPtr ObjectFactory::create(wl_shm* wlObject)
{
    VersionAwareCreator<Shm> creator(wlObject);

    creator.processType<Shm1>();

    return creator.getPointer();
}

CompositorPtr ObjectFactory::create(wl_compositor* wlObject)
{
    VersionAwareCreator<Compositor> creator(wlObject);

    creator.processType<Compositor1>();

    return creator.getPointer();
}

ShellPtr ObjectFactory::create(wl_shell* wlObject)
{
    VersionAwareCreator<Shell> creator(wlObject);

    creator.processType<Shell1>();

    return creator.getPointer();
}

#if defined(WESTEROS)
SimpleShellPtr ObjectFactory::create(wl_simple_shell* wlObject)
{
    VersionAwareCreator<SimpleShell> creator(wlObject);

    creator.processType<SimpleShell1>();

    return creator.getPointer();
}
#endif

SurfacePtr ObjectFactory::create(wl_surface* wlObject)
{
    VersionAwareCreator<Surface> creator(wlObject);

    creator.processType<Surface1>();

    return creator.getPointer();
}

ShellSurfacePtr ObjectFactory::create(wl_shell_surface* wlObject)
{
    VersionAwareCreator<ShellSurface> creator(wlObject);

    creator.processType<ShellSurface1>();

    return creator.getPointer();
}

BufferPtr ObjectFactory::create(wl_buffer* wlObject,
                                FilePtr file,
                                const BufferParams& params)
{
    VersionAwareCreator<Buffer> creator(wlObject);

    creator.processType<Buffer1>(file, params);

    return creator.getPointer();
}

ShmPoolPtr ObjectFactory::create(wl_shm_pool* wlObject,
                                 FilePtr file)
{
    VersionAwareCreator<ShmPool> creator(wlObject);

    creator.processType<ShmPool1>(file);

    return creator.getPointer();
}

OutputPtr ObjectFactory::create(wl_output* wlObject)
{
    VersionAwareCreator<Output> creator(wlObject);

    creator.processType<Output1>();

    return creator.getPointer();
}

SeatPtr ObjectFactory::create(wl_seat* wlObject)
{
    VersionAwareCreator<Seat> creator(wlObject);

    creator.processType<Seat1>();

    return creator.getPointer();
}

KeyboardPtr ObjectFactory::create(wl_keyboard* wlObject)
{
    VersionAwareCreator<Keyboard> creator(wlObject);

    creator.processType<Keyboard1>();

    return creator.getPointer();
}

XdgWmBasePtr ObjectFactory::create(xdg_wm_base* wlObject)
{
    VersionAwareCreator<XdgWmBase> creator(wlObject);

    creator.processType<XdgWmBase1>();

    return creator.getPointer();
}

XdgSurfacePtr ObjectFactory::create(xdg_surface* wlObject)
{
    VersionAwareCreator<XdgSurface> creator(wlObject);

    creator.processType<XdgSurface1>();

    return creator.getPointer();
}

XdgToplevelPtr ObjectFactory::create(xdg_toplevel* wlObject)
{
    VersionAwareCreator<XdgToplevel> creator(wlObject);

    creator.processType<XdgToplevel1>();

    return creator.getPointer();
}

} // namespace waylandcpp
