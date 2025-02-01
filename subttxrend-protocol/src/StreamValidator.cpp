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


#include "StreamValidator.hpp"

namespace subttxrend
{
namespace protocol
{

common::Logger StreamValidator::m_logger("Protocol", "StreamValidator");

StreamValidator::StreamValidator() :
        m_streamValid(true),
        m_nextCounter(0)
{
    // noop
}

bool StreamValidator::validate(const Packet& packet)
{
    if (packet.isValid())
    {
        m_logger.trace("%s - validating packet (type=%d, counter=%u)", __LOGGER_FUNC__,
                static_cast<int>(packet.getType()), packet.getCounter());

        if (packet.getType() == Packet::Type::RESET_ALL)
        {
            m_logger.debug("%s - RESET ALL received - resetting stream state",
                    __LOGGER_FUNC__);
            reset();
        }
        else
        {
            if (isValid())
            {
                (void)validateCounter(packet.getCounter());
            }
            else
            {
                m_logger.trace("%s - Stream not valid, ignoring packet",
                        __LOGGER_FUNC__);

            }
        }
    }
    else
    {
        m_logger.error("%s - Invalid packet received, invalidating stream",
                __LOGGER_FUNC__);
        invalidate();
    }

    return isValid();
}

bool StreamValidator::validateCounter(std::uint32_t newCounterValue)
{
    // packet counter check fails when two processes are sending packets to subtec
    // because they both start with counter=0
    return true;
    // if (newCounterValue == m_nextCounter)
    // {
    //     m_logger.trace("%s - Matching packet counter, continuing",
    //             __LOGGER_FUNC__);
    //     ++m_nextCounter;
    // }
    // else
    // {
    //     m_logger.error(
    //             "%s - Packet counter mismatch (expected: %u, found: %u), invalidating stream",
    //             __LOGGER_FUNC__, m_nextCounter,
    //             newCounterValue);
    //     invalidate();
    // }
    // return isValid();
}

bool StreamValidator::isValid() const
{
    return m_streamValid;
}

void StreamValidator::invalidate()
{
    m_streamValid = false;
    m_nextCounter = 0;
}

void StreamValidator::reset()
{
    m_streamValid = true;
    m_nextCounter = 1;
}

}
// namespace protocol
}// namespace subttxrend
