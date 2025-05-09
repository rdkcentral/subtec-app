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

#pragma once

#include <subttxrend/common/DataBuffer.hpp>
#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>

#include <array>
#include <cstdint>
#include <unordered_map>

namespace subttxrend
{
namespace protocol
{

class Buffer;
class BufferReader;

/**
 * Base class for data packets.
 */
class Packet : private common::NonCopyable
{
public:

    /**
     * Packet type.
     */
    enum class Type : std::uint32_t
    {
        PES_DATA = 1,
        TIMESTAMP = 2,
        RESET_ALL = 3,
        RESET_CHANNEL = 4,
        SUBTITLE_SELECTION = 5,
        TELETEXT_SELECTION = 6,
        TTML_SELECTION = 7,
        TTML_DATA = 8,
        TTML_TIMESTAMP = 9,
        CC_DATA = 10,
        PAUSE = 11,
        RESUME = 12,
        MUTE = 13,
        UNMUTE = 14,
        WEBVTT_SELECTION = 15,
        WEBVTT_DATA = 16,
        WEBVTT_TIMESTAMP = 17,
        SET_CC_ATTRIBUTES = 18,
        TTML_INFO = 19,
        FLUSH = 20,

        MAX,
        INVALID = 0xFFFFFFFF,
    };

    /**
     * Constructor.
     */
    Packet();

    /**
     * Destructor.
     */
    virtual ~Packet() = default;

    /**
     * Checks if given packet is valid.
     *
     * @retval true
     *      Packet is valid (type is not INVALID and packet was
     *      successfully parsed).
     * @retval false
     *      Packet is not valid.
     */
    bool isValid() const;

    /**
     * Returns type of the packet.
     *
     * @return
     *      Packet type.
     */
    virtual Type getType() const = 0;

    /**
     * Returns counter field value.
     *
     * @return
     *      Counter field value.
     */
    std::uint32_t getCounter() const;

    /**
     * Returns size field value.
     *
     * @return
     *      Size field value.
     */
    std::uint32_t getSize() const;

    /**
     * Parses the packet.
     *
     * @param dataBuffer
     *      Buffer with data to parse.
     *
     * @retval true
     *      Packet was successfully parsed.
     * @retval false
     *      Packet parsing failed.
     */
    bool parse(common::DataBufferPtr dataBuffer);

    /**
     * Extracts packet type from given data.
     *
     * It takes value from the buffer, there is no checking if it
     * matches any of the defined Type values.
     *
     * @param buffer
     *      Buffer with data to parse.
     *
     * @return
     *      Packet type (as uint32_t).
     *      Value for INVALID type is used when packet type cannot be extracted.
     */
    static std::uint32_t extractType(const Buffer& buffer);

    /**
     * Similar to @extractType but returns Type not int value.
     *
     * @param dataBuffer
     *      Packet buffer.
     *
     * @return
     *      Packet type.
     */
    static Type getType(const common::DataBuffer& dataBuffer);

    /**
     * Reads packet counter from data buffer.
     *
     * @return
     *      Read packet counter value.
     */
    static std::uint32_t getCounter(const common::DataBuffer& dataBuffer);

    /**
     * Checks if given type if data packet.
     *
     * @param type
     * @return
     *      True for packets containing subtitle data, false otherwise.
     */
    static bool isDataPacket(Type type);

    /**
     * Header size getter.
     *
     * @return
     *      Size of a packet header.
     */
    static std::uint32_t getHeaderSize();

    /**
     * Extracts packet size from header.
     *
     * @return
     *      Size of a packet.
     */
    static std::uint32_t getSizeFromHeader(const common::DataBuffer& dataBuffer);

protected:
    /**
     * Parses the packet data.
     *
     * This method is called from parse().
     *
     * Following packet elements are already extracted at this point:
     * - Type (also checked against this packet type).
     * - Counter.
     * - Size (also checked against size of the data).
     *
     * @param bufferReader
     *      Buffer with data to parse.
     *
     * @retval true
     *      Packet was successfully parsed.
     * @retval false
     *      Packet parsing failed.
     */
    virtual bool parseDataHeader(BufferReader& bufferReader) = 0;

    /**
     * Transfers ownership of the buffer. Used for packets that carry subtitle data.
     *
     * @param dataBuffer
     *      Buffer with packet contents.
     * @param dataOffset
     *      Offset in the buffer where data starts.
     * @return
     *      True if it took the buffer, false if buffer was not needed.
     */
    virtual bool takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset) = 0;

private:
    /** Packet valid flag. */
    bool m_valid;

    /** Counter value. */
    std::uint32_t m_counter;

    /** Size value. */
    std::uint32_t m_size;

    /** Logger. */
    static common::Logger m_logger;
};
static const std::unordered_map<Packet::Type, std::string, std::hash<Packet::Type>> packetTypeStr = {
        {Packet::Type::PES_DATA, "PES_DATA"},
        {Packet::Type::TIMESTAMP, "TIMESTAMP"},
        {Packet::Type::RESET_ALL, "RESET_ALL"},
        {Packet::Type::RESET_CHANNEL, "RESET_CHANNEL"},
        {Packet::Type::SUBTITLE_SELECTION, "SUBTITLE_SELECTION"},
        {Packet::Type::TELETEXT_SELECTION, "TELETEXT_SELECTION"},
        {Packet::Type::TTML_SELECTION, "TTML_SELECTION"},
        {Packet::Type::TTML_DATA, "TTML_DATA"},
        {Packet::Type::TTML_TIMESTAMP, "TTML_TIMESTAMP"},
        {Packet::Type::CC_DATA , "CC_DATA"},
        {Packet::Type::PAUSE , "PAUSE"},
        {Packet::Type::RESUME , "RESUME"},
        {Packet::Type::MUTE , "MUTE"},
        {Packet::Type::UNMUTE , "UNMUTE"},
        {Packet::Type::WEBVTT_SELECTION,"WEBVTT_SELECTION"},
        {Packet::Type::WEBVTT_DATA,"WEBVTT_DATA"},
        {Packet::Type::WEBVTT_TIMESTAMP,"WEBVTT_TIMESTAMP"},
        {Packet::Type::SET_CC_ATTRIBUTES,"SET_CC_ATTRIBUTES"},
        {Packet::Type::TTML_INFO,"TTML_INFO"},
        {Packet::Type::FLUSH, "FLUSH"}
};

inline std::ostream& operator<<(std::ostream& out, Packet::Type packetType)
{
    auto index = static_cast<int>(packetType);
    std::string str = "INVALID(" + std::to_string(index)+")";
    if (packetTypeStr.find(packetType)!= packetTypeStr.end())
    {
      str = packetTypeStr.at(packetType);
    }
    return out << "type:" << str;
}

inline std::ostream& operator<<(std::ostream& out, Packet const& packet)
{
    return out << packet.getType() << " counter=" << packet.getCounter();
}

class PacketGeneric : public Packet
{
  public:
    PacketGeneric(Type type);
    Type getType() const override;

  protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

    /** @copydoc Packet::takeBuffer */
    bool takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset) override;

  private:
    Type m_type{};
};

class PacketChannelSpecific : public PacketGeneric
{
  public:
    PacketChannelSpecific(Type type);
    /**
     * Returns channel ID.
     *
     * @return
     *      Channel id.
     */
    std::uint32_t getChannelId() const;

  protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

    /** @copydoc Packet::takeBuffer */
    bool takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset) override;

  private:
    /** Channel ID. */
    std::uint32_t m_channelId{};
};

} // namespace protocol
} // namespace subttxrend

