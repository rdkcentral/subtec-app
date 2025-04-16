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

#include "WebvttController.hpp"

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/webvttengine/Factory.hpp>
#include <subttxrend/webvttengine/WebVTTAttributes.hpp>
#include <subttxrend/protocol/PacketWebvttSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketWebvttTimestamp.hpp>

using subttxrend::webvttengine::WebVTTAttributes;
using subttxrend::protocol::PacketSetCCAttributes;

namespace subttxrend {
namespace ctrl {

WebvttController::WebvttController(const protocol::PacketChannelSpecific& dataPacket,
                               const common::ConfigProvider& config,
                               gfx::WindowPtr const& gfxWindow)
    : m_channel()
    , m_logger("App", "WebvttController", this)
    , m_webvttEngine(webvttengine::Factory::createWebvttEngine())
{
    m_logger.oswarning(__LOGGER_FUNC__, " created");
    m_webvttEngine->init(&config, gfxWindow);
    select(dataPacket);
}


void WebvttController::select(const protocol::PacketChannelSpecific& packet)
{
    const auto& webvttSelectionPacket = static_cast<const protocol::PacketWebvttSelection&>(packet);
    select(webvttSelectionPacket.getChannelId(),
           webvttSelectionPacket.getRelatedVideoWidth(),
           webvttSelectionPacket.getRelatedVideoHeight());
}

void WebvttController::select(std::uint32_t channelId, std::uint32_t width, std::uint32_t height)
{
    m_logger.osinfo(__LOGGER_FUNC__, " channel=", channelId, " related video size: ", width, 'x', height);
    m_channel.set(channelId);
    m_webvttEngine->setRelatedVideoSize(gfx::Size(width, height));
    m_webvttEngine->start();
}

WebvttController::~WebvttController()
{
    m_logger.oswarning(__LOGGER_FUNC__);
    m_webvttEngine->stop();
    m_channel.reset();
}

void WebvttController::process()
{
    m_webvttEngine->process();
}

void WebvttController::activate()
{
    m_logger.oswarning(__LOGGER_FUNC__, " not implemented");
}

void WebvttController::deactivate()
{
    m_logger.oswarning(__LOGGER_FUNC__, " not implemented");
}

void WebvttController::addData(const protocol::PacketData& packet)
{
    addData(reinterpret_cast<const std::uint8_t*>(packet.getData()), packet.getDataSize(), packet.getDisplayOffset());
}

void WebvttController::addData(const std::uint8_t* data, std::size_t dataLen, std::int64_t displayOffset)
{
    m_webvttEngine->addData(data, dataLen, displayOffset);
}

std::chrono::milliseconds WebvttController::getWaitTime() const
{
    return m_webvttEngine->getWaitTime();
}

bool WebvttController::wantsData(const protocol::PacketChannelSpecific& packet) const
{
    return wantsData(packet.getChannelId());
}

bool WebvttController::wantsData(std::uint32_t channelId) const
{
    return (m_channel.isActive() && m_channel.isEqual(channelId));
}

void WebvttController::processTimestamp(const protocol::Packet& packet)
{
    auto const& tsPack = static_cast<protocol::PacketWebvttTimestamp const&>(packet);
    processTimestamp(tsPack.getTimestamp());
}

void WebvttController::processTimestamp(const std::uint64_t mediatime)
{
    m_webvttEngine->currentMediatime(mediatime);
}

void WebvttController::pause()
{
    m_webvttEngine->pause();
}

void WebvttController::resume()
{
    m_webvttEngine->resume();
}

void WebvttController::mute(bool m)
{
    if (m) {
        m_webvttEngine->mute();
    } else {
        m_webvttEngine->unmute();
    }
}

void WebvttController::processSetCCAttributesPacket(const PacketSetCCAttributes& packet)
{
    WebVTTAttributes attributes;

    using attributeType = PacketSetCCAttributes::CcAttribType;

    const auto attributesMap = packet.getAttributes();

    for(const auto& p : attributesMap)
    {
        attributeType type;
        uint32_t value;
        std::tie(type, value) = p;

        printf("new value for attibId[0x%x] = %d\n", (int)type, value);

        switch(type)
        {
            case attributeType::FONT_COLOR:          attributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, value);         break;
            case attributeType::BACKGROUND_COLOR:    attributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR, value);   break;
            case attributeType::FONT_SIZE:           attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, value);          break;
            case attributeType::FONT_STYLE:          attributes.setInteger(WebVTTAttributes::AttributeType::FONT_STYLE, value);         break;
            case attributeType::EDGE_TYPE:           attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_STYLE, value);         break;
            case attributeType::EDGE_COLOR:          attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, value);         break;
            case attributeType::FONT_OPACITY:        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_OPACITY, value);       break;
            case attributeType::BACKGROUND_OPACITY:  attributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY, value); break;
            case attributeType::WIN_COLOR:           attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, value);       break;
            case attributeType::WIN_OPACITY:         attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY, value);     break;
            default:
                m_logger.oswarning(__LOGGER_FUNC__, "attibId[0x%x] not supported", (int)type);
        }
    }

    m_webvttEngine->setAttributes(attributes);
}

} // namespace ctrl
} // namespace subttxrend

