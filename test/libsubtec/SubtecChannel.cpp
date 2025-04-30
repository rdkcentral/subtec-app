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

#include <memory>

#include "PacketSender.hpp"
#include "SubtecChannel.hpp"
#include "SubtecPacket.hpp"
#include "TtmlPacket.hpp"
#include "WebVttPacket.hpp"
#include "ClosedCaptionsPacket.hpp"


std::unique_ptr<SubtecChannel> SubtecChannel::SubtecChannelFactory(ChannelType type)
{
    std::unique_ptr<SubtecChannel> subtecChannel;

    switch (type)
    {
        case ChannelType::TTML:
            subtecChannel = subtec_utils::make_unique<TtmlChannel>();
            break;
        case ChannelType::WEBVTT:
            subtecChannel = subtec_utils::make_unique<WebVttChannel>();
            break;
        case ChannelType::CC:
            break;
        default:
            break;
    }

    return subtecChannel;
}

bool SubtecChannel::InitComms()
{
    const char *socket_path = ::getenv("AAMP_SUBTITLE_SOCKET");

    if (!socket_path)
    {
        socket_path = SOCKET_PATH;
    }

    return InitComms(socket_path);
}

bool SubtecChannel::InitComms(const char* socket_path)
{
    return PacketSender::Instance()->Init(socket_path);
}


template<typename PacketType, typename ...Args>
void SubtecChannel::sendPacket(Args && ...args)
{
    std::unique_lock<std::mutex> lock(mChannelMtx);
    PacketSender::Instance()->SendPacket(subtec_utils::make_unique<PacketType>(m_channelId, m_counter++, std::forward<Args>(args)...));
}

void SubtecChannel::SendResetAllPacket()
{
    std::unique_lock<std::mutex> lock(mChannelMtx);
    m_counter = 1;
    PacketSender::Instance()->SendPacket(subtec_utils::make_unique<ResetAllPacket>());
}

void SubtecChannel::SendResetChannelPacket() {
    sendPacket<ResetChannelPacket>();
}
void SubtecChannel::SendPausePacket() {
    sendPacket<PausePacket>();
}
void SubtecChannel::SendResumePacket() {
    sendPacket<ResumePacket>();
}
void SubtecChannel::SendMutePacket() {
    sendPacket<MutePacket>();
}
void SubtecChannel::SendUnmutePacket() {
    sendPacket<UnmutePacket>();
}
void SubtecChannel::SendCCSetAttributePacket(std::uint32_t ccType, std::uint32_t attribType, const attributesType &attributesValues) {
    AAMPLOG_INFO("SendCCSetAttributePacket bit mask is 0x%X", attribType);
    for(uint i = 0; i < attributesValues.size(); i++)
    {
        if (attribType & (1 << i))
        {
            AAMPLOG_TRACE("SendCCSetAttributePacket attribute[%u]: %u", i, attributesValues[i]);
        }
    }
    sendPacket<CCSetAttributePacket>(ccType, attribType, attributesValues);
}

SubtecChannel::~SubtecChannel() {}
