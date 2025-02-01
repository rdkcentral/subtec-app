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


#ifndef TTXDECODER_PROPERTY_HPP_
#define TTXDECODER_PROPERTY_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Page element properties.
 */
namespace Property
{

/** VALUE_FLASH. */
const std::uint16_t VALUE_FLASH = 0x0010;
/** VALUE_UNDERLINED. */
const std::uint16_t VALUE_UNDERLINED = 0x0020;
/** VALUE_DOUBLE_HEIGHT. */
const std::uint16_t VALUE_DOUBLE_HEIGHT = 0x0040;
/** VALUE_DOUBLE_WIDTH. */
const std::uint16_t VALUE_DOUBLE_WIDTH = 0x0080;
/** VALUE_CONCEALED. */
const std::uint16_t VALUE_CONCEALED = 0x0100;
/** VALUE_HIDDEN. */
const std::uint16_t VALUE_HIDDEN = 0x2000;

/** VALUE_DIACRITIC_NONE. */
const std::uint16_t VALUE_DIACRITIC_NONE = (0x0 << 9);
/** VALUE_DIACRITIC_GRAVE. */
const std::uint16_t VALUE_DIACRITIC_GRAVE = (0x1 << 9);
/** VALUE_DIACRITIC_ACUTE. */
const std::uint16_t VALUE_DIACRITIC_ACUTE = (0x2 << 9);
/** VALUE_DIACRITIC_CIRCUMFLEX. */
const std::uint16_t VALUE_DIACRITIC_CIRCUMFLEX = (0x3 << 9);
/** VALUE_DIACRITIC_TILDE. */
const std::uint16_t VALUE_DIACRITIC_TILDE = (0x4 << 9);
/** VALUE_DIACRITIC_MACRON. */
const std::uint16_t VALUE_DIACRITIC_MACRON = (0x5 << 9);
/** VALUE_DIACRITIC_BREVE. */
const std::uint16_t VALUE_DIACRITIC_BREVE = (0x6 << 9);
/** VALUE_DIACRITIC_DOT_ABOVE. */
const std::uint16_t VALUE_DIACRITIC_DOT_ABOVE = (0x7 << 9);
/** VALUE_DIACRITIC_UMLAUT. */
const std::uint16_t VALUE_DIACRITIC_UMLAUT = (0x8 << 9);
/** VALUE_DIACRITIC_DOT_BELOW. */
const std::uint16_t VALUE_DIACRITIC_DOT_BELOW = (0x9 << 9);
/** VALUE_DIACRITIC_RING. */
const std::uint16_t VALUE_DIACRITIC_RING = (0xA << 9);
/** VALUE_DIACRITIC_CEDILLA. */
const std::uint16_t VALUE_DIACRITIC_CEDILLA = (0xB << 9);
/** VALUE_DIACRITIC_UNDERLINE. */
const std::uint16_t VALUE_DIACRITIC_UNDERLINE = (0xC << 9);
/** VALUE_DIACRITIC_DOUBLE_ACUTE. */
const std::uint16_t VALUE_DIACRITIC_DOUBLE_ACUTE = (0xD << 9);
/** VALUE_DIACRITIC_OGONEK. */
const std::uint16_t VALUE_DIACRITIC_OGONEK = (0xE << 9);
/** VALUE_DIACRITIC_CARON. */
const std::uint16_t VALUE_DIACRITIC_CARON = (0xF << 9);

/** Diacritic mask. */
const std::uint16_t MASK_DIACRITIC = (0xF << 9);

} // namespace Property

} // namespace ttxdecoder

#endif /*TTXDECODER_PROPERTY_HPP_*/
