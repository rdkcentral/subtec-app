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


#ifndef TTXDECODER_TOPNAVPROCESSOR_HPP_
#define TTXDECODER_TOPNAVPROCESSOR_HPP_

#include "PageId.hpp"
#include "PageBtt.hpp"

namespace ttxdecoder
{

class PageBtt;
class DecodedPage;
class Database;

/**
 * TOP navigation metadata processor.
 */
class TopNavProcessor
{
public:
    /**
     * Constructor.
     *
     * @param database
     *      Database to use.
     */
    TopNavProcessor(Database& database);

    /**
     * Destructor.
     */
    ~TopNavProcessor() = default;

    /**
     * Resets the processor state.
     */
    void reset();

    /**
     * Returns page buffer.
     *
     * Asks the processor for buffer for processing metadata.
     * If processor is interested in the given page it shall return
     * buffer. The buffer shall remain valid until next call to getPageBuffer.
     *
     * @param pageId
     *      Page id.

     * @return
     *      Page buffer or nullptr if page is not needed
     */
    Page* getPageBuffer(const PageId& pageId);

    /**
     * Processes the collected page.
     *
     * @param page
     *      Page to be processed.
     */
    void processPage(const Page& page);

private:
    /** BTT magazine page number. */
    static const std::uint16_t BTT_PAGE_ID = 0x01F0;

    /**
     * Page type used in BTT page data.
     * Source: https://www-user.tu-chemnitz.de/~heha/vt/topspez/topspez.htm (Figure 4)
     */
    enum BttPageType
        : uint8_t
        {
            BTT_NO_PAGE = 0,     // No Page
            BTT_SUBTITLE,        // Subtitle page
            BTT_PROGRAM_INDEX_S, // Program Index, Single Page
            BTT_PROGRAM_INDEX_M, // Program Index Multiple Pages
            BTT_BLOCK_S,         // Page in Block, Single Page
            BTT_BLOCK_M,         // Page in Block, Multipe Pages
            BTT_GROUP_S,         // Page in Group, Single Page
            BTT_GROUP_M,         // Page in Group, Multiple Pages
            BTT_NORMAL_S,        // Normal Page, Single Page
            BTT_NORMAL_S_AI,     // Normal Page, Single Page, Additional Information
            BTT_NORMAL_M,        // Normal Page, Multiple Pages
            BTT_NORMAL_M_AI,     // Normal Page, Multiple Pages, Additional Information
            BTT_FUTURE_USE1,     // Reserved for future use
            BTT_FUTURE_USE2,     // Reserved for future use
            BTT_FUTURE_USE3,     // Reserved for future use
            BTT_FUTURE_USE4      // Reserved for future use
    };

    /**
     * Processes TOP navigation metadata page.
     *
     * @param collectedPage
     *      Data of the page to process.
     */
    void processTypedPage(const PageBtt& collectedPage);

    /**
     * Sets page types.
     *
     * @param collectedPage
     *      BTT page data.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure.
     */
    bool setTypes(const PageBtt& collectedPage);

    /**
     * Fills the previous page numbers in database.
     */
    void fillPrevPage();

    /**
     * Fills the next page numbers in database.
     */
    void fillNextPage();

    /**
     * Fills the next group page numbers in database.
     */
    void fillNextGroup();

    /**
     * Fills the next block page numbers in database.
     */
    void fillNextBlock();

    /**
     * Converts hex page ID to index.
     *
     * @param hexPage
     *      HEX page id to convert. Must be valid decimal page id.
     *
     * @return
     *      Index of the page in database.
     */
    int hexPageToindex(uint16_t hexPage) const;

    /**
     * Converts index of the page to hex page ID.
     *
     * @param index
     *      Index to convert.
     *
     * @return
     *      Hex encoded page id.
     */
    uint16_t indexToHexPage(int index) const;

    /**
     * Checks if type is group type.
     *
     * @param type
     *      Type to check.
     *
     * @return
     *      True if given type is group type, false otherwise.
     */
    static bool isGroupType(BttPageType type);

    /**
     * Checks if type is block type.
     *
     * @param type
     *      Type to check.
     *
     * @return
     *      True if given type is block type, false otherwise.
     */
    static bool isBlockType(BttPageType type);

    /**
     * Checks if type is navigable type.
     *
     * @param type
     *      Type to check.
     *
     * @return
     *      True if given type is navigable type, false otherwise.
     */
    static bool isNavigableType(BttPageType type);

    /** Database to use. */
    Database& m_database;

    /** Page information. */
    BttPageType m_pageTypes[800];

    /** BTT page data buffer. */
    PageBtt m_pageBtt;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_TOPNAVPROCESSOR_HPP_*/
