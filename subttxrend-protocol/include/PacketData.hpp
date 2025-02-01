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


#ifndef SUBTTXREND_PROTOCOL_PACKETCCDATA_HPP_
#define SUBTTXREND_PROTOCOL_PACKETCCDATA_HPP_

#include "Packet.hpp"

#include <vector>

namespace subttxrend {
namespace protocol {

/**
 * Packet - data.
 */
class PacketData : public PacketChannelSpecific
{
  public:
    /**
     * Constructor.
     */
    PacketData(Type type);

    /**
     * Returns channel type.
     *
     * @return
     *      Channel type.
     */
    std::uint32_t getChannelType() const;

    /**
     * Returns CC data buffer.
     *
     * @return
     *      CC data buffer (may be null for invalid packets!).
     */
    const char* getData() const;

    /**
     * Returns CC data size.
     *
     * @return
     *      CC data buffer size in bytes.
     */
    std::size_t getDataSize() const;

    /**
     * Returns offset for calculating display time.
     *
     * @return
     *      data display offset.
     */
    std::int64_t getDisplayOffset() const;

  protected:
    /** @copydoc Packet::parseData */
    virtual bool parseDataHeader(BufferReader& bufferReader) override;

    /** @copydoc Packet::takeBuffer */
    bool takeBuffer(common::DataBufferPtr dataBuffer, std::size_t dataOffset) override;

  private:

    /** Channel Type. */
    std::uint32_t m_channelType{static_cast<std::uint32_t>(-1)};

    common::DataBufferPtr m_dataBuffer;
    std::size_t m_dataOffset{0};

    /** Data display offset in ms. */
    std::int64_t m_displayOffsetMs{0};

    /** Logger. */
    static common::Logger m_logger;
};

} // namespace protocol
} // namespace subttxrend

#endif /*SUBTTXREND_PROTOCOL_PACKETCCDATA_HPP_*/
