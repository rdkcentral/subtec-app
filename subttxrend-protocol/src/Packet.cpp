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


#include "Buffer.hpp"
#include "BufferReader.hpp"
#include "Packet.hpp"

#include <cassert>

namespace subttxrend
{
namespace protocol
{

namespace
{
std::size_t constexpr HEADER_SIZE = 4 /* type */ + 4 /*counter*/ + 4 /* size */;
}
common::Logger Packet::m_logger("Protocol", "Packet");

Packet::Packet() :
        m_valid(false),
        m_counter(0),
        m_size(0)
{
    // noop
}

bool Packet::isValid() const
{
    return m_valid;
}

std::uint32_t Packet::getCounter() const
{
    return m_counter;
}

std::uint32_t Packet::getSize() const
{
    return m_size;
}

bool Packet::parse(common::DataBufferPtr dataBuffer)
{
    m_valid = false;

    const std::uint32_t thisType = static_cast<std::uint32_t>(getType());
    std::uint32_t bufferType = 0;

    BufferReader bufferReader(Buffer(dataBuffer->data(), dataBuffer->size()));

    m_logger.ostrace("parsing packet type: ", thisType);
    if (!bufferReader.extractLeUint32(bufferType))
    {
        m_logger.oswarning(__LOGGER_FUNC__, " - Cannot extract packet type");
        return false;
    }

    if (!bufferReader.extractLeUint32(m_counter))
    {
        m_logger.oswarning(__LOGGER_FUNC__, " - Cannot extract packet counter");
        return false;
    }

    if (!bufferReader.extractLeUint32(m_size))
    {
        m_logger.oswarning(__LOGGER_FUNC__, " - Cannot extract packet size");
        return false;
    }

    if (bufferType != thisType)
    {
        m_logger.oswarning(__LOGGER_FUNC__,
            " - Received packet type was invalid - expected: ",
            thisType,
            ", found: ",
            bufferType);
        return false;
    }

    std::uint32_t dataBytesAvailable = bufferReader.getSize() - bufferReader.getOffset();

    if (dataBytesAvailable != m_size)
    {
        m_logger.oswarning(__LOGGER_FUNC__,
                " - Invalid number of data bytes - expected: ", m_size,", found: ", dataBytesAvailable);
        return false;
    }

    if (!parseDataHeader(bufferReader))
    {
        m_logger.oswarning(__LOGGER_FUNC__, " - Broken packet contents");
        return false;
    }

    if (bufferReader.getOffset() != bufferReader.getSize())
    {
        if (!takeBuffer(std::move(dataBuffer), bufferReader.getOffset()))
        {
            m_logger.oswarning(__LOGGER_FUNC__, " - Not all bytes consumed offset: ",
                bufferReader.getOffset(), " size: ", bufferReader.getSize(), " for type: ", thisType);
            return false;
        }
    }

    m_valid = true;

    return true;
}

std::uint32_t Packet::extractType(const Buffer& buffer)
{
    const auto INVALID_VALUE = static_cast<std::uint32_t>(Type::INVALID);

    std::uint32_t type = INVALID_VALUE;

    if (buffer.extractLeUint32(0, type))
    {
        return type;
    }
    else
    {
        return INVALID_VALUE;
    }
}

Packet::Type Packet::getType(const common::DataBuffer& dataBuffer)
{
    return static_cast<Packet::Type>(extractType(Buffer(dataBuffer.data(), dataBuffer.size())));
}

bool Packet::isDataPacket(Type type)
{
    return (type == Type::TTML_DATA) || (type == Type::PES_DATA) || (type == Type::CC_DATA) || (type == Type::WEBVTT_DATA);
}

std::uint32_t Packet::getHeaderSize()
{
    return HEADER_SIZE;
}

std::uint32_t Packet::getSizeFromHeader(const common::DataBuffer& headerBuffer)
{
    assert(headerBuffer.size() >= HEADER_SIZE);

    auto constexpr PACKET_SIZE_OFFSET = 4 + 4;
    std::uint32_t sizeFromHeader{0};
    Buffer buffer{headerBuffer.data(), headerBuffer.size()};

    buffer.extractLeUint32(PACKET_SIZE_OFFSET, sizeFromHeader);
    return sizeFromHeader;
}

std::uint32_t Packet::getCounter(const common::DataBuffer& dataBuffer)
{
    std::uint32_t counter = 0;
    Buffer buffer{dataBuffer.data(), dataBuffer.size()};

    if (buffer.extractLeUint32(4, counter))
    {
        return counter;
    }
    else
    {
        return 0;
    }
}

PacketGeneric::PacketGeneric(Type type)
    : m_type{type}
{
}

Packet::Type PacketGeneric::getType() const
{
    return m_type;
}

bool PacketGeneric::parseDataHeader(BufferReader&)
{
    return true;
}

bool PacketGeneric::takeBuffer(common::DataBufferPtr, std::size_t)
{
    return false;
}

PacketChannelSpecific::PacketChannelSpecific(Type type)
    : PacketGeneric(type)
{
}

std::uint32_t PacketChannelSpecific::getChannelId() const
{
    return m_channelId;
}

bool PacketChannelSpecific::parseDataHeader(BufferReader& bufferReader)
{
    if (PacketGeneric::parseDataHeader(bufferReader)) {
        return bufferReader.extractLeUint32(m_channelId);
    }
    return false;
}

bool PacketChannelSpecific::takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset)
{
    return false;
}

} // namespace protocol
} // namespace subttxrend
