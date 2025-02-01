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


#ifndef DVBSUBDECODER_PESPACKETREADER_HPP_
#define DVBSUBDECODER_PESPACKETREADER_HPP_

#include <exception>
#include <cstdint>

#include "Types.hpp"

namespace dvbsubdecoder
{

/**
 * Pes packet header.
 */
struct PesPacketHeader
{
    /**
     * Checks if packet type is subtitles.
     *
     * @return
     *      True if packet type is subtitles, false otherwise.
     */
    bool isSubtitlesPacket() const
    {
        return m_streamId == 0xBD;
    }

    /**
     * Returns total packet size.
     *
     * @return
     *      Packet size in bytes. Zero if unknown.
     */
    std::uint32_t getTotalSize() const
    {
        if (m_pesPacketLength == 0)
        {
            return 0;
        }

        return static_cast<std::uint32_t>(m_pesPacketLength) + 6;
    }


    /** PES stream identifier. */
    std::uint8_t m_streamId;

    /** PES packet length. */
    std::uint16_t m_pesPacketLength;

    /** Flag indicating if PTS value is availalbe. */
    bool m_hasPts;

    /** PTS value (top 32 bits). */
    StcTime m_pts;
};

/**
 * Buffer for PES packets.
 */
class PesPacketReader
{
public:
    /**
     * Exception throw on errors.
     */
    class Exception : public std::exception
    {
    public:
        /**
         * Constructor
         *
         * @param message
         *      Exception message.
         */
        Exception(const char* message) :
                std::exception(),
                m_message(message)
        {
            // noop
        }

        /**
         * Returns exception message.
         *
         * Returns a C-style character string describing the general cause
         * of the current error.
         *
         * @return
         *      Message given to constructor.
         */
        virtual const char* what() const noexcept override
        {
            return m_message ? m_message : "";
        }

    private:
        /** Exception message. */
        const char* const m_message;
    };

    /**
     * Constructor.
     *
     * Constructs empty reader (reader that provides no data).
     */
    PesPacketReader();

    /**
     * Constructor.
     *
     * Constructs reader for given data.
     *
     * @param chunkData1
     *      Chunk 1 data.
     * @param chunkLen1
     *      Chunk 1 length
     * @param chunkData2
     *      Chunk 2 data.
     * @param chunkLen2
     *      Chunk 2 length
     */
    PesPacketReader(const std::uint8_t* chunkData1,
                    std::size_t chunkLen1,
                    const std::uint8_t* chunkData2,
                    std::size_t chunkLen2);

    /**
     * Constructor.
     *
     * Constructs sub-reader.
     *
     * @param reader
     *      Reader being the base.
     * @param count
     *      Number of bytes to be available via new reader.
     *
     * @throw PesPacketReader::Exception
     *      if operation cannot be done.
     */
    PesPacketReader(const PesPacketReader& reader,
                    std::size_t count);

    /**
     * Copy assignment operator.
     *
     * This class helps reading data from existing buffers. It does
     * not own them so shallow copy is sufficient.
     *
     * @param other
     *      Other instance.
     */
    PesPacketReader& operator=(const PesPacketReader& other) = default;

    /**
     * Destructor.
     */
    ~PesPacketReader();

    /**
     * Peeks data (reads but does not move pointer).
     *
     * @return
     *      Data read.
     *
     * @throw PesPacketReader::Exception
     *      if operation cannot be done.
     */
    std::uint8_t peekUint8() const;

    /**
     * Reads data.
     *
     * @return
     *      Data read.
     *
     * @throw PesPacketReader::Exception
     *      if operation cannot be done.
     */
    std::uint8_t readUint8();

    /**
     * Reads data.
     *
     * @return
     *      Data read.
     *
     * @throw PesPacketReader::Exception
     *      if operation cannot be done.
     */
    std::uint16_t readUint16be();

    /**
     * Skips given number of characters.
     *
     * @param count
     *      Number of characters to skip.
     *
     * @throw PesPacketReader::Exception
     *      if operation cannot be done.
     */
    void skip(std::size_t count);

    /**
     * Returns number of bytes left in the buffer.
     *
     * @return
     *      Number of bytes available.
     */
    std::size_t getBytesLeft() const
    {
        return m_chunkLen1 + m_chunkLen2;
    }

private:
    /** Fake bytes used by empty reader. */
    static std::uint8_t m_fakeByte;

    /** Chunk 1 data. */
    const std::uint8_t* m_chunkData1;

    /** Chunk 1 length. */
    std::size_t m_chunkLen1;

    /** Chunk 2 data. */
    const std::uint8_t* m_chunkData2;

    /** Chunk 1 length. */
    std::size_t m_chunkLen2;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PESPACKETREADER_HPP_*/
