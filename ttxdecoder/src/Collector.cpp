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


#include "Collector.hpp"

#include <subttxrend/common/Logger.hpp>

#include "CollectorListener.hpp"
#include "PesPacketReader.hpp"
#include "PacketAll.hpp"
#include "ControlInfo.hpp"

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "Collector");

constexpr std::uint8_t TELETEXT_UNIT_ID = 0x02;
constexpr std::uint8_t SUBTITLES_UNIT_ID = 0x03;
constexpr std::uint8_t TELETEXT_PACKET_LENGTH = 0x2C;

} // anonymous

namespace ttxdecoder
{

/**
 * Implemtation of the CollectorPacketContext.
 */
class CollectorPacketContextImpl : public CollectorPacketContext
{
public:
    /**
     * Constructor.
     *
     * @param collector
     *      Pointer to context owner.
     * @param reader
     *      Reader to packet data.
     * @param magazineNumber
     *      Magazine number (0-7).
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     */
    CollectorPacketContextImpl(Collector* collector,
                               PesPacketReader& reader,
                               std::uint8_t magazineNumber,
                               std::uint8_t packetAddress,
                               std::int8_t designationCode) :
            m_collector(collector),
            m_reader(reader),
            CollectorPacketContext(magazineNumber, packetAddress,
                    designationCode)
    {
        // noop
    }

    /** @copydoc CollectorPacketContext::consume */
    virtual bool consume(Packet& packet) override
    {
        packet.setMagazineNumber(getMagazineNumber());
        packet.setPacketAddress(getPacketAddress());

        return m_collector->collectPacket(packet, m_reader);
    }

private:
    /** Pointer to context owner. */
    Collector* const m_collector;

    /** Reader to packet data. */
    PesPacketReader& m_reader;
};

Collector::Collector(CollectorListener& listener) :
        m_listener(listener),
        m_hamming()
{
    // noop
}

Collector::~Collector()
{
    // noop
}

void Collector::reset()
{
    // noop
}

void Collector::processPacketData(PesPacketReader& reader)
{
    std::uint8_t dataIdentifier = reader.readUint8();

    g_logger.trace("%s - data identifier: %d", __func__,
            static_cast<int>(dataIdentifier));

    while (reader.getBytesLeft() > 0)
    {
        std::uint8_t dataUnitId = reader.readUint8();
        std::uint8_t dataUnitLength = reader.readUint8();

        if ((dataUnitId != SUBTITLES_UNIT_ID) and (dataUnitId != TELETEXT_UNIT_ID))
        {
            g_logger.trace("%s unsupported unit id (%d), skipping %d bytes", __func__,
                static_cast<int>(dataUnitId), static_cast<int>(dataUnitLength));
            reader.skip(dataUnitLength);
            continue;
        }

        if (dataUnitLength != TELETEXT_PACKET_LENGTH)
        {
            g_logger.info("%s - strange data unit length: %d for dataUnitId %d)", __func__,
                static_cast<int>(dataUnitLength), static_cast<int>(dataUnitId));
        }

        PesPacketReader unitReader(reader, dataUnitLength);

        processDataUnit(unitReader, dataUnitId, dataUnitLength);

        reader.skip(dataUnitLength);
    }
}

void Collector::processDataUnit(PesPacketReader& reader,
                                std::uint8_t dataUnitId,
                                std::uint8_t dataUnitLength)
{
    g_logger.trace("%s - data unit: id=%d length=%d", __func__,
            static_cast<int>(dataUnitId), static_cast<int>(dataUnitLength));

    std::uint8_t dataUnitControl = reader.readUint8();
    std::uint8_t framingCode = reader.readUint8();
    std::uint8_t mpHamming1 = reader.readUint8();
    std::uint8_t mpHamming2 = reader.readUint8();

    g_logger.trace(
            "%s - data unit: control=%d framing=%d mpHamming1=%d mpHamming2=%d",
            __func__, static_cast<int>(dataUnitControl),
            static_cast<int>(framingCode), static_cast<int>(mpHamming1),
            static_cast<int>(mpHamming2));

    const std::int8_t mpByte1 = m_hamming.decode84(mpHamming1);
    const std::int8_t mpByte2 = m_hamming.decode84(mpHamming2);

    static const std::uint8_t FRAMING_CODE = 0xE4;
    if (framingCode != FRAMING_CODE)
    {
        g_logger.trace("%s - invalid framing code (not teletext?)", __func__);
        return;
    }

    if ((mpByte1 < 0) || (mpByte2 < 0))
    {
        g_logger.info("%s - invalid magazine/packet encoding", __func__);
        return;
    }

    std::uint8_t magazine = mpByte1 & 0x07;

    std::uint8_t packetAddress = ((mpByte1 & 0x08) >> 3);
    packetAddress |= ((mpByte2 & 0x0F) << 1);

    collectTeletextPacket(magazine, packetAddress, reader);
}

void Collector::collectTeletextPacket(std::uint8_t magazine,
                                      std::uint8_t packetAddress,
                                      PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__, magazine,
            packetAddress);

