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


#ifndef DVBSUBDECODER_PARSER_HPP_
#define DVBSUBDECODER_PARSER_HPP_

#include <cstdint>

#include "ParserCDS.hpp"
#include "ParserDDS.hpp"
#include "ParserEDS.hpp"
#include "ParserODS.hpp"
#include "ParserPCS.hpp"
#include "ParserRCS.hpp"
#include "Types.hpp"

namespace dvbsubdecoder
{

class DecoderClient;
class Region;
class Database;
class PesBuffer;
class PesPacketReader;
class PixelWriter;
class ObjectInstance;

/**
 * Subtitles parser.
 *
 * The Parser class is responsible for parsing the PES packets with
 * DVB subtitles data and storing the parsed data in the database.
 */
class Parser : private ParserCDS,
               private ParserDDS,
               private ParserEDS,
               private ParserODS,
               private ParserPCS,
               private ParserRCS
{
public:
    /**
     * Constructor.
     *
     * @param client
     *      Client interface.
     * @param database
     *      Database to use.
     * @param pesBuffer
     *      PES buffer to use.
     */
    Parser(DecoderClient& client,
           Database& database,
           PesBuffer& pesBuffer);

    /**
     * Destructor.
     */
    ~Parser() = default;

    /**
     * Processes the PES packets.
     *
     * Checks if the current STC value matches the Presentation Time Stamp
     * (PTS) of one or more PES packets in the internal PES packet queue,
     * and decodes them. Packets are removed from the buffer after parsing.
     *
     * @param stcTime
     *      Current STC time.
     *
     * @retval true
     *      Some data was processed. If there were some new subtitles decoded
     *      draw() should be called to render them.
     * @retval false
     *      No data was processed.
     */
    bool process(const StcTime& stcTime);

private:
    /**
     * Parses packet data.
     *
     * @param reader
     *      Reader to packet data.
     */
    void parsePacketData(PesPacketReader& reader);

    /**
     * Processes single segment.
     *
     * @param pesPacketReader
     *      Reader to packet data.
     */
    void processSubtitlingSegment(PesPacketReader& pesPacketReader);

    /**
     * Parses segment.
     *
     * @param pageId
     *      Page identifier.
     * @param segmentType
     *      Segment type.
     * @param dataReader
     *      Reader to segment data.
     */
    void parseSegment(std::uint16_t pageId,
                      std::uint8_t segmentType,
                      PesPacketReader& dataReader);

    /**
     * Returns low 32-bits timestamp.
     *
     * @param timeInfo
     *      Timestamp.
     *
     * @return
     *      Low 32-bits of timestamp.
     */
    std::uint32_t getTimeLow32(const StcTime& timeInfo) const;

    /**
     * Describes what to do with PES packet.
     */
    enum class PesAction
    {
        PROCESS, //!< PROCESS
        DROP,    //!< DROP
        WAIT     //!< WAIT
    };

    /**
     * Checks if it is a time to process the packet.
     *
     * Checks the PTS of a packet against STC if the packet shall be
     * currently processed.
     *
     * @param stc
     *      Current STC.
     * @param pts
     *      Packet PTS.
     *
     * @retval PesProcessTiming::PROCESS
     *      Packet shall be processed.
     * @retval PesProcessTiming::WAIT
     *      Packet processing shall be delayed.
     * @retval PesProcessTiming::DROP
     *      Packet is too old or too much in the future, should be dropped.
     */
    PesAction isTimeToProcess(const StcTime& stc,
                              const StcTime& pts) const;

    /**
     * Checks page timeout.
     *
     * Checks if page timeouted and if yes then resets its contents.
     *
     * @param stc
     *      Current STC.
     *
     * @return
     *      True if page timed out, false otherwise.
     */
    bool checkPageTimeout(const StcTime& stc);

    /** Client interface. */
    DecoderClient& m_client;

    /** Database to use. */
    Database& m_database;

    /** PES buffer to use. */
    PesBuffer& m_pesBuffer;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PARSER_HPP_*/
