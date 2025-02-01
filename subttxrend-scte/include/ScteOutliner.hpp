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

#pragma once

#include <cstdint>
#include <utility>

#include "ScteSimpleBitmap.hpp"

namespace subttxrend
{
namespace scte
{

class Outliner
{
public:
    Outliner(std::uint8_t *bytemap, uint32_t width, uint32_t height);
    void outline(unsigned outline_thickness, uint32_t value);
    void setRange(const Coords &tl, const Coords &br);

private:
    void setPixel(unsigned x, unsigned y, uint32_t color);
    void initCircle(unsigned outline_thickness);
    void drawOutline(unsigned bx, unsigned by, unsigned outline_thickness, uint32_t value);

private:
    std::uint8_t *m_bytemap;
    const uint32_t m_width;
    const uint32_t m_height;
    std::pair<uint32_t, uint32_t> m_range;
    static constexpr int C_SIZE = 16;
    static bool m_circle[C_SIZE][C_SIZE];
};

} // namespace scte
} // namespace subttxtrend
