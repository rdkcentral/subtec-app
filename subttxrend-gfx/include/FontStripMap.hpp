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


#ifndef SUBTTXREND_GFX_FONT_STRIP_MAP_HPP_
#define SUBTTXREND_GFX_FONT_STRIP_MAP_HPP_

#include <cstdint>
#include <map>

namespace subttxrend
{
namespace gfx
{

/**
 * Mapping between characters (UTF-16) and glyph indexes.
 */
class FontStripMap
{
public:
    /**
     * Constructor.
     *
     * Constructs empty map.
     */
    FontStripMap() = default;

    /**
     * Clears the mapping.
     */
    void clear()
    {
        m_mapCharToIndex.clear();
        m_mapIndexToChar.clear();
    }

    /**
     * Returns number of glyphs needed to load all mapped characters.
     *
     * @return
     *      Number of glyphs.
     */
    std::size_t getNeededGlyphCount() const
    {
        if (!m_mapIndexToChar.empty())
        {
            return m_mapIndexToChar.rbegin()->first + 1;
        }
        else
        {
            return 0;
        }
    }

    /**
     * Adds mapping between character and glyph index.
     *
     * @param character
     *      Character UTF-16 code.
     * @param glyphIndex
     *      Glyph index.
     *
     * @note There is no checking done so previous mapping
     *       may be overriden if method is missused.
     */
    void addMapping(std::uint16_t character,
                    std::uint16_t glyphIndex)
    {
        m_mapCharToIndex[character] = glyphIndex;
        m_mapIndexToChar[glyphIndex] = character;
    }

    /**
     * Converts glyph index to character.
     *
     * @param glyphIndex
     *      Glyph index.
     *
     * @return
     *      Negative value means there was no mapping.
     *      Otherwise the lower 16 bits are used to return UTF-16 character
     *      mapped to glyph index.
     */
    std::int32_t toCharacter(std::uint16_t glyphIndex) const
    {
        auto iter = m_mapIndexToChar.find(glyphIndex);
        if (iter != m_mapIndexToChar.end())
        {
            return iter->second;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Converts character to glyph index.
     *
     * @param character
     *      Character UTF-16 code.
     *
     * @return
     *      Negative value means there was no mapping.
     *      Otherwise the lower 16 bits are used to return glyph index mapped
     *      to the character.
     */
    std::int32_t toGlyphIndex(std::uint16_t character) const
    {
        auto iter = m_mapCharToIndex.find(character);
        if (iter != m_mapCharToIndex.end())
        {
            return iter->second;
        }
        else
        {
            return -1;
        }
    }

private:
    /** Index-to-character mapping. */
    std::map<std::uint16_t,std::uint16_t> m_mapIndexToChar;

    /** Character-to-index mapping. */
    std::map<std::uint16_t,std::uint16_t> m_mapCharToIndex;
};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_FONT_STRIP_MAP_HPP_*/
