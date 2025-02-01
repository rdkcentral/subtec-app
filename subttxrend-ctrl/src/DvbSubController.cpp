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


#include "DvbSubController.hpp"
#include "PesFinder.hpp"
#include "StcProvider.hpp"
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>

#include <subttxrend/dvbsub/Factory.hpp>

namespace subttxrend {
namespace app {

DvbSubController::DvbSubController(const protocol::PacketChannelSpecific& packet,
                                   const gfx::WindowPtr& gfxWindow,
                                   const gfx::EnginePtr&,
                                   StcProvider& stcProvider)
    : m_channel()
    , m_stcProvider(&stcProvider)
    , m_renderer{dvbsub::Factory::createSubtitlesRenderer()}
    , m_logger("App", "DvbSubController")
{
    m_logger.info("%s created", __LOGGER_FUNC__);
    if (!m_renderer->init(gfxWindow.get(), this)) {
        m_logger.error("%s - init failed", __LOGGER_FUNC__);
    }
    select(packet);
}

DvbSubController::~DvbSubController()
{
    if (m_renderer->isStarted()) {
        m_renderer->stop();
    }

    m_channel.reset();
    m_status.reset();
    m_renderer->shutdown();
}

void DvbSubController::process()
{
    m_renderer->processData();
}

void DvbSubController::addData(protocol::PacketData const& packet)
{
    addData(reinterpret_cast<const std::uint8_t*>(packet.getData()), packet.getDataSize());
}

void DvbSubController::addData(const std::uint8_t* bufferStart, std::size_t dataSize)
{
    m_logger.trace("%s Processing PES data - buffer: %p size: %zu ", __LOGGER_FUNC__, bufferStart, dataSize);

    if (!m_renderer->isStarted()) {
        return;
    }

    PesFinder pesFinder(bufferStart, dataSize);

    const std::uint8_t* pesStart = nullptr;
    std::uint16_t pesSize = 0;

    while (pesFinder.findNextPes(pesStart, pesSize)) {
        m_renderer->addPesPacket(pesStart, pesSize);
    }
}

void DvbSubController::activate()
{
    start();
}

void DvbSubController::deactivate()
{
    if (m_renderer->isStarted()) {
        m_renderer->stop();
    }
}

void DvbSubController::select(const protocol::PacketChannelSpecific& packet)
{
    auto const& selectPacket = static_cast<protocol::PacketSubtitleSelection const&>(packet);
    select(packet.getChannelId(), selectPacket.getAuxId1(), selectPacket.getAuxId2());
}

void DvbSubController::select(std::uint32_t channelId, std::uint32_t compositionPageId, std::uint32_t ancillaryPageId)
{
    m_logger.info("%s channel=%u compositionPageId=%u ancillaryPageId=%u",
                  __LOGGER_FUNC__,
                  channelId,
                  compositionPageId,
                  ancillaryPageId);

    m_channel.set(channelId);
    m_status.m_auxId1 = compositionPageId;
    m_status.m_auxId2 = ancillaryPageId;
    m_status.m_started = true;

    start();
}

void DvbSubController::start()
{
    if (m_renderer->start(m_status.m_auxId1, m_status.m_auxId2)) {
        m_logger.trace("%s channelId: %u compositionPageId: %u ancillaryPageId: %u",
                       __LOGGER_FUNC__,
                       m_channel.getChannelId(),
                       m_status.m_auxId1,
                       m_status.m_auxId2);
    } else {
        m_logger.oserror(__LOGGER_FUNC__, " start failed");
        m_channel.reset();
    }
}

void DvbSubController::mute(bool muted)
{
    m_logger.info("%s (%d)", __LOGGER_FUNC__, muted);

    m_status.m_muted = muted;
    if (muted) {
        m_renderer->mute();
    } else {
        m_renderer->unmute();
    }
}

bool DvbSubController::wantsData(protocol::PacketChannelSpecific const& packet) const
{
    return m_channel.isEqual(packet.getChannelId());
}

std::uint32_t DvbSubController::getStc()
{
    if (m_stcProvider) {
        return m_stcProvider->getStc();
    } else {
        return 0;
    }
}

} // namespace app
} // namespace subttxrend

