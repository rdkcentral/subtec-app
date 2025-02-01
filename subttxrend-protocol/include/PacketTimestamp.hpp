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


#ifndef SUBTTXREND_PROTOCOL_PACKETTIMESTAMP_HPP_
#define SUBTTXREND_PROTOCOL_PACKETTIMESTAMP_HPP_

#include "Packet.hpp"

namespace subttxrend {
namespace protocol {

/**
 * Packet - TIMESTAMP.
 */
class PacketTimestamp final : public PacketGeneric
{
  public:
    /**
     * Constructor.
     */
    PacketTimestamp();

    /**
     * Returns STC.
     *
     * @return
     *      STC in 45kHz units.
     */
    std::uint32_t getStc() const;

    /**
     * Returns timestamp.
     *
     * @return
     *      Timestamp (msec since unix epoch).
     */
    std::uint64_t getTimestamp() const;

  protected:
    /** @copydoc Packet::parseData */
    bool parseDataHeader(BufferReader& bufferReader) override;

  private:
    /** STC. */
    std::uint32_t m_stc{};

    /** Timestamp. */
    std::uint64_t m_timestamp{};

    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETTIMESTAMP_HPP_*/
