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


#ifndef SUBTTXREND_GFX_COLORIZED_PIXMAP_HPP_
#define SUBTTXREND_GFX_COLORIZED_PIXMAP_HPP_

#include <stdint.h>

#include "AlphaPixmap.hpp"
#include "Pixel.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Alpha pixmap wrapper that will colorize it.
 */
class ColorizedPixmap
{
public:
    /**
     * Iterator over line elements (const).
     */
    class ConstLineIterator
    {
    public:
        /**
         * Constructor.
         *
         * Creates iterator pointing to nothing.
         */
        ConstLineIterator() :
                m_peerIterator(),
                m_fgColor(),
                m_bgColor()
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param peerIterator
         *      Peer iterator to use to access line data.
         * @param fgColor
         *      Foreground color.
         * @param bgColor
         *      Background color.
         */
        ConstLineIterator(const AlphaPixmap::ConstLineIterator& peerIterator,
                          PixelArgb8888 fgColor,
                          PixelArgb8888 bgColor) :
                m_peerIterator(peerIterator),
                m_fgColor(fgColor),
                m_bgColor(bgColor)
        {
            // noop
        }

        /**
         * Convert to bool operator.
         *
         * @return
         *      True if iterator received pointer during construction
         *      or false otherwise (invalid empty iterator).
         */
        operator bool() const
        {
            return m_peerIterator;
        }

        /**
         * Moves iterator to next position.
         *
         * @return
         *      Reference to itself.
         */
        ConstLineIterator& operator++()
        {
            ++m_peerIterator;
            return *this;
        }

        /**
         * Moves iterator offset elements forward.
         *
         * @param offset
         *      Number of elements.
         *
         * @return
         *      Reference to itself.
         */
        ConstLineIterator& operator +=(int offset)
        {
            m_peerIterator += offset;
            return *this;
        }

        /**
         * Returns iterator that points offset elements forward.
         *
         * @param offset
         *      Number of elements.
         *
         * @return
         *      Iterator to requested position.
         */
        ConstLineIterator operator +(int offset) const
        {
            return ConstLineIterator(m_peerIterator + offset, m_fgColor,
                    m_bgColor);
        }

        /**
         * Member access operator.
         *
         * @return
         *      Current item (alpha-colorized pixel).
         */
        PixelArgb8888 operator*() const
        {
            const auto alpha = *m_peerIterator;
            if ((alpha == 255) && (m_fgColor.m_a == 255))
            {
                return m_fgColor;
            }
            else if (alpha == 0)
            {
                return m_bgColor;
            }
            else if (m_bgColor.m_a == 0)
            {
                std::uint32_t af = premultiply(m_fgColor.m_a, alpha);

                std::uint8_t a = (af >> 8);

                std::uint8_t r = m_fgColor.m_r;
                std::uint8_t g = m_fgColor.m_g;
                std::uint8_t b = m_fgColor.m_b;

                return PixelArgb8888(a, r, g, b);
            }
            else
            {
                std::uint32_t af = premultiply(m_fgColor.m_a, alpha);

                std::uint32_t cf = (af >> 8);

                std::uint32_t rf = premultiply(m_fgColor.m_r, cf);
                std::uint32_t gf = premultiply(m_fgColor.m_g, cf);
                std::uint32_t bf = premultiply(m_fgColor.m_b, cf);

                std::uint32_t cb = 255 - (af >> 8);

                std::uint32_t ab = premultiply(m_bgColor.m_a, cb);
                std::uint32_t rb = premultiply(m_bgColor.m_r, cb);
                std::uint32_t gb = premultiply(m_bgColor.m_g, cb);
                std::uint32_t bb = premultiply(m_bgColor.m_b, cb);

                std::uint32_t a = (af + ab) >> 8;
                std::uint32_t r = (rf + rb) >> 8;
                std::uint32_t g = (gf + gb) >> 8;
                std::uint32_t b = (bf + bb) >> 8;

                return PixelArgb8888(a, r, g, b);
            }
        }

        /**
         * Performs alpha premultiplication.
         *
         * @param component
         *      Color component value.
         * @param alpha
         *      Alpha value.
         *
         * @return
         *      Premultiplied color value (16 bit range).
         */
        static uint32_t premultiply(std::uint8_t component,
                                    std::uint8_t alpha)
        {
            if (alpha == 255)
            {
                return component << 8;
            }
            else if (alpha == 0)
            {
                return 0;
            }
            else
            {
                uint32_t value = component;
                value *= alpha;
                return value;
            }
        }

    private:
        /** Pointer to line data. */
        AlphaPixmap::ConstLineIterator m_peerIterator;

        /** Foreground color. */
        PixelArgb8888 m_fgColor;

        /** Background color. */
        PixelArgb8888 m_bgColor;
    };

    /**
     * Constructor.
     *
     * @param alphaPixmap
     *      Wrapped alpha pixmap that will be colorized.
     * @param fgColor
     *      Foreground color.
     * @param bgColor
     *      Background color.
     */
    ColorizedPixmap(const AlphaPixmap& alphaPixmap,
                    PixelArgb8888 fgColor,
                    PixelArgb8888 bgColor) :
            m_alphaPixmap(alphaPixmap),
            m_fgColor(fgColor),
            m_bgColor(bgColor)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~ColorizedPixmap() = default;

    /**
     * Returns width.
     *
     * @return
     *      Width in pixels.
     */
    std::int32_t getWidth() const
    {
        return m_alphaPixmap.getWidth();
    }

    /**
     * Returns height.
     *
     * @return
     *      Height in pixels.
     */
    std::int32_t getHeight() const
    {
        return m_alphaPixmap.getHeight();
    }

    /**
     * Returns pointer to line data.
     *
     * @param y
     *      Line to get.
     *
     * @return
     *      Pointer to line data, null if not available.
     */
    ConstLineIterator getLine(int y) const
    {
        return ConstLineIterator(m_alphaPixmap.getLine(y), m_fgColor, m_bgColor);
    }

private:
    /** Wrapped pixmap. */
    const AlphaPixmap& m_alphaPixmap;

    /** Foreground color. */
    PixelArgb8888 m_fgColor;

    /** Background color. */
    PixelArgb8888 m_bgColor;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_COLORIZED_PIXMAP_HPP_
