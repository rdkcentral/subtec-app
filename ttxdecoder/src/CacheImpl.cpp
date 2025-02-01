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


#include "CacheImpl.hpp"

#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <map>
#include <list>
#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "CacheImpl");

} // namespace <anonymous>

namespace ttxdecoder
{

CacheImpl::CacheImpl(std::uint8_t* buffer,
                     std::size_t bufferSize) :
        m_currentPageId(),
        m_linkedPageIdsCount(0),
        m_clearPagesInUse(),
        m_exactMatchPage(),
        m_cachedRange(0)
{
    std::size_t pagesNeeded = 0;

    // exact match
    pagesNeeded += 1;

    // current
    pagesNeeded += 1;

    // links
    pagesNeeded += LINK_PAGES_COUNT;

    // locked
    pagesNeeded += IN_USE_PAGES_MARGIN;

    // free pages margin
    pagesNeeded += FREE_PAGES_MARGIN;

    // check if we have enough memory
    if (pagesNeeded * sizeof(CachePage) > bufferSize)
    {
        throw std::invalid_argument("Given buffer it too small");
    }

    // prepare space for cache range around current page
    while (m_cachedRange < MAX_CACHED_PAGES_RANGE)
    {
        if ((pagesNeeded + 2) * sizeof(CachePage) > bufferSize)
        {
            break;
        }

        m_cachedRange++;
        pagesNeeded += 2; // 1 backward, 1 forward
    }

    // create pages
    for (std::size_t i = 0; i < pagesNeeded; ++i)
    {
        CachePage* page = new (buffer) CachePage();
        m_freePages.append(page);

        buffer += sizeof(CachePage);
    }
}

CacheImpl::~CacheImpl()
{
    // noop
}

void CacheImpl::setCurrentPage(PageId pageId)
{
    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    // if same - nothing to do
    if (pageId == m_currentPageId)
    {
        return;
    }

    // store
    m_currentPageId = pageId;

    // if there is exact match - release it
    if (m_exactMatchPage)
    {
        releaseInternal(m_exactMatchPage);
        m_exactMatchPage = nullptr;
    }

    // release cached pages if not needed anymore
    refreshCachedPages();

    // try to restore exact match
    CachePage* cachePage = m_cachedPages.getPage(pageId.getMagazinePage());
    if (cachePage)
    {
        if (cachePage->getPageId() == m_currentPageId)
        {
            g_logger.trace("%s - setting exact match page", __func__);

            m_exactMatchPage = markUsed(cachePage);
        }
    }
}

void CacheImpl::setLinkedPages(const PageId* pageIds,
                               std::size_t count)
{
    if (!pageIds)
    {
        return;
    }

    m_linkedPageIdsCount = 0;
    for (std::size_t i = 0;
            (i < count) && (m_linkedPageIdsCount < LINK_PAGES_COUNT); ++i)
    {
        if (pageIds[i].isValidDecimal())
        {
            m_linkedPageIds[m_linkedPageIdsCount++] = pageIds[i];
        }
    }

    refreshCachedPages();
}

void CacheImpl::clear()
{
    g_logger.trace("%s", __func__);

    // release exact match
    if (m_exactMatchPage)
    {
        releaseInternal(m_exactMatchPage);
        m_exactMatchPage = nullptr;
    }

    // release cached pages
    m_cachedPages.iterate([this](CachePage* cachePage) -> bool
    {
        releaseInternal(cachePage);
        return false;
    });

    // remove links
    m_linkedPageIdsCount = 0;
}

bool CacheImpl::isPageNeeded(PageId pageId) const
{
    // equals currently requested page
    if (pageId.getMagazinePage() == m_currentPageId.getMagazinePage())
    {
        return true;
    }

    // if current page is valid decimal - build cache range around it
    // queried page must also be a valid decimal
    if (m_currentPageId.isValidDecimal() && pageId.isValidDecimal())
    {
        const std::int32_t currentMagazinePage =
                m_currentPageId.getDecimalMagazinePage();
        const std::int32_t rangeLow = currentMagazinePage - m_cachedRange;
        const std::int32_t rangeHigh = currentMagazinePage + m_cachedRange;

        // check cache range
        const std::int32_t magazinePage = pageId.getDecimalMagazinePage();
        if ((rangeLow <= magazinePage) && (magazinePage <= rangeHigh))
        {
            return true;
        }

        const std::int32_t magazinePageLow = magazinePage - 800;
        if ((rangeLow <= magazinePageLow) && (magazinePageLow <= rangeHigh))
        {
            return true;
        }

        const std::int32_t magazinePageHigh = magazinePage + 800;
        if ((rangeLow <= magazinePageHigh) && (magazinePageHigh <= rangeHigh))
        {
            return true;
        }
    }

    // links
    auto currentPage = m_exactMatchPage;
    if (!currentPage)
    {
        currentPage = m_cachedPages.getPage(m_currentPageId.getMagazinePage());
    }
    if (currentPage)
    {
        const std::int32_t magazinePage = pageId.getMagazinePage();

        for (std::size_t i = 0; i < m_linkedPageIdsCount; ++i)
        {
            if (m_linkedPageIds[i].getMagazinePage() == magazinePage)
            {
                return true;
            }
        }
    }

    return false;
}

const PageDisplayable* CacheImpl::getPage(PageId pageId)
{
    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    return getPageInternal(pageId);
}

PageDisplayable* CacheImpl::getMutablePage(PageId pageId)
{
    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    CachePage* resultPage = getPageInternal(pageId);
    if (resultPage)
    {
        m_clearPagesInUse.insert(resultPage);
    }
    return resultPage;
}

const PageDisplayable* CacheImpl::getNewestSubpage(PageId pageId)
{
    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    CachePage* cachePage = m_cachedPages.getPage(pageId.getMagazinePage());
    if (cachePage)
    {
        g_logger.trace("%s - found cached page", __func__);

        return markUsed(cachePage);
    }

    return nullptr;
}

PageDisplayable* CacheImpl::getClearPage()
{
    g_logger.debug("%s - pages in use %zu", __func__, m_clearPagesInUse.size());

    // try to get free page
    auto freePage = m_freePages.takeOne();
    if (!freePage)
    {
        g_logger.warning("%s - no free pages available", __func__);
        return nullptr;
    }

    m_clearPagesInUse.insert(freePage);

    freePage->invalidate();

    g_logger.trace("%s - returning clear page %p", __func__, freePage);

    return markUsed(freePage);
}

void CacheImpl::insertPage(PageDisplayable* newPage)
{
    CachePage* cachePage = static_cast<CachePage*>(newPage);

    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            cachePage->getPageId().getMagazinePage(),
            cachePage->getPageId().getSubpage());

    auto eraseResult = m_clearPagesInUse.erase(cachePage);
    if (eraseResult == 0)
    {
        throw std::invalid_argument(
                "Given page is not one of clear pages returned");
    }

    if (newPage->isValid() && isPageNeeded(cachePage->getPageId()))
    {
        // process exact match first
        if (cachePage->getPageId() == m_currentPageId)
        {
            g_logger.trace("%s - setting exact match page", __func__);

            if (m_exactMatchPage)
            {
                releaseInternal(m_exactMatchPage);
                m_exactMatchPage = nullptr;
            }

            m_exactMatchPage = markUsed(cachePage);
        }

        // processes subpages
        CachePage* oldSubpage = m_cachedPages.insert(markUsed(cachePage));
        if (oldSubpage)
        {
            releaseInternal(oldSubpage);
        }
    }
    else
    {
        g_logger.trace("%s - not needed, skipping", __func__);
    }

    // release the reference from getClearPage()
    // (if page was not put into cache it would go to free pages collection)
    releaseInternal(cachePage);
}

void CacheImpl::releasePage(const PageDisplayable* page)
{
    // const cast so user could release pages returned via
    // getPage()/getNewestSubpage()
    CachePage* cachePage =
            static_cast<CachePage*>(const_cast<PageDisplayable*>(page));

    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            cachePage->getPageId().getMagazinePage(),
            cachePage->getPageId().getSubpage());

    releaseInternal(cachePage);
}

void CacheImpl::refreshCachedPages()
{
    auto newMagazinePage = m_currentPageId.getMagazinePage();
    m_cachedPages.iterate([this,newMagazinePage](CachePage* cachePage) -> bool
    {
        if (isPageNeeded(cachePage->getPageId()))
        {
            // leave it as-is
            return true;
        }

        // remove
        releaseInternal(cachePage);
        return false;
    });
}

void CacheImpl::releaseInternal(CachePage* cachePage)
{
    if (cachePage->decUseCount())
    {
        m_clearPagesInUse.erase(cachePage);
        m_freePages.append(cachePage);
    }
}

CachePage* CacheImpl::markUsed(CachePage* cachePage)
{
    cachePage->incUseCount();
    return cachePage;
}

CachePage* CacheImpl::getPageInternal(PageId pageId)
{
    g_logger.trace("%s - mag=%04hX sub=%04hX", __func__,
            pageId.getMagazinePage(), pageId.getSubpage());

    if (m_exactMatchPage)
    {
        if (m_exactMatchPage->getPageId() == pageId)
        {
            g_logger.trace("%s - found exact match", __func__);

            return markUsed(m_exactMatchPage);
        }
    }

    CachePage* cachePage = m_cachedPages.getPage(pageId.getMagazinePage());
    if (cachePage)
    {
        if (cachePage->getPageId() == pageId)
        {
            g_logger.trace("%s - found cached page", __func__);

            return markUsed(cachePage);
        }
    }

    return nullptr;
}

} // namespace ttxdecoder
