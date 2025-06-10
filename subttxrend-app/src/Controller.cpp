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


#include "Controller.hpp"

#include <subttxrend/ctrl/DvbSubController.hpp>
#include <subttxrend/ctrl/CcSubController.hpp>
#include <subttxrend/ctrl/Configuration.hpp>
#include <subttxrend/ctrl/TtxController.hpp>
#include <subttxrend/ctrl/TtmlController.hpp>
#include <subttxrend/ctrl/WebvttController.hpp>
#include <subttxrend/ctrl/ScteSubController.hpp>

#include <subttxrend/common/TtmlAsClient.hpp>
#include <subttxrend/common/WsConnection.hpp>
#include <subttxrend/common/Properties.hpp>

#include <chrono>

namespace subttxrend {
namespace app {


namespace  {

template <class Ctrls, class Packet, class Argument>
void forAllControllers(Ctrls const& ctrls, Packet const& packet, Argument arg, void (ctrl::ControllerInterface::*function)(Argument)) {
    for (auto const& ctrl: ctrls) {
        if (ctrl && ctrl->wantsData(packet)) {
            ((*ctrl).*function)(arg);
        }
    }
}

template <class Ctrls, class Packet, class Argument>
void forAllControllers(Ctrls const& ctrls, Packet const& packet, void (ctrl::ControllerInterface::*function)(Argument))
{
    for (auto const& ctrl: ctrls) {
        if (ctrl && ctrl->wantsData(packet)) {
            ((*ctrl).*function)(packet);
        }
    }
}

template <class Ctrls, class Packet>
void forAllControllers(Ctrls const& ctrls, Packet const& packet, void (ctrl::ControllerInterface::*function)()) {
    for (auto const& ctrl: ctrls) {
        if (ctrl && ctrl->wantsData(packet)) {
            ((*ctrl).*function)();
        }
    }
}

constexpr const std::chrono::milliseconds connection_status_check_timeout{1000};

} /* namespace  */

Controller::Controller(ctrl::Configuration const& config, gfx::EnginePtr gfxEngine, gfx::WindowPtr gfxWindow)
    : m_config(config)
    , m_gfxEngine(gfxEngine)
    , m_gfxWindow(gfxWindow)
    , m_fontCache{std::make_shared<gfx::PrerenderedFontCache>()}
    , m_stcProvider()
    , m_logger("App", "Controller", this)
    , m_endpoint{std::make_unique<common::WsEndpoint>()}
{
    m_asClient = std::make_unique<common::AsClient>(connection_status_check_timeout,
               std::make_unique<common::WsConnection>("subttxrend-app", *m_endpoint));
    m_asLstnr = std::make_unique<common::TtmlAsClient>();

    common::AsHelpers helpers;
    auto tmp = m_asLstnr->getHelpers();
    helpers.insert(helpers.end(), tmp.begin(), tmp.end());
    m_asClient->setup(std::move(helpers));
}

void Controller::startAsync()
{
    m_logger.osinfo(__LOGGER_FUNC__);
    {
        LockGuard lock{m_renderMutex};
        m_quitRenderThread = false;
    }
    m_renderThread = std::thread(&Controller::processLoop, this);
}

void Controller::stop()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    m_logger.osinfo(__LOGGER_FUNC__, " stopping render thread");
    {
        LockGuard lock{m_renderMutex};
        m_quitRenderThread = true;
    }
    m_renderCond.notify_one();

    if (m_renderThread.joinable()) {
        m_logger.osinfo(__LOGGER_FUNC__, " waiting for render thread to finish");
        m_renderThread.join();
    }

    LockGuard lock{m_mutex};

