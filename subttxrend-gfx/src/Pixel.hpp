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


#ifndef SUBTTXREND_GFX_PIXEL_HPP_
#define SUBTTXREND_GFX_PIXEL_HPP_

#include <cstdint>

#include "ColorArgb.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Single ARGB:8888 pixel representation.
 */
struct PixelArgb8888
{
    /**
     * Constructor.
     *
     * Created solid white pixel.
     */
    PixelArgb8888() :
            m_a(0xFF),
            m_r(0xFF),
            m_g(0xFF),
            m_b(0xFF)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param color
     *      Color in little-endian ARGB:8888 format.
     */
    explicit PixelArgb8888(std::uint32_t color) :
            m_a((color >> 24) & 0xFF),
            m_r((color >> 16) & 0xFF),
            m_g((color >> 8) & 0xFF),
            m_b((color >> 0) & 0xFF)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param a
     *      Alpha component.
     * @param r
     *      Red component.
     * @param g
     *      Green component.
     * @param b
     *      Blue component.
     */
    PixelArgb8888(std::uint8_t a,
                  std::uint8_t r,
                  std::uint8_t g,
                  std::uint8_t b) :
            m_a(a),
            m_r(r),
            m_g(g),
            m_b(b)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param color
     *      Color value.
     */
    PixelArgb8888(ColorArgb color) :
            m_a(color.m_a),
            m_r(color.m_r),
            m_g(color.m_g),
            m_b(color.m_b)
    {
        // noop
    }

    /* === NOTE: order is important!!! === */

    /** Blue component. */
    std::uint8_t m_b;

    /** Green component. */
    std::uint8_t m_g;

    /** Red component. */
    std::uint8_t m_r;

    /** Alpha component. */
    std::uint8_t m_a;
};

static_assert(sizeof(PixelArgb8888) == 4, "Invalid struct size");

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_PIXEL_HPP_
