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


#ifndef SUBTTXREND_PROTOCOL_STREAMVALIDATOR_HPP_
#define SUBTTXREND_PROTOCOL_STREAMVALIDATOR_HPP_

#include <subttxrend/common/Logger.hpp>

#include "Packet.hpp"

namespace subttxrend
{
namespace protocol
{

/**
 * Packet stream validator.
 */
class StreamValidator
{
public:
    /**
     * Constructor.
     */
    StreamValidator();

    /**
     * Destructor.
     */
    virtual ~StreamValidator() = default;

    /**
     * Checks current stream state.
     *
     * @return
     *      Current stream state.
     */
    bool isValid() const;

    /**
     * Validates next packet.
     *
     * @param packet
     *      Packet to analyze.
     *
     * @retval true
     *      Stream is valid. The packet could be processed.
     * @retval false
     *      Stream is invalid. The packet should be dropped.
     */
    bool validate(const Packet& packet);

    /**
     * Validates new counter. Checks if given value is the next
     * expected value.
     *
     * @param counterValue
     *      Counter value to validate.
     *
     * @retval true
     *      Stream is valid. The packet could be processed.
     * @retval false
     *      Stream is invalid. The packet should be dropped.
     */
    bool validateCounter(std::uint32_t counterValue);

private:
    /**
     * Sets stream state to invalid.
     */
    void invalidate();

    /**
     * Resets stream state.
     *
     * Stream validity is set to true and next expected counter value is reset.
     */
    void reset();

    /** Stream validity flag. */
    bool m_streamValid;

    /** Next expected counter value. */
    std::uint32_t m_nextCounter;

    /** Logger to use. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_STREAMVALIDATOR_HPP_*/
