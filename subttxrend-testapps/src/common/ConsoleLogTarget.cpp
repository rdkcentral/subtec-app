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


#include "ConsoleLogTarget.hpp"

#include <iostream>

#include <subttxrend/protocol/Packet.hpp>
#include <subttxrend/protocol/PacketTimestamp.hpp>
#include <subttxrend/protocol/PacketResetChannel.hpp>
#include <subttxrend/protocol/PacketResetAll.hpp>
#include <subttxrend/protocol/PacketSubtitleSelection.hpp>
#include <subttxrend/protocol/PacketTeletextSelection.hpp>
#include <subttxrend/protocol/PacketData.hpp>

namespace subttxrend
{
namespace testapps
{

ConsoleLogTarget::ConsoleLogTarget(const std::string& path) :
        DataTarget(path),
        m_parsePackets(false)
{
    if (path == "parse")
    {
        m_parsePackets = true;
    }
}

ConsoleLogTarget::~ConsoleLogTarget()
{
    close();
}

bool ConsoleLogTarget::open()
{
    return true;
}

void ConsoleLogTarget::close()
{
    // noop
}

bool ConsoleLogTarget::wantsMorePackets()
{
    // packets always wanted
    return true;
}

bool ConsoleLogTarget::writePacket(const DataPacket& packet)
{
    const std::size_t BYTES_TO_PRINT = 32;

    std::cout << "Packet: size=" << packet.getSize() << std::endl;
    std::cout << "   ";
    for (std::size_t i = 0; (i < BYTES_TO_PRINT) && (i < packet.getSize()); ++i)
    {
        int value = static_cast<int>(packet.getBuffer()[i]) & 0xFF;
        char valueStr[32];

        sprintf(valueStr, " %02X", value);

        std::cout << valueStr;
    }
    std::cout << std::endl;


    if (m_parsePackets) {
        auto dataBuffer = std::make_unique<common::DataBuffer>(packet.getSize());
        dataBuffer->assign(packet.getBuffer(), packet.getBuffer() + packet.getSize());
        const protocol::Packet& parsedPacket = m_packetParser.parse(std::move(dataBuffer));
        std::cout << "Packet parser: Got packet:" << static_cast<int>(parsedPacket.getType()) <<
            " counter: " << parsedPacket.getCounter() <<
            " size: " << parsedPacket.getSize();

        switch (parsedPacket.getType())
        {
            case protocol::Packet::Type::PES_DATA:
                {
                    const protocol::PacketData& pdp = dynamic_cast<const protocol::PacketData&>(parsedPacket);
                    std::cout << " channelId: " << pdp.getChannelId()
                        << " channelType: " << pdp.getChannelType()
                        << " pesDataSize: " << pdp.getDataSize();
                    break;
                }
            case protocol::Packet::Type::TIMESTAMP:
                {
                    const protocol::PacketTimestamp& tp = dynamic_cast<const protocol::PacketTimestamp&>(parsedPacket);
                    std::cout << " timestamp: " << tp.getTimestamp()
                        << " stc: " << tp.getStc();
                    break;
                }
            case protocol::Packet::Type::RESET_ALL:
                {
                    std::cout << " reset all";
                    break;
                }
            case protocol::Packet::Type::RESET_CHANNEL:
                {
                    const protocol::PacketResetChannel& rcp = dynamic_cast<const protocol::PacketResetChannel&>(parsedPacket);
                    std::cout << " reset channel:" << rcp.getChannelId();
                    break;
                }
            case protocol::Packet::Type::TELETEXT_SELECTION:
                {
                    const protocol::PacketTeletextSelection& tsp = dynamic_cast<const protocol::PacketTeletextSelection&>(parsedPacket);
                    std::cout << " teletext selection:"
                        << " channel id: " << tsp.getChannelId();
                    break;
                }
            case protocol::Packet::Type::SUBTITLE_SELECTION:
                {
                    const protocol::PacketSubtitleSelection& ssp = dynamic_cast<const protocol::PacketSubtitleSelection&>(parsedPacket);
                    std::cout << " subtitle selection:"
                        << " channel id: " << ssp.getChannelId()
                        << " subtitles type: " << ssp.getSubtitlesType()
                        << " aux1: " << ssp.getAuxId1()
                        << " aux2: " << ssp.getAuxId2();
                    break;
                }
            case protocol::Packet::Type::INVALID:
                {
                    std::cout << " INVALID packet";
                    break;
                }
            default:
                {
                    break;
                }
        }

        std::cout << std::endl;
    }


    return true;
}

} // namespace testapps
} // namespace subttxrend
