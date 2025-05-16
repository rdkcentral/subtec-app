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


#ifndef ONEMW_SRC_AV_SUBTTXREND_APP_SRC_PESFINDER_HPP_
#define ONEMW_SRC_AV_SUBTTXREND_APP_SRC_PESFINDER_HPP_

#include <cstdint>
#include <cstddef>

#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace ctrl
{

/**
 * Class designed to find PES packets in a buffer. There can be multiple PES packets
 * present and each call to "findNextPes" will return start of next packet until none
 * is found.
 *
 */
class PesFinder
{
public:

    /**
     * The only constructor.
     *
     * @param buffer
     *      Data buffer with (possibly multiple) pes data.
     * @param bufferSize
     *      Size of data buffer.
     */
    PesFinder(const std::uint8_t* buffer,
              std::size_t bufferSize);

    /**
     * Destructor.
     */
    ~PesFinder() = default;

    /**
     * Finds next PES packet.
     *
     * @param pesStart
     *      Variable to store the pointer to PES found.
     * @param pesSize
     *      Variable to store the size of PES found.
     *
     * @retval true
     *      PES was found.
     * @retval false
     *      PES not found.
     */
    bool findNextPes(const std::uint8_t*& pesStart,
                     std::uint16_t& pesSize);

private:

    /** First unprocessed byte in a buffer. */
    const std::uint8_t* m_bufferPosition;

    /** Byte just after the buffer. */
    const std::uint8_t* m_bufferEnd;

    /** Logger object. */
    common::Logger m_logger;
};

} // namespace ctrl
} // namespace subttxrend

#endif /* ONEMW_SRC_AV_SUBTTXREND_APP_SRC_PESFINDER_HPP_ */
