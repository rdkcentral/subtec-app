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


#ifndef SUBTTXREND_DVBSUB_DECODER_TIME_PROVIDER_HPP_
#define SUBTTXREND_DVBSUB_DECODER_TIME_PROVIDER_HPP_

#include <dvbsubdecoder/TimeProvider.hpp>

#include "TimeSource.hpp"

namespace subttxrend
{
namespace dvbsub
{

/**
 * Time provider interface implementation.
 */
class DecoderTimeProvider : public dvbsubdecoder::TimeProvider
{
public:
    /**
     * Constructor.
     */
    DecoderTimeProvider() :
            m_timeSource(nullptr)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~DecoderTimeProvider() = default;

    /**
     * Initializes the time provider.
     *
     * @param timeSource
     *      Time source to use.
     */
    void init(TimeSource* timeSource)
    {
        m_timeSource = timeSource;
    }

    /**
     * Returns STC time.
     *
     * @return
     *      STC time information.
     */
    virtual dvbsubdecoder::StcTime getStc() override
    {
        if (m_timeSource)
        {
            return dvbsubdecoder::StcTime(dvbsubdecoder::StcTimeType::HIGH_32,
                    m_timeSource->getStc());
        }

        return dvbsubdecoder::StcTime();
    }

private:
    /** Time source to use. */
    TimeSource* m_timeSource;
};

} // namespace dvbsub
} // namespace subttxrend

#endif /*SUBTTXREND_DVBSUB_DECODER_TIME_PROVIDER_HPP_*/
