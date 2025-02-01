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


#ifndef SUBTTXREND_GFX_SURFACE_HPP_
#define SUBTTXREND_GFX_SURFACE_HPP_

#include <subttxrend/common/NonCopyable.hpp>

#include <memory>

#include "Blitter.hpp"
#include "Pixmap.hpp"
#include "AlphaPixmap.hpp"
#include "Types.hpp"
#include "Pixel.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Surface (pixmap with buffering).
 *
 * @note Surface buffer is never shrinked (it may only grow).
 */
template<class PixmapType>
class SurfaceBase : private subttxrend::common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * Creates empty surface.
     */
    SurfaceBase() :
            m_maxWidth(0),
            m_maxHeight(0)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~SurfaceBase() = default;

    /**
     * Returns pixmap.
     *
     * @note Pixmap could become invalid when surface is resized.
     *
     * @return
     *      Pixmap.
     */
    PixmapType& getPixmap()
    {
        return m_pixmap;
    }

    /**
     * Returns pixmap.
     *
     * @note Pixmap could become invalid when surface is resized.
     *
     * @return
     *      Pixmap.
     */
    const PixmapType& getPixmap() const
    {
        return m_pixmap;
    }

    /**
     * Returns size.
     *
     * @return
     *      Size in pixels.
     */
    Size getSize() const
    {
        return Size
        { m_pixmap.getWidth(), m_pixmap.getHeight() };
    }

    /**
     * Returns width.
     *
     * @return
     *      Width in pixels.
     */
    int32_t getWidth() const
    {
        return m_pixmap.getWidth();
    }

    /**
     * Returns height.
     *
     * @return
     *      Height in pixels.
     */
    int32_t getHeight() const
    {
        return m_pixmap.getHeight();
    }

    /**
     * Resizes the surface.
     *
     * @param width
     *      New width in pixels.
     * @param height
     *      New height in pixels.
     * @param newPixelValue
     *      Value to be used for new pixels.
     */
    template<class PixelType>
    void resize(std::int32_t width,
                std::int32_t height,
                PixelType newPixelValue)
    {
        auto oldWidth = m_pixmap.getWidth();
        auto oldHeight = m_pixmap.getHeight();

        if ((width == oldWidth) && (height == oldHeight))
        {
            return;
        }

        if ((width > m_maxWidth) || (height > m_maxHeight))
        {
            m_maxWidth = std::max(m_maxWidth, width);
            m_maxHeight = std::max(m_maxHeight, height);
            auto stride = m_maxWidth * 4;

            m_swapBuffer.swap(m_buffer);
            auto oldPixmap = m_pixmap;

            m_buffer.resize(stride * m_maxHeight);
            m_pixmap = PixmapType(m_buffer.data(), width, height, stride);

            Blitter::write(m_pixmap, oldPixmap);
        }
        else
        {
            auto stride = m_maxWidth * 4;
            m_pixmap = PixmapType(m_buffer.data(), width, height, stride);
        }

        // fill new contents
        if (width > oldWidth)
        {
            Rectangle rect
            { oldWidth, 0, width - oldWidth, std::min(oldHeight, height) };

            Blitter::fillRectangle(m_pixmap, rect, newPixelValue);
        }
        if (height > oldHeight)
        {
            Rectangle rect
            { 0, oldHeight, std::min(oldWidth, width), height - oldHeight };
            Blitter::fillRectangle(m_pixmap, rect, newPixelValue);
        }
        if ((width > oldWidth) && (height > oldHeight))
        {
            Rectangle rect
            { oldWidth, oldHeight, width - oldWidth, height - oldHeight };
            Blitter::fillRectangle(m_pixmap, rect, newPixelValue);
        }
    }

private:
    /** Surface buffer. */
    std::vector<uint8_t> m_buffer;
    std::vector<uint8_t> m_swapBuffer;

    /** Pixmap descriptor for the current buffer. */
    PixmapType m_pixmap;

    /** Maximum surface width (used for resize logic). */
    int32_t m_maxWidth;

    /** Maximum surface height (used for resize logic). */
    int32_t m_maxHeight;
};

/**
 * ARGB pixmap surface.
 *
 * @note Class used instead of typedef so forward refs are working.
 */
class Surface : public SurfaceBase<Pixmap>
{
public:
    /**
     * Constructor.
     */
    Surface() = default;

    /**
     * Destructor.
     */
    virtual ~Surface() = default;
};

/**
 * Alpha pixmap surface.
 *
 * @note Class used instead of typedef so forward refs are working.
 */
class AlphaSurface : public SurfaceBase<AlphaPixmap>
{
public:
    /**
     * Constructor.
     */
    AlphaSurface() = default;

    /**
     * Destructor.
     */
    virtual ~AlphaSurface() = default;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_SURFACE_HPP_
