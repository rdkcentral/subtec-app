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


#ifndef TTXDECODER_CACHE_HPP_
#define TTXDECODER_CACHE_HPP_

#include <subttxrend/common/NonCopyable.hpp>

#include "PageId.hpp"

namespace ttxdecoder
{

class PageDisplayable;

/**
 * Cache.
 */
class Cache : private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     */
    Cache() = default;

    /**
     * Destructor.
     */
    virtual ~Cache() = default;

    /**
     * Sets current page.
     *
     * @note This method is used to provide cache with hint which
     *       pages shall be cached.
     *
     * @param pageId
     *      Id of the current page.
     */
    virtual void setCurrentPage(PageId pageId) = 0;

    /**
     * Sets linked pages.
     *
     * @note This method is used to provide cache with hint which
     *       pages shall be cached.
     *
     * @note There is a limit of linked pages that could be added.
     *
     * @param pageIds
     *      Linked pages ids.
     * @param count
     *      Number of pages in array.
     */
    virtual void setLinkedPages(const PageId* pageIds,
                                std::size_t count) = 0;

    /**
     * Removes everything from cache.
     */
    virtual void clear() = 0;

    /**
     * Checks if page with given id is needed and should be decoded.
     *
     * @param pageId
     *      If od the page.
     *
     * @retval true
     *      Page should be decoded.
     * @retval false
     *      Page should be skipped.
     */
    virtual bool isPageNeeded(PageId pageId) const = 0;

    /**
     * Returns page with matching id.
     *
     * @param pageId
     *      Id of the page to get.
     *
     * @note The returned page must be released with releasePage() when
     *       no longer needed.
     *
     * @return
     *      Page if found, null otherwise.
     */
    virtual const PageDisplayable* getPage(PageId pageId) = 0;


    /**
     * Returns existing page for update.
     *
     * @param pageId
     *      Id of the page to get.
     *
     * @note The returned page must be released with releasePage() when
     *       no longer needed.
     *
     * @return
     *      Page if found, null otherwise.
     */
    virtual PageDisplayable* getMutablePage(PageId pageId) = 0;

    /**
     * Returns newest subpage of given page.
     *
     * @param pageId
     *      Id of the page to get.
     *      Only magazine page number is used.
     *
     * @note The returned page must be released with releasePage() when
     *       no longer needed.
     *
     * @return
     *      Page if found, null otherwise.
     */
    virtual const PageDisplayable* getNewestSubpage(PageId pageId) = 0;

    /**
     * Gets clear page that could be used by decoder.
     *
     * @return
     *      Page if available, null otherwise.
     */
    virtual PageDisplayable* getClearPage() = 0;

    /**
     * Inserts page into cache.
     *
     * @param page
     *      Page previously returned by getClearPage().
     */
    virtual void insertPage(PageDisplayable* page) = 0;

    /**
     * Releases the page so it could be removed from cache.
     *
     * @param page
     *      Page to unlock.
     */
    virtual void releasePage(const PageDisplayable* page) = 0;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CACHE_HPP_*/
