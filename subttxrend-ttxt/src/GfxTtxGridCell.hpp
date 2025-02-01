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


#ifndef SUBTTXREND_TTXT_GFX_TTX_GRID_CELL_HPP_
#define SUBTTXREND_TTXT_GFX_TTX_GRID_CELL_HPP_

#include <cstdint>

namespace subttxrend
{
namespace ttxt
{

/**
 * Teletext grid cell descriptor.
 */
class GfxTtxGridCell
{
public:
    /**
     * Constructor.
     */
    GfxTtxGridCell();

    /**
     * Destructor.
     */
    ~GfxTtxGridCell();

    /**
     * Clears the cell.
     *
     * @param bg
     *      Background color index.
     */
    void clear(std::uint8_t bg);

    /**
     * Enables/disables the cell.
     *
     * @param enabled
     *      True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * Hides/shows the cell.
     *
     * @param hidden
     *      True to hide, false to show.
     */
    void setHidden(bool hidden);

    /**
     * Sets character.
     *
     * @param ch
     *      Current character.
     */
    void setCharacter(std::uint16_t ch);

    /**
     * Sets size.
     *
     * @param xMultiplier
     *      X size multiplier.
     * @param yMultiplier
     *      Y size multiplier.
     */
    void setSize(std::uint8_t xMultiplier,
                 std::uint8_t yMultiplier);

    /**
     * Sets colors.
     *
     * @param fgColor
     *      Foreground color index.
     * @param bgColor
     *      Background color index.
     */
    void setColors(std::uint8_t fgColor,
                   std::uint8_t bgColor);

    /**
     * Marks as changed (unconditionally).
     */
    void markChanged();

    /**
     * Marks as changed if uses given color.
     *
     * @param color
     *      The color.
     */
    void markChangedByColor(std::uint8_t color);

    /**
     * Checks if this cell is enabled.
     *
     * @return
     *      true if this cell is enabled.
     */
    bool isEnabled() const
    {
        return m_enabled;
    }

    /**
     * Returns the char.
     *
     * @return
     *      Char code.
     */
    std::uint16_t getChar() const
    {
        return m_char;
    }

    /**
     * Returns the X size multiplier.
     *
     * @return
     *      X size multiplier.
     */
    std::uint8_t getXMultiplier() const
    {
        return m_xMultiplier;
    }

    /**
     * Returns the Y size multiplier.
     *
     * @return
     *      Y size multiplier.
     */
    std::uint8_t getYMultiplier() const
    {
        return m_yMultiplier;
    }

    /**
     * Returns foreground color index.
     *
     * @return
     *      Color index.
     */
    std::uint8_t getFgColor() const
    {
        return m_fgColor;
    }

    /**
     * Returns background color index.
     *
     * @return
     *      Color index.
     */
    std::uint8_t getBgColor() const
    {
        return m_bgColor;
    }

    /**
     * Processes the cell redraw start.
     *
     * @return
     *      True if cell shall be redrawn, false otherwise.
     */
    bool startRedraw() const;

private:
    /**
     * Checks and sets field value.
     *
     * @param var
     *      Value (field) to update.
     * @param newValue
     *      New value to set.
     *
     * @return
     *      True if new value was different from previous one,
     *      false otherwise.
     */
    template<typename T>
    bool checkAndSet(T& var,
                     T newValue)
    {
        if (var != newValue)
        {
            var = newValue;
            return true;
        }
        else
        {
            return false;
        }
    }

    /** Enable flag. */
    bool m_enabled;

    /** Hidden flag. */
    bool m_hidden;

    /**
     * Contents have changed since last draw flag.
     *
     * Mutable for start redraw to be able to clear it.
     */
    mutable bool m_dirty;

    /** Foreground color index. */
    std::uint8_t m_fgColor;

    /** Background color index. */
    std::uint8_t m_bgColor;

    /** The character to draw. */
    std::uint16_t m_char;

    /** Width multiplier for rendering. */
    std::uint8_t m_xMultiplier;

    /** Height multiplier for rendering. */
    std::uint8_t m_yMultiplier;
};

} // namespace ttxt
} // namespace subttxrend

#endif                          // SUBTTXREND_TTXT_GFX_TTX_GRID_CELL_HPP_
