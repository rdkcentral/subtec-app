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

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>

#include "ControllerInterface.hpp"
#include "Configuration.hpp"
#include "StcProvider.hpp"

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/AsClient.hpp>
#include <subttxrend/common/AsListener.hpp>
#include <subttxrend/common/WsEndpoint.hpp>

#include <subttxrend/socksrc/PacketReceiver.hpp>

#include <subttxrend/protocol/PacketParser.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketResetChannel.hpp>
#include <subttxrend/protocol/PacketResetAll.hpp>
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>
#include <subttxrend/protocol/PacketTeletextSelection.hpp>
#include <subttxrend/protocol/PacketTimestamp.hpp>
#include <subttxrend/protocol/PacketTtmlSelection.hpp>
#include <subttxrend/protocol/PacketTtmlTimestamp.hpp>
#include <subttxrend/protocol/PacketWebvttSelection.hpp>
#include <subttxrend/protocol/PacketWebvttTimestamp.hpp>
#include <subttxrend/protocol/PacketPause.hpp>
#include <subttxrend/protocol/PacketResume.hpp>
#include <subttxrend/protocol/PacketMute.hpp>
#include <subttxrend/protocol/PacketUnmute.hpp>
#include <subttxrend/protocol/PacketTtmlInfo.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>

#include <subttxrend/gfx/Engine.hpp>

