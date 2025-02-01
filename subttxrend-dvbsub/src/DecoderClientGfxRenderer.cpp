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


#include "DecoderClientGfxRenderer.hpp"

#include <stdexcept>
#include <chrono>
#include <subttxrend/common/Logger.hpp>

#include "TimeSource.hpp"

namespace subttxrend
{
namespace dvbsub
{

namespace
{

common::Logger g_logger("DvbSub", "DecoderClientGfxRenderer");

gfx::Rectangle convert(const dvbsubdecoder::Rectangle& srcRect)
{
    return gfx::Rectangle
    { srcRect.m_x1, srcRect.m_y1, srcRect.m_x2 - srcRect.m_x1, srcRect.m_y2
            - srcRect.m_y1 };
}

gfx::ClutBitmap convert(const dvbsubdecoder::Bitmap& srcBitmap)
{
    return gfx::ClutBitmap(srcBitmap.m_width, srcBitmap.m_height,
            srcBitmap.m_width, srcBitmap.m_pixels, srcBitmap.m_clut, 256);
}

}

DecoderClientGfxRenderer::DecoderClientGfxRenderer() :
        m_gfxWindow(nullptr)
{
    // noop
}

DecoderClientGfxRenderer::~DecoderClientGfxRenderer()
{
    // noop
}

dvbsubdecoder::DecoderClient& DecoderClientGfxRenderer::getDecoderClient()
{
    return *this;
}

void DecoderClientGfxRenderer::gfxInit(gfx::Window* gfxWindow)
{
    g_logger.trace("%s (window=%p)", __func__, gfxWindow);

    if (!gfxWindow)
    {
        throw std::invalid_argument("gfxWindow");
    }
    m_gfxWindow = gfxWindow;

    g_logger.info("%s - done", __func__);
}

void DecoderClientGfxRenderer::gfxShutdown()
{
    g_logger.trace("%s", __func__);
}

void DecoderClientGfxRenderer::gfxShow()
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        m_gfxWindow->setSize(gfx::Size
        { 0, 0 });
        m_gfxWindow->setVisible(true);
        m_gfxWindow->update();
    }
}

void DecoderClientGfxRenderer::gfxHide()
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        m_gfxWindow->setSize(gfx::Size
        { 0, 0 });
        m_gfxWindow->setVisible(false);
        m_gfxWindow->update();
    }
}

void DecoderClientGfxRenderer::gfxSetDisplayBounds(const dvbsubdecoder::Rectangle& displayBounds,
                                                   const dvbsubdecoder::Rectangle& windowBounds)
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        m_gfxWindow->setSize(gfx::Size
        { displayBounds.m_x2, displayBounds.m_y2 });

        auto& drawContext = m_gfxWindow->getDrawContext();
        drawContext.fillRectangle(gfx::ColorArgb::TRANSPARENT,
                m_gfxWindow->getBounds());
        // no update - will be done in gfxFinish
    }
}

void DecoderClientGfxRenderer::gfxDraw(const dvbsubdecoder::Bitmap& bitmap,
                                       const dvbsubdecoder::Rectangle& srcRect,
                                       const dvbsubdecoder::Rectangle& dstRect)
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        auto& drawContext = m_gfxWindow->getDrawContext();
        drawContext.drawPixmap(convert(bitmap), convert(srcRect),
                convert(dstRect));
        // no update - will be done in gfxFinish
    }
}

void DecoderClientGfxRenderer::gfxClear(const dvbsubdecoder::Rectangle& rect)
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        auto& drawContext = m_gfxWindow->getDrawContext();
        drawContext.fillRectangle(gfx::ColorArgb::TRANSPARENT, convert(rect));
        // no update - will be done in gfxFinish
    }
}

void DecoderClientGfxRenderer::gfxFinish(const dvbsubdecoder::Rectangle& rect)
{
    g_logger.trace("%s", __func__);

    if (m_gfxWindow)
    {
        m_gfxWindow->update();
    }
}

void* DecoderClientGfxRenderer::gfxAllocate(std::uint32_t size)
{
    g_logger.trace("%s", __func__);

    return ::operator new(size, std::nothrow);
}

void DecoderClientGfxRenderer::gfxFree(void* block)
{
    g_logger.trace("%s", __func__);

    return ::operator delete(block);
}

} // namespace dvbsub
} // namespace subttxrend
