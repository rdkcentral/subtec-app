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


#include "EngineImpl.hpp"

#include <subttxrend/common/Logger.hpp>

#include "EngineClient.hpp"
#include "Allocator.hpp"
#include "PesBuffer.hpp"
#include "Decoder.hpp"
#include "Decoder.hpp"
#include "CacheImpl.hpp"
#include "MetadataProcessor.hpp"
#include "Database.hpp"
#include "Parser.hpp"
#include "ControlInfo.hpp"

namespace ttxdecoder
{

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "EngineImpl");

/**
 * Empty page (for decodding purposes).
 */
const PageDisplayable EMPTY_PAGE;

/**
 * Minimum acceptable timestamp difference.
 *
 * Also provides some time for decoding.
 *
 * Calculation: 75 ms * 45 (khz units).
 */
const std::uint32_t TIMESTAMP_DIFF_MIN_45KHZ = 75 * 45;

/**
 * Maximum acceptable timestamp difference.
 *
 * Calculation: 30 sec * 45 (khz units).
 */
const std::uint32_t TIMESTAMP_DIFF_MAX_45KHZ = 30 * 1000 * 45;

/**
 * Maximum acceptable timestamp difference for late packets
 *
 * Calculation: 500 ms * 45 (khz units).
 */
const std::uint32_t TIMESTAMP_DIFF_MAX_LATE_45KHZ = 500 * 45;

}

EngineImpl::EngineImpl(EngineClient& client,
                       std::unique_ptr<Allocator> allocator) :
        m_client(client),
        m_allocator(std::move(allocator)),
        m_charsetManager(),
        m_displayPage(),
        m_currentPageData(),
        m_stalePageData(),
        m_navigationMode(NavigationMode::DEFAULT),
        m_ignorePts(false)
{
    auto cacheBufferSize = m_allocator->getFreeSize() / 2;

    // maybe use allocator for this?
    m_database.reset(new Database());

    m_cache.reset(
            new CacheImpl(m_allocator->alloc(cacheBufferSize),
                    cacheBufferSize));

    auto pesBufferSize = m_allocator->getFreeSize();

    // use rest of the memory as the buffer for PES packets
    m_pesBuffer.reset(
            new PesBuffer(m_allocator->alloc(pesBufferSize), pesBufferSize));

    m_decoder.reset(new Decoder(*m_database.get(), *m_cache.get(), *this));

    m_parser.reset(
            new Parser(PresentationLevel::LEVEL_1, *m_database.get(),
                    m_charsetManager));

    // Explicit call to specific resetAcquisition() method from EngineImpl is a result of static code analyser warning
    // as described here: https://pvs-studio.com/en/blog/posts/cpp/0891/
    EngineImpl::resetAcquisition();
}

EngineImpl::~EngineImpl()
{
    // noop
}

void EngineImpl::resetAcquisition()
{
    g_logger.trace("%s", __func__);

    unsetCurrentPage(false);

    m_cache->clear();
    m_decoder->reset();
    m_pesBuffer->clear();
    m_database->reset();

    m_pageData.clear();

    m_lastHeader.setPageInfo(PageId(), 0, 0);
}

std::uint32_t EngineImpl::process()
{
    std::uint32_t packets = 0;

    g_logger.trace("%s", __func__);

    try
    {
        PesPacketHeader header;
        PesPacketReader dataReader;

        while (m_pesBuffer->getNextPacket(header, dataReader))
        {
            PesAction action = getActionForPacket(header);

            // not yet, waiting for right time
            if (action == PesAction::WAIT)
            {
                break;
            }

            if (action == PesAction::PROCESS)
            {
                m_decoder->processPacketData(dataReader);
            }

            // at this point packet was processed or should be dropped, consume in any case
            m_pesBuffer->markPacketConsumed(header);
            ++packets;
        }
    }
    catch (PesPacketReader::Exception& e)
    {
        g_logger.warning("%s - reset forced, reader error: %s", __func__,
                e.what());

        // handle issues silently
        m_pesBuffer->clear();
    }

    g_logger.trace("%s - complete", __func__);

    return packets;
}

bool EngineImpl::addPesPacket(const std::uint8_t* packet,
                              std::uint16_t length)
{
    return m_pesBuffer->addPesPacket(packet, length);
}

