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


#ifndef DVBSUBDECODER_BITSTREAMWRITER_HPP_
#define DVBSUBDECODER_BITSTREAMWRITER_HPP_

#include <cstdint>
#include <vector>
#include <cassert>

/**
 * Utility for building bit streams.
 */
class BitStreamWriter
{
public:
    /**
     * Constructor.
     *
     * Creates empty bit stream.
     */
    BitStreamWriter()
    {
        clear();
    }

    /**
     * Clears the bit stream contents.
     */
    void clear()
    {
        m_bitsLeft = 0;
        m_streamBytes.clear();
    }


    /**
     * Returns pixel bit stream.
     *
     * @return
     *      Byte array with bit stream.
     */
    const std::uint8_t* data() const
    {
        return m_streamBytes.data();
    }

    /**
     * Returns bit stream bytes size.
     *
     * @return
     *      Size of bit stream in bytes.
     */
    const std::size_t size() const
    {
        return m_streamBytes.size();
    }

    /**
     * Writes value to stream.
     *
     * @param value
     *      Value to write (stored on less significant bits).
     * @param size
     *      Size of the value to write.
     */
    void write(std::uint64_t value,
               std::uint8_t size)
    {
        assert((size >= 1) && (size <= 64));

        while (size > 8)
        {
            size -= 8;
            writeInternal((value >> size) & 0xFF, 8);
        }

        writeInternal(value & 0xFF, size);
    }

    /**
     * Returns number of stuffing bits needed to align to byte boundary.
     *
     * @return
     *      Number of stuffing bits needed.
     */
    const std::uint8_t getStuffSizeNeeded() const
    {
        return m_bitsLeft;
    }

private:
    /**
     * Writes value to stream.
     *
     * @param value
     *      Value to write (stored on less significant bits).
     * @param size
     *      Size of the value to write.
     */
    void writeInternal(std::uint8_t value,
                       std::uint8_t size)
    {
        assert((size >= 1) && (size <= 8));

        if (m_bitsLeft == 0)
        {
            m_streamBytes.push_back(0);
            m_bitsLeft = 8;
        }

        if (size > m_bitsLeft)
        {
            auto part2size = size - m_bitsLeft;

            write(value >> part2size, m_bitsLeft);
            write(value, part2size);
        }
        else
        {
            auto shiftSize = m_bitsLeft - size;
            std::uint8_t mask = ((static_cast<std::uint32_t>(1) << size) - 1)
                    & 0xFF;

            *m_streamBytes.rbegin() |= (value & mask) << shiftSize;
            m_bitsLeft -= size;
        }
    }

    /** Stream bytes. */
    std::vector<std::uint8_t> m_streamBytes;

    /** Number of bits left in current byte. */
    std::uint8_t m_bitsLeft;
};

#endif /*DVBSUBDECODER_BITSTREAMWRITER_HPP_*/