    std::int8_t designationCode = 0;
    if ((packetAddress >= 26) && (packetAddress <= 31))
    {
        designationCode = m_hamming.decode84(reader.peekUint8());
    }

    CollectorPacketContextImpl context(this, reader, magazine, packetAddress,
            designationCode);

    m_listener.onPacketReady(context);
}

bool Collector::collectPacket(Packet& packet,
                              PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d type=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress(),
            static_cast<int>(packet.getType()));

    switch (packet.getType())
    {
    case PacketType::RAW:
        return collectTypedPacket(static_cast<PacketRaw&>(packet), reader);

    case PacketType::HEADER:
        return collectTypedPacket(static_cast<PacketHeader&>(packet), reader);

    case PacketType::LOP_DATA:
        return collectTypedPacket(static_cast<PacketLopData&>(packet), reader);

    case PacketType::BTT_PAGE_TYPE:
        return collectTypedPacket(static_cast<PacketBttPageType&>(packet),
                reader);

    case PacketType::EDITORIAL_LINKS:
        return collectTypedPacket(static_cast<PacketEditorialLinks&>(packet),
                reader);

    case PacketType::BCAST_SERVICE_DATA:
        return collectTypedPacket(static_cast<PacketBcastServiceData&>(packet),
                reader);

    case PacketType::TRIPLETS:
        return collectTypedPacket(static_cast<PacketTriplets&>(packet), reader);

    default:
        g_logger.fatal("%s - Unsupported packet type: %d", __func__,
                static_cast<int>(packet.getType()));
        return false;
    }
}

bool Collector::collectTypedPacket(PacketRaw& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    auto buffer = packet.getBuffer();
    auto length = packet.getBufferLength();
    for (std::size_t i = 0; i < length; ++i)
    {
        buffer[i] = reader.readUint8();
    }

    return true;
}

bool Collector::collectTypedPacket(PacketHeader& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    std::int8_t decodedHeaderBytes[8];
    for (std::size_t i = 0; i < sizeof(decodedHeaderBytes); ++i)
    {
        decodedHeaderBytes[i] = m_hamming.decode84(reader.readUint8());
        if (decodedHeaderBytes[i] < 0)
        {
            g_logger.info("%s - invalid header byte encoding", __func__);
            return false;
        }
    }

    std::uint8_t magazineNumber = packet.getMagazineNumber();
    if (magazineNumber == 0)
    {
        magazineNumber = 8;
    }

    std::uint16_t magazinePage = 0;
    magazinePage |= (decodedHeaderBytes[0] & 0x0F) << 0;
    magazinePage |= (decodedHeaderBytes[1] & 0x0F) << 4;
    magazinePage |= magazineNumber << 8;

    std::uint16_t subpage = 0;
    subpage |= (decodedHeaderBytes[2] & 0x0F) << 0;
    subpage |= (decodedHeaderBytes[3] & 0x07) << 4;
    subpage |= (decodedHeaderBytes[4] & 0x0F) << 8;
    subpage |= (decodedHeaderBytes[5] & 0x03) << 12;

    std::uint8_t controlInfo = 0;
    controlInfo |= (decodedHeaderBytes[3] & 0x08) ? ControlInfo::ERASE_PAGE : 0;
    controlInfo |= (decodedHeaderBytes[5] & 0x04) ? ControlInfo::NEWSFLASH : 0;
    controlInfo |= (decodedHeaderBytes[5] & 0x08) ? ControlInfo::SUBTITLE : 0;
    controlInfo |= (decodedHeaderBytes[6] & 0x01) ? ControlInfo::SUPRESS_HEADER : 0;
    controlInfo |= (decodedHeaderBytes[6] & 0x02) ? ControlInfo::UPDATE_INDICATOR : 0;
    controlInfo |=
            (decodedHeaderBytes[6] & 0x04) ? ControlInfo::INTERRUPTED_SEQUENCE : 0;
    controlInfo |= (decodedHeaderBytes[6] & 0x08) ? ControlInfo::INHIBIT_DISPLAY : 0;
    controlInfo |= (decodedHeaderBytes[7] & 0x01) ? ControlInfo::MAGAZINE_SERIAL : 0;

    std::uint8_t nationalOption = 0;
    nationalOption |= (decodedHeaderBytes[7] & 0x08) ? 0x01 : 0;
    nationalOption |= (decodedHeaderBytes[7] & 0x04) ? 0x02 : 0;
    nationalOption |= (decodedHeaderBytes[7] & 0x02) ? 0x04 : 0;

    g_logger.trace("%s - controlInfo=%02X", __func__, controlInfo);

    PageId pageId(magazinePage, subpage);

    packet.setPageInfo(pageId, controlInfo, nationalOption);

    collectReadableDataBytes(reader, packet.getBuffer(),
            packet.getBufferLength());

    return true;
}

