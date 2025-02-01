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


#include "PacketTtmlInfo.hpp"
#include "BufferReader.hpp"
#include <algorithm>

namespace subttxrend
{
namespace protocol
{

common::Logger PacketTtmlInfo::m_logger("Protocol", "PacketTtmlInfo");

PacketTtmlInfo::PacketTtmlInfo()
    : PacketChannelSpecific{Type::TTML_INFO}
{
    // noop
}

std::string PacketTtmlInfo::getContentType() const
{
    return m_contentType;
}

std::string PacketTtmlInfo::getSubtitleInfo() const
{
    return m_subtitleInfo;
}

bool PacketTtmlInfo::parseDataHeader(BufferReader& bufferReader)
{
    auto size = getSize();
    m_logger.trace("%s - Packet size: %d", __LOGGER_FUNC__, size);

    if (!PacketChannelSpecific::parseDataHeader(bufferReader)) {
        return false;
    }

    std::vector<char> buffer;
    if (!bufferReader.extractBuffer(size - sizeof(PacketChannelSpecific::getChannelId()), buffer)) {
        m_logger.debug("%s - Failed to extract related subtitle info", __LOGGER_FUNC__);
        return false;
    }

    auto it = std::find(buffer.begin(), buffer.end(), ' ');
    if (it != buffer.end()) {
        m_contentType = {buffer.begin(), it};
        m_subtitleInfo = {++it, buffer.end()};
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend

