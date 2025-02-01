/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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

#pragma once

#include <cstdint>

#include "Channel.hpp"

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/gfx/Window.hpp>

#include <subttxrend/webvttengine/WebvttEngine.hpp>
#include "ControllerInterface.hpp"
#include "StcProvider.hpp"

namespace subttxrend
{
namespace app
{

/**
 * Controller class for WebVTT IP VOD subtitles.
 */
class WebvttController final : public ControllerInterface
{
  public:
    /**
     * Constructor.
     */
    WebvttController(const protocol::PacketChannelSpecific& dataPacket, 
                   const common::ConfigProvider& config, 
                   gfx::WindowPtr const& gfxWindow);
    ~WebvttController();

    void process() override;
    void addData(const protocol::PacketData& dataPacket) override;

    void activate() override;
    void deactivate() override;
    
    std::chrono::milliseconds getWaitTime() const override;

    void processTimestamp(const protocol::Packet& dataPacket) override;
    void mute(bool muted) override;
    bool wantsData(protocol::PacketChannelSpecific const& packet) const override;

    void pause() override;
    void resume() override;

    void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet) override;

  private:
    bool wantsData(std::uint32_t channelId) const;
    void select(std::uint32_t channelId, std::uint32_t width, std::uint32_t height);
    void addData(const std::uint8_t* data, std::size_t dataLen, std::int64_t displayOffset);
    void processTimestamp(const std::uint64_t mediatime);

    void select(const protocol::PacketChannelSpecific& dataPacket);

  private:
    /** Current subtitle channel. */
    Channel m_channel;

    /** Logger object. */
    common::Logger m_logger;

    /** WebVTT decoder instance */
    std::unique_ptr<webvttengine::WebvttEngine> m_webvttEngine;
};

} // namespace app
} // namespace subttxrend
