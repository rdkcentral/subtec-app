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


#ifndef TTXDECODER_PAGEID_HPP_
#define TTXDECODER_PAGEID_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Page identifier.
 *
 * Represents magazine page + subpage pair and provide some utilities
 * to handle the page numbers.
 *
 * The magazine page is 3-digit hex number encoded as MPP. Each digit takes
 * 4 bits.
 */
class PageId
{
public:
    /** Invalid magazine page number. */
    static const uint16_t INVALID_MAGAZINE_PAGE = 0xFFFF;

    /** Any subpage number. */
    static const uint16_t ANY_SUBPAGE = 0xFFFF;

    /** NULL page mask. */
    static const uint16_t NULL_MAGAZINE_PAGE_MASK = 0x00FF;

    /** NULL subpage number. */
    static const uint16_t NULL_SUBPAGE = 0x3F7F;


    /**
     * Constructor (default).
     *
     * Sets magazine page 000 and subpage 0.
     */
    PageId() :
            m_magazinePage(INVALID_MAGAZINE_PAGE),
            m_subpage(ANY_SUBPAGE)
    {
        // noop
    }

    /**
     * Constructor (default).
     *
     * @param magazinePage
     *      Magazine page number.
     * @param subpage
     *      Subpage number.
     */
    PageId(std::uint16_t magazinePage,
           std::uint16_t subpage) :
            m_magazinePage(magazinePage),
            m_subpage(subpage)
    {
        // noop
    }

    /**
     * Checks if it is NULL page address.
     *
     * @retval true
     *      It is a null page.
     * @retval false
     *      It is not a null page.
     */
    bool isNull() const
    {
        return (((m_magazinePage & NULL_MAGAZINE_PAGE_MASK)
                == NULL_MAGAZINE_PAGE_MASK) && (m_subpage == NULL_SUBPAGE));
    }

    /**
     * Checks if it is any subpage address.
     *
     * @retval true
     *      It is any subpage address.
     * @retval false
     *      It is not any subpage address.
     */
    bool isAnySubpage() const
    {
        return (m_subpage == ANY_SUBPAGE) || (m_subpage == NULL_SUBPAGE);
    }

    /**
     * Checks if subpage number is valid.
     *
     * @retval true
     *      Subpage number is valid.
     * @retval false
     *      Subpage number is not valid.
     */
    bool isValidSubpage() const
    {
        if (isAnySubpage())
        {
            return true;
        }
        else
        {
            uint16_t d1 = (m_subpage >> 12) & 0x0F;
            uint16_t d3 = (m_subpage >> 4) & 0x0F;

            return (d1 <= 3) && (d3 <= 7);
        }
    }

    /**
     * Checks if page address is valid decimal address.
     *
     * @retval true
     *      Page address is valid decimal address.
     * @retval false
     *      Page address is not valid decimal address.
     */
    bool isValidDecimal() const
    {
        if (isValidSubpage())
        {
            uint16_t m = (m_magazinePage >> 8);
            uint16_t p1 = (m_magazinePage >> 4) & 0x0F;
            uint16_t p2 = (m_magazinePage >> 0) & 0x0F;

            if ((m >= 1) && (m <= 8) && (p1 <= 9) && (p2 <= 9))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Returns magazine page number.
     *
     * @return
     *      Magazine page number.
     */
    std::uint16_t getMagazinePage() const
    {
        return m_magazinePage;
    }

    /**
     * Returns decimal magazine page number.
     *
     * @return
     *      Decimal magazine page number or
     *      INVALID_MAGAZINE_PAGE if page id is not a valid decimal.
     */
    std::uint16_t getDecimalMagazinePage() const
    {
        if (isValidDecimal())
        {
            uint16_t m = (m_magazinePage >> 8);
            uint16_t p1 = (m_magazinePage >> 4) & 0x0F;
            uint16_t p2 = (m_magazinePage >> 0) & 0x0F;

            return m * 100 + p1 * 10 + p2;
        }
        return INVALID_MAGAZINE_PAGE;
    }

    /**
     * Returns page number.
     *
     * @return
     *      Page number.
     */
    std::uint8_t getPage() const
    {
        return m_magazinePage & 0xFF;
    }

    /**
     * Returns subpage number.
     *
     * @return
     *      Subpage number.
     */
    std::uint16_t getSubpage() const
    {
        return m_subpage;
    }

private:
    /** Magazine page. */
    std::uint16_t m_magazinePage;

    /** Subpage. */
    std::uint16_t m_subpage;

};

/**
 * Equality operator.
 *
 * @param pageId1
 *      First page id to compare.
 * @param pageId2
 *      Second page id to compare.
 *
 * @retval true
 *      Ids are equal.
 * @retval false
 *      Ids are different.
 */
inline bool operator ==(const PageId& pageId1,
                        const PageId& pageId2)
{
    return (pageId1.getMagazinePage() == pageId2.getMagazinePage())
            && (pageId1.getSubpage() == pageId2.getSubpage());
}

} // namespace ttxdecoder

#endif /*TTXDECODER_PAGEID_HPP_*/
