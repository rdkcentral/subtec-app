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


#ifndef TTXDECODER_DECODEDPAGE_HPP_
#define TTXDECODER_DECODEDPAGE_HPP_

#include <cstdint>
#include <array>

#include "PageId.hpp"

namespace ttxdecoder
{

/**
 * Segment of the decoded page row.
 */
template<std::size_t WIDTH>
class DecodedPageRowSegment
{
public:
    /**
     * Returns segment size.
     */
    std::size_t getWidth() const
    {
        return WIDTH;
    }

    /**
     * Clears the segment contents.
     */
    void clear()
    {
        m_charArray.fill(' ');
        m_bgColorIndexArray.fill(0);
        m_fgColorIndexArray.fill(0);
        m_propertiesArray.fill(0);
    }

    /** Segment characters. */
    std::array<std::uint16_t, WIDTH> m_charArray;

    /** Background color indexes. */
    std::array<std::uint8_t, WIDTH> m_bgColorIndexArray;

    /** Foreground color indexes. */
    std::array<std::uint8_t, WIDTH> m_fgColorIndexArray;

    /** Character properties. */
    std::array<uint16_t, WIDTH> m_propertiesArray;
};

/**
 * Decoded page row data.
 */
class DecodedPageRow
{
public:
    /** Width of the row. */
    static const std::size_t LEVEL_ONE_PAGE_WIDTH = 40;

    /**
     * Clears the row contents.
     */
    void clear()
    {
        m_levelOnePageSegment.clear();
    }

    /** Main panel data. */
    DecodedPageRowSegment<LEVEL_ONE_PAGE_WIDTH> m_levelOnePageSegment;
};

/**
 * Decoded teletext page.
 */
class DecodedPage
{
public:
    /** Maximum number of rows (0..25). */
    static const std::size_t MAX_ROWS = 1 + 25;

    /** Colour key link type. */
    enum class Link
    {
        /** RED */
        RED = 0,
        /** GREEN */
        GREEN = 1,
        /** YELLOW */
        YELLOW = 2,
        /** CYAN */
        CYAN = 3,
        /** FLOF INDEX */
        FLOF_INDEX = 4,

        /** Last enum value. */
        LAST = FLOF_INDEX
    };

    /**
     * Constructor.
     *
     * Clears the page.
     */
    DecodedPage();

    /**
     * Destructor.
     */
    ~DecodedPage() = default;

    /**
     * Clears the page contents.
     */
    void clear();

    /**
     * Returns page id (number).
     *
     * @return
     *      Page identifier.
     */
    PageId getPageId() const;

    /**
     * Sets page id (number).
     *
     * @param pageId
     *      Page identifier.
     */
    void setPageId(PageId pageId);

    /**
     * Returns page control bits value.
     *
     * @return
     *      Control bits value.
     */
    std::uint8_t getPageControlInfo() const;

    /**
     * Sets page control bits value.
     *
     * @param controlInfo
     *      Control bits value.
     */
    void setPageControlInfo(std::uint8_t controlInfo);

    /**
     * Returns number of available rows.
     *
     * @return
     *      Number of rows.
     */
    std::size_t getRowCount() const;

    /**
     * Returns page row.
     *
     * @param row
     *      Row number.
     *      Index 0 is used by header.
     *      Available indexes are in range 0..MAX_ROWS-1.
     *
     * @return
     *      Page row.
     */
    DecodedPageRow& getRow(std::size_t row);

    /**
     * Returns page row.
     *
     * @param row
     *      Row number.
     *      Index 0 is used by header.
     *      Available indexes are in range 0..MAX_ROWS-1.
     *
     * @return
     *      Page row.
     */
    const DecodedPageRow& getRow(std::size_t row) const;

    /**
     * Returns colour link.
     *
     * @param link
     *      Link type.
     * @param linkValue
     *      Link value.
     */
    void setColourKeyLink(Link link,
                          const PageId& linkValue);

    /**
     * Returns colour link.
     *
     * @param link
     *      Link type.
     *
     * @return
     *      Link value (may be invalid).
     */
    const PageId& getColourKeyLink(Link link) const;

    /**
     * Dumps the page contents using logger.
     *
     * @param charsOnly
     *      If true only characters are dumped.
     */
    void dump(bool charsOnly);

private:
    /** Number of colour links. */
    static const std::size_t COLOUR_LINKS_COUNT =
            static_cast<std::size_t>(Link::LAST) + 1;

    /** Page identifier (number). */
    PageId m_pageId;

    /** Control info bits value. */
    std::uint8_t m_controlInfo;

    /** Rows data. */
    std::array<DecodedPageRow, MAX_ROWS> m_rows;

    /** Colour key links. */
    std::array<PageId,COLOUR_LINKS_COUNT> m_colourKeyLinks;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_DECODEDPAGE_HPP_*/
