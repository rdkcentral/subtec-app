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
#include "CcExceptions.hpp"

#include <array>
#include <algorithm>


namespace subttxrend
{
namespace cc
{

/*
 * 608 parsing
 */


#define LOG_CURRENT_WINDOW(text)\
{\
    WindowDefinition wd;\
    if (m_proc->getWindowDefinition(wd))\
    {\
        logger.debug(text, wd.id);\
    }\
    else\
    {\
        logger.debug(text, -1);\
    }\
}


typedef enum {
    ChannelNone = 0,
    CaptionChannel1 = 1000,
    CaptionChannel2,
    CaptionChannel3,
    CaptionChannel4,
    TextChannel1,
    TextChannel2,
    TextChannel3,
    TextChannel4,
} CC608Channel;

typedef enum
{
    CaptionState_Idle,                   // Idle
    CaptionState_RollUp2,                // Roll-up 2
    CaptionState_RollUp3,                // Roll-up 3
    CaptionState_RollUp4,                // Roll-up 4
    CaptionState_PopOn1,                 // Pop-on 1
    CaptionState_PopOn2,                 // Pop-on 2
    CaptionState_PaintOn1,               // Paint-on 1
    CaptionState_PaintOn2,               // Paint-on 2
    CaptionState_Text
} CC608State_t;

uint32_t CommandParser::m_608CaptionState = CaptionState_Idle;
uint32_t CommandParser::m_requested608Channel = ChannelNone;
CcData::CcType CommandParser::m_608DataType = CcData::CcType::CEA_608_FIELD_1;
uint32_t CommandParser::m_current608Channel = ChannelNone;
uint8_t CommandParser::m_lastData1 = 0;
uint8_t CommandParser::m_lastData2 = 0;
bool CommandParser::m_isLastPairControl = false;

typedef enum
{
    CaptionControlRCL = 0x20,          // ResumeCaptionLoading
    CaptionControlBS  = 0x21,          // BackSp
    CaptionControlAOF = 0x22,          // Reserved
    CaptionControlAON = 0x23,          // Reserved
    CaptionControlDER = 0x24,          // DeleteToEndOfRow
    CaptionControlRU2 = 0x25,          // RollUp2
    CaptionControlRU3 = 0x26,          // RollUp3
    CaptionControlRU4 = 0x27,          // RollUp4
    CaptionControlFON = 0x28,          // FlashOn
    CaptionControlRDC = 0x29,          // ResumeDirectCaptioning
    CaptionControlTR  = 0x2A,          // TextRestart
    CaptionControlRTD = 0x2B,          // ResumeTextDisplay
    CaptionControlEDM = 0x2C,          // EraseDispMem
    CaptionControlCR  = 0x2D,          // CReturn
    CaptionControlENM = 0x2E,          // EraseNonDispMem
    CaptionControlEOC = 0x2F           // EndOfCaption
} CaptionControl_t ;



const int MAX_608_COLS = 32;
const int MAX_608_ROWS = 15;
const int MAX_608_WINDOWS = 8;
#define VALID_WINDOW_ID(w) ((w >= 0) && (w < MAX_608_WINDOWS))

int CommandParser::m_lastBaseRow = MAX_608_ROWS;


typedef struct
{
    int32_t baseRow;
    int32_t indent;
    int32_t assignedRow;

    void reset()
    {
        baseRow = -1;
        indent = 0;
        assignedRow = -1;
    }
} WindowInfo;

static WindowInfo windowInfo[MAX_608_WINDOWS] =
{
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1},
    {-1,0,-1}
};


typedef struct
{
    bool     windowVisible[MAX_608_WINDOWS];
    uint32_t windowRowCount[MAX_608_WINDOWS];
} WindowConfiguration;

static const WindowConfiguration windowConfigurations[] =
{
    {{0,0,0,0,0,0,0,0}, {1,1,1,1,1,1,1,1}}, /* CaptionState_Idle */
    {{1,0,0,0,0,0,0,0}, {2,1,1,1,1,1,1,1}}, /* CaptionState_RollUp2 */
    {{1,0,0,0,0,0,0,0}, {3,1,1,1,1,1,1,1}}, /* CaptionState_RollUp3 */
    {{1,0,0,0,0,0,0,0}, {4,1,1,1,1,1,1,1}}, /* CaptionState_RollUp4 */
    {{1,1,1,1,0,0,0,0}, {1,1,1,1,1,1,1,1}}, /* CaptionState_PopOn1 */
    {{0,0,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1}}, /* CaptionState_PopOn2 */
    {{1,1,1,1,0,0,0,0}, {1,1,1,1,1,1,1,1}}, /* CaptionState_PaintOn1 */
    {{0,0,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1}}, /* CaptionState_PaintOn2 */
    {{1,0,0,0,0,0,0,0}, {8,1,1,1,1,1,1,1}}  /* CaptionState_Text */
};

typedef enum {
    OPACITY_TRANSPARENT = 0x00, // Transparent
    OPACITY_SEMITRANSLUCENT = 0x5F, // Semi translucent
    OPACITY_TRANSLUCENT = 0xA0, // Translucent
    OPACITY_SOLID = 0xFF // Solid
} opacity608;

typedef enum {
    COLOUR_TRANSPARENT = 0x00000000,
    COLOUR_BLACK = 0xFF000000,
    COLOUR_WHITE = 0xFFFFFFFF,
    COLOUR_GREY = 0xFF444444,
    COLOUR_RED = 0xFFFF0000,
    COLOUR_GREEN = 0xFF00FF00,
    COLOUR_BLUE = 0xFF0000FF,
    COLOUR_YELLOW = 0xFFFFFF00,
    COLOUR_MAGENTA = 0xFFFF00FF,
    COLOUR_CYAN = 0xFF00FFFF,
    COLOUR_ORANGE = 0xFFFF8C00,
    COLOUR_LIGHT_GREY = 0xFF555555,
    COLOUR_LIGHT_RED = 0xFFFF0000,
    COLOUR_LIGHT_GREEN = 0xFF00FF00,
    COLOUR_LIGHT_BLUE = 0xFF2424FF,
    COLOUR_LIGHT_YELLOW = 0xFFFFFF33,
    COLOUR_LIGHT_MAGENTA = 0xFFFF24FF,
    COLOUR_LIGHT_CYAN = 0xFF24FFFF,
    COLOUR_DARK_GREY = 0xFF222222,
    COLOUR_DARK_RED = 0xFFFF0F0F,
    COLOUR_DARK_GREEN = 0xFF009400,
    COLOUR_DARK_BLUE = 0xFF0000E6,
    COLOUR_DARK_YELLOW = 0xFFF5F500,
    COLOUR_DARK_MAGENTA = 0xFFFF61FF,
    COLOUR_DARK_CYAN = 0xFF00A8A8,
    COLOUR_DIAG_BLUE = 0xB4000023,
    COLOUR_DIAG_DARK_BLUE = 0xB40000FF,
} colour608;

#define COLOUR_RGB(colour, opacity) ((colour & 0x00FFFFFF) + (opacity << 24))


#define MAX_COLOURS 7
#define BLACK_TEXT_STYLE (MAX_COLOURS + 1)
unsigned int colourAttributeMap[MAX_COLOURS + 1] =
{
    COLOUR_WHITE,
    COLOUR_GREEN,
    COLOUR_BLUE,
    COLOUR_CYAN,
    COLOUR_RED,
    COLOUR_YELLOW,
    COLOUR_MAGENTA
    //attribute value 7 is italics
};

// Bytes should have odd parity
bool CommandParser::check608Parity(uint8_t data)
{
    unsigned count = 0;
    for (unsigned bit = 1; bit <= 0x80; bit <<= 1)
    {
        if (data & bit)
        {
            count++;
        }
    }
    return ((count % 2) != 0);
}

void CommandParser::insert608Char(uint8_t data)
{
    uint8_t mappedCharater;

    if ( data < 0x20)
    {
        return;
    }

    // Convert special chars to iso-8859 character set
    switch ( data )
    {
        case 0x2a: mappedCharater = 0xe1; handleG1(&mappedCharater, 1); break;
        case 0x5c: mappedCharater = 0xe9; handleG1(&mappedCharater, 1); break;
        case 0x5e: mappedCharater = 0xed; handleG1(&mappedCharater, 1); break;
        case 0x5f: mappedCharater = 0xf3; handleG1(&mappedCharater, 1); break;
        case 0x60: mappedCharater = 0xfa; handleG1(&mappedCharater, 1); break;
        case 0x7b: mappedCharater = 0xe7; handleG1(&mappedCharater, 1); break;
        case 0x7c: mappedCharater = 0xf7; handleG1(&mappedCharater, 1); break;
        case 0x7d: mappedCharater = 0xd1; handleG1(&mappedCharater, 1); break;
        case 0x7e: mappedCharater = 0xf1; handleG1(&mappedCharater, 1); break;
        default:
            //logger.debug("Insert char %c", data);
            handleG0(&data, 1);
            break;
    }
}

void CommandParser::process608Char(uint8_t data)
{
    logger.trace("%s\n", __func__);

    if (active608Window() == -1)
    {
        logger.debug("%s no active window", __func__);
        return;
    }

    insert608Char(data);
}

void CommandParser::reset608CaptionState()
{
    m_608CaptionState       = CaptionState_Idle;
    m_requested608Channel   = ChannelNone;
    m_608DataType           = CcData::CcType::CEA_608_FIELD_1;
    m_current608Channel     = ChannelNone;
    m_lastData1             = 0;
    m_lastData2             = 0;

    for (auto &info : windowInfo)
    {
        info.reset();
    }

    m_608Windowlist.clear();
    m_proc->resetWindowTimeout();

    WindowsMap wm = {true, true, true, true, true, true, true, true};
    m_proc->clearWindows(wm);
    m_proc->deleteWindows(wm);
}


bool CommandParser::xdsCode(uint8_t data1)
{
    return (data1 <= 0x0f);
}

bool CommandParser::controlCode(uint8_t data1)
{
    return ((data1 >= 0x10) && (data1 <= 0x1f));
}

bool CommandParser::textChannel(uint32_t channel)
{
    return ((channel >= TextChannel1) && (channel <= TextChannel4));
}

uint8_t CommandParser::miscControlCode(uint8_t data1, uint8_t data2)
{
    if (data2 >= CaptionControlRCL && data2 <= CaptionControlEOC &&
        (m_608DataType == CcData::CcType::CEA_608_FIELD_1 || m_608DataType == CcData::CcType::CEA_608_FIELD_2) &&
        (data1 == 0x14 || data1 == 0x15 || data1 == 0x1C || data1 == 0x1D))
        return data2;

    return 0; // not a misc control code
}

uint8_t CommandParser::fieldFromChannel(uint32_t channel)
{
    if ((channel == CaptionChannel1) ||
        (channel == CaptionChannel2) ||
        (channel == TextChannel1) ||
        (channel == TextChannel2))
    {
        return CcData::CEA_608_FIELD_1;
    }
    else
    {
        return CcData::CEA_608_FIELD_2;
    }
}

uint32_t CommandParser::currentChannel(uint8_t data1, uint8_t data2)
{
    uint32_t currentChannel = m_current608Channel; // If not an appropriate control code, just maintain the current channel

    if ((m_608DataType == CcData::CEA_608_FIELD_2) &&
        xdsCode(data1))
    {
        currentChannel = ChannelNone;
    }
    else if (controlCode(data1))
    {
        bool textChannel = false;
        bool captionChannel = false;
        bool firstChannel = (( data1 & 0x08 ) == 0);

        // Start of text mode?
        uint8_t miscCode = miscControlCode(data1, data2);
        if ((miscCode == CaptionControlTR) ||
            (miscCode == CaptionControlRTD))
        {
            textChannel = true;
        }
        // else use the following control codes to identify the channel
        else if ((miscCode == CaptionControlEOC) ||
                 (miscCode == CaptionControlRCL) ||
                 (miscCode == CaptionControlRDC) ||
                 (miscCode == CaptionControlRU2) ||
                 (miscCode == CaptionControlRU3) ||
                 (miscCode == CaptionControlRU4))
        {
            // If we are in text mode, return a text channel (an 'end of text' control)
            if (m_608CaptionState == CaptionState_Text)
            {
                textChannel = true;
            }
            else
            {
                captionChannel = true;
            }
        }

        if (textChannel)
        {
            if (m_608DataType == CcData::CEA_608_FIELD_1)
            {
                currentChannel = firstChannel ? TextChannel1 : TextChannel2;
            }
            else // (m_608DataType == CcData::CEA_608_FIELD_2)
            {
                currentChannel = firstChannel ? TextChannel3 : TextChannel4;
            }
        }
        else if (captionChannel)
        {
            if (m_608DataType == CcData::CEA_608_FIELD_1)
            {
                currentChannel = firstChannel ? CaptionChannel1 : CaptionChannel2;
            }
            else // (m_608DataType == CcData::CEA_608_FIELD_2)
            {
                currentChannel = firstChannel ? CaptionChannel3 : CaptionChannel4;
            }
        }
    }

    return currentChannel;
}

void CommandParser::reset608()
{
    reset608CaptionState();
    m_proc->enable608(false);
}

void CommandParser::set608()
{
    m_proc->enable608(true);
}

void CommandParser::process608Data(uint32_t channel, CcData::CcType ccType, uint8_t data1, uint8_t data2)

{
    logger.trace("%s", __func__);

    if (channel != m_requested608Channel)
    {
        logger.info("%s reset captions on service change (%d)", __func__, channel);
        reset608CaptionState();
    }

    m_requested608Channel = channel;

    if (m_requested608Channel == ChannelNone)
    {
        logger.debug("%s no 608 channel selected", __func__);
        return;
    }

    m_608DataType = ccType;

    if ((m_608DataType != CcData::CEA_608_FIELD_1) &&
        (m_608DataType != CcData::CEA_608_FIELD_2)) // check it is CC1 or CC2 type data
    {
        logger.debug("%s not 608 data", __func__);
        return;
    }

    if (m_608DataType != fieldFromChannel(m_requested608Channel))
    {
        logger.debug("%s data is not selected field", __func__);
        return;
    }

    // Ignore any byte pairs with failed parity checks
    if ( !check608Parity(data1) || !check608Parity(data2))
    {
        logger.debug("%s data failed parity check", __func__);
        return;
    }

    // Remove parity bit 0x80
    data1 &= 0x7f;
    data2 &= 0x7f;

    // Ignore null data
    if ((data1 == 0) && (data2 == 0))
    {
        logger.debug("%s null packet", __func__);
        return;
    }

    // Check the current channel
    m_current608Channel = currentChannel(data1, data2);

    // Make sure current data matches requested channel
    if (m_requested608Channel != m_current608Channel)
    {
        logger.debug("%s not requested channel", __func__);
        return;
    }

    // Process the data
    if (controlCode(data1))
    {
        // This is a control code
        if (m_isLastPairControl && (data1 == m_lastData1) && (data2 == m_lastData2))
        {
            logger.debug("%s skip repeated control code", __func__);
            m_isLastPairControl = false;
        }
        else
        {
            m_isLastPairControl = true;
            process608control(data1, data2);
        }
    }
    else
    {
        m_isLastPairControl = false;
        if (data1 > 0x0F)
        {
            process608Char(data1);
        }
        process608Char(data2);
    }

    m_lastData1 = data1;
    m_lastData2 = data2;
}

WindowsMap CommandParser::windowIdTo608WindowMap(int window)
{
    WindowsMap map = {false, false, false, false, false, false, false, false};
    if(VALID_WINDOW_ID(window))
    {
        map[window] = true;
    }
    return map;
}

void CommandParser::define608Windows()
{
    logger.trace("%s, state %d", __func__, m_608CaptionState);

    // First process windows we want to hide
    for( int window = 0; window < 8; window++ )
    {
        if (!windowConfigurations[m_608CaptionState].windowVisible[window])
        {
            define608Window(window);
        }
    }

    // Then process windows we want to show
    for( int window = 0; window < 8; window++ )
    {
        if (windowConfigurations[m_608CaptionState].windowVisible[window])
        {
            define608Window(window);
        }
    }

    LOG_CURRENT_WINDOW("Current window after define %d");
}

void CommandParser::update608WindowDefinitionPosition(WindowDefinition &wd, uint32_t baseRow, uint32_t indent)
{
    uint32_t topRow;

    if (indent >= MAX_608_COLS)
    {
        indent = MAX_608_COLS -1;
    }
    if (baseRow > MAX_608_ROWS)
    {
        baseRow = MAX_608_ROWS;
    }

    wd.row_count = windowConfigurations[m_608CaptionState].windowRowCount[wd.id];
    topRow = baseRow - wd.row_count;

    wd.anchor_vertical = topRow;
    windowInfo[wd.id].baseRow = baseRow; // store it

    wd.anchor_horizontal = indent;
    windowInfo[wd.id].indent = indent;
}

void CommandParser::set608WindowCursor(uint32_t row, uint32_t col)
{
    m_proc->setPenLocation(row, col);
}

void CommandParser::set608WindowCursor(uint32_t row)
{
    set608WindowCursor(row, 0);
}

void CommandParser::set608WindowFill(uint32_t colour, uint32_t opacity)
{
    WindowDefinition wd;
    if (m_proc->getWindowDefinition(wd))
    {
        WindowAttributes wattrs = wd.win_style;
        wattrs.fill_color = colour & 0x00FFFFFF;
        wattrs.fill_color |= (opacity << 24);
        m_proc->setWindowAttributes(wattrs);
    }
}

void CommandParser::define608Window(int window)
{
    if(VALID_WINDOW_ID(window))
    {
        WindowDefinition wd;
        uint32_t rowCount = windowConfigurations[m_608CaptionState].windowRowCount[window];

        if (m_proc->getWindowDefinition(window, wd)) // Check if window already created
        {
            // If both old and new row count are greater than 1, it is a transition from rollup
            // to rollup. Then we need not clear the window.
            if ((wd.row_count > 1) && (rowCount > 1))
            {
                // Update it with the new row count and adjust the existing text rows
                m_proc->updateWindowRowCount(window, rowCount, true);
            }
            else if ((m_608CaptionState == CaptionState_PaintOn1) ||
                     (m_608CaptionState == CaptionState_PaintOn2))
            {
                // If we are moving to a 'paint on' state then keep the existing windows till
                // told to clear them
            }
            else if(rowCount != (uint32_t)wd.row_count)
            {
                //Clear the window
                clear608Window(window);
            }
	     m_proc->setCurrentWindow((uint8_t)window);
        }
        else
        {
            WindowAttributes wattrs; // leave as default
            PenAttributes pattrs; // leave as default

            wattrs.fill_color = COLOUR_RGB(COLOUR_BLACK, OPACITY_TRANSPARENT);
            wattrs.border_color = COLOUR_RGB(COLOUR_BLACK, OPACITY_TRANSPARENT);

            wd.id = window;
            wd.priority = 0;
            wd.col_lock = 0;
            wd.row_lock = 0;
            wd.visible = 0;
            wd.col_count = MAX_608_COLS;
            wd.row_count = rowCount;
            wd.relative_pos = 0;
            wd.anchor_point = PenAnchorPoint::TOP_LEFT;
            wd.pen_style = pattrs;
            wd.win_style = wattrs;

            update608WindowDefinitionPosition(wd, MAX_608_ROWS, 0);
            m_proc->defineWindow(wd);
        }

        // Show the window if 'visible' flag is set
        if(windowConfigurations[m_608CaptionState].windowVisible[window])
        {
            if (windowInfo[window].assignedRow != -1)
            {
                logger.debug("%s, window %d is visible", __func__, window);
                m_proc->displayWindows(windowIdTo608WindowMap(window));
            }
            else
            {
                logger.debug("%s, window %d is visible but has no row assigned", __func__, window);
            }
        }
        else
        {
            logger.debug("%s, window %d is not visible", __func__, window);

            m_proc->hideWindows(windowIdTo608WindowMap(window));
        }
    }
    else
    {
        logger.error("%s, invlaid window id", __func__);
    }
}

void CommandParser::set608Background(uint8_t colourAttribute, uint32_t opacity, bool midrow)
{
    logger.trace("%s, colour %d, opacity 0x%02X, midrow %d", __func__, colourAttribute, opacity, midrow);
    PenAttributes pattrs;

    if (m_proc->activePenAttributes(pattrs))
    {
        if (colourAttribute >= MAX_COLOURS)
        {
            pattrs.pen_color.bg_color = COLOUR_BLACK;
        }
        else
        {
            pattrs.pen_color.bg_color = colourAttributeMap[colourAttribute];
        }

        if (midrow)
        {
            insert608Char(0x20);
        }

        pattrs.pen_color.bg_color &= 0x00FFFFFF;
        pattrs.pen_color.bg_color += (opacity << 24);

        pattrs.flashing = false;
        m_proc->overridePenAttributes(pattrs, midrow);
    }
}

void CommandParser::set608StyleAttribute(uint8_t colourAttribute, bool underline, bool midrow)
{
    logger.trace("%s, colour %d, underline %d, midrow %d", __func__, colourAttribute, underline, midrow);
    PenAttributes pattrs;

    if (m_proc->activePenAttributes(pattrs))
    {
        pattrs.italics = false;

        if (colourAttribute >= BLACK_TEXT_STYLE)
        {
            pattrs.pen_color.fg_color = COLOUR_BLACK;
        }
        else if (colourAttribute == MAX_COLOURS)
        {
            pattrs.italics = true;
        }
        else
        {
            pattrs.pen_color.fg_color = colourAttributeMap[colourAttribute];
        }

        if (midrow && underline)
        {
            insert608Char(0x20);
        }

        pattrs.underline = underline;
        pattrs.flashing = false;
        m_proc->overridePenAttributes(pattrs, midrow);

        if (midrow && !underline)
        {
            insert608Char(0x20);
        }
    }
}

void CommandParser::set608Underline(bool underline)
{
    logger.trace("%s, underline %d", __func__, underline);
    PenAttributes pattrs;

    if (m_proc->activePenAttributes(pattrs))
    {
        pattrs.underline = underline;
        pattrs.flashing = false;
        m_proc->overridePenAttributes(pattrs, false);
    }
}

void CommandParser::special608Character(uint8_t code)
{
    uint8_t mappedCharater;

    switch (code)
    {
        case 0x00: mappedCharater = 0xae; handleG1(&mappedCharater, 1); break;
        case 0x01: mappedCharater = 0xb0; handleG1(&mappedCharater, 1); break;
        case 0x02: mappedCharater = 0xbd; handleG1(&mappedCharater, 1); break;
        case 0x03: mappedCharater = 0xbf; handleG1(&mappedCharater, 1); break;
        case 0x04: mappedCharater = 0x39; handleG2G3(&mappedCharater); break;
        case 0x05: mappedCharater = 0xa2; handleG1(&mappedCharater, 1); break;
        case 0x06: mappedCharater = 0xa3; handleG1(&mappedCharater, 1); break;
        case 0x07: mappedCharater = 0x7f; handleG0(&mappedCharater, 1); break;
        case 0x08: mappedCharater = 0xe0; handleG1(&mappedCharater, 1); break;
        case 0x09: mappedCharater = 0xa0; handleG1(&mappedCharater, 1); break;
        case 0x0a: mappedCharater = 0xe8; handleG1(&mappedCharater, 1); break;
        case 0x0b: mappedCharater = 0xe2; handleG1(&mappedCharater, 1); break;
        case 0x0c: mappedCharater = 0xea; handleG1(&mappedCharater, 1); break;
        case 0x0d: mappedCharater = 0xee; handleG1(&mappedCharater, 1); break;
        case 0x0e: mappedCharater = 0xf4; handleG1(&mappedCharater, 1); break;
        case 0x0f: mappedCharater = 0xfb; handleG1(&mappedCharater, 1); break;
    }
}


// Map the 608 extended character to G1
wchar_t extendedCharaterMap[] =
{
     // Table 5:  Spanish
      0x00C1,
      0x00C9,
      0x00D3,
      0x00DA,
      0x00DC,
      0x00FC,
      0x2018,
      0x00A1,

      // Table 6:  Miscellaneous
      0x002A,
      0x0027,
      0x2500,
      0x00A9,
      0x2120,
      0x2022,
      0x201C,
      0x201D,

      // Table 7:  French
      0x00C0,
      0x00C2,
      0x00C7,
      0x00C8,
      0x00CA,
      0x00CB,
      0x00EB,
      0x00CE,
      0x00CF,
      0x00EF,
      0x00D4,
      0x00D9,
      0x00F9,
      0x00DB,
      0x00AB,
      0x00BB,

      // Table 8:  Portugese
      0x00C3,
      0x00E3,
      0x00CD,
      0x00CC,
      0x00EC,
      0x00D2,
      0x00F2,
      0x00D5,
      0x00F5,
      0x007B,
      0x007D,
      0x005C,
      0x005E,
      0x005F,
      0x007C,
      0x007E,

      // Table 9:  German
      0x00C4,
      0x00E4,
      0x00D6,
      0x00F6,
      0x00DF,
      0x00A5,
      0x00A4,
      0x2502,

      // Table 10:  Danish
      0x00C5,
      0x00E5,
      0x00D8,
      0x00F8,
      0x250C,
      0x2510,
      0x2514,
      0x2518
};

void CommandParser::extended608Character(uint8_t c1, uint8_t c2)
{
    uint8_t index = ((c2 & 0x1F) + ((c1 & 0x01) << 5));
    wchar_t extendedCharater;
    uint8_t mappedCharater;

    // implied backspace
    process608Backspace();

    extendedCharater = extendedCharaterMap[index];
    if (extendedCharater > 0xFF)
    {
        // Map certain G1 characters to G2G3
        switch (extendedCharater)
        {
            case 0x2018: mappedCharater = 0x31; handleG2G3(&mappedCharater); break;
            case 0x2500: mappedCharater = 0x7d; handleG2G3(&mappedCharater); break;
            case 0x2120: mappedCharater = 0x3d; handleG2G3(&mappedCharater); break;
            case 0x2022: mappedCharater = 0x39; handleG2G3(&mappedCharater); break;
            case 0x201C: mappedCharater = 0x33; handleG2G3(&mappedCharater); break;
            case 0x201D: mappedCharater = 0x34; handleG2G3(&mappedCharater); break;
            case 0x2502: mappedCharater = 0x7a; handleG2G3(&mappedCharater); break;
            case 0x250C: mappedCharater = 0x7f; handleG2G3(&mappedCharater); break;
            case 0x2510: mappedCharater = 0x7b; handleG2G3(&mappedCharater); break;
            case 0x2514: mappedCharater = 0x7c; handleG2G3(&mappedCharater); break;
            case 0x2518: mappedCharater = 0x7e; handleG2G3(&mappedCharater); break;
        }
    }
    else
    {
        mappedCharater = (uint8_t)(extendedCharater);
        handleG1(&mappedCharater, 1);
    }
}

void CommandParser::process608TabOffset(uint8_t c2)
{
    uint8_t offset = c2 - 0x20;

    m_proc->setTabOffset(offset);
}

void CommandParser::process608PAC(uint8_t c1, uint8_t c2)
{
    logger.trace("%s", __func__);
    uint8_t  row = 1;
    uint8_t  code = (c1 & 0x07) << 1;
    code += ( c2 & 0x20 ) ? 1 : 0;

    switch (code)
    {
        case 0x02: row = 1; break;
        case 0x03: row = 2; break;
        case 0x04: row = 3; break;
        case 0x05: row = 4; break;
        case 0x06: row = 12; break;
        case 0x07: row = 13; break;
        case 0x08: row = 14; break;
        case 0x09: row = 15; break;
        case 0x0A: row = 5; break;
        case 0x0B: row = 6; break;
        case 0x0C: row = 7; break;
        case 0x0D: row = 8; break;
        case 0x0E: row = 9; break;
        case 0x0F: row = 10; break;
        default: row = 11; break;/*case 0x00: case 0x01:*/
    }

    int32_t window = get608WindowFromRow(row);
    if (window == -1)
    {
        // error, should not happen
        logger.error("%s, no window", __func__);
        return;
    }

    logger.debug("PAC row %d (set current window %d)", row, window);
    m_proc->setCurrentWindow((uint8_t)window);

    bool underline = (c2 & 0x01) != 0;
    bool setCursor = (c2 & 0x10) != 0;
    uint8_t indent = windowInfo[window].indent;

    code = ((c2 & 0x0E) >> 1); // pac code is either a style attribute or an indent size

    if (setCursor)
    {
        indent = code * 4;
        logger.debug("PAC set indent %d", indent);
    }

    if (m_608CaptionState == CaptionState_Text)
    {
        set608WindowCursor(0, indent); //TBD
//            process608CariageReturn();
    }
    else
    {
        set608WindowPosition(m_608CaptionState, row, indent);
        m_lastBaseRow = row;

//        if(( m_608CaptionState == CaptionState_RollUp2 )||
//           ( m_608CaptionState == CaptionState_RollUp3 )||
//           ( m_608CaptionState == CaptionState_RollUp4 ))
//        {
//            // RollUp Style
//            process608CariageReturn();
//        }

        // If PaintOn style, show the window after it's assigned and positioned
        if((m_608CaptionState == CaptionState_PaintOn1)||
           (m_608CaptionState == CaptionState_PaintOn2))
        {
            m_proc->displayWindows(windowIdTo608WindowMap(window));
        }
    }

    if (setCursor)
    {
        set608Underline(underline);
    }
    else if (!m_proc->hasText(row)) // ignore style if there is already text on this row
    {
        // this is a style attribute
        set608StyleAttribute(code, underline, false);
    }

    LOG_CURRENT_WINDOW("Window after PAC %d");
}

void CommandParser::process608control(uint8_t c1, uint8_t c2)
{
    logger.trace("%s", __func__);
    bool isPreambleAddressCode = (c2 & 0x40) != 0;
    bool isMidrowStyleChange = (c2 & 0x20) != 0;

    logger.debug("control 0x%02x, 0x%02x", c1, c2);

    switch(c1)
    {
        case 0x10:
        case 0x18:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if (isMidrowStyleChange)
            {
                // Not sure these are 'midrow' changes, set false so no space is inserted
                set608Background(((c2 & 0x0E) >> 1),
                        ((c2 & 0x01) ? OPACITY_TRANSLUCENT : OPACITY_SOLID),
                        false);
            }
            break;

        case 0x11:
        case 0x19:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if((c2 & 0xF0) == 0x30 )
            {
                special608Character(c2 & 0x0F);
            }
            else if (isMidrowStyleChange)
            {
                set608StyleAttribute(((c2 & 0x0E) >> 1), (c2 & 0x01), true);
            }
            break;

        case 0x12:
        case 0x1A:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if ((c2 & 0xE0)== 0x20)
            {
                extended608Character(c1, c2);
            }
            else
            {
                logger.debug("Unknown control code 0x%02x, 0x%02x", c1, c2);
            }
            break;

        case 0x13:
        case 0x1B:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if ((c2 & 0xE0)== 0x20)
            {
                extended608Character(c1, c2);
            }
            else
            {
                logger.debug("Unknown control code 0x%02x, 0x%02x", c1, c2);
            }
            break;

        case 0x14:
        case 0x1C:
        case 0x15:
        case 0x1D:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if (m_608DataType == CcData::CcType::CEA_608_FIELD_1 ||
                     m_608DataType == CcData::CcType::CEA_608_FIELD_2)
            {
                process608CaptionControl(c1, c2);
            }
            break;

        case 0x16:
        case 0x1E:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else
            {
                logger.debug("Unknown control code 0x%02x, 0x%02x", c1, c2);
            }
            break;

        case 0x17:
        case 0x1F:
            if (isPreambleAddressCode)
            {
                process608PAC(c1, c2);
            }
            else if ((c2 >= 0x21) && (c2 <= 0x23))
            {
                process608TabOffset(c2);
            }
            else if (isMidrowStyleChange)
            {
                // Not sure these are 'midrow' changes, set false so no space is inserted
                if (c2 & 0x02)
                {
                    set608StyleAttribute( BLACK_TEXT_STYLE, (c2 & 0x01), false);
                }
                else
                {
                    set608Background( MAX_COLOURS, OPACITY_TRANSPARENT, false); //black transparent background
                }
            }
            else
            {
                logger.debug("Unknown control code 0x%02x, 0x%02x", c1, c2);
            }
            break;

        default:
            logger.debug("Unknown control code 0x%02x, 0x%02x", c1, c2);
            break;
    }
}

void CommandParser::set608timeout(int timeout)
{
    m_proc->resetWindowTimeout(timeout);
}

void CommandParser::erase608DisplayedMemory(void)
{
    for ( uint32_t window = 0; window < MAX_608_WINDOWS; window++ )
    {
        if ( windowConfigurations[m_608CaptionState].windowVisible[window] )
        {
            clear608Window(window);
            removeFrom608WindowList(window);
        }
    }
}

void CommandParser::removeFrom608WindowList(int window)
{
    std::vector<int>::iterator iter = std::find(m_608Windowlist.begin(), m_608Windowlist.end(), window);
    if (iter != m_608Windowlist.end())
    {
        m_608Windowlist.erase(iter);
    }
}

int CommandParser::active608Window()
{
    WindowDefinition wd;

    if (!m_proc->getWindowDefinition(wd))
    {
        return -1;
    }
    else if (windowInfo[wd.id].assignedRow == -1)
    {
        return -1;
    }
    else
    {
        return wd.id;
    }
}

int32_t CommandParser::get608WindowFromRow(int32_t row)
{
    int32_t window;
    int32_t offset;

    if ( m_608CaptionState == CaptionState_Idle )
    {
        return -1;
    }

    if(( m_608CaptionState == CaptionState_RollUp2 )||
        ( m_608CaptionState == CaptionState_RollUp3 )||
        ( m_608CaptionState == CaptionState_RollUp4 )||
        ( m_608CaptionState == CaptionState_Text ))
    {
        windowInfo[0].assignedRow = row;
        return 0;
    }

    if ( m_608CaptionState == CaptionState_PopOn2 || m_608CaptionState == CaptionState_PaintOn1)
    {
        offset = 0 ;
    }
    else
    {
        offset = 4 ;
    }

    logger.debug("%s, row assignments: %d, %d, %d, %d, %d, %d, %d, %d", __func__,
        windowInfo[0].assignedRow,
        windowInfo[1].assignedRow,
        windowInfo[2].assignedRow,
        windowInfo[3].assignedRow,
        windowInfo[4].assignedRow,
        windowInfo[5].assignedRow,
        windowInfo[6].assignedRow,
        windowInfo[7].assignedRow);

    // Find window assigned to this row
    for ( window = offset ; window < offset + 4 ; window++ )
    {
        if ( windowInfo[window].assignedRow == row )
        {
            // already have a window assigned to this row
            removeFrom608WindowList(window);
            m_608Windowlist.emplace_back(window);
            return window;
        }
    }

    // Find an unassigned window
    for ( window = offset; window < offset + 4; window++ )
    {
        if ( windowInfo[window].assignedRow == -1 )
        {
            windowInfo[window].assignedRow = row ;
            m_608Windowlist.emplace_back(window);
            return window;
        }
    }

    // Replace oldest assigned window
    window = m_608Windowlist[0];
    m_608Windowlist.erase(m_608Windowlist.begin());
    m_608Windowlist.emplace_back(window);

    windowInfo[window].assignedRow = row;
    return window;
}

void CommandParser::clear608Window(uint32_t window)
{
    m_proc->clearWindows(windowIdTo608WindowMap(window));
    m_proc->updateWindowRowCount(window, windowConfigurations[m_608CaptionState].windowRowCount[window], false);
    windowInfo[window].reset();
}

void CommandParser::erase608NonDisplayedMemory(void)
{
    for ( uint32_t window = 0; window < MAX_608_WINDOWS; window++ )
    {
        if ( !windowConfigurations[m_608CaptionState].windowVisible[window] )
        {
            clear608Window(window);
            removeFrom608WindowList(window);
        }
    }
}

void CommandParser::set608WindowPosition(int state, unsigned char baseRow, unsigned char column)
{
    WindowDefinition wd;
    uint32_t cursorRow = 0;
    uint32_t windowColumn = 0;
    uint32_t cursorColumn = 0;

    if (!m_proc->getWindowDefinition(wd))
    {
        logger.warning("%s, set position on undefined window", __func__);
        return;
    }

    switch(state)
    {
        case CaptionState_RollUp2:
            if(baseRow < 2 )
            {
                baseRow = 2;
            }
            cursorRow = 1;
            cursorColumn = column;
            break;

        case CaptionState_RollUp3:
            if(baseRow < 3 )
            {
                baseRow = 3;
            }
            cursorRow = 2;
            cursorColumn = column;
            break;

        case CaptionState_RollUp4:
            if(baseRow < 4 )
            {
                baseRow = 4;
            }
            cursorRow = 3;
            cursorColumn = column;
            break;

        case CaptionState_Text:
            if(baseRow < 8)
            {
                baseRow = 8;
            }
            cursorRow = 0;
            cursorColumn = column;
            break;

        case CaptionState_PaintOn1:
        case CaptionState_PaintOn2:
            if (m_proc->hasText(cursorRow))
            {
                set608WindowCursor(wd.row_count - 1, column);
                return;
            }
            /* fall through */
        default:
            cursorRow = 0;
            cursorColumn = column;
//            windowColumn = column;
            break;
    }

    update608WindowDefinitionPosition(wd, baseRow, windowColumn);
    m_proc->defineWindow(wd);

    set608WindowCursor(cursorRow, cursorColumn);

    if (state == CaptionState_Text)
    {
        set608WindowFill(COLOUR_BLACK, OPACITY_SOLID);
        m_proc->clearWindow();
    }
}

void CommandParser::process608CaptionControl(uint8_t c1, uint8_t control)
{
    uint32_t last608CaptionState = m_608CaptionState;
    bool windowTimedout = m_proc->isWindowTimedout();
    bool defineWindowOnTimeout = false;

    LOG_CURRENT_WINDOW("Window for control %d");

    switch (control)
    {
        case CaptionControlRCL:
            logger.debug("%s, RCL", __func__);
            process608ResumeCaptionLoading();
            break;
        case CaptionControlBS:
            logger.debug("%s, BS", __func__);
            process608Backspace();
            break;
        case CaptionControlAOF:
            logger.debug("%s, AOF", __func__);
            process608AlarmOn();
            break;
        case CaptionControlAON:
            logger.debug("%s, AON", __func__);
            process608AlarmOff();
            break;
        case CaptionControlDER:
            logger.debug("%s, DER", __func__);
            process608DeleteToEndOfRow();
            break;
        case CaptionControlRU2:
        case CaptionControlRU3:
        case CaptionControlRU4:
            logger.debug("%s, ROLLUP %d", __func__, control);
            process608Rollup(control);
            defineWindowOnTimeout = true;
            break;
        case CaptionControlFON:
            logger.debug("%s, FON", __func__);
            process608FlashOn();
            break;
        case CaptionControlRDC:
            logger.debug("%s, RDC", __func__);
            process608ResumeDirectCaptioning();
            break;
        case CaptionControlTR:
            logger.debug("%s, TR", __func__);
            process608TextRestart();
            break;
        case CaptionControlRTD:
            logger.debug("%s, RTD", __func__);
            process608ResumeTextDisplay();
            break;
        case CaptionControlEDM:
            logger.debug("%s, EDM", __func__);
            process608Erase608DisplayedMemory();
            break;
        case CaptionControlCR:
            logger.debug("%s, CR", __func__);
            process608CariageReturn();
            break;
        case CaptionControlENM:
            logger.debug("%s, ENM", __func__);
            process608Erase608NonDisplayedMemory();
            break;
        case CaptionControlEOC:
            logger.debug("%s, EOC", __func__);
            process608EndOfCaption();
            break;

        default:
            logger.warning("%s, Unrecognised control code 0x%02X", __func__, control);
            break;
    }

    if ((last608CaptionState != m_608CaptionState) || (windowTimedout && defineWindowOnTimeout))
    {
        m_608Windowlist.clear();

        // Clear the windows if required
        if((control != CaptionControlEOC) &&
           (control != CaptionControlRCL )&&
           (control != CaptionControlRDC))
        {
            clear608Windows();
        }

        // Define the new windows necessary for this state
        define608Windows();

        // Set Current Window to 0 if its a rollup state*/
        if((m_608CaptionState == CaptionState_RollUp2) ||
           (m_608CaptionState == CaptionState_RollUp3) ||
           (m_608CaptionState == CaptionState_RollUp4) ||
           (m_608CaptionState == CaptionState_Text))
        {
            m_proc->setCurrentWindow(0);

            const int row_count = [&]{
                WindowDefinition wd;
                if (!m_proc->getWindowDefinition(wd))
                {
                    logger.warning("%s, cannot obtain row count - defaulting base row to %d", __func__, MAX_608_ROWS);
                    return MAX_608_ROWS;
                }
                else
                {
                    return wd.row_count;
                }
            }();

            if (windowTimedout && defineWindowOnTimeout)
                set608WindowPosition(m_608CaptionState, m_lastBaseRow, 0);
            else
                set608WindowPosition(m_608CaptionState, row_count, 0);
            m_proc->displayWindows( windowIdTo608WindowMap(0));

        }
    }

    set608timeout(m_608CaptionState == CaptionState_Text ? 0 : DISPLAY_CAPTION_TIMEOUT);
}

void CommandParser::process608ResumeCaptionLoading()
{
    if ((m_608CaptionState == CaptionState_PopOn1) ||
        (m_608CaptionState == CaptionState_PopOn2))
    {
        // nothing to do
    }
    else if (m_608CaptionState == CaptionState_PaintOn2)
    {
        m_608CaptionState = CaptionState_PopOn2;
    }
    else
    {
        m_608CaptionState = CaptionState_PopOn1;
    }
}

void CommandParser::process608Backspace()
{
    if (m_608CaptionState != CaptionState_Idle)
    {
        m_proc->backspace();
    }
}

void CommandParser::process608AlarmOn()
{
    //TBD
}

void CommandParser::process608AlarmOff()
{
    //TBD
}

void CommandParser::process608DeleteToEndOfRow()
{
    if (m_608CaptionState != CaptionState_Idle)
    {
        //TBD ProcessDeleteToEndOfRow();
    }
}

void CommandParser::process608Rollup(uint8_t control)
{
    if ((m_608CaptionState != CaptionState_RollUp2) &&
        (m_608CaptionState != CaptionState_RollUp3) &&
        (m_608CaptionState != CaptionState_RollUp4))
    {
        erase608DisplayedMemory();
        erase608NonDisplayedMemory();
    }

    switch (control)
    {
        case CaptionControlRU2:
            m_608CaptionState = CaptionState_RollUp2;
            break;
        case CaptionControlRU3:
            m_608CaptionState = CaptionState_RollUp3;
            break;
        case CaptionControlRU4:
        default:
            m_608CaptionState = CaptionState_RollUp4;
            break;
    }
}

void CommandParser::process608FlashOn()
{
    // This will draw flashing text until the next style (or row) is processed
    // (Should it just be until the next style?)
    logger.trace("%s", __func__);
    PenAttributes pattrs;

    if (m_proc->activePenAttributes(pattrs))
    {
        pattrs.flashing = true;
        m_proc->overridePenAttributes(pattrs, false);
    }
}

void CommandParser::process608ResumeDirectCaptioning()
{
//    if ((m_608CaptionState == CaptionState_RollUp2) ||
//        (m_608CaptionState == CaptionState_RollUp3) ||
//        (m_608CaptionState == CaptionState_RollUp4) ||
//        (m_608CaptionState == CaptionState_PopOn2) ||
//        (m_608CaptionState == CaptionState_PaintOn2) ||
//        (m_608CaptionState == CaptionState_Text))
    if ((m_608CaptionState == CaptionState_PopOn2) ||
        (m_608CaptionState == CaptionState_PaintOn2))
    {
        m_608CaptionState = CaptionState_PaintOn2;
    }
    else
    {
        m_608CaptionState = CaptionState_PaintOn1;
    }
}

void CommandParser::process608TextRestart()
{
    if (m_608CaptionState == CaptionState_Text)
    {
        set608WindowCursor(0, 0);
    }

    erase608DisplayedMemory();
    m_608CaptionState = CaptionState_Text;
}

void CommandParser::process608ResumeTextDisplay()
{
    m_608CaptionState = CaptionState_Text;
}

void CommandParser::process608Erase608DisplayedMemory()
{
    erase608DisplayedMemory();
}

void CommandParser::process608CariageReturn()
{
    if ((m_608CaptionState == CaptionState_RollUp2) ||
        (m_608CaptionState == CaptionState_RollUp3) ||
        (m_608CaptionState == CaptionState_RollUp4) ||
        (m_608CaptionState == CaptionState_Text))
    {
        m_proc->carriageReturn();
    }
}

void CommandParser::process608Erase608NonDisplayedMemory()
{
    erase608NonDisplayedMemory();
}

void CommandParser::process608EndOfCaption()
{
    if ((m_608CaptionState == CaptionState_RollUp2) ||
        (m_608CaptionState == CaptionState_RollUp3) ||
        (m_608CaptionState == CaptionState_RollUp4) ||
        (m_608CaptionState == CaptionState_PopOn1) ||
        (m_608CaptionState == CaptionState_PaintOn1))
    {
        m_608CaptionState = CaptionState_PopOn2;
    }
    else
    {
        m_608CaptionState = CaptionState_PopOn1;
    }
}

void CommandParser::clear608Windows(void)
{
    bool clear = false;

    for ( uint32_t window = 0; window < MAX_608_WINDOWS; window++ )
    {
        if ((m_608CaptionState == CaptionState_Idle) ||
            (m_608CaptionState == CaptionState_Text))
        {
            clear = true;
        }
        else if ((m_608CaptionState == CaptionState_RollUp2) ||
                 (m_608CaptionState == CaptionState_RollUp3) ||
                 (m_608CaptionState == CaptionState_RollUp4) ||
                 (m_608CaptionState == CaptionState_PaintOn1) ||
                 (m_608CaptionState == CaptionState_PaintOn2))
        {
            clear = false;
        }
        else
        {
            clear = windowConfigurations[m_608CaptionState].windowVisible[window];
        }

        if ( clear )
        {
            clear608Window(window);
            removeFrom608WindowList(window);
        }
    }
}

} // namespace cc
} // namespace subttxrend

