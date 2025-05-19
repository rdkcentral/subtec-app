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


#include "PesFinder.hpp"
#include "TtxController.hpp"
#include "StcProvider.hpp"

#include <subttxrend/ttxt/Factory.hpp>
#include <subttxrend/protocol/PacketTeletextSelection.hpp>
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>

namespace subttxrend {
namespace ctrl {

TtxController::TtxController(protocol::PacketChannelSpecific const& packet,common::ConfigProvider const& config,
                             const gfx::WindowPtr& gfxWindow,
                             const gfx::EnginePtr& gfxEngine,
                             StcProvider& stcProvider)
    : m_subtitleStatus()
    , m_teletextStatus()
    , m_teletextChannel()
    , m_subtitleChannel()
    , m_stcProvider(&stcProvider)
    , m_browserRenderer{ttxt::Factory::createBrowserRenderer()}
    , m_subtitleRenderer{ttxt::Factory::createSubtitlesRenderer()}
    , m_logger("App", "TtxController")
{
    m_logger.info("%s created", __LOGGER_FUNC__);

    if (!m_subtitleRenderer->init(gfxWindow.get(), gfxEngine, &config, this)) {
        m_logger.error("%s - subtitle renderer init failed", __LOGGER_FUNC__);
    }

    if (!m_browserRenderer->init(gfxWindow.get(), gfxEngine, &config, this)) {
        m_logger.error("%s - browser renderer init failed", __LOGGER_FUNC__);
    }
    select(packet);
    start();
}

void TtxController::activate()
{
    m_logger.osinfo(__LOGGER_FUNC__);
    start();
}

void TtxController::deactivate()
{
    m_logger.osinfo(__LOGGER_FUNC__);
    stop();
}

void TtxController::stop()
{
    switch (m_selected) {
        case Selected::teletext:
            stopTeletext();
            break;
        case Selected::subtitle:
            stopSubtitle();
            break;
        case Selected::none:
            break;
    }
}

void TtxController::stopTeletext()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    if (m_browserRenderer->isStarted()) {
        m_browserRenderer->stop();
    }
}

void TtxController::stopSubtitle()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    if (m_subtitleRenderer->isStarted()) {
        m_subtitleRenderer->stop();
    }
}

void TtxController::start()
{
    switch (m_selected) {
        case Selected::teletext:
            startTeletext();
            break;
        case Selected::subtitle:
            startSubtitle();
            break;
        case Selected::none:
            break;
    }
}

void TtxController::startTeletext()
{
    m_logger.info("%s", __LOGGER_FUNC__);

    assert(m_teletextChannel.isActive());

    if (m_browserRenderer->start(m_teletextStatus.m_initialMagazine, m_teletextStatus.m_initialPage)) {
        m_logger.trace("%s channelId: %u magazine: %d page: %d",
                       __LOGGER_FUNC__,
                       m_teletextChannel.getChannelId(),
                       m_teletextStatus.m_initialMagazine,
                       m_teletextStatus.m_initialPage);
    } else {
        m_logger.error("%s start failed", __LOGGER_FUNC__);
        m_teletextChannel.reset();
    }
}

void TtxController::startSubtitle()
{
    m_logger.info("%s", __LOGGER_FUNC__);

    assert(m_subtitleChannel.isActive());

    if (m_subtitleRenderer->start(m_subtitleStatus.m_auxId1, m_subtitleStatus.m_auxId2)) {
        m_logger.trace("%s channelId: %u compositionPageId: %u ancillaryPageId: %u",
                       __LOGGER_FUNC__,
                       m_subtitleChannel.getChannelId(),
                       m_subtitleStatus.m_auxId1,
                       m_subtitleStatus.m_auxId2);
    } else {
        m_logger.error("%s start failed", __LOGGER_FUNC__);
        m_subtitleChannel.reset();
    }
}

TtxController::~TtxController()
{
    reset();
    m_subtitleRenderer->shutdown();
    m_browserRenderer->shutdown();
}

void TtxController::process()
{
    switch (m_selected) {
        case Selected::subtitle:
            m_subtitleRenderer->processData();
            break;
        case Selected::teletext:
            m_browserRenderer->processData();
            break;
        default:
            break;
    }
}

