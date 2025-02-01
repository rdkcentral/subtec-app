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

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace subttxrend
{
namespace cc
{

const int MAX_WINDOWS = 8;

using Color = uint32_t;

enum CommandCode
{
    CX_INVALID =   -1,
    C0_NUL     = 0x00,
    C0_ETX     = 0x03,
    C0_BS      = 0x08,
    C0_FF      = 0x0c,
    C0_CR      = 0x0d,
    C0_HCR     = 0x0e,
    C0_EXT1    = 0x10,
    C0_P16     = 0x18,
    C1_CW0     = 0x80,
    C1_CW1     = 0x81,
    C1_CW2     = 0x82,
    C1_CW3     = 0x83,
    C1_CW4     = 0x84,
    C1_CW5     = 0x85,
    C1_CW6     = 0x86,
    C1_CW7     = 0x87,
    C1_CLW     = 0x88,
    C1_DSW     = 0x89,
    C1_HDW     = 0x8A,
    C1_TGW     = 0x8B,
    C1_DLW     = 0x8C,
    C1_DLY     = 0x8D,
    C1_DLC     = 0x8E,
    C1_RST     = 0x8F,
    C1_SPA     = 0x90,
    C1_SPC     = 0x91,
    C1_SPL     = 0x92,
    C1_RSV93   = 0x93,
    C1_RSV94   = 0x94,
    C1_RSV95   = 0x95,
    C1_RSV96   = 0x96,
    C1_SWA     = 0x97,
    C1_DF0     = 0x98,
    C1_DF1     = 0x99,
    C1_DF2     = 0x9A,
    C1_DF3     = 0x9B,
    C1_DF4     = 0x9C,
    C1_DF5     = 0x9D,
    C1_DF6     = 0x9E,
    C1_DF7     = 0x9F,
};

struct Command
{
    Command(CommandCode code, const char *name, const char *description, size_t length) :
        code(code), name(name), description(description), length(length)
    {}
    const CommandCode code;
    const char * const name;
    const char * const description;
    const size_t length;
};

enum class WindowJustify
{
    LEFT,
    RIGHT,
    CENTER,
    FULL,
    DEFAULT = LEFT,
};

enum class WindowPd //Print Direction
{
    LEFT_RIGHT,
    RIGHT_LEFT,
    TOP_BOTTOM,
    BOTTOM_TOP,
    DEFAULT = LEFT_RIGHT,
};

enum class WindowSd //Scroll Direction
{
    LEFT_RIGHT,
    RIGHT_LEFT,
    TOP_BOTTOM,
    BOTTOM_TOP,
    DEFAULT = BOTTOM_TOP,
};

enum class WindowSde //Scroll Display Effect
{
    SNAP,
    FADE,
    WIPE,
    DEFAULT = SNAP,
};

enum class WindowEd //Effect Direction
{
    LEFT_RIGHT,
    RIGHT_LEFT,
    TOP_BOTTOM,
    BOTTOM_TOP,
    DEFAULT = LEFT_RIGHT,
};

enum class WindowBorder
{
    NONE,
    RAISED,
    DEPRESSED,
    UNIFORM,
    SHADOW_LEFT,
    SHADOW_RIGHT,
    DEFAULT = NONE,
};

enum class PenSize
{
    SMALL,
    STANDARD,
    LARGE,
    EXTRALARGE,
    DEFAULT = STANDARD,
};

enum class PenFontStyle
{
    DEFAULT_OR_UNDEFINED,
    MONOSPACED_WITH_SERIFS,
    PROPORTIONALLY_SPACED_WITH_SERIFS,
    MONOSPACED_WITHOUT_SERIFS,
    PROPORTIONALLY_SPACED_WITHOUT_SERIFS,
    CASUAL_FONT_TYPE,
    CURSIVE_FONT_TYPE,
    SMALL_CAPITALS,
    DEFAULT = DEFAULT_OR_UNDEFINED,
};

enum class PenTextTag
{
    DIALOG,
    SOURCE_OR_SPEAKER_ID,
    ELECTRONIC_VOICE,
    FOREIGN_LANGUAGE,
    VOICEOVER,
    AUDIBLE_TRANSLATION,
    SUBTITLE_TRANSLATION,
    VOICE_QUALITY_DESCRIPTION,
    SONG_LYRICS,
    SOUND_EFFECT_DESCRIPTION,
    MUSICAL_SCORE_DESCRIPTION,
    EXPLETIVE,
    UNDEFINED_0,
    UNDEFINED_1,
    UNDEFINED_2,
    NOT_TO_BE_DISPLAYED,
    DEFAULT = UNDEFINED_0,
};

enum class PenOffset
{
    SUBSCRIPT,
    NORMAL,
    SUPERSCRIPT,
    DEFAULT = NORMAL,
};

enum class PenEdge
{
    NONE,
    RAISED,
    DEPRESSED,
    UNIFORM,
    LEFT_DROP_SHADOW,
    RIGHT_DROP_SHADOW,
    DEFAULT = NONE,
};

enum class PenAnchorPoint
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
    DEFAULT = TOP_LEFT,
};

enum class Opacity
{
    SOLID,
    FLASH,
    TRANSLUCENT,
    TRANSPARENT,
    DEFAULT = SOLID,
};

enum class FlashControl
{
    Show,
    Hide
};


constexpr uint8_t COLOR_BLACK = 0x00;
constexpr uint8_t COLOR_WHITE = 0x3f;
uint32_t decodeColor(uint32_t rgb, uint8_t opacity = 0x00);

struct PenColor
{
    PenColor() :
        fg_color(decodeColor(COLOR_WHITE)),
        bg_color(decodeColor(COLOR_BLACK)),
        edge_color(decodeColor(COLOR_WHITE))
    {}

    bool operator==(const PenColor &b) const
    {
        return (
                fg_color == b.fg_color &&
                bg_color == b.bg_color &&
                edge_color == b.edge_color);
    }

    Color fg_color;
    Color bg_color;
    Color edge_color;
};

struct PenAttributes
{
    PenAttributes() :
        pen_size(PenSize::DEFAULT),
        offset(PenOffset::DEFAULT),
        text_tag(PenTextTag::DEFAULT),
        font_tag(PenFontStyle::DEFAULT),
        edge_type(PenEdge::DEFAULT),
        underline(false),
        italics(false),
        flashing(false)
    {}
    PenAttributes& operator=(const PenAttributes &o) = default;

    bool operator==(const PenAttributes &b) const
    {
        return (
                pen_size == b.pen_size &&
                offset == b.offset &&
                text_tag == b.text_tag &&
                font_tag == b.font_tag &&
                edge_type == b.edge_type &&
                underline == b.underline &&
                italics == b.italics &&
                flashing == b.flashing &&
                pen_color == b.pen_color);
    }

    bool operator!=(const PenAttributes &b) const
    {
        return !(*this == b);
    }

    PenSize pen_size;
    PenOffset offset;
    PenTextTag text_tag;
    PenFontStyle font_tag;
    PenEdge edge_type;
    bool underline;
    bool italics;
    PenColor pen_color;
    bool flashing;
};

struct WindowAttributes
{
    WindowAttributes() :
        justify(WindowJustify::DEFAULT),
        print_direction(WindowPd::DEFAULT),
        scroll_direction(WindowSd::DEFAULT),
        word_wrap(false),
        display_effect(WindowSde::DEFAULT),
        effect_direction(WindowEd::DEFAULT),
        effect_speed(0),
        fill_color(decodeColor(COLOR_BLACK)),
        border_type(WindowBorder::DEFAULT),
        border_color(decodeColor(COLOR_BLACK))
    {}

    bool operator==(const WindowAttributes &b) const
    {
        return (
                justify == b.justify &&
                print_direction == b.print_direction &&
                scroll_direction == b.scroll_direction &&
                word_wrap == b.word_wrap &&
                display_effect == b.display_effect &&
                effect_direction == b.effect_direction &&
                effect_speed == b.effect_speed &&
                fill_color == b.fill_color &&
                border_type == b.border_type &&
                border_color == b.border_color);
    }

    bool operator!=(const WindowAttributes &b) const
    {
        return !(*this == b);
    }

    WindowJustify justify;
    WindowPd print_direction;
    WindowSd scroll_direction;
    bool word_wrap;
    WindowSde display_effect;
    WindowEd effect_direction;
    uint8_t effect_speed;
    Color fill_color;
    WindowBorder border_type;
    Color border_color;
};

struct WindowDefinition
{
    int id;
    int priority;
    bool col_lock;
    bool row_lock;
    bool visible;
    bool relative_pos;
    int anchor_vertical;
    int anchor_horizontal;
    int row_count;
    PenAnchorPoint anchor_point;
    int col_count;
    PenAttributes pen_style;
    WindowAttributes win_style;

    bool operator==(const WindowDefinition &b) const
    {
        return (
                (id == b.id) &&
                (priority == b.priority) &&
                (col_lock == b.col_lock) &&
                (row_lock == b.row_lock) &&
                (visible == b.visible) &&
                (relative_pos == b.relative_pos) &&
                (anchor_vertical == b.anchor_vertical) &&
                (anchor_horizontal == b.anchor_horizontal) &&
                (row_count == b.row_count) &&
                (anchor_point == b.anchor_point) &&
                (col_count == b.col_count) &&
                (pen_style == b.pen_style) &&
                (win_style == b.win_style));
    }

    bool operator!=(const WindowDefinition &b) const
    {
        return !(*this == b);
    }
};

Command getCommand(CommandCode code);
using WindowsMap = std::array<bool, MAX_WINDOWS>;
WindowsMap toWindowsMap(uint8_t wm);
void setOpacity(uint32_t &color, uint8_t opacity);

} // namespace cc
} // namespace subttxrend
