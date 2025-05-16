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


#ifndef _SUBTTXREND_APP_TTXCONTROLLER_HPP_
#define _SUBTTXREND_APP_TTXCONTROLLER_HPP_

#include <cstdint>
#include <memory>

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/dbus/SubtitleStatus.hpp>
#include <subttxrend/dbus/TeletextStatus.hpp>
#include <subttxrend/ttxt/SubtitlesRenderer.hpp>
#include <subttxrend/ttxt/BrowserRenderer.hpp>
#include <subttxrend/ttxt/TimeSource.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include "ControllerInterface.hpp"

#include "Channel.hpp"

namespace subttxrend {
namespace common {
class ConfigProvider;
}

namespace ctrl {

using dbus::SubtitleStatus;
using dbus::TeletextStatus;

class StcProvider;

/**
 * Class controls teletext and teletext subtitles behavior.
 */
class TtxController final : public ControllerInterface, private ttxt::TimeSource
{
  public:
    /**
     * Constructor.
     */
    TtxController(protocol::PacketChannelSpecific const& packet,
                  common::ConfigProvider const& config,
                  const gfx::WindowPtr& gfxWindow,
                  const gfx::EnginePtr& gfxEngine,
                  StcProvider& stcProvider);

    ~TtxController();

    /**
     * Performs data processing.
     */
    void process() override;

    /**
     * Processes teletext data packet.
     *
     * @param data
     *      Buffer with packet data.
     * @param dataLen
     *      Length of the data in bytes.
     */
    void addData(protocol::PacketData const& data) override;

    void activate() override;
    void deactivate() override;

    void selectTeletext(std::uint32_t channelId, std::uint32_t initialMagazine, std::uint32_t initialPage);
    /**
     * Selects teletext subtitles for rendering.
     *
     * @param channelId
     *      Specifies channel on which data for subtitles is transmitted.
     * @param magazineNbr
     *      Magazine number.
     * @param pageNumber
     *      Page number.
     */
    void selectSubtitle(std::uint32_t channelId, std::uint32_t magazineNbr, std::uint32_t pageNumber);

    void mute(bool muted) override;
    /**
     * Sets teletext muted state.
     *
     * @param muted
     *      True if teletext should be muted, false otherwise.
     */
    void setMutedTeletext(bool muted);

    /**
     * Sets subtitle muted state.
     *
     * @param muted
     *      True if subtitles should be muted, false otherwise.
     */
    void setMutedSubtitles(bool muted);

    /**
     * Checks if instance is interested with data.
     *
     * @param channelType
     *      Indicates data type: 0 - DVB Subtitles, 1 - Teletext subtitles,
     *      2 - SCTE-27 subtitles.
     *
     * @return
     *      True if it want to process data, false otherwise.
     */
    bool wantsData(protocol::PacketChannelSpecific const& packet) const override;

    /**
     * Teletext subtitle channel getter.
     *
     * @return
     *      Subtitle channel.
     */
    Channel getSubtitleChannel() const
    {
        return m_subtitleChannel;
    }

    /**
     * Teletext channel getter.
     *
     * @return
     *      Teletext channel.
     */
    Channel getTeletextChannel() const
    {
        return m_teletextChannel;
    }

  private:

    /**
     * Checks if instance is interested with data for given channel id.
     *
     * @param channelId
     *      Channel id data is intended for.
     * @return
     *      True if it want to process data, false otherwise.
     */
    bool wantsData(std::uint32_t channelId) const;

    enum class Selected
    {
        none,
        teletext,
        subtitle
    };

    bool isTeletextActive() const;
    bool areSubtitlesActive() const;

    void select(protocol::PacketChannelSpecific const& packet);
    void start();
    void stop();
    void reset();

    void startTeletext();
    void stopTeletext();
    void startSubtitle();
    void stopSubtitle();
    /**
     * Returns current STC value.
     *
     * @return
     *      Current STC value (top 32-bits, 45kHz).
     */
    virtual std::uint32_t getStc() override;

    Selected m_selected{};

    /** Current subtitle status. */
    SubtitleStatus m_subtitleStatus;

    /** Current teletext status. */
    TeletextStatus m_teletextStatus;

    /** Current teletext channel. */
    Channel m_teletextChannel;

    /** Current subtitle channel. */
    Channel m_subtitleChannel;

    /** STC time provider. */
    StcProvider* m_stcProvider;

    /** Renderer instance. */
    std::unique_ptr<ttxt::BrowserRenderer> m_browserRenderer;

    /** Renderer instance. */
    std::unique_ptr<ttxt::SubtitlesRenderer> m_subtitleRenderer;

    /** Logger object. */
    common::Logger m_logger;
};

} // namespace ctrl
} // namespace subttxrend

#endif /* _SUBTTXREND_APP_TTXCONTROLLER_HPP_ */
