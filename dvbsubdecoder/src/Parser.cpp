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


#include "Parser.hpp"

#include <cstdint>
#include <subttxrend/common/Logger.hpp>

#include "PesBuffer.hpp"
#include "DecoderClient.hpp"
#include "Database.hpp"
#include "ParserException.hpp"
#include "Config.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Parser");

} // namespace <anonmymous>

Parser::Parser(DecoderClient& client,
               Database& database,
               PesBuffer& pesBuffer) :
        m_client(client),
        m_database(database),
        m_pesBuffer(pesBuffer)
{
    // noop
}

bool Parser::process(const StcTime& stcTime)
{
    bool anyPacketsProcessed = false;

    g_logger.trace("%s", __func__);

    try
    {
        PesPacketHeader header;
        PesPacketReader dataReader;

        while (m_pesBuffer.getNextPacket(StcTimeType::LOW_32, header,
                dataReader))
        {
            if (header.isSubtitlesPacket())
            {
                if (header.m_hasPts)
                {
                    PesAction pesTiming = isTimeToProcess(stcTime, header.m_pts);
                    if (pesTiming == PesAction::WAIT)
                    {
                        g_logger.trace("%s - synchronizing, processing aborted",
                                __func__);
                        break;
                    }
                    else if (pesTiming == PesAction::PROCESS)
                    {
                        // notify that some packets were processed
                        anyPacketsProcessed = true;
                        m_database.getStatus().setLastPts(header.m_pts);
                        parsePacketData(dataReader);
                    }
                    else if (pesTiming == PesAction::DROP)
                    {
                        g_logger.info("%s - dropping overdue subtitle packet",
                                __func__);
                    }
                    else
                    {
                        assert(false);
                    }
                }
                else
                {
                    g_logger.info("%s - skipping subtitle packet without PTS",
                            __func__);
                }
            }
            else
            {
                g_logger.info("%s - skipping non-subtitle packet", __func__);
            }
            m_pesBuffer.markPacketConsumed(header);
        }
    }
    catch (const PesPacketReader::Exception& e)
    {
        g_logger.warning("%s - reset forced, reader error: %s", __func__,
                e.what());

        // handle issues silently
        m_pesBuffer.clear();

        // notify that some packets were processed
        anyPacketsProcessed = true;
    }

    checkPageTimeout(stcTime);

    g_logger.trace("%s - complete", __func__);

    return anyPacketsProcessed;
}

void Parser::parsePacketData(PesPacketReader& reader)
{
    g_logger.trace("%s - length=%zu", __func__, reader.getBytesLeft());

    // TODO: mark objects as needed etc. - e.g. page requires region,
    // TODO:   region requires object and clut etc. - do we need such level
    //         of details?

    auto dataId = reader.readUint8();
    if (dataId != PesBits::SUBTITLE_DATA_IDENTIFIER)
    {
        g_logger.info("%s - Invalid data identifier: %u", __func__, dataId);
        return;
    }

    auto subtitleStreamId = reader.readUint8();
    if (subtitleStreamId != PesBits::STREAM_ID_VALUE)
    {
        g_logger.info("%s - Invalid DVB subtitles stream identifier: %u",
                __func__, subtitleStreamId);
        return;
    }

    try
    {
        while (reader.peekUint8() == PesBits::SYNC_BYTE_VALUE)
        {
            processSubtitlingSegment(reader);
        }

        auto endOfPesDataFieldMarker = reader.readUint8();
        if (endOfPesDataFieldMarker != PesBits::END_MARKER_VALUE)
        {
            g_logger.info("%s - Invalid end marker: %02X", __func__,
                    endOfPesDataFieldMarker);

            throw PesPacketReader::Exception("Invalid data end marker");
        }
    }
    catch (const ParserException& e)
    {
        g_logger.info("%s - parsing error: %s - resetting epoch", __func__,
                e.what());
        m_database.epochReset();
    }
    catch (const PesPacketReader::Exception& e)
    {
        g_logger.info("%s - data stream error: %s - resetting epoch", __func__,
                e.what());
        m_database.epochReset();
    }
}

void Parser::processSubtitlingSegment(PesPacketReader& pesPacketReader)
{
    const std::uint8_t syncByte = pesPacketReader.readUint8();
    const std::uint8_t segmentType = pesPacketReader.readUint8();

    const std::uint16_t pageId = pesPacketReader.readUint16be();

    const std::uint16_t segmentLength = pesPacketReader.readUint16be();

    g_logger.trace("%s - sync=%02X type=%02X page=%04X length=%u", __func__,
            syncByte, segmentType, pageId, segmentLength);

    if (syncByte != PesBits::SYNC_BYTE_VALUE)
    {
        throw PesPacketReader::Exception("Invalid sync byte");
    }

    if (m_database.getStatus().isSelectedPage(pageId))
    {
        PesPacketReader segmentDataReader(pesPacketReader, segmentLength);

        parseSegment(pageId, segmentType, segmentDataReader);
    }
    else
    {
        g_logger.trace("%s - Segment for page that is not selected: %u",
                __func__, pageId);
    }

    pesPacketReader.skip(segmentLength);
}

