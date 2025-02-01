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


#include "Parser.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ParserRowContext.hpp"
#include "DecodedPage.hpp"
#include "PageDisplayable.hpp"
#include "PageMagazine.hpp"
#include "Database.hpp"
#include "ControlInfo.hpp"

/*

 ====== Elements to be supported for Presentation Level 1 =======

 Display Rows and Columns

 Up to 24 rows, (plus an optional 25th row for navigation purposes), transmitted in
 packets X/0 to X/24, top to bottom of a page in magazine X.
 40 character spaces (columns) in rows 1 to 24, transmitted from left to right.
 Following the 8 control codes (which should not be displayed), 32 characterspaces
 in the header (row 0), transmitted from left to right.
 NOTE: It is usual practice to transmit "time-of-day" information in the last 8
 character spaces of packet X/0.

 ======================

 Character Bytes

 In packets X/0 to X/25, each data byte (coded 7 bits plus odd parity) selects an
 alphanumeric character from a G0 character set, a mosaic character from the G1
 character set or a spacing attribute to occupy a character-space.

 ======================

 G0 Character Set

 Set of 95 alphanumeric characters, plus SPACE (2/0), used as the default
 character set. Address range 2/0 to 7/F. The Latin G0 set, table 35, is the default
 set.
 National option characters exist for certain locations in some G0 sets. Where the
 decoder is capable of displaying more than one national option sub-set, the
 correct character to display is determined from control bits C12 - C14. Note that
 this substitution of certain characters only occurs when a G0 set character is
 referenced from packets X/1-X/25 of the Level 1 page. It does not apply when
 characters are referenced via packets X/26 or objects.
 In some countries two G0 sets are required to meet the language requirements.
 The spacing attribute ESC (code 1/B) is used to switch between the two
 alternative character sets. The default G0 set to be used at the start of each
 display row is determined from the C12 - C14 control bits. Each occurrence of
 ESC within the display row causes the decoder to toggle between the two sets.

 ======================

 G1 Character Set

 Fixed set of 63 block mosaic characters plus SPACE (2/0) and 32 alphanumeric
 characters, table 47. Address range 2/0 to 7/F. The mosaic characters are
 displayed with their elements either contiguous or separated, (see clause 15.7.1).
 The alphanumeric characters at positions 4/0 to 5/F are taken from the G0 set,
 applying the same rules to national option positions as described above.

 ======================

 Character Colours

 8 background full intensity colours: Black, Red, Green, Yellow, Blue, Magenta, Cyan, White.
 7 foreground full intensity colours: Red, Green, Yellow, Blue, Magenta, Cyan, White.
 Invoked as spacing attributes via codes in packets X/0 to X/25.

 ======================

 (the list is to be completed)

 */

// #define FULL_STANDARD_COMPLIANT
#undef FULL_STANDARD_COMPLIANT

namespace ttxdecoder
{

namespace
{

// E.14 Real-time clock Page Header Packets
// The last 8 bytes of page header packets are usually coded with
// a real time clock: hours, minutes seconds numerically in that order.
const std::size_t CLOCK_LENGTH = 8;

struct CharsetDesignator
{
    uint8_t m_code;
    NationalCharset m_charsetG0;
    Charset m_charsetG2;
};

#define CHARSET_CODE(group,option) \
    ((((group) & 0xF) << 3) | (((option) & 0x7) << 0))

const CharsetDesignator CHARSET_DESIGNATORS[] =
{
{ CHARSET_CODE(0x0, 0x0), NationalCharset::ENGLISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x1), NationalCharset::GERMAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x2), NationalCharset::SWEDISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x3), NationalCharset::ITALIAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x4), NationalCharset::FRENCH, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x5), NationalCharset::SPANISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x0, 0x6), NationalCharset::CZECH, Charset::G2_LATIN },

{ CHARSET_CODE(0x1, 0x0), NationalCharset::POLISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x1, 0x1), NationalCharset::GERMAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x1, 0x2), NationalCharset::SWEDISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x1, 0x3), NationalCharset::ITALIAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x1, 0x4), NationalCharset::FRENCH, Charset::G2_LATIN },
{ CHARSET_CODE(0x1, 0x6), NationalCharset::CZECH, Charset::G2_LATIN },

