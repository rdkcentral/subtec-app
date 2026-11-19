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

#include <algorithm>

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

/**
 * Converts pixmap contents from straight to premultiplied alpha in place.
 *
 * @param pixmap
 *      Pixmap to convert.
 */
void premultiplyAlpha(Pixmap& pixmap)
{
    const auto width = pixmap.getWidth();
    const auto height = pixmap.getHeight();

    for (std::int32_t y = 0; y < height; ++y)
    {
        auto line = pixmap.getLine(y);

        for (std::int32_t x = 0; x < width; ++x, ++line)
        {
            auto pixel = *line;

            if (pixel.m_a == 0) {
                *line = PixelArgb(TRANSPARENT_BLACK);
            }
            else if (pixel.m_a != 255)
            {
                pixel.m_r = static_cast<std::uint8_t>((pixel.m_r * pixel.m_a) / 255);
                pixel.m_g = static_cast<std::uint8_t>((pixel.m_g * pixel.m_a) / 255);
                pixel.m_b = static_cast<std::uint8_t>((pixel.m_b * pixel.m_a) / 255);

                *line = pixel;
            }
        }
    }
}

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
    const auto background = getListener()->getBackgroundImage();
    const auto contentSize = calculateContentSize();

    // the background image is only drawn together with visible windows, so it is
    // hidden while subtitles are muted or stopped
    const bool windowsVisible = (contentSize.m_w > 0) || (contentSize.m_h > 0);

    if (g_resizeMode == ResizeMode::MAX_WINDOW_SIZE)
    {
        setCurrentSize(contentSize, false);
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
        paintPixels(buffer, background, windowsVisible);
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

void WaylandBackendShm::paintPixels(const WaylandBuffer::Ptr& buffer,
                                    const BackgroundImage& background,
                                    bool windowsVisible)
{
    auto params = buffer->getParams();

    g_logger.trace("%s - w=%d h=%d", __func__, params.m_width, params.m_height);

    Pixmap screenPixmap(reinterpret_cast<uint8_t*>(buffer->getDataPtr()),
            params.m_width, params.m_height, params.m_stride);

    Blitter::clear(screenPixmap);

    const bool hasBackground = windowsVisible && drawBackgroundImage(screenPixmap, background);

    g_logger.trace("%s - rendering windows", __func__);

    class RenderEnumerator : public BackendWindowEnumerator
    {
    public:
        RenderEnumerator(Pixmap& screenPixmap, bool blend) :
                m_screenPixmap(screenPixmap),
                m_blend(blend)
        {
            // noop
        }

        virtual void processWindow(const Pixmap& pixmap) override
        {
            g_logger.trace("%s - pixmap=%p size=%dx%d", __func__, &pixmap,
                    pixmap.getWidth(), pixmap.getHeight());

            if (m_blend)
            {
                const Rectangle rect{0, 0,
                        std::min(pixmap.getWidth(), m_screenPixmap.getWidth()),
                        std::min(pixmap.getHeight(), m_screenPixmap.getHeight())};

                Blitter::writeWithBlend(m_screenPixmap, pixmap, rect, rect);
                return;
            }

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
        const bool m_blend;
    };

    RenderEnumerator enumerator(screenPixmap, hasBackground);

    getListener()->enumerateVisibleWindows(enumerator);

    // window pixmaps use straight alpha, wl_shm ARGB8888 is premultiplied
    premultiplyAlpha(screenPixmap);

    g_logger.trace("%s - complete", __func__);
}

bool WaylandBackendShm::drawBackgroundImage(Pixmap& screenPixmap,
                                            const BackgroundImage& background)
{
    if (!background.m_image)
    {
        m_scaledBackground.clear();
        m_scaledBackground.shrink_to_fit();
        m_scaledBackgroundSize = Size{0, 0};
        return false;
    }

    const auto& image = *background.m_image;
    const Size screenSize{screenPixmap.getWidth(), screenPixmap.getHeight()};
    const auto scaledStride = static_cast<std::uint32_t>(screenSize.m_w * sizeof(PixelArgb8888));

    // scaling is expensive, so it is only redone when the image or the buffer size changes
    if ((background.m_generation != m_scaledBackgroundGeneration)
            || (screenSize != m_scaledBackgroundSize))
    {
        g_logger.info("%s - scaling background image %dx%d to %dx%d", __func__,
                image.m_width, image.m_height, screenSize.m_w, screenSize.m_h);

        m_scaledBackground.assign(static_cast<std::size_t>(scaledStride) * screenSize.m_h, 0);

        const Pixmap imagePixmap(const_cast<std::uint8_t*>(image.m_buffer.data()),
                image.m_width, image.m_height, image.m_stride);
        Pixmap scaledPixmap(m_scaledBackground.data(), screenSize.m_w,
                screenSize.m_h, scaledStride);

        Blitter::write(scaledPixmap, imagePixmap,
                Rectangle{0, 0, image.m_width, image.m_height},
                Rectangle{0, 0, screenSize.m_w, screenSize.m_h});

        m_scaledBackgroundGeneration = background.m_generation;
        m_scaledBackgroundSize = screenSize;
    }

    const Pixmap scaledPixmap(m_scaledBackground.data(), screenSize.m_w,
            screenSize.m_h, scaledStride);
    Blitter::write(screenPixmap, scaledPixmap);

    return true;
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
