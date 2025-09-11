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


#include "GfxRenderer.hpp"

#include <stdexcept>
#include <subttxrend/common/Logger.hpp>
#include <ttxdecoder/ControlInfo.hpp>
#include <ttxdecoder/Property.hpp>
#include <ttxdecoder/CharacterMarker.hpp>

// Things to be supported for profiles 2.5+ - not needed now:
// TODO: support for G3 (smooth mosaics) - extra font
// TODO: support for DRCS - separate (manual) font strip

namespace subttxrend
{
namespace ttxt
{

namespace
{

static const Size GRID_SIZE_IN_CELLS =
{ 40, 25 };

static std::uint32_t MAX_DIGIT_TIME_MS = 5000;

static const RowRange GRID_RANGE_ZOOM_NONE(0, GRID_SIZE_IN_CELLS.m_h);
static const RowRange GRID_RANGE_ZOOM_TOP(0, (GRID_SIZE_IN_CELLS.m_h + 1) / 2);
static const RowRange GRID_RANGE_ZOOM_BOTTOM(GRID_SIZE_IN_CELLS.m_h / 2,
        GRID_SIZE_IN_CELLS.m_h - (GRID_SIZE_IN_CELLS.m_h / 2));

common::Logger g_logger("Ttxt", "GfxRenderer");

}

GfxRenderer& GfxRenderer::getSingleton()
{
    static GfxRenderer theRenderer;
    return theRenderer;
}

GfxRenderer::GfxRenderer() :
        m_gridModel(GRID_SIZE_IN_CELLS),
        m_grid(m_gridModel, m_clut)
{
    g_logger.trace("%s", __func__);

    m_startTime = SteadyClock::now();
}

GfxRenderer::~GfxRenderer()
{
    g_logger.trace("%s", __func__);

    if (m_initCount > 0)
    {
        g_logger.fatal("%s - Init count is not zero", __func__);
    }
}

void GfxRenderer::gfxInit(GfxRendererClient* client)
{
    g_logger.trace("%s", __func__);

    if (m_initCount > 100)
    {
        throw std::logic_error("Too many inits");
    }

    ++m_initCount;

    if (m_initCount == 1)
    {
        initInternal(client);
    }
}

void GfxRenderer::gfxShutdown()
{
    g_logger.trace("%s", __func__);

    if (m_initCount == 0)
    {
        throw std::logic_error("Not initialized");
    }

    --m_initCount;

    if (m_initCount == 0)
    {
        shutdownInternal();
    }
}

void GfxRenderer::gfxShow(GfxRendererClient* client)
{
    g_logger.trace("%s", __func__);

    if (!client)
    {
        throw std::invalid_argument("client");
    }

    if (m_initCount == 0)
    {
        throw std::logic_error("Not initialized");
    }

    if (m_currentClient)
    {
        throw std::logic_error("Already shown");
    }

    m_currentClient = client;

    showInternal();
}

void GfxRenderer::gfxHide(GfxRendererClient* client)
{
    g_logger.trace("%s", __func__);

    if (!client)
    {
        throw std::invalid_argument("client");
    }

    if (m_initCount == 0)
    {
        throw std::logic_error("Not initialized");
    }

    if (!m_currentClient)
    {
        throw std::logic_error("Not shown");
    }

    if (m_currentClient != client)
    {
        throw std::logic_error("Shown by other client");
    }

    hideInternal();

    m_currentClient = nullptr;
}

void GfxRenderer::gfxDraw(GfxRendererClient* client,
                          bool updateHeader,
                          bool updatePage)
{
//#define VERBOSE_LOGGING 1
#ifdef VERBOSE_LOGGING
    g_logger.trace("%s updateHeader: %d updatePage: %d", __func__, updateHeader, updatePage);
#endif

    if (!client)
    {
        throw std::invalid_argument("client");
    }

    if (m_initCount == 0)
    {
        throw std::logic_error("Not initialized");
    }

    if (!m_currentClient)
    {
        throw std::logic_error("Not shown");
    }

    if (m_currentClient != client)
    {
        throw std::logic_error("Shown by other client");
    }

    std::uint8_t drawFlags = 0;
    drawFlags |= updateHeader ? UPDATE_HEADER : 0;
    drawFlags |= updatePage ? UPDATE_PAGE : 0;

    drawInternal(drawFlags);
}

void GfxRenderer::initInternal(GfxRendererClient* client)
{
    g_logger.trace("%s", __func__);

    if (!client)
    {
        throw std::invalid_argument("client");
    }

    auto config = client->getConfiguration();
    if (!config)
    {
        throw std::logic_error("Client does not provide configuration");
    }

    auto gfxEngine = client->getGfxEngine();
    if (!gfxEngine)
    {
        throw std::logic_error("Client does not provide gfx engine");
    }

    auto dataSource = client->getDataSource();
    if (!dataSource)
    {
        throw std::logic_error("Client does not provide data source");
    }

    auto gfxWindow = client->getGfxWindow();
    if (!gfxWindow)
    {
        throw std::logic_error("Client does not provide window");
    }

    m_config.init(config);

    g_logger.trace("%s - got window %p", __func__, gfxWindow);

    m_gfxWindow = gfxWindow;
    m_gfxWindow->addKeyEventListener(this);

    const Size windowSizePixels = m_config.getWindowSizePixels();
    const Size cellSizePixels = m_config.getGridCellSizePixels();

    Rect gridSizeZoomNone;
    gridSizeZoomNone.m_w = cellSizePixels.m_w * GRID_SIZE_IN_CELLS.m_w;
    gridSizeZoomNone.m_h = cellSizePixels.m_h * GRID_SIZE_IN_CELLS.m_h;
    gridSizeZoomNone.m_x = (windowSizePixels.m_w - gridSizeZoomNone.m_w) / 2;
    gridSizeZoomNone.m_y = (windowSizePixels.m_h - gridSizeZoomNone.m_h) / 2;

    Rect gridSizeZoomTop;
    gridSizeZoomTop.m_w = cellSizePixels.m_w * GRID_SIZE_IN_CELLS.m_w;
    gridSizeZoomTop.m_h = cellSizePixels.m_h * 2 * GRID_SIZE_IN_CELLS.m_h;
    auto topVisibleH = cellSizePixels.m_h * 2 * GRID_RANGE_ZOOM_TOP.m_count;
    gridSizeZoomTop.m_x = (windowSizePixels.m_w - gridSizeZoomTop.m_w) / 2;
    gridSizeZoomTop.m_y = (windowSizePixels.m_h - topVisibleH) / 2;

    Rect gridSizeZoomBottom;
    gridSizeZoomBottom.m_w = cellSizePixels.m_w * GRID_SIZE_IN_CELLS.m_w;
    gridSizeZoomBottom.m_h = cellSizePixels.m_h * 2 * GRID_SIZE_IN_CELLS.m_h;
    auto bottomVisibleH = cellSizePixels.m_h * 2
            * GRID_RANGE_ZOOM_BOTTOM.m_count;
    auto bottomInvisibleH = gridSizeZoomBottom.m_h - bottomVisibleH;
    gridSizeZoomBottom.m_x = (windowSizePixels.m_w - gridSizeZoomBottom.m_w)
            / 2;
    gridSizeZoomBottom.m_y = (windowSizePixels.m_h - bottomVisibleH) / 2
            - bottomInvisibleH;

    m_grid.init(*dataSource, gfxEngine, m_config);
    m_grid.addModeSettings(ZoomMode::NONE, GRID_RANGE_ZOOM_NONE,
            gridSizeZoomNone);
    m_grid.addModeSettings(ZoomMode::TOP, GRID_RANGE_ZOOM_TOP, gridSizeZoomTop);
    m_grid.addModeSettings(ZoomMode::BOTTOM, GRID_RANGE_ZOOM_BOTTOM,
            gridSizeZoomBottom);

    g_logger.info("%s - gfx initialized", __func__);
}

void GfxRenderer::shutdownInternal()
{
    m_grid.shutdown();
    m_gfxWindow->removeKeyEventListener(this);
    m_gfxWindow = nullptr;
}

void GfxRenderer::showInternal()
{
    if (m_gfxWindow)
    {
        m_gfxWindow->setSize(m_config.getWindowSizePixels());

        resetRenderState();

        drawInternal(UPDATE_PAGE | REPAINT_ALL);
        m_gfxWindow->setVisible(true);
    }
}

void GfxRenderer::hideInternal()
{
    if (m_gfxWindow)
    {
        m_gfxWindow->setVisible(false);
    }
}

void GfxRenderer::drawInternal(std::uint8_t flags)
{
    auto currentTime = SteadyClock::now();

    {
        auto flashTimeDiff = currentTime - m_startTime;
        auto flashTimeDiffMs = std::chrono::duration_cast<
                std::chrono::milliseconds>(flashTimeDiff).count();
        auto flashPeriodDiffMs = flashTimeDiffMs % m_config.getFlashPeriodMs();
        auto flashEnabled = (flashPeriodDiffMs
                < (m_config.getFlashPeriodMs() / 2));

        if (m_gridModel.setFlashEnabled(flashEnabled))
        {
            flags |= UPDATE_PAGE;
        }
    }

    if (m_newPageId > 0)
    {
        auto digitTimeDiff = currentTime - m_lastDigitTime;
        auto digitTimeDiffMs = std::chrono::duration_cast<
                std::chrono::milliseconds>(digitTimeDiff).count();
        if (digitTimeDiffMs > MAX_DIGIT_TIME_MS)
        {
            m_newPageId = 0;
            flags |= UPDATE_SELECTION;
        }
    }

    if (!m_paused)
    {
        if (flags & UPDATE_PAGE)
        {
            m_gridModel.refreshPage(m_currentClient->isSubtitlesRenderer());
            refreshColors();
            if (refreshBgMode())
            {
                flags |= REPAINT_ALL;
            }

            flags |= UPDATE_SELECTION;
            flags |= REPAINT_GRID;
        }
        else if (flags & UPDATE_HEADER)
        {
            m_gridModel.refreshHeader(m_currentClient->isSubtitlesRenderer());
            if (refreshBgMode())
            {
                flags |= REPAINT_ALL;
            }

            flags |= UPDATE_SELECTION;
            flags |= REPAINT_GRID;
        }
    }

    if (!m_currentClient->isSubtitlesRenderer())
    {
        if (flags & UPDATE_SELECTION)
        {
            m_gridModel.refreshSelection(m_paused, m_newPageId);
            flags |= REPAINT_GRID;
        }
    }

    if (flags & REPAINT_ALL)
    {
        repaintBackground();

        m_gridModel.markAllAsChanged();

        flags |= REPAINT_GRID;
    }

    if (flags & REPAINT_GRID)
    {
        m_grid.draw(m_zoomMode, m_gfxWindow->getDrawContext(), m_bgAlpha);

        m_gfxWindow->update();
    }
}

bool GfxRenderer::refreshBgMode()
{
    bool transparentMode = false;

    if (m_currentClient->isSubtitlesRenderer())
    {
        transparentMode = true;
    }
    else
    {
        auto dataSource = m_currentClient->getDataSource();
        auto controlInfo = dataSource->getPageControlInfo();
        if ((controlInfo & ttxdecoder::ControlInfo::SUBTITLE)
                || (controlInfo & ttxdecoder::ControlInfo::NEWSFLASH))
        {
            transparentMode = true;
        }
    }

    if (m_transparentMode != transparentMode)
    {
        m_transparentMode = transparentMode;
        return true;
    }
    else
    {
        return false;
    }
}

void GfxRenderer::repaintBackground()
{
    gfx::ColorArgb screenColour(m_bgAlpha, 0x00, 0x00, 0x00);

    if (m_transparentMode)
    {
        screenColour = gfx::ColorArgb(0x00, 0x00, 0x00, 0x00);
    }

    m_gfxWindow->getDrawContext().fillRectangle(screenColour,
            m_gfxWindow->getBounds());
    // no update, would be done later
}

void GfxRenderer::refreshColors()
{
    auto dataSource = m_currentClient->getDataSource();

    std::array<std::uint32_t, 16> dynamicClut;

    if (!dataSource->getColors(dynamicClut))
    {
        return;
    }

    std::size_t offset = 16;
    for (std::size_t i = 0; i < dynamicClut.size(); ++i)
    {
        if (m_clut.setColor(i + offset, dynamicClut[i]))
        {
            m_gridModel.markChangedByColor(i + offset);
        }
    }
}

void GfxRenderer::resetRenderState()
{
    m_newPageId = 0;

    m_bgAlpha = m_config.getDefaultBackgroundAlpha();

    m_clut.resetColors();

    m_gridModel.init(m_currentClient->getDataSource());
    m_gridModel.clearAll(m_currentClient->isSubtitlesRenderer());

    m_zoomMode = ZoomMode::NONE;

    m_transparentMode = false;
    m_paused = false;

    m_gridModel.setRevealEnabled(false);
}

void GfxRenderer::processKey(xkb_keysym_t keySymbol)
{
    switch (keySymbol)
    {
    case XKB_KEY_Up:
        onKeyNextPage();
        break;

    case XKB_KEY_Down:
        onKeyPrevPage();
        break;

    case XKB_KEY_Left:
        onKeyChangeZoomMode();
        break;

    case XKB_KEY_Right:
        onKeyTogglePause();
        break;

    case XKB_KEY_Return:
        onKeyChangeBackgroundAlpha();
        break;

    case XKB_KEY_XF86AudioPlay:
        onKeyToggleReveal();
        break;

    case XKB_KEY_XF86Red:
        onKeyGotoLink(ttxdecoder::DecodedPage::Link::RED);
        break;

    case XKB_KEY_XF86Green:
        onKeyGotoLink(ttxdecoder::DecodedPage::Link::GREEN);
        break;

    case XKB_KEY_XF86Yellow:
        onKeyGotoLink(ttxdecoder::DecodedPage::Link::YELLOW);
        break;

    case XKB_KEY_XF86Blue:
        onKeyGotoLink(ttxdecoder::DecodedPage::Link::CYAN);
        break;

    case XKB_KEY_0:
        onKeyDigit(0);
        break;

    case XKB_KEY_1:
        onKeyDigit(1);
        break;

    case XKB_KEY_2:
        onKeyDigit(2);
        break;

    case XKB_KEY_3:
        onKeyDigit(3);
        break;

    case XKB_KEY_4:
        onKeyDigit(4);
        break;

    case XKB_KEY_5:
        onKeyDigit(5);
        break;

    case XKB_KEY_6:
        onKeyDigit(6);
        break;

    case XKB_KEY_7:
        onKeyDigit(7);
        break;

    case XKB_KEY_8:
        onKeyDigit(8);
        break;

    case XKB_KEY_9:
        onKeyDigit(9);
        break;

    default:
        break;
    }
}

void GfxRenderer::onKeyNextPage()
{
    auto dataSource = m_currentClient->getDataSource();
    auto currentPage = dataSource->getPageId(
            ttxdecoder::PageIdType::CURRENT_PAGE);
    auto newPage = dataSource->getNextPageId(currentPage);
    if (newPage.isValidDecimal())
    {
        setNewCurrentPage(newPage);
    }
    else
    {
        g_logger.warning("%s - cannot calculate next page", __func__);
    }
}

void GfxRenderer::onKeyPrevPage()
{
    auto dataSource = m_currentClient->getDataSource();
    auto currentPage = dataSource->getPageId(
            ttxdecoder::PageIdType::CURRENT_PAGE);
    auto newPage = dataSource->getPrevPageId(currentPage);
    if (newPage.isValidDecimal())
    {
        setNewCurrentPage(newPage);
    }
    else
    {
        g_logger.warning("%s - cannot calculate previous page", __func__);
    }
}

void GfxRenderer::onKeyGotoLink(ttxdecoder::DecodedPage::Link link)
{
    auto dataSource = m_currentClient->getDataSource();
    auto& page = dataSource->getPage();
    auto newPage = page.getColourKeyLink(link);

    if (newPage.isValidDecimal())
    {
        setNewCurrentPage(newPage);
    }
}

void GfxRenderer::onKeyDigit(std::uint16_t digit)
{
    if (digit > 9)
    {
        g_logger.warning("%s - invalid digit: %u", __func__, digit);
        return;
    }

    m_paused = false;

    m_lastDigitTime = SteadyClock::now();

    m_newPageId *= 0x10;
    m_newPageId += digit;

    // first digit cannot be 0 or 9
    if (m_newPageId == 9)
    {
        m_newPageId = 8;
    }
    else if (m_newPageId == 0)
    {
        m_newPageId = 1;
    }

    if (m_newPageId >= 0x100)
    {
        auto newPage = ttxdecoder::PageId(m_newPageId,
                ttxdecoder::PageId::ANY_SUBPAGE);

        // draws internally
        setNewCurrentPage(newPage);
    }
    else
    {
        drawInternal(UPDATE_SELECTION);
    }
}

void GfxRenderer::onKeyTogglePause()
{
    m_paused = !m_paused;

    if (m_paused)
    {
        drawInternal(UPDATE_SELECTION);
    }
    else
    {
        drawInternal(UPDATE_PAGE | UPDATE_SELECTION);
    }
}

void GfxRenderer::onKeyChangeZoomMode()
{
    switch (m_zoomMode)
    {
    case ZoomMode::NONE:
        m_zoomMode = ZoomMode::TOP;
        break;

    case ZoomMode::TOP:
        m_zoomMode = ZoomMode::BOTTOM;
        break;

    case ZoomMode::BOTTOM:
        m_zoomMode = ZoomMode::NONE;
        break;

    default:
        m_zoomMode = ZoomMode::NONE;
        break;
    }

    g_logger.trace("%s - mode=%d", __func__, static_cast<int>(m_zoomMode));

    drawInternal(REPAINT_ALL);
}

void GfxRenderer::onKeyChangeBackgroundAlpha()
{
    switch (m_bgAlpha)
    {
    case 0xFF:
        m_bgAlpha = 0xCA;
        break;

    case 0xCA:
        m_bgAlpha = 0x80;
        break;

    case 0x80:
        m_bgAlpha = 0x40;
        break;

    case 0x40:
        m_bgAlpha = 0x00;
        break;

    default:
        m_bgAlpha = 0xFF;
        break;
    }

    drawInternal(REPAINT_ALL);
}

void GfxRenderer::onKeyToggleReveal()
{
    m_gridModel.toggleRevealEnabled();

    drawInternal(UPDATE_PAGE);
}

void GfxRenderer::setNewCurrentPage(const ttxdecoder::PageId& pageId)
{
    m_currentClient->setCurrentPage(pageId);

    m_newPageId = 0;
    m_paused = false;

    m_gridModel.setRevealEnabled(false);

    drawInternal(UPDATE_PAGE);
}

void GfxRenderer::onKeyEvent(const gfx::KeyEvent& event)
{
    if ((!m_currentClient) || (m_currentClient->isSubtitlesRenderer()))
    {
        return;
    }

    if (event.getType() == gfx::KeyEvent::Type::RELEASED)
    {
        return;
    }

    g_logger.trace("%s - key=%08X", __func__, event.getSymbol());

    processKey(event.getSymbol());
}

} // namespace ttxt
} // namespace subttxrend
