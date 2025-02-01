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


#include "PesBuffer.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include <subttxrend/common/Logger.hpp>

#include "AllocatorTraits.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "PesBuffer");

} // namespace <anonmymous>

PesBuffer::PesBuffer(Allocator& allocator) :
        m_bufferArray(AllocatorTraits(allocator).allocUnique<SizedArray<std::uint8_t, 2 * 64 * 1024>>()),
        m_buffer(m_bufferArray->getData()),
        m_size(m_bufferArray->getSize()),
        m_used(0),
        m_readOffset(0),
        m_writeOffset(0)
{
    g_logger.trace("%s", __func__);
}

PesBuffer::~PesBuffer()
{
    g_logger.trace("%s", __func__);
}

bool PesBuffer::addPesPacket(const std::uint8_t* const packet,
                             const std::uint16_t length)
{
    const std::uint8_t* packetData = packet;
    std::size_t packetLength = length;
    std::size_t sizeLeft = m_size - m_used;

    if (packetLength > sizeLeft)
    {
        g_logger.info(
                "%s - Data dropped - not enough space. Needed: %d, space: %d",
                __func__, static_cast<int>(packetLength),
                static_cast<int>(sizeLeft));
        return false;
    }

    if (packetLength < 6)
    {
        g_logger.info("%s - Data dropped - invalid packet size. Size: %d",
                __func__, static_cast<int>(packetLength));
        return false;
    }

    if ((packetData[0] != 0) || (packetData[1] != 0) || (packetData[2] != 1)
            || (packetData[3] != 0xBD))
    {
        g_logger.info("%s - Data dropped - invalid packet header.", __func__);
        return false;
    }

    std::uint16_t pesLength1 = packetData[4] & 0xFF;
    std::uint16_t pesLength2 = packetData[5] & 0xFF;
    std::uint16_t pesLength = (pesLength1 << 8) | pesLength2;

    if (pesLength == 0)
    {
        g_logger.info("%s - Data dropped - empty PES packet.", __func__);
        return false;
    }

    if (pesLength != packetLength - 6)
    {
        g_logger.info(
                "%s - Data dropped - invalid PES length (found: %d, expected: %d).",
                __func__, static_cast<int>(pesLength),
                static_cast<int>(packetLength - 6));
        return false;
    }

    // copy first chunk (safe as space left was already checked)
    const std::size_t maxSizeChunk1 = m_size - m_writeOffset;
    if (maxSizeChunk1 > 0)
    {
        std::size_t bytesToCopy = std::min(maxSizeChunk1, packetLength);

        (void) std::memcpy(&m_buffer[m_writeOffset], packetData, bytesToCopy);

        packetData += bytesToCopy;
        packetLength -= bytesToCopy;

        m_writeOffset += bytesToCopy;
        if (m_writeOffset == m_size)
        {
            m_writeOffset = 0;
        }
    }

    // copy second chunk (safe as space left was already checked)
    const std::size_t maxSizeChunk2 = m_size - m_writeOffset;
    if ((maxSizeChunk2 > 0) && (packetLength > 0))
    {
        std::size_t bytesToCopy = std::min(maxSizeChunk2, packetLength);

        (void) std::memcpy(&m_buffer[m_writeOffset], packetData, bytesToCopy);

        m_writeOffset += bytesToCopy;
        if (m_writeOffset == m_size)
        {
            m_writeOffset = 0;
        }
    }

    m_used += length;

    g_logger.trace("%s - Pes added (size=%d, left=%d)", __func__,
            static_cast<int>(length), static_cast<int>(m_size - m_used));

    return true;
}

void PesBuffer::clear()
{
    g_logger.trace("%s - Buffer cleared (left=%d)", __func__,
            static_cast<int>(m_size - m_used));

    m_used = 0;
    m_readOffset = 0;
    m_writeOffset = 0;
}

bool PesBuffer::getNextPacket(StcTimeType timeType,
                              PesPacketHeader& header,
                              PesPacketReader& dataReader)
{
    if (m_used == 0)
    {
        return false;
    }

    // first chunk length
    std::size_t maxSizeChunk1 = m_size - m_readOffset;
    if (maxSizeChunk1 > m_used)
    {
        maxSizeChunk1 = m_used;
    }

    // second chunk length
    std::size_t maxSizeChunk2 = m_used - maxSizeChunk1;

    // prepare reader
    PesPacketReader allDataReader(&m_buffer[m_readOffset], maxSizeChunk1,
            &m_buffer[0], maxSizeChunk2);

    try
    {
        readHeader(timeType, allDataReader, header);

        std::uint32_t consumedBytes = m_used - allDataReader.getBytesLeft();

        std::uint32_t packetSize = header.getTotalSize();

        if (packetSize > m_used)
        {
            throw PesPacketReader::Exception(
                    "Not enough bytes for PES packet.");
        }

        if (packetSize < consumedBytes)
        {
            throw PesPacketReader::Exception(
                    "Invalid packet length - smaller than header");
        }

        // build data reader

        maxSizeChunk1 = m_size - m_readOffset;
        if (maxSizeChunk1 > packetSize)
        {
            maxSizeChunk1 = packetSize;
        }

        maxSizeChunk2 = packetSize - maxSizeChunk1;

        dataReader = PesPacketReader(&m_buffer[m_readOffset], maxSizeChunk1,
                &m_buffer[0], maxSizeChunk2);

        dataReader.skip(consumedBytes);

        return true;
    }
    catch (PesPacketReader::Exception& e)
    {
        g_logger.warning("%s - forcing reset, error: %s", __func__, e.what());

        // handle issues silently
        clear();

        return false;
    }
}

