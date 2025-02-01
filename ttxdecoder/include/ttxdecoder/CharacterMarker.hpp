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


#ifndef TTXDECODER_CHARACTERMARKER_HPP_
#define TTXDECODER_CHARACTERMARKER_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Page character type markers.
 */
namespace CharacterMarker
{

/** MASK_BLOCK_MOSAIC */
const std::uint16_t MASK_BLOCK_MOSAIC = 0xE300;
/** MASK_SEPARATE_MOSAIC */
const std::uint16_t MASK_SEPARATE_MOSAIC = 0xE400;
/** MASK_SMOOTH_MOSAIC */
const std::uint16_t MASK_SMOOTH_MOSAIC = 0xE500;
/** MASK_DRCS */
const std::uint16_t MASK_DRCS = 0xF800;

} // namespace CharacterMarker

} // namespace ttxdecoder

#endif /*TTXDECODER_CHARACTERMARKER_HPP_*/
