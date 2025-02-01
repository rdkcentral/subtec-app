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


#include "MetadataProcessor.hpp"

#include <subttxrend/common/Logger.hpp>

#include "DecodedPage.hpp"
#include "PageAll.hpp"
#include "Database.hpp"

namespace ttxdecoder
{

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "MetadataProcessor");

} // namespace <anonymous>

MetadataProcessor::MetadataProcessor(Database& database) :
        m_database(database),
        m_topNavProcessor(database)
{
    reset();
}

void MetadataProcessor::reset()
{
    m_topNavProcessor.reset();
}

Page* MetadataProcessor::getPageBuffer(const PageId& pageId)
{
    // currently only TOP processor needs specific pages
    return m_topNavProcessor.getPageBuffer(pageId);
}

Packet* MetadataProcessor::getPacketBuffer(std::uint8_t magazine,
                                           std::uint8_t packetAddress,
                                           std::int8_t designationCode)
{
    if ((magazine == 0) && (packetAddress == 30))
    {
        if ((designationCode >= 0) && (designationCode <= 3))
        {
            return &m_bsdPacket;
        }
    }

    if ((packetAddress == 29) && (magazine < Database::MAGAZINE_COUNT))
    {
        auto& magazinePage = m_database.getMagazinePage(magazine);
        return magazinePage.takePacket(packetAddress, designationCode);
    }

    return nullptr;
}

void MetadataProcessor::processPage(const Page& page)
{
    // currently only TOP processor needs to process pages
    m_topNavProcessor.processPage(page);
}

void MetadataProcessor::processPacket(const Packet& packet)
{
    switch (packet.getType())
    {
    case PacketType::HEADER:
        processTypedPacket(static_cast<const PacketHeader&>(packet));
        break;

    case PacketType::BCAST_SERVICE_DATA:
        processTypedPacket(static_cast<const PacketBcastServiceData&>(packet));
        break;

    case PacketType::TRIPLETS:
        processTypedPacket(static_cast<const PacketTriplets&>(packet));
        break;

    default:
        break;
    }
}

void MetadataProcessor::processTypedPacket(const PacketHeader& packet)
{
    // TODO fill database meta
}

void MetadataProcessor::processTypedPacket(const PacketBcastServiceData& packet)
{
    m_database.setIndexPageP830(packet.getInitialPage());
}

void MetadataProcessor::processTypedPacket(const PacketTriplets& packet)
{
    auto packetAddress = packet.getPacketAddress();
    auto magazine = packet.getMagazineNumber();

    if ((packetAddress == 29) && (magazine < Database::MAGAZINE_COUNT))
    {
        auto& magazinePage = m_database.getMagazinePage(magazine);
        magazinePage.setLastPacketValid(&packet);
    }
}

} // namespace ttxdecoder
