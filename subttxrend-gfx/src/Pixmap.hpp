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


#ifndef SUBTTXREND_GFX_PIXMAP_HPP_
#define SUBTTXREND_GFX_PIXMAP_HPP_

#include <cassert>
#include <cstdint>

#include "Pixel.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

class ClutPixmap;

/**
 * Pixmap on which elements could be drawn.
 */
class Pixmap
{
public:
    /**
     * Iterator over line elements (base).
     */
    template<typename ItemType>
    class LineIteratorBase
    {
    public:
        /** Pointer to line item type. */
        typedef ItemType* Pointer;

        /** Reference to line item type. */
        typedef ItemType& Reference;

        /**
         * Constructor.
         *
         * Creates iterator pointing to nothing.
         */
        LineIteratorBase() :
                m_pointer(nullptr)
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param pointer
         *      Initial item pointer.
         */
        explicit LineIteratorBase(Pointer pointer) :
                m_pointer(pointer)
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
         * Returns pointer to item.
         *
         * @return
         *      Pointer to item.
         */
        Pointer ptr() const
        {
            return m_pointer;
        }

        /**
         * Moves iterator to next position.
         *
         * @return
         *      Reference to itself.
         */
        LineIteratorBase& operator++()
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
        LineIteratorBase& operator +=(int offset)
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
        LineIteratorBase operator +(int offset) const
        {
            assert(m_pointer);
            return LineIteratorBase(m_pointer + offset);
        }

        /**
         * Member access operator.
         *
         * @return
         *      Pointer to current item.
         */
        Pointer operator->() const
        {
            return m_pointer;
        }

        /**
         * Member access operator.
         *
         * @return
         *      Reference to current item.
         */
        Reference operator*() const
        {
            assert(m_pointer);
            return *m_pointer;
        }

    private:
        /** Pointer to current item (null for empty iterator). */
        Pointer m_pointer;
    };

    /** Line items iterator. */
    typedef LineIteratorBase<PixelArgb8888> LineIterator;

    /** Line item iterator (const). */
    typedef LineIteratorBase<const PixelArgb8888> ConstLineIterator;

    /**
     * Constructor.
     *
     * Construct empty pixmap.
     */
    Pixmap() :
            m_buffer(nullptr),
            m_width(0),
            m_height(0),
            m_stride(0)
    {
        // noop
    }

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
     */
    Pixmap(std::uint8_t* buffer,
           std::int32_t width,
           std::int32_t height,
           std::uint32_t stride) :
            m_buffer(buffer),
            m_width(width),
            m_height(height),
            m_stride(stride)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~Pixmap() = default;

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

        auto ptr = reinterpret_cast<const PixelArgb8888*>(m_buffer
                + y * m_stride);

        return ConstLineIterator(ptr);
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
    LineIterator getLine(std::int32_t y)
    {
        if (!m_buffer)
        {
            return LineIterator();
        }

        auto ptr = reinterpret_cast<PixelArgb8888*>(m_buffer + y * m_stride);

        return LineIterator(ptr);
    }

private:
    /** Pixmap buffer. */
    std::uint8_t* m_buffer;

    /** Width in pixels. */
    std::int32_t m_width;

    /** Height in pixels. */
    std::int32_t m_height;

    /** Line stride in bytes. */
    std::uint32_t m_stride;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_PIXMAP_HPP_
