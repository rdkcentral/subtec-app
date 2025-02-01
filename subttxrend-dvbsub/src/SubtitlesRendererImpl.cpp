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


#include "SubtitlesRendererImpl.hpp"

#include <stdexcept>
#include <subttxrend/common/Logger.hpp>

#include "TimeSource.hpp"

namespace subttxrend
{
namespace dvbsub
{

namespace
{

common::Logger g_logger("DvbSub", "SubtitlesRendererImpl");

}

SubtitlesRendererImpl::SubtitlesRendererImpl() :
        m_isStarted(false),
        m_isMuted(false)
{
    g_logger.trace("%s", __func__);
}

SubtitlesRendererImpl::~SubtitlesRendererImpl() noexcept
{
    g_logger.trace("%s", __func__);
}

bool SubtitlesRendererImpl::init(gfx::Window* gfxWindow,
                                 TimeSource* timeSource)
{
    g_logger.trace("%s", __func__);

    if (m_decoderInstance)
    {
        g_logger.warning("%s - already initialized", __func__);
        return false;
    }

    m_timeProvider.init(timeSource);

    m_decoderInstance = std::move(
            dvbsubdecoder::DecoderFactory::create(
                    dvbsubdecoder::Specification::VERSION_1_3_1,
                    m_decoderAllocator, m_gfxRenderer.getDecoderClient(),
                    m_timeProvider));

    m_gfxRenderer.gfxInit(gfxWindow);

    g_logger.trace("%s - done", __func__);

    return true;
}

void SubtitlesRendererImpl::shutdown()
{
    g_logger.trace("%s", __func__);

    if (!m_decoderInstance)
    {
        g_logger.warning("%s - not initialized", __func__);
        return;
    }

    m_gfxRenderer.gfxShutdown();

    m_decoderInstance.reset();

    g_logger.trace("%s - done", __func__);
}

bool SubtitlesRendererImpl::addPesPacket(const void* buffer,
                                         const std::uint16_t length)
{
    if (m_isStarted)
    {
        g_logger.trace("%s", __func__);

        return m_decoderInstance->addPesPacket(
                reinterpret_cast<const std::uint8_t*>(buffer), length);
    }
    else
    {
        g_logger.trace("%s - skipping, not started", __func__);

        return false;
    }
}

bool SubtitlesRendererImpl::start(std::uint16_t compositionPageId,
                                  std::uint16_t ancillaryPageId)
{
    g_logger.info("%s - cp=%u ap=%u", __func__, compositionPageId,
            ancillaryPageId);

    if (m_isStarted)
    {
        stop();
    }

    m_decoderInstance->reset();

    m_decoderInstance->setPageIds(compositionPageId, ancillaryPageId);
    m_decoderInstance->start();

    if (!m_isMuted)
    {
        m_gfxRenderer.gfxShow();
    }

    m_isStarted = true;

    g_logger.trace("%s - done", __func__);

    return true;
}

bool SubtitlesRendererImpl::stop()
{
    g_logger.info("%s", __func__);

    if (m_isStarted)
    {
        m_decoderInstance->stop();

        m_gfxRenderer.gfxHide();

        m_isStarted = false;
    }

    g_logger.trace("%s - done", __func__);

    return true;
}

bool SubtitlesRendererImpl::isStarted() const
{
    return m_isStarted;
}

void SubtitlesRendererImpl::mute()
{
    g_logger.info("%s", __func__);

    if (!m_isMuted)
    {
        m_isMuted = true;

        if (m_isStarted)
        {
            m_gfxRenderer.gfxHide();
        }
    }

    g_logger.trace("%s - done", __func__);
}

void SubtitlesRendererImpl::unmute()
{
    g_logger.info("%s", __func__);

    if (m_isMuted)
    {
        m_isMuted = false;

        if (m_isStarted)
        {
            // clear & show the screen
            m_gfxRenderer.gfxShow();

            // force complete redraw
            m_decoderInstance->invalidate();
        }
    }

    g_logger.trace("%s - done", __func__);
}

bool SubtitlesRendererImpl::isMuted() const
{
    return m_isMuted;
}

void SubtitlesRendererImpl::processData()
{
    g_logger.trace("%s", __func__);

    if (!m_isStarted)
    {
        g_logger.trace("%s - not started", __func__);
        return;
    }

    if (!m_decoderInstance->process())
    {
        g_logger.trace("%s - nothing processed", __func__);
        return;
    }

    if (m_isMuted)
    {
        g_logger.trace("%s - muted", __func__);
        return;
    }

    g_logger.trace("%s - drawing", __func__);

    m_decoderInstance->draw();
}

} // namespace dvbsub
} // namespace subttxrend
