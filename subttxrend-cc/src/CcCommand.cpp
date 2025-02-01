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

#include "CcCommand.hpp"

#include <unordered_map>

namespace std
{
    template <> struct hash<subttxrend::cc::CommandCode>
    {
        size_t operator()(subttxrend::cc::CommandCode x) const
        {
            return hash<int>()(x);
        }
    };
}

namespace subttxrend
{
namespace cc
{

static const std::unordered_map<CommandCode, Command> COMMANDS_MAP =
{
    {C0_NUL,   {C0_NUL,   "NUL",   "NULL",                 1}},
    {C0_ETX,   {C0_ETX,   "ETX",   "End of text",          1}},
    {C0_BS ,   {C0_BS ,   "BS",    "Backspace",            1}},
    {C0_FF ,   {C0_FF ,   "FF",    "Form feed",            1}},
    {C0_CR ,   {C0_CR ,   "CR",    "Carriage return",      1}},
    {C0_HCR,   {C0_HCR,   "HCR",   "Hor. carriage return", 1}},
    {C0_EXT1,  {C0_EXT1,  "EXT1",  "Extended table",       1}},
    {C0_P16,   {C0_P16,   "P16",   "Two byte char",        1}},
    {C1_CW0,   {C1_CW0,   "CW0",   "SetCurrentWindow0",    1}},
    {C1_CW1,   {C1_CW1,   "CW1",   "SetCurrentWindow1",    1}},
    {C1_CW2,   {C1_CW2,   "CW2",   "SetCurrentWindow2",    1}},
    {C1_CW3,   {C1_CW3,   "CW3",   "SetCurrentWindow3",    1}},
    {C1_CW4,   {C1_CW4,   "CW4",   "SetCurrentWindow4",    1}},
    {C1_CW5,   {C1_CW5,   "CW5",   "SetCurrentWindow5",    1}},
    {C1_CW6,   {C1_CW6,   "CW6",   "SetCurrentWindow6",    1}},
    {C1_CW7,   {C1_CW7,   "CW7",   "SetCurrentWindow7",    1}},
    {C1_CLW,   {C1_CLW,   "CLW",   "ClearWindows",         2}},
    {C1_DSW,   {C1_DSW,   "DSW",   "DisplayWindows",       2}},
    {C1_HDW,   {C1_HDW,   "HDW",   "HideWindows",          2}},
    {C1_TGW,   {C1_TGW,   "TGW",   "ToggleWindows",        2}},
    {C1_DLW,   {C1_DLW,   "DLW",   "DeleteWindows",        2}},
    {C1_DLY,   {C1_DLY,   "DLY",   "Delay",                2}},
    {C1_DLC,   {C1_DLC,   "DLC",   "DelayCancel",          1}},
    {C1_RST,   {C1_RST,   "RST",   "Reset",                1}},
    {C1_SPA,   {C1_SPA,   "SPA",   "SetPenAttributes",     3}},
    {C1_SPC,   {C1_SPC,   "SPC",   "SetPenColor",          4}},
    {C1_SPL,   {C1_SPL,   "SPL",   "SetPenLocation",       3}},
    {C1_RSV93, {C1_RSV93, "RSV93", "Reserved",             1}},
    {C1_RSV94, {C1_RSV94, "RSV94", "Reserved",             1}},
    {C1_RSV95, {C1_RSV95, "RSV95", "Reserved",             1}},
    {C1_RSV96, {C1_RSV96, "RSV96", "Reserved",             1}},
    {C1_SWA,   {C1_SWA,   "SWA",   "SetWindowAttributes",  5}},
    {C1_DF0,   {C1_DF0,   "DF0",   "DefineWindow0",        7}},
    {C1_DF1,   {C1_DF1,   "DF1",   "DefineWindow1",        7}},
    {C1_DF2,   {C1_DF2,   "DF2",   "DefineWindow2",        7}},
    {C1_DF3,   {C1_DF3,   "DF3",   "DefineWindow3",        7}},
    {C1_DF4,   {C1_DF4,   "DF4",   "DefineWindow4",        7}},
    {C1_DF5,   {C1_DF5,   "DF5",   "DefineWindow5",        7}},
    {C1_DF6,   {C1_DF6,   "DF6",   "DefineWindow6",        7}},
    {C1_DF7,   {C1_DF7,   "DF7",   "DefineWindow7",        7}},
};

Command getCommand(CommandCode code)
{
    auto pos = COMMANDS_MAP.find(code);

    if (pos != COMMANDS_MAP.end())
    {
        return pos->second;
    }

    return {CX_INVALID, "Reserved", "Reserved", 1};
}

WindowsMap toWindowsMap(uint8_t wm)
{
    WindowsMap retval;

    for(int i = 0; i < MAX_WINDOWS; ++i)
    {
        retval[i] = wm & 0x1;
        wm >>= 1;
    }

    return retval;
}

void setOpacity(uint32_t &color, uint8_t opacity)
{
    color &= 0x00ffffff;

    uint8_t alpha = 0;
    switch (static_cast<Opacity>(opacity))
    {
        case Opacity::SOLID:
            alpha = 255;
            break;
        case Opacity::FLASH:
            alpha = 250;
            break;
        case Opacity::TRANSLUCENT:
            alpha = 100;
            break;
        case Opacity::TRANSPARENT:
            alpha = 0;
            break;
    }

    color |= (static_cast<uint32_t>(alpha) << 24);
}

uint32_t decodeColor(uint32_t rgb, uint8_t opacity)
{
    uint32_t retval;

    uint32_t red = ((rgb >> 4) & 3)/3.0 * 255;
    uint32_t green = ((rgb >> 2) & 3)/3.0 * 255;
    uint32_t blue = ((rgb >> 0) & 3)/3.0 * 255;
    retval = (red << 16) | (green << 8) | (blue << 0);
    setOpacity(retval, opacity);

    return retval;
}

} // namespace cc
} // namespace subttxrend
