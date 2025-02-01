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


#ifndef TTXDECODER_METADATAPROCESSOR_HPP_
#define TTXDECODER_METADATAPROCESSOR_HPP_

#include "TopNavProcessor.hpp"
#include "PacketBcastServiceData.hpp"
#include "PageMagazine.hpp"

namespace ttxdecoder
{

class Packet;
class PageDisplayable;
class PageBtt;
class Database;

/**
 * Metadata processor.
 */
class MetadataProcessor
{
public:
    /**
     * Constructor.
     *
     * @param database
     *      Database to use.
     */
    MetadataProcessor(Database& database);

    /**
     * Destructor.
     */
    ~MetadataProcessor() = default;

    /**
     * Resets the processor state.
     */
    void reset();

    /**
     * Returns page buffer.
     *
     * Asks the processor for buffer for processing metadata.
     * If processor is interested in the given page it shall return
     * buffer. The buffer shall remain valid until next call to getPageBuffer.
     *
     * @param pageId
     *      Page id.

     * @return
     *      Page buffer or nullptr if page is not needed
     */
    Page* getPageBuffer(const PageId& pageId);

    /**
     * Returns packet buffer.
     *
     * Asks the processor for buffer for processing metadata.
     * If processor is interested in the given packet it shall return
     * buffer. The buffer shall remain valid until next call to getPacketBuffer.
     *
     * @param magazine
     *      Magazine number (0-7).
     * @param packetAddress
     *      Packet address (0-31).
     * @param designationCode
     *      Designation code. Only valid when defined by packet type.
     *      Hamming-8-4 decoded, but not verified (will be negative if
     *      value is not valid for hamming encoding).
     *
     * @return
     *      Packet buffer or nullptr.
     */
    Packet* getPacketBuffer(std::uint8_t magazine,
                            std::uint8_t packetAddress,
                            std::int8_t designationCode);

    /**
     * Processes the collected page.
     *
     * @param page
     *      Page to be processed.
     */
    void processPage(const Page& page);

    /**
     * Processes the collected packet.
     *
     * @param packet
     *      Packet to be processed.
     */
    void processPacket(const Packet& packet);

private:
    /**
     * Processes the collected packet.
     *
     * @param packet
     *      Packet to be processed.
     */
    void processTypedPacket(const PacketHeader& packet);

    /**
     * Processes the collected packet.
     *
     * @param packet
     *      Packet to be processed.
     */
    void processTypedPacket(const PacketBcastServiceData& packet);

    /**
     * Processes the collected packet.
     *
     * @param packet
     *      Packet to be processed.
     */
    void processTypedPacket(const PacketTriplets& packet);

    /** Database to use. */
    Database& m_database;

    /** TOP navigation data processor. */
    TopNavProcessor m_topNavProcessor;

    /** Packet buffer. */
    PacketBcastServiceData m_bsdPacket;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_METADATAPROCESSOR_HPP_*/
