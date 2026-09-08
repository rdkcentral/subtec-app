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
#include "PrerenderedFont.hpp"

#include <subttxrend/gfx/Types.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace subttxrend
{
namespace gfx
{

class FontStrip;
using FontStripPtr = std::shared_ptr<FontStrip>;

class DrawContext
{
public:
    DrawContext() = default;
    virtual ~DrawContext() = default;

    virtual void fillRectangle(ColorArgb color,
                               const Rectangle& rectangle) = 0;

    virtual void drawUnderline(ColorArgb color,
                               const Rectangle& rectangle) = 0;

    virtual void drawPixmap(const ClutBitmap& bitmap,
                            const Rectangle& srcRect,
                            const Rectangle& dstRect) = 0;

    virtual void drawBitmap(const Bitmap& bitmap,
                            const Rectangle& dstRect) = 0;

    virtual void drawGlyph(const FontStripPtr& fontStrip,
                           std::int32_t glyphIndex,
                           const Rectangle& rect,
                           ColorArgb fgColor,
                           ColorArgb bgColor) = 0;

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