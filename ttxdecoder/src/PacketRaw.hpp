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


#ifndef TTXDECODER_PACKETRAW_HPP_
#define TTXDECODER_PACKETRAW_HPP_

#include "Packet.hpp"

namespace ttxdecoder
{

/**
 * Raw packet.
 *
 * Hold bytes as they were found in stream.
 */
class PacketRaw : public Packet
{
public:
    /** @copydoc Packet::getType() */
    virtual PacketType getType() const override
    {
        return PacketType::RAW;
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
    std::uint8_t* getBuffer()
    {
        return m_buffer;
    }

    /**
     * Returns data buffer.
     *
     * @return
     *      Data buffer.
     */
    const std::uint8_t* getBuffer() const
    {
        return m_buffer;
    }

private:
    /** Buffer length. */
    static const std::size_t BUFFER_LENGTH = 40;

    /** Data buffer. */
    std::uint8_t m_buffer[BUFFER_LENGTH];
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PACKETRAW_HPP_*/
