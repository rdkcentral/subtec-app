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


#include "WaylandBackendShm.hpp"

#include <subttxrend/common/Logger.hpp>

#include "Blitter.hpp"
#include "Pixmap.hpp"
#include "WaylandBuffer.hpp"
#include "WaylandBufferManager.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

/**
 * Wayland surface resize mode.
 */
enum class ResizeMode
{
    /**
     * Resize surface to output resolution.
     */
    OUTPUT_RESOLUTION,

    /**
     * Resize surface to maximum surface size.
     */
    MAX_WINDOW_SIZE,

    /**
     * User controlled.
     */
    USER_CONTROLLED
};

/** Currently selected resize mode. */
const ResizeMode g_resizeMode = ResizeMode::MAX_WINDOW_SIZE;

subttxrend::common::Logger g_logger("Gfx", "WaylandBackendShm");

} // namespace <anonymous>

//------------------------------------------

const Size WaylandBackendShm::DEFAULT_SIZE =
{ 720, 480 };

WaylandBackendShm::WaylandBackendShm(BackendListener* listener) :
        WaylandBackend(listener),
        m_currentSize({0, 0})
{
    // noop
}

WaylandBackendShm::~WaylandBackendShm()
{
    // noop
}

bool WaylandBackendShm::initRendering()
{
    m_bufferManager.reset(new WaylandBufferManager(m_shm));

    if (g_resizeMode == ResizeMode::USER_CONTROLLED)
    {
        setCurrentSize(DEFAULT_SIZE, false);
    }
    else
    {
        setCurrentSize(Size
        { 0, 0 }, false);
    }

    return true;
}

bool WaylandBackendShm::checkRequiredInterfaces() const
{
    if (!m_shm)
    {
        g_logger.error("%s - Can't find shm", __func__);
        return false;
    }
    else
    {
        g_logger.info("%s - Found shm", __func__);
    }

    return true;
}

void WaylandBackendShm::format(waylandcpp::ShmPtr /*shm*/,
                               uint32_t nativeFormat)
{
    auto format = waylandcpp::PixelFormatUtils::fromNativeFormat(nativeFormat);

    std::string formatStr = waylandcpp::PixelFormatUtils::getFormatString(
            format);

    g_logger.info("%s - Possible shmem format native=%u format=%s", __func__,
            nativeFormat, formatStr.c_str());
}

void WaylandBackendShm::currentOutputModeChanged(const Size& size)
{
    g_logger.info("%s - w=%d h=%d", __func__, size.m_w, size.m_h);

    if (g_resizeMode == ResizeMode::OUTPUT_RESOLUTION)
    {
        setCurrentSize(size, true);
    }
}

void WaylandBackendShm::surfaceResizeRequested(const Size& size)
{
    g_logger.info("%s - width=%d height=%d", __func__, size.m_w, size.m_h);

    if (g_resizeMode == ResizeMode::USER_CONTROLLED)
    {
        setCurrentSize(size, true);
    }

    getListener()->onPreferredSize(size);
}

void WaylandBackendShm::redraw(const waylandcpp::Surface1::Ptr& m_surface)
{
    if (g_resizeMode == ResizeMode::MAX_WINDOW_SIZE)
    {
        setCurrentSize(calculateContentSize(), false);
    }

    const auto& size = getCurrentSize();

    WaylandBuffer::Ptr buffer = m_bufferManager->getBuffer(size);

    if (!buffer)
    {
        g_logger.fatal("%s - cannot get buffer", __func__);

        // simulating redraw complete, so there is a chance
        // to get some buffer in future
        m_renderRequested = false;
        m_frameReady = true;
        return;
    }

    m_renderRequested = false;
    m_frameReady = false;

    if (!buffer->isEmpty())
    {
        paintPixels(buffer);
    }

    m_surface->damage(0, 0, buffer->getParams().m_width,
            buffer->getParams().m_height);
    m_surface->attach(buffer->markAttached());

    m_surface->commit();
}

void WaylandBackendShm::interfaceAdded(waylandcpp::Registry1::Ptr registry,
                                       uint32_t name,
                                       std::string interface,
                                       uint32_t version)
{
    g_logger.info("%s - name=%u interface=%s:%u", __func__, name,
            interface.c_str(), version);

    if (interface == "wl_shm")
    {
        m_shm = registry->bind<waylandcpp::Shm1>(name);
        if (m_shm)
        {
            m_shm->setListener(this);
        }
    }
}

void WaylandBackendShm::interfaceRemoved(waylandcpp::Registry1::Ptr registry,
                                         uint32_t name)
{
    // noop
}

void WaylandBackendShm::paintPixels(const WaylandBuffer::Ptr& buffer)
{
    auto params = buffer->getParams();

    g_logger.trace("%s - w=%d h=%d", __func__, params.m_width, params.m_height);

    Pixmap screenPixmap(reinterpret_cast<uint8_t*>(buffer->getDataPtr()),
            params.m_width, params.m_height, params.m_stride);

    Blitter::clear(screenPixmap);

    g_logger.trace("%s - rendering windows", __func__);

    class RenderEnumerator : public BackendWindowEnumerator
    {
    public:
        RenderEnumerator(Pixmap& screenPixmap) :
                m_screenPixmap(screenPixmap)
        {
            // noop
        }

        virtual void processWindow(const Pixmap& pixmap) override
        {
            g_logger.trace("%s - pixmap=%p size=%dx%d", __func__, &pixmap,
                    pixmap.getWidth(), pixmap.getHeight());

            // TODO: configurable mode
            auto mode = Blitter::DrawPosition::CENTER;
            // auto mode = Blitter::ResizeMode::UPDOWN;
            // auto mode = Blitter::ResizeMode::UPDOWN_PRESERVE_ASPECT;

            Blitter::write(m_screenPixmap, pixmap, mode,
                    Blitter::ResizeMode::NO_RESIZE,
                    Blitter::RenderMode::SMOOTH);
        }

    private:
        Pixmap& m_screenPixmap;
    };

    RenderEnumerator enumerator(screenPixmap);

    getListener()->enumerateVisibleWindows(enumerator);

    g_logger.trace("%s - complete", __func__);
}

void WaylandBackendShm::setCurrentSize(const Size& newSize,
                                       bool doRepaint)
{
    if ((m_currentSize.m_w == newSize.m_w)
            && (m_currentSize.m_h == newSize.m_h))
    {
        return;
    }

    g_logger.info("%s - w=%d h=%d", __func__, newSize.m_w, newSize.m_h);

    m_currentSize = newSize;

    if (doRepaint)
    {
        requestRender();
    }
}

const Size& WaylandBackendShm::getCurrentSize() const
{
    return m_currentSize;
}

} // namespace gfx
} // namespace subttxrend
