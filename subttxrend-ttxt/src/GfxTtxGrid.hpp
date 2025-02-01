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


#ifndef SUBTTXREND_TTXT_GFX_TTX_GRID_HPP_
#define SUBTTXREND_TTXT_GFX_TTX_GRID_HPP_

#include <map>

#include <ttxdecoder/Engine.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/FontStrip.hpp>

#include "CharsetHandler.hpp"
#include "GfxTypes.hpp"

namespace subttxrend
{
namespace ttxt
{

class GfxConfig;
class GfxTtxGridModel;
class GfxTtxClut;
class GfxTtxGridCell;

/**
 * Teletext grid.
 */
class GfxTtxGrid
{
public:
    /**
     * Constructor.
     *
     * @param model
     *      Grid model.
     * @param clut
     *      CLUT to use.
     */
    GfxTtxGrid(GfxTtxGridModel& model,
               GfxTtxClut& clut);

    /**
     * Initializes the grid.
     *
     * Allocated all the resources needed e.g. loads fonts.
     *
     * @param ttxEngine
     *      Teletext engine to use.
     * @param gfxEngine
     *      Graphics engine to use.
     * @param config
     *      Configuration.
     */
    void init(const ttxdecoder::Engine& ttxEngine,
              const gfx::EnginePtr& gfxEngine,
              const GfxConfig& config);

    /**
     * Shuts the grid down.
     *
     * Releases all resources allocated by init.
     */
    void shutdown();

    /**
     * Clears all previously added zoom mode settings.
     */
    void clearModeSettings();

    /**
     * Adds zoom mode settings.
     *
     * @param mode
     *      Zoom mode.
     * @param rowRange
     *      Row range to display.
     * @param gridRectangle
     *      Grid bounds (not that the bounds includes rows outside
     *      the range).
     */
    void addModeSettings(ZoomMode mode,
                         const RowRange& rowRange,
                         const Rect& gridRectangle);

    /**
     * Draws the grid.
     *
     * @param mode
     *      Zoom mode.
     * @param context
     *      Context for drawing operations.
     * @param bgAlpha
     *      Background alpha to use.
     */
    void draw(ZoomMode mode,
              gfx::DrawContext& context,
              std::uint8_t bgAlpha);

private:
    /**
     * Mode settings.
     */
    struct ModeSettings
    {
        /**
         * Constructor.
         *
         * Fill with defaults.
         */
        ModeSettings() :
                m_rowRange(),
                m_gridRectangle()
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param rowRange
         *      Rows range.
         * @param gridRectangle
         *      Grid bounds (including rows outside range).
         */
        ModeSettings(const RowRange& rowRange,
                     const Rect& gridRectangle) :
                m_rowRange(rowRange),
                m_gridRectangle(gridRectangle)
        {
            // noop
        }

        /** Rows drange. */
        RowRange m_rowRange;

        /** Grid bounds (including rows outside range). */
        Rect m_gridRectangle;
    };

    /**
     * Loads G0/G2 font.
     *
     * @param gfxEngine
     *      Graphics engine to use.
     * @param config
     *      Configuration.
     */
    void loadFontG0G2(const gfx::EnginePtr& gfxEngine,
                      const GfxConfig& config);

    /**
     * Draws cell.
     *
     * @param x
     *      Cell column.
     * @param y
     *      Cell row.
     * @param bounds
     *      Cell bounds (rectangle to draw).
     * @param context
     *      Graphics context to use.
     * @param bgAlpha
     *      Background alpha level.
     * @param rowLimit
     *      Limit of rows to draw (for multirow cells).
     */
    void drawCell(std::int32_t x,
                  std::int32_t y,
                  const Rect& bounds,
                  gfx::DrawContext& context,
                  std::uint8_t bgAlpha,
                  std::int32_t rowLimit);

    /**
     * Blends the color with background alpha.
     *
     * @param argb
     *      Color to blend.
     * @param bgAlpha
     *      Background alpha.
     *
     * @return
     *      Calculated color.
     */
    uint32_t blendAlpha(std::uint32_t argb,
                        std::uint8_t bgAlpha);

    /**
     * Gets the color for index.
     *
     * @param colorIndex
     *      Color index.
     *
     * @return
     *      ARGB color.
     */
    uint32_t getColor(std::uint8_t colorIndex) const;

    /**
     * Draws cell.
     *
     * @param cell
     *      Cell to draw.
     * @param rect
     *      Cell bounds (rectangle to draw).
     * @param context
     *      Graphics context to use.
     * @param bgAlpha
     *      Background alpha level.
     */
    void drawCell(const GfxTtxGridCell* cell,
                  const Rect& rect,
                  gfx::DrawContext& context,
                  std::uint8_t bgAlpha);

    /**
     * Draws cell character.
     *
     * @param ch
     *      Character to draw.
     * @param rect
     *      Cell bounds (rectangle to draw).
     * @param fgColor
     *      Foreground color.
     * @param bgColor
     *      Background color.
     * @param context
     *      Graphics context to use.
     */
    void drawCellChar(std::uint16_t ch,
                      const Rect& rect,
                      gfx::ColorArgb fgColor,
                      gfx::ColorArgb bgColor,
                      gfx::DrawContext& context);

    /** Model with grid data. */
    GfxTtxGridModel& m_model;

    /** Size of the grid. */
    gfx::Size m_gridSizeCells;

    /** CLUT to use. */
    GfxTtxClut& m_clut;

    /** Map with settings for different zoom modes. */
    std::map<ZoomMode, ModeSettings> m_modeSettingsMap;

    /** Handler for charset operations. */
    CharsetHandler m_charsetHandler;

    /** GFX elements - Font strip for G0/G2 charsets (standard chars). */
    gfx::FontStripPtr m_gfxFontStripG0G2;

    /** GFX elements - Font strip for charset G1 (block/separated mosaics). */
    gfx::FontStripPtr m_gfxFontStripG1;

    /** GFX elements - Font strip map for charset G1 (block/separated mosaics). */
    gfx::FontStripMap m_gfxFontStripMapG1;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_TTX_GRID_HPP_*/
