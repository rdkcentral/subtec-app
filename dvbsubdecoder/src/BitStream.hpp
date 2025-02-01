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


#ifndef DVBSUBDECODER_BITSTREAM_HPP_
#define DVBSUBDECODER_BITSTREAM_HPP_

#include "PesPacketReader.hpp"

namespace dvbsubdecoder
{

/**
 * Reader for stream of bits.
 */
class BitStream
{
public:
    /**
     * Constructor.
     *
     * @param reader
     *      Reader from which bytes of data are read.
     */
    BitStream(PesPacketReader& reader) :
            m_reader(reader),
            m_bitsLeft(0),
            m_bitsValues(0)
    {
        // noop
    }

    /**
     * Reads number of bits from the stream.
     *
     * The bits are read from the stream in most significant bit first
     * order.
     *
     * @return
     *      The value read (stored on lowest bits).
     *
     * @tparam LEN
     *      Number of bits to read.
     */
    template<std::uint8_t LEN>
    std::uint8_t read()
    {
        static_assert((LEN > 0) && (LEN <= 8), "Unsupported length");

        static const std::uint32_t MASK = (1 << LEN) - 1;

        if (m_bitsLeft < LEN)
        {
            m_bitsValues <<= 8;
            m_bitsValues |= m_reader.readUint8();
            m_bitsLeft += 8;
        }

        m_bitsLeft -= LEN;
        return static_cast<std::uint8_t>((m_bitsValues >> m_bitsLeft) & MASK);
    }

private:
    /** Reader from which bytes of data are read. */
    PesPacketReader& m_reader;
    /** Number of bits left in the internal buffer. */
    std::uint8_t m_bitsLeft;
    /** Internal buffer with bits. */
    std::uint32_t m_bitsValues;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_BITSTREAM_HPP_*/
