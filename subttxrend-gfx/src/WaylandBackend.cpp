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


#include "WaylandBackend.hpp"

#include <set>
#include <cstring>
#include <unistd.h>
#include <linux/input.h>

#include <subttxrend/common/Logger.hpp>

#include "waylandcpp-utils/EpollDisplayHandler.hpp"
#include "waylandcpp-utils/KeymapFactory.hpp"
#include "waylandcpp-client/Client.hpp"
#include "waylandcpp-client/File.hpp"
#include "waylandcpp-client/PixelFormat.hpp"

#include "Pixmap.hpp"
#include "WaylandBackendLoopGlib.hpp"
#include "WaylandBackendLoopEpoll.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

subttxrend::common::Logger g_logger("Gfx", "WaylandBackend");

} // namespace <anonymous>

//------------------------------------------

WaylandBackend::WaylandBackend(BackendListener* listener) :
        Backend(listener)
{
    // noop
}

WaylandBackend::~WaylandBackend()
{
    // noop
}

bool WaylandBackend::startDisplay(const std::string &displayName)
{
    g_logger.info("%s - Connecting to display", __func__);

    m_display = waylandcpp::Client::connect<waylandcpp::Display1>(displayName.empty() ? nullptr : displayName.c_str());
    if (!m_display)
    {
        g_logger.error("%s - Can't connect to display", __func__);
        return false;
    }

    g_logger.info("%s - connected to display", __func__);

    m_registry = m_display->getRegistry<waylandcpp::Registry1>();
    if (!m_registry)
    {
        g_logger.error("%s - Can't get registry:1", __func__);
        return false;
    }
    m_registry->setListener(this);

    m_display->dispatch();
    m_display->roundtrip();

    return true;
}

bool WaylandBackend::checkRequiredBaseInterfaces() const
{
    if (!m_compositor)
    {
        g_logger.error("%s - Can't find compositor", __func__);
        return false;
    }
    else
    {
        g_logger.info("%s - Found compositor", __func__);
    }

    if (!m_shell)
    {
        g_logger.error("%s - Can't find shell", __func__);
        return false;
    }
    else
    {
        g_logger.info("%s - Found shm", __func__);
    }

    return true;
}

bool WaylandBackend::createSurface()
{
    m_surface = m_compositor->createSurface<waylandcpp::Surface1>();
    if (!m_surface)
    {
        g_logger.error("%s - Can't create surface", __func__);
        return false;
    }
    else
    {
        g_logger.info("%s - Created surface", __func__);
    }

    m_shellSurface = m_shell->getShellSurface<waylandcpp::ShellSurface1>(
            m_surface);
    if (!m_shellSurface)
    {
        g_logger.error("%s - Can't create shell surface", __func__);
        return false;
    }
    else
    {
        g_logger.info("%s - Created shell surface", __func__);
    }
    m_shellSurface->setTopLevel();
//    m_shellSurface->setFullScreen();
#ifdef PC_BUILD
    m_shellSurface->setMaximized(nullptr);
#endif
    m_shellSurface->setListener(this);

    return true;
}

bool WaylandBackend::isSyncNeeded() const
{
    if (m_loop)
    {
        return m_loop->isSyncNeeded();
    }
    return true;
}

bool WaylandBackend::init(const std::string &displayName)
{
    if (!startDisplay(displayName))
    {
        g_logger.error("%s - Can't start display", __func__);
        return false;
    }

    m_loop.reset(new WaylandBackendLoopEpoll());

    return true;
}

bool WaylandBackend::start()
{
    return m_loop->start(m_display, this);
}

void WaylandBackend::stop()
{
    m_loop->stop();
}

void WaylandBackend::requestRender()
{
    g_logger.trace("%s", __func__);

    if (not m_renderRequested.exchange(true))
    {
        m_loop->requestWakeup();
    }
}

void WaylandBackend::forceRender()
{
    g_logger.trace("%s", __func__);

    m_forceRender.store(true, std::memory_order_relaxed);
    m_renderRequested.store(true, std::memory_order_relaxed);

    m_loop->requestWakeup();
}

