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

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/DataBuffer.hpp>

#include <cstdint>
#include <map>
#include <memory>

namespace subttxrend
{
namespace protocol
{

class Packet;

/**
 * Parser for packets.
 *
 * Please note that the parser does not copy the parsed data
 * and does not create new objects for parsed packets. That means
 * the parsed data must remain valid as long as the parsed object
 * is used and the parsed objects are valid until the parser is valid
 * and until the parsing of next packet is called.
 */
class PacketParser : private common::NonCopyable
{
public:
    /**
     * Constructor.
     */
    PacketParser();

    /**
     * Destructor.
     */
    virtual ~PacketParser() = default;

    /**
     * Parses the data.
     *
     * @param data
     *      Data to parse.
     * @param size
     *      Size of the data in bytes.
     *
     * @return
     *      Parsed packet. Please note that the packet may be invalid.
     */
    const Packet& parse(common::DataBufferPtr packetBuffer);

private:
    /**
     * Adds parsable packet to be handled.
     *
     * @param packet
     *      Packet object.
     */
    void addParsablePacket(std::unique_ptr<Packet> packet);

    /** Map of parsable packets. */
    std::map<std::uint32_t, std::unique_ptr<Packet>> m_packets;

    /** Invalid packet instance. */
    std::unique_ptr<Packet> m_invalidPacket;

    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

