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


#ifndef TTXDECODER_CACHEIMPL_HPP_
#define TTXDECODER_CACHEIMPL_HPP_

#include <set>

#include "CachePageList.hpp"
#include "CachePageMap.hpp"

#include "Cache.hpp"
#include "PageDisplayable.hpp"

namespace ttxdecoder
{

/**
 * Cache implementation.
 */
class CacheImpl : public Cache
{
public:
    /**
     * Constructor.
     *
     * @param buffer
     *      Memory for cached data.
     * @param bufferSize
     *      Size of the buffer in bytes.
     */
    CacheImpl(std::uint8_t* buffer,
              std::size_t bufferSize);

    /**
     * Destructor.
     */
    virtual ~CacheImpl();

    /** @copydoc Cache::setCurrentPage */
    virtual void setCurrentPage(PageId pageId) override;

    /** @copydoc Cache::setLinkedPages */
    virtual void setLinkedPages(const PageId* pageIds,
                                std::size_t count) override;

    /** @copydoc Cache::clear */
    virtual void clear() override;

    /** @copydoc Cache::isPageNeeded */
    virtual bool isPageNeeded(PageId pageId) const override;

    /** @copydoc Cache::getPage */
    virtual const PageDisplayable* getPage(PageId pageId) override;

    /** @copydoc Cache::getMutablePage */
    virtual PageDisplayable* getMutablePage(PageId pageId) override;

    /** @copydoc Cache::getNewestSubpage */
    virtual const PageDisplayable* getNewestSubpage(PageId pageId) override;

    /** @copydoc Cache::getClearPage */
    virtual PageDisplayable* getClearPage() override;

    /** @copydoc Cache::insertPage */
    virtual void insertPage(PageDisplayable* page) override;

    /** @copydoc Cache::releasePage */
    virtual void releasePage(const PageDisplayable* page) override;

private:
    /** Maximum range of the cached pages. */
    static const std::int32_t MAX_CACHED_PAGES_RANGE = 50;

    /** Cache margin for (colour) linked pages. */
    static const std::size_t LINK_PAGES_COUNT = 4;

    /** Cache margin for pages 'in use'. */
    static const std::size_t IN_USE_PAGES_MARGIN = 3;

    /** Cache margin for free pages. */
    static const std::size_t FREE_PAGES_MARGIN = 2;

    /**
     * Refresh cached pages.
     *
     * Performs a lookup of cached pages and releases these
     * which are no longer needed.
     */
    void refreshCachedPages();

    /**
     * Internal page release implementation.
     *
     * @param cachePage
     *      Page to be released.
     */
    void releaseInternal(CachePage* cachePage);

    /**
     * Marks page as used.
     *
     * @param cachePage
     *      Page to be released.
     *
     * @return
     *      The given page.
     */
    CachePage* markUsed(CachePage* cachePage);

    /**
     * Helper function to retrieve a page.
     *
     * @param pageId
     *      Page to retrieve.
     *
     * @return
     *      Requested page or nullptr if not found.
     */
    CachePage* getPageInternal(PageId pageId);

    /** Current page id. */
    PageId m_currentPageId;

    /** Used linked pages count. */
    std::size_t m_linkedPageIdsCount;

    /** Linked pages. */
    PageId m_linkedPageIds[LINK_PAGES_COUNT];

    /** Set of clear pages returned. */
    std::set<CachePage*> m_clearPagesInUse;

    /** Page that exactly matches current page id. */
    CachePage* m_exactMatchPage;

    /** Collection of cached pages. */
    CachePageMap m_cachedPages;

    /** Collection of free pages. */
    CachePageList m_freePages;

    /** Range of the cached pages. */
    std::int32_t m_cachedRange;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CACHEIMPL_HPP_*/
