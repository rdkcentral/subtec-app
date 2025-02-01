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


#ifndef TTXDECODER_PARSER_HPP_
#define TTXDECODER_PARSER_HPP_

#include <subttxrend/common/NonCopyable.hpp>

#include "CharsetConfig.hpp"
#include "Types.hpp"
#include "PresentationLevel.hpp"

namespace ttxdecoder
{

class Database;
class ParserRowContext;
class DecodedPage;
class DecodedPageRow;
class PageDisplayable;
class PageMagazine;
class PacketHeader;
class PacketLopData;
class PacketEditorialLinks;
class PacketTriplets;

/**
 * Parser.
 *
 * Parses the collected data.
 */
class Parser : private subttxrend::common::NonCopyable
{
public:
    /** Parsing mode. */
    enum class Mode
    {
        /** Parse clock characters only. */
        CLOCK_ONLY,
        /** Parse header line only. */
        HEADER_ONLY,
        /** Parse full page. */
        FULL_PAGE
    };

    /**
     * Constructor.
     *
     * @param level
     *      Presentation level enabled.
     * @param database
     *      Database to use.
     * @param charsetConfig
     *      Current charset configuration.
     */
    Parser(PresentationLevel level,
           const Database& database,
           const CharsetConfig& charsetConfig);

    /**
     * Destructor.
     */
    virtual ~Parser();

    /**
     * Parses the page.
     *
     * This method processes collected page data and decodes the
     * page (split characters from attributes etc.).
     *
     * @param page
     *      Source of data (page).
     * @param header
     *      Source of data (header).
     * @param mode
     *      Parsing mode.
     *      If full page mode is selected the target is cleared
     *      before decoding.
     * @param navigationMode
     *      Navigation mode to use.
     * @param target
     *      Target to store data (parsed page).
     *
     * @return
     *      True if page was modified, false otherwise.
     */
    bool parsePage(const PageDisplayable& page,
                   const PacketHeader& header,
                   const Mode mode,
                   const NavigationMode navigationMode,
                   DecodedPage& target);

private:
    /** Row parsing result. */
    struct RowParseResult
    {
        /**
         * Contructor with default initialization.
         */
        RowParseResult() :
                m_anyChange(false),
                m_anyDoubleHeight(false)
        {
            // noop
        }

        /**
         * Logical or operator.
         *
         * @param other
         *      Other value.
         */
        const RowParseResult& operator |= (const RowParseResult& other)
        {
            m_anyChange |= other.m_anyChange;
            m_anyDoubleHeight |= other.m_anyDoubleHeight;
            return *this;
        }

        /** Any data change was detected when parsing the row. */
        bool m_anyChange;
        /** Any double height character was parsed in this row. */
        bool m_anyDoubleHeight;
    };

    /**
     * Clears FLOF navigation data.
     *
     * @param target
     *      Target to store changes.
     */
    void clearFlof(DecodedPage& target) const;

    /**
     * Fills TOP navigation data.
     *
     * @param target
     *      Target to store data.
     *
     * @return
     *      True if navigation data was stored, false otherwise.
     */
    bool fillTopNavigation(DecodedPage& target) const;

    /**
     * Fills FLOF navigation data.
     *
     * @param source
     *      Source of data (collected page).
     * @param target
     *      Target to store data.
     *
     * @return
     *      True if navigation data was stored, false otherwise.
     */
    bool fillFlofNavigation(const PageDisplayable& source,
                            DecodedPage& target) const;

    /**
     * Resets the parsing context to defaults.
     */
    void resetContext();


    /** Helper type for readability. */
    using NationalAndG2Charset = std::pair<NationalCharset, Charset>;

    /**
     * Decode charset.
     *
     * @param code
     *      Charsets code (7 bits, 4 group + 3 national option).
     *
     * @return
     *      Pair of national charset and G2 charset.
     */
    NationalAndG2Charset decodeCharset(std::uint8_t code) const;

    /**
     * Processes metadata from header.
     *
     * @param source
     *      Header with data.
     */
    void processMetaHeader(const PacketHeader& source);

    /**
     * Processes metadata from packet M/29/4.
     *
     * @param source
     *      Page with the data.
     */
    void processMetaM294(const PageMagazine& source);

    /**
     * Processes metadata from packet M/29/0.
     *
     * @param source
     *      Page with the data.
     */
    void processMetaM290(const PageMagazine& source);

    /**
     * Processes metadata from packet X/28/4.
     *
     * @param source
     *      Page with the data.
     */
    void processMetaX284(const PageDisplayable& source);

    /**
     * Processes metadata from packet X/28/0.
     *
     * @param source
     *      Page with the data.
     */
    void processMetaX280(const PageDisplayable& source);

    /**
     * Processes metadata from packet X/28/0 Format 1 and
     * similar.
     *
     * @param source
     *      Packet with the data.
     * @param primaryClut
     *      The packet modified primary CLUTs (0,1) if true
     *      or secondary CLUTs (2,3) if false.
     */
    void processMetaFormat1(const PacketTriplets& source,
                            bool primaryClut);

