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


#include "ColorArgb.hpp"
#include <map>
#include <iomanip>

namespace subttxrend
{
namespace gfx
{
const ColorArgb ColorArgb::TRANSPARENT(0x00, 0x00, 0x00, 0x00);
const ColorArgb ColorArgb::WHITE(0xFF, 0xFF, 0xFF, 0xFF);
const ColorArgb ColorArgb::BLACK(0xFF, 0x00, 0x00, 0x00);
const ColorArgb ColorArgb::SILVER(0xFF, 0xc0, 0xc0, 0xc0);
const ColorArgb ColorArgb::GRAY(0xFF, 0x80, 0x80, 0x80);
const ColorArgb ColorArgb::MAROON(0xFF, 0x80, 0x00, 0x00);
const ColorArgb ColorArgb::RED(0xFF, 0xFF, 0x00, 0x00);
const ColorArgb ColorArgb::PURPLE(0xFF, 0x80, 0x00, 0x80);
const ColorArgb ColorArgb::FUCHSIA(0xFF, 0xFF, 0x00, 0xFF);
const ColorArgb ColorArgb::MAGENTA(0xFF, 0xFF, 0x00, 0xFF);
const ColorArgb ColorArgb::GREEN(0xFF, 0x00, 0x80, 0x00);
const ColorArgb ColorArgb::LIME(0xFF, 0x00, 0xFF, 0x00);
const ColorArgb ColorArgb::OLIVE(0xFF, 0x80, 0x80, 0x00);
const ColorArgb ColorArgb::YELLOW(0xFF, 0xFF, 0xFF, 0x00);
const ColorArgb ColorArgb::NAVY(0xFF, 0x00, 0x00, 0x80);
const ColorArgb ColorArgb::BLUE(0xFF, 0x00, 0x00, 0xFF);
const ColorArgb ColorArgb::TEAL(0xFF, 0x00, 0x80, 0x80);
const ColorArgb ColorArgb::AQUA(0xFF, 0x00, 0xFF, 0xFF);
const ColorArgb ColorArgb::CYAN(0xFF, 0x00, 0xFF, 0xFF);

namespace
{
const std::map<std::string, gfx::ColorArgb> namedColorsMap = {
    {"TRANSPARENT", gfx::ColorArgb::TRANSPARENT},
    {"BLACK",       gfx::ColorArgb::BLACK},
    {"SILVER",      gfx::ColorArgb::SILVER},
    {"GRAY",        gfx::ColorArgb::GRAY},
    {"WHITE",       gfx::ColorArgb::WHITE},
    {"MAROON",      gfx::ColorArgb::MAROON},
    {"RED",         gfx::ColorArgb::RED},
    {"PURPLE",      gfx::ColorArgb::PURPLE},
    {"FUCHSIA",     gfx::ColorArgb::FUCHSIA},
    {"MAGENTA",     gfx::ColorArgb::MAGENTA},
    {"GREEN",       gfx::ColorArgb::GREEN},
    {"LIME",        gfx::ColorArgb::LIME},
    {"OLIVE",       gfx::ColorArgb::OLIVE},
    {"YELLOW",      gfx::ColorArgb::YELLOW},
    {"NAVY",        gfx::ColorArgb::NAVY},
    {"BLUE",        gfx::ColorArgb::BLUE},
    {"TEAL",        gfx::ColorArgb::TEAL},
    {"AQUA",        gfx::ColorArgb::AQUA},
    {"CYAN",        gfx::ColorArgb::CYAN},
};

std::string toString(const ColorArgb& color) {
    std::stringstream stream;
    stream << std::hex << std::setfill('0')
           << "(0x" << std::setw(2) << (int)color.m_a << ", "
           << "0x" << std::setw(2) << (int)color.m_r << ", "
           << "0x" << std::setw(2) << (int)color.m_g << ", "
           << "0x" << std::setw(2) << (int)color.m_b << ")";
    return stream.str();
}

}//namespace anonymous

bool ColorArgb::getColorByName(std::string name, ColorArgb& output)
{
    //Search defined color map
    auto upperCase = name;
    auto result = false;
    for (auto& c: upperCase) {
        c = toupper(c);
    }
    auto findIt = namedColorsMap.find(upperCase);
    if (findIt != namedColorsMap.end()) {
        output = findIt->second;
        result = true;
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, ColorArgb const& color)
{
    std::string colorStr;
    for (auto it = namedColorsMap.begin(); it != namedColorsMap.end(); it++) {
        if (it->second == color) {
            colorStr = it->first;
            break;
        }
    }

    return os << (colorStr.empty() ? toString(color) : colorStr);
}


} // namespace gfx
} // namespace subttxrend
