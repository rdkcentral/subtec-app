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

#ifndef AV_SUBTTXREND_APP_STCPROVIDER_HPP
#define AV_SUBTTXREND_APP_STCPROVIDER_HPP

#include <cstdint>
#include <mutex>

#include <subttxrend/common/StcProvider.hpp>
#include <subttxrend/common/NonCopyable.hpp>

namespace subttxrend
{
namespace app
{
/**
 * Process timestamp messages and provides current stc value on request.
 */
class StcProvider : public common::StcProvider, private common::NonCopyable
{
public:

    /**
     * Constructor.
     */
    StcProvider();

    /**
     * Destructor.
     */
    virtual ~StcProvider() = default;

    /**
     * Processes timestamp packet.
     *
     * @param newStc
     *      New stc value.
     * @param stcTimestampMs
     *      Timestamp of stc.
     */
    void processTimestamp(std::uint32_t newStc,
                          std::uint64_t stcTimestampMs);

    /**
     * Current stc getter.
     *
     * @return
     *      Current stc value.
     */
    virtual std::uint32_t getStc() const override;

    /**
     * Callback function to be registered for stc queries.
     *
     * @param instance
     *      Pointer to class instance.
     * @return
     *      Current stc value.
     */
    static std::uint32_t stcCallback(void *instance);

private:

    /**
     * Return current time in ms since unix epoch.
     *
     * @return
     *      Number of milliseconds since unix epoch.
     */
    std::uint64_t getCurrentTimestampMs() const;

    /**
     * Converts value in milliseconds to stc 45kHz units (32 most significant bit of 90kHz 33 bits value).
     *
     * @param valueUs
     *      Value to convert in milliseconds.
     * @return
     *      Value converted to 45kHz units.
     */
    std::uint32_t convertToHighStcUnits(const std::uint64_t valueUs) const;

    /** Last received stc. */
    std::uint32_t m_lastStc;

    /** Last time stc was received. */
    std::uint64_t m_lastStcTimestampMs;

    /** Mutex preventing race condition on stc value and timestamp. */
    mutable std::mutex m_stcDataMutex;
};

} // namespace app
} // namespace subttxrend

#endif /* AV_SUBTTXREND_APP_STCPROVIDER_HPP */
