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


#ifndef TTXDECODER_CACHEPAGEMAP_HPP_
#define TTXDECODER_CACHEPAGEMAP_HPP_

#include <cstdint>
#include <map>

#include "CachePage.hpp"

namespace ttxdecoder
{

/**
 * Map of cache pages.
 *
 * Provides mapping from magazine page number to cache page.
 */
class CachePageMap
{
public:
    /**
     * Iterates over collection elements.
     *
     * @param f
     *      Functor that takes cached page pointer and returns
     *      false when page shall be removed from the collection.
     */
    template<class Functor>
    void iterate(Functor f)
    {
        for (auto iter = m_pageMap.begin(); iter != m_pageMap.end();)
        {
            if (f(iter->second))
            {
                ++iter;
            }
            else
            {
                iter = m_pageMap.erase(iter);
            }
        }
    }

    /**
     * Returns page matching given magazine page number.
     *
     * @param magazinePage
     *      Magazine page number.
     *
     * @return
     *      Page if found or nullptr otherwise.
     */
    CachePage* getPage(uint16_t magazinePage) const
    {
        auto iter = m_pageMap.find(magazinePage);
        if (iter != m_pageMap.end())
        {
            return iter->second;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Inserts page into cache.
     *
     * @param page
     *      Page to insert.
     *
     * @return
     *      The replaced page or nullptr if there was none.
     */
    CachePage* insert(CachePage * page)
    {
        auto ret = m_pageMap.insert(
                std::make_pair(page->getPageId().getMagazinePage(), page));
        if (ret.second)
        {
            // new element inserted
            return nullptr;
        }
        else
        {
            std::swap(ret.first->second, page);

            return page;
        }
    }

private:
    /** Internal collection. */
    std::map<uint16_t, CachePage*> m_pageMap;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CACHEPAGEMAP_HPP_*/