void Parser::parseSegment(std::uint16_t pageId,
                          std::uint8_t segmentType,
                          PesPacketReader& dataReader)
{
    g_logger.trace("%s - page=%04X type=%02X", __func__, pageId, segmentType);

    switch (segmentType)
    {
    case SegmentTypeBits::PAGE_COMPOSITION:
        if (m_database.getStatus().isCompositionPage(pageId))
        {
            parsePageCompositionSegment(m_database, dataReader);
        }
        else
        {
            g_logger.trace(
                    "%s - ignored page composition segment for page=%04X",
                    __func__, pageId);
        }
        break;

    case SegmentTypeBits::REGION_COMPOSITION:
        if (m_database.getStatus().isCompositionPage(pageId))
        {
            parseRegionCompositionSegment(m_database, dataReader);
        }
        else
        {
            g_logger.trace(
                    "%s - ignored region composition segment for page=%04X",
                    __func__, pageId);
        }
        break;

    case SegmentTypeBits::CLUT_DEFINITION:
        parseClutDefinitionSegment(m_database, dataReader);
        break;

    case SegmentTypeBits::OBJECT_DATA:
        parseObjectDataSegment(m_database, dataReader);
        break;

    case SegmentTypeBits::DISPLAY_DEFINITION:
        if (m_database.getStatus().getSpecVersion()
                == Specification::VERSION_1_3_1)
        {
            if (m_database.getStatus().isCompositionPage(pageId))
            {
                parseDisplayDefinitionSegment(m_database, dataReader);
            }
            else
            {
                g_logger.trace(
                        "%s - ignored display definition segment for page=%04X",
                        __func__, pageId);
            }
        }
        else
        {
            g_logger.trace(
                    "%s - ignored unexpected display definition (using spec 1.2.1)",
                    __func__);
        }
        break;

    case SegmentTypeBits::DISPARITY_SIGNALLING:
        g_logger.trace(
                "%s - ignored disparity definition segment for page=%04X",
                __func__, pageId);
        break;

    case SegmentTypeBits::END_OF_DISPLAY_SET:
        // checking for ancillary only as in case where it is same
        // as composition page it would work good and in case where
        // the pages are different the ancillary page segment shall
        // define end of data
        if (m_database.getStatus().isAncillaryPage(pageId))
        {
            parseEndOfDisplaySetSegment(m_database, dataReader);
        }
        else
        {
            g_logger.trace(
                    "%s - ignored end of display segment for page=%04X",
                    __func__, pageId);
        }
        break;

    default:
        g_logger.info("%s - unknown segment type %02X for page %04X", __func__,
                segmentType, pageId);
        break;
    }
}

std::uint32_t Parser::getTimeLow32(const StcTime& timeInfo) const
{
    std::uint32_t value = timeInfo.m_time;
    if (timeInfo.m_type == StcTimeType::HIGH_32)
    {
        // top bit lost, last bit set to zero
        value <<= 1;
    }
    return value;
}

Parser::PesAction Parser::isTimeToProcess(const StcTime& stc,
                                                 const StcTime& pts) const
{
    const std::uint32_t stcValue = getTimeLow32(stc);
    const std::uint32_t ptsValue = getTimeLow32(pts);
    const std::uint32_t ptsStcDiff = ptsValue - stcValue;
    const std::uint32_t stcPtsDiff = stcValue - ptsValue;

    PesAction result = PesAction::DROP;

    if ((PTS_STC_DIFF_MIN_90KHZ <= ptsStcDiff)
            && (ptsStcDiff <= PTS_STC_DIFF_MAX_90KHZ))
    {
        result = PesAction::WAIT;
    }
    else if (ptsStcDiff <= PTS_STC_DIFF_MIN_90KHZ
            || stcPtsDiff <= PES_DELAY_MAX_90KHZ)
    {
        result = PesAction::PROCESS;
    }

    g_logger.trace("%s - stc=%u pts=%u diff=%u process=%d", __func__,
            stc.m_time, pts.m_time, ptsStcDiff, static_cast<int>(result));

    return result;
}

bool Parser::checkPageTimeout(const StcTime& stc)
{
    g_logger.trace("%s", __func__);

    auto& page = m_database.getPage();
    if (page.getState() != Page::State::COMPLETE)
    {
        g_logger.trace("%s - page not complete, skipping", __func__);
        return false;
    }

    const std::uint32_t stcValue = getTimeLow32(stc);
    const std::uint32_t ptsValue = getTimeLow32(page.getPts());

    // check for case when page is displayed before STC due to decoding
    // start (TIMESTAMP_DIFF_MIN_45KHZ)
    const std::uint32_t diffBefore = ptsValue - stcValue;
    if (diffBefore <= PTS_STC_DIFF_MIN_90KHZ)
    {
        g_logger.trace("%s - STC before PTS, skipping", __func__);
        return false;
    }

    // check timeout using page timeout field
    const std::uint32_t diffAfter = stcValue - ptsValue;
    // timeout + extra second converted to 90khz units
    const std::uint32_t timeout = (page.getTimeout() + 1) * 1000 * 90;
    if (diffAfter < timeout)
    {
        g_logger.trace(
                "%s - timeout not elapsed. stc=%u pts=%u diff=%u timeout=%u",
                __func__, stcValue, ptsValue, diffAfter, timeout);

        return false;
    }

    g_logger.info("%s - timeout elapsed. stc=%u pts=%u diff=%u timeout=%u",
            __func__, stcValue, ptsValue, diffAfter, timeout);

    // page timed out
    page.setTimedOut();

    return true;
}

} // namespace dvbsubdecoder
