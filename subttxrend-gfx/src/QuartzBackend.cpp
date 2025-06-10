/**
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 */
#include "QuartzBackend.hpp"

#ifdef __APPLE__
#import <cocoa_window.h>
#endif

#include <subttxrend/common/Logger.hpp>

#include "Pixmap.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{
subttxrend::common::Logger g_logger("Gfx", "QuartzBackend");
} // namespace <anonymous>

//------------------------------------------

const Size QuartzBackend::DEFAULT_SIZE(854, 480);

QuartzBackend::QuartzBackend(BackendListener* listener) :
        Backend(listener)
{
}

QuartzBackend::~QuartzBackend()
{
}

bool QuartzBackend::isSyncNeeded() const
{
    return true;
}

bool QuartzBackend::init()
{

    if ((m_initialSize.m_w == 0) && (m_initialSize.m_h == 0))
    {
        m_initialSize = DEFAULT_SIZE;
    }

    return true;
}

bool QuartzBackend::start()
{
    return true;
}

void QuartzBackend::startBlockingApplicationWindow()
{
    createAndRunCocoaWindow(DEFAULT_SIZE.m_w, DEFAULT_SIZE.m_h);
}


void QuartzBackend::stop()
{
}

void QuartzBackend::requestRender()
{
    g_logger.trace("%s", __func__);

    redraw();
}

void QuartzBackend::forceRender()
{
    g_logger.trace("%s", __func__);

    redraw();
}

void QuartzBackend::redraw()
{
    g_logger.trace("%s", __func__);

    auto contentSize = calculateContentSize();

    const bool anythingToDraw = (contentSize.m_w > 0) || (contentSize.m_h > 0);

    /* When rendering ensure background is Clear - with 0% opacity so that any underlying content
       is still visible. */

    // render windows
    if (anythingToDraw)
    {
        if (!prepareBuffer(contentSize))
        {
            g_logger.fatal("%s - cannot prepare buffer of size %dx%d", __func__, contentSize.m_w, contentSize.m_h);
            return;
        }

        if (!copyToBuffer(contentSize))
        {
            g_logger.fatal("%s - cannot copy contents", __func__);
            return;
        }

		g_logger.trace("%s - width=%d height=%d size=%d ptr=%X", __func__, contentSize.m_w, contentSize.m_h, m_buffer.size(), m_buffer.data());
        // Send the buffer to Quartz
        setSubtitleData(m_buffer.data(), m_buffer.size(), contentSize.m_w, contentSize.m_h);
    }
    else
    {
        g_logger.trace("%s nothing to draw", __func__);
        // Tell Quartz to mute the subtitles
        muteSubtitles();
    }
}

Size QuartzBackend::calculateContentSize()
{
    g_logger.trace("%s", __func__);

    class SizeCalculator : public BackendWindowEnumerator
    {
    public:
        virtual void processWindow(const Pixmap& pixmap) override
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

bool QuartzBackend::prepareBuffer(const Size& contentSize)
{
    size_t cap = contentSize.m_w * contentSize.m_h * 4;

    if (cap != m_buffer.size())
    {
        m_buffer.resize(cap, 0x00);
    }

    return true;
}

bool QuartzBackend::copyToBuffer(const Size& contentSize)
{
    g_logger.trace("%s", __func__);

    class BitmapCopier : public BackendWindowEnumerator
    {
    public:
        BitmapCopier(const Size& imageSize, std::vector<uint8_t>& buffer) :
                m_imageSize(imageSize),
                m_buffer(buffer)
        {
        }

        virtual void processWindow(const Pixmap& pixmap) override
        {
            const auto pw = pixmap.getWidth();
            const auto ph = pixmap.getHeight();

            if ((pw > m_imageSize.m_w) || (ph > m_imageSize.m_h))
            {
                g_logger.info("%s - pixmap larger than image, skipping", __func__);
                return;
            }

            // copy the bitmap data
            std::vector<uint8_t>::iterator it = m_buffer.begin();
            for (int y = 0; y < ph; y++)
            {
                auto linePtr = pixmap.getLine(y).ptr();

                /* The in memory ordering of the lines is correct for Linux but MacOs' origin is in the vertically opposite
                corner. We don't have to worry about it here because we can use the texture rendering to flip the origin. */
                std::copy((uint8_t*)linePtr, (uint8_t*)linePtr + pw * 4, it);
                it += (pw * 4);
            }
        }

    private:
        const Size m_imageSize;
        std::vector<uint8_t>& m_buffer;
    };

    if (m_clearArrayBuffer.size() != (contentSize.m_w * contentSize.m_h * 4))
    {
        m_clearArrayBuffer.resize(contentSize.m_w * contentSize.m_h * 4, 0x00);
    }

    // clear the buffer
    m_buffer = m_clearArrayBuffer;

    BitmapCopier copier(contentSize, m_buffer);

    getListener()->enumerateVisibleWindows(copier);

    return true;
}

} // namespace gfx
} // namespace subttxrend
