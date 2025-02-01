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


#ifndef SUBTTXREND_GFX_WAYLAND_BACKEND_SHM_HPP_
#define SUBTTXREND_GFX_WAYLAND_BACKEND_SHM_HPP_

#include "WaylandBackend.hpp"
#include "WaylandBuffer.hpp"
#include "waylandcpp-client/Shm.hpp"

namespace subttxrend
{
namespace gfx
{

class WaylandBufferManager;

/**
 * Rendering backend using wayland-shm.
 */
class WaylandBackendShm : public WaylandBackend,
                          private waylandcpp::ShmListener
{
public:
    /**
     * Constructor.
     *
     * @param listener
     *      Listener for backend events.
     */
    WaylandBackendShm(BackendListener* listener);

    /**
     * Destructor.
     */
    virtual ~WaylandBackendShm();

protected:
    virtual bool initRendering() override;

    virtual bool checkRequiredInterfaces() const override;

    virtual void currentOutputModeChanged(const Size& size) override;

    virtual void surfaceResizeRequested(const Size& size) override;

    virtual void redraw(const waylandcpp::Surface1::Ptr& surface) override;

    virtual void interfaceAdded(waylandcpp::Registry1::Ptr registry,
                                uint32_t name,
                                std::string interface,
                                uint32_t version) override;

    virtual void interfaceRemoved(waylandcpp::Registry1::Ptr registry,
                                  uint32_t name) override;

private:
    virtual void format(waylandcpp::ShmPtr object,
                        uint32_t nativeFormat) override;

    /**
     * Paints the frame pixels.
     *
     * @param buffer
     *      Buffer to draw on.
     */
    void paintPixels(const WaylandBuffer::Ptr& buffer);

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

    /** Wayland interface - shm. */
    waylandcpp::Shm1::Ptr m_shm;

    /** Buffer manager. */
    std::unique_ptr<WaylandBufferManager> m_bufferManager;

    /** Current size. */
    Size m_currentSize;

    /** Default size. */
    static const Size DEFAULT_SIZE;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_BACKEND_SHM_HPP_
