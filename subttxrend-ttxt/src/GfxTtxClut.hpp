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


#ifndef SUBTTXREND_TTXT_GFX_TTX_CLUT_HPP_
#define SUBTTXREND_TTXT_GFX_TTX_CLUT_HPP_

#include <array>
#include <cstdint>
namespace subttxrend
{
namespace ttxt
{

/**
 * Teletext color lookup table.
 */
class GfxTtxClut
{
public:
    /** Color index - black. */
    static const std::uint8_t COLOR_INDEX_BLACK = 0;
    /** Color index - red. */
    static const std::uint8_t COLOR_INDEX_RED = 1;
    /** Color index - green. */
    static const std::uint8_t COLOR_INDEX_GREEN = 2;
    /** Color index - yellow. */
    static const std::uint8_t COLOR_INDEX_YELLOW = 3;
    /** Color index - cyan. */
    static const std::uint8_t COLOR_INDEX_CYAN = 5;
    /** Color index - white. */
    static const std::uint8_t COLOR_INDEX_WHITE = 7;
    /** Color index - transparent. */
    static const std::uint8_t COLOR_INDEX_TRANSPARENT = 8;

    /**
     * Constructor.
     *
     * Sets all color to opaque black.
     */
    GfxTtxClut();

    /**
     * Resets all colors to teletext defaults.
     */
    void resetColors();

    /**
     * Sets the color.
     *
     * @param index
     *      Index of color to set.
     * @param color
     *      Color ARGB value.
     *
     * @return
     *      True if color was modified, false otherwise.
     */
    bool setColor(std::size_t index,
                  std::uint32_t color);

    /**
     * Returns CLUT array.
     *
     * @return
     *      CLUT array.
     */
    const std::uint32_t* getArray() const;

    /**
     * Returns CLUT size.
     *
     * @return
     *      CLUT size.
     */
    std::size_t getSize() const;

private:
    /** Clut colors array. */
    std::array<std::uint32_t,32> m_clut;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_TTX_CLUT_HPP_*/
