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


#ifndef SUBTTXREND_TTXT_RENDERER_IMPL_HPP_
#define SUBTTXREND_TTXT_RENDERER_IMPL_HPP_

#include <memory>

#include <ttxdecoder/Engine.hpp>
#include <ttxdecoder/EngineClient.hpp>

#include "Renderer.hpp"
#include "GfxRendererClient.hpp"

namespace subttxrend
{
namespace ttxt
{

class TimeSource;
class GfxRenderer;
class GfxRendererClient;

/**
 * Renderer implementation.
 */
class RendererImpl : virtual public Renderer,
                     private ttxdecoder::EngineClient,
                     private GfxRendererClient
{
public:
    /**
     * Constructor.
     *
     * @param subtitlesRenderer
     *      Subtitles renderer flag.
     */
    RendererImpl(bool subtitlesRenderer);

    /**
     * Destructor.
     */
    virtual ~RendererImpl();

    virtual bool init(gfx::Window* gfxWindow,
                      const gfx::EnginePtr& gfxEngine,
                      const common::ConfigProvider* configProvider,
                      TimeSource* timeSource) override;

    virtual void shutdown() override;

    virtual void processData() override;

    virtual bool addPesPacket(const void* buffer,
                              const std::uint16_t length) override;

    virtual void setCurrentPage(const ttxdecoder::PageId& pageId) override;

    virtual bool stop() override;

    virtual bool isStarted() const override;

    virtual void mute() override;

    virtual void unmute() override;

    virtual bool isMuted() const override;

protected:
    /**
     * Starts rendering.
     *
     * @return
     *      True on success, false on error.
     */
    bool startInternal();

private:
    virtual const ttxdecoder::Engine* getDataSource() const override;

    virtual const common::ConfigProvider* getConfiguration() const override;

    virtual gfx::Window* getGfxWindow() const override;

    virtual const gfx::EnginePtr& getGfxEngine() const override;

    /**
     * Called when new page data is ready.
     */
    virtual void pageReady() override;

    /**
     * Called when new header data is ready.
     */
    virtual void headerReady() override;

    /**
     * Called when DRCS char was decoded.
     *
     * @param index
     *      Character index.
     * @param data
     *      Character data.
     */
    virtual void drcsCharDecoded(unsigned char index,
                                 unsigned char* data) override;

    /**
     * Called to get current STC value.
     *
     * @param stc
     *      Variable to store the STC value.
     *
     * @retval true
     *      Value was available,
     * @retval false
     *      Value cannot be returned.
     */
    virtual bool getStc(std::uint32_t& stc) override;

    /** Graphics renderer. */
    GfxRenderer& m_gfxRenderer;

    /** Renderer started flag. */
    bool m_isStarted;

    /** Renderer muted flag. */
    bool m_isMuted;

    /** Render header flag. */
    bool m_renderHeader;

    /** Render page flag. */
    bool m_renderPage;

    /** Window to draw on. */
    gfx::Window* m_gfxWindow;

    /** Graphics engine to use. */
    gfx::EnginePtr m_gfxEngine;

    /** Configuration provider. */
    const common::ConfigProvider* m_configProvider;

    /** Time source. */
    TimeSource* m_timeSource;

    /** Teletext decoder engine. */
    std::unique_ptr<ttxdecoder::Engine> m_decoderEngine;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_RENDERER_IMPL_HPP_*/
