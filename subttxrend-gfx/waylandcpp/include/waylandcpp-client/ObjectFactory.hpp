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


#ifndef WAYLANDCPP_OBJECTFACTORY_HPP_
#define WAYLANDCPP_OBJECTFACTORY_HPP_

#include "Types.hpp"

namespace waylandcpp
{

/**
 * Factory for wrapped wayland objects.
 */
class ObjectFactory
{
    ObjectFactory() = delete;
    ~ObjectFactory() = delete;

public:
    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static DisplayPtr create(wl_display* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static RegistryPtr create(wl_registry* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static ShmPtr create(wl_shm* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static CompositorPtr create(wl_compositor* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static ShellPtr create(wl_shell* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static SurfacePtr create(wl_surface* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static ShellSurfacePtr create(wl_shell_surface* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     * @param file
     *      File used for buffering.
     * @param params
     *      Buffer parameters.
     *
     * @return
     *      Created object or null on error.
     */
    static BufferPtr create(wl_buffer* wlObject,
                            FilePtr file,
                            const BufferParams& params);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     * @param file
     *      File used for buffering.
     *
     * @return
     *      Created object or null on error.
     */
    static ShmPoolPtr create(wl_shm_pool* wlObject,
                             FilePtr file);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static OutputPtr create(wl_output* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static SeatPtr create(wl_seat* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static KeyboardPtr create(wl_keyboard* wlObject);

#if defined(WESTEROS)
    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland native object.
     *
     * @return
     *      Created object or null on error.
     */
    static SimpleShellPtr create(wl_simple_shell* wlObject);
#endif

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland XDG wm base native object.
     *
     * @return
     *      Created object or null on error.
     */
    static XdgWmBasePtr create(xdg_wm_base* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland XDG surface native object.
     *
     * @return
     *      Created object or null on error.
     */
    static XdgSurfacePtr create(xdg_surface* wlObject);

    /**
     * Creates object for wayland native object.
     *
     * @param wlObject
     *      Wayland XDG toplevel native object.
     *
     * @return
     *      Created object or null on error.
     */
    static XdgToplevelPtr create(xdg_toplevel* wlObject);

};

} // namespace waylandcpp

#endif /*WAYLANDCPP_OBJECTFACTORY_HPP_*/