{ CHARSET_CODE(0x2, 0x0), NationalCharset::ENGLISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x1), NationalCharset::GERMAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x2), NationalCharset::SWEDISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x3), NationalCharset::ITALIAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x4), NationalCharset::FRENCH, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x5), NationalCharset::SPANISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x2, 0x6), NationalCharset::TURKISH, Charset::G2_LATIN },

{ CHARSET_CODE(0x3, 0x5), NationalCharset::SERBIAN, Charset::G2_LATIN },

{ CHARSET_CODE(0x4, 0x0), NationalCharset::CYRILLIC_1, Charset::G2_CYRILLIC },
{ CHARSET_CODE(0x4, 0x1), NationalCharset::GERMAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x4, 0x2), NationalCharset::ESTONIAN, Charset::G2_LATIN },
{ CHARSET_CODE(0x4, 0x3), NationalCharset::LETTISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x4, 0x4), NationalCharset::CYRILLIC_2, Charset::G2_CYRILLIC },
{ CHARSET_CODE(0x4, 0x5), NationalCharset::CYRILLIC_3, Charset::G2_CYRILLIC },
{ CHARSET_CODE(0x4, 0x6), NationalCharset::CZECH, Charset::G2_LATIN },

{ CHARSET_CODE(0x6, 0x6), NationalCharset::TURKISH, Charset::G2_LATIN },
{ CHARSET_CODE(0x6, 0x7), NationalCharset::GREEK, Charset::G2_GREEK },

{ CHARSET_CODE(0x8, 0x0), NationalCharset::ENGLISH, Charset::G2_ARABIC },
{ CHARSET_CODE(0x8, 0x4), NationalCharset::FRENCH, Charset::G2_ARABIC },
{ CHARSET_CODE(0x8, 0x7), NationalCharset::ARABIC, Charset::G2_ARABIC },

{ CHARSET_CODE(0xA, 0x5), NationalCharset::HEBREW, Charset::G2_ARABIC },
{ CHARSET_CODE(0xA, 0x7), NationalCharset::ARABIC, Charset::G2_ARABIC }, };

subttxrend::common::Logger g_logger("TtxDecoder", "Parser");

} // namespace <anonymous>

Parser::Parser(PresentationLevel level,
               const Database& database,
               const CharsetConfig& charsetConfig) :
        m_database(database),
        m_level(level),
        m_boxedMode(),
        m_charsetConfig(charsetConfig),
        m_primaryNationalCharset(charsetConfig.getPrimaryNationalCharset()),
        m_secondaryNationalCharset(charsetConfig.getSecondaryNationalCharset()),
        m_currentG2Charset(Charset::G2_LATIN)
{
    // to init the data
    getEmptyLopData();
}

Parser::~Parser()
{
    // noop
}

