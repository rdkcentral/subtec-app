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


#include "PacketSubtitleSelection.hpp"

#include "BufferReader.hpp"

namespace subttxrend {
namespace protocol {

common::Logger PacketSubtitleSelection::m_logger("Protocol", "PacketSubtitleSelection");

PacketSubtitleSelection::PacketSubtitleSelection()
    : PacketChannelSpecific{Type::SUBTITLE_SELECTION}
    , m_subtitlesType(0)
    , m_auxId1(0)
    , m_auxId2(0)
{
    // noop
}
std::uint32_t PacketSubtitleSelection::getSubtitlesType() const
{
    return m_subtitlesType;
}

std::uint32_t PacketSubtitleSelection::getAuxId1() const
{
    return m_auxId1;
}

std::uint32_t PacketSubtitleSelection::getAuxId2() const
{
    return m_auxId2;
}

bool PacketSubtitleSelection::parseDataHeader(BufferReader& bufferReader)
{
    if (getSize() != 16) {
        m_logger.debug("%s - Invalid size", __LOGGER_FUNC__);
        return false;
    }

    PacketChannelSpecific::parseDataHeader(bufferReader);

    if (!bufferReader.extractLeUint32(m_subtitlesType)) {
        m_logger.debug("%s - Failed to extract subtitles type", __LOGGER_FUNC__);
        return false;
    }

    if (!bufferReader.extractLeUint32(m_auxId1)) {
        m_logger.debug("%s - Failed to extract aux ID 1", __LOGGER_FUNC__);
        return false;
    }

    if (!bufferReader.extractLeUint32(m_auxId2)) {
        m_logger.debug("%s - Failed to extract aux ID 2", __LOGGER_FUNC__);
        return false;
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend
