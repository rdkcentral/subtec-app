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


#include <subttxrend/protocol/PacketSubtitleSelection.hpp>

#include "ScteSubController.hpp"
#include "StcProvider.hpp"

namespace subttxrend {
namespace app {

ScteSubController::ScteSubController(const protocol::PacketChannelSpecific& packet,
                                     gfx::WindowPtr const& gfxWindow,
                                     StcProvider& stcProvider)
    : m_stcProvider(&stcProvider)
    , m_renderer()
    , m_logger("App", "ScteSubController")
{
    m_logger.info("%s created", __LOGGER_FUNC__);
    m_renderer.init(gfxWindow.get(), &stcProvider);
    select(packet);
    start();
}

ScteSubController::~ScteSubController()
{
    if (m_renderer.isStarted()) {
        m_renderer.stop();
    }
    m_renderer.shutdown();
}

void ScteSubController::process()
{
    m_renderer.process();
}

void ScteSubController::addData(const protocol::PacketData& dataPacket)
{
    m_logger.info("%s Processing section data - buffer: %p size: %zu ",
                  __LOGGER_FUNC__,
                  dataPacket.getData(),
                  dataPacket.getDataSize());
    m_renderer.addData(dataPacket);
}

void ScteSubController::activate()
{
    m_renderer.start();
}
void ScteSubController::deactivate()
{
    if (m_renderer.isStarted()) {
        m_renderer.stop();
    }
}

void ScteSubController::select(const protocol::PacketChannelSpecific&)
{
    m_logger.info("%s", __LOGGER_FUNC__);
}

void ScteSubController::start()
{
    m_logger.info("%s", __LOGGER_FUNC__);

    if (m_renderer.start()) {
        m_logger.trace("%s", __LOGGER_FUNC__);
    } else {
        m_logger.error("%s start failed", __LOGGER_FUNC__);
    }
}

void ScteSubController::mute(bool muted)
{
    m_logger.info("%s (%d)", __LOGGER_FUNC__, muted);

    if (muted) {
        m_renderer.mute();
    } else {
        m_renderer.unmute();
    }
}

bool ScteSubController::wantsData(protocol::PacketChannelSpecific const& packet) const
{
    auto const& data = static_cast<protocol::PacketData const&>(packet);
    return wantsData(data.getChannelType());
}

bool ScteSubController::wantsData(uint32_t channelType) const
{
    if (channelType == protocol::PacketSubtitleSelection::SUBTITLES_TYPE_SCTE) {
        // SCTE-27 subtitles
        return true;
    }
    return false;
}

} // namespace app
} // namespace subttxrend

