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


#include "PesFinder.hpp"

namespace subttxrend
{
namespace ctrl
{

PesFinder::PesFinder(const std::uint8_t* buffer,
                     std::size_t bufferSize) :
        m_bufferPosition(buffer),
        m_bufferEnd(m_bufferPosition + bufferSize),
        m_logger("App", "PesFinder")
{
    // noop
}

bool PesFinder::findNextPes(const std::uint8_t*& pesStart,
                            std::uint16_t& pesSize)
{
    while (m_bufferPosition < m_bufferEnd)
    {
        std::size_t bytesLeft = m_bufferEnd - m_bufferPosition;
        if (bytesLeft < 6)
        {
            m_logger.trace("%s Not enough PES bytes", __LOGGER_FUNC__);
            return false;
        }

        if (m_bufferPosition[0] != 0)
        {
            m_logger.trace("%s skipping 1 byte", __LOGGER_FUNC__);
            m_bufferPosition += 1;
            continue;
        }

        if (m_bufferPosition[1] != 0)
        {
            m_logger.trace("%s skipping 2 bytes", __LOGGER_FUNC__);
            m_bufferPosition += 2;
            continue;
        }

        if (m_bufferPosition[2] != 1)
        {
            m_logger.trace("%s skipping 3 bytes", __LOGGER_FUNC__);
            m_bufferPosition += 3;
            continue;
        }

        if (m_bufferPosition[3] != 0xBD)
        {
            m_logger.trace("%s skipping 3 bytes", __LOGGER_FUNC__);
            m_bufferPosition += 3;
            continue;
        }

        std::uint16_t length1 = m_bufferPosition[4] & 0xFF;
        std::uint16_t length2 = m_bufferPosition[5] & 0xFF;
        std::uint16_t length = (length1 << 8) | length2;

        m_logger.trace("%s Found PES of length: %u", __LOGGER_FUNC__, length);
        m_logger.trace("%s Bytes left: %zu", __LOGGER_FUNC__, bytesLeft);

        if (length == 0)
        {
            m_logger.warning("%s Zero length PES not supported", __LOGGER_FUNC__);
            return false;
        }

        if (bytesLeft < 6U + length)
        {
            m_logger.warning("%s Not enough PES data bytes", __LOGGER_FUNC__);
            // not enough bytes
            return false;
        }

        pesStart = m_bufferPosition;
        pesSize = 6 + length;

        m_bufferPosition = pesStart + pesSize;

        m_logger.trace("%s Found PES. Total size: %u", __LOGGER_FUNC__, pesSize);

        return true;
    }

    m_logger.trace("%s No PES data", __LOGGER_FUNC__);

    return false;
}

} // namespace ctrl
} // namespace subttxrend

