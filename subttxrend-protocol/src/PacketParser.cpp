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


#include "PacketParser.hpp"

#include <stdexcept>

#include "Buffer.hpp"
#include "BufferReader.hpp"

#include "PacketTimestamp.hpp"
#include "PacketResetAll.hpp"
#include "PacketResetChannel.hpp"
#include "PacketSubtitleSelection.hpp"
#include "PacketTeletextSelection.hpp"
#include "PacketTtmlInfo.hpp"
#include "PacketTtmlSelection.hpp"
#include "PacketTtmlTimestamp.hpp"
#include "PacketWebvttSelection.hpp"
#include "PacketWebvttTimestamp.hpp"
#include "PacketInvalid.hpp"
#include "PacketData.hpp"
#include "PacketPause.hpp"
#include "PacketResume.hpp"
#include "PacketMute.hpp"
#include "PacketUnmute.hpp"
#include "PacketSetCCAttributes.hpp"

namespace subttxrend
{
namespace protocol
{

common::Logger PacketParser::m_logger("Protocol", "PacketParser");

PacketParser::PacketParser()
{
    addParsablePacket(std::make_unique<PacketData>(PacketChannelSpecific::Type::PES_DATA));
    addParsablePacket(std::make_unique<PacketData>(PacketChannelSpecific::Type::TTML_DATA));
    addParsablePacket(std::make_unique<PacketData>(PacketChannelSpecific::Type::CC_DATA));
    addParsablePacket(std::make_unique<PacketData>(PacketChannelSpecific::Type::WEBVTT_DATA));
    addParsablePacket(std::make_unique<PacketTimestamp>());
    addParsablePacket(std::make_unique<PacketResetAll>());
    addParsablePacket(std::make_unique<PacketResetChannel>());
    addParsablePacket(std::make_unique<PacketSubtitleSelection>());
    addParsablePacket(std::make_unique<PacketTeletextSelection>());
    addParsablePacket(std::make_unique<PacketTtmlSelection>());
    addParsablePacket(std::make_unique<PacketTtmlTimestamp>());
    addParsablePacket(std::make_unique<PacketWebvttSelection>());
    addParsablePacket(std::make_unique<PacketWebvttTimestamp>());
    addParsablePacket(std::make_unique<PacketPause>());
    addParsablePacket(std::make_unique<PacketResume>());
    addParsablePacket(std::make_unique<PacketMute>());
    addParsablePacket(std::make_unique<PacketUnmute>());
    addParsablePacket(std::make_unique<PacketTtmlInfo>());
    addParsablePacket(std::unique_ptr<Packet>(new PacketSetCCAttributes()));

    m_invalidPacket.reset(new PacketInvalid());
}

const Packet& PacketParser::parse(common::DataBufferPtr packetBuffer)
{
    std::uint32_t packetType = Packet::extractType(Buffer(packetBuffer->data(), packetBuffer->size()));

    m_logger.trace("%s - Parsed packet type: %u", __LOGGER_FUNC__, packetType);

    Packet* packet = m_invalidPacket.get();

    auto packetIterator = m_packets.find(packetType);
    if (packetIterator != m_packets.end())
    {
        m_logger.trace("%s - Packet object found", __LOGGER_FUNC__);

        if (packetIterator->second->parse(std::move(packetBuffer)))
        {
            m_logger.trace("%s - Packet successfully parsed", __LOGGER_FUNC__);

            packet = packetIterator->second.get();
        }
        else
        {
            m_logger.debug("%s - Packet parsing failed", __LOGGER_FUNC__);
        }
    }
    else
    {
        m_logger.debug("%s - Packet object not found", __LOGGER_FUNC__);
    }

    return *packet;
}

void PacketParser::addParsablePacket(std::unique_ptr<Packet> packet)
{
    auto type = static_cast<std::uint32_t>(packet->getType());
    m_packets.insert(std::make_pair(type, std::move(packet)));
}

} // namespace protocol
} // namespace subttxrend
