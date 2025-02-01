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


#ifndef SUBTTXREND_DVBSUB_SUBTITLES_RENDERER_IMPL_HPP_
#define SUBTTXREND_DVBSUB_SUBTITLES_RENDERER_IMPL_HPP_

#include "SubtitlesRenderer.hpp"

#include <dvbsubdecoder/DecoderFactory.hpp>
#include <dvbsubdecoder/DecoderClient.hpp>
#include <dvbsubdecoder/DynamicAllocator.hpp>

#include "DecoderTimeProvider.hpp"
#include "DecoderClientGfxRenderer.hpp"

namespace subttxrend
{
namespace dvbsub
{

class TimeSource;

/**
 * Renderer for DVB Subtitles - implementation.
 */
class SubtitlesRendererImpl : public SubtitlesRenderer
{
public:
    /**
     * Constructor.
     */
    SubtitlesRendererImpl();

    /**
     * Destructor.
     */
    virtual ~SubtitlesRendererImpl() noexcept;

    virtual bool init(gfx::Window* gfxWindow,
                      TimeSource* timeSource) override;

    virtual void shutdown() override;

    virtual void processData() override;

    virtual bool addPesPacket(const void* buffer,
                              const std::uint16_t length) override;

    virtual bool start(std::uint16_t compositionPageId,
                       std::uint16_t ancillaryPageId) override;

    virtual bool stop() override;

    virtual bool isStarted() const override;

    virtual void mute() override;

    virtual void unmute() override;

    virtual bool isMuted() const override;

public:
    /** Renderer started flag. */
    bool m_isStarted;

    /** Renderer muted flag. */
    bool m_isMuted;

    /** Time source to use. */
    DecoderTimeProvider m_timeProvider;

    /** Gfx renderer. */
    DecoderClientGfxRenderer m_gfxRenderer;

    /** Decoder allocator. */
    dvbsubdecoder::DynamicAllocator m_decoderAllocator;

    /** Decoder. */
    dvbsubdecoder::DecoderPtr m_decoderInstance;
};

} // namespace dvbsub
} // namespace subttxrend

#endif /*SUBTTXREND_DVBSUB_SUBTITLES_RENDERER_IMPL_HPP_*/
