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


#ifndef SUBTTXREND_GFX_FONT_STRIP_HPP_
#define SUBTTXREND_GFX_FONT_STRIP_HPP_

#include <memory>

#include "FontStripMap.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Font strip.
 *
 * Font strip is a set of glyphs of same size. The individual glyphs may
 * contain font characters e.g. decoded from the font file.
 */
class FontStrip
{
public:
    /**
     * Constructor.
     */
    FontStrip() = default;

    /**
     * Destructor.
     */
    virtual ~FontStrip() = default;

    /**
     * Loads glyphs from font.
     *
     * @param fontName
     *      Font name.
     * @param charSize
     *      Character size (in freetype points).
     * @param charMap
     *      Character mapping to use to map glyph indexes to character codes.
     *
     * @return
     *      True on success, false on failure.
     */
    virtual bool loadFont(const std::string& fontName,
                          const Size& charSize,
                          const FontStripMap& charMap) = 0;

    /**
     * Loads single glyph from array.
     *
     * The array is encoded as set of alpha levels for pixels in rows,
     * from top to bottom, from left to right.
     *
     * @param glyphIndex
     *      Index of the glyph.
     * @param data
     *      Array with data.
     * @param size
     *      Size of the data.
     *
     * @return
     *      True on success, false otherwise.
     */
    virtual bool loadGlyph(std::int32_t glyphIndex,
                           const std::uint8_t* data,
                           const std::size_t size) = 0;

};

/**
 * Font strip pointer.
 */
using FontStripPtr = std::shared_ptr<FontStrip>;

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_FONT_STRIP_HPP_*/
