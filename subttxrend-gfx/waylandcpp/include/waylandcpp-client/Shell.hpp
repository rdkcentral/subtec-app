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


#ifndef WAYLANDCPP_SHELL_HPP_
#define WAYLANDCPP_SHELL_HPP_

#include "Types.hpp"

#include <string>

#include "ShellSurface.hpp"

namespace waylandcpp
{

/**
 * Shell (common).
 */
class Shell : public Proxy<Shell, wl_shell>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Shell> Ptr;

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
    virtual ~Shell();

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

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
    Shell(WaylandObjectType* const wlObject,
        DestructorFunc const destructorFunction);
};

/**
 * Shell (version 1).
 */
class Shell1 : public Shell
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Shell1> Ptr;

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
    Shell1(WaylandObjectType* const wlObject);

    /**
     * Creates shell surface.
     *
     * @param surface
     *      Surface for which shell surface is created.
     *
     * @return
     *      Created shell surface or null on error.
     */
    ShellSurfacePtr getShellSurface(SurfacePtr surface);

    /**
     * Creates shell surface.
     *
     * @param surface
     *      Surface for which shell surface is created.
     *
     * @return
     *      Created shell surface or null on error.
     *
     * @tparam ShellSurfaceInterface
     *      Requested shell surface interface.
     */
    template <class ShellSurfaceInterface>
    typename ShellSurfaceInterface::Ptr getShellSurface(SurfacePtr surface)
    {
        ShellSurfacePtr ptr = getShellSurface(surface);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<ShellSurfaceInterface>();
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
    Shell1(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
            Shell(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SHELL_HPP_*/
