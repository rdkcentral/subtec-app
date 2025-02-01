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


#ifndef DVBSUBDECODER_PESBUFFER_HPP_
#define DVBSUBDECODER_PESBUFFER_HPP_

#include <subttxrend/common/NonCopyable.hpp>

#include "PesPacketReader.hpp"
#include "Array.hpp"
#include "Allocator.hpp"
#include "Types.hpp"

namespace dvbsubdecoder
{

class Allocator;

/**
 * Buffer for PES packets.
 */
class PesBuffer : private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     */
    PesBuffer(Allocator& allocator);

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
     * @param timeType
     *      Time type to extract to match STC.
     * @param header
     *      Variable to be filled with header information.
     * @param dataReader
     *      Variable where reader to data will be stored.
     *
     * @return
     *      True if next packet is available, false otherwise.
     */
    bool getNextPacket(StcTimeType timeType,
                       PesPacketHeader& header,
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
     * @param timeType
     *      Time type to extract to match STC.
     * @param reader
     *      Data reader to use.
     * @param header
     *      Header to be filled with data.
     */
    void readHeader(StcTimeType timeType,
                    PesPacketReader& reader,
                    PesPacketHeader& header);

    /** Buffer. */
    Allocator::UniquePtr<Array<uint8_t>> m_bufferArray;

    /** Pointer to buffer data. */
    std::uint8_t* const m_buffer;

    /** Size of the buffer. */
    std::size_t const m_size;

    /** Number of currently used bytes. */
    std::size_t m_used;

    /** Current read offset. */
    std::size_t m_readOffset;

    /** Current write offset. */
    std::size_t m_writeOffset;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PESBUFFER_HPP_*/
