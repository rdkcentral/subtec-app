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

#include "ipp2/Logger.h"

#include "Pixmap.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{
DEFINE_DEFAULT_CAT("SUBS.GFX.GRAPHICS.QTZBEND");
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
    log_trace(" ");

    redraw();
}

void QuartzBackend::forceRender()
{
    log_trace(" ");

    redraw();
}

void QuartzBackend::redraw()
{
    log_trace(" ");

    auto contentSize = calculateContentSize();

    const bool anythingToDraw = (contentSize.m_w > 0) || (contentSize.m_h > 0);

    /* When rendering ensure background is Clear - with 0% opacity so that any underlying content
       is still visible. */

    // render windows
    if (anythingToDraw)
    {
        if (!prepareBuffer(contentSize))
        {
            log_error("cannot prepare buffer of size " << contentSize.m_w << "x" << contentSize.m_h);
            return;
        }

        if (!copyToBuffer(contentSize))
        {
            log_error("cannot copy contents");
            return;
        }

        log_trace("width=" << contentSize.m_w << " height=" << contentSize.m_h << " size=" << m_buffer.size() << " ptr=" <<  m_buffer.data());
        // Send the buffer to Quartz
        setSubtitleData(m_buffer.data(), m_buffer.size(), contentSize.m_w, contentSize.m_h);
    }
    else
    {
        log_trace(" nothing to draw");
        // Tell Quartz to mute the subtitles
        muteSubtitles();
    }
}

Size QuartzBackend::calculateContentSize()
{
    log_trace(" ");

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
    log_trace(" ");

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
                log_info("pixmap larger than image, skipping");
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
