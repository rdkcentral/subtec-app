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


#ifndef TTXDECODER_CACHEPAGE_HPP_
#define TTXDECODER_CACHEPAGE_HPP_

#include "PageDisplayable.hpp"

#include <cassert>

namespace ttxdecoder
{

/**
 * Cached page.
 */
class CachePage : public PageDisplayable
{
public:
    /**
     * Constructor.
     */
    CachePage() :
            m_lockCount(0)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~CachePage() = default;

    /**
     * Increments usage counter.
     */
    void incUseCount()
    {
        ++m_lockCount;
    }

    /**
     * Decrements usage counter.
     *
     * @retval true
     *      Page is no longer used.
     * @retval false
     *      Page is still used.
     */
    bool decUseCount()
    {
        assert(m_lockCount > 0);

        --m_lockCount;

        return m_lockCount == 0;
    }

private:
    /** Page lock count. */
    std::uint32_t m_lockCount;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_CACHEPAGE_HPP_*/