void PesBuffer::markPacketConsumed(const PesPacketHeader& header)
{
    auto packetSize = header.getTotalSize();

    m_readOffset += packetSize;
    m_readOffset %= m_size;

    m_used -= packetSize;

    g_logger.trace("%s - Packet consumed (consumed=%d, left=%d)", __func__,
            static_cast<int>(packetSize), static_cast<int>(m_size - m_used));
}

void PesBuffer::readHeader(StcTimeType timeType,
                           PesPacketReader& reader,
                           PesPacketHeader& header)
{
    auto startCodePrefix1 = reader.readUint8();
    auto startCodePrefix2 = reader.readUint8();
    auto startCodePrefix3 = reader.readUint8();

    g_logger.trace("%s - start_code=%02X:%02X:%02X", __func__,
            startCodePrefix1, startCodePrefix2, startCodePrefix3);

    if ((startCodePrefix1 != 0x00) || (startCodePrefix2 != 0x00)
            || (startCodePrefix3 != 0x01))
    {
        throw PesPacketReader::Exception("Invalid start code prefix");
    }

    header.m_streamId = reader.readUint8();

    g_logger.trace("%s - stream_id=%02X", __func__, header.m_streamId);

    uint16_t pesPacketLength1 = reader.readUint8();
    uint16_t pesPacketLength2 = reader.readUint8();

    header.m_pesPacketLength = (pesPacketLength1 << 8) | pesPacketLength2;

    g_logger.trace("%s - pes_packet_length=%d", __func__,
            header.m_pesPacketLength);

    header.m_hasPts = false;
    header.m_pts = StcTime();

    if (header.isSubtitlesPacket())
    {
        auto controlByte1 = reader.readUint8();
        auto controlByte2 = reader.readUint8();

        g_logger.trace("%s - control_bytes=%02X:%02X", __func__, controlByte1,
                controlByte2);

        auto pesHeaderLength = reader.readUint8();

        g_logger.trace("%s - pes_header_length=%02X", __func__,
                pesHeaderLength);

        auto ptsDtsFlags = (controlByte2 >> 6) & 0x03;

        g_logger.trace("%s - pts_dts_flags=%u", __func__, ptsDtsFlags);

        if ((ptsDtsFlags == 3) || (ptsDtsFlags == 2))
        {
            if (pesHeaderLength < 5)
            {
                throw PesPacketReader::Exception("Invalid PES header length");
            }

            uint32_t ptsByte1 = reader.readUint8();
            uint32_t ptsByte2 = reader.readUint8();
            uint32_t ptsByte3 = reader.readUint8();
            uint32_t ptsByte4 = reader.readUint8();
            uint32_t ptsByte5 = reader.readUint8();

            uint32_t pts = 0;

            if (timeType == StcTimeType::HIGH_32)
            {
                // 1 - bits 3..1 (3) to bits 31..29
                // 2 - bits 7..0 (8) to bits 28..21
                // 3 - bits 7..1 (7) to bits 20..14
                // 4 - bits 7..0 (8) to bits 13..06
                // 5 - bits 7..2 (6) to bits 05..00

                pts |= (ptsByte1 << (29 - 1)) & 0xE0000000;
                pts |= (ptsByte2 << (21 - 0)) & 0x1FE00000;
                pts |= (ptsByte3 << (14 - 1)) & 0x001FC000;
                pts |= (ptsByte4 << (6 - 0))  & 0x00003FC0;
                pts |= (ptsByte5 >> (2 - 0))  & 0x0000003F;

                header.m_hasPts = true;
                header.m_pts = StcTime(timeType, pts);
            }
            else if (timeType == StcTimeType::LOW_32)
            {
                // 1 - bits 2..1 (2) to bits 31..30
                // 2 - bits 7..0 (8) to bits 29..22
                // 3 - bits 7..1 (7) to bits 21..15
                // 4 - bits 7..0 (8) to bits 14..07
                // 5 - bits 7..1 (7) to bits 06..00

                pts |= (ptsByte1 << (30 - 1)) & 0xC0000000;
                pts |= (ptsByte2 << (22 - 0)) & 0x3FC00000;
                pts |= (ptsByte3 << (15 - 1)) & 0x003F8000;
                pts |= (ptsByte4 << (7 - 0))  & 0x00007F80;
                pts |= (ptsByte5 >> (1 - 0))  & 0x0000007F;

                header.m_hasPts = true;
                header.m_pts = StcTime(timeType, pts);
            }
            else
            {
                g_logger.fatal("%s - unknown time type", __func__);
            }

            g_logger.trace("%s - PTS=%u", __func__, header.m_pts.m_time);

            // skip rest of header
            reader.skip(pesHeaderLength - 5);
        }
        else
        {
            // skip header
            reader.skip(pesHeaderLength);
        }
    }
}

} // namespace dvbsubdecoder
