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


#ifndef TTXDECODER_COLLECTOR_HPP_
#define TTXDECODER_COLLECTOR_HPP_

#include <subttxrend/common/NonCopyable.hpp>
#include <cstdint>

#include "Hamming.hpp"

namespace ttxdecoder
{

class PesPacketReader;
class CollectorListener;
class PageId;
class Packet;
class PacketRaw;
class PacketHeader;
class PacketLopData;
class PacketBttPageType;
class PacketEditorialLinks;
class PacketBcastServiceData;
class PacketTriplets;

class CollectorPacketContextImpl;

/**
 * Collector.
 *
 * Collects the page data.
 *
 * @note Internal decoding methods may throw PesPacketReader::Exception
 *       when the data cannot be read from the reader.
 */
class Collector : private subttxrend::common::NonCopyable
{
    friend class CollectorPacketContextImpl;

public:
    /**
     * Constructor.
     *
     * @param listener
     *      Collector events listener.
     */
    Collector(CollectorListener& listener);

    /**
     * Destructor.
     */
    virtual ~Collector();

    /**
     * Resets collector state.
     *
     * All currently collected data is dropped.
     */
    void reset();

    /**
     * Processes PES packet data.
     *
     * @param reader
     *      Reader for PES packet data.
     *
     * @throws PesPacketReader::Exception
     *      If data cannot be read from reader.
     */
    void processPacketData(PesPacketReader& reader);

private:
    /**
     * Processes PES teletext data unit.
     *
     * @param reader
     *      Reader to use (unit data contents only).
     * @param dataUnitId
     *      Data unit identifier.
     * @param dataUnitLength
     *      Data unit length.
     */
    void processDataUnit(PesPacketReader& reader,
                         std::uint8_t dataUnitId,
                         std::uint8_t dataUnitLength);

    /**
     * Collects teletext packet.
     *
     * @param magazine
     *      Magazine number.
     * @param packetAddress
     *      Packet address.
     * @param reader
     *      Reader to use.
     */
    void collectTeletextPacket(std::uint8_t magazine,
                               std::uint8_t packetAddress,
                               PesPacketReader& reader);

    /**
     * @name    Packet collecting methods.
     * @brief   Collects packet.
     *
     * @param packet
     *      Packet data storage.
     * @param reader
     *      Reader to use.
     *
     * @retval true
     *      Collecting finished successfully.
     * @retval false
     *      Collecting failed.
     */
    /** @{ */
    bool collectPacket(Packet& packet,
                       PesPacketReader& reader);

    bool collectTypedPacket(PacketRaw& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketHeader& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketLopData& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketBttPageType& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketEditorialLinks& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketBcastServiceData& packet,
                            PesPacketReader& reader);

    bool collectTypedPacket(PacketTriplets& packet,
                            PesPacketReader& reader);
    /** @} */

    /**
     * Collects readable data bytes.
     *
     * Used to collect human readable bytes from X/0-X/25 packets.
     *
     * @param reader
     *      Reader to use.
     * @param buffer
     *      Buffer for character data.
     * @param bufferLen
     *      Length of the line buffer.
     */
    void collectReadableDataBytes(PesPacketReader& reader,
                                  int8_t* buffer,
                                  std::size_t bufferLen);

    /** Collector events listener. */
    CollectorListener& m_listener;

    /** Hamming code decoder. */
    Hamming m_hamming;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_COLLECTOR_HPP_*/
