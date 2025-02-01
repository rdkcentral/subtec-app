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


#include <algorithm>
#include <cassert>

#include <subttxrend/common/Logger.hpp>

#include "CharsetMapping.hpp"
#include "Property.hpp"
#include "Types.hpp"

namespace ttxdecoder
{

namespace // anonymous
{

subttxrend::common::Logger g_logger("TtxDecoder", "CharsetMapping");

constexpr std::uint16_t CHARSET_MAP_CHAR_OFFSET = 0x20;

} // namespace anonymous

CharsetMapping::CharsetMapping(const CharsetConfig& charsetConfig,
                               const NationalCharset primaryG0,
                               const NationalCharset secondaryG0) :
        m_charsetConfig(charsetConfig),
        m_charsetMaps(m_charsetConfig.getCharsetMaps()),
        m_primaryG0(primaryG0),
        m_secondaryG0(secondaryG0),
        m_currentG0(m_primaryG0),
        m_G1Charset(Charset::G1_BLOCK_MOSAIC)
{
    // noop
}

void CharsetMapping::switchG0Charset()
{
    if (m_currentG0 == m_primaryG0)
    {
        m_currentG0 = m_secondaryG0;
    }
    else
    {
        m_currentG0 = m_primaryG0;
    }
}

void CharsetMapping::setCurrentG1(Charset g1Charset)
{
    assert((g1Charset == Charset::G1_BLOCK_MOSAIC) || (g1Charset == Charset::G1_BLOCK_MOSAIC_SEPARATED));
    m_G1Charset = g1Charset;
}

CharPropertyPair CharsetMapping::mapG0Character(const std::uint16_t character) const
{
    if (isReplacable(character, m_currentG0))
    {
        const NationalCharactersArray& nationalArray = m_charsetConfig.getNationalSubsets()[m_currentG0];

        auto index = std::distance(REPLACEBLE_CHARS.begin(),
                std::find(REPLACEBLE_CHARS.begin(), REPLACEBLE_CHARS.end(), character));

        g_logger.trace("%s index: %u for character %04X (charset %u)", __func__,
                static_cast<int>(index), character, toIndex(m_currentG0));

        const NationalCharacter& nationalCharacter = nationalArray[index];

        if (m_charsetConfig.getMode() == CharsetConfig::Mode::DIACTRIC)
        {
            auto charset = nationalCharacter.m_charset;
            auto characterIndex = characterToIndex(nationalCharacter.m_charsetMapIndex);

            auto mappedCharacter = m_charsetMaps[charset][characterIndex];
            auto property = nationalCharacter.m_diacriticProperty;

            g_logger.trace("%s diacritic mapping %04X -> %04X (props: %04X)", __func__, character, mappedCharacter,
                    property);
            return std::make_pair(mappedCharacter, property);
        }
        else
        {
            g_logger.trace("%s utf16 mapping %04X -> %04X", __func__, character, nationalCharacter.m_utf16);
            return std::make_pair(nationalCharacter.m_utf16, Property::VALUE_DIACRITIC_NONE);
        }
    }
    else
    {
        g_logger.trace("%s direct char %04X (charset %u)", __func__, character, toIndex(m_currentG0));
        return mapCharacter(character, m_charsetMaps[m_currentG0]);
    }
}

CharPropertyPair CharsetMapping::mapG1Character(const std::uint16_t character) const
{
    return mapCharacter(character, m_charsetMaps[m_G1Charset]);
}

std::uint16_t CharsetMapping::characterToIndex(const std::uint16_t character) const
{
    if (character >= CHARSET_MAP_CHAR_OFFSET)
    {
        std::uint16_t index = character - CHARSET_MAP_CHAR_OFFSET;
        if (index < CHARSET_MAPPING_SIZE)
        {
            return index;
        }
    }

    g_logger.warning("%s - Character outside of charset range: %04X", __func__, character);

    // forcing space in case of invalid character
    return 0;
}

CharPropertyPair CharsetMapping::mapCharacter(const uint16_t character,
                                              const CharsetMappingArray& charsetMap) const
{
    const uint16_t characterIndex = characterToIndex(character);

    const auto mappedCharacter = charsetMap[characterIndex];

    if (character != mappedCharacter)
    {
        g_logger.trace("%s character: %04X -> %04X", __func__, character, mappedCharacter);
    }

    return std::make_pair(mappedCharacter, Property::VALUE_DIACRITIC_NONE);
}

bool CharsetMapping::isReplacable(std::uint16_t character,
                                  NationalCharset charset) const
{
    auto begin = std::begin(REPLACEBLE_CHARSETS);
    auto end = std::end(REPLACEBLE_CHARSETS);

    auto findResult = std::find(begin, end, charset);

    if (findResult != end)
    {
        auto charsBegin = std::begin(REPLACEBLE_CHARS);
        auto charsEnd = std::end(REPLACEBLE_CHARS);

        auto findCharResult = std::find(charsBegin, charsEnd, character);

        if (findCharResult != charsEnd)
        {
            return true;
        }
    }
    return false;
}

} // namespace ttxdecoder