    m_dataqueue.clear();
    m_activeControllers.clear();
}

std::chrono::milliseconds Controller::processData()
{
    auto waitTime = std::chrono::milliseconds::zero();
    {
        bool shouldStop = false;
        UniqueLock lock{m_mutex};
        while (!shouldStop)
        {
            if(m_dataqueue.empty())
            {
                shouldStop = true;  //even if dataqueue is empty, we need to run controller->process() and then finish the loop
            }
            else
            {
                auto const& packet = m_parser.parse(std::move(m_dataqueue.front()));
                {
                    auto t = m_logger.timing("doOnPacketReceived");
                    doOnPacketReceived(lock, packet);
                }
                m_dataqueue.pop_front();
            }
        }
    }
    {
        std::shared_ptr<ctrl::ControllerInterface> controller;
        {
            LockGuard lock{m_mutex};
            m_inuse = true;
            controller = topController();
        }
        if (controller) {
            controller->process();
            waitTime = controller->getWaitTime();
        }

        {
            LockGuard lock{m_mutex};
            controller.reset();
            m_inuse = false;
        }
        m_cond.notify_one();
    }
    return waitTime;
}


void Controller::addBuffer(common::DataBufferPtr buffer)
{
    assert(buffer);
    m_logger.osdebug(__LOGGER_FUNC__, " bufferSize=", buffer->size());
    const bool renderingActive = isRenderingActive();
    if(renderingActive)
    {
        {
            LockGuard lock{m_mutex};
            m_dataqueue.emplace_back(std::move(buffer));
        }
        m_renderCond.notify_one();
    }
    else
    {
        m_logger.osdebug(__LOGGER_FUNC__, " no active controller, skipping the packet");
    }
}

void Controller::onPacketReceived(protocol::Packet const& packet)
{
    m_logger.osdebug(__LOGGER_FUNC__, " packet=", packet);
    {
        UniqueLock lock{m_mutex};
        doOnPacketReceived(lock, packet);
    }
    m_renderCond.notify_one();
}

void Controller::doOnPacketReceived(UniqueLock& lock, const protocol::Packet& packet)
{
    m_logger.ostrace(__LOGGER_FUNC__, " - Packet received (type: ", static_cast<unsigned int>(packet.getType()), ")");

    switch (packet.getType()) {
        case protocol::Packet::Type::SUBTITLE_SELECTION:
        case protocol::Packet::Type::TELETEXT_SELECTION:
        case protocol::Packet::Type::TTML_SELECTION:
        case protocol::Packet::Type::WEBVTT_SELECTION: {
            processSelection(static_cast<protocol::PacketChannelSpecific const&>(packet));
            break;
        }
        case protocol::Packet::Type::PES_DATA:
        case protocol::Packet::Type::TTML_DATA:
        case protocol::Packet::Type::WEBVTT_DATA:
        case protocol::Packet::Type::CC_DATA: {
            processDataPacket(static_cast<protocol::PacketData const&>(packet));
            break;
        }
        case protocol::Packet::Type::RESET_ALL: {
            processResetAll(lock, static_cast<protocol::PacketResetAll const&>(packet));
            break;
        }
        case protocol::Packet::Type::RESET_CHANNEL: {
            processResetChannel(lock, static_cast<protocol::PacketResetChannel const&>(packet));
            break;
        }
        case protocol::Packet::Type::TIMESTAMP: {
            const auto& timestampPacket = static_cast<const protocol::PacketTimestamp&>(packet);
            m_stcProvider.processTimestamp(timestampPacket.getStc(), timestampPacket.getTimestamp());
            break;
        }
        case protocol::Packet::Type::TTML_TIMESTAMP: {
            processTtmlTimestamp(static_cast<protocol::PacketTtmlTimestamp const&>(packet));
            break;
        }
        case protocol::Packet::Type::WEBVTT_TIMESTAMP: {
            processWebvttTimestamp(static_cast<protocol::PacketWebvttTimestamp const&>(packet));
            break;
        }
        case protocol::Packet::Type::FLUSH: {
            processFlushPacket(static_cast<protocol::PacketFlush const&>(packet));
            break;
        }
        case protocol::Packet::Type::PAUSE: {
            processPausePacket(static_cast<protocol::PacketPause const&>(packet));
            break;
        }
        case protocol::Packet::Type::RESUME: {
            processResumePacket(static_cast<protocol::PacketResume const&>(packet));
            break;
        }
        case protocol::Packet::Type::MUTE: {
            processMutePacket(static_cast<protocol::PacketMute const&>(packet));
            break;
        }
        case protocol::Packet::Type::UNMUTE: {
            processUnmutePacket(static_cast<protocol::PacketUnmute const&>(packet));
            break;
        }
        case protocol::Packet::Type::TTML_INFO: {
            processTtmlInfoPacket(static_cast<protocol::PacketTtmlInfo const&>(packet));
            break;
        }
        case protocol::Packet::Type::SET_CC_ATTRIBUTES: {
            processSetCCAttributes(static_cast<protocol::PacketSetCCAttributes const&>(packet));
            break;
        }
        default: {
            m_logger.oserror(__LOGGER_FUNC__,
                             " - Invalid packet type (type: ",
                             static_cast<unsigned int>(packet.getType()),
                             ")");
            break;
        }
    }
}

void Controller::onStreamBroken()
{
    m_logger.osinfo(__LOGGER_FUNC__, " - Packets stream broken - resetting");
    LockGuard lock{m_mutex};
    resetAll();
}

void Controller::processSelection(const protocol::PacketChannelSpecific& packet)
{
    switch (packet.getType()) {
        case protocol::Packet::Type::SUBTITLE_SELECTION:
            processSubtitleSelection(static_cast<protocol::PacketSubtitleSelection const&>(packet));
            break;
        case protocol::Packet::Type::TELETEXT_SELECTION:
            processTeletextSelection(static_cast<protocol::PacketTeletextSelection const&>(packet));
            break;
        case protocol::Packet::Type::TTML_SELECTION:
            processTtmlSelection(static_cast<protocol::PacketTtmlSelection const&>(packet));
            break;
        case protocol::Packet::Type::WEBVTT_SELECTION:
            processWebvttSelection(static_cast<protocol::PacketWebvttSelection const&>(packet));
            break;
        default:
            break;
    }
}
void Controller::processSubtitleSelection(const protocol::PacketSubtitleSelection& packet)
{
    using namespace subttxrend::protocol;

    auto timing = m_logger.timing(__LOGGER_FUNC__);
    const std::uint32_t subtitleType = packet.getSubtitlesType();

    switch (subtitleType) {
        case PacketSubtitleSelection::SUBTITLES_TYPE_DVB:
            processDvbSubtitleSelection(packet);
            break;
        case PacketSubtitleSelection::SUBTITLES_TYPE_SCTE:
            processScteSubtitleSelection(packet);
            break;
        case PacketSubtitleSelection::SUBTITLES_TYPE_CC:
            processCcSubtitleSelection(packet);
            break;
        case PacketSubtitleSelection::SUBTITLES_TYPE_TELETEXT:
            processTtxSubtitleSelection(packet);
            break;
        default:
            m_logger.oserror(__LOGGER_FUNC__, " unknown subtitle type=", subtitleType);
            break;
    }
}

void Controller::processDvbSubtitleSelection(const protocol::PacketSubtitleSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting DVB Subtitles: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::MutexedController<ctrl::DvbSubController>>(packet, m_gfxWindow, m_gfxEngine, m_stcProvider));
}

