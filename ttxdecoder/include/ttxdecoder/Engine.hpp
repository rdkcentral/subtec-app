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


#ifndef TTXDECODER_ENGINE_HPP_
#define TTXDECODER_ENGINE_HPP_

#include <cstdint>
#include <array>

#include "PageId.hpp"
#include "DecodedPage.hpp"
#include "Types.hpp"

namespace ttxdecoder
{

/**
 * Engine.
 */
class Engine
{
public:
    /**
     * Constructor.
     */
    Engine() = default;

    /**
     * Destructor.
     */
    virtual ~Engine() = default;

    /**
     * Resets the Teletext decoder acquisition.
     *
     * This function resets the Teletext decoder acquisition. All data within
     * the Teletext database is cleared and the decoder will start acquisition
     * again. This function should be called on each channel change to avoid a
     * mixup of Teletext streams.
     */
    virtual void resetAcquisition() = 0;

    /**
     * Processes data stored in the packet buffer.
     *
     * This function is processing the Teletext data which is stored in the
     * packet buffer. Within this function the currently selected Teletext
     * pages are decoded and prepared for display.
     *
     * @return
     *      Number of processed packets.
     */
    virtual std::uint32_t process() = 0;

    /**
     * Adds a PES packet to be processed.
     *
     * The packet data is copied by this method.
     *
     * @param packet
     *      Pointer to packet memory.
     * @param length
     *      Length of packet in bytes.
     *
     * @retval true
     *      Packet successfully added.
     * @retval false
     *      Operation failed (e.g. lack of buffer space).
     */
    virtual bool addPesPacket(const std::uint8_t* packet,
                              std::uint16_t length) = 0;

    /**
     * Sets current page id.
     *
     * @param pageId
     *      Identifier of the new current page.
     */
    virtual void setCurrentPageId(const PageId& pageId) = 0;

    /**
     * Returns next page id for given page.
     *
     * @param inputPageId
     *      The page number for which next page is to be returned.
     *
     * @return
     *      Page identifier.
     *      May be invalid if requested page cannot be found.
     */
    virtual PageId getNextPageId(const PageId& inputPageId) const = 0;

    /**
     * Returns previous page id for given page.
     *
     * @param inputPageId
     *      The page number for which previous page is to be returned.
     *
     * @return
     *      Page identifier.
     *      May be invalid if requested page cannot be found.
     */
    virtual PageId getPrevPageId(const PageId& inputPageId) const = 0;

    /**
     * Returns page id for given page type.
     *
     * @param type
     *      The type of the page to get.
     *
     * @return
     *      Page identifier.
     *      May be invalid if requested page cannot be found.
     */
    virtual PageId getPageId(PageIdType type) const = 0;

    /**
     * Returns page data.
     *
     * @return
     *      Current page data.
     *      Please not that it may be empty (no characters etc.) if the page
     *      was not yet decoded.
     *      Please not that the state of this page is temporary - any operation
     *      on engine may change its contents.
     */
    virtual const DecodedPage& getPage() const = 0;

    /**
     * Changes the preselected navigation mode.
     *
     * Use this function to change the preselected navigation mode.
     * The mode is used to prepare data needed for requested navigation
     * type when the page is decoded.
     *
     * @param navigationMode
     *      The navigation mode to set.
     */
    virtual void setNavigationMode(NavigationMode navigationMode) = 0;

    /**
     * Return currently used navigation mode.
     *
     * Use this function to get the navigation used in the current
     * page. This mode depends on the global defined navigation mode
     * and the current page navigation content.
     *
     * @return
     *      The navigation state for current page.
     */
    virtual NavigationState getNavigationState() const = 0;

    /**
     * Returns specified TOP navigation link text for the given page.
     *
     * The function is used to get the specified link text for the given page.
     * The link text buffer size must be 13 double byte or more.
     *
     * @note This function is currently not supported.
     *
     * @param magazine
     *      The page to get the link text for.
     * @param linkType
     *      The link type.
     * @param linkbuffer
     *      The buffer for the returned link text.
     */
    virtual void getTopLinkText(std::uint16_t magazine,
                                std::uint32_t linkType,
                                std::uint16_t* linkbuffer) const = 0;

