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


#ifndef SUBTTXREND_GFX_CLUT_PIXMAP_HPP_
#define SUBTTXREND_GFX_CLUT_PIXMAP_HPP_

#include <cassert>
#include <cstdint>

#include "Pixel.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Pixmap with colour lookup table.
 */
class ClutPixmap
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
                m_pointer(nullptr),
                m_clut(nullptr),
                m_clutSize(0)
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param pointer
         *      Initial item pointer.
         * @param clut
         *      Colour lookup table.
         * @param clutSize
         *      Number of entries in colour lookup table.
         */
        ConstLineIterator(const std::uint8_t* pointer,
                          const std::uint32_t* clut,
                          std::size_t clutSize) :
                m_pointer(pointer),
                m_clut(clut),
                m_clutSize(clutSize)
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
            return m_pointer;
        }

        /**
         * Moves iterator to next position.
         *
         * @return
         *      Reference to itself.
         */
        ConstLineIterator& operator++()
        {
            assert(m_pointer);
            ++m_pointer;
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
            assert(m_pointer);
            m_pointer += offset;
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
            assert(m_pointer);
            return ConstLineIterator(m_pointer + offset, m_clut, m_clutSize);
        }

        /**
         * Member access operator.
         *
         * @return
         *      Current item (color translated to argb using clut).
         */
        PixelArgb8888 operator*() const
        {
            static const uint32_t TRANSPARENT_BLACK = 0;

            assert(m_pointer);
            auto value = *m_pointer;
            if (value < m_clutSize)
            {
                PixelArgb8888 ret = PixelArgb8888(m_clut[value]);
                // apply per-pixel alpha-premultiplication
                ret.m_r = uint8_t(uint32_t(ret.m_r * ret.m_a) / 255);
                ret.m_g = uint8_t(uint32_t(ret.m_g * ret.m_a) / 255);
                ret.m_b = uint8_t(uint32_t(ret.m_b * ret.m_a) / 255);

                return ret;
            }

            return PixelArgb8888(TRANSPARENT_BLACK);
        }

    private:
        /** Pointer to current item (null for empty iterator). */
        const std::uint8_t* m_pointer;

        /** Colour lookup table. */
        const std::uint32_t* m_clut;

        /** Number of entries in colour lookup table. */
        std::size_t m_clutSize;
    };

    /**
     * Constructor.
     *
     * @param buffer
     *      Pixmap buffer.
     * @param width
     *      Width in pixels.
     * @param height
     *      Height in pixels.
     * @param stride
     *      Line stride in bytes.
     * @param clut
     *      Colour lookup table.
     * @param clutSize
     *      Number of entries in colour lookup table.
     */
    ClutPixmap(const std::uint8_t* buffer,
                    std::int32_t width,
                    std::int32_t height,
                    std::uint32_t stride,
                    const std::uint32_t* clut,
                    std::size_t clutSize) :
            m_buffer(buffer),
            m_width(width),
            m_height(height),
            m_stride(stride),
            m_clut(clut),
            m_clutSize(clutSize)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~ClutPixmap() = default;

    /**
     * Returns width.
     *
     * @return
     *      Width in pixels.
     */
    std::int32_t getWidth() const
    {
        return m_width;
    }

    /**
     * Returns height.
     *
     * @return
     *      Height in pixels.
     */
    std::int32_t getHeight() const
    {
        return m_height;
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
    ConstLineIterator getLine(std::int32_t y) const
    {
        if (!m_buffer)
        {
            return ConstLineIterator();
        }

        auto ptr = m_buffer + y * m_stride;

        return ConstLineIterator(ptr, m_clut, m_clutSize);
    }

private:
    /** Pixmap buffer. */
    const std::uint8_t* m_buffer;

    /** Width in pixels. */
    std::int32_t m_width;

    /** Height in pixels. */
    std::int32_t m_height;

    /** Line stride in bytes. */
    std::uint32_t m_stride;

    /** Colour lookup table. */
    const std::uint32_t* m_clut;

    /** Number of entries in colour lookup table. */
    std::size_t m_clutSize;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_CLUT_PIXMAP_HPP_
