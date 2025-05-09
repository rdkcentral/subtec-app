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


#include "TtmlController.hpp"

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/ttmlengine/Factory.hpp>
#include <subttxrend/protocol/PacketTtmlSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketTtmlTimestamp.hpp>
#include <subttxrend/protocol/PacketTtmlInfo.hpp>

namespace subttxrend {
namespace ctrl {

TtmlController::TtmlController(const protocol::PacketChannelSpecific& dataPacket,
                               const common::ConfigProvider& config,
                               gfx::WindowPtr const& gfxWindow,
                               common::Properties const& properties)
    : m_channel()
    , m_logger("App", "TtmlController", this)
    , m_ttmlEngine(ttmlengine::Factory::createTtmlEngine())
{
    m_logger.oswarning(__LOGGER_FUNC__, " created");
    m_ttmlEngine->init(&config, gfxWindow.get(), properties);
    select(dataPacket);
}


void TtmlController::select(const protocol::PacketChannelSpecific& packet)
{
    const auto& ttmlSelectionPacket = static_cast<const protocol::PacketTtmlSelection&>(packet);
    select(ttmlSelectionPacket.getChannelId(),
           ttmlSelectionPacket.getRelatedVideoWidth(),
           ttmlSelectionPacket.getRelatedVideoHeight());
}

void TtmlController::select(std::uint32_t channelId, std::uint32_t width, std::uint32_t height)
{
    m_logger.osinfo(__LOGGER_FUNC__, " channel=", channelId, " related video size: ", width, 'x', height);
    m_channel.set(channelId);
    m_ttmlEngine->setRelatedVideoSize(gfx::Size(width, height));
    m_ttmlEngine->start();
}

TtmlController::~TtmlController()
{
    m_logger.oswarning(__LOGGER_FUNC__);
    m_ttmlEngine->stop();
    m_channel.reset();
}

void TtmlController::process()
{
    m_ttmlEngine->process();
}

void TtmlController::activate()
{
    m_logger.oswarning(__LOGGER_FUNC__, " not implemented");
}

void TtmlController::deactivate()
{
    m_logger.oswarning(__LOGGER_FUNC__, " not implemented");
}

void TtmlController::addData(const protocol::PacketData& packet)
{
    addData(reinterpret_cast<const std::uint8_t*>(packet.getData()), packet.getDataSize(), packet.getDisplayOffset());
}

std::chrono::milliseconds TtmlController::getWaitTime() const
{
    return m_ttmlEngine->getWaitTime();
}

void TtmlController::addData(const std::uint8_t* data, std::size_t dataLen, std::int64_t displayOffset)
{
    m_ttmlEngine->addData(data, dataLen, displayOffset);
}

bool TtmlController::wantsData(protocol::PacketChannelSpecific const& packet) const
{
    return wantsData(packet.getChannelId());
}

bool TtmlController::wantsData(std::uint32_t channelId) const
{
    return (m_channel.isActive() && m_channel.isEqual(channelId));
}

void TtmlController::processTimestamp(const protocol::Packet& packet)
{
    auto const& tsPack = static_cast<protocol::PacketTtmlTimestamp const&>(packet);
    processTimestamp(tsPack.getTimestamp());
}

void TtmlController::processInfo(const protocol::Packet& packet)
{
    auto const& infoPack = static_cast<protocol::PacketTtmlInfo const&>(packet);
    processInfo(infoPack.getContentType(), infoPack.getSubtitleInfo());
}

void TtmlController::processTimestamp(const std::uint64_t mediatime)
{
    m_ttmlEngine->currentMediatime(mediatime);
}

void TtmlController::processInfo(const std::string& contentType, const std::string& subsInfo)
{
    m_ttmlEngine->setSubtitleInfo(contentType, subsInfo);
}

void TtmlController::flush()
{
    m_ttmlEngine->flush();
}

void TtmlController::pause()
{
    m_ttmlEngine->pause();
}

void TtmlController::resume()
{
    m_ttmlEngine->resume();
}

void TtmlController::mute(bool m)
{
    if (m) {
        m_ttmlEngine->mute();
    } else {
        m_ttmlEngine->unmute();
    }
}

} // namespace ctrl
} // namespace subttxrend

