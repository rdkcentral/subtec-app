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


#ifndef SUBTTXREND_PROTOCOL_PACKETSETCCATTRIBUTES_HPP_
#define SUBTTXREND_PROTOCOL_PACKETSETCCATTRIBUTES_HPP_

#include "Packet.hpp"

#include <array>
#include <unordered_map>

namespace subttxrend
{
namespace protocol
{

/**
 * Packet - SUBTITLE SELECTION.
 */
class PacketSetCCAttributes final : public PacketChannelSpecific
{
public:

     enum class CcAttribType {
        FONT_COLOR = 0x0001,
        BACKGROUND_COLOR = 0x0002,
        FONT_OPACITY = 0x0004,
        BACKGROUND_OPACITY = 0x0008,
        FONT_STYLE = 0x0010,
        FONT_SIZE = 0x0020,
        FONT_ITALIC = 0x0040,
        FONT_UNDERLINE = 0x0080,
        BORDER_TYPE = 0x0100,
        BORDER_COLOR = 0x0200,
        WIN_COLOR = 0x0400,
        WIN_OPACITY = 0x0800,
        EDGE_TYPE = 0x1000,
        EDGE_COLOR = 0x2000,
    };

    /**
     * Constructor.
     */
    PacketSetCCAttributes();

    bool containsAttribute(CcAttribType attr) const;
    uint32_t getAttributeValue(CcAttribType attr) const;

    std::unordered_map<CcAttribType, uint32_t> getAttributes() const
    {
        return m_attributesMap;
    }


protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

private:
    uint32_t m_ccType{};
    std::unordered_map<CcAttribType, uint32_t> m_attributesMap;
    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETSETCCATTRIBUTES_HPP_*/
