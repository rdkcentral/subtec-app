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


#include "Parser.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserRowContext.hpp"
#include "PacketTriplets.hpp"
#include "PageDisplayable.hpp"
#include "DecodedPage.hpp"

namespace ttxdecoder
{

namespace // <anonymous>
{

subttxrend::common::Logger g_logger("TtxDecoder", "ParserX26");

struct Position
{
    std::uint8_t row;
    std::uint8_t column;

    void reset()
    {
        row = 0;
        column = 0;
    }
};

// row address triplets
static constexpr std::uint8_t SET_ACTIVE_POSITION = 0x4;
static constexpr std::uint8_t ADDRESS_DISPLAY_ROW0 = 0x7;

// column address triplets
static constexpr std::uint8_t LINE_DRAWING_OR_SMOOTH_MOSAIC = 0x2;
static constexpr std::uint8_t CHARACTER_FROM_G2 = 0xF;

static constexpr std::uint32_t TRIPLET_TERMINATION_MARKER = 0x7FF;

/**
 * Checks if given triplet is termination triplet.
 *
 * @param value
 *      Triplet value.
 * @return
 *      True for termination triplet, false otherwise.
 */
bool isTerminationTriplet(std::uint32_t value)
{
    if ((value & TRIPLET_TERMINATION_MARKER) == TRIPLET_TERMINATION_MARKER)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Converts X26 packet address to row number.
 *
 * From the spec (ETSI EN 300 706 V1.2.1 (2003-04, 12.3.2 Active Position):
 * "Address value 40 (decimal) implies row 24 and values 41 to 63 indicate rows 1 to 23 inclusive"
 *
 * @param address
 *      Address field.
 * @return
 *      Row index.
 */
std::uint8_t addressToRow(std::uint8_t address)
{
    static constexpr std::uint8_t MIN_ROW_ADDRESS = 40;
    static constexpr std::uint8_t MAX_ROW_ADDRESS = 63;

    if ((MIN_ROW_ADDRESS <= address) && (address <= MAX_ROW_ADDRESS))
    {
        return ((address == 40) ? 24 : (address - 40));
    }
    else
    {
        return 0;
    }
}

/**
 *  Converts address field from X26 packet to column index.
 *
 * @param address
 *      Address field content.
 * @return
 *      Column index.
 */
std::uint8_t addressToColumn(std::uint8_t address)
{
    return ((address < 40) ? address : 0);
}

/**
 *  Converts data field from X26 packet to column index.
 *
 * @param data
 *      Data field content.
 * @return
 *      Column index.
 */
std::uint8_t dataFieldToColumn(std::uint8_t data)
{
    return ((data < 40) ? data : 0);
}

/**
 * Extracts diacritic property from mode.
 *
 * @param mode
 *      Mode value.
 * @return
 *      One of Property::VALUE_DIACRITIC_* constants.
 */
std::uint16_t convertModeToDiacriticProperty(std::uint8_t mode)
{
    return ((mode & 0xF) << 9);
}

/**
 * Replaces character at active position with given one.
 *
 * @param target
 *      Decoded page.
 * @param position
 *      Active position.
 * @param targetCharacter
 *      Character to replace with.
 */
void replaceCharacter(DecodedPage& target,
                      const Position& position,
                      std::uint8_t targetCharacter)
{
    auto& decodedRow = target.getRow(position.row);
    auto& segment = decodedRow.m_levelOnePageSegment;

    g_logger.trace("%s - pos %d:%d replacing character 0x%x with 0x%x",
            __func__, position.row, position.column,
            segment.m_charArray[position.column], targetCharacter);

    segment.m_charArray[position.column] = targetCharacter;
}

/**
 * Checks if mode is in "character with diacritic" mode range (0x10 - 0x1F).
 *
 * @param mode
 *      Triplet mode.
 * @return
 *      True mode is "Characters Including Diacritical Marks", false otherwise.
 */
bool isCharWithDiacriticMode(std::uint8_t mode)
{
    return (mode >= 0x10 && mode <= 0x1F);
}

/**
 * Translates triplet data field to character map index.
 *
 * @param data
 *      Triplet data field.
 * @return
 *      Charset map index.
 */
std::uint16_t dataToCharacterIndex(std::uint16_t data)
{
    return ((data >= 0x20) ? (data - 0x20) : 0x0);
}

/**
 * Processes single X26 packet.
 *
 * @param source
 *      Page with the data.
 * @param target
 *      Page target.
 * @param charsetMaps
 *      Active charset maps set.
 * @param position
 *      Active position.
 * @return
 *      True if termination marker was found, false otherwise.
 */
bool processX26Packet(const PacketTriplets& source,
                      DecodedPage& target,
                      const CharsetMaps& charsetMaps,
                      Charset currentG2Charset,
                      Position& position)
{
    bool terminationMarkerFound = false;

    for (std::size_t i = 0; i < PacketTriplets::TRIPLET_COUNT; i++)
    {
        auto value = source.getTripletValue(i);

        const std::uint8_t address = value & 0x3F;
        const std::uint8_t mode = (value >> 6) & 0x1F;
        const std::uint8_t data = (value >> 11) & 0x7F;

        g_logger.trace("%s - triplet[%d]=0x%x address=0x%x mode=0x%x data=0x%x", __func__, static_cast<int>(i), value,
                address, mode, data);

        if (isTerminationTriplet(value))
        {
            terminationMarkerFound = true;
            break;
        }

        switch (mode)
        {
            case SET_ACTIVE_POSITION:
            {
                position.row = addressToRow(address);
                position.column = dataFieldToColumn(data);

                g_logger.trace("%s set active position: %d:%d", __func__, position.row,
                        position.column);
                break;
            }

            case ADDRESS_DISPLAY_ROW0:
            {
                if (address == 0x1F)
                {
                    position.row = 0;
                    position.column = 0;
                }
                g_logger.trace("%s active pos: %d:%d", __func__, position.row, position.column);
                break;
            }

            case LINE_DRAWING_OR_SMOOTH_MOSAIC:
            {
                position.column = addressToColumn(address);

                if (data >= 0x20)
                {
                    const auto& smoothMosaicCharset = charsetMaps[Charset::G3_SMOOTH_MOSAIC];
                    auto targetCharacter = smoothMosaicCharset[dataToCharacterIndex(data)];

                    replaceCharacter(target, position, targetCharacter);
                }

                break;
            }

            case CHARACTER_FROM_G2:
            {
                position.column = addressToColumn(address);

                if (data >= 0x20)
                {
                    const auto& g2Charset = charsetMaps[currentG2Charset];
                    auto targetCharacter = g2Charset[dataToCharacterIndex(data)];

                    replaceCharacter(target, position, targetCharacter);
                }
                break;
            }

            default:
            {
                if (isCharWithDiacriticMode(mode))
                {
                    position.column = addressToColumn(address);

                    if (data >= 0x20)
                    {
                        const auto& g0Charset = charsetMaps[Charset::G0_LATIN];

                        /* "No diacritical mark exists for mode description value 10000. An unmodified G0 character is
                         then displayed unless the 7 bits of the data field have the value 0101010 (2/A) when the
                         symbol "@" shall be displayed."*/
                        auto targetCharacter =
                                ((mode == 0x10) && (data == 0x2A)) ?
                                        g0Charset[0x40] : g0Charset[dataToCharacterIndex(data)];

                        auto& decodedRow = target.getRow(position.row);
                        auto& segment = decodedRow.m_levelOnePageSegment;

                        auto diacritic = convertModeToDiacriticProperty(mode);

                        g_logger.trace("%s - pos %d:%d replacing character 0x%x with 0x%x and diacritic 0x%x", __func__,
                                position.row, position.column,
                                segment.m_charArray[position.column], targetCharacter, diacritic);

                        segment.m_charArray[position.column] = targetCharacter;
                        segment.m_propertiesArray[position.column] |= diacritic;
                    }
                }
                else
                {
                    g_logger.trace("%s skipping unsupported mode=0x%x", __func__, mode);
                }
                break;
            }
        }
    }
    return terminationMarkerFound;
}

} // namespace <anonymous>

void Parser::processX26(const PageDisplayable& source,
                        DecodedPage& target)
{
    g_logger.trace("%s", __func__);

    Position position = { 0, 0 };

    for (std::size_t i = 0; i < PageDisplayable::X_26_PACKET_COUNT; i++)
    {
        auto packet = source.getPacketX26(i);
        g_logger.trace("%s i: %zd packet: %p", __func__, i, packet);

        if (!packet)
        {
            break;
        }

        bool terminationMarkerFound = processX26Packet(*packet, target, m_charsetConfig.getCharsetMaps(),
                m_currentG2Charset, position);

        if (terminationMarkerFound)
        {
            break;
        }
    }
}

}