void Controller::processScteSubtitleSelection(const protocol::PacketSubtitleSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting SCTE Subtitles: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::MutexedController<ctrl::ScteSubController>>(packet, m_gfxWindow, m_stcProvider));
}

void Controller::processCcSubtitleSelection(const protocol::PacketSubtitleSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting CC Subtitles: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::MutexedController<ctrl::CcSubController>>(packet, m_gfxWindow, m_fontCache));
}

void Controller::processTtxSubtitleSelection(const protocol::PacketSubtitleSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting TTX Subtitles: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::MutexedController<ctrl::TtxController>>(packet, m_config.getTeletextConfig(), m_gfxWindow, m_gfxEngine, m_stcProvider));
}

void Controller::processTeletextSelection(const protocol::PacketTeletextSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting TTX: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::MutexedController<ctrl::TtxController>>(packet, m_config.getTeletextConfig(), m_gfxWindow, m_gfxEngine, m_stcProvider));
}

void Controller::processTtmlSelection(const protocol::PacketTtmlSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting TTML Subtitles: ", packet.getChannelId());
    deactivateController();

    common::Properties properties;
    try {
        properties = m_asLstnr->getData();
    } catch (std::exception const& e) {
        m_logger.oserror(__LOGGER_FUNC__, " exception: ", e.what());
    }

    pushController(std::make_shared<ctrl::TtmlController>(packet, m_config.getTtmlConfig(), m_gfxWindow, properties));
}

void Controller::processWebvttSelection(const protocol::PacketWebvttSelection& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    m_logger.osinfo("Selecting WebVTT Subtitles: ", packet.getChannelId());
    deactivateController();
    pushController(std::make_shared<ctrl::WebvttController>(packet, m_config.getWebvttConfig(), m_gfxWindow));
}

using namespace std::string_literals;

