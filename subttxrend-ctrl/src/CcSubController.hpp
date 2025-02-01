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


#ifndef _SUBTTXREND_APP_CCSUBCONTROLLER_HPP_
#define _SUBTTXREND_APP_CCSUBCONTROLLER_HPP_

#include <cstdint>
#include <memory>

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/dbus/SubtitleStatus.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/cc/CcController.hpp>
#include "ControllerInterface.hpp"

namespace subttxrend {
namespace app {

/**
 * Controller class for Closed Captions.
 */
class CcSubController final : public ControllerInterface
{
  public:
    /**
     * Constructor.
     */
    CcSubController(protocol::PacketChannelSpecific const& packet, gfx::WindowPtr const& gfxWindow, std::shared_ptr<gfx::PrerenderedFontCache> fontCache);
    ~CcSubController();

    void process() override;
    void addData(const protocol::PacketData& dataPacket) override;
    void activate() override;
    void deactivate() override;
    void mute(bool muted) override;
    bool wantsData(protocol::PacketChannelSpecific const& packet) const override;
    void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet) override;
    void setTextForPreview(const std::string& text);

  private:

    void select(protocol::PacketChannelSpecific const& packet);
    void select(cc::CeaType ceaType, std::uint32_t serviceNo);

  private:

    cc::Controller m_controller;

    std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;

    /** Logger object. */
    common::Logger m_logger;

    std::uint32_t m_channelId{};
};

} // namespace app
} // namespace subttxrend

#endif /* _SUBTTXREND_APP_SCTESUBCONTROLLER_HPP_ */
