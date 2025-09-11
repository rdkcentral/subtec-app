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


#ifndef SUBTTXREND_TTXT_GFX_RENDERER_HPP_
#define SUBTTXREND_TTXT_GFX_RENDERER_HPP_

#include <chrono>
#include <queue>
#include <mutex>
#include <memory>

#include <ttxdecoder/Engine.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/KeyEventListener.hpp>

#include "GfxTypes.hpp"
#include "GfxRendererClient.hpp"
#include "GfxTtxClut.hpp"
#include "GfxTtxGrid.hpp"
#include "GfxTtxGridModel.hpp"
#include "GfxConfig.hpp"

namespace subttxrend
{
namespace ttxt
{

/**
 * Graphics renderer.
 *
 * The renderer is currently a singleton. Only one client should use it
 * at the same time. The number of inits/shutdowns is calculated and only
 * called when needed.
 */
class GfxRenderer : private gfx::KeyEventListener
{
public:
    /**
     * Returns renderer singleton.
     *
     * @return
     *      Renderer object.
     */
    static GfxRenderer& getSingleton();

    /**
     * Destructor.
     */
    ~GfxRenderer();

    /**
     * Initializes the renderer if needed.
     *
     * @param client
     *      Client (used to init configuration).
     */
    void gfxInit(GfxRendererClient* client);

    /**
     * Shuts down the renderer if needed.
     */
    void gfxShutdown();

    /**
     * Shows the renderer.
     *
     * @param client
     *      Client that wants to use the renderer.
     */
    void gfxShow(GfxRendererClient* client);

    /**
     * Hides the renderer.
     *
     * @param client
     *      Client that wants to use the renderer.
     */
    void gfxHide(GfxRendererClient* client);

    /**
     * Draws the renderer.
     *
     * @param client
     *      Client that wants to use the renderer.
     * @param updateHeader
     *      Header data should be updated flag.
     * @param updatePage
     *      Page data should be updated flag.
     */
    void gfxDraw(GfxRendererClient* client,
                 bool updateHeader,
                 bool updatePage);

private:
    /** Monotonic clock. */
    using SteadyClock = std::chrono::steady_clock;
    /** Monotonic clock time. */
    using TimePoint = SteadyClock::time_point;

    /** Draw flags - update page. */
    static const std::uint8_t UPDATE_PAGE = (1 << 0);
    /** Draw flags - update header. */
    static const std::uint8_t UPDATE_HEADER = (1 << 1);
    /** Draw flags - update selection. */
    static const std::uint8_t UPDATE_SELECTION = (1 << 2);
    /** Draw flags - repaint grid. */
    static const std::uint8_t REPAINT_GRID = (1 << 3);
    /** Draw flags - repaint everything (including background). */
    static const std::uint8_t REPAINT_ALL = (1 << 4);

    /**
     * Constructor.
     */
    GfxRenderer();

    /**
     * Initializes the renderer.
     *
     * @param client
     *      Client (used to init configuration).
     */
    void initInternal(GfxRendererClient* client);

    /**
     * Deinitializes the renderer.
     */
    void shutdownInternal();

    /**
     * Shows the renderer.
     */
    void showInternal();

    /**
     * Hides the renderer.
     */
    void hideInternal();

    /**
     * Draws the renderer.
     *
     * @param flags
     *      Set of draw flags.
     */
    void drawInternal(std::uint8_t flags);

    /**
     * Repaints the background.
     */
    void repaintBackground();

    /**
     * Resets the render state to defaults.
     */
    void resetRenderState();

    /**
     * Refreshes the CLUT.
     */
    void refreshColors();

    /**
     * Refreshes background mode information.
     *
     * @return
     *      True if mode changed.
     */
    bool refreshBgMode();

    /**
     * Processes key press.
     *
     * @param keySymbol
     *      Key symbol to process.
     */
    void processKey(xkb_keysym_t keySymbol);

    /**
     * Processes NEXT key.
     */
    void onKeyNextPage();

    /**
     * Processes PREV key.
     */
    void onKeyPrevPage();

    /**
     * Processes GOTO LINK key.
     *
     * @param link
     *      Link type to follow.
     */
    void onKeyGotoLink(ttxdecoder::DecodedPage::Link link);

    /**
     * Processes DIGIT key.
     *
     * @param digit
     *      Digit value (0-9).
     */
    void onKeyDigit(std::uint16_t digit);

    /**
     * Processes PAUSE key.
     */
    void onKeyTogglePause();

    /**
     * Processes CHANGE ZOOM key.
     */
    void onKeyChangeZoomMode();

    /**
     * Processes CHANGE BACKGROUND ALPHA key.
     */
    void onKeyChangeBackgroundAlpha();

    /**
     * Processes REVEAL key.
     */
    void onKeyToggleReveal();

    /**
     * Sets new current page.
     *
     * @param pageId
     *      Page to set.
     */
    void setNewCurrentPage(const ttxdecoder::PageId& pageId);

    /**
     * Processes key event.
     *
     * @param event
     *      Key event descriptor.
     */
    virtual void onKeyEvent(const gfx::KeyEvent& event) override;

    /** Number of initializations. */
    int m_initCount{};

    /** Current client. */
    GfxRendererClient* m_currentClient{};

    /** GFX elements - window. */
    gfx::Window* m_gfxWindow{};

    /** Current background alpha. */
    std::uint8_t m_bgAlpha{};

    /** Current new page id (selected using DIGIT keys). */
    std::uint16_t m_newPageId{};

    /** Last renderer start (show) time. */
    TimePoint m_startTime;

    /** Last digit select time. */
    TimePoint m_lastDigitTime;

    /** Rendering paused. */
    bool m_paused{};

    /** Transparent background mode (subtitles/news). */
    bool m_transparentMode{};

    /** Current zoom mode. */
    ZoomMode m_zoomMode{ZoomMode::NONE};

    /** Teletext CLUT. */
    GfxTtxClut m_clut;

    /** Teletext grid model. */
    GfxTtxGridModel m_gridModel;

    /** Teletext grid. */
    GfxTtxGrid m_grid;

    /** Configuration. */
    GfxConfig m_config;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_RENDERER_HPP_*/
