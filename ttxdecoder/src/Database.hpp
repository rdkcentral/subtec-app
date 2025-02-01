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


#ifndef TTXDECODER_DATABASE_HPP_
#define TTXDECODER_DATABASE_HPP_

#include "Types.hpp"
#include "PageId.hpp"
#include "PageMagazine.hpp"

namespace ttxdecoder
{

/**
 * Decoder database
 */
class Database
{
public:
    /** Magazine count. */
    static const std::size_t MAGAZINE_COUNT = 8;

    /** Page count. */
    static const std::size_t PAGE_COUNT = MAGAZINE_COUNT * 0x100;

    /**
     * TOP navigation metadata.
     */
    struct TopMetadata
    {
        /**
         * Resest the metadata.
         */
        void reset()
        {
            m_nextPage = PageId::INVALID_MAGAZINE_PAGE;
            m_prevPage = PageId::INVALID_MAGAZINE_PAGE;
            m_nextGroupPage = PageId::INVALID_MAGAZINE_PAGE;
            m_nextBlockPage = PageId::INVALID_MAGAZINE_PAGE;
        }

        /** Next page id. */
        uint16_t m_nextPage;
        /** Prev page id. */
        uint16_t m_prevPage;
        /** Next group page id. */
        uint16_t m_nextGroupPage;
        /** Next block page id. */
        uint16_t m_nextBlockPage;
    };

    /**
     * Constructor.
     */
    Database();

    /**
     * Destructor.
     */
    ~Database() = default;

    /**
     * Resets the database.
     */
    void reset();

    /**
     * Resets TOP navigation metadata.
     */
    void resetTopMetadata();

    /**
     * Returns magazine metadata (page).
     *
     * @param magazine
     *      Magazine number.
     */
    const PageMagazine& getMagazinePage(std::uint8_t magazine) const;

    /**
     * Returns magazine metadata (page).
     *
     * @param magazine
     *      Magazine number.
     */
    PageMagazine& getMagazinePage(std::uint8_t magazine);

    /**
     * Returns next page.
     *
     * @param inputPage
     *      Page for which relative page is to be found.
     * @param navigationMode
     *      Navigation mode to be used.
     *
     * @return
     *      Page identifier (may be invalid).
     */
    PageId getNextPage(const PageId& inputPage,
                       NavigationMode navigationMode) const;

    /**
     * Returns previous page.
     *
     * @param inputPage
     *      Page for which relative page is to be found.
     * @param navigationMode
     *      Navigation mode to be used.
     *
     * @return
     *      Page identifier (may be invalid).
     */
    PageId getPrevPage(const PageId& inputPage,
                       NavigationMode navigationMode) const;

    /**
     * Sets initial page from packet 8/30.
     *
     * @param pageId
     *      Page identifier.
     */
    void setIndexPageP830(const PageId& pageId);

    /**
     * Returns initial page from packet 8/30.
     *
     * @return
     *      Page identifier.
     */
    PageId getIndexPageP830() const;

    /**
     * Returns first subpage.
     *
     * @param page
     *      Page for which operation is performed.
     *
     * @return
     *      Calculated page, invalid if not available.
     */
    PageId getFirstSubpage(const PageId& page) const;

    /**
     * Returns next subpage.
     *
     * @param page
     *      Page for which operation is performed.
     *
     * @return
     *      Calculated page, invalid if not available.
     */
    PageId getNextSubpage(const PageId& page) const;

    /**
     * Returns previous subpage.
     *
     * @param page
     *      Page for which operation is performed.
     *
     * @return
     *      Calculated page, invalid if not available.
     */
    PageId getPrevSubpage(const PageId& page) const;

    /**
     * Returns highest subpage.
     *
     * @param page
     *      Page for which operation is performed.
     *
     * @return
     *      Calculated page, invalid if not available.
     */
    PageId getHighestSubpage(const PageId& page) const;

    /**
     * Returns last received subpage.
     *
     * @param page
     *      Page for which operation is performed.
     *
     * @return
     *      Calculated page, invalid if not available.
     */
    PageId getLastReceivedSubpage(const PageId& page) const;

    /**
     * Returns TOP metadata.
     *
     * @param magazinePage
     *      Magazine page number.
     *
     * @return
     *      TOP metadata structure.
     */
    TopMetadata& getTopMetatadata(const uint16_t magazinePage);

    /**
     * Returns TOP metadata.
     *
     * @param magazinePage
     *      Magazine page number.
     *
     * @return
     *      TOP metadata structure.
     */
    const TopMetadata& getTopMetatadata(const uint16_t magazinePage) const;

private:
    /**
     * Page information.
     */
    struct PageMetadata
    {
        /** TOP navigation metadata. */
        TopMetadata m_topMetadata;
    };

    /**
     * Calculates next magazine page identifier.
     *
     * @param inputPage
     *      Page for which relative page is to be found.
     *
     * @return
     *      Next page id.
     */
    PageId calculateNextPage(const PageId& inputPage) const;

    /**
     * Calculates previous magazine page identifier.
     *
     * @param inputPage
     *      Page for which relative page is to be found.
     *
     * @return
     *      Next page id.
     */
    PageId calculatePrevPage(const PageId& inputPage) const;

    /** Index page from packet 8/30. */
    PageId m_indexPage830;

    /** Magazine metadata. */
    PageMagazine m_magazinePages[MAGAZINE_COUNT];

    /** Pages metadata. */
    PageMetadata m_metadata[PAGE_COUNT];
};

} // namespace ttxdecoder

#endif /*TTXDECODER_DATABASE_HPP_*/
