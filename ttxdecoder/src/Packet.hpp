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


#ifndef TTXDECODER_PACKET_HPP_
#define TTXDECODER_PACKET_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Packet type.
 */
enum class PacketType
{
    /** Raw packet type. */
    RAW,

    /** Header. */
    HEADER,

    /** LOP data. */
    LOP_DATA,

    /** BTT - page type. */
    BTT_PAGE_TYPE,

    /** Editorial links. */
    EDITORIAL_LINKS,

    /** Broadcast service data. */
    BCAST_SERVICE_DATA,

    /** Enhancement (triplets). */
    TRIPLETS
};

/**
 * Packet base class.
 */
class Packet
{
public:
    /**
     * Constructor.
     */
    Packet() :
            m_magazineNumber(),
            m_packetAddress()
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~Packet() = default;

    /**
     * Returns packet type.
     *
     * @return
     *      Packet type.
     */
    virtual PacketType getType() const = 0;

    /**
     * Returns magazine number.
     *
     * @return
     *      Magazine number (0-7).
     */
    std::uint8_t getMagazineNumber() const
    {
        return m_magazineNumber;
    }

    /**
     * Sets magazine number.
     *
     * @param magazineNumber
     *      Magazine number.
     */
    void setMagazineNumber(const std::uint8_t magazineNumber)
    {
        m_magazineNumber = magazineNumber;
    }

    /**
     * Returns packet address.
     *
     * @return
     *      Packet address (0-31).
     */
    std::uint8_t getPacketAddress() const
    {
        return m_packetAddress;
    }

    /**
     * Sets packet address.
     *
     * @param packetAddress
     *      Packet address.
     */
    void setPacketAddress(const std::uint8_t packetAddress)
    {
        m_packetAddress = packetAddress;
    }

private:
    /** Magazine number. */
    std::uint8_t m_magazineNumber;

    /** Packet address. */
    std::uint8_t m_packetAddress;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKET_HPP_*/
