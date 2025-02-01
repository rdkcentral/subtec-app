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


#ifndef SUBTTXREND_GFX_COLOR_ARGB_HPP_
#define SUBTTXREND_GFX_COLOR_ARGB_HPP_

#include <cstdint>
#include <string>

namespace subttxrend
{
namespace gfx
{

/**
 * ARGB:8888 color.
 */
class ColorArgb
{
public:
    /** standard colors */
    static const ColorArgb TRANSPARENT;
    static const ColorArgb WHITE;
    static const ColorArgb BLACK;
    static const ColorArgb SILVER;
    static const ColorArgb GRAY;
    static const ColorArgb MAROON;
    static const ColorArgb RED;
    static const ColorArgb PURPLE;
    static const ColorArgb FUCHSIA;
    static const ColorArgb MAGENTA;
    static const ColorArgb GREEN;
    static const ColorArgb LIME;
    static const ColorArgb OLIVE;
    static const ColorArgb YELLOW;
    static const ColorArgb NAVY;
    static const ColorArgb BLUE;
    static const ColorArgb TEAL;
    static const ColorArgb AQUA;
    static const ColorArgb CYAN;

    static bool getColorByName(std::string name, ColorArgb& color);

    /**
     * Constructor.
     *
     * Created solid white pixel.
     */
    ColorArgb() :
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
     *      Color in ARGB:8888 format.
     */
    explicit ColorArgb(std::uint32_t color) :
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
    ColorArgb(std::uint8_t a,
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
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if objects are the same. False otherwise.
     */
    friend bool operator==(const ColorArgb& lhs,
                           const ColorArgb& rhs)
    {
        return ((lhs.m_b == rhs.m_b)
                && (lhs.m_g == rhs.m_g)
                && (lhs.m_r == rhs.m_r)
                && (lhs.m_a == rhs.m_a));
    }

    friend bool operator!=(const ColorArgb& lhs,
                           const ColorArgb& rhs)
    {
        return !(lhs == rhs);
    }

public:
    /** Blue component. */
    std::uint8_t m_b;

    /** Green component. */
    std::uint8_t m_g;

    /** Red component. */
    std::uint8_t m_r;

    /** Alpha component. */
    std::uint8_t m_a;
};

std::ostream& operator<<(std::ostream& os, const ColorArgb& color);

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_COLOR_ARGB_HPP_*/