bool Parser::parsePage(const PageDisplayable& page,
                       const PacketHeader& header,
                       const Mode mode,
                       const NavigationMode navigationMode,
                       DecodedPage& target)
{
    g_logger.trace("%s mode=%d", __func__, static_cast<int>(mode));

    auto pageChanged = header.getPageId().getDecimalMagazinePage() != target.getPageId().getDecimalMagazinePage();
    auto eraseFlag = ((header.getControlInfo() & ControlInfo::ERASE_PAGE) != 0);

    auto erasePage = (eraseFlag || pageChanged);

    if ((mode == Mode::FULL_PAGE) && erasePage)
    {
        target.clear();
    }

    const auto& magazine = m_database.getMagazinePage(
            header.getMagazineNumber());

    resetContext();

    // the order is important, as the data is overwritten
    // - Where M/29/0 and M/29/4 are transmitted for the same magazine,
    //   M/29/0 takes precedence over M/29/4.
    // - Where packets 28/0 and 28/4 are both transmitted as part of a page,
    //   packet 28/0 takes precedence over 28/4 for all but the colour map
    //   entry coding.
    if (mode != Mode::CLOCK_ONLY)
    {
        processMetaHeader(header);
    }
    else
    {
        processMetaHeader(*page.getHeader());
    }
    processMetaM294(magazine);
    processMetaM290(magazine);
    processMetaX284(page);
    processMetaX280(page);

    if (mode == Mode::FULL_PAGE)
    {
        processPageInfo(header, target);
    }

    RowParseResult rowResult;

    if (mode != Mode::CLOCK_ONLY)
    {
        rowResult = processHeader(header, target.getRow(0));
    }
    else
    {
        // build temporary header packet with copied clock bytes

        PacketHeader tmpHeader = *page.getHeader();

        auto tmpData = tmpHeader.getBuffer() + tmpHeader.getBufferLength()
                - CLOCK_LENGTH;
        const auto newData = header.getBuffer() + header.getBufferLength()
                - CLOCK_LENGTH;

        for (std::size_t i = 0; i < CLOCK_LENGTH; ++i)
        {
            tmpData[i] = newData[i];
        }

        rowResult = processHeader(tmpHeader, target.getRow(0));
    }

    if (mode == Mode::FULL_PAGE)
    {
        rowResult.m_anyChange = true;

        for (std::uint8_t i = 1; i <= PageDisplayable::DISPLAYABLE_ROWS; ++i)
        {
            if (rowResult.m_anyDoubleHeight)
            {
                processDoubleHeightNextRow(target.getRow(i - 1),
                        target.getRow(i));

                rowResult.m_anyDoubleHeight = false;
            }
            else
            {
                auto displayableRow = page.getLopData(i);
                if (!displayableRow && erasePage)
                {
                    displayableRow = getEmptyLopData();
                }
                if (displayableRow)
                {
                    auto tmpResult = processDisplayableRow(*displayableRow,
                            target.getRow(i));
                    rowResult.m_anyDoubleHeight = tmpResult.m_anyDoubleHeight;
                }
            }
        }

        bool hasFlof = false;

        switch (navigationMode)
        {
        case NavigationMode::TOP_DEFAULT:
            fillTopNavigation(target);
            break;
        case NavigationMode::FLOF_DEFAULT:
            hasFlof = fillFlofNavigation(page, target);
            break;
        case NavigationMode::TOP_FLOF_DEFAULT:
            if (!fillTopNavigation(target))
            {
                hasFlof = fillFlofNavigation(page, target);
            }
            break;
        case NavigationMode::FLOF_TOP_DEFAULT:
            hasFlof = fillFlofNavigation(page, target);
            if (!hasFlof)
            {
                (void) fillTopNavigation(target);
            }
            break;
        default:
            break;
        }

        if (!hasFlof)
        {
            clearFlof(target);
        }

        processX26(page, target);

        // parse other elements if needed
    }

    return rowResult.m_anyChange;
}

void Parser::clearFlof(DecodedPage& target) const
{
    // clear the navigation row
    auto emptyRow = getEmptyLopData();
    processDisplayableRow(*emptyRow, target.getRow(24));
}

bool Parser::fillTopNavigation(DecodedPage& target) const
{
    auto magazinePage = target.getPageId().getMagazinePage();
    const auto& topMetadata = m_database.getTopMetatadata(magazinePage);

    PageId redLink(topMetadata.m_nextPage, PageId::ANY_SUBPAGE);
    PageId greenLink(topMetadata.m_prevPage, PageId::ANY_SUBPAGE);
    PageId yellowLink(topMetadata.m_nextGroupPage, PageId::ANY_SUBPAGE);
    PageId cyanLink(topMetadata.m_nextBlockPage, PageId::ANY_SUBPAGE);

    if (redLink.isValidDecimal() && greenLink.isValidDecimal()
            && yellowLink.isValidDecimal() && cyanLink.isValidDecimal())
    {
        g_logger.trace("%s - %04X - %04X %04X %04X %04X", __func__,
                magazinePage, redLink.getMagazinePage(),
                greenLink.getMagazinePage(), yellowLink.getMagazinePage(),
                cyanLink.getMagazinePage());

        target.setColourKeyLink(DecodedPage::Link::RED, redLink);
        target.setColourKeyLink(DecodedPage::Link::GREEN, greenLink);
        target.setColourKeyLink(DecodedPage::Link::YELLOW, yellowLink);
        target.setColourKeyLink(DecodedPage::Link::CYAN, cyanLink);

        return true;
    }

    return false;
}