void WaylandBackend::loopStarted()
{
    g_logger.trace("%s", __func__);

    // at this point the registry listener should have
    // reported the interfaces

    if (!checkRequiredBaseInterfaces() || !checkRequiredInterfaces())
    {
        g_logger.fatal("%s - Required wayland interfaces missing", __func__);
        return;
    }

    if (!createSurface())
    {
        g_logger.fatal("%s - Cannot create surface", __func__);
        return;
    }

    if (!initRendering())
    {
        g_logger.fatal("%s - Cannot init rendering", __func__);
        return;
    }

    m_surface->addFrameListener(this);

    m_frameReady = true;
    m_renderRequested = true;

    redraw(0);
}

void WaylandBackend::loopFinished()
{
    g_logger.trace("%s", __func__);
}

void WaylandBackend::loopWakeupReceived()
{
    auto forceRender = m_forceRender.load(std::memory_order_relaxed);

    g_logger.trace("%s forceRender=%d", __func__, forceRender);

    if ((m_renderRequested and m_frameReady) or forceRender)
    {
        redraw(0);
    }
}

void WaylandBackend::global(waylandcpp::RegistryPtr /*registry*/,
                            uint32_t name,
                            std::string interface,
                            uint32_t version)
{
    g_logger.info("%s - name=%u interface=%s:%u", __func__, name,
            interface.c_str(), version);

    if (interface == "wl_compositor")
    {
        m_compositor = m_registry->bind<waylandcpp::Compositor1>(name);
    }
    else if (interface == "wl_shell")
    {
        m_shell = m_registry->bind<waylandcpp::Shell1>(name);
    }
    else if (interface == "wl_output")
    {
        auto output = m_registry->bind<waylandcpp::Output1>(name);
        if (output)
        {
            g_logger.info("%s - Output added: name=%u", __func__, name);

            m_outputs.insert(std::make_pair(name, output));
            output->setListener(this);
        }
    }
    else if (interface == "wl_seat")
    {
        auto seat = m_registry->bind<waylandcpp::Seat1>(name);
        if (seat)
        {
            g_logger.info("%s - Seat added: name=%u", __func__, name);

            m_seats.insert(std::make_pair(name, SeatEntry(seat)));
            seat->setListener(this);
        }
    }
#if defined(WESTEROS)
    else if (interface == "wl_simple_shell")
    {
        m_simpleShell = m_registry->bind<waylandcpp::SimpleShell1>(name);
        if (m_simpleShell)
        {
            g_logger.info("%s - wl_simple_shell added: name=%u", __func__, name);
            m_simpleShell->setListener(this);
        }
        else
        {
            g_logger.error("%s - Failed to bind wl_simple_shell: name:%u", __func__, name);
        }
    }
#endif

    interfaceAdded(m_registry, name, interface, version);
}

void WaylandBackend::globalRemove(waylandcpp::RegistryPtr /*registry*/,
                                  uint32_t name)
{
    g_logger.info("%s - Got a registry losing event for %u", __func__, name);

    auto outputIter = m_outputs.find(name);
    if (outputIter != m_outputs.end())
    {
        g_logger.info("%s - Output removed: name=%u", __func__, name);

        m_outputs.erase(outputIter);
    }

    auto seatIter = m_seats.find(name);
    if (seatIter != m_seats.end())
    {
        g_logger.info("%s - Seat removed: name=%u", __func__, name);

        m_seats.erase(seatIter);
    }

    interfaceRemoved(m_registry, name);
}

void WaylandBackend::configure(waylandcpp::ShellSurfacePtr /*shellSurface*/,
                               uint32_t edges,
                               int32_t width,
                               int32_t height)
{
    g_logger.info("%s - edges=%u width=%d height=%d", __func__, edges, width,
            height);

    surfaceResizeRequested(Size
    { width, height });
}

