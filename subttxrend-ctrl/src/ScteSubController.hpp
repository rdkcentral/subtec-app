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


#ifndef _SUBTTXREND_APP_STCSUBCONTROLLER_HPP_
#define _SUBTTXREND_APP_STCSUBCONTROLLER_HPP_

#include <cstdint>
#include <memory>

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/dbus/SubtitleStatus.hpp>
#include <subttxrend/dvbsub/TimeSource.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/scte/ScteController.hpp>

#include "ControllerInterface.hpp"

namespace subttxrend
{
namespace ctrl
{

class StcProvider;

/**
 * Controller class for SCTE-27 subtitles.
 */
class ScteSubController final : public ControllerInterface 
{
public:

    /**
     * Constructor.
     */
    ScteSubController(const protocol::PacketChannelSpecific& dataPacket, gfx::WindowPtr const& gfxWindow, StcProvider& stcProvider);
    ~ScteSubController();
    /**
     * Performs data processing.
     */
    void process() override;
    void addData(const protocol::PacketData& dataPacket) override;
    void activate() override;
    void deactivate() override;

    /**
     * Sets muted state.
     *
     * @param muted
     *      True if subtitles should be muted, false otherwise.
     */
    void mute(bool muted) override;

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

private:
    bool wantsData(uint32_t channelType) const;

    void select(const protocol::PacketChannelSpecific& dataPacket);
    void start();

    /** STC time provider. */
    StcProvider* m_stcProvider;

    //TODO: Add renderer field
    /** Renderer instance. */
    scte::Controller m_renderer;

    /** Logger object. */
    common::Logger m_logger;
};

} // namespace ctrl
} // namespace subttxrend

#endif /* _SUBTTXREND_APP_SCTESUBCONTROLLER_HPP_ */
