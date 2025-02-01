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


#ifndef TTXDECODER_ENGINEIMPL_HPP_
#define TTXDECODER_ENGINEIMPL_HPP_

#include <memory>
#include <subttxrend/common/NonCopyable.hpp>

#include "CharsetManager.hpp" 

#include "Engine.hpp"
#include "PageId.hpp"
#include "DecodedPage.hpp"
#include "DecoderListener.hpp"
#include "Parser.hpp"
#include "PacketHeader.hpp"

namespace ttxdecoder
{

class PageDisplayable;
class PesBuffer;
class PesPacketHeader;
class EngineClient;
class Allocator;
class Database;
class Cache;
class Decoder;
class MetadataProcessor;

/**
 * Engine implementation.
 */
class EngineImpl : public Engine,
                   private subttxrend::common::NonCopyable,
                   private DecoderListener
{
public:
    /**
     * Constructor.
     *
     * @param client
     *      Engine client interface.
     * @param allocator
     *      Allocator.
     */
    EngineImpl(EngineClient& client,
               std::unique_ptr<Allocator> allocator);

    /**
     * Destructor.
     */
    virtual ~EngineImpl();

    /** @copydoc Engine::resetAcquisition */
    virtual void resetAcquisition() override;

    /** @copydoc Engine::process */
    virtual std::uint32_t process() override;

    /** @copydoc Engine::addPesPacket */
    virtual bool addPesPacket(const std::uint8_t* packet,
                              std::uint16_t length) override;

    /** @copydoc Engine::setCurrentPageId */
    virtual void setCurrentPageId(const PageId& pageId) override;

    /** @copydoc Engine::getNextPageId */
    virtual PageId getNextPageId(const PageId& inputPageId) const override;

    /** @copydoc Engine::getPrevPageId */
    virtual PageId getPrevPageId(const PageId& inputPageId) const override;

    /** @copydoc Engine::getPageId */
    virtual PageId getPageId(PageIdType type) const override;

    /** @copydoc Engine::getPage */
    virtual const DecodedPage& getPage() const override;

    /** @copydoc Engine::setNavigationMode */
    virtual void setNavigationMode(NavigationMode navigationMode) override;

    /** @copydoc Engine::getNavigationState */
    virtual NavigationState getNavigationState() const override;

    /** @copydoc Engine::getTopLinkText */
    virtual void getTopLinkText(std::uint16_t magazine,
                                std::uint32_t linkType,
                                std::uint16_t* linkbuffer) const override;

    /** @copydoc Engine::getScreenColorIndex */
    virtual std::uint8_t getScreenColorIndex() const override;

    /** @copydoc Engine::getRowColorIndex */
    virtual std::uint8_t getRowColorIndex(std::uint8_t row) const override;

    /** @copydoc Engine::getColors */
    virtual bool getColors(std::array<std::uint32_t, 16>& colors) const
            override;

    /** @copydoc Engine::getCharsetMapping */
    virtual const CharsetMappingArray& getCharsetMapping(Charset charset) const
            override;

    /** @copydoc Engine::setCharsetMapping */
    virtual void setCharsetMapping(Charset charset,
                                   const CharsetMappingArray& mappingArray)
                                           override;

    /** @copydoc Engine::setDefaultPrimaryNationalCharset */
    virtual void setDefaultPrimaryNationalCharset(std::uint8_t index,
                                                  NationalCharset charset)
                                                          override;

    /** @copydoc Engine::setDefaultSecondaryNationalCharset */
    virtual void setDefaultSecondaryNationalCharset(std::uint8_t index,
                                                    NationalCharset charset)
                                                            override;

    /** @copydoc Engine::getPageControlInfo */
    virtual uint8_t getPageControlInfo() const override;

    /** @copydoc Engine::setIgnorePts */
    virtual void setIgnorePts(bool ignorePts) override;

private:
    /** @copydoc DecoderListener::pageDecoded */
    virtual void pageDecoded(const PageId& pageId) override;

    /** @copydoc DecoderListener::headerDecoded */
    virtual void headerDecoded(const PacketHeader& header) override;

    /**
     * Unsets current page.
     *
     * Clears the current page data.
     *
     * @param useAsStale
     *      If true current page is used as stale page,
     *      if false the current and stale pages are both dropped.
     */
    void unsetCurrentPage(bool useAsStale);

    /**
     * Tries to restore the current page from cache.
     */
    void tryRestoreCurrentPage();

    /**
     * Returns colour key link value.
     *
     * @param link
     *      Link type.
     *
     * @return
     *      Link value if available, display page id otherwise.
     */
    PageId getColourKeyLink(DecodedPage::Link link) const;

    /**
     * Refreshes decoded page data.
     *
     * @param page
     *      Page data.
     * @param header
     *      Header data.
     * @param mode
     *      Parser mode that defines which elements to refresh.
     */
    void refreshPageData(const PageDisplayable& page,
                         const PacketHeader& header,
                         const Parser::Mode mode);

    /**
     * Describes what to do with PES packet.
     */
    enum class PesAction
    {
        PROCESS, //!< PROCESS
        DROP,    //!< DROP
        WAIT     //!< WAIT
    };

    /**
     * Determines what to do with the packet. It can be processed, dropped or postponed depending on packet timing.
     *
     * @param header
     *      Packet header.
     * @return
     *      Action to be taken for packet - process, wait or drop.
     */
    PesAction getActionForPacket(const PesPacketHeader& header);

    /** Engine client interface. */
    EngineClient& m_client;

    /** Allocator. */
    std::unique_ptr<Allocator> m_allocator;

    /** Database. */
    std::unique_ptr<Database> m_database;

    /** Cache for decoded pages. */
    std::unique_ptr<Cache> m_cache;

    /** Buffer for PES packets. */
    std::unique_ptr<PesBuffer> m_pesBuffer;

    /** Metadata processor. */
    std::unique_ptr<MetadataProcessor> m_metadataProcessor;

    /** Teletext decoder. */
    std::unique_ptr<Decoder> m_decoder;

    /** Charset manager. */
    CharsetManager m_charsetManager;

    /** Teletext parser. */
    std::unique_ptr<Parser> m_parser;

    /** Last header suitable for clock update. */
    PacketHeader m_lastHeader;

    /** Page requested to be displayed (current). */
    PageId m_displayPage;

    /** Current page data. */
    const PageDisplayable* m_currentPageData;

    /**
     * Stale page data.
     *
     * Stale page is used when new page is not yet ready
     */
    const PageDisplayable* m_stalePageData;

    /**
     * Header page data.
     */
    DecodedPage m_pageData;

    /** Requested navigation mode. */
    NavigationMode m_navigationMode;

    /** Ignore PTS in decoded packets. */
    bool m_ignorePts;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_ENGINEIMPL_HPP_*/
