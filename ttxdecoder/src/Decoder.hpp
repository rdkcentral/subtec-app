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


#ifndef TTXDECODER_DECODER_HPP_
#define TTXDECODER_DECODER_HPP_

#include <array>

#include <subttxrend/common/NonCopyable.hpp>

#include "PresentationLevel.hpp"
#include "Collector.hpp"
#include "CollectorListener.hpp"
#include "MetadataProcessor.hpp"
#include "PacketHeader.hpp"

namespace ttxdecoder
{

class Database;
class Cache;
class PesPacketReader;
class DecoderListener;

/**
 * Decoder.
 *
 * @note Internal decoding methods may throw PesPacketReader::Exception
 *       when the data cannot be read from the reader.
 */
class Decoder : private subttxrend::common::NonCopyable,
                private CollectorListener
{
public:
    /**
     * Constructor.
     *
     * @param database
     *      Database to use.
     * @param cache
     *      Page cache to use.
     * @param listener
     *      Listener for decoder events.
     */
    Decoder(Database& database,
            Cache& cache,
            DecoderListener& listener);

    /**
     * Destructor.
     */
    virtual ~Decoder();

    /**
     * Resets decoder state.
     *
     * All currently decoded data is dropped.
     */
    void reset();

    /**
     * Processes PES packet data.
     *
     * @param reader
     *      Reader for PES packet data.
     *
     * @throws PesPacketReader::Exception
     *      If data cannot be read from reader.
     */
    void processPacketData(PesPacketReader& reader);

private:

    struct PageInfo
    {
        /** Page pointer. */
        Page* page{nullptr};

        /** Flag indicating page was taken from cache. */
        bool fromCache{false};

        void reset()
        {
            fromCache = false;
            page = nullptr;
        }
    };

    /** @copydoc CollectorListener::onPacketReady */
    virtual void onPacketReady(CollectorPacketContext& context) override;

    /**
     * Prepares packet to be collected.
     *
     * @param context
     *      Packet collecting context.
     *
     * @return
     *      Packet pointer or null if packet is not needed.
     */
    Packet* preparePacket(const CollectorPacketContext& context);

    /**
     * Processes collected packet.
     *
     * @param collectedPacket
     *      Collected packet.
     */
    void processPacket(const Packet& collectedPacket);

    /**
     * Processes currently collected page(s).
     *
     * @param newHeader
     *      Newly received page header.
     */
    void processCurrentPage(const PacketHeader& newHeader);

    /**
     * Sets current page.
     *
     * @param collectedHeader
     *      Header packet data.
     */
    void setCurrentPage(const PacketHeader& collectedHeader);

    /**
     * Sets current page header.
     *
     * @param collectedHeader
     *      Header packet data.
     */
    void setCurrentPageHeader(const PacketHeader& collectedHeader);

    /**
     * Processes given PageInfo.
     *
     * @param pageInfo
     *      PageInfo to process.
     */
    void processPageInfo(PageInfo& pageInfo);

    /** Maximum number of magazines. */
    constexpr static int MAGAZINES_NUMBER = 8;

    /** Database to use. */
    Database& m_database;

    /** Cache to use. */
    Cache& m_cache;

    /** Listener for decoder events. */
    DecoderListener& m_listener;

    /** Page data collector. */
    Collector m_collector;

    /** Header packet buffer. */
    PacketHeader m_headerPacketBuffer;

    /** Teletext transmission mode. */
    enum class Mode
    {
        SERIAL,
        PARALLEL
    };

    /** Current teletext mode. */
    Mode m_mode;

    /** Current page per magazine. */
    std::array<PageInfo, MAGAZINES_NUMBER> m_currentPages;

    /** Metadata processor. */
    MetadataProcessor m_metadataProcessor;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_DECODER_HPP_*/
