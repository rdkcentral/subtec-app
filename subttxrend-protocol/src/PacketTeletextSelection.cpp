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


#include "PacketTeletextSelection.hpp"

#include "BufferReader.hpp"

namespace subttxrend {
namespace protocol {

namespace // anonymous
{
/** Default initial magazine and page value, used when not present in packet. */
const static std::uint32_t DEFAULT_MAGAZINE_NUMBER = 0x1;
const static std::uint32_t DEFAULT_PAGE_NUMBER = 0;
} // namespace

common::Logger PacketTeletextSelection::m_logger("Protocol", "PacketTeletextSelection");

PacketTeletextSelection::PacketTeletextSelection()
    : PacketChannelSpecific{Type::TELETEXT_SELECTION}
    , m_intitialMagazine(DEFAULT_MAGAZINE_NUMBER)
    , m_initialPage(DEFAULT_PAGE_NUMBER)
{
    // noop
}

std::uint32_t PacketTeletextSelection::getInitialMagazine() const
{
    return m_intitialMagazine;
}

std::uint32_t PacketTeletextSelection::getInitialPage() const
{
    return m_initialPage;
}

bool PacketTeletextSelection::parseDataHeader(BufferReader& bufferReader)
{
    if ((getSize() != 4) && (getSize() != 12)) {
        m_logger.debug("%s - Invalid size %u ??", __LOGGER_FUNC__, getSize());
        return false;
    }

    if (!PacketChannelSpecific::parseDataHeader(bufferReader)) {
        return false;
    }

    if (getSize() == 12) {
        if (!bufferReader.extractLeUint32(m_intitialMagazine)) {
            m_logger.debug("%s - Failed to extract aux ID 1", __LOGGER_FUNC__);
            return false;
        }

        if (!bufferReader.extractLeUint32(m_initialPage)) {
            m_logger.debug("%s - Failed to extract aux ID 2", __LOGGER_FUNC__);
            return false;
        }
    }

    return true;
}

} // namespace protocol
} // namespace subttxrend
