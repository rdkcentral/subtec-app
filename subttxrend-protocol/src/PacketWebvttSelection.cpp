/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#include "PacketWebvttSelection.hpp"

#include "BufferReader.hpp"

namespace subttxrend
{
namespace protocol
{

common::Logger PacketWebvttSelection::m_logger("Protocol", "PacketWebvttSelection");

PacketWebvttSelection::PacketWebvttSelection()
    : PacketChannelSpecific{Type::WEBVTT_SELECTION}
    , m_relatedVideoWidht(0)
    , m_relatedVideoHeight(0)
{
    // noop
}

std::uint32_t PacketWebvttSelection::getRelatedVideoWidth() const
{
    return m_relatedVideoWidht;
}

std::uint32_t PacketWebvttSelection::getRelatedVideoHeight() const
{
    return m_relatedVideoHeight;
}

bool PacketWebvttSelection::parseDataHeader(BufferReader& bufferReader)
{
    if (getSize() != PACKET_SIZE)
    {
        m_logger.debug("%s - Invalid size: %d", __LOGGER_FUNC__, getSize());
        return false;
    }

    if (!PacketChannelSpecific::parseDataHeader(bufferReader)) {
        return false;
    }

    if (!bufferReader.extractLeUint32(m_relatedVideoWidht))
    {
        m_logger.debug("%s - Failed to extract related video width", __LOGGER_FUNC__);
        return false;
    }

    if (!bufferReader.extractLeUint32(m_relatedVideoHeight))
    {
        m_logger.debug("%s - Failed to extract related video height", __LOGGER_FUNC__);
        return false;
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend

