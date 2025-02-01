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


#include "PacketData.hpp"

#include "BufferReader.hpp"

#include <cassert>

namespace subttxrend
{
namespace protocol
{

common::Logger PacketData::m_logger("Protocol", "PacketData");

PacketData::PacketData(Type type)
    : PacketChannelSpecific(type)
{
    // noop
}

std::uint32_t PacketData::getChannelType() const
{
    return m_channelType;
}

const char* PacketData::getData() const
{
    assert(m_dataBuffer);
    return m_dataBuffer->data() + m_dataOffset;
}

std::size_t PacketData::getDataSize() const
{
    assert(m_dataBuffer);
    return m_dataBuffer->size() - m_dataOffset;
}

std::int64_t PacketData::getDisplayOffset() const
{
    return m_displayOffsetMs;
}

bool PacketData::parseDataHeader(BufferReader& bufferReader)
{
    PacketChannelSpecific::parseDataHeader(bufferReader);

    auto const packetType = getType();
    if (packetType == Type::CC_DATA || packetType == Type::PES_DATA) {
        if (!bufferReader.extractLeUint32(m_channelType)) {
            m_logger.osdebug(__LOGGER_FUNC__, " - Failed to extract channel type");
            return false;
        }
        if (packetType == Type::CC_DATA) {
            std::uint32_t dummy;
            if (!bufferReader.extractLeUint32(dummy)) {
                m_logger.osdebug(__LOGGER_FUNC__, " - Failed to extract PTS presence type");
                return false;
            }

            if (!bufferReader.extractLeUint32(dummy)) {
                m_logger.osdebug(__LOGGER_FUNC__, " - Failed to extract PTS type");
                return false;
            }
        }
    } else if (packetType == Type::TTML_DATA || packetType == Type::WEBVTT_DATA) {
        if (!bufferReader.extractLeInt64(m_displayOffsetMs)) {
            m_logger.osdebug(__LOGGER_FUNC__, " - Failed to extract display offset");
            return false;
        }
    }

    return true;
}

bool PacketData::takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset)
{
    assert(dataBuffer && (dataOffset < dataBuffer->size()));
    m_dataBuffer = std::move(dataBuffer);
    m_dataOffset = dataOffset;
    return true;
}


} // namespace protocol
} // namespace subttxrend
