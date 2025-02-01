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


#include "GfxTtxGrid.hpp"

#include <ttxdecoder/CharacterMarker.hpp>
#include <subttxrend/common/Logger.hpp>

#include "GfxConfig.hpp"
#include "GfxTtxGridModel.hpp"
#include "GfxTtxClut.hpp"
#include "GfxMosaicGenerator.hpp"

namespace subttxrend
{
namespace ttxt
{

namespace
{

common::Logger g_logger("Ttxt", "GfxTtxGrid");

}

GfxTtxGrid::GfxTtxGrid(GfxTtxGridModel& model,
                       GfxTtxClut& clut) :
        m_model(model),
        m_gridSizeCells(model.getSize()),
        m_clut(clut)
{
    g_logger.trace("%s", __func__);
}

void GfxTtxGrid::init(const ttxdecoder::Engine& ttxEngine,
                      const gfx::EnginePtr& gfxEngine,
                      const GfxConfig& config)
{
    g_logger.trace("%s", __func__);

    m_charsetHandler.init(ttxEngine);

    loadFontG0G2(gfxEngine, config);

    std::tie(m_gfxFontStripG1, m_gfxFontStripMapG1) =
            GfxMosaicGenerator::generateStripG1(ttxEngine, gfxEngine);
}

void GfxTtxGrid::shutdown()
{
    g_logger.trace("%s", __func__);

    m_gfxFontStripG0G2.reset();

    m_charsetHandler.shutdown();
}

void GfxTtxGrid::clearModeSettings()
{
    g_logger.trace("%s", __func__);

    m_modeSettingsMap.clear();
}

void GfxTtxGrid::addModeSettings(ZoomMode mode,
                                 const RowRange& rowRange,
                                 const Rect& gridRectangle)
{
    g_logger.trace("%s mode=%d", __func__, static_cast<int>(mode));

    m_modeSettingsMap[mode] = ModeSettings(rowRange, gridRectangle);
}

void GfxTtxGrid::draw(ZoomMode mode,
                      gfx::DrawContext& context,
                      std::uint8_t bgAlpha)
{
    g_logger.trace("%s", __func__);

    RowRange rowRange;
    Rect bounds;

    auto iter = m_modeSettingsMap.find(mode);
    if (iter != m_modeSettingsMap.end())
    {
        rowRange = iter->second.m_rowRange;
        bounds = iter->second.m_gridRectangle;
    }

    // drawing rows in bottom-to-top, right-to-left order to correctly
    // render double size characters
    auto rowLimit = static_cast<std::int32_t>(rowRange.m_first
            + rowRange.m_count);
    for (std::int32_t i = static_cast<std::int32_t>(rowRange.m_count) - 1;
            i >= 0; --i)
    {
        auto y = rowRange.m_first + i;

        for (std::int32_t x = m_gridSizeCells.m_w - 1; x >= 0; --x)
        {
            drawCell(x, y, bounds, context, bgAlpha, rowLimit);
        }
    }
}

void GfxTtxGrid::loadFontG0G2(const gfx::EnginePtr& gfxEngine,
                              const GfxConfig& config)
{
    g_logger.trace("%s", __func__);

    const auto& fontInfoG0G2 = config.getFontInfoG0G2();

    m_gfxFontStripG0G2 = gfxEngine->createFontStrip(fontInfoG0G2.m_glyphSize,
            m_charsetHandler.getMapping().getNeededGlyphCount());
    if (!m_gfxFontStripG0G2)
    {
        // TODO: better exception
        throw std::logic_error("Cannot create G0/G2 font strip");
    }

    if (!m_gfxFontStripG0G2->loadFont(fontInfoG0G2.m_name,
            fontInfoG0G2.m_charSize, m_charsetHandler.getMapping()))
    {
        // TODO: better exception
        throw std::logic_error("Cannot load G0/G2 font strip");
    }
}

void GfxTtxGrid::drawCell(std::int32_t x,
                          std::int32_t y,
                          const Rect& bounds,
                          gfx::DrawContext& context,
                          std::uint8_t bgAlpha,
                          std::int32_t rowLimit)
{
    auto cell = m_model.getConstCell(x, y);
    if (!cell)
    {
        return;
    }

    std::int32_t tx = std::min(x + cell->getXMultiplier(), m_gridSizeCells.m_w);
    std::int32_t ty = std::min(y + cell->getYMultiplier(), m_gridSizeCells.m_h);
    ty = std::min(ty, rowLimit);

    gfx::Rectangle cellRect;

    cellRect.m_x = bounds.m_x + (x * bounds.m_w) / m_gridSizeCells.m_w;
    cellRect.m_y = bounds.m_y + (y * bounds.m_h) / m_gridSizeCells.m_h;
    cellRect.m_w = bounds.m_x + (tx * bounds.m_w) / m_gridSizeCells.m_w;
    cellRect.m_h = bounds.m_y + (ty * bounds.m_h) / m_gridSizeCells.m_h;
    cellRect.m_w -= cellRect.m_x;
    cellRect.m_h -= cellRect.m_y;

    drawCell(cell, cellRect, context, bgAlpha);
}

uint32_t GfxTtxGrid::blendAlpha(std::uint32_t argb,
                                std::uint8_t bgAlpha)
{
    auto alpha = (((((argb >> 24) + 1) * bgAlpha) << 16) & 0xFF000000);
    auto rgb = (argb & 0x00FFFFFF);
    return alpha | rgb;
}

uint32_t GfxTtxGrid::getColor(std::uint8_t colorIndex) const
{
    if (colorIndex >= m_clut.getSize())
    {
        colorIndex = 0;
    }

    return m_clut.getArray()[colorIndex];
}

void GfxTtxGrid::drawCell(const GfxTtxGridCell* cell,
                          const Rect& rect,
                          gfx::DrawContext& context,
                          std::uint8_t bgAlpha)
{
    if (cell->startRedraw())
    {
        if (cell->isEnabled())
        {
            gfx::ColorArgb fgColor(getColor(cell->getFgColor()));
            gfx::ColorArgb bgColor(
                    blendAlpha(getColor(cell->getBgColor()), bgAlpha));

            context.fillRectangle(bgColor, rect);

            drawCellChar(cell->getChar(), rect, fgColor, bgColor, context);
        }
        else
        {
            context.fillRectangle(gfx::ColorArgb::TRANSPARENT, rect);
        }
    }
}

void GfxTtxGrid::drawCellChar(std::uint16_t ch,
                              const Rect& rect,
                              gfx::ColorArgb fgColor,
                              gfx::ColorArgb bgColor,
                              gfx::DrawContext& context)
{
    g_logger.trace("%s - char=%d (%c) pos=%d,%d", __func__, ch, ch, rect.m_x,
            rect.m_y);

    bool result = true;

    std::uint16_t charMask = ch & 0xFF00;

    if (charMask == ttxdecoder::CharacterMarker::MASK_BLOCK_MOSAIC)
    {
        auto glyph = m_gfxFontStripMapG1.toGlyphIndex(ch);
        if (glyph < 0)
        {
            return;
        }

        context.drawGlyph(m_gfxFontStripG1, glyph, rect, fgColor, bgColor);
    }
    else if (charMask == ttxdecoder::CharacterMarker::MASK_SEPARATE_MOSAIC)
    {
        auto glyph = m_gfxFontStripMapG1.toGlyphIndex(ch);
        if (glyph < 0)
        {
            return;
        }

        context.drawGlyph(m_gfxFontStripG1, glyph, rect, fgColor, bgColor);
    }
    else if (charMask == ttxdecoder::CharacterMarker::MASK_SMOOTH_MOSAIC)
    {
        // not supported
        // draw_glyph(m_gfxFontStripSM, ch - 0xEF20, x, y, width, height);
    }
    else if (charMask == ttxdecoder::CharacterMarker::MASK_DRCS)
    {
        // not supported
        // draw_glyph(m_gfxFontStripDRCS, ch & 0x001F, x, y, width, height);
    }
    else
    {
        auto glyph = m_charsetHandler.getMapping().toGlyphIndex(ch);
        if (glyph < 0)
        {
            return;
        }

        context.drawGlyph(m_gfxFontStripG0G2, glyph, rect, fgColor, bgColor);
    }
}

} // namespace ttxt
} // namespace subttxrend
