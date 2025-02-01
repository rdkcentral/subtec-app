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


#ifndef TTXDECODER_PACKETLOPDATA_HPP_
#define TTXDECODER_PACKETLOPDATA_HPP_

#include "Packet.hpp"

namespace ttxdecoder
{

/**
 * LOP data packet.
 *
 * Used for display data packets - X/1-X/23 LOP, X/24 FLOF, X/25.
 */
class PacketLopData : public Packet
{
public:
    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::LOP_DATA;
    }

    /**
     * Returns data buffer length.
     *
     * @return
     *      Data buffer length in bytes.
     */
    std::size_t getBufferLength() const
    {
        return BUFFER_LENGTH;
    }

    /**
     * Returns data buffer.
     *
     * @return
     *      Data buffer.
     */
    std::int8_t* getBuffer()
    {
        return m_buffer;
    }

    /**
     * Returns data buffer.
     *
     * @return
     *      Data buffer.
     */
    const std::int8_t* getBuffer() const
    {
        return m_buffer;
    }

private:
    /** Number of the data bytes. */
    static const std::size_t BUFFER_LENGTH = 40;

    /** Data buffer. */
    std::int8_t m_buffer[BUFFER_LENGTH];
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKETLOPDATA_HPP_*/
