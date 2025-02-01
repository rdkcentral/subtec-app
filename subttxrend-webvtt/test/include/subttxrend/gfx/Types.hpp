/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#ifndef SUBTTXREND_GFX_TYPES_HPP_
#define SUBTTXREND_GFX_TYPES_HPP_

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

namespace subttxrend
{
namespace gfx
{

/**
 * Size.
 */
struct Size
{
    /**
     * Constructor.
     *
     * Fills with zeroes.
     */
    Size() :
            m_w(0),
            m_h(0)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param w
     *      Width.
     * @param h
     *      Height.
     */
    Size(std::int32_t w,
         std::int32_t h) :
            m_w(w),
            m_h(h)
    {
        // noop
    }

    /** Width. */
    std::int32_t m_w;

    /** Height. */
    std::int32_t m_h;
};

static inline bool operator == (Size const& lhs, Size const& rhs)
{
    return lhs.m_w == rhs.m_w && lhs.m_h == rhs.m_h;
}

static inline bool operator != (Size const& lhs, Size const& rhs)
{
    return !(lhs == rhs);
}
static inline std::ostream& operator<<(std::ostream& out, Size const& s)
{
    return out << "size:[" << s.m_w << "x" << s.m_h << "]";
}
/**
 * Rectangle.
 */
struct Rectangle
{
    /**
     * Constructor.
     *
     * Fills with zeroes.
     */
    Rectangle() :
            m_x(0),
            m_y(0),
            m_w(0),
            m_h(0)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param x
     *      Left position.
     * @param y
     *  Top position.
     * @param w
     *      Width.
     * @param h
     *      Height.
     */
    Rectangle(std::int32_t x,
              std::int32_t y,
              std::int32_t w,
              std::int32_t h) :
            m_x(x),
            m_y(y),
            m_w(w),
            m_h(h)
    {
        // noop
    }

    Size getSize() const
    {
        return Size(m_w, m_h);
    }
    /** Left position. */
    std::int32_t m_x;

    /** Top position. */
    std::int32_t m_y;

    /** Width. */
    std::int32_t m_w;

    /** Height. */
    std::int32_t m_h;
};

static inline std::ostream& operator<<(std::ostream& out, Rectangle const& r)
{
    return out << "rect:[" << r.m_x << ";" << r.m_y << "][" << r.m_w << "x" << r.m_h << "]";
}
/**
 * Bitmap.
 */
struct ClutBitmap
{
    /**
     * Constructor.
     *
     * @param width
     *      Width.
     * @param height
     *      Height.
     * @param stride
     *      Stride size (number of bytes to move between lines).
     * @param pixels
     *      Pixels buffer.
     * @param clut
     *      Colour lookup table.
     * @param clutSize
     *      Size of colour lookup table.
     */
    ClutBitmap(std::uint32_t width,
               std::uint32_t height,
               std::uint32_t stride,
               const std::uint8_t* pixels,
               const std::uint32_t* clut,
               std::size_t clutSize) :
            m_width(width),
            m_height(height),
            m_stride(stride),
            m_pixels(pixels),
            m_clut(clut),
            m_clutSize(clutSize)
    {
        // noop
    }

    /** Width. */
    const std::uint32_t m_width;
    /** Height. */
    const std::uint32_t m_height;
    /** Stride. */
    const std::uint32_t m_stride;
    /** Pixels buffer. */
    const std::uint8_t* const m_pixels;
    /** Colour lookup table. */
    const std::uint32_t* const m_clut;
    /** CLUT size. */
    const std::size_t m_clutSize;
};

struct Bitmap
{
    /**
     * Constructor.
     *
     * @param width
     *      Width.
     * @param height
     *      Height.
     * @param stride
     *      Stride size (number of bytes to move between lines).
     */
    Bitmap(
        std::int32_t width,
        std::int32_t height,
        std::uint32_t stride) :
            m_buffer(height * stride),
            m_width(width),
            m_height(height),
            m_stride(stride)
    {
        // no-op
    }

    Size getSize() const
    {
        return Size(m_width, m_height);
    }

    ~Bitmap() = default;

    /* buffer, created & owned by bitmap */
    std::vector<std::uint8_t> m_buffer;
    /** Width. */
    const std::int32_t m_width;
    /** Height. */
    const std::int32_t m_height;
    /** Stride. */
    const std::uint32_t m_stride;
};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_TYPES_HPP_*/
