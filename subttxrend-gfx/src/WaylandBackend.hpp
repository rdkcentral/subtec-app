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


#ifndef SUBTTXREND_GFX_WAYLAND_BACKEND_HPP_
#define SUBTTXREND_GFX_WAYLAND_BACKEND_HPP_

#include <map>
#include <atomic>

#include "Types.hpp"
#include "Backend.hpp"
#include "waylandcpp-client/Types.hpp"
#include "waylandcpp-client/Registry.hpp"
#include "waylandcpp-client/ShellSurface.hpp"
#include "waylandcpp-client/Surface.hpp"
#include "waylandcpp-client/Output.hpp"
#include "waylandcpp-client/Seat.hpp"
#include "waylandcpp-client/Display.hpp"
#include "waylandcpp-client/Compositor.hpp"
#include "waylandcpp-client/Shell.hpp"
#include "waylandcpp-utils/Keymap.hpp"
#include "WaylandBackendLoop.hpp"
#if defined(WESTEROS)
#include "waylandcpp-client/SimpleShell.hpp"
#endif

namespace subttxrend
{
namespace gfx
{

/**
 * Rendering backend using wayland.
 */
class WaylandBackend : public Backend,
                       private waylandcpp::RegistryListener,
                       private waylandcpp::ShellSurfaceListener,
#if defined(WESTEROS)
                       private waylandcpp::SimpleShellListener,
#endif
                       private waylandcpp::SurfaceFrameListener,
                       private waylandcpp::OutputListener,
                       private waylandcpp::SeatListener,
                       private waylandcpp::KeyboardListener,
                       private WaylandBackendLoopListener
{
public:
    /**
     * Constructor.
     *
     * @param listener
     *      Listener for backend events.
     */
    WaylandBackend(BackendListener* listener);

    /**
     * Destructor.
     */
    virtual ~WaylandBackend();

    /** @copydoc Backend::isSyncNeeded() */
    virtual bool isSyncNeeded() const override final;

    /** @copydoc Backend::init() */
    virtual bool init(const std::string &displayName) override final;

    /** @copydoc Backend::start() */
    virtual bool start() override final;

    /** @copydoc Backend::stop() */
    virtual void stop() override final;

    /**
     * Requests render.
     *
     * This method shall be used to request the frame to be redrawn.
     */
    virtual void requestRender() final;

    /**
     * Force render.
     *
     * This method shall be used to request the frame to be redrawn without
     * waiting for external conditions (like frame done callback).
     */
    virtual void forceRender() final;

protected:
    /**
     * Initializes elements required for contents rendering.
     *
     * @return
     *      True on success, false on error.
     */
    virtual bool initRendering() = 0;

    /**
     * Checks if required wayland interfaces are present.
     *
     * @return
     *      True on success, false on error.
     */
    virtual bool checkRequiredInterfaces() const = 0;

    /**
     * Processes current output mode change notification.
     *
     * @param size
     *      New size of the output.
     */
    virtual void currentOutputModeChanged(const Size& size) = 0;

    /**
     * Surface resize (configuration change) was requested.
     *
     * @param size
     *      New requested size.
     */
    virtual void surfaceResizeRequested(const Size& size) = 0;

    /**
     * Performs redraw of the surface contents.
     *
     * @param surface
     *      Surface on which to draw.
     */
    virtual void redraw(const waylandcpp::Surface1::Ptr& surface) = 0;

    /**
     * Called when wayland interface is added.
     *
     * @param registry
     *      Registry object.
     * @param name
     *      Interface identifier.
     * @param interface
     *      Interface type name.
     * @param version
     *      Interface version.
     */
    virtual void interfaceAdded(waylandcpp::Registry1::Ptr registry,
                                uint32_t name,
                                std::string interface,
                                uint32_t version) = 0;

    /**
     * Called when wayland interface is removed.
     *
     * @param registry
     *      Registry object.
     * @param name
     *      Interface identifier.
     */
    virtual void interfaceRemoved(waylandcpp::Registry1::Ptr registry,
                                  uint32_t name) = 0;

    /**
     * Returns display object.
     *
     * @return
     *      Display object.
     */
    waylandcpp::Display1::Ptr getDisplay()
    {
        return m_display;
    }

    /**
     * Returns surface object.
     *
     * @return
     *      Surface object.
     */
    waylandcpp::Surface1::Ptr getSurface()
    {
        return m_surface;
    }

    /**
     * Returns simple shell object.
     *
     * @return
     *      Surface object.
     */
#if defined(WESTEROS)
    waylandcpp::SimpleShell1::Ptr getSimpleShell()
    {
        return m_simpleShell;
    }
#endif

    /**
     * Calculates content size.
     *
     * @return
     *      Calculated size.
     */
    Size calculateContentSize();

    // TODO: shall not be protected

    /** Frame ready flag. */
    std::atomic<bool> m_frameReady;

    /** Render requested flag. */
    std::atomic<bool> m_renderRequested;

    /** Force render flag. */
    std::atomic<bool> m_forceRender;

    /** Current surface id. */
    uint32_t m_currentSurfaceId;

private:
    /**
     * Seat related data.
     */
    struct SeatEntry
    {
        /**
         * Constructor.
         *
         * @param seat
         *      Seat pointer.
         */
        SeatEntry(const waylandcpp::Seat1::Ptr& seat) :
                m_seat(seat)
        {
            // noop
        }

        /** Seat pointer. */
        waylandcpp::Seat1::Ptr m_seat;

        /** Keyboard pointer. */
        waylandcpp::Keyboard1::Ptr m_keyboard;

        /** Keyboard keymap.*/
        waylandcpp::KeymapPtr m_keymap;
    };

    /**
     * Starts wayland display.
     *
     * Connects to wayland display, gets registary, sets listeners
     * and asks for existing interfaces to be enumerated.
     *
     * @return
     *      True on success, false otherwise.
     */
    bool startDisplay(const std::string &displayName);

    /**
     * Checks if required wayland interfaces are present.
     *
     * @return
     *      True on success, false on error.
     */
    bool checkRequiredBaseInterfaces() const;

    /**
     * Creates surface.
     *
     * Constructs wayland surface and shell surface.
     *
     * @return
     *      True on success, false on error.
     */
    bool createSurface();

    /**
     * Finds seat entry for given object.
     *
     * @param object
     *      Object for each find is requested.
     *
     * @return
     *      Pointer to entry if found, null otherwise.
     */
    SeatEntry* findSeatEntry(const waylandcpp::SeatPtr& object);

    /**
     * Finds seat entry for given object.
     *
     * @param object
     *      Object for each find is requested.
     *
     * @return
     *      Pointer to entry if found, null otherwise.
     */
    SeatEntry* findSeatEntry(const waylandcpp::KeyboardPtr& object);

    /** @copydoc WaylandBackendLoopListener::loopStarted */
    virtual void loopStarted() override;

    /** @copydoc WaylandBackendLoopListener::loopFinished */
    virtual void loopFinished() override;

    /** @copydoc WaylandBackendLoopListener::loopWakeupReceived */
    virtual void loopWakeupReceived() override;

    /** @copydoc waylandcpp::RegistryListener::global */
    virtual void global(waylandcpp::RegistryPtr object,
                        uint32_t name,
                        std::string interface,
                        uint32_t version) override;

    /** @copydoc waylandcpp::RegistryListener::globalRemove */
    virtual void globalRemove(waylandcpp::RegistryPtr object,
                              uint32_t name) override;

    /** @copydoc waylandcpp::ShellSurfaceListener::ping */
    virtual void ping(waylandcpp::ShellSurfacePtr object,
                      uint32_t serial) override;

    /** @copydoc waylandcpp::ShellSurfaceListener::configure */
    virtual void configure(waylandcpp::ShellSurfacePtr object,
                           uint32_t edges,
                           int32_t width,
                           int32_t height) override;

    /** @copydoc waylandcpp::ShellSurfaceListener::popupDone */
    virtual void popupDone(waylandcpp::ShellSurfacePtr object) override;

    /** @copydoc waylandcpp::OutputListener::geometry */
    virtual void geometry(waylandcpp::OutputPtr object,
                          int32_t x,
                          int32_t y,
                          int32_t physicalWidth,
                          int32_t physicalHeight,
                          int32_t subpixel,
                          const char *make,
                          const char *model,
                          int32_t transform) override;

    /** @copydoc waylandcpp::OutputListener::mode */
    virtual void mode(waylandcpp::OutputPtr object,
                      uint32_t flags,
                      int32_t width,
                      int32_t height,
                      int32_t refresh) override;

    /** @copydoc waylandcpp::OutputListener::outputEventsDone */
    virtual void outputEventsDone(waylandcpp::OutputPtr object) override;

    /** @copydoc waylandcpp::OutputListener::scale */
    virtual void scale(waylandcpp::OutputPtr object,
                       int32_t factor) override;

    /** @copydoc waylandcpp::SeatListener::capabilities */
    virtual void capabilities(waylandcpp::SeatPtr object,
                              uint32_t capabilities) override;

    /** @copydoc waylandcpp::SeatListener::name */
    virtual void name(waylandcpp::SeatPtr object,
                      const char *name) override;

    /** @copydoc waylandcpp::SurfaceFrameListener::frameDone */
    virtual void frameDone(uint32_t frameTime) override;

    /** @copydoc waylandcpp::KeyboardListener::keymap */
    virtual void keymap(waylandcpp::KeyboardPtr object,
                        uint32_t format,
                        int32_t fd,
                        uint32_t size) override;

    /** @copydoc waylandcpp::KeyboardListener::enter */
    virtual void enter(waylandcpp::KeyboardPtr object,
                       uint32_t serial,
                       struct wl_surface *surface,
                       struct wl_array *keys) override;

    /** @copydoc waylandcpp::KeyboardListener::leave */
    virtual void leave(waylandcpp::KeyboardPtr object,
                       uint32_t serial,
                       struct wl_surface *surface) override;

    /** @copydoc waylandcpp::KeyboardListener::key */
    virtual void key(waylandcpp::KeyboardPtr object,
                     uint32_t serial,
                     uint32_t time,
                     uint32_t key,
                     uint32_t state) override;

    /** @copydoc waylandcpp::KeyboardListener::modifiers */
    virtual void modifiers(waylandcpp::KeyboardPtr object,
                           uint32_t serial,
                           uint32_t mods_depressed,
                           uint32_t mods_latched,
                           uint32_t mods_locked,
                           uint32_t group) override;

    /** @copydoc waylandcpp::KeyboardListener::repeatInfo */
    virtual void repeatInfo(waylandcpp::KeyboardPtr object,
                            int32_t rate,
                            int32_t delay) override;

#if defined(WESTEROS)
    /** @copydoc waylandcpp::SimpleShellListener::surfaceId */
    virtual void surfaceId(waylandcpp::SimpleShellPtr object,
                           struct wl_surface *surface,
                           uint32_t surfaceId) override;

    /** @copydoc waylandcpp::SimpleShellListener::surfaceCreated */
    virtual void surfaceCreated(waylandcpp::SimpleShellPtr object,
                                uint32_t surfaceId,
                                const char *name) override;

    /** @copydoc waylandcpp::SimpleShellListener::surfaceDestroyed */
    virtual void surfaceDestroyed(waylandcpp::SimpleShellPtr object,
                                  uint32_t surfaceId,
                                  const char *name) override;

    /** @copydoc waylandcpp::SimpleShellListener::surfaceStatus */
    virtual void surfaceStatus(waylandcpp::SimpleShellPtr object,
                               uint32_t surfaceId,
                               const char *name,
                               uint32_t visible,
                               int32_t x,
                               int32_t y,
                               int32_t width,
                               int32_t height,
                               wl_fixed_t opacity,
                               wl_fixed_t zorder) override;

    /** @copydoc waylandcpp::SimpleShellListener::surfacesDone */
    virtual void surfacesDone(waylandcpp::SimpleShellPtr object) override;
#endif /* defined(WESTEROS) */

    /**
     * Redraws the frame.
     *
     * @param time
     *      Ignored parameter (frame update time).
     */
    void redraw(uint32_t time);

    /**
     * Sets current size.
     *
     * @param newSize
     *      New size.
     * @param doRepaint
     *      Request repaint if true and size was modified.
     */
    void setCurrentSize(const Size& newSize,
                        bool doRepaint);

    /**
     * Returns current size.
     *
     * @return
     *      Current size.
     */
    const Size& getCurrentSize() const;

    /**
     * Maps key to vlHal input events codes.
     *
     * @param keyEvent
     *      Key event description.
     *
     * @return
     *      VL HAL input event code.
     */
    uint32_t getHalInputEventCode(const waylandcpp::KeyEvent& keyEvent) const;

    /** Wayland interface - display. */
    waylandcpp::Display1::Ptr m_display;

    /** Wayland interface - registry. */
    waylandcpp::Registry1::Ptr m_registry;

    /** Wayland interface - compositor. */
    waylandcpp::Compositor1::Ptr m_compositor;

    /** Wayland interface - shell. */
    waylandcpp::Shell1::Ptr m_shell;

#if defined(WESTEROS)
    /** Westeros interface - simple shell. */
    waylandcpp::SimpleShell1::Ptr m_simpleShell;
#endif

    /** Wayland interface - surface. */
    waylandcpp::Surface1::Ptr m_surface;

    /** Wayland interface - shell surface. */
    waylandcpp::ShellSurface1::Ptr m_shellSurface;

    /** Wayland interfaces - outputs. */
    std::map<uint32_t, waylandcpp::Output1::Ptr> m_outputs;

    /** Wayland interfaces - seats. */
    std::map<uint32_t, SeatEntry> m_seats;

    /** Wayland display loop. */
    std::unique_ptr<WaylandBackendLoop> m_loop;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_BACKEND_HPP_