bool Collector::collectTypedPacket(PacketLopData& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    collectReadableDataBytes(reader, packet.getBuffer(),
            packet.getBufferLength());

    return true;
}

bool Collector::collectTypedPacket(PacketBttPageType& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    auto buffer = packet.getBuffer();
    auto length = packet.getBufferLength();

    for (std::size_t i = 0; i < length; ++i)
    {
        buffer[i] = m_hamming.decode84(reader.readUint8());
        if (buffer[i] < 0)
        {
            g_logger.info("%s - invalid byte encoding", __func__);
            return false;
        }
    }

    return true;
}

bool Collector::collectTypedPacket(PacketEditorialLinks& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    auto magazine = packet.getMagazineNumber();

    const std::int8_t designationCode = m_hamming.decode84(reader.readUint8());
    if ((designationCode < 0) || (designationCode > 3))
    {
        g_logger.info("%s - invalid designation code: %d", __func__,
                designationCode);
        return false;
    }

    packet.setDesignationCode(designationCode);

    for (std::uint8_t index = 0; index < 6; ++index)
    {
        std::int8_t decodedLinkBytes[6];
        for (std::size_t i = 0; i < sizeof(decodedLinkBytes); ++i)
        {
            decodedLinkBytes[i] = m_hamming.decode84(reader.readUint8());
            if (decodedLinkBytes[i] < 0)
            {
                g_logger.info("%s - invalid link byte encoding", __func__);
                return false;
            }
        }

        std::uint8_t relativeMagazineNumber = 0;
        relativeMagazineNumber |= (decodedLinkBytes[3] & 0x08) ? 0x01 : 0;
        relativeMagazineNumber |= (decodedLinkBytes[5] & 0x04) ? 0x02 : 0;
        relativeMagazineNumber |= (decodedLinkBytes[5] & 0x08) ? 0x04 : 0;

        std::uint8_t linkMagazineNumber = magazine ^ relativeMagazineNumber;
        if (linkMagazineNumber == 0)
        {
            linkMagazineNumber = 8;
        }

        std::uint16_t linkMagazinePage = 0;
        linkMagazinePage |= (decodedLinkBytes[0] & 0x0F) << 0;
        linkMagazinePage |= (decodedLinkBytes[1] & 0x0F) << 4;
        linkMagazinePage |= linkMagazineNumber << 8;

        std::uint16_t linkSubpage = 0;
        linkSubpage |= (decodedLinkBytes[2] & 0x0F) << 0;
        linkSubpage |= (decodedLinkBytes[3] & 0x07) << 4;
        linkSubpage |= (decodedLinkBytes[4] & 0x0F) << 8;
        linkSubpage |= (decodedLinkBytes[5] & 0x03) << 12;

        // When no particular page number is to be specified, the page number
        // FF is transmitted. When no particular page subcode
        // is to be specified, the page sub-code 3F7F is transmitted. When the
        // page address XFF:3F7F is transmitted, no page is specified.

        PageId linkPageId(linkMagazinePage, linkSubpage);

        g_logger.trace(
                "%s - storing editorial link (index=%u mag=%04hX sub=%04hX)",
                __func__, index, linkPageId.getMagazinePage(),
                linkPageId.getSubpage());

        packet.setLink(index, linkPageId);
    }

    if (designationCode == 0)
    {
        int8_t linkControl = m_hamming.decode84(reader.readUint8());
        if (linkControl < 0)
        {
            g_logger.trace("%s - invalid link control", __func__);
            return false;
        }

        packet.setLinkControl(linkControl);

        uint16_t crc1 = reader.readUint8();
        uint16_t crc2 = reader.readUint8();

        packet.setCrc((crc1 << 8) | (crc2));
    }
    else
    {
        reader.skip(3);

        packet.setLinkControl(0xFF);
        packet.setCrc(0xFFFF);
    }

    return true;
}

