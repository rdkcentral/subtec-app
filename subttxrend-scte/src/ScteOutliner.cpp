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

#include "ScteOutliner.hpp"

#include "ScteSimpleBitmap.hpp"

namespace subttxrend
{
namespace scte
{
bool Outliner::m_circle[C_SIZE][C_SIZE];

Outliner::Outliner(std::uint8_t *bytemap, uint32_t width, uint32_t height)
    : m_bytemap(bytemap), m_width(width), m_height(height), m_range{0, height - 1}
{
}

void Outliner::setPixel(unsigned x, unsigned y, uint32_t color)
{
    auto pos = y * m_width + x;
    if (pos < m_width * m_height && (m_bytemap[pos] == COLOR_TRANSPARENT || m_bytemap[pos] == COLOR_FRAME))
    {
        m_bytemap[pos] = static_cast<uint8_t>(color);
    }
}

void Outliner::initCircle(unsigned outline_thickness)
{
    static bool initialized = false;

    if (initialized)
    {
        return;
    }
    for (unsigned dy = 0; dy <= C_SIZE - 1; dy++)
    {
        for (unsigned dx = 0; dx <= C_SIZE -1; dx++)
        {
            m_circle[dy][dx] = (dx * dx + dy * dy <= outline_thickness * outline_thickness);
        }
    }
    initialized = true;
}

void Outliner::drawOutline(unsigned bx, unsigned by, unsigned outline_thickness, uint32_t value)
{
    for (unsigned dy = 0; dy <= outline_thickness; dy++)
    {
        for (unsigned dx = 0; dx <= outline_thickness; dx++)
        {
            if (m_circle[dy][dx])
            {
                setPixel(bx + dx, by + dy, value);
                setPixel(bx - dx, by + dy, value);
                setPixel(bx + dx, by - dy, value);
                setPixel(bx - dx, by - dy, value);
            }
        }
    }
}

void Outliner::outline(unsigned outline_thickness, uint32_t value)
{
    initCircle(outline_thickness);
    for (unsigned by = m_range.first; by < m_range.second + 1; by++)
    {
        for (unsigned bx = 0; bx < m_width; bx++)
        {
            if (m_bytemap[by * m_width + bx] != 1)
            {
                continue;
            }
            drawOutline(bx, by, outline_thickness, value);
        }
    }
}

void Outliner::setRange(const Coords &tl, const Coords &br)
{
    m_range = {tl.y, br.y};
}

} // namespace scte
} // namespace subttxtrend