void TtxController::addData(protocol::PacketData const& data)
{
    const std::uint8_t* bufferStart = reinterpret_cast<const std::uint8_t*>(data.getData());
    std::size_t dataSize = data.getDataSize();

    m_logger.trace("%s Processing PES data - buffer: %p size: %zu ", __LOGGER_FUNC__, bufferStart, dataSize);

    if (!m_subtitleRenderer->isStarted() && !m_browserRenderer->isStarted()) {
        return;
    }

    PesFinder pesFinder(bufferStart, dataSize);

    const std::uint8_t* pesStart = nullptr;
    std::uint16_t pesSize = 0;
    const auto channelId = data.getChannelId();

    while (pesFinder.findNextPes(pesStart, pesSize)) {
        switch (m_selected) {
            case Selected::subtitle:
                if (m_subtitleChannel.isEqual(channelId)) {
                    m_subtitleRenderer->addPesPacket(pesStart, pesSize);
                } else {
                    m_logger.trace("%s discarding packet - incorrect channelId: %d", __LOGGER_FUNC__, channelId);
                }
                break;
            case Selected::teletext:
                if (m_teletextChannel.isEqual(channelId)) {
                    m_browserRenderer->addPesPacket(pesStart, pesSize);
                } else {
                    m_logger.trace("%s discarding packet - incorrect channelId: %d", __LOGGER_FUNC__, channelId);
                }
                break;
            default:
                break;
        }
    }
}

void TtxController::select(protocol::PacketChannelSpecific const& packet)
{
    using Type = protocol::Packet::Type;
    using namespace protocol;
    auto const packetType = packet.getType();

    if (packetType == Type::SUBTITLE_SELECTION) {
        auto const& subsel = static_cast<PacketSubtitleSelection const&>(packet);
        selectSubtitle(subsel.getChannelId(), subsel.getAuxId1(), subsel.getAuxId2());
    } else if (packetType == Type::TELETEXT_SELECTION) {
        auto const& ttxsel = static_cast<PacketTeletextSelection const&>(packet);
        selectTeletext(packet.getChannelId(), ttxsel.getInitialMagazine(), ttxsel.getInitialPage());
    }
}

void TtxController::selectTeletext(std::uint32_t channelId, std::uint32_t initialMagazine, std::uint32_t initialPage)
{
    m_logger.info("%s channel=%u initialMagazine=%d initialPage=%d",
                  __LOGGER_FUNC__,
                  channelId,
                  initialMagazine,
                  initialPage);

    m_selected = Selected::teletext;
    m_teletextChannel.set(channelId);
    m_teletextStatus.m_started = true;
    m_teletextStatus.m_initialMagazine = initialMagazine;
    m_teletextStatus.m_initialPage = initialPage;
}

void TtxController::mute(bool muted) {
    switch (m_selected) {
        case Selected::subtitle:
            setMutedSubtitles(muted);
            break;
        case Selected::teletext:
            setMutedTeletext(muted);
            break;
        default:
            break;
    }
}

bool TtxController::wantsData(protocol::PacketChannelSpecific const& packet) const
{
    return wantsData(packet.getChannelId());
}


void TtxController::setMutedTeletext(bool muted)
{
    m_logger.osinfo(__LOGGER_FUNC__,"(",muted,")");

    m_teletextStatus.m_muted = muted;

    if (muted) {
        m_browserRenderer->mute();
    } else {
        m_browserRenderer->unmute();
    }
}

void TtxController::selectSubtitle(std::uint32_t channelId, std::uint32_t magazineNbr, std::uint32_t pageNumber)
{
    m_logger.info("%s channel=%u mgz_nbr=%u page=%u", __LOGGER_FUNC__, channelId, magazineNbr, pageNumber);

    m_selected = Selected::subtitle;
    m_subtitleChannel.set(channelId);
    m_subtitleStatus.m_started = true;
    m_subtitleStatus.m_auxId1 = magazineNbr;
    m_subtitleStatus.m_auxId2 = pageNumber;
}

void TtxController::reset()
{
    m_logger.info("%s resetting all", __LOGGER_FUNC__);

    stopTeletext();
    stopSubtitle();

    m_selected = Selected::none;
    m_browserRenderer->resetStartPage();

    m_teletextChannel.reset();
    m_teletextStatus.reset();

    m_subtitleChannel.reset();
    m_subtitleStatus.reset();
}

void TtxController::setMutedSubtitles(bool muted)
{
    m_logger.info("%s (%d)", __LOGGER_FUNC__, muted);

    m_subtitleStatus.m_muted = muted;
    if (muted) {
        m_subtitleRenderer->mute();
    } else {
        m_subtitleRenderer->unmute();
    }
}

bool TtxController::isTeletextActive() const
{
    return m_teletextStatus.m_started && (!m_teletextStatus.m_muted);
}

bool TtxController::areSubtitlesActive() const
{
    return m_subtitleStatus.m_started && (!m_subtitleStatus.m_muted);
}

bool TtxController::wantsData(std::uint32_t channelId) const
{
    return m_teletextChannel.isEqual(channelId) || m_subtitleChannel.isEqual(channelId);
}

std::uint32_t TtxController::getStc()
{
    if (m_stcProvider) {
        return m_stcProvider->getStc();
    } else {
        return 0;
    }
}

} // namespace ctrl
} // namespace subttxrend