bool Collector::collectTypedPacket(PacketBcastServiceData& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    int8_t designationCode = m_hamming.decode84(reader.readUint8());
    if ((designationCode < 0) || (designationCode > 3))
    {
        g_logger.trace("%s - broken designation code: %d", __func__,
                designationCode);
        return false;
    }

    packet.setDesignationCode(designationCode);

    int8_t bytes[6];

    for (int i = 0; i < 6; ++i)
    {
        bytes[i] = m_hamming.decode84(reader.readUint8());
        if (bytes[i] < 0)
        {
            g_logger.trace("%s - invalid byte found", __func__);
            return false;
        }
    }

    std::uint16_t magazineNumber = 0;
    magazineNumber |= (bytes[3] & 0x08) >> 3;
    magazineNumber |= (bytes[3] & 0x0C) >> 1;

    std::uint16_t magazinePage = 0;
    magazinePage |= (bytes[0] & 0x0F) << 0;
    magazinePage |= (bytes[1] & 0x0F) << 4;
    magazinePage |= magazineNumber << 8;

    std::uint16_t subpage = 0;
    subpage |= (bytes[2] & 0x0F) << 0;
    subpage |= (bytes[3] & 0x07) << 4;
    subpage |= (bytes[4] & 0x0F) << 8;
    subpage |= (bytes[5] & 0x03) << 12;

    packet.setInitialPage(PageId(magazinePage, subpage));

    // skipping service info data
    reader.skip(25 - 13 + 1);

    auto statusDisplayBuffer = packet.getStatusDisplayBuffer();
    auto statusDisplayLength = packet.getStatusDisplayBufferLength();
    for (std::size_t i = 0; i < statusDisplayLength; ++i)
    {
        statusDisplayBuffer[i] = m_hamming.decodeParity(reader.readUint8());
        if (statusDisplayBuffer[i] < 0)
        {
            g_logger.trace("%s - invalid status display byte", __func__);
            return false;
        }
    }

    return true;
}

bool Collector::collectTypedPacket(PacketTriplets& packet,
                                   PesPacketReader& reader)
{
    g_logger.trace("%s - magazine=%d packet=%d", __func__,
            packet.getMagazineNumber(), packet.getPacketAddress());

    int8_t designationCode = m_hamming.decode84(reader.readUint8());
    if (designationCode < 0)
    {
        g_logger.trace("%s - broken designation code", __func__);
        return false;
    }

    packet.setDesignationCode(designationCode);

    for (std::size_t i = 0; i < 13; ++i)
    {
        auto byte1 = reader.readUint8();
        auto byte2 = reader.readUint8();
        auto byte3 = reader.readUint8();

        auto value = m_hamming.decode2418(byte1, byte2, byte3);
        if (value < 0)
        {
            g_logger.trace("%s - broken triplet: %i", __func__,
                    static_cast<int>(i));
            return false;
        }

        packet.setTripletValue(i, value);
    }

    return true;
}

void Collector::collectReadableDataBytes(PesPacketReader& reader,
                                         int8_t* buffer,
                                         std::size_t bufferLen)
{
    for (std::size_t i = 0; i < bufferLen; ++i)
    {
        std::uint8_t dataByte = reader.readUint8();

        std::int8_t dataChar = m_hamming.decodeParity(dataByte);
        if (dataChar >= 0)
        {
            buffer[i] = static_cast<char>(dataChar);
        }
        else
        {
            buffer[i] = ' ';
        }
    }
}

} // namespace ttxdecoder
