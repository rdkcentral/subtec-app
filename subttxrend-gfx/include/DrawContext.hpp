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


#ifndef SUBTTXREND_GFX_DRAW_CONTEXT_HPP_
#define SUBTTXREND_GFX_DRAW_CONTEXT_HPP_

#include "ColorArgb.hpp"
#include "Types.hpp"
#include "FontStrip.hpp"
#include "PrerenderedFont.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Context for drawing.
 */
class DrawContext
{
public:
    /**
     * Constructor.
     */
    DrawContext() = default;

    /**
     * Destructor.
     */
    virtual ~DrawContext() = default;

    /**
     * Fills rectangle with given color.
     *
     * @param color
     *      Color to use.
     * @param rectangle
     *      Rectangle to fill.
     */
    virtual void fillRectangle(ColorArgb color,
                               const Rectangle& rectangle) = 0;

    /**
     * Draws underline with given color.
     *
     * @param color
     *      Color to use.
     * @param rectangle
     *      Rectrangle with underline info to fill.
     */
    virtual void drawUnderline(ColorArgb color,
                               const Rectangle& rectangle) = 0;

    /**
     * Draws pixmap.
     *
     * @param bitmap
     *      Bitmap to draw.
     * @param srcRect
     *      Source rectangle.
     * @param dstRect
     *      Destination rectangle.
     */
    virtual void drawPixmap(const ClutBitmap& bitmap,
                            const Rectangle& srcRect,
                            const Rectangle& dstRect) = 0;

    /**
     * Draws bitmap.
     *
     * @param bitmap
     *      Bitmap to draw.
     * @param dstRect
     *      Destination rectangle.
     */
    virtual void drawBitmap(const Bitmap& bitmap,
                            const Rectangle& dstRect) = 0;
    /**
     * Draws font glyph.
     *
     * @param fontStrip
     *      Font strip to use.
     * @param glyphIndex
     *      Glyph index.
     * @param rect
     *      Destination rectangle.
     * @param fgColor
     *      Foreground color.
     * @param bgColor
     *      Background color.
     */
    virtual void drawGlyph(const FontStripPtr& fontStrip,
                           std::int32_t glyphIndex,
                           const Rectangle& rect,
                           ColorArgb fgColor,
                           ColorArgb bgColor) = 0;

    /**
     * Draws the string, with font provided.
     *
     * @param font
     *      The font to use
     * @param destinationRect
     *      Destination rectangle.
     * @param glyphs
     *      List of glyphs to draw.
     * @param fgColor
     *      Font foreground color.
     * @param bgColor
     *      Font background color.
     * @param outlineSize
     *      Size of the outline in pixels.
     */
    virtual void drawString(PrerenderedFont& font,
                            const Rectangle &destinationRect,
                            const std::vector<GlyphData>& glyphs,
                            const ColorArgb fgColor,
                            const ColorArgb bgColor,
                            int outlineSize = 0,
                            int verticalOffset = 0) = 0;
};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_DRAW_CONTEXT_HPP_*/
