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


#include "GfxTtxClut.hpp"

namespace subttxrend
{
namespace ttxt
{

namespace
{

/**
 * Standard color lookup table.
 */
const std::array<std::uint32_t, 32> STD_CLUT =
{
        0xFF000000,    // BLACK
        0xFFFF0000,    // RED
        0xFF00FF00,    // GREEN
        0xFFFFFF00,    // YELLOW
        0xFF0000FF,    // BLUE
        0xFFFF00FF,    // MAGENTA
        0xFF00FFFF,    // CYAN
        0xFFFFF0FF,    // WHITE

        0x00000000,    // TRANSPARENT
        0xFF770000,    // HALF_RED
        0xFF007700,    // HALF_GREEN
        0xFF777700,    // HALF_YELLOW
        0xFF000077,    // HALF_BLUE
        0xFF770077,    // HALF_MAGENTA
        0xFF007777,    // HALF_CYAN
        0xFF777777,    // GREY

        0xFFFF0055,    // CLUT2_0
        0xFFFF7700,    // CLUT2_1
        0xFF00FF77,    // CLUT2_2
        0xFFFFFFBB,    // CLUT2_3
        0xFF00CCAA,    // CLUT2_4
        0xFF550000,    // CLUT2_5
        0xFF665522,    // CLUT2_6
        0xFFCC7777,    // CLUT2_7

        0xFF333333,    // CLUT3_0
        0xFFFF7777,    // CLUT3_1
        0xFF77FF77,    // CLUT3_2
        0xFFFFFF77,    // CLUT3_3
        0xFF7777FF,    // CLUT3_4
        0xFFFF77FF,    // CLUT3_5
        0xFF77FFFF,    // CLUT3_6
        0xFFDDDDDD     // CLUT3_7
        };

}
GfxTtxClut::GfxTtxClut()
{
    m_clut.fill(0);
}

void GfxTtxClut::resetColors()
{
    m_clut = STD_CLUT;
}

bool GfxTtxClut::setColor(std::size_t index,
                          std::uint32_t color)
{
    if (index < m_clut.size())
    {
        if (m_clut[index] != color)
        {
            m_clut[index] = color;
            return true;
        }
    }
    return false;
}

const std::uint32_t* GfxTtxClut::getArray() const
{
    return m_clut.data();
}

std::size_t GfxTtxClut::getSize() const
{
    return m_clut.size();
}

} // namespace ttxt
} // namespace subttxrend
