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


#ifndef TTXDECODER_CHARSETMAPPING_HPP
#define TTXDECODER_CHARSETMAPPING_HPP

#include <cstdint>
#include <utility>      // std::pair

#include <subttxrend/common/NonCopyable.hpp>

#include "CharsetConfig.hpp"
#include "CharsetMaps.hpp"
#include "Types.hpp"

namespace ttxdecoder
{

using CharPropertyPair = std::pair<std::uint16_t, std::uint16_t>;

/**
 * Utility for mapping characters basing on selected charset. Allows
 * to switch between primary and alternative charset.
 */
class CharsetMapping
{
public:

    /**
     * Constructor.
     *
     * @param charsetConfig
     *      Charsets configuration.
     * @param primaryG0
     *      Primary G0 charset.
     * @param secondaryG0
     *      Secondary G0 charset.
     */
    CharsetMapping(const CharsetConfig& charsetConfig,
                   const NationalCharset primaryG0,
                   const NationalCharset secondaryG0);

    /**
     * Destructor.
     */
    virtual ~CharsetMapping() = default;

    /**
     * Switches between primary and alternative G0 charsets.
     */
    void switchG0Charset();

    /**
     * Sets active G1 charset.
     */
    void setCurrentG1(Charset g1Charset);

    /**
     * Maps character according to selected G0 charset.
     *
     * @param character
     *      Character to map.
     *
     * @return
     *      Pair consisting of mapped character (first element) and property (second element).
     */
    CharPropertyPair mapG0Character(std::uint16_t character) const;

    /**
     * Maps character according to selected G1 charset.
     *
     * @param character
     *      Character to map.
     *
     * @return
     *      Pair consisting of mapped character (first element) and property (second element).
     */
    CharPropertyPair mapG1Character(std::uint16_t character) const;

private:

    /**
     * Converts character to charset table index.
     *
     * @param character
     *      Character to convert.
     *
     * @return
     *      Index of character in charset table or index of 'space' if character was out of bounds.
     */
    std::uint16_t characterToIndex(const std::uint16_t character) const;

    /**
     * Helper function mapping character using given charset map.
     *
     * @param character
     *      Character to map.
     * @param charsetMap
     *      Charset map to use.
     *
     * @return
     *      Pair consisting of mapped character (first element) and property (second element).
     */
    CharPropertyPair mapCharacter(const uint16_t character,
                                  const CharsetMappingArray& charsetMap) const;

    /**
     * Checks if character is replaceable in given charset.
     *
     * @param character
     *      Character code.
     * @param charset
     *      Charset used.
     *
     * @return
     *      True if character should be replaced, false otherwise.
     */
    bool isReplacable(std::uint16_t character,
                      NationalCharset charset) const;

    /** Default charset configuration. */
    const CharsetConfig& m_charsetConfig;

    /** Helper variable to easier access to charset maps. */
    const CharsetMaps& m_charsetMaps;

    /** Primary G0/G2 charset. */
    NationalCharset m_primaryG0;

    /** Secondary G0 charset. */
    NationalCharset m_secondaryG0;

    /** Current G0 charset. */
    NationalCharset m_currentG0;

    /** Active G1 charset - either G1_BLOCK_MOSAIC or G1_BLOCK_MOSAIC_SEPARATED. */
    Charset m_G1Charset;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CHARSETMAPPING_HPP*/
