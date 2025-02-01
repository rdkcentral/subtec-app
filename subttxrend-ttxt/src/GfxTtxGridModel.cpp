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


#include "GfxTtxGridModel.hpp"

#include <subttxrend/common/Logger.hpp>
#include <ttxdecoder/Property.hpp>
#include <ttxdecoder/ControlInfo.hpp>

#include "GfxTtxClut.hpp"

namespace subttxrend
{
namespace ttxt
{

namespace
{

common::Logger g_logger("Ttxt", "GfxTtxGridModel");

}

GfxTtxGridModel::GfxTtxGridModel(const Size& size) :
        m_size(size),
        m_dataSource(nullptr),
        m_flashEnabled(false),
        m_revealEnabled(false)
{
    m_cells.resize(size.m_w * size.m_h);
}

void GfxTtxGridModel::init(const ttxdecoder::Engine* dataSource)
{
    m_dataSource = dataSource;
}

void GfxTtxGridModel::clearAll(bool subtitlesMode)
{
    auto bgColor =
            subtitlesMode ?
                    GfxTtxClut::COLOR_INDEX_TRANSPARENT :
                    GfxTtxClut::COLOR_INDEX_BLACK;

    for (auto& cell : m_cells)
    {
        cell.clear(bgColor);
    }
}

void GfxTtxGridModel::markAllAsChanged()
{
    for (auto& cell : m_cells)
    {
        cell.markChanged();
    }
}

void GfxTtxGridModel::markChangedByColor(std::uint8_t colorIndex)
{
    for (auto& cell : m_cells)
    {
        cell.markChangedByColor(colorIndex);
    }
}

void GfxTtxGridModel::refreshPage(const bool subtitlesMode)
{
    refreshData(0, m_size.m_h - 1, subtitlesMode);
}

void GfxTtxGridModel::refreshHeader(const bool subtitlesMode)
{
    refreshData(0, 0, subtitlesMode);
}

void GfxTtxGridModel::refreshSelection(bool paused,
                                       std::uint16_t newPageId)
{
    g_logger.trace("%s", __func__);

    std::array<char, 16> charBuffer;

    if (!preparePageNumberString(charBuffer, paused, newPageId))
    {
        return;
    }

    for (std::uint32_t col = 0; col < 4; ++col)
    {
        updateChar(col, 0, charBuffer[col], GfxTtxClut::COLOR_INDEX_WHITE,
                GfxTtxClut::COLOR_INDEX_BLACK, 0);
    }
}

void GfxTtxGridModel::disableRow(std::int32_t row)
{
    for (std::int32_t col = 0; col < m_size.m_w; ++col)
    {
        disableCell(col, row);
    }
}

void GfxTtxGridModel::disableCell(std::int32_t col,
                                  std::int32_t row)
{
    auto cell = getCell(col, row);
    if (cell)
    {
        cell->setEnabled(false);
    }
}

const GfxTtxGridCell* GfxTtxGridModel::getConstCell(std::int32_t col,
                                                    std::int32_t row) const
{
    if ((col < 0) || (row < 0) || (col >= m_size.m_w) || (row >= m_size.m_h))
    {
        return nullptr;
    }

    return &m_cells[row * m_size.m_w + col];
}

GfxTtxGridCell* GfxTtxGridModel::getCell(std::int32_t col,
                                         std::int32_t row)
{
    if ((col < 0) || (row < 0) || (col >= m_size.m_w) || (row >= m_size.m_h))
    {
        return nullptr;
    }

    return &m_cells[row * m_size.m_w + col];
}

void GfxTtxGridModel::refreshData(std::uint32_t startRow,
                                  std::uint32_t endRow,
                                  const bool subtitlesMode)
{
    g_logger.trace("%s - range=%u..%u", __func__, startRow, endRow);

    // reverse order is important due to disabling neighbours
    // for double-sized characters

    auto& page = m_dataSource->getPage();

    const auto controlInfo = page.getPageControlInfo();

    bool hideData = (controlInfo & ttxdecoder::ControlInfo::INHIBIT_DISPLAY);
    bool hideHeader = (controlInfo & ttxdecoder::ControlInfo::SUPRESS_HEADER);

    const bool subtitlePage = (controlInfo & ttxdecoder::ControlInfo::SUBTITLE);

    if (subtitlesMode && !subtitlePage)
    {
        hideHeader = true;
        hideData = true;
    }

    for (auto row = endRow; row >= startRow;)
    {
        if (row >= page.getRowCount())
        {
            // row outside range (safety check) - do not draw
            disableRow(row);
        }
        else if ((row == 0) && hideHeader)
        {
            // header suppressed - do not draw
            disableRow(row);
        }
        else if ((row > 0) && hideData)
        {
            // display inhibited - do not draw
            disableRow(row);
        }
        else
        {
            updateRow(page, row);
        }

        if (row > 0)
        {
            --row;
        }
        else
        {
            break;
        }
    }

    if ((startRow <= LINKS_ROW) && (LINKS_ROW <= endRow))
    {
        if (!hideData && !subtitlesMode)
        {
            updateLinksRow(page);
        }
    }
}

bool GfxTtxGridModel::preparePageNumberString(std::array<char, 16>& charBuffer,
                                              bool paused,
                                              std::uint16_t newPageId)
{
    if (!paused)
    {
        const auto& page = m_dataSource->getPage();

        std::uint16_t pageId = 0;

        if (newPageId != 0)
        {
            pageId = newPageId;
        }
        else
        {
            const auto controlInfo = page.getPageControlInfo();
            const bool hideHeader = (controlInfo
                    & ttxdecoder::ControlInfo::SUPRESS_HEADER);
            if (hideHeader)
            {
                // hidden, do not update
                return false;
            }

            pageId = m_dataSource->getPageId(
                    ttxdecoder::PageIdType::CURRENT_PAGE).getMagazinePage();
        }

        if (pageId != 0)
        {
            snprintf(charBuffer.data(), charBuffer.size(), "P%X--", pageId);
        }
        else
        {
            snprintf(charBuffer.data(), charBuffer.size(), "P???");
        }
    }
    else
    {
        snprintf(charBuffer.data(), charBuffer.size(), "PAUS");
    }

    return true;
}

void GfxTtxGridModel::updateRow(const ttxdecoder::DecodedPage& page,
                                std::uint32_t row)
{
    g_logger.trace("%s - row=%u", __func__, row);

    const auto& rowData = page.getRow(row).m_levelOnePageSegment;

    for (std::int32_t col = 0; col < m_size.m_w; ++col)
    {
        const auto character = rowData.m_charArray[col];
        const auto fgColor = rowData.m_fgColorIndexArray[col];
        const auto bgColor = rowData.m_bgColorIndexArray[col];
        const auto properties = rowData.m_propertiesArray[col];

        updateChar(col, row, character, fgColor, bgColor, properties);
    }
}

void GfxTtxGridModel::updateLinksRow(const ttxdecoder::DecodedPage& page)
{
    g_logger.trace("%s", __func__);

    const auto& rowData = page.getRow(LINKS_ROW).m_levelOnePageSegment;

    for (std::int32_t col = 0; col < m_size.m_w; ++col)
    {
        if (rowData.m_propertiesArray[col] & ttxdecoder::Property::VALUE_HIDDEN)
        {
            continue;
        }

        if ((rowData.m_charArray[col] != '\0')
                && (rowData.m_charArray[col] != ' '))
        {
            g_logger.trace("%s - visible char %d in link row, skipping",
                    __func__, rowData.m_charArray[col]);
            return;
        }
    }

    static const ttxdecoder::DecodedPage::Link LINK_TYPES[4] =
    { ttxdecoder::DecodedPage::Link::RED, ttxdecoder::DecodedPage::Link::GREEN,
            ttxdecoder::DecodedPage::Link::YELLOW,
            ttxdecoder::DecodedPage::Link::CYAN };
    static const std::uint8_t COLOUR_INDEX[4] =
    { GfxTtxClut::COLOR_INDEX_RED, GfxTtxClut::COLOR_INDEX_GREEN,
            GfxTtxClut::COLOR_INDEX_YELLOW, GfxTtxClut::COLOR_INDEX_CYAN };

    for (std::uint32_t linkIndex = 0; linkIndex < 4; ++linkIndex)
    {
        const auto& link = page.getColourKeyLink(LINK_TYPES[linkIndex]);

        g_logger.trace("%s - link %u -> %04X", __func__, linkIndex,
                link.getMagazinePage());

        if (!link.isValidDecimal())
        {
            continue;
        }

        std::array<char, 32> charBuffer;
        snprintf(charBuffer.data(), charBuffer.size(), "   %03X    ",
                link.getMagazinePage());

        const auto fgColorIndex = COLOUR_INDEX[linkIndex];
        const std::uint32_t columnOffset = linkIndex * 10;
        for (std::uint32_t columnIndex = 0; columnIndex < 10; ++columnIndex)
        {
            const std::uint32_t column = columnOffset + columnIndex;

            updateChar(column, LINKS_ROW, charBuffer[columnIndex], fgColorIndex,
                    rowData.m_bgColorIndexArray[column],
                    rowData.m_propertiesArray[column]);
        }
    }
}

void GfxTtxGridModel::updateChar(std::uint16_t x,
                                 std::uint16_t y,
                                 std::uint16_t character,
                                 std::uint8_t fgColor,
                                 std::uint8_t bgColor,
                                 std::uint16_t properties)
{
    const bool flash = properties & ttxdecoder::Property::VALUE_FLASH;
    const bool doubleWidth = properties
            & ttxdecoder::Property::VALUE_DOUBLE_WIDTH;
    const bool doubleHeight = properties
            & ttxdecoder::Property::VALUE_DOUBLE_HEIGHT;
    const bool concealed = properties & ttxdecoder::Property::VALUE_CONCEALED;
    const bool hidden = properties & ttxdecoder::Property::VALUE_HIDDEN;
    const std::uint16_t diacritic = properties
            & ttxdecoder::Property::MASK_DIACRITIC;

    const bool enabled = (!hidden) && (character != '\0');

    auto cell = getCell(x, y);
    if (cell)
    {
        cell->setEnabled(enabled);
        if (enabled)
        {
            if (flash && m_flashEnabled)
            {
                fgColor = bgColor;
            }

            auto mappedCharacter = CharsetHandler::getDiacriticCharacterCode(
                    character, diacritic);

            if (concealed && !m_revealEnabled)
            {
                mappedCharacter = '*';
            }

            cell->setCharacter(mappedCharacter);
            cell->setSize(doubleWidth ? 2 : 1, doubleHeight ? 2 : 1);
            cell->setColors(fgColor, bgColor);

            // Disable neighbor cells for double width & height characters
            if (doubleWidth)
            {
                auto neighCell = getCell(x + 1, y);
                if (neighCell)
                {
                    neighCell->setHidden(true);
                }
            }
            if (doubleHeight)
            {
                auto neighCell = getCell(x, y + 1);
                if (neighCell)
                {
                    neighCell->setHidden(true);
                }
            }
            if (doubleWidth && doubleHeight)
            {
                auto neighCell = getCell(x + 1, y + 1);
                if (neighCell)
                {
                    neighCell->setHidden(true);
                }
            }
        }
        else
        {
            // ARRISEOS-16434: ensure that disabled cell are not stretched
            cell->setSize(1, 1);
        }
    }
}

} // namespace ttxt
} // namespace subttxrend