bool Parser::fillFlofNavigation(const PageDisplayable& source,
                                DecodedPage& target) const
{
    auto editorialLinks = source.getEditorialLinks();
    if (editorialLinks)
    {
        if (editorialLinks->getLink(0).isValidDecimal()
                || editorialLinks->getLink(1).isValidDecimal()
                || editorialLinks->getLink(2).isValidDecimal()
                || editorialLinks->getLink(3).isValidDecimal()
                || editorialLinks->getLink(5).isValidDecimal())
        {
            g_logger.trace("%s - %04X", __func__,
                    target.getPageId().getMagazinePage());

            // link control currently not used

            target.setColourKeyLink(DecodedPage::Link::RED,
                    editorialLinks->getLink(0));
            target.setColourKeyLink(DecodedPage::Link::GREEN,
                    editorialLinks->getLink(1));
            target.setColourKeyLink(DecodedPage::Link::YELLOW,
                    editorialLinks->getLink(2));
            target.setColourKeyLink(DecodedPage::Link::CYAN,
                    editorialLinks->getLink(3));
            target.setColourKeyLink(DecodedPage::Link::FLOF_INDEX,
                    editorialLinks->getLink(5));

            return true;
        }
    }

    return false;
}

void Parser::resetContext()
{
    m_boxedMode = false;

    m_primaryNationalCharset = m_charsetConfig.getPrimaryNationalCharset();
    m_secondaryNationalCharset = m_charsetConfig.getSecondaryNationalCharset();

    m_currentG2Charset = Charset::G2_LATIN;
}

Parser::NationalAndG2Charset Parser::decodeCharset(std::uint8_t code) const
{
    static const auto DESIGNATOR_COUNT = sizeof(CHARSET_DESIGNATORS)
            / sizeof(CHARSET_DESIGNATORS[0]);

    for (std::size_t i = 0; i < DESIGNATOR_COUNT; ++i)
    {
        if (CHARSET_DESIGNATORS[i].m_code == code)
        {
            return std::make_pair(CHARSET_DESIGNATORS[i].m_charsetG0,
                                  CHARSET_DESIGNATORS[i].m_charsetG2);
        }
    }

    return std::make_pair(NationalCharset::ENGLISH, Charset::G2_LATIN);
}

void Parser::processMetaHeader(const PacketHeader& source)
{
    m_boxedMode = isBoxedMode(source);

    std::uint8_t nationalCode = source.getNationalOption();

    m_primaryNationalCharset = m_charsetConfig.getPrimaryNationalCharset(
            nationalCode);
    m_secondaryNationalCharset = m_charsetConfig.getSecondaryNationalCharset(
            nationalCode);
}

void Parser::processMetaM294(const PageMagazine& source)
{
    auto packet = source.getPacket29_4();
    if (!packet)
    {
        return;
    }

    g_logger.trace("%s", __func__);

    processMetaFormat1(*packet, true);
}

void Parser::processMetaM290(const PageMagazine& source)
{
    auto packet = source.getPacket29_0();
    if (!packet)
    {
        return;
    }

    g_logger.trace("%s", __func__);

    processMetaFormat1(*packet, false);
}

void Parser::processMetaX284(const PageDisplayable& source)
{
    auto packet = source.getPacketX28(4);
    if (!packet)
    {
        return;
    }

    g_logger.trace("%s", __func__);

    processMetaFormat1(*packet, true);
}

void Parser::processMetaX280(const PageDisplayable& source)
{
    auto packet = source.getPacketX28(0);
    if (!packet)
    {
        return;
    }

    g_logger.trace("%s", __func__);

    processMetaFormat1(*packet, false);
}

void Parser::processMetaFormat1(const PacketTriplets& source,
                                bool primaryClut)
{
    uint32_t triplet1 = source.getTripletValue(0);
    uint32_t triplet2 = source.getTripletValue(1);

    if ((triplet1 & 0x7F) != 0)
    {
        return;
    }

    const uint32_t primaryCharsetCode = ((triplet1 >> 7) & 0x7F);
    const uint32_t secondaryCharsetCode = ((triplet1 >> 11) & 0x78)
            + ((triplet2 >> 0) & 0x07);

    auto decodedPrimaryCharsets = decodeCharset(primaryCharsetCode);

    m_primaryNationalCharset = decodedPrimaryCharsets.first;
    m_currentG2Charset = decodedPrimaryCharsets.second;

    m_secondaryNationalCharset = decodeCharset(secondaryCharsetCode).first;

    g_logger.debug("%s - selected charsets: %d / %d, G2: %d", __func__,
            static_cast<int>(m_primaryNationalCharset),
            static_cast<int>(m_secondaryNationalCharset),
            static_cast<int>(m_currentG2Charset));

    // decode other elements if needed (CLUTs, side panels, ...)
}

