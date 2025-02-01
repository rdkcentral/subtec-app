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


#ifndef SUBTTXREND_COMMON_LOGGERLEVEL_HPP_
#define SUBTTXREND_COMMON_LOGGERLEVEL_HPP_

#include <cstdint>

namespace subttxrend
{
namespace common
{

/**
 * Logger logging levels.
 */
enum class LoggerLevel : std::uint32_t
{
    /** Logging level - FATAL. */
    FATAL = (1 << 0),

    /** Logging level - ERROR. */
    ERROR = (1 << 1),

    /** Logging level - WARNING. */
    WARNING = (1 << 2),

    /** Logging level - INFO. */
    INFO = (1 << 3),

    /** Logging level - DEBUG. */
    DEBUG = (1 << 4),

    /** Logging level - TRACE. */
    TRACE = (1 << 5),
};

/** Logger level flags (combination of LoggerLevel values). */
typedef std::uint32_t LoggerLevelFlags;

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGERLEVEL_HPP_*/