void WaylandBackend::ping(waylandcpp::ShellSurfacePtr /*shellSurface*/,
                          uint32_t serial)
{
    g_logger.info("%s - Pinged and ponged", __func__);
    m_shellSurface->pong(serial);
}

void WaylandBackend::popupDone(waylandcpp::ShellSurfacePtr /*shellSurface*/)
{
    g_logger.info("%s", __func__);
}

#if defined(WESTEROS)
void WaylandBackend::surfaceId(waylandcpp::SimpleShellPtr /*simpleShell*/,
                               struct wl_surface *surface,
                               uint32_t surfaceId)
{
    g_logger.info("%s - simple_shell surfaceId - surface:%p id:%u", __func__, surface, surfaceId);

    if (m_simpleShell && m_surface)
    {
        if (m_surface->getNativeObject() == surface)
        {
            m_currentSurfaceId = surfaceId;

            const char* env_name = ::getenv("SUBTEC_SURFACE_NAME");
            wl_simple_shell_set_name(m_simpleShell->getNativeObject(), surfaceId, env_name==NULL ? "subtitles-subtec" : env_name);
        }
    }
}

void WaylandBackend::surfaceCreated(waylandcpp::SimpleShellPtr /*simpleShell*/,
                                    uint32_t surfaceId,
                                    const char *name)
{
    g_logger.info("%s - simple_shell surfaceCreated - name:%s id:%u", __func__, name, surfaceId);

    /* Start with the surface invisible */
    if (m_simpleShell && (m_currentSurfaceId == surfaceId))
    {
        g_logger.info("%s - Hiding surface with id %u on creation", __func__, surfaceId);
        wl_simple_shell_set_visible(m_simpleShell->getNativeObject(), surfaceId, 0);
    }
}

void WaylandBackend::surfaceDestroyed(waylandcpp::SimpleShellPtr /*simpleShell*/,
                      uint32_t surfaceId,
                      const char *name)
{
    g_logger.info("%s - SimpleShell surfaceDestroyed - '%s'", __func__, name);
}

void WaylandBackend::surfaceStatus(waylandcpp::SimpleShellPtr /*simpleShell*/,
                   uint32_t surfaceId,
                   const char *name,
                   uint32_t visible,
                   int32_t x,
                   int32_t y,
                   int32_t width,
                   int32_t height,
                   wl_fixed_t opacity,
                   wl_fixed_t zorder)
{
    g_logger.info("%s - SimpleShell status - name:%s visible:%s position:%u,%u size:%u,%u opacity:%f zorder:%f",
                  __func__, name, visible ? "Yes" : "No", x, y, width, height, wl_fixed_to_double(opacity),
                  wl_fixed_to_double(zorder) );
}

void WaylandBackend::surfacesDone(waylandcpp::SimpleShellPtr /*simpleShell*/)
{
    g_logger.info("%s - SimpleShell surfacesDone", __func__);
}
#endif /* defined(WESTEROS) */

void WaylandBackend::geometry(waylandcpp::OutputPtr owner,
                              int32_t x,
                              int32_t y,
                              int32_t physicalWidth,
                              int32_t physicalHeight,
                              int32_t subpixel,
                              const char *make,
                              const char *model,
                              int32_t transform)
{
    g_logger.info(
            "%s - out=%p x=%d y=%d pw=%d ph=%d sp=%d make=%s model=%s tfm=%d",
            __func__, owner->getNativeObject(), x, y, physicalWidth,
            physicalHeight, subpixel, make, model, transform);
}

void WaylandBackend::mode(waylandcpp::OutputPtr owner,
                          uint32_t flags,
                          int32_t width,
                          int32_t height,
                          int32_t refresh)
{
    g_logger.info("%s - out=%p flags=%u w=%d h=%d r=%d", __func__,
            owner->getNativeObject(), flags, width, height, refresh);

    if ((flags & OUTPUT_MODE_FLAG_CURRENT) != 0)
    {
        currentOutputModeChanged(Size
        { width, height });
    }
}