namespace subttxrend {
namespace app {

/**
 * Class controlling subtitles and teletext behavior.
 *
 * It allows selection of teletext, teletext subtitles and dvb subtitles.
 * Implements logic for for what should be rendered. If multiple components
 * are selected (and not muted) the priorities are:
 * 1. teletext
 * 2. teletext subtitles
 * 3. dvb subtitles
 */
class Controller : private common::NonCopyable,
                   public socksrc::PacketReceiver
{
  public:
    /**
     * Constructor.
     */
    // clang-format off
    Controller(
            Configuration const& config,
            gfx::EnginePtr gfxEngine,
            gfx::WindowPtr gfxWindow,
            const std::string region);
    // clang-format on

    /**
     * Perform initialization.
     *
     * @param gfxWindow
     *      Window to use.
     * @param gfxEngine
     *      Graphics engine to use.
     * @param teletextConfig
     *      Teletext configuration provider.
     */
    void init(gfx::Window *gfxWindow,
              const gfx::EnginePtr &gfxEngine,
              const Configuration &config);

    void startAsync();

    /**
     * Performs shutdown.
     */
    void stop();

    /**
     * Valid packet received notification.
     *
     * This method is called by source if new valid packet is received.
     *
     * @param packet
     *      Packet received. The packet is only valid within notification.
     */
    virtual void onPacketReceived(const protocol::Packet& packet) override;

    /**
     * Enqueue data packet to be processed by another thread
     */
    void addBuffer(common::DataBufferPtr buffer) override;

    /**
     * Stream broken notification.
     *
     * This method is called by source if the packet stream is broken
     * (e.g. invalid packet was received).
     */
    virtual void onStreamBroken() override;

private:
    /**
     * Performs data processing.
     *
     * @return
     *      Amount of time until next action is required.
     */
    std::chrono::milliseconds processData();

    void doOnPacketReceived(std::unique_lock<std::mutex>& lock, const protocol::Packet& packet);

    void processSelection(const protocol::PacketChannelSpecific& packet);
    /**
     * Processes subtitle selection packet.
     *
     * @param packet
     *      Subtitle selection packet.
     */
    void processSubtitleSelection(const protocol::PacketSubtitleSelection& packet);

    /**
     * Performs DVB subtitle selection.
     *
     * @param packet
     *      Subtitle selection packet.
     */
    void processDvbSubtitleSelection(const protocol::PacketSubtitleSelection& packet);

    /**
     * Performs SCTE-27 subtitle selection.
     *
     * @param packet
     *      Subtitle selection packet.
     */
    void processScteSubtitleSelection(const protocol::PacketSubtitleSelection& packet);

    /**
     * Performs Closed Captions selection.
     *
     * @param packet
     *      Subtitle selection packet.
     */
    void processCcSubtitleSelection(const protocol::PacketSubtitleSelection& packet);

    /**
     * Performs teletext subtitle selection.
     *
     * @param packet
     *      Subtitle selection packet.
     */
    void processTtxSubtitleSelection(const protocol::PacketSubtitleSelection& packet);

    /**
     * Processes teletext selection packet.
     *
     * @param packet
     *      Teletext selection packet.
     */
    void processTeletextSelection(const protocol::PacketTeletextSelection& packet);

    /**
     * Processes TTML subtitles selection packet.
     *
     * @param packet
     *      TTML selection packet.
     */
    void processTtmlSelection(const protocol::PacketTtmlSelection& packet);
    
    /**
     * Processes WebVTT subtitles selection packet.
     *
     * @param packet
     *      WebVTT selection packet.
     */
    void processWebvttSelection(const protocol::PacketWebvttSelection& packet);


    /**
     * Processes subtitle reset channel packet.
     *
     * @param packet
     *      Subtitle reset packet.
     */
    void processResetChannel(std::unique_lock<std::mutex>& lock, const protocol::PacketResetChannel& packet);

    /**
     * Processes subtitle reset all packet.
     *
     * @param packet
     *      Subtitle reset packet.
     */
    void processResetAll(std::unique_lock<std::mutex>& lock, const protocol::PacketResetAll& packet);

    /**
     * Resets all channels.
     */
    void resetAll();

    /**
     * Processes  data packet.
     *
     * @param packet
     *      data packet (TTML, PES, CC).
     */
    void processDataPacket(const protocol::PacketData& packet);
    /**
     * Processes TTML timestamp.
     *
     * @param packet
     *      timestamp packet.
     */
    void processTtmlTimestamp(const protocol::PacketTtmlTimestamp& packet);

    /**
     * Processes ttml info packet.
     *
     * @param packet
     *      ttml info packet.
     */
    void processTtmlInfoPacket(const protocol::PacketTtmlInfo& packet);

    /**
     * Processes setCCAttributes packet.
     *
     * @param setCCAttributesPacket
     */
    void processSetCCAttributes(const protocol::PacketSetCCAttributes& setCCAttributesPacket);

    /**
     * Processes WebVTT timestamp.
     *
     * @param timestampPacket
     *      the timestamp packet.
     */
    void processWebvttTimestamp(const protocol::PacketWebvttTimestamp& timestampPacket);

    /**
     * Processes pause packet.
     *
     * @param packet
     *      pause packet.
     */
    void processPausePacket(const protocol::PacketChannelSpecific& packet);

    /**
     * Processes resume packet.
     *
     * @param packet
     *      resume packet.
     */
    void processResumePacket(const protocol::PacketChannelSpecific& packet);

    /**
     * Processes mute packet.
     *
     * @param packet
     *      mute packet.
     */
    void processMutePacket(const protocol::PacketMute& packet);

    /**
     * Processes unmute packet.
     *
     * @param packet
     *      unmute packet.
     */
    void processUnmutePacket(const protocol::PacketUnmute& packet);

    /**
     * Selects teletext for rendering.
     *
     * @param channelId
     *      Specifies channel on which data for subtitles is transmitted.
     * @param magazineNbr
     *      Magazine number.
     * @param pageNumber
     *      Page number.
     */
    void selectTeletextSubtitles(std::uint32_t channelId, std::uint32_t magazineNbr, std::uint32_t pageNumber);

    void deactivateController();
    void pushController(std::shared_ptr<ControllerInterface> controller);
    void popNullController();
    std::shared_ptr<ControllerInterface> topController();

    bool isRenderingActive() const;
    bool isDataQueued() const;

    void processLoop();

    Configuration const& m_config;

    gfx::EnginePtr m_gfxEngine;
    gfx::WindowPtr m_gfxWindow;

    std::string m_region;

    /** Processes timestamp messages and provides stc value. */
    StcProvider m_stcProvider;

    std::vector<std::shared_ptr<ControllerInterface>> m_activeControllers;

    /** Logger object. */
    common::Logger m_logger;

    /** Rendering thread. */
    std::thread m_renderThread;
    std::mutex m_renderMutex;
    std::condition_variable m_renderCond;
    bool m_quitRenderThread{false};

    using UniqueLock = std::unique_lock<std::mutex>;
    using LockGuard = std::lock_guard<std::mutex>;

    protocol::PacketParser m_parser;
    std::deque<common::DataBufferPtr> m_dataqueue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_inuse{false};
    std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;

    std::unique_ptr<common::AsListener> m_asLstnr;
    std::unique_ptr<common::WsEndpoint> m_endpoint;
    std::unique_ptr<common::AsClient> m_asClient;
};

using ControllerPtr = std::unique_ptr<Controller>;

} // namespace app
} // namespace subttxrend

