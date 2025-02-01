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

#include "PacketWebvttTimestamp.hpp"

#include "BufferReader.hpp"

namespace subttxrend
{
namespace protocol
{

common::Logger PacketWebvttTimestamp::m_logger("Protocol", "PacketWebvttTimestamp");

PacketWebvttTimestamp::PacketWebvttTimestamp() :
        PacketChannelSpecific{Type::WEBVTT_TIMESTAMP},
        m_timestamp(0)
{
    // noop
}

std::uint64_t PacketWebvttTimestamp::getTimestamp() const
{
    return m_timestamp;
}

bool PacketWebvttTimestamp::parseDataHeader(BufferReader& bufferReader)
{
    if (getSize() != 12)
    {
        m_logger.debug("%s - Invalid size", __LOGGER_FUNC__);
        return false;
    }

    if (!PacketChannelSpecific::parseDataHeader(bufferReader)) {
        return false;
    }

    if (!bufferReader.extractLeUint64(m_timestamp))
    {
        m_logger.debug("%s - Failed to extract timestamp", __LOGGER_FUNC__);
        return false;
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend
