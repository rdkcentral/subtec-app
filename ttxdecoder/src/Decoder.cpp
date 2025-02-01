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


#include <algorithm>

#include "Decoder.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ControlInfo.hpp"
#include "PesPacketReader.hpp"
#include "Property.hpp"
#include "CharacterMarker.hpp"
#include "CharsetMapping.hpp"
#include "DecoderListener.hpp"
#include "Database.hpp"
#include "PageDisplayable.hpp"
#include "Cache.hpp"

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "Decoder");

} // namespace <anonymous>

namespace ttxdecoder
{

Decoder::Decoder(Database& database,
                 Cache& cache,
                 DecoderListener& listener) :
        m_database(database),
        m_cache(cache),
        m_listener(listener),
        m_collector(*this),
        m_currentPages(),
        m_mode(Mode::SERIAL),
        m_metadataProcessor(database)
{
    // noop
}

Decoder::~Decoder()
{
    // noop
}

void Decoder::reset()
{
    m_collector.reset();
    m_metadataProcessor.reset();
}

void Decoder::processPacketData(PesPacketReader& reader)
{
    m_collector.processPacketData(reader);
}

void Decoder::onPacketReady(CollectorPacketContext& context)
{
    g_logger.trace("%s", __func__);

    Packet* packet = preparePacket(context);
    if (packet)
    {
        if (context.consume(*packet))
        {
            processPacket(*packet);
        }
        else
        {
            g_logger.debug("%s - packet collection failed", __func__);
        }
    }
    else
    {
        g_logger.trace("%s - skipping packet", __func__);
    }
}

Packet* Decoder::preparePacket(const CollectorPacketContext& context)
{
    auto magazineNumber = context.getMagazineNumber();
    auto packetAddress = context.getPacketAddress();
    auto designationCode = context.getDesignationCode();

    g_logger.trace("%s - magazine=%d packet=%d designation=%d mode=%d page=%p", __func__,
            magazineNumber, packetAddress, designationCode, static_cast<int>(m_mode), m_currentPages[magazineNumber].page);

    // new header, must always be processed
    if (packetAddress == 0)
    {
        return &m_headerPacketBuffer;
    }

    auto& currentPage = m_currentPages[magazineNumber].page;

    // ask current page if packet is needed
    if (currentPage)
    {
        // check magazine number
        auto pageHeader = currentPage->getHeader();
        if (pageHeader)
        {
            if (pageHeader->getMagazineNumber() == magazineNumber)
            {
                return currentPage->takePacket(packetAddress, designationCode);
            }
            else
            {
                g_logger.trace("%s - magazine mismatch, skipping", __func__);
            }
        }
        else
        {
            g_logger.warning("%s - current page misses header", __func__);
        }
    }

    // metadata processing
    return m_metadataProcessor.getPacketBuffer(magazineNumber, packetAddress,
            designationCode);
}

void Decoder::processPacket(const Packet& collectedPacket)
{
    g_logger.trace("%s - type=%d magazine=%d packet=%d", __func__,
            static_cast<int>(collectedPacket.getType()),
            collectedPacket.getMagazineNumber(),
            collectedPacket.getPacketAddress());

    auto& currentPage = m_currentPages[collectedPacket.getMagazineNumber()].page;

    if (collectedPacket.getPacketAddress() == 0)
    {
        if (collectedPacket.getType() != PacketType::HEADER)
        {
            g_logger.error("%s - Unexpected packet type: %d.", __func__,
                    static_cast<int>(collectedPacket.getType()));
            return;
        }

        auto collectedHeader = static_cast<const PacketHeader&>(collectedPacket);

        m_mode = (collectedHeader.getControlInfo() & ControlInfo::MAGAZINE_SERIAL) ? Mode::SERIAL : Mode::PARALLEL;

        processCurrentPage(collectedHeader);
        setCurrentPage(collectedHeader);

        if (currentPage)
        {
            setCurrentPageHeader(collectedHeader);
        }

        m_listener.headerDecoded(collectedHeader);
    }
    else if (currentPage)
    {
        currentPage->setLastPacketValid(&collectedPacket);
    }

    m_metadataProcessor.processPacket(collectedPacket);
}

void Decoder::processCurrentPage(const PacketHeader& newHeader)
{
    auto magazineNumber = newHeader.getMagazineNumber();

    if (m_mode == Mode::SERIAL)
    {
        auto it = std::find_if(m_currentPages.begin(), m_currentPages.end(), [](const PageInfo& pi)
                               {return (pi.page != nullptr);});

        if (it != m_currentPages.end())
        {
            auto& currentPageInfo = *it;

            auto headerMgzPage = newHeader.getPageId().getMagazinePage();
            auto currentMgzPage = currentPageInfo.page->getPageId().getMagazinePage();

            g_logger.trace("%s - serial mode, headerMgPage=%d currentMgzPage=%d",
                __func__,
                headerMgzPage,
                currentMgzPage);
            if (headerMgzPage != currentMgzPage)
            {
                processPageInfo(currentPageInfo);
            }
        }
    }
    else
    {
        auto currentPageInfo = m_currentPages[magazineNumber];
        if (currentPageInfo.page)
        {
            auto headerPage = newHeader.getPageId().getPage();
            auto currentPage = currentPageInfo.page->getPageId().getPage();

            g_logger.trace("%s - parallel mode, headerPage=%d current=%d", __func__, headerPage, currentPage);
            if (headerPage != currentPage)
            {
                processPageInfo(m_currentPages[magazineNumber]);
            }
        }
    }
}

void Decoder::setCurrentPage(const PacketHeader& collectedHeader)
{
    g_logger.trace("%s - page: %04X:%04X", __func__,
            collectedHeader.getPageId().getMagazinePage(),
            collectedHeader.getPageId().getSubpage());

    auto& currentPageInfo = m_currentPages[collectedHeader.getMagazineNumber()];

    if (currentPageInfo.page)
    {
        g_logger.info("%s - page %d partially collected but new header received",
            __func__,
            collectedHeader.getPageId().getMagazinePage());

        if (currentPageInfo.fromCache)
        {
            auto page = static_cast<PageDisplayable*>(currentPageInfo.page);
            m_cache.releasePage(page);
        }
        // otherwise static buffers are used - no need to release
        currentPageInfo.reset();
    }

    if (m_cache.isPageNeeded(collectedHeader.getPageId()))
    {
        if ((collectedHeader.getControlInfo() & ControlInfo::ERASE_PAGE) == 0)
        {
            g_logger.trace("%s - no erase flag, looking for a page in cache", __func__);
            currentPageInfo.page = m_cache.getMutablePage(collectedHeader.getPageId());
        }

        if (! currentPageInfo.page)
        {
            g_logger.trace("%s - page not found in cache, using clear page", __func__);
            currentPageInfo.page = m_cache.getClearPage();
        }

        if (currentPageInfo.page)
        {
            currentPageInfo.fromCache = true;
            g_logger.trace("%s - got page from cache for mgz=%d", __func__, collectedHeader.getMagazineNumber());
        }
        else
        {
            g_logger.info("%s - could not get page from cache", __func__);
        }
    }
    else
    {
        g_logger.trace("%s - page %d not needed", __func__, collectedHeader.getPageId().getMagazinePage());
    }

    if (!currentPageInfo.page)
    {
        currentPageInfo.page = m_metadataProcessor.getPageBuffer(
                collectedHeader.getPageId());
        currentPageInfo.fromCache = false;
    }
}

void Decoder::setCurrentPageHeader(const PacketHeader& collectedHeader)
{
    g_logger.trace("%s - page: %04X:%04X", __func__,
            collectedHeader.getPageId().getMagazinePage(),
            collectedHeader.getPageId().getSubpage());

    auto& currentPage = m_currentPages[collectedHeader.getMagazineNumber()].page;

    if (!currentPage)
    {
        g_logger.fatal("%s - Current page is not set", __func__);
        return;
    }

    // set header
    auto pagePacket = currentPage->takeHeader();
    if (!pagePacket)
    {
        g_logger.fatal("%s - Cannot get page header.", __func__);
        return;
    }

    if (pagePacket->getType() != PacketType::HEADER)
    {
        g_logger.fatal("%s - Invalid page header type: %d.", __func__,
                static_cast<int>(pagePacket->getType()));
        return;
    }

    auto pageHeader = static_cast<PacketHeader*>(pagePacket);

    *pageHeader = collectedHeader;

    currentPage->setLastPacketValid(pagePacket);
}

void Decoder::processPageInfo(PageInfo& pageInfo)
{
    if (!pageInfo.page)
    {
        return;
    }
    else
    {
        if (pageInfo.page->getHeader())
        {
            g_logger.trace("%s - page=%04X:%04X", __func__,
                    pageInfo.page->getHeader()->getPageId().getMagazinePage(),
                    pageInfo.page->getHeader()->getPageId().getSubpage());
        }
        else
        {
            g_logger.trace("%s - page=meta", __func__);
        }
    }

    const PageId pageId = pageInfo.page->getPageId();
    const bool pageValid = pageInfo.page->isValid();

    g_logger.trace("%s - page=%04d:%04d valid=%d", __func__,
            pageId.getMagazinePage(), pageId.getSubpage(), pageValid);

    // process metadata for all valid pages
    if (pageValid)
    {
        m_metadataProcessor.processPage(*pageInfo.page);
    }

    // put page to cache or release it
    if ((pageInfo.page->getType() == PageType::DISPLAYABLE) &&
            pageInfo.fromCache)
    {
        auto page = static_cast<PageDisplayable*>(pageInfo.page);

        if (pageValid && page->getHeader())
        {
            m_cache.insertPage(page);
        }
        else
        {
            m_cache.releasePage(page);
        }
    }

    pageInfo.reset();

    // if valid page was decoded notify listener
    if (pageValid)
    {
        m_listener.pageDecoded(pageId);
    }
}

} // namespace ttxdecoder
