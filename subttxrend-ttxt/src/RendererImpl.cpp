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


#include "RendererImpl.hpp"

#include <subttxrend/common/Logger.hpp>
#include <ttxdecoder/EngineFactory.hpp>

#include "TimeSource.hpp"
#include "GfxRenderer.hpp"

namespace subttxrend
{
namespace ttxt
{

namespace
{

const std::size_t DECODER_MEMORY_SIZE = 256 * 1024;

common::Logger g_logger("Ttxt", "RendererImpl");

}

RendererImpl::RendererImpl(bool subtitlesRenderer) :
        GfxRendererClient(subtitlesRenderer),
        m_gfxRenderer(GfxRenderer::getSingleton()),
        m_isStarted(false),
        m_isMuted(false),
        m_renderHeader(false),
        m_renderPage(false),
        m_gfxWindow(nullptr),
        m_configProvider(nullptr),
        m_timeSource(nullptr)
{
    // noop
}

RendererImpl::~RendererImpl()
{
    // noop
}

const ttxdecoder::Engine* RendererImpl::getDataSource() const
{
    return m_decoderEngine.get();
}

const common::ConfigProvider* RendererImpl::getConfiguration() const
{
    return m_configProvider;
}

gfx::Window* RendererImpl::getGfxWindow() const
{
    return m_gfxWindow;
}


const gfx::EnginePtr& RendererImpl::getGfxEngine() const
{
    return m_gfxEngine;
}

bool RendererImpl::init(gfx::Window* gfxWindow,
                        const gfx::EnginePtr& gfxEngine,
                        const common::ConfigProvider* configProvider,
                        TimeSource* timeSource)
{
    g_logger.trace("%s", __func__);

    if (!gfxWindow)
    {
        throw std::invalid_argument("gfxWindow");
    }
    if (!gfxEngine)
    {
        throw std::invalid_argument("gfxEngine");
    }
    if (!configProvider)
    {
        throw std::invalid_argument("configProvider");
    }
    if (!timeSource)
    {
        throw std::invalid_argument("timeSource");
    }

    if (m_decoderEngine)
    {
        return false;
    }

    m_gfxWindow = gfxWindow;
    m_gfxEngine = gfxEngine;
    m_configProvider = configProvider;
    m_timeSource = timeSource;

    auto decoderAllocator = std::unique_ptr<ttxdecoder::Allocator>(
            new ttxdecoder::StandardAllocator(DECODER_MEMORY_SIZE));

    m_decoderEngine = std::move(
            ttxdecoder::EngineFactory::createEngine(*this,
                    std::move(decoderAllocator)));
    m_decoderEngine->setNavigationMode(
            ttxdecoder::NavigationMode::FLOF_TOP_DEFAULT);
    if (!isSubtitlesRenderer())
    {
        m_decoderEngine->setIgnorePts(true);
    }

    m_gfxRenderer.gfxInit(this);

    return true;
}

void RendererImpl::shutdown()
{
    g_logger.trace("%s", __func__);

    m_gfxRenderer.gfxShutdown();

    m_decoderEngine.reset();

    m_timeSource = nullptr;
    m_configProvider = nullptr;
    m_gfxEngine.reset();
}

void RendererImpl::processData()
{
    if (!m_isStarted)
    {
        return;
    }

    m_decoderEngine->process();

    if (m_isMuted)
    {
        return;
    }

    m_gfxRenderer.gfxDraw(this, m_renderHeader, m_renderPage);

    m_renderHeader = false;
    m_renderPage = false;
}

bool RendererImpl::addPesPacket(const void* buffer,
                                const std::uint16_t length)
{
    if (m_isStarted)
    {
        return m_decoderEngine->addPesPacket(
                reinterpret_cast<const std::uint8_t*>(buffer), length);
    }
    else
    {
        return false;
    }
}

void RendererImpl::setCurrentPage(const ttxdecoder::PageId& pageId)
{
    m_decoderEngine->setCurrentPageId(pageId);
}

bool RendererImpl::startInternal()
{
    g_logger.info("%s", __func__);

    if (m_isStarted)
    {
        stop();
    }

    m_decoderEngine->resetAcquisition();

    if (!m_isMuted)
    {
        // force complete redraw
        m_renderHeader = true;
        m_renderPage = true;

        m_gfxRenderer.gfxShow(this);
    }

    m_isStarted = true;

    return true;
}

bool RendererImpl::stop()
{
    g_logger.info("%s", __func__);

    if (m_isStarted)
    {
        if (!m_isMuted)
        {
            m_gfxRenderer.gfxHide(this);
        }

        m_isStarted = false;
    }

    return true;
}

bool RendererImpl::isStarted() const
{
    return m_isStarted;
}

void RendererImpl::mute()
{
    g_logger.info("%s", __func__);

    if (!m_isMuted)
    {
        m_isMuted = true;

        if (m_isStarted)
        {
            m_gfxRenderer.gfxHide(this);
        }
    }
}

void RendererImpl::unmute()
{
    g_logger.info("%s", __func__);

    if (m_isMuted)
    {
        m_isMuted = false;

        if (m_isStarted)
        {
            // force complete redraw
            m_renderHeader = true;
            m_renderPage = true;

            // clear & show the screen
            m_gfxRenderer.gfxShow(this);
        }
    }
}

bool RendererImpl::isMuted() const
{
    return m_isMuted;
}

void RendererImpl::pageReady()
{
    if (m_isStarted && !m_isMuted)
    {
        m_renderPage = true;
    }
}

void RendererImpl::headerReady()
{
    if (m_isStarted && !m_isMuted)
    {
        m_renderHeader = true;
    }
}

void RendererImpl::drcsCharDecoded(unsigned char /*index*/,
                                   unsigned char* /*data*/)
{
    // noop
}

bool RendererImpl::getStc(std::uint32_t& stc)
{
    if (m_timeSource)
    {
        stc = m_timeSource->getStc();
        return true;
    }
    else
    {
        stc = 0;
        return false;
    }
}

} // namespace ttxt
} // namespace subttxrend