    /**
     * Returns the screen color index for the current page.
     *
     * @return
     *      The screen color index.
     */
    virtual std::uint8_t getScreenColorIndex() const = 0;

    /**
     * Returns the row color index for the current page.
     *
     * Returns the row color index for the specified row. Row colors are
     * supported in Teletext level 2.5 only.
     *
     * @param row
     *      The row number to get the color for
     *
     * @return
     *      The row color index.
     */
    virtual std::uint8_t getRowColorIndex(std::uint8_t row) const = 0;

    /**
     * Returns colors for level 2.5 profile.
     *
     * @param colors
     *      Colors array to fill.
     *
     * @retval true
     *      Colors successfully returned.
     * @retval false
     *      Operation failed.
     */
    virtual bool getColors(std::array<std::uint32_t, 16>& colors) const = 0;

    /**
     * Returns charset mapping table.
     *
     * Gets 96 characters from 0x20 to 0x7F.
     *
     * @param charset
     *      Charset to get mapping for.
     *
     * @return
     *      Mapping array.
     */
    virtual const CharsetMappingArray& getCharsetMapping(Charset charset) const = 0;

    /**
     * Sets charset mapping table.
     *
     * Used to modify the specified charset mapping table, e.g. if needed
     * to map to nonstandard font.
     *
     * Sets 96 characters from 0x20 to 0x7F
     *
     * National options are coded as characters from Latin G0 or G2 PLUS
     * diacritic mark. It should be possible to get an approximate rendering of
     * national options using only Latin G0 and G2 glyphs.
     *
     * @note Any diacritic mark from a national option will replace any
     *       transmitted diacritic mark.
     *
     * @note To support the English, Polish and Turkish national options
     *       characters 0x59, 0x5A and 0x5B that are not transmitted are used
     *       to indicate the double vertical line, Z with slash and turkish
     *       lira sign respectively in Latin G2 charset.
     *
     * @param charset
     *      Charset to set mapping for.
     * @param mappingArray
     *      Array with mapping.
     */
    virtual void setCharsetMapping(Charset charset,
                                   const CharsetMappingArray& mappingArray) = 0;

    /**
     * Redefines default national primary charset entries.
     *
     * For Teletext pages without additional charset information, the national
     * option bits C12,13,14 are translated using a regional charset table to
     * get the correct charset setting. This function allows to redefine this
     * table.
     *
     * @param index
     *      The national option index (0-7 allowed)
     * @param charset
     *      The charset to set for this entry
     */
    virtual void setDefaultPrimaryNationalCharset(std::uint8_t index,
                                                  NationalCharset charset) = 0;

    /**
     * Redefines default national secondary charset entry.
     *
     * For Teletext pages without additional charset information, the national
     * option bits C12,13,14 are translated using a regional charset table to
     * get the correct charset setting. This function allows to redefine this
     * table.
     *
     * @param index
     *      The national option index (0-7 allowed)
     * @param charset
     *      The charset to set for this entry
     */
    virtual void setDefaultSecondaryNationalCharset(std::uint8_t index,
                                                    NationalCharset charset) = 0;

    /**
     * Returns the control bits of the header row.
     *
     * This function returns the control bits 10,9,8,7,6,5,4 of the header row
     * of current page.
     *
     * @return
     *   The C bits 11,10,9,8,7,6,5,4 of the header as one byte bit coded.
     */
    virtual uint8_t getPageControlInfo() const = 0;

    /**
     * Sets flag to ignore PTS in PES packets.
     *
     * @param ignorePts
     *      Value of flag.
     */
    virtual void setIgnorePts(bool ignorePts) = 0;

};

} // namespace ttxdecoder

#endif /*TTXDECODER_ENGINE_HPP_*/
