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
#include "CcSubController.hpp"
#include <subttxrend/protocol/PacketData.hpp>

#include <cassert>

namespace subttxrend {
namespace app {

CcSubController::~CcSubController()
{
    if (m_controller.isStarted()) {
        m_controller.stop();
    }
    m_controller.shutdown();
}

void CcSubController::select(cc::CeaType ceaType, std::uint32_t serviceNo)
{
    m_logger.osinfo(__LOGGER_FUNC__);
    m_controller.setActiveService(ceaType, serviceNo);
    if (!m_controller.isStarted()) {
        m_controller.start();
    }
    m_logger.osinfo(__LOGGER_FUNC__, " - done");
}

void CcSubController::select(protocol::PacketChannelSpecific const& packet)
{
    auto const& ccsel = static_cast<protocol::PacketSubtitleSelection const&>(packet);
    assert((ccsel.getSubtitlesType() == protocol::PacketSubtitleSelection::SUBTITLES_TYPE_CC) and "invalid subtitle type");
    select(static_cast<cc::CeaType>(ccsel.getAuxId1()), ccsel.getAuxId2());
    m_channelId = packet.getChannelId();
}

CcSubController::CcSubController(protocol::PacketChannelSpecific const& packet, gfx::WindowPtr const& gfxWindow, std::shared_ptr<gfx::PrerenderedFontCache> fontCache)
    : m_controller()
    , m_logger("App", "CcSubController")
    , m_fontCache{fontCache}
{
    m_logger.oswarning(__LOGGER_FUNC__, " created");
    if (!m_controller.init(gfxWindow.get(), m_fontCache)) {
        m_logger.oserror(__LOGGER_FUNC__, " - init failed");
    }
    select(packet);
}

void CcSubController::process()
{
    m_logger.ostrace(__LOGGER_FUNC__);
    m_controller.process();
    m_logger.ostrace(__LOGGER_FUNC__, " - done");
}

void CcSubController::addData(const protocol::PacketData& dataPacket)
{
    m_logger.osdebug(__LOGGER_FUNC__, " Processing section data - buffer size: ", dataPacket.getDataSize());

    m_controller.addData(dataPacket);

    m_logger.osdebug(__LOGGER_FUNC__, " - done");
}

void CcSubController::activate()
{
    m_controller.start();
}

void CcSubController::deactivate()
{
    if (m_controller.isStarted()) {
        m_controller.stop();
    }
}

void CcSubController::mute(bool muted)
{
    m_logger.oswarning(__LOGGER_FUNC__, " (", muted, ")");
    if (muted) {
        m_controller.mute();
    } else {
        m_controller.unmute();
    }
}

bool CcSubController::wantsData(protocol::PacketChannelSpecific const& packet) const
{
    return  (packet.getChannelId() == m_channelId) and (m_controller.isStarted());
}

void CcSubController::processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet)
{
    m_controller.processSetCCAttributesPacket(packet);
}

void CcSubController::setTextForPreview(const std::string& text)
{
    m_controller.setTextForPreview(text);
}

} // namespace app
} // namespace subttxrend

