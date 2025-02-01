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


#ifndef WAYLANDCPP_COMPOSITOR_HPP_
#define WAYLANDCPP_COMPOSITOR_HPP_

#include "Types.hpp"

#include <string>

#include "Surface.hpp"

namespace waylandcpp
{

/**
 * Compositor (common).
 */
class Compositor : public Proxy<Compositor, wl_compositor>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Compositor> Ptr;

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
    virtual ~Compositor();

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Compositor(WaylandObjectType* const wlObject,
        DestructorFunc const destructorFunction);
};

/**
 * Compositor (version 1).
 */
class Compositor1 : public Compositor
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Compositor1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Compositor1(WaylandObjectType* const wlObject);

    /**
     * Creates surface.
     *
     * @return
     *      Created surface or null on error.
     */
    SurfacePtr createSurface();

    /**
     * Creates surface.
     *
     * @return
     *      Created surface or null on error.
     *
     * @tparam SurfaceInterface
     *      Requested surface interface.
     */
    template <class SurfaceInterface>
    typename SurfaceInterface::Ptr createSurface()
    {
        SurfacePtr ptr = createSurface();

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<SurfaceInterface>();
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
    Compositor1(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
            Compositor(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_COMPOSITOR_HPP_*/
