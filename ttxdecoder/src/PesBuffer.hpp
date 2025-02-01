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


#ifndef TTXDECODER_PESBUFFER_HPP_
#define TTXDECODER_PESBUFFER_HPP_

#include <subttxrend/common/NonCopyable.hpp>

#include "PesPacketReader.hpp"

namespace ttxdecoder
{

/**
 * Buffer for PES packets.
 */
class PesBuffer : private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param buffer
     *      Buffer for the data.
     * @param size
     *      Size of the buffer in bytes.
     */
    PesBuffer(std::uint8_t* buffer,
              std::size_t size);

    /**
     * Destructor.
     */
    ~PesBuffer();

    /**
     * Add a PES packet to the Teletext engine.
     *
     * @param packet
     *      Pointer to packet memory.
     * @param length
     *      Length of packet in bytes.
     *
     * @retval true
     *      Packet successfully added.
     * @retval false
     *      Operation failed.
     */
    bool addPesPacket(const std::uint8_t* packet,
                      std::uint16_t length);

    /**
     * Clears buffer contents.
     */
    void clear();

    /**
     * Returns next packet.
     *
     * @param header
     *      Variable to be filled with header information.
     * @param dataReader
     *      Variable where reader to data will be stored.
     *
     * @return
     *      True if next packet is available, false otherwise.
     */
    bool getNextPacket(PesPacketHeader& header,
                       PesPacketReader& dataReader);

    /**
     * Marks the packet returned by getNextPacket() as consumed.
     *
     * Calling this method is required to free space in the buffer for
     * next packets.
     *
     * @param header
     *      Header with data returned by getNextPacket().
     */
    void markPacketConsumed(const PesPacketHeader& header);

private:
    /**
     * Reads packet header.
     *
     * @param reader
     *      Data reader to use.
     * @param header
     *      Header to be filled with data.
     */
    void readHeader(PesPacketReader& reader,
                    PesPacketHeader& header);

    /** Buffer to store data. */
    std::uint8_t* const m_buffer;

    /** Size of the buffer. */
    const std::size_t m_size;

    /** Number of currently used bytes. */
    std::size_t m_used;

    /** Current read offset. */
    std::size_t m_readOffset;

    /** Current write offset. */
    std::size_t m_writeOffset;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PESBUFFER_HPP_*/
