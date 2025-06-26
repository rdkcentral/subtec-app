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


#include "StcProvider.hpp"

#include <sys/time.h>
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace ctrl
{

namespace
{

subttxrend::common::Logger g_logger("App", "StcProvider");

}

StcProvider::StcProvider() :
        m_lastStc(),
        m_lastStcTimestampMs(),
        m_stcDataMutex()
{
    // noop
}

void StcProvider::processTimestamp(std::uint32_t newStc,
                                   std::uint64_t timestampMs)
{
    std::lock_guard<std::mutex> lock(m_stcDataMutex);

    m_lastStcTimestampMs = timestampMs;
    m_lastStc = newStc;

    g_logger.trace("%s update stc: %u timestamp:  %u", __LOGGER_FUNC__,
            m_lastStc, static_cast<std::uint32_t>(m_lastStcTimestampMs));
}

std::uint32_t StcProvider::stcCallback(void *instance)
{
    StcProvider* thiz = static_cast<StcProvider*>(instance);
    return thiz->getStc();
}

std::uint32_t StcProvider::getStc() const
{
    std::lock_guard<std::mutex> lock(m_stcDataMutex);

    const std::uint64_t timestampDiffMs = getCurrentTimestampMs()
            - m_lastStcTimestampMs;

    static constexpr std::uint64_t DIFF_ONE_HOUR_MS = 1 * 60 * 60 * 1000;

    std::uint32_t stcUpdate = 0;
    if (timestampDiffMs > DIFF_ONE_HOUR_MS)
    {
        g_logger.debug("%s skipping stc update - timestamp diff too big: %u ms",
                __LOGGER_FUNC__, static_cast<std::uint32_t>(timestampDiffMs));
    }
    else
    {
        stcUpdate = convertToHighStcUnits(timestampDiffMs);
    }

    return m_lastStc + stcUpdate;
}

std::uint64_t StcProvider::getCurrentTimestampMs() const
{
    struct timeval tv = timeval();
    gettimeofday(&tv, NULL);

    const std::uint64_t clockSec = tv.tv_sec;
    const std::uint64_t clockUsec = tv.tv_usec;

    return (clockSec * 1000) + (clockUsec / 1000);
}

std::uint32_t StcProvider::convertToHighStcUnits(const std::uint64_t valueMs) const
{
    /*
     * "full" stc: 33 bits with 90kHz units
     * high stc: 32 most significant bits of "full" 90kHz value => 45kHz units
     *
     * stcValue = (valueMs / 1000) * 45000 => stcValue = (valueMs) * 45
     */
    return (valueMs * 45);
}

} // namespace ctrl
} // namespace subttxrend

