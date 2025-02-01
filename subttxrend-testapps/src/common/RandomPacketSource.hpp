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


#ifndef SUBTTXREND_TESTAPPS_RANDOMPACKETSOURCE_HPP_
#define SUBTTXREND_TESTAPPS_RANDOMPACKETSOURCE_HPP_

#include <cstdint>

#include "DataSource.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Random data source.
 */
class RandomPacketSource : public DataSource
{
public:
    /**
     * Constructor.
     *
     * @param path
     *      Source path.
     */
    RandomPacketSource(const std::string& path);

    /**
     * Destructor.
     */
    virtual ~RandomPacketSource() = default;

    /** @copydoc DataSource::open() */
    virtual bool open() override;

    /** @copydoc DataSource::close() */
    virtual void close() override;

    /** @copydoc DataSource::readPacket() */
    virtual bool readPacket(DataPacket& packet) override;

private:
    static const std::uint32_t PACKET_TYPE_PES_DATA = 1;
    static const std::uint32_t PACKET_TYPE_TIMESTAMP = 2;
    static const std::uint32_t PACKET_TYPE_RESET_ALL = 3;
    static const std::uint32_t PACKET_TYPE_RESET_CHANNEL = 4;
    static const std::uint32_t PACKET_TYPE_SUBTITLE_SELECTION = 5;
    static const std::uint32_t PACKET_TYPE_TELETEXT_SELECTION = 6;

    /**
     * Generates packet.
     *
     * @param packet
     *      Pointer to packet to be filled with data.
     *
     * @retval true
     *      Packet was read.
     *      If the size of packet read is zero then there was no more data.
     * @retval false
     *      Error occurred during read operation.
     */
    bool generatePacket(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generatePesData(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generateTimestamp(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generateResetAll(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generateResetChannel(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generateSubtitleSelection(DataPacket& packet);

    /** @copydoc RandomSource::generatePacket */
    bool generateTeletextSelection(DataPacket& packet);

    /** Number of packets to generate. Zero means infinite. */
    uint32_t m_packetCount;

    /** Sleep time between packets in milliseconds. Zero means no sleep. */
    uint32_t m_sleepTimeMs;

    /** Packet counter. Only used if limit is set. */
    uint32_t m_packetNo;

    /** Value of the next counter. */
    uint32_t m_nextPacketCounter;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_RANDOMPACKETSOURCE_HPP_*/