void EngineImpl::setCurrentPageId(const PageId& pageId)
{
    g_logger.info("%s - magazine=%04hX subpage=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    // no change - nothing to do
    if (m_displayPage == pageId)
    {
        return;
    }

    // store the request
    m_displayPage = pageId;

    // release the old page so it no longer be used
    unsetCurrentPage(true);

    // notify cache about the change
    m_cache->setCurrentPage(m_displayPage);

    // try to obtain the page from cache
    tryRestoreCurrentPage();
}

PageId EngineImpl::getNextPageId(const PageId& inputPageId) const
{
    return m_database->getNextPage(inputPageId, m_navigationMode);
}

PageId EngineImpl::getPrevPageId(const PageId& inputPageId) const
{
    return m_database->getPrevPage(inputPageId, m_navigationMode);
}

PageId EngineImpl::getPageId(PageIdType type) const
{
    switch (type)
    {
    case PageIdType::FLOF_INDEX_PAGE:
        return getColourKeyLink(DecodedPage::Link::FLOF_INDEX);
    case PageIdType::RED_KEY:
        return getColourKeyLink(DecodedPage::Link::RED);
    case PageIdType::GREEN_KEY:
        return getColourKeyLink(DecodedPage::Link::GREEN);
    case PageIdType::YELLOW_KEY:
        return getColourKeyLink(DecodedPage::Link::YELLOW);
    case PageIdType::CYAN_KEY:
        return getColourKeyLink(DecodedPage::Link::CYAN);
    case PageIdType::WHITE_KEY:
        // TODO: not supported yet
        break;

    case PageIdType::CURRENT_PAGE:
        return m_displayPage;

    case PageIdType::ACTUAL_SUBPAGE:
        if (m_currentPageData)
        {
            return m_currentPageData->getPageId();
        }
        return m_displayPage;
    case PageIdType::FIRST_SUBPAGE:
        return m_database->getFirstSubpage(m_displayPage);
    case PageIdType::NEXT_SUBPAGE:
        return m_database->getNextSubpage(m_displayPage);
    case PageIdType::PREV_SUBPAGE:
        return m_database->getPrevSubpage(m_displayPage);
    case PageIdType::HIGHEST_SUBPAGE:
        return m_database->getHighestSubpage(m_displayPage);
    case PageIdType::LAST_RECEIVED_SUBPAGE:
        return m_database->getLastReceivedSubpage(m_displayPage);

    case PageIdType::LAST_PAGE:
        if (m_currentPageData)
        {
            return m_currentPageData->getPageId();
        }
        else if (m_stalePageData)
        {
            return m_stalePageData->getPageId();
        }
        // not available
        break;

    case PageIdType::PRIOR_PAGE:
        // TODO: not supported yet
        break;

    case PageIdType::INDEX_PAGE_P830:
        return m_database->getIndexPageP830();

    default:
        // not supported
        break;
    }

    return PageId();
}

const DecodedPage& EngineImpl::getPage() const
{
    return m_pageData;
}

void EngineImpl::setNavigationMode(NavigationMode navigationMode)
{
    m_navigationMode = navigationMode;
}

NavigationState EngineImpl::getNavigationState() const
{
    // TODO: get from page instead of guessing

    if (m_pageData.getColourKeyLink(DecodedPage::Link::FLOF_INDEX).isValidDecimal())
    {
        return NavigationState::FLOF;
    }

    if (m_pageData.getColourKeyLink(DecodedPage::Link::RED).isValidDecimal())
    {
        return NavigationState::TOP;
    }

    return NavigationState::DEFAULT;
}

void EngineImpl::getTopLinkText(std::uint16_t magazine,
                                std::uint32_t linkType,
                                std::uint16_t* linkbuffer) const
{
    // TODO: not supported

    if (linkbuffer)
    {
        *linkbuffer = 0;
    }
}

std::uint8_t EngineImpl::getScreenColorIndex() const
{
    // TODO screen color

    return 0;
}

std::uint8_t EngineImpl::getRowColorIndex(std::uint8_t row) const
{
    // TODO row color

    return 0;
}

bool EngineImpl::getColors(std::array<std::uint32_t, 16>& colors) const
{
    static const uint32_t DEFAULT_EXTENDED_CLUT[16] =
    { 0xFFFF0055,    // CLUT2_0
            0xFFFF7700,    // CLUT2_1
            0xFF00FF77,    // CLUT2_2
            0xFFFFFFBB,    // CLUT2_3
            0xFF00CCAA,    // CLUT2_4
            0xFF550000,    // CLUT2_5
            0xFF665522,    // CLUT2_6
            0xFFCC7777,    // CLUT2_7

            0xFF333333,    // CLUT3_0
            0xFFFF7777,    // CLUT3_1
            0xFF77FF77,    // CLUT3_2
            0xFFFFFF77,    // CLUT3_3
            0xFF7777FF,    // CLUT3_4
            0xFFFF77FF,    // CLUT3_5
            0xFF77FFFF,    // CLUT3_6
            0xFFDDDDDD     // CLUT3_7
            };

    // no page specific colors currently supported, return defaults
    for (int i = 0; i < 16; ++i)
    {
        colors[i] = DEFAULT_EXTENDED_CLUT[i];
    }

    return true;
}

const CharsetMappingArray& EngineImpl::getCharsetMapping(Charset charset) const
{
    return m_charsetManager.getCharsetMapping(charset);
}

void EngineImpl::setCharsetMapping(Charset charset,
                                   const CharsetMappingArray& mappingArray)
{
    m_charsetManager.setCharsetMapping(charset, mappingArray);
}

void EngineImpl::setDefaultPrimaryNationalCharset(std::uint8_t index,
                                                  NationalCharset charset)
{
    m_charsetManager.setDefaultPrimaryNationalCharset(index, charset);
}

void EngineImpl::setDefaultSecondaryNationalCharset(std::uint8_t index,
                                                    NationalCharset charset)
{
    m_charsetManager.setDefaultSecondaryNationalCharset(index, charset);
}

uint8_t EngineImpl::getPageControlInfo() const
{
    return m_pageData.getPageControlInfo();
}

void EngineImpl::pageDecoded(const PageId& pageId)
{
    g_logger.trace("%s - magazine=%04hX subpage=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    // try to set the new page as current
    if (pageId.getMagazinePage() == m_displayPage.getMagazinePage())
    {
        tryRestoreCurrentPage();
    }
}

void EngineImpl::headerDecoded(const PacketHeader& header)
{
    g_logger.trace("%s - magazine=%04hX subpage=%04hX", __func__,
            header.getPageId().getMagazinePage(),
            header.getPageId().getSubpage());

    if (!header.getPageId().isValidDecimal())
    {
        // skip as it may contain non-textual data
        return;
    }

    auto controlInfo = header.getControlInfo();

    if (((controlInfo & ControlInfo::SUBTITLE) != 0)
            || ((controlInfo & ControlInfo::NEWSFLASH) != 0)
            || ((controlInfo & ControlInfo::SUPRESS_HEADER) != 0)
            || ((controlInfo & ControlInfo::INTERRUPTED_SEQUENCE) != 0))
    {
        // skip, the header may not be suitable
        return;
    }

    m_lastHeader = header;

    if (m_currentPageData)
    {
        refreshPageData(*m_currentPageData, header, Parser::Mode::CLOCK_ONLY);
    }
    else if (m_stalePageData)
    {
        refreshPageData(*m_stalePageData, header,
                Parser::Mode::HEADER_ONLY);
    }
    else
    {
        refreshPageData(EMPTY_PAGE, header, Parser::Mode::HEADER_ONLY);
    }
}

void EngineImpl::unsetCurrentPage(bool useAsStale)
{
    g_logger.trace("%s", __func__);

    if (useAsStale)
    {
        if (!m_currentPageData)
        {
            return;
        }

        if (m_stalePageData)
        {
            m_cache->releasePage(m_stalePageData);
            m_stalePageData = nullptr;
        }

        m_stalePageData = m_currentPageData;
        m_currentPageData = nullptr;
    }
    else
    {
        if (m_stalePageData)
        {
            m_cache->releasePage(m_stalePageData);
            m_stalePageData = nullptr;
        }

        if (m_currentPageData)
        {
            m_cache->releasePage(m_currentPageData);
            m_currentPageData = nullptr;
        }
    }
}

void EngineImpl::tryRestoreCurrentPage()
{
    const PageDisplayable* page = nullptr;

    if (m_displayPage.isAnySubpage())
    {
        page = m_cache->getNewestSubpage(m_displayPage);
    }
    else
    {
        page = m_cache->getPage(m_displayPage);

        // check if we do not have no subpages / subpages request mismatch
        if (!page)
        {
            page = m_cache->getNewestSubpage(m_displayPage);
            if (page)
            {
                if (m_displayPage.getSubpage() == 0)
                {
                    if (page->getPageId().getSubpage() != 0)
                    {
                        g_logger.info(
                                "%s - requested single page but multiple found",
                                __func__);
                    }
                    else
                    {
                        m_cache->releasePage(page);
                        page = nullptr;
                    }
                }
                else
                {
                    if (page->getPageId().getSubpage() == 0)
                    {
                        g_logger.info(
                                "%s - requested multiple page but single found",
                                __func__);
                    }
                    else
                    {
                        m_cache->releasePage(page);
                        page = nullptr;
                    }
                }
            }
        }
    }

    g_logger.trace("%s - magazine=%04hX subpage=%04hX => page=%p", __func__,
            m_displayPage.getMagazinePage(), m_displayPage.getSubpage(), page);

    if (page)
    {
        if (page != m_currentPageData || page->isVolatile())
        {
            g_logger.trace("%s - setting - magazine=%04hX subpage=%04hX",
                    __func__, page->getPageId().getMagazinePage(),
                    page->getPageId().getSubpage());

            unsetCurrentPage(false);
            m_currentPageData = page;

            refreshPageData(*m_currentPageData, *m_currentPageData->getHeader(),
                    Parser::Mode::FULL_PAGE);

            if (m_lastHeader.getPageId().isValidDecimal())
            {
                refreshPageData(*m_currentPageData, m_lastHeader,
                        Parser::Mode::CLOCK_ONLY);
            }
        }
        else
        {
            g_logger.trace("%s - skipping, not changed", __func__);

            m_cache->releasePage(page);
        }
    }
}

PageId EngineImpl::getColourKeyLink(DecodedPage::Link link) const
{
    return m_pageData.getColourKeyLink(link);
}

void EngineImpl::refreshPageData(const PageDisplayable& page,
                                 const PacketHeader& header,
                                 const Parser::Mode mode)
{
    if (!m_parser->parsePage(page, header, mode, m_navigationMode, m_pageData))
    {
        // nothing changed, skip
        return;
    }

    if (mode == Parser::Mode::FULL_PAGE)
    {
        PageId linkedPages[] =
        { m_pageData.getColourKeyLink(DecodedPage::Link::RED),
                m_pageData.getColourKeyLink(DecodedPage::Link::GREEN),
                m_pageData.getColourKeyLink(DecodedPage::Link::YELLOW),
                m_pageData.getColourKeyLink(DecodedPage::Link::CYAN),
                m_pageData.getColourKeyLink(DecodedPage::Link::FLOF_INDEX), };

        m_cache->setLinkedPages(linkedPages,
                sizeof(linkedPages) / sizeof(linkedPages[0]));

#if VERBOSE_LOGGING
        m_pageData.dump(true);
#endif /*VERBOSE_LOGGING*/
    }

    g_logger.trace("%s - notifying client", __func__);

    m_client.headerReady();
    if (mode == Parser::Mode::FULL_PAGE)
    {
        m_client.pageReady();
    }
}

EngineImpl::PesAction EngineImpl::getActionForPacket(const PesPacketHeader& header)
{
    PesAction result = PesAction::DROP;

    if (header.isTeletextPacket())
    {
        if (header.m_hasPts)
        {
            g_logger.trace("%s - pts=%u", __func__, header.m_pts);
            if (m_ignorePts)
            {
                g_logger.trace("%s - pts ignored - accepted for processing", __func__);
                return PesAction::PROCESS;
            }

            std::uint32_t stc = 0;

            if (m_client.getStc(stc))
            {
                const std::uint32_t pts = header.m_pts;
                const std::uint32_t diff = pts - stc;

                g_logger.trace("%s - PTS = %u : STC = %u : DIFF = %u", __func__, pts, stc, diff);

                if (diff <= TIMESTAMP_DIFF_MIN_45KHZ)
                {
                    g_logger.trace("%s - accepted for processing", __func__);
                    result = PesAction::PROCESS;
                }
                else if (diff <= TIMESTAMP_DIFF_MAX_45KHZ)
                {
                    g_logger.trace("%s - synchronizing - waiting ", __func__);
                    result = PesAction::WAIT;
                }
                else
                {
                    if (stc > pts)
                    {
                        const std::uint32_t late = stc - pts;
                        g_logger.warning("%s - late packet (%u)", __func__, late);
                        if (late <= TIMESTAMP_DIFF_MAX_LATE_45KHZ)
                        {
                            g_logger.trace("%s - accepted for processing", __func__);
                            result = PesAction::PROCESS;
                        }
                        else
                        {
                            g_logger.warning("%s - dropping late packet, time diff too big (%u)", __func__, late);
                            result = PesAction::DROP;
                        }
                    }
                    else
                    {
                        g_logger.warning("%s - dropping packet, time diff too big (%u)", __func__, diff);
                        result = PesAction::DROP;
                    }
                }
            }
        }
        else
        {
            g_logger.trace("%s - 'no pts' packet accepted for processing", __func__);
            result = PesAction::PROCESS;
        }
    }
    return result;
}

void EngineImpl::setIgnorePts(bool ignorePts)
{
    g_logger.trace("%s ignorePts=%d", __func__, ignorePts);
    m_ignorePts = ignorePts;
}

} // namespace ttxdecoder