bool Parser::isBoxedMode(const PacketHeader& source) const
{
    bool boxedMode = false;

    if (((source.getControlInfo() & ControlInfo::NEWSFLASH) != 0)
            || ((source.getControlInfo() & ControlInfo::SUBTITLE) != 0))
    {
        boxedMode = true;
    }

    return boxedMode;
}

void Parser::processPageInfo(const PacketHeader& source,
                             DecodedPage& target) const
{
    target.setPageId(source.getPageId());
    target.setPageControlInfo(source.getControlInfo());
}

Parser::RowParseResult Parser::processHeader(const PacketHeader& source,
                                             DecodedPageRow& target) const
{
    return parseRow(source.getBuffer(), source.getBufferLength(), target);
}

Parser::RowParseResult Parser::processDisplayableRow(const PacketLopData& source,
                                                     DecodedPageRow& target) const
{
    return parseRow(source.getBuffer(), source.getBufferLength(), target);
}

void Parser::processDoubleHeightNextRow(const DecodedPageRow& source,
                                        DecodedPageRow& target) const
{
    const auto& sourceSegment = source.m_levelOnePageSegment;
    auto& targetSegment = target.m_levelOnePageSegment;

    target.clear();
    targetSegment.m_bgColorIndexArray = sourceSegment.m_bgColorIndexArray;
    targetSegment.m_propertiesArray = sourceSegment.m_propertiesArray;
    for (std::size_t i = 0; i < targetSegment.getWidth(); ++i)
    {
        targetSegment.m_propertiesArray[i] &= Property::VALUE_HIDDEN;
    }
}

Parser::RowParseResult Parser::parseRow(const int8_t* data,
                                        std::size_t length,
                                        DecodedPageRow& decodedRow) const
{
    Parser::RowParseResult rowResult;

    ParserRowContext rowContext(m_primaryNationalCharset,
            m_secondaryNationalCharset, m_charsetConfig, m_boxedMode);

    // side panels are currently not supported - so nothing to do

    // process initial characters
    std::size_t column = 0;
    std::size_t padding = DecodedPageRow::LEVEL_ONE_PAGE_WIDTH - length;
    for (std::size_t i = 0; i < padding; ++i)
    {
        (void)parseRowCharacter(rowContext, decodedRow, column, ' ');
        ++column;
    }
    for (std::size_t i = 0; i < length; ++i)
    {
        rowResult |= parseRowCharacter(rowContext, decodedRow, column, data[i]);

        ++column;
    }

    return rowResult;
}

Parser::RowParseResult Parser::parseRowCharacter(ParserRowContext& context,
                                                 DecodedPageRow& decodedRow,
                                                 uint8_t column,
                                                 int8_t inputCharacter) const
{
    uint16_t character = 0;
    uint8_t spacing = 0;
    Parser::RowParseResult result;

    unpackCharacter(inputCharacter, character, spacing);

    if (character == 0)
    {
        processSetAtSpacingAttributes(context, spacing);
    }

    context.storeCharacter(character);

    result.m_anyChange = storeDecodedCharacter(context, decodedRow, column);

    if (character == 0)
    {
        result.m_anyDoubleHeight = processSetAfterSpacingAttributes(context,
                spacing);
    }

    return result;
}

void Parser::unpackCharacter(int8_t inputCharacter,
                             uint16_t& character,
                             uint8_t& spacing) const
{
    if ((inputCharacter & 0x80) != 0)
    {
        character = ' ';
        spacing = 0;
    }
    else if (inputCharacter <= 0x1F)
    {
        character = 0;
        spacing = static_cast<uint8_t>(inputCharacter);
    }
    else
    {
        character = static_cast<uint16_t>(inputCharacter);
        spacing = 0;
    }
}

bool Parser::storeDecodedCharacter(const ParserRowContext& context,
                                   DecodedPageRow& decodedRow,
                                   uint8_t column) const
{
    bool anyChange = false;

    auto& segment = decodedRow.m_levelOnePageSegment;
    auto properties = context.getProperties();
    if (((properties & Property::VALUE_DOUBLE_WIDTH) != 0) && (column > 0))
    {
        if (segment.m_propertiesArray[column - 1]
                & Property::VALUE_DOUBLE_WIDTH)
        {
            properties &= ~Property::VALUE_DOUBLE_WIDTH;
        }
    }

    anyChange |= (segment.m_charArray[column] != context.getCharacter());
    anyChange |= (segment.m_bgColorIndexArray[column] != context.getBgColorIndex());
    anyChange |= (segment.m_fgColorIndexArray[column] != context.getFgColorIndex());
    anyChange |= (segment.m_propertiesArray[column] != properties);

    if (anyChange)
    {
        segment.m_charArray[column] = context.getCharacter();
        segment.m_bgColorIndexArray[column] = context.getBgColorIndex();
        segment.m_fgColorIndexArray[column] = context.getFgColorIndex();
        segment.m_propertiesArray[column] = properties;
    }

#if VERBOSE_LOGGING
    g_logger.trace("%s - seg=%p col=%u - ch=%04x bg=%02x fg=%02x pr=%04x",
            __func__, &segment, column, segment.m_charArray[column],
            segment.m_bgColorIndexArray[column], segment.m_fgColorIndexArray[column],
            segment.m_propertiesArray[column]);
#endif

    return anyChange;
}

