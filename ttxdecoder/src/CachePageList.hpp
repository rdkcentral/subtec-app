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


#ifndef TTXDECODER_CACHEPAGELIST_HPP_
#define TTXDECODER_CACHEPAGELIST_HPP_

#include <list>

#include "CachePage.hpp"

namespace ttxdecoder
{

/**
 * List of cache pages.
 */
class CachePageList
{
public:
    /**
     * Takes (gets & removes) one element from the list.
     *
     * @return
     *      Element if available, nullptr otherwise.
     */
    CachePage* takeOne()
    {
        if (m_list.empty())
        {
            return nullptr;
        }

        auto last = m_list.back();

        m_list.pop_back();

        return last;
    }

    /**
     * Appends element.
     *
     * @param page
     *      Element to add.
     */
    void append(CachePage* page)
    {
        m_list.push_back(page);
    }

private:
    /** Internal list of elements. */
    std::list<CachePage*> m_list;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CACHEPAGELIST_HPP_*/