void Controller::processResetChannel(UniqueLock& lock, const protocol::PacketResetChannel& packet)
{
    auto timing1 = m_logger.timing("wait"s + __LOGGER_FUNC__);
    m_cond.wait(lock, [this] { return !m_inuse;});
    auto timing2 = m_logger.timing(__LOGGER_FUNC__);

    m_logger.osinfo("try to reset channel: ", packet.getChannelId());
    for (auto& ctrl: m_activeControllers)
    {
        // reseting channel may cause that nullptr remain in m_controllers
        // if reseted channel wasn't the top one
        if (ctrl)
        {
            if (ctrl->wantsData(packet))
            {
                m_logger.osinfo("reseting channel: ", packet.getChannelId());
                ctrl.reset();
                break;
            }
        } else {
            m_logger.osdebug("ctrl is null");
        }
    }
    popNullController();
}

void Controller::processResetAll(UniqueLock& lock, const protocol::PacketResetAll&)
{
    m_cond.wait(lock, [this] { return !m_inuse;});
    m_logger.osinfo("reset All");
    resetAll();
}

void Controller::resetAll()
{
    m_dataqueue.clear();
    m_activeControllers.clear();

}

void Controller::processDataPacket(const protocol::PacketData& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    auto ctrl = topController();
    if (ctrl) {
        ctrl->addData(packet);
    }
}

void Controller::processTtmlTimestamp(const protocol::PacketTtmlTimestamp& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::processTimestamp);
}

void Controller::processTtmlInfoPacket(const protocol::PacketTtmlInfo& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::processInfo);
}

void Controller::processWebvttTimestamp(const protocol::PacketWebvttTimestamp& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::processTimestamp);
}

void Controller::processFlushPacket(const protocol::PacketChannelSpecific& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::flush);
    m_dataqueue.clear();
    m_renderCond.notify_one();
}

void Controller::processPausePacket(const protocol::PacketChannelSpecific& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::pause);
}

void Controller::processResumePacket(const protocol::PacketChannelSpecific& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::resume);
}

void Controller::processMutePacket(const protocol::PacketMute& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, true, &ctrl::ControllerInterface::mute);
}

void Controller::processUnmutePacket(const protocol::PacketUnmute& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, false, &ctrl::ControllerInterface::mute);
}

void Controller::pushController(std::shared_ptr<ctrl::ControllerInterface> controller)
{
    m_activeControllers.emplace_back(std::move(controller));
}

void Controller::deactivateController()
{
    auto top = topController();
    if (top) {
        m_logger.osinfo("deactivate controller");
        top->deactivate();
        m_activeControllers.pop_back();
    }
}

void Controller::popNullController()
{
    bool poped = false;
    while (!m_activeControllers.empty()) {
        if (m_activeControllers.back())
            break;
        poped = true;
        m_activeControllers.pop_back();
    }
    if (poped) {
        auto top = topController();
        if (top) {
            m_logger.osinfo("activate controller");
            top->activate();
        }
    }
}

std::shared_ptr<ctrl::ControllerInterface> Controller::topController()
{
    std::shared_ptr<ctrl::ControllerInterface> controller;
    if (!m_activeControllers.empty()) {
        controller = m_activeControllers.back();
    }
    return controller;
}

bool Controller::isRenderingActive() const
{
    LockGuard lock{m_mutex};
    return (! m_activeControllers.empty());
}

bool Controller::isDataQueued() const
{
    LockGuard lock{m_mutex};
    return ! m_dataqueue.empty();
}

void Controller::processLoop()
{
    m_logger.osinfo(__LOGGER_FUNC__, " starting process loop");

    UniqueLock lock(m_renderMutex);
    while (! m_quitRenderThread) {
        m_renderCond.wait(lock, [this]() {return m_quitRenderThread || (isRenderingActive() && isDataQueued());});
        m_logger.osinfo("woken up, quit=", m_quitRenderThread, " isRenderingActive=", isRenderingActive());

        while (isRenderingActive() && !m_quitRenderThread) {
            auto const processWaitTime = processData();
            m_gfxEngine->execute();

            if (processWaitTime == std::chrono::milliseconds::zero()) {
                break;
            }
            m_renderCond.wait_for(lock, processWaitTime, [this]() {return m_quitRenderThread;});
        }
                m_logger.osdebug(__LOGGER_FUNC__, " no active controller, clearing the data queue");
                UniqueLock lock{m_mutex};
                m_dataqueue.clear();
    }
}

void Controller::processSetCCAttributes(const protocol::PacketSetCCAttributes& packet)
{
    auto timing = m_logger.timing(__LOGGER_FUNC__);
    forAllControllers(m_activeControllers, packet, &ctrl::ControllerInterface::processSetCCAttributesPacket);
}

} // namespace app
} // namespace subttxrend