    /**
     * Processes X/26 packets and updates target page accordingly.
     *
     * @param source
     *      Page with the data.
     * @param target
     *      Page target.
     */
    void processX26(const PageDisplayable& source,
                    DecodedPage& target);

    /**
     * Checks if boxed mode is enabled for page.
     *
     * @param source
     *      Page source.
     *
     * @return
     *      True if boxed mode is enabled, false otherwise.
     */
    bool isBoxedMode(const PacketHeader& source) const;

    /**
     * Processes page info.
     *
     * @param source
     *      Page source.
     * @param target
     *      Page target.
     */
    void processPageInfo(const PacketHeader& source,
                         DecodedPage& target) const;

    /**
     * Processes header row.
     *
     * @param source
     *      Page source.
     * @param target
     *      Page target.
     *
     * @return
     *      Parsing result.
     */
    RowParseResult processHeader(const PacketHeader& source,
                                 DecodedPageRow& target) const;

    /**
     * Processes displayable row.
     *
     * @param source
     *      Page source.
     * @param target
     *      Page target.
     *
     * @return
     *      Parsing result.
     */
    RowParseResult processDisplayableRow(const PacketLopData& source,
                                         DecodedPageRow& target) const;

    /**
     * Processes row after row with double height characters.
     *
     * @param target
     *      Page target.
     * @param source
     *      Page source.
     */
    void processDoubleHeightNextRow(const DecodedPageRow& source,
                                    DecodedPageRow& target) const;

    /**
     * Processes editorial links (X/27/0) row.
     *
     * @param source
     *      Page source.
     * @param target
     *      Page target.
     */
    void processEditorialLinks(const PacketEditorialLinks& source,
                               DecodedPage& target) const;

    /**
     * Parses page row.
     *
     * @param data
     *      Row characters data.
     * @param length
     *      Row characters length.
     * @param decodedRow
     *      Place to store decoded row data.
     *
     * @return
     *      Parsing result.
     */
    RowParseResult parseRow(const int8_t* data,
                            std::size_t length,
                            DecodedPageRow& decodedRow) const;

    /**
     * Parses single row character.
     *
     * @param context
     *      Row parsing context.
     * @param decodedRow
     *      Place to store decoded row data.
     * @param column
     *      Column number.
     * @param inputCharacter
     *      Character to parse.
     *
     * @return
     *      Character parsing result.
     */
    RowParseResult parseRowCharacter(ParserRowContext& context,
                                     DecodedPageRow& decodedRow,
                                     uint8_t column,
                                     int8_t inputCharacter) const;

    /**
     * Unpacks the character.
     *
     * @param inputCharacter
     *      Character to parse.
     * @param character
     *      Variable to store character to process (0 - not a character).
     * @param spacing
     *      Variable to store spacing to process (0 - no spacing attributes).
     */
    void unpackCharacter(int8_t inputCharacter,
                         uint16_t& character,
                         uint8_t& spacing) const;

    /**
     * Stores parsed character.
     *
     * @param context
     *      Row parsing context.
     *      Contains current character properties.
     * @param decodedRow
     *      Place to store decoded row data.
     * @param column
     *      Column number.
     *
     * @return
     *      True if anything changed, false otherwise.
     */
    bool storeDecodedCharacter(const ParserRowContext& context,
                               DecodedPageRow& decodedRow,
                               uint8_t column) const;

    /**
     * Processes 'set-at' spacing attributes.
     *
     * @param context
     *      Row parsing context.
     * @param spacing
     *      Spacing attributes to process.
     */
    void processSetAtSpacingAttributes(ParserRowContext& context,
                                       uint8_t spacing) const;

    /**
     * Processes 'set-after' spacing attributes.
     *
     * @param context
     *      Row parsing context.
     * @param spacing
     *      Spacing attributes to process.
     *
     * @return
     *      True if double height/size attribute was parsed,
     *      false otherwise.
     */
    bool processSetAfterSpacingAttributes(ParserRowContext& context,
                                          uint8_t spacing) const;

    /**
     * Returns empty lop data packet.
     *
     * The packet is filled with all spaces.
     *
     * @return
     *      Packet pointer.
     */
    const PacketLopData* getEmptyLopData() const;

    /**
     * Inserts TOP navigation metadata to a page.
     *
     * The metadata is only added if there is no existing navigation
     * metadata and the TOP navigation metadata is available.
     *
     * @param page
     *      Page to which TOP navigation metadata shall be added.
     */
    void insertTopMeta(DecodedPage& page);

    /** Database to used. */
    const Database& m_database;

    /** Presentation level enabled. */
    const PresentationLevel m_level;

    /** Parsing context - boxed mode. */
    bool m_boxedMode;

    /** Default charset configuration. */
    const CharsetConfig& m_charsetConfig;

    /** Current primary national charset. */
    NationalCharset m_primaryNationalCharset;

    /** Current secondary national charset. */
    NationalCharset m_secondaryNationalCharset;

    /** Current G2 charset selected. */
    Charset m_currentG2Charset;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PARSER_HPP_*/
