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


#ifndef DVBSUBDECODER_CONFIG_HPP_
#define DVBSUBDECODER_CONFIG_HPP_

#include <cstddef>
#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Maximum number of supported regions.
 */
const std::size_t MAX_SUPPORTED_REGIONS = 16;

/**
 * Maximum number of supported CLUTs.
 */
const std::size_t MAX_SUPPORTED_CLUTS = MAX_SUPPORTED_REGIONS;

/**
 * Maximum number of supported objects.
 */
const std::size_t MAX_SUPPORTED_OBJECTS = 256;

/**
 * Minimum acceptable timestamp difference.
 *
 * Also provides some time for decoding.
 *
 * Calculation: 50 ms * 90 (khz units).
 */
const std::uint32_t PTS_STC_DIFF_MIN_90KHZ = 50 * 90;

/**
 * Maximum acceptable timestamp difference.
 *
 * Calculation: 30 sec * 90 (khz units).
 */
const std::uint32_t PTS_STC_DIFF_MAX_90KHZ = 30 * 1000 * 90;

/**
 * Maximum acceptable PTS delay in 90 khz units.
 *
 * Calculation: 3 sec * 90 (khz units).
 */
const std::uint32_t PES_DELAY_MAX_90KHZ = 3 * 1000 * 90;

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_CONFIG_HPP_*/
