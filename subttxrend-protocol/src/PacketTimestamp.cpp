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


#include "PacketTimestamp.hpp"

#include "BufferReader.hpp"

namespace subttxrend {
namespace protocol {

common::Logger PacketTimestamp::m_logger("Protocol", "PacketTimestamp");

PacketTimestamp::PacketTimestamp()
    : PacketGeneric{Type::TIMESTAMP}
{
}

std::uint32_t PacketTimestamp::getStc() const
{
    return m_stc;
}

std::uint64_t PacketTimestamp::getTimestamp() const
{
    return m_timestamp;
}

bool PacketTimestamp::parseDataHeader(BufferReader& bufferReader)
{
    if (getSize() != 12) {
        m_logger.debug("%s - Invalid size", __LOGGER_FUNC__);
        return false;
    }

    if (!bufferReader.extractLeUint64(m_timestamp)) {
        m_logger.debug("%s - Failed to extract timestamp", __LOGGER_FUNC__);
        return false;
    }

    if (!bufferReader.extractLeUint32(m_stc)) {
        m_logger.debug("%s - Failed to extract STC", __LOGGER_FUNC__);
        return false;
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend
