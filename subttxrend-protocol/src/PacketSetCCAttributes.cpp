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


#include "PacketSetCCAttributes.hpp"

#include "BufferReader.hpp"

namespace subttxrend {
namespace protocol {

namespace {

static const std::array<PacketSetCCAttributes::CcAttribType, 14> attributMasks{
    PacketSetCCAttributes::CcAttribType::FONT_COLOR,
    PacketSetCCAttributes::CcAttribType::BACKGROUND_COLOR,
    PacketSetCCAttributes::CcAttribType::FONT_OPACITY,
    PacketSetCCAttributes::CcAttribType::BACKGROUND_OPACITY,
    PacketSetCCAttributes::CcAttribType::FONT_STYLE,
    PacketSetCCAttributes::CcAttribType::FONT_SIZE,
    PacketSetCCAttributes::CcAttribType::FONT_ITALIC,
    PacketSetCCAttributes::CcAttribType::FONT_UNDERLINE,
    PacketSetCCAttributes::CcAttribType::BORDER_TYPE,
    PacketSetCCAttributes::CcAttribType::BORDER_COLOR,
    PacketSetCCAttributes::CcAttribType::WIN_COLOR,
    PacketSetCCAttributes::CcAttribType::WIN_OPACITY,
    PacketSetCCAttributes::CcAttribType::EDGE_TYPE,
    PacketSetCCAttributes::CcAttribType::EDGE_COLOR
};

}

common::Logger PacketSetCCAttributes::m_logger("Protocol", "PacketSetCCAttributes");

PacketSetCCAttributes::PacketSetCCAttributes()
    : PacketChannelSpecific{Type::SET_CC_ATTRIBUTES}
{
    // noop
}

bool PacketSetCCAttributes::parseDataHeader(BufferReader& bufferReader)
{
    if (getSize() != 68) {
        m_logger.error("%s - Invalid size, getSize() = %d", __LOGGER_FUNC__, (int)getSize());
        return false;
    }

    PacketChannelSpecific::parseDataHeader(bufferReader);

    if (!bufferReader.extractLeUint32(m_ccType)) {
        m_logger.error("%s - Failed to extract ccType", __LOGGER_FUNC__);
        return false;
    }

    uint32_t attribType{};

    if (!bufferReader.extractLeUint32(attribType)) {
        m_logger.error("%s - Failed to extract attribType", __LOGGER_FUNC__);
        return false;
    }

    for(const auto mask : attributMasks)
    {
        uint32_t attribValue{};

        if (!bufferReader.extractLeUint32(attribValue)) {
            m_logger.error("%s - Failed to extract attribValue", __LOGGER_FUNC__);
            return false;
        }
        if(attribType & (uint32_t)mask)
        {
            m_attributesMap[mask] = attribValue;
        }
    }

    return true;
}

bool PacketSetCCAttributes::containsAttribute(CcAttribType attr) const
{
    return m_attributesMap.find(attr) != m_attributesMap.end();
}

uint32_t PacketSetCCAttributes::getAttributeValue(CcAttribType attr) const
{
    return m_attributesMap.at(attr);
}

} // namespace protocol
} // namespace subttxrend
