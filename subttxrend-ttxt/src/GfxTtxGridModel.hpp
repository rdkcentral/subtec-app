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


#ifndef SUBTTXREND_TTXT_GFX_TTX_GRID_MODEL_HPP_
#define SUBTTXREND_TTXT_GFX_TTX_GRID_MODEL_HPP_

#include <vector>

#include <ttxdecoder/Engine.hpp>
#include <ttxdecoder/Property.hpp>
#include <ttxdecoder/ControlInfo.hpp>

#include "GfxTypes.hpp"
#include "CharsetHandler.hpp"
#include "GfxTtxGridCell.hpp"

namespace subttxrend
{
namespace ttxt
{

/**
 * Teletext grid model.
 */
class GfxTtxGridModel
{
public:
    /**
     * Constructor.
     *
     * @param size
     *      Size of the grid.
     */
    GfxTtxGridModel(const Size& size);

    /**
     * Returns size of the grid.
     *
     * @return
     *      Size of the grid.
     */
    const Size& getSize() const
    {
        return m_size;
    }

    /**
     * Returns cell at given position.
     *
     * @param col
     *      Column (x coordinate).
     * @param row
     *      Row (y coordinate).
     *
     * @return
     *      Cell at given position or null if coordinates are invalid.
     */
    const GfxTtxGridCell* getConstCell(std::int32_t col,
                                       std::int32_t row) const;

    /**
     * Initializes the model.
     *
     * @param dataSource
     *      Data source to use.
     */
    void init(const ttxdecoder::Engine* dataSource);

    /**
     * Sets flash (blinking) status.
     *
     * @param flashEnabled
     *      Flash (blinking) enabled flag.
     *
     * @return
     *      True if setting was changed.
     */
    bool setFlashEnabled(bool flashEnabled)
    {
        if (m_flashEnabled != flashEnabled)
        {
            m_flashEnabled = flashEnabled;
            return true;
        }
        return false;
    }

    /**
     * Toggles reveal mode flag.
     */
    void toggleRevealEnabled()
    {
        m_revealEnabled = !m_revealEnabled;
    }

    /**
     * Sets reveal mode flag.
     *
     * @param revealEnabled
     *      Reveal enable flag.
     *      True to show concealed characters, false otherwise.
     *
     * @return
     *      True if setting was changed.
     */
    bool setRevealEnabled(bool revealEnabled)
    {
        if (m_revealEnabled != revealEnabled)
        {
            m_revealEnabled = revealEnabled;
            return true;
        }
        return false;
    }

    /**
     * Clears all cells.
     *
     * @param subtitlesMode
     *      Subtitles mode flag.
     */
    void clearAll(bool subtitlesMode);

    /**
     * Marks all cells as changed to force redraw.
     */
    void markAllAsChanged();

    /**
     * Marks cells using given color as changed.
     *
     * @param colorIndex
     *      Color index.
     */
    void markChangedByColor(std::uint8_t colorIndex);

    /**
     * Refreshes page contents.
     *
     * @param subtitlesMode
     *      Subtitles mode flag.
     */
    void refreshPage(const bool subtitlesMode);

    /**
     * Refreshes header contents.
     *
     * @param subtitlesMode
     *      Subtitles mode flag.
     */
    void refreshHeader(const bool subtitlesMode);

    /**
     * Refreshes page selection info.
     *
     * @param paused
     *      True if display is paused, false otherwise.
     * @param newPageId
     *      New page being selected, if 0 no selection is in progress.
     */
    void refreshSelection(bool paused,
                          std::uint16_t newPageId);

private:
    /** Index of row with colour key links. */
    static const std::int32_t LINKS_ROW = 24;

    /**
     * Disables all cells in row.
     *
     * @param row
     *      Row index.
     */
    void disableRow(std::int32_t row);

    /**
     * Disables cell.
     *
     * @param col
     *      Column index.
     * @param row
     *      Row index.
     */
    void disableCell(std::int32_t col,
                     std::int32_t row);

    /**
     * Returns cell at given position.
     *
     * @param col
     *      Column (x coordinate).
     * @param row
     *      Row (y coordinate).
     *
     * @return
     *      Cell at given position or null if coordinates are invalid.
     */
    GfxTtxGridCell* getCell(std::int32_t col,
                            std::int32_t row);

    /**
     * Refreshes cells data.
     *
     * @param startRow
     *      Start row index.
     * @param endRow
     *      End row index.
     * @param subtitlesMode
     *      Subtitles mode flag.
     */
    void refreshData(std::uint32_t startRow,
                     std::uint32_t endRow,
                     const bool subtitlesMode);

    /**
     * Prepares page number string.
     *
     * @param charBuffer
     *      Buffer to fill with page number string.
     * @param paused
     *      Pause flag (true = paused).
     * @param newPageId
     *      New page being selected (0 - no selection in progress).
     *
     * @return
     *      True if string was prepared.
     */
    bool preparePageNumberString(std::array<char, 16>& charBuffer,
                                 bool paused,
                                 std::uint16_t newPageId);

    /**
     * Updates row.
     *
     * @param page
     *      Page data.
     * @param row
     *      Row to be updated.
     */
    void updateRow(const ttxdecoder::DecodedPage& page,
                   std::uint32_t row);

    /**
     * Updates links row.
     *
     * @param page
     *      Page data.
     */
    void updateLinksRow(const ttxdecoder::DecodedPage& page);

    /**
     * Updates single char (cell).
     *
     * @param x
     *      Column (x coordinate).
     * @param y
     *      Row (y coordinate).
     * @param character
     *      New character.
     * @param fgColor
     *      Foreground color index.
     * @param bgColor
     *      Background color index.
     * @param properties
     *      Set of properties.
     */
    void updateChar(std::uint16_t x,
                    std::uint16_t y,
                    std::uint16_t character,
                    std::uint8_t fgColor,
                    std::uint8_t bgColor,
                    std::uint16_t properties);

    /** Size in number of cells. */
    const Size m_size;

    /** Array with cells data. */
    std::vector<GfxTtxGridCell> m_cells;

    /** Data source to use. */
    const ttxdecoder::Engine* m_dataSource;

    /** Flags (blinking) is enabled flag. */
    bool m_flashEnabled;

    /** Flags (blinking) is enabled flag. */
    bool m_revealEnabled;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_TTX_GRID_MODEL_HPP_*/
