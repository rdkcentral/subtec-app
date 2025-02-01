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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#include "CcCommandParser.hpp"
#include "CcCommand.hpp"
#include "CcServiceBlock.hpp"
#include "CcExceptions.hpp"

#include <array>

//#define DUMP_BLOCKS 100

#ifdef DUMP_BLOCKS
#include <fstream>
void dumpServiceBlock(const subttxrend::cc::ServiceBlock &block)
{
    static int count = 0;

    if (count >= DUMP_BLOCKS)
    {
        return;
    }

    std::ofstream out("/tmp/service_block_" + std::to_string(count) + ".bin", std::ofstream::out);
    out.write(reinterpret_cast<const char*>(block.getSbData().data()), block.getSbData().size());

    ++count;
}
#endif // DUMP_BLOCKS

namespace subttxrend
{
namespace cc
{

CommandParser::CommandParser(std::unique_ptr<Clock> clock) :
    logger("ClosedCaptions", "CommandParser"), m_proc(nullptr), m_delayActive(false), m_clock(std::move(clock)), m_blockSize(0), m_serviceBlockSize(0), m_cmdLengthExceeds(false), m_serviceBlock()
{

}

void CommandParser::setProcessor(CommandProcessor *processor)
{
    if (!processor)
    {
        throw InvalidArgument("Processor cannot be nullptr");
    }

    m_proc = processor;
}

void CommandParser::processBlock(const ServiceBlock &block)
{
    size_t i = 0;
    const auto &data = block.getSbData();
    size_t data_len = data.size();

    while (i < data_len)
    {
        size_t consumed = 0;

        size_t (CommandParser::*fun)(const uint8_t*, size_t) = nullptr;
        if (!m_cmdLengthExceeds)
        {
            if (data[i] != C0_EXT1)
            {
                if (data[i] <= 0x1F)
                {
                    fun = &CommandParser::handleC0;
                }
                else if (data[i] >= 0x20 && data[i] <= 0x7F)
                {
                    fun = &CommandParser::handleG0;
                }
                else if (data[i] >= 0x80 && data[i] <= 0x9F)
                {
                    fun = &CommandParser::handleC1;
                }
                else if (data[i] >= 0xA0)
                {
                    fun = &CommandParser::handleG1;
                }
            }
            else // Use extended set
            {
                fun = &CommandParser::handleExtChar;
            }
        }
        else
        {
            if (m_serviceBlock[0] >= 0x80 && m_serviceBlock[0] <= 0x9F)
            {
                fun = &CommandParser::handleC1;
            }
        }
        if (fun)
        {
            if (!m_cmdLengthExceeds)
	        consumed = (this->*fun)(&data[i], data_len - i);
        }

        if (m_cmdLengthExceeds)
        {
            if ((m_serviceBlockSize + data_len) <= (2 * MAX_SERVICE_BLOCK_LENGTH))
            {
                for (size_t itr=0; itr<data_len; itr++)
                {
                    m_serviceBlock[itr+m_serviceBlockSize] = data[itr];
                }
                m_serviceBlockSize += data_len;
                consumed = (this->*fun)(&m_serviceBlock[0], m_serviceBlockSize);
            }
            else
            {
                logger.warning("Block size exceeds the limit");
                m_cmdLengthExceeds = false;
                m_serviceBlockSize = 0;
            }
        }
        else
        {
            m_serviceBlockSize = 0;
        }

        i += consumed;
    }
}

bool CommandParser::handleDelay()
{
    if (!m_delayActive)
    {
        return false;
    }


    if (m_blockSize >= MAX_BLOCKS_IN_BUFFER)
    {
        delayCancel();
        return false;
    }

    auto now = m_clock->now();
    if (now > m_timeout)
    {
        delayCancel();
        return false;
    }

    return true;
}

CommandParser::RstOrDlc CommandParser::scanForRSTDLC(const ServiceBlock &block)
{
    // find last reset or delay cancel in buffer
    const auto &data = block.getSbData();

    RstOrDlc retval;

    for (size_t i = 0; i < data.size();)
    {
        if (data[i] == C0_EXT1)
        {
            i += 2;
        }
        if (data[i] == C1_RST)
        {
            retval.rst_pos = i;
            retval.dlc_pos = i;
        }
        else if(data[i] == C1_DLC)
        {
            retval.dlc_pos = i;
        }
        i += getCommand(static_cast<CommandCode>(data[i])).length;
    }
    return retval;
}

void CommandParser::process(const ServiceBlock &block)
{
#ifdef DUMP_BLOCKS
    dumpServiceBlock(block);
#endif // DUMP_BLOCKS

    auto rstordlc = scanForRSTDLC(block);

    if (rstordlc.rst_pos >= 0)
    {
        ServiceBlock newblock(block);
        newblock.eraseTo(rstordlc.rst_pos + 1);
        m_buffer = std::queue<ServiceBlock>{};
        m_blockSize = 0;
        m_proc->reset();
        m_buffer.push(std::move(newblock));
    }
    if (rstordlc.dlc_pos >= 0)
    {
        delayCancel();
    }
    if (rstordlc.rst_pos < 0)
    {
        m_buffer.push(block);
    }

    m_blockSize += m_buffer.back().getBlockSize();

    if (handleDelay())
    {
        return;
    }

    while(!m_buffer.empty())
    {
        processBlock(m_buffer.front());
        m_blockSize -= m_buffer.front().getBlockSize();
        m_buffer.pop();
    }
}

size_t CommandParser::handleC0(const uint8_t *data, size_t data_length)
{
    auto com = getCommand(static_cast<CommandCode>(data[0]));

    logger.debug("C0: 0x%x, %s, %s, %zu", data[0], com.name, com.description, com.length);
    size_t len = 0;
    CommandCode c0 = static_cast<CommandCode>(data[0]);

    if (c0 <= 0xF)
    {
        switch (c0)
        {
            case C0_FF:
                m_proc->formFeed();
                break;
            case C0_CR:
                m_proc->carriageReturn();
                break;
            case C0_NUL:
                break;
            case C0_HCR:
                m_proc->horizontalCarriageReturn();
                break;
            case C0_ETX:
                break;
            case C0_BS:
                m_proc->backspace();
                break;
            default:
                logger.error("handleC0: unhandled branch");
                break;
        }

        len = 1;
    }
    else if (c0 >= 0x10 && c0 <= 0x17)
    {
        len = 2;
    }
    else if (c0 >= 0x18 && c0 <= 0x1F)
    {
        if (c0 == C0_P16)
        {
            handleC0P16(data + 1);
        }

        len = 3;
    }

    if (len > data_length)
    {
        return 0;
    }

    return len;
}

PenAttributes CommandParser::decodePenAttributes(const uint8_t *data)
{
    PenAttributes attrs;

    attrs.pen_size  = static_cast<PenSize>(data[1] & 0x3);
    attrs.offset    = static_cast<PenOffset>((data[1] >> 2) & 0x3);
    attrs.text_tag  = static_cast<PenTextTag>((data[1] >> 4) & 0xf);
    attrs.font_tag  = static_cast<PenFontStyle>(data[2] & 0x7);
    attrs.edge_type = static_cast<PenEdge>((data[2] >> 3) & 0x7);
    attrs.underline = (data[2] >> 6) & 0x1;
    attrs.italics   = (data[2] >> 7) & 0x1;

    return attrs;
}

PenColor CommandParser::decodePenColor(const uint8_t *data)
{
    PenColor color;

    color.fg_color = decodeColor(data[1] & 0x3f, (data[1] >> 6) & 0x03);
    color.bg_color = decodeColor(data[2] & 0x3f, (data[2] >> 6) & 0x03);
    /* There's no edge opacity in Set Pen Color command. It's inherited from
    * foreground opacity (data[1]).
    */
    color.edge_color = decodeColor(data[3] & 0x3f, (data[1] >> 6) & 0x03);

    return color;
}

WindowAttributes CommandParser::decodeWindowAttributes(const uint8_t *data)
{
    WindowAttributes attrs;

    attrs.fill_color        = decodeColor(data[1] & 0x3f, (data[1] >> 6) & 0x03);
    attrs.border_color      = decodeColor(data[2] & 0x3f);
    attrs.justify           = static_cast<WindowJustify>(data[3] & 0x03);
    attrs.scroll_direction  = static_cast<WindowSd>((data[3] >> 2) & 0x03);
    attrs.print_direction   = static_cast<WindowPd>((data[3] >> 4) & 0x03);
    attrs.word_wrap         = (data[3] >> 6) & 0x01;
    attrs.border_type       = static_cast<WindowBorder>(((data[3] >> 5) & 0x04) | ((data[2] >> 6) & 0x03));
    attrs.display_effect    = static_cast<WindowSde>(data[4] & 0x03);
    attrs.effect_direction  = static_cast<WindowEd>((data[4] >> 2) & 0x03);
    attrs.effect_speed      = (data[4] >> 4) & 0x0f;

    return attrs;
}

WindowDefinition CommandParser::decodeWindowDefinition(uint8_t id, const uint8_t *data)
{
    WindowDefinition wd;

    wd.id                = id - C1_DF0;
    wd.priority          = (data[1]) & 0x7;
    wd.col_lock          = (data[1] >> 3) & 0x1;
    wd.row_lock          = (data[1] >> 4) & 0x1;
    wd.visible           = (data[1] >> 5) & 0x1;
    wd.anchor_vertical   = data[2] & 0x7f;
    wd.relative_pos      = data[2] >> 7;
    wd.anchor_horizontal = data[3];
    wd.row_count         = (data[4] & 0xf) + 1;
    wd.anchor_point      = static_cast<PenAnchorPoint>(data[4] >> 4);
    wd.col_count         = (data[5] & 0x3f) + 1;
    wd.pen_style         = getPredefinedPenAttr(data[6] & 0x7);
    wd.win_style         = getPredefinedWinAttr((data[6] >> 3) & 0x7);

    return wd;
}

WindowAttributes CommandParser::getPredefinedWinAttr(uint8_t idx)
{
    WindowAttributes attrs;

    switch(idx)
    {
        case 2:
            setOpacity(attrs.fill_color, 0x03);
            break;
        case 3:
            attrs.justify = WindowJustify::CENTER;
            break;
        case 4:
            attrs.word_wrap = true;
            break;
        case 5:
            attrs.word_wrap = true;
            setOpacity(attrs.fill_color, 0x03);
            break;
        case 6:
            attrs.justify = WindowJustify::CENTER;
            attrs.word_wrap = true;
            break;
        case 7:
            attrs.print_direction = WindowPd::TOP_BOTTOM;
            attrs.scroll_direction = WindowSd::RIGHT_LEFT;
            break;
    }

    return attrs;
}

PenAttributes CommandParser::getPredefinedPenAttr(uint8_t idx)
{
    PenAttributes attrs;

    switch (idx)
    {
        case 2:
            attrs.font_tag = PenFontStyle::MONOSPACED_WITH_SERIFS;
            break;
        case 3:
            attrs.font_tag = PenFontStyle::PROPORTIONALLY_SPACED_WITH_SERIFS;
            break;
        case 4:
            attrs.font_tag = PenFontStyle::MONOSPACED_WITHOUT_SERIFS;
            break;
        case 5:
            attrs.font_tag = PenFontStyle::PROPORTIONALLY_SPACED_WITHOUT_SERIFS;
            break;
        case 6:
            attrs.font_tag = PenFontStyle::MONOSPACED_WITHOUT_SERIFS;
            attrs.edge_type = PenEdge::UNIFORM;
            setOpacity(attrs.pen_color.bg_color, 0x03);
            break;
        case 7:
            attrs.font_tag = PenFontStyle::PROPORTIONALLY_SPACED_WITHOUT_SERIFS;
            attrs.edge_type = PenEdge::UNIFORM;
            setOpacity(attrs.pen_color.bg_color, 0x03);
            break;
    }

    return attrs;
}

size_t CommandParser::handleC1(const uint8_t *data, size_t data_length)
{
    auto com = getCommand(static_cast<CommandCode>(data[0]));

    logger.debug("C1: 0x%x, %s, %s, %zu", data[0], com.name, com.description, com.length);

    if (com.length > data_length)
    {
        logger.debug("C1:commandlength exceed");
        m_cmdLengthExceeds = true;
        return data_length;
    }
    m_cmdLengthExceeds = false;

    switch (com.code)
    {
        case C1_CW0:
        case C1_CW1:
        case C1_CW2:
        case C1_CW3:
        case C1_CW4:
        case C1_CW5:
        case C1_CW6:
        case C1_CW7:
            m_proc->setCurrentWindow(com.code - C1_CW0);
            break;
        case C1_CLW:
            m_proc->clearWindows(toWindowsMap(data[1]));
            break;
        case C1_DSW:
            m_proc->displayWindows(toWindowsMap(data[1]));
            m_proc->resetWindowTimeout(DISPLAY_CAPTION_TIMEOUT);
            break;
        case C1_HDW:
            m_proc->hideWindows(toWindowsMap(data[1]));
            break;
        case C1_TGW:
            m_proc->toggleWindows(toWindowsMap(data[1]));
            m_proc->resetWindowTimeout(DISPLAY_CAPTION_TIMEOUT);
            break;
        case C1_DLW:
            m_proc->deleteWindows(toWindowsMap(data[1]));
            break;
        case C1_DLY:
            delay(std::chrono::milliseconds(data[1]) * 100);
            break;
        case C1_SPA:
            m_proc->setPenAttributes(decodePenAttributes(data));
            break;
        case C1_SPC:
            m_proc->setPenColor(decodePenColor(data));
            break;
        case C1_SPL:
            m_proc->setPenLocation(data[1] & 0x0f, data[2] & 0x3f);
            break;
        case C1_RSV93:
        case C1_RSV94:
        case C1_RSV95:
        case C1_RSV96:
            break;
        case C1_SWA:
            m_proc->setWindowAttributes(decodeWindowAttributes(data));
            break;
        case C1_DF0:
        case C1_DF1:
        case C1_DF2:
        case C1_DF3:
        case C1_DF4:
        case C1_DF5:
        case C1_DF6:
        case C1_DF7:
            m_proc->defineWindow(decodeWindowDefinition(com.code, data));
            if ((data[1] >> 5) & 0x1)
                m_proc->resetWindowTimeout(DISPLAY_CAPTION_TIMEOUT);
            break;
        default:
            break;
    }

    return com.length;
}

size_t CommandParser::handleC2(const uint8_t *data)
{
    auto com = getCommand(static_cast<CommandCode>(data[0]));

    logger.info("C2: 0x%x, %s, %s, %zu", data[0], com.name, com.description, com.length);

    if (data[0] <= 0x07)
    {
        return 1;
    }
    else if (data[0] <= 0x0f)
    {
        return 2;
    }
    else if (data[0] <= 0x17)
    {
        return 3;
    }

    return 4;
}

size_t CommandParser::handleC3(const uint8_t *data)
{
    auto c = data[0];
    auto com = getCommand(static_cast<CommandCode>(data[0]));

    logger.info("C3: 0x%x, %s, %s, %zu", data[0], com.name, com.description, com.length);

    if (c <= 0x87)
    {
        return 5;
    }
    else if (c <= 0x8F)
    {
        return 6;
    }
    else if (c >= 0x90 && c <= 0x9F)
    {
        size_t commandLength = (data[1] & 0x3f) + 2;
        return commandLength;
    }

    return 0;
}

size_t CommandParser::handleG0(const uint8_t *data, size_t)
{
    auto c = static_cast<char>(data[0]);

    logger.debug("G0: 0x%x %c", data[0], data[0]);
    // special case: ASCII delete becomes musical note
    // utf8 EIGHTH NOTES code
    if (c == 0x7F)
    {
        m_proc->report("\xE2\x99\xAA");
    }
    else
    {
        m_proc->report({c});
    }

    return 1;
}

size_t CommandParser::handleG1(const uint8_t *data, size_t)
{
    logger.debug("G1: 0x%x %c", data[0], data[0]);

    // convert latin1 to utf-8
    std::string str;
    uint8_t ch = static_cast<char>(data[0]);

    if (ch < 0x80) {
        str.push_back(ch);
    }
    else {
        str.push_back(0xc0 | ch >> 6);
        str.push_back(0x80 | (ch & 0x3f));
    }

    m_proc->report(str);

    return 1;
}

size_t CommandParser::handleG2G3(const uint8_t *data)
{
    logger.debug("G2G3: 0x%x", data[0]);
    struct Utf8El
    {
        uint8_t c;
        std::array<uint8_t, 4> utf8;
    };

    switch (data[0])
    {
        case 0x20:
            m_proc->transparentSpace(false);
            return 1;
        case 0x21:
            m_proc->transparentSpace(true);
            return 1;
    }

    static const std::array<Utf8El, 27> code2utf8{{
        /* G2 */
        { 0x25,     0xe2, 0x80, 0xa6       },// HORIZONTAL ELLIPSIS
        { 0x2a,     0xc5, 0xa0             },// LATIN CAPITAL LETTER S WITH CARON
        { 0x2c,     0xc5, 0x92             },// LATIN CAPITAL LIGATURE OE
        { 0x30,     0xe2, 0x96, 0x88       },// FULL BLOCK
        { 0x31,     0xe2, 0x80, 0x98       },// LEFT SINGLE QUOTATION MARK
        { 0x32,     0xe2, 0x80, 0x99       },// RIGHT SINGLE QUOTATION MARK
        { 0x33,     0xe2, 0x80, 0x9c       },// LEFT DOUBLE QUOTATION MARK
        { 0x34,     0xe2, 0x80, 0x9d       },// RIGHT DOUBLE QUOTATION MARK
        { 0x35,     0xe2, 0x80, 0xa2       },// BULLET
        { 0x39,     0xe2, 0x84, 0xa2       },// Trademark symbol (TM)
        { 0x3a,     0xc5, 0xa1             },// LATIN SMALL LETTER S WITH CARON
        { 0x3c,     0xc5, 0x93             },// LATIN SMALL LIGATURE OE
        { 0x3d,     0xe2, 0x84, 0xa0       },// SERVICE MARK
        { 0x3f,     0xc5, 0xb8             },// LATIN CAPITAL LETTER Y WITH DIAERESIS
        { 0x76,     0xe2, 0x85, 0x9b       },// VULGAR FRACTION ONE EIGHTH
        { 0x77,     0xe2, 0x85, 0x9c       },// VULGAR FRACTION THREE EIGHTHS
        { 0x78,     0xe2, 0x85, 0x9d       },// VULGAR FRACTION FIVE EIGHTHS
        { 0x79,     0xe2, 0x85, 0x9e       },// VULGAR FRACTION SEVEN EIGHTHS
        { 0x7a,     0xe2, 0x94, 0x82       },// BOX DRAWINGS LIGHT VERTICAL
        { 0x7b,     0xe2, 0x94, 0x90       },// BOX DRAWINGS LIGHT DOWN AND LEFT
        { 0x7c,     0xe2, 0x94, 0x94       },// BOX DRAWINGS LIGHT UP AND RIGHT
        { 0x7d,     0xe2, 0x94, 0x80       },// BOX DRAWINGS LIGHT HORIZONTAL
        { 0x7e,     0xe2, 0x94, 0x98       },// BOX DRAWINGS LIGHT UP AND LEFT
        { 0x7f,     0xe2, 0x94, 0x8c       },// BOX DRAWINGS LIGHT DOWN AND RIGHT
        /* G3 */
        { 0xa0,     0xf0, 0x9f, 0x85, 0xb2 },// CC (replaced with NEGATIVE SQUARED LATIN CAPITAL LETTER C)
    }};

    for(const auto &el : code2utf8)
    {
        if(el.c == data[0])
        {
            m_proc->report(std::string{reinterpret_cast<const char*>(el.utf8.data())});
            return 1;
        }
    }

    m_proc->report("_");
    return 1;
}

size_t CommandParser::handleExtChar(const uint8_t *data, size_t)
{
    uint8_t code = data[1];
    size_t used = 0;

    logger.debug("EXT: 0x%x", code);

    if (code <= 0x1F)
    {
        used = handleC2(data + 1);
    }
    else if (code >= 0x20 && code <= 0x7F)
    {
        used = handleG2G3(data + 1);
    }
    else if (code >= 0x80 && code <= 0x9F)
    {
        used = handleC3(data + 1);
    }
    else
    {
        used = handleG2G3(data + 1);
    }

    ++used; // Since we had C0_EXT1
    return used;
}

size_t CommandParser::handleC0P16(const uint8_t *data)
{
    uint8_t sym[] = {data[0], data[1], '\0'};

    logger.debug("P16 : Skipping next two bytes");
    //m_proc->report(reinterpret_cast<char*>(sym));

    return 2;
}

void CommandParser::delay(std::chrono::milliseconds timeout)
{
    m_delayActive = true;
    m_timeout = m_clock->now() + timeout;
}

void CommandParser::delayCancel()
{
    m_delayActive = false;
}

void CommandParser::resetServiceBuffer()
{
    delayCancel();
}

} // namespace cc
} // namespace subttxrend