void WaylandBackend::outputEventsDone(waylandcpp::OutputPtr owner)
{
    g_logger.info("%s - out=%p", __func__, owner->getNativeObject());
}

void WaylandBackend::scale(waylandcpp::OutputPtr owner,
                           int32_t factor)
{
    g_logger.info("%s - out=%p factor=%d", __func__, owner->getNativeObject(),
            factor);
}

WaylandBackend::SeatEntry* WaylandBackend::findSeatEntry(const waylandcpp::SeatPtr& object)
{
    for (auto& item : m_seats)
    {
        if (item.second.m_seat)
        {
            if (item.second.m_seat->getNativeObject()
                    == object->getNativeObject())
            {
                return &item.second;
            }
        }
    }

    return nullptr;
}

WaylandBackend::SeatEntry* WaylandBackend::findSeatEntry(const waylandcpp::KeyboardPtr& object)
{
    for (auto& item : m_seats)
    {
        if (item.second.m_keyboard)
        {
            if (item.second.m_keyboard->getNativeObject()
                    == object->getNativeObject())
            {
                return &item.second;
            }
        }
    }

    return nullptr;
}

void WaylandBackend::capabilities(waylandcpp::SeatPtr object,
                                  uint32_t capabilities)
{
    g_logger.info("%s - seat=%p capabilities=%u", __func__,
            object->getNativeObject(), capabilities);

    auto seatEntry = findSeatEntry(object);

    if (seatEntry)
    {
        if ((capabilities & CAPABILITY_FLAG_KEYBOARD) != 0)
        {
            // keyboard is available

            if (!seatEntry->m_keyboard)
            {
                auto keyboard = seatEntry->m_seat->getKeyboard<
                        waylandcpp::Keyboard1>(capabilities);
                if (keyboard)
                {
                    if (keyboard->setListener(this))
                    {
                        g_logger.info("%s - Keyboard added: seat=%p", __func__,
                                seatEntry->m_seat->getNativeObject());

                        seatEntry->m_keyboard = keyboard;
                    }
                    else
                    {
                        g_logger.info("%s - cannot set keyboard listener",
                                __func__);
                    }
                }
                else
                {
                    g_logger.info("%s - cannot get keyboard", __func__);
                }
            }
        }
        else
        {
            // keyboard is not available

            if (seatEntry->m_keyboard)
            {
                g_logger.info("%s - Keyboard removed: seat=%p", __func__,
                        seatEntry->m_seat->getNativeObject());

                seatEntry->m_keyboard.reset();
            }
        }
    }
    else
    {
        g_logger.info("%s - invalid seat version", __func__);
    }
}

void WaylandBackend::name(waylandcpp::SeatPtr object,
                          const char *name)
{
    g_logger.info("%s - seat=%p name=%s", __func__, object->getNativeObject(),
            name);
}

void WaylandBackend::keymap(waylandcpp::KeyboardPtr object,
                            uint32_t format,
                            int32_t fd,
                            uint32_t size)
{
    g_logger.info("%s - keyboard=%p format=%u fd=%d size=%u", __func__,
            object->getNativeObject(), format, fd, size);

    auto seatEntry = findSeatEntry(object);

    if (seatEntry)
    {
        if (seatEntry->m_keymap)
        {
            g_logger.info("%s - keymap removed for keyboard=%p", __func__,
                    object->getNativeObject());

            seatEntry->m_keymap.reset();
        }

        seatEntry->m_keymap = waylandcpp::KeymapFactory::createKeymap(format,
                fd, size);
        if (seatEntry->m_keymap)
        {
            g_logger.info("%s - keymap set for keyboard=%p", __func__,
                    object->getNativeObject());
        }
    }

    ::close(fd);
}

void WaylandBackend::enter(waylandcpp::KeyboardPtr object,
                           uint32_t serial,
                           struct wl_surface *surface,
                           struct wl_array *keys)
{
    g_logger.trace("%s - keyboard=%p", __func__, object->getNativeObject());

