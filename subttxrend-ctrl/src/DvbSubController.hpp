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


#ifndef _SUBTTXREND_APP_DVBSUBCONTROLLER_HPP_
#define _SUBTTXREND_APP_DVBSUBCONTROLLER_HPP_

#include <cstdint>
#include <memory>

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/dbus/SubtitleStatus.hpp>
#include <subttxrend/dvbsub/SubtitlesRenderer.hpp>
#include <subttxrend/dvbsub/TimeSource.hpp>
#include <subttxrend/gfx/Engine.hpp>

#include "ControllerInterface.hpp"
#include "Channel.hpp"

namespace subttxrend {
namespace app {

class StcProvider;

/**
 * Controller class for DVB subtitles.
 */
class DvbSubController final : public ControllerInterface, private dvbsub::TimeSource
{
  public:
    /**
     * Constructor.
     */
    DvbSubController(const protocol::PacketChannelSpecific& dataPacket,
                     const gfx::WindowPtr& gfxWindow,
                     const gfx::EnginePtr& gfxEngine,
                     StcProvider& stcProvider);

    /**
     * Destructor.
     */
    ~DvbSubController();

    /**
     * Performs data processing.
     */
    void process() override;

    /**
     * Processes DVB subtitles data packet.
     *
     * @param bufferStart
     *      Start of data buffer.
     * @param dataSize
     *      Size of data buffer.
     */
    void addData(protocol::PacketData const& packet) override;


    void activate() override;
    void deactivate() override;
    /**
     * Sets muted state.
     *
     * @param muted
     *      True if subtitles should be muted, false otherwise.
     */
    void mute(bool mute) override;

    /**
     * Checks if instance is interested with data for given channel id.
     *
     * @param channelId
     *      Channel id data is intended for.
     * @return
     *      True if it want to process data, false otherwise.
     */
    bool wantsData(protocol::PacketChannelSpecific const& packet) const override;

  private:
    void addData(const std::uint8_t* bufferStart, std::size_t dataSize);
    void select(std::uint32_t channelId, std::uint32_t compositionPageId, std::uint32_t ancillaryPageId);
    void select(const protocol::PacketChannelSpecific& dataPacket);
    void start();

    /**
     * Returns current STC value.
     *
     * @return
     *      Current STC value (top 32-bits, 45kHz).
     */
    virtual std::uint32_t getStc() override;

    /** Active subtitle channel. */
    Channel m_channel;

    /** Subtitle status data. */
    dbus::SubtitleStatus m_status;

    /** STC time provider. */
    StcProvider* m_stcProvider;

    /** Renderer instance. */
    std::unique_ptr<dvbsub::SubtitlesRenderer> m_renderer;

    /** Logger object. */
    common::Logger m_logger;
};

} // namespace app
} // namespace subttxrend

#endif /* _SUBTTXREND_APP_DVBSUBCONTROLLER_HPP_ */