void Parser::processSetAtSpacingAttributes(ParserRowContext& context,
                                           uint8_t spacing) const
{
    // set-at spacing processing
    switch (spacing)
    {
    case 0x09:
        context.unsetFlash();
        break;
    case 0x0C:
        context.setSizeMode(ParserRowContext::SizeMode::NORMAL);
        break;
    case 0x18:
        context.setConcealed();
        break;
    case 0x19:
        context.setMosaicMode(ParserRowContext::MosaicMode::CONTIGUOUS);
        break;
    case 0x1A:
        context.setMosaicMode(ParserRowContext::MosaicMode::SEPARATED);
        break;
    case 0x1C:
        context.setBlackBackground();
        break;
    case 0x1D:
        // TODO: CLUT logic for PL 2.5/3.5
        context.setNewBackground();
        break;
    case 0x1E:
        context.holdMosaics();
        break;
    }
}

bool Parser::processSetAfterSpacingAttributes(ParserRowContext& context,
                                              uint8_t spacing) const
{
    bool doubleHeight = false;

    // set-after spacing processing
    switch (spacing)
    {
    case 0x00:
#ifdef FULL_STANDARD_COMPLIANT
        if ((m_level == PresentationLevel::LEVEL_1)
                || (m_level == PresentationLevel::LEVEL_1_5))
        {
            // ignore
            break;
        }
#endif /*FULL_STANDARD_COMPLIANT*/
        // no break
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        context.setFgColorAndCharset(spacing, ParserRowContext::Charset::G0);
        break;
    case 0x08:
        context.setFlash();
        break;
    case 0x0A:
        context.endBox();
        break;
    case 0x0B:
        context.startBox();
        break;
    case 0x0D:
        context.setSizeMode(ParserRowContext::SizeMode::DOUBLE_HEIGHT);
        doubleHeight = true;
        break;
    case 0x0E:
        if ((m_level == PresentationLevel::LEVEL_1)
                || (m_level == PresentationLevel::LEVEL_1_5))
        {
            // ignore
        }
        else
        {
            context.setSizeMode(ParserRowContext::SizeMode::DOUBLE_WIDTH);
        }
        break;
    case 0x0F:
        if ((m_level == PresentationLevel::LEVEL_1)
                || (m_level == PresentationLevel::LEVEL_1_5))
        {
            // ignore
        }
        else
        {
            context.setSizeMode(ParserRowContext::SizeMode::DOUBLE_SIZE);
            doubleHeight = true;
        }
        break;
    case 0x10:
#ifdef FULL_STANDARD_COMPLIANT
        if ((m_level == PresentationLevel::LEVEL_1)
                || (m_level == PresentationLevel::LEVEL_1_5))
        {
            // ignore
        }
#endif /*FULL_STANDARD_COMPLIANT*/
        // no break
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
        context.setFgColorAndCharset(spacing - 0x10,
                ParserRowContext::Charset::G1);
        break;
    case 0x1B:
        context.switchG0Charset();
        break;
    case 0x1F:
        context.releaseMosaics();
        break;
    }

    return doubleHeight;
}

const PacketLopData* Parser::getEmptyLopData() const
{
    static PacketLopData emptyLopRow;
    static bool emptyLopRowInited = false;

    if (!emptyLopRowInited)
    {
        auto data = emptyLopRow.getBuffer();
        auto length = emptyLopRow.getBufferLength();
        for (std::size_t i = 0; i < length; ++i)
        {
            data[i] = ' ';
        }

        emptyLopRowInited = true;
    }

    return &emptyLopRow;
}

} // namespace ttxdecoder
