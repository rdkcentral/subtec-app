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


#ifndef SUBTTXREND_PROTOCOL_PACKETSUBTITLESELECTION_HPP_
#define SUBTTXREND_PROTOCOL_PACKETSUBTITLESELECTION_HPP_

#include "Packet.hpp"

namespace subttxrend
{
namespace protocol
{

/**
 * Packet - SUBTITLE SELECTION.
 */
class PacketSubtitleSelection final : public PacketChannelSpecific
{
public:
    /** Subtitles type - DVB. */
    static const std::uint32_t SUBTITLES_TYPE_DVB = 0;
    /** Subtitles type - Teletext. */
    static const std::uint32_t SUBTITLES_TYPE_TELETEXT = 1;
    /** Subtitles type - SCTE-27. */
    static const std::uint32_t SUBTITLES_TYPE_SCTE = 2;
    /** Subtitles type - Closed Captions. */
    static const std::uint32_t SUBTITLES_TYPE_CC = 3;

    /**
     * Constructor.
     */
    PacketSubtitleSelection();

    /**
     * Returns subtitles type.
     *
     * @return
     *      Subtitles type (0 - DVB, 1 - teletext).
     */
    std::uint32_t getSubtitlesType() const;

    /**
     * Returns aux ID 1.
     *
     * @return
     *      Aux ID 1 (composition page id, magazine nbr).
     */
    std::uint32_t getAuxId1() const;

    /**
     * Returns aux ID 2.
     *
     * @return
     *      Aux ID2 (ancillary page id, page number).
     */
    std::uint32_t getAuxId2() const;

protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

private:
    /** Subtitles type. */
    std::uint32_t m_subtitlesType;

    /** Aux ID 1. */
    std::uint32_t m_auxId1;

    /** Aux ID 2. */
    std::uint32_t m_auxId2;

    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETSUBTITLESELECTION_HPP_*/