    if (m_surface)
    {
        if (m_surface->getNativeObject() == surface)
        {
            getListener()->focusGained();
        }
    }
}

void WaylandBackend::leave(waylandcpp::KeyboardPtr object,
                           uint32_t serial,
                           struct wl_surface *surface)
{
    g_logger.trace("%s - keyboard=%p", __func__, object->getNativeObject());

    getListener()->focusLost();
}

void WaylandBackend::key(waylandcpp::KeyboardPtr object,
                         uint32_t serial,
                         uint32_t time,
                         uint32_t key,
                         uint32_t state)
{
    g_logger.trace("%s - keyboard=%p serial=%u t=%u k=%u s=%u", __func__,
            object->getNativeObject(), serial, time, key, state);

    auto seatEntry = findSeatEntry(object);

    if (seatEntry && seatEntry->m_keymap)
    {
        auto waylandKeyEvent = seatEntry->m_keymap->processKey(key, state);

        g_logger.trace("%s - type=%d code=%u mod=%u symbol=%08X", __func__,
                (int) waylandKeyEvent.m_type, waylandKeyEvent.m_code,
                waylandKeyEvent.m_modifiers, waylandKeyEvent.m_symbol);

        if (waylandKeyEvent.m_symbol != XKB_KEY_VoidSymbol)
        {
            if (waylandKeyEvent.m_type == waylandcpp::KeyEvent::Type::PRESSED)
            {
                getListener()->processKeyEvent(
                        KeyEvent(KeyEvent::Type::PRESSED,
                                waylandKeyEvent.m_symbol));
            }
            else if (waylandKeyEvent.m_type
                    == waylandcpp::KeyEvent::Type::RELEASED)
            {
                getListener()->processKeyEvent(
                        KeyEvent(KeyEvent::Type::RELEASED,
                                waylandKeyEvent.m_symbol));
            }
        }
    }
}

void WaylandBackend::modifiers(waylandcpp::KeyboardPtr object,
                               uint32_t serial,
                               uint32_t mods_depressed,
                               uint32_t mods_latched,
                               uint32_t mods_locked,
                               uint32_t group)
{
    g_logger.trace("%s - keyboard=%p", __func__, object->getNativeObject());

    auto seatEntry = findSeatEntry(object);

    if (seatEntry)
    {
        if (seatEntry->m_keymap)
        {
            seatEntry->m_keymap->processModifiers(mods_depressed, mods_latched,
                    mods_locked, group);
        }
    }
}

void WaylandBackend::repeatInfo(waylandcpp::KeyboardPtr object,
                                int32_t rate,
                                int32_t delay)
{
    g_logger.info("%s - keyboard=%p", __func__, object->getNativeObject());
}

void WaylandBackend::frameDone(uint32_t frameTime)
{
    g_logger.trace("%s", __func__);

    m_frameReady = true;

    if (m_renderRequested)
    {
        redraw(frameTime);
    }
}

void WaylandBackend::redraw(uint32_t /*time*/)
{
    g_logger.debug("%s - Redrawing", __func__);

    m_forceRender.store(false, std::memory_order_relaxed);
    redraw(m_surface);
}

Size WaylandBackend::calculateContentSize()
{
    class SizeCalculator : public BackendWindowEnumerator
    {
    public:
        virtual void processWindow(const Pixmap& pixmap) override
        {
            m_size.m_w = std::max(m_size.m_w, pixmap.getWidth());
            m_size.m_h = std::max(m_size.m_h, pixmap.getHeight());
        }

        virtual void processWindow(const Pixmap& pixmap, const Pixmap& bgPixmap) override
        {
            m_size.m_w = std::max(m_size.m_w, pixmap.getWidth());
            m_size.m_h = std::max(m_size.m_h, pixmap.getHeight());
        }
        Size getSize() const
        {
            return m_size;
        }

    private:
        Size m_size;
    };

    SizeCalculator calculator;

    getListener()->enumerateVisibleWindows(calculator);

    return calculator.getSize();
}

} // namespace gfx
} // namespace subttxrend
