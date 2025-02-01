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


#include "Database.hpp"

#include <subttxrend/common/Logger.hpp>

namespace ttxdecoder
{

namespace
{

subttxrend::common::Logger g_logger("TtxDecoder", "Database");

} // namespace <anonymous>

Database::Database()
{
    reset();
}

void Database::reset()
{
    m_indexPage830 = PageId();

    for (std::size_t i = 0; i < MAGAZINE_COUNT; ++i)
    {
        m_magazinePages[i].invalidate();
    }

    resetTopMetadata();
}

void Database::resetTopMetadata()
{
    for (std::size_t i = 0; i < PAGE_COUNT; ++i)
    {
        m_metadata[i].m_topMetadata.reset();
    }
}

const PageMagazine& Database::getMagazinePage(std::uint8_t magazine) const
{
    if (magazine < MAGAZINE_COUNT)
    {
        return m_magazinePages[magazine];
    }
    else
    {
        return m_magazinePages[1];
    }
}

PageMagazine& Database::getMagazinePage(std::uint8_t magazine)
{
    if (magazine < MAGAZINE_COUNT)
    {
        return m_magazinePages[magazine];
    }
    else
    {
        return m_magazinePages[1];
    }
}

PageId Database::getNextPage(const PageId& inputPage,
                             NavigationMode navigationMode) const
{
    // TODO getNextPage - navigation modes

    return calculateNextPage(inputPage);
}

PageId Database::getPrevPage(const PageId& inputPage,
                             NavigationMode navigationMode) const
{
    // TODO getPrevPage - navigation modes

    return calculatePrevPage(inputPage);
}

void Database::setIndexPageP830(const PageId& pageId)
{
    m_indexPage830 = pageId;
}

PageId Database::getIndexPageP830() const
{
    return m_indexPage830;
}

PageId Database::getFirstSubpage(const PageId& page) const
{
    // TODO: getFirstSubpage
    return PageId();
}

PageId Database::getNextSubpage(const PageId& page) const
{
    // TODO: getNextSubpage
    return PageId();
}

PageId Database::getPrevSubpage(const PageId& page) const
{
    // TODO: getPrevSubpage
    return PageId();
}

PageId Database::getHighestSubpage(const PageId& page) const
{
    // TODO: getHighestSubpage
    return PageId();
}

PageId Database::getLastReceivedSubpage(const PageId& page) const
{
    // TODO: getLastReceivedSubpage
    return PageId();
}

Database::TopMetadata& Database::getTopMetatadata(const uint16_t magazinePage)
{
    // invalid index protection
    if ((magazinePage >= 0x100) && (magazinePage <= 0x8FF))
    {
        return m_metadata[magazinePage - 0x100].m_topMetadata;
    }

    return m_metadata[0x00FF].m_topMetadata;
}

const Database::TopMetadata& Database::getTopMetatadata(const uint16_t magazinePage) const
{
    // invalid index protection
    if ((magazinePage >= 0x100) && (magazinePage <= 0x8FF))
    {
        return m_metadata[magazinePage - 0x100].m_topMetadata;
    }

    return m_metadata[0x00FF].m_topMetadata;
}

PageId Database::calculateNextPage(const PageId& inputPage) const
{
    if (inputPage.isValidDecimal())
    {
        uint16_t d2 = (inputPage.getMagazinePage() >> 8) & 0xF;
        uint16_t d1 = (inputPage.getMagazinePage() >> 4) & 0xF;
        uint16_t d0 = (inputPage.getMagazinePage() >> 0) & 0xF;

        if (++d0 > 9)
        {
            d0 = 0;

            if (++d1 > 9)
            {
                d1 = 0;

                if (++d2 > 8)
                {
                    d2 = 1;
                }
            }
        }

        uint16_t newMagPage = (d2 << 8) | (d1 << 4) | d0;

        return PageId(newMagPage, PageId::ANY_SUBPAGE);
    }
    else
    {
        return PageId();
    }
}

PageId Database::calculatePrevPage(const PageId& inputPage) const
{
    if (inputPage.isValidDecimal())
    {
        uint16_t d2 = (inputPage.getMagazinePage() >> 8) & 0xF;
        uint16_t d1 = (inputPage.getMagazinePage() >> 4) & 0xF;
        uint16_t d0 = (inputPage.getMagazinePage() >> 0) & 0xF;

        if (d0 > 0)
        {
            --d0;
        }
        else
        {
            d0 = 9;

            if (d1 > 0)
            {
                --d1;
            }
            else
            {
                d1 = 9;

                if (d2 > 1)
                {
                    --d2;
                }
                else
                {
                    d2 = 8;
                }
            }
        }

        uint16_t newMagPage = (d2 << 8) | (d1 << 4) | d0;

        return PageId(newMagPage, PageId::ANY_SUBPAGE);
    }
    else
    {
        return PageId();
    }
}

} // namespace ttxdecoder
