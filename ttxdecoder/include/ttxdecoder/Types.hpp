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


#ifndef TTXDECODER_TYPES_HPP_
#define TTXDECODER_TYPES_HPP_

#include <array>
#include <cstdint>
namespace ttxdecoder
{

/**
 * Charset mapping array size.
 */
constexpr std::size_t CHARSET_MAPPING_SIZE = 96;

/**
 * Charset mapping array.
 *
 * The range of mapped characters is 20..7F (96).
 */
using CharsetMappingArray = std::array<std::uint16_t, CHARSET_MAPPING_SIZE>;

/**
 * Page type.
 */
enum class PageIdType
{
    /** Navigation - FLOF index page. */
    FLOF_INDEX_PAGE,
    /** Navigation - red key page. */
    RED_KEY,
    /** Navigation - green key page. */
    GREEN_KEY,
    /** Navigation - yellow key page. */
    YELLOW_KEY,
    /** Navigation - cyan key page. */
    CYAN_KEY,
    /** Navigation - white key page. */
    WHITE_KEY,

    /** Currently requested page. */
    CURRENT_PAGE,

    /** Current page related - actual subpage. */
    ACTUAL_SUBPAGE,
    /** Current page related - first subpage. */
    FIRST_SUBPAGE,
    /** Current page related - next subpage. */
    NEXT_SUBPAGE,
    /** Current page related - previous subpage. */
    PREV_SUBPAGE,
    /** Current page related - highest subpage seen. */
    HIGHEST_SUBPAGE,
    /** Current page related - last received subpage. */
    LAST_RECEIVED_SUBPAGE,

    /** Last available page. */
    LAST_PAGE,
    /** Previously requested page. */
    PRIOR_PAGE,
    /** Index page from packet 8/30. */
    INDEX_PAGE_P830
};

/**
 * Navigation mode.
 *
 * Navigation mode defines in which order the decoder tries to prepare
 * page navigation data.
 */
enum class NavigationMode
{
    /** Navigation modes order: DEFAULT. */
    DEFAULT,
    /** Navigation modes order: TOP, DEFAULT. */
    TOP_DEFAULT,
    /** Navigation modes order: FLOF, DEFAULT. */
    FLOF_DEFAULT,
    /** Navigation modes order: TOP, FLOF, DEFAULT. */
    TOP_FLOF_DEFAULT,
    /** Navigation modes order: FLOF, TOP, DEFAULT. */
    FLOF_TOP_DEFAULT,
};

/**
 * Navigation state.
 *
 * Navigation state shows which navigation mode data is available
 * in the decoded page.
 */
enum class NavigationState
{
    /** Default. */
    DEFAULT,
    /** TOP. */
    TOP,
    /** FLOF. */
    FLOF,
    /** TOP with text. */
    FLOF_TEXT,
};

/**
 * Character sets.
 */
enum class Charset
{
    G0_LATIN,                   //!< G0_LATIN
    G0_ARABIC,                  //!< G0_ARABIC
    G0_CYRILLIC1,               //!< G0_CYRILLIC1
    G0_CYRILLIC2,               //!< G0_CYRILLIC2
    G0_CYRILLIC3,               //!< G0_CYRILLIC3
    G0_GREEK,                   //!< G0_GREEK
    G0_HEBREW,                  //!< G0_HEBREW
    G0_FARSI,                   //!< G0_FARSI
    G1_BLOCK_MOSAIC,            //!< G1_BLOCK_MOSAIC
    G1_BLOCK_MOSAIC_SEPARATED,  //!< G1_BLOCK_MOSAIC_SEPARATED
    G2_LATIN,                   //!< G2_LATIN
    G2_ARABIC,                  //!< G2_ARABIC
    G2_CYRILLIC,                //!< G2_CYRILLIC
    G2_GREEK,                   //!< G2_GREEK
    G2_FARSI,                   //!< G2_FARSI
    G3_SMOOTH_MOSAIC,           //!< G3_SMOOTH_MOSAIC
    G3_SMOOTH_MOSAIC_SEPARATED, //!< G3_SMOOTH_MOSAIC_SEPARATED
    DRCS,                       //!< DRCS

    LAST = DRCS,                //!< LAST
};

/**
 * Charset controlled via national options.
 */
enum class NationalCharset
{
    CZECH,          //!< CZECH
    ENGLISH,        //!< ENGLISH
    ESTONIAN,       //!< ESTONIAN
    FRENCH,         //!< FRENCH
    GERMAN,         //!< GERMAN
    ITALIAN,        //!< ITALIAN
    LETTISH,        //!< LETTISH
    POLISH,         //!< POLISH
    SPANISH,        //!< SPANISH
    RUMANIAN,       //!< RUMANIAN
    SERBIAN,        //!< SERBIAN
    SWEDISH,        //!< SWEDISH
    TURKISH,        //!< TURKISH
    CYRILLIC_1,     //!< CYRILLIC_1
    CYRILLIC_2,     //!< CYRILLIC_2
    CYRILLIC_3,     //!< CYRILLIC_3
    GREEK,          //!< GREEK
    HEBREW,         //!< HEBREW
    ARABIC,         //!< ARABIC
    FARSI,          //!< FARSI
    ENGLISH_ARABIC, //!< ENGLISH_ARABIC
    FRENCH_ARABIC,  //!< FRENCH_ARABIC
    ENGLISH_FARSI,  //!< ENGLISH_FARSI
    FRENCH_FARSI,   //!< FRENCH_FARSI
};

} // namespace ttxdecoder

#endif /*TTXDECODER_TYPES_HPP_*/
