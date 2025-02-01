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


#ifndef FTCPP_MONOSPACE_RENDERER_HPP_
#define FTCPP_MONOSPACE_RENDERER_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H
#include <sstream>

#include "Face.hpp"
#include "Library.hpp"

namespace ftcpp
{

/**
 * Renderer for monospace characters.
 *
 * This is an utility class that allows rendering of the monospace characters.
 */
class MonospaceRenderer
{
public:
    /**
     * Constructor.
     *
     * @param library
     *      Library handle.
     */
    MonospaceRenderer(Library* library) :
            m_grayscaleBitmap(library->newBitmap())
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~MonospaceRenderer() = default;

    /**
     * Renders current glyph.
     *
     * When rendering is requested the character is drawn centered in the
     * specified area.
     *
     * @note Currently only grayscale pixel format is supported!
     *
     * @param face
     *      Font face with the glyph to render.
     * @param width
     *      Width of the rendering area in pixels.
     * @param height
     *      Height of the rendering area in pixels.
     */
    void render(Face& face,
                FT_Long width,
                FT_Long height)
    {
        clear(width, height);

        const auto& slot = face.getGlyph();

        FT_Long advanceX = (slot->advance.x + 32) / 64;

        FT_Long centerOffsetY = (height - calculateHeight(face)) / 2;
        FT_Long centerOffsetX = (width - advanceX) / 2;

        FT_Long bitmapOffsetY = calculateBaseline(face) - slot->bitmap_top;

        FT_Long offsetX = slot->bitmap_left + centerOffsetX;
        FT_Long offsetY = centerOffsetY + bitmapOffsetY;

        m_grayscaleBitmap->convert(slot->bitmap);

        std::uint32_t levelsDivider =
                m_grayscaleBitmap->getNativeObject()->num_grays;
        if (levelsDivider <= 1)
        {
            levelsDivider = 1;
        }
        else
        {
            --levelsDivider;
        }

        const auto& bitmap = *m_grayscaleBitmap.get();

        for (std::size_t bitmapY = 0; bitmapY < bitmap->rows; ++bitmapY)
        {
            auto p = reinterpret_cast<std::uint8_t*>(bitmap->buffer)
                    + (bitmapY * bitmap->pitch);

            FT_Long pixelY = bitmapY + offsetY;

            for (std::size_t bitmapX = 0; bitmapX < bitmap->width; ++bitmapX)
            {
                FT_Long pixelX = bitmapX + offsetX;

                if ((pixelX < 0) || (pixelX >= width) || (pixelY < 0)
                        || (pixelY >= height))
                {
                    continue;
                }

                std::uint32_t value = p[bitmapX];
                value *= 255;
                value /= levelsDivider;

                setPixel(pixelX, pixelY, value);
            }
        }
    }

protected:
    /**
     * Clears specified area.
     *
     * @param width
     *      Width of the area to clear.
     * @param height
     *      Height of the area to clear.
     */
    virtual void clear(FT_Long width,
                       FT_Long height) = 0;

    /**
     * Sets pixel.
     *
     * @param x
     *      X coordinate of the pixel.
     * @param y
     *      X coordinate of the pixel.
     * @param value
     *      Value of the pixel (graphscale).
     */
    virtual void setPixel(FT_Long x,
                          FT_Long y,
                          uint8_t value) = 0;

private:
    /**
     * Calculates current font face height.
     *
     * @param face
     *      Font face.
     *
     * @return
     *      Font face height in pixels.
     */
    FT_Long calculateHeight(Face& face) const
    {
        return (face.getSize()->metrics.height + 32) / 64;
    }

    /**
     * Calculates current font face baseline.
     *
     * @param face
     *      Font face.
     *
     * @return
     *      Baseline in pixels.
     */
    FT_Long calculateBaseline(Face& face) const
    {
        return ((face.getSize()->metrics.ascender + 32) / 64);
    }

    /** Grayscale bitmap. */
    std::unique_ptr<Bitmap> m_grayscaleBitmap;
};

} // namespace ftcpp

#endif /*FTCPP_MONOSPACE_RENDERER_HPP_*/
