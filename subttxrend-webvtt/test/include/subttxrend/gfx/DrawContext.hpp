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
#pragma once

#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <subttxrend/gfx/FontStrip.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>

namespace subttxrend
{
namespace gfx
{

class DrawContext {
public:
    virtual ~DrawContext() = default;

    virtual void fillRectangle(ColorArgb color, const Rectangle& rectangle) = 0;

    virtual void drawUnderline(ColorArgb color,
                               const Rectangle& rectangle)
    {
    }

    virtual void drawPixmap(const ClutBitmap& bitmap,
                            const Rectangle& srcRect,
                            const Rectangle& dstRect)
    {
    }

    virtual void drawBitmap(const Bitmap& bitmap,
                            const Rectangle& dstRect)
    {
    }

    virtual void drawGlyph(const FontStripPtr& fontStrip,
                           std::int32_t glyphIndex,
                           const Rectangle& rect,
                           ColorArgb fgColor,
                           ColorArgb bgColor)
    {
    }

    virtual void drawString(PrerenderedFont &m_font,
                            const Rectangle &rectangle,
                            const std::vector<GlyphData> &glyphs,
                            const ColorArgb fgColor,
                            const ColorArgb bgColor,
                            int outlineSize = 0,
                            int verticalOffset = 0) = 0;
};
}
}