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


#ifndef WAYLANDCPP_TYPES_HPP_
#define WAYLANDCPP_TYPES_HPP_

#include <memory>

#include "Proxy.hpp"

/* == wayland types == */
struct wl_display;
struct wl_registry;
struct wl_shm;
struct wl_compositor;
struct wl_shell;
struct wl_shell_surface;
struct wl_surface;
struct wl_output;
struct wl_buffer;
struct wl_shm_pool;
struct wl_seat;
struct wl_array;
struct wl_keyboard;

#if defined(WESTEROS)
struct wl_simple_shell;
#endif

struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;

struct wl_interface;
struct wl_callback;

namespace waylandcpp
{

/* == types == */

class Display;
class Registry;
class Shm;
class ShmPool;
class Compositor;
class Shell;
class ShellSurface;
class Surface;
class Buffer;
class BufferParams;
class Output;
class File;
class Seat;
class Keyboard;

#if defined(WESTEROS)
class SimpleShell;
#endif

class XdgWmBase;
class XdgSurface;
class XdgToplevel;

/* == pointers == */

/** Pointer - Display object. */
typedef std::shared_ptr<Display> DisplayPtr;

/** Pointer - Registry. */
typedef std::shared_ptr<Registry> RegistryPtr;

/** Pointer - Shm. */
typedef std::shared_ptr<Shm> ShmPtr;

/** Pointer - Shm Pool. */
typedef std::shared_ptr<ShmPool> ShmPoolPtr;

/** Pointer - Compositor. */
typedef std::shared_ptr<Compositor> CompositorPtr;

/** Pointer - Shell. */
typedef std::shared_ptr<Shell> ShellPtr;

#if defined(WESTEROS)
/** Pointer - Simple Shell. */
typedef std::shared_ptr<SimpleShell> SimpleShellPtr;
#endif

/** Pointer - Shell Surface. */
typedef std::shared_ptr<ShellSurface> ShellSurfacePtr;

/** Pointer - Surface. */
typedef std::shared_ptr<Surface> SurfacePtr;

/** Pointer - Buffer. */
typedef std::shared_ptr<Buffer> BufferPtr;

/** Pointer - Output. */
typedef std::shared_ptr<Output> OutputPtr;

/** Pointer - File. */
typedef std::shared_ptr<File> FilePtr;

/** Pointer - Seat. */
typedef std::shared_ptr<Seat> SeatPtr;

/** Pointer - Keyboard. */
typedef std::shared_ptr<Keyboard> KeyboardPtr;

/** Pointer - XDG wm base. */
typedef std::shared_ptr<XdgWmBase> XdgWmBasePtr;

/** Pointer - XDG surface. */
typedef std::shared_ptr<XdgSurface> XdgSurfacePtr;

/** Pointer - XDG toplevel. */
typedef std::shared_ptr<XdgToplevel> XdgToplevelPtr;

} // namespace waylandcpp

#endif /*WAYLANDCPP_TYPES_HPP_*/
