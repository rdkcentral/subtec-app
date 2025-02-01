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


#ifndef TTXDECODER_COLLECTORLISTENER_HPP_
#define TTXDECODER_COLLECTORLISTENER_HPP_

#include <cstdint>

namespace ttxdecoder
{

class Packet;

/**
 * Collector packet context.
 *
 * The context is used when new packet is ready to be collected.
 */
class CollectorPacketContext
{
public:
    /**
     * Constructor.
     *
     * @param magazineNumber
     *      Magazine number (0-7).
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     */
    CollectorPacketContext(std::uint8_t magazineNumber,
                           std::uint8_t packetAddress,
                           std::int8_t designationCode) :
            m_magazineNumber(magazineNumber),
            m_packetAddress(packetAddress),
            m_designationCode(designationCode)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~CollectorPacketContext() = default;

    /**
     * Requests the packet consumption.
     *
     * @param packet
     *      Packet to be filled with consumed data.
     *
     * @retval true
     *      Data consumed successfully.
     *      Packet is filled with valid data.
     * @retval false
     *      Consumption error (invalid or not enough data, etc.).
     */
    virtual bool consume(Packet& packet) = 0;

    /**
     * Returns magazine number.
     *
     * @return
     *      Magazine number.
     */
    std::uint8_t getMagazineNumber() const
    {
        return m_magazineNumber;
    }

    /**
     * Returns packet address.
     *
     * @return
     *      Packet address.
     */
    std::uint8_t getPacketAddress() const
    {
        return m_packetAddress;
    }

    /**
     * Returns designation code.
     *
     * Only valid when defined by packet type.
     *
     * @return
     *      Designation code. Invalid means hamming decoding failed.
     */
    std::int8_t getDesignationCode() const
    {
        return m_designationCode;
    }

private:
    /** Magazine number (0-7). */
    const std::uint8_t m_magazineNumber;

    /** Packet address (0-31). */
    const std::uint8_t m_packetAddress;

    /** Designation code. Only valid when defined by packet type. */
    const std::int8_t m_designationCode;
};

/**
 * Listener for collector events.
 */
class CollectorListener
{
public:
    /**
     * Constructor.
     */
    CollectorListener() = default;

    /**
     * Destructor.
     */
    virtual ~CollectorListener() = default;

    /**
     * Called when packet is ready to be collected.
     *
     * @param context
     *      Packet context. Provides information about packet type
     *      and allows to consume the packet data.
     */
    virtual void onPacketReady(CollectorPacketContext& context) = 0;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_COLLECTORLISTENER_HPP_*/
