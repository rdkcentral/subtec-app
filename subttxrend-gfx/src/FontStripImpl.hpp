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


#ifndef SUBTTXREND_GFX_FONT_STRIP_IMPL_HPP_
#define SUBTTXREND_GFX_FONT_STRIP_IMPL_HPP_

#include "FontStrip.hpp"
#include "Surface.hpp"
#include <fontconfig/fontconfig.h>

namespace subttxrend
{
namespace gfx
{

/**
 * Font strip - implementation.
 */
class FontStripImpl : public FontStrip
{
public:
    /**
     * Constructor.
     *
     * @param glyphSize
     *      Size of single font glyph.
     * @param glyphCount
     *      Number of glyphs.
     */
    FontStripImpl(const Size& glyphSize,
                  const std::size_t glyphCount);

    /**
     * Destructor.
     */
    virtual ~FontStripImpl();

    /** @copydoc FontStrip::loadFont */
    virtual bool loadFont(const std::string& fontName,
                          const Size& charSize,
                          const FontStripMap& charMap) override;

    /** @copydoc FontStrip::loadGlyph */
    virtual bool loadGlyph(std::int32_t glyphIndex,
                           const std::uint8_t* data,
                           const std::size_t size) override;

    /**
     * Returns strip pixmap.
     *
     * @return
     *      Pixmap with glyphs.
     */
    const AlphaPixmap& getPixmap() const;

    /**
     * Returns rectangle for a glyph.
     *
     * @param glyphIndex
     *      Glyph index.
     *
     * @return
     *      Rectangle withing the pixmap where glyph is stored.
     *      If glyph is not available empty rectangle is returned.
     */
    Rectangle getGlyphRect(std::int32_t glyphIndex) const;

    /**
     * Finds font file path.
     *
     * @param fontName
     *      Font name (may be absolute path if starts from '/').
     *
     * @return
     *      Calculated font file path.
     */
     static std::string findFontFile(const std::string& fontName);

private:

    /** Size of single font glyph. */
    const gfx::Size m_glyphSize;

    /** Number of glyphs. */
    const std::size_t m_glyphCount;

    /**
     * Surface with glyphs.
     */
    AlphaSurface m_surface;

    static FcConfig* m_font_config;
};

/**
 * Font strip implementation pointer.
 */
using FontStripImplPtr = std::shared_ptr<FontStripImpl>;

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_FONT_STRIP_IMPL_HPP_*/
