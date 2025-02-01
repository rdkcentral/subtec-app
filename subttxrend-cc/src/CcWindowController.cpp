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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#include "CcWindowController.hpp"

namespace subttxrend
{
namespace cc
{

WindowController::WindowController(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache)
        : m_gfx(gfx),
          m_fontCache(fontCache),
          m_selectedWindow(nullptr),
          m_flashTransition(std::chrono::steady_clock::now()),
          m_windowTransition(std::chrono::steady_clock::now()),
          m_flashControl(FlashControl::Show),
          m_windowTimeout(0),
          m_608Enabled(false),
          m_windowTimedout(false),
          logger("ClosedCaptions", "WindowController")
{
}

bool WindowController::hasFlashingText()
{
    bool flashing = false;

    for (auto &window : m_windowsById)
    {
        if (window && window->hasFlashingText())
        {
            flashing = true;
            break;
        }
    }
    return flashing;
}

bool WindowController::redrawFlashingText()
{
    bool redraw = false;

    if (hasFlashingText())
    {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = currentTime - m_flashTransition;
        auto elapsedTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

        // Flash text for 1/4s every second
        redraw = (elapsedTimeMs >= ((m_flashControl == FlashControl::Show) ? 250 : 750));
        if (redraw)
        {
            m_flashTransition = currentTime;
            m_flashControl = (m_flashControl == FlashControl::Show ? FlashControl::Hide : FlashControl::Show);

            for (auto &window : m_windowsById)
            {
                if (window)
                {
                    window->setFlashState(m_flashControl);
                }
            }
        }
    }
    else
    {
        m_flashControl = FlashControl::Show;
        m_flashTransition = std::chrono::steady_clock::now();
    }

    return redraw;
}

void WindowController::drawWindows()
{
    bool clearAndUpdate = false;
    bool redraw = false;
    if (windowTimeout())
    {
	reset();
	return;
    }

    if (!redraw)
    {
        redraw = redrawFlashingText(); // redraw if we need to update flashing text
    }

    if (!redraw)
    {
        for (auto &window : m_windowsById)
        {
            if (window && window->changed())
            {
                if (window->isVisible())
                {
                    redraw = true;
                    clearAndUpdate = false;
                    break;
                }
                else if (window->windowVisibilityChanged())
                {
                    clearAndUpdate = true;
                    window->draw();
                }
            }
        }
    }

    if (!redraw)
    {
        if (clearAndUpdate)
        {
            m_gfx->clear();
            m_gfx->update();
        }
        return;
    }

    m_gfx->clear();

    // Nested loop of maximum of 64 iterations in total. Not worth optimizing.
    for (int priority = MAX_WINDOWS -1; priority >= 0; --priority)
    {
        for (auto &window : m_windowsById)
        {
            if (window && window->getPriority() == priority)
            {
                window->draw();
            }
        }
    }
    m_gfx->update();
    m_windowTransition = std::chrono::steady_clock::now();
}

void WindowController::setCurrentWindow(uint8_t id)
{
    m_selectedWindow = m_windowsById[id].get();
    logger.trace("setCurrentWindow(%d): %p", id, m_selectedWindow);
}

void WindowController::clearWindows(WindowsMap wm)
{
    logger.trace("%s", __func__);
    executeOnWindows(wm, &Window::clear);
}

void WindowController::defineWindow(const WindowDefinition &wd)
{
    if (wd.id >= MAX_WINDOWS or wd.priority >= MAX_WINDOWS)
        return;

    m_userSettingsCtrl.onEmbeddedPenColor(wd.pen_style.pen_color);
    m_userSettingsCtrl.onEmbeddedPenAttributes(wd.pen_style);
    m_userSettingsCtrl.onEmbeddedWindowAttributes(wd.win_style);

    auto windowDef = wd;

    windowDef.win_style = m_userSettingsCtrl.getWindowAttributes();
    windowDef.pen_style = m_userSettingsCtrl.getPenAttributes();
    windowDef.pen_style.pen_color = m_userSettingsCtrl.getPenColor();
    logger.debug("modyfying configuration from defineWindow with user defined style attributes");

    auto& window = m_windowsById[wd.id];
    if (window)
    {
        window->update(windowDef);
    }
    else
    {
        window.reset(createWindow(m_gfx, windowDef));
    }
    setCurrentWindow(windowDef.id);
    m_windowTimedout = false;
}

void WindowController::deleteWindows(WindowsMap wm)
{
    logger.trace("DELETE ");
    for (auto& window : m_windowsById)
    {
        if (window)
        {
            if (wm[window->ID()])
            {
                if (window.get() == m_selectedWindow)
                {
                    m_selectedWindow = nullptr;
                    m_gfx->clear();
                    m_gfx->update();
                }
                window.reset(nullptr);
            }
        }
    }
}

void WindowController::displayWindows(WindowsMap wm)
{
    logger.trace("DISPLAY");
    executeOnWindows(wm, &Window::show);
    if (!m_608Enabled)
        drawWindows();
}

void WindowController::toggleWindows(WindowsMap wm)
{
    logger.trace("TOGGLE ");
    executeOnWindows(wm, &Window::toggle);
    if (!m_608Enabled)
        drawWindows();
}

void WindowController::setWindowAttributes(WindowAttributes attr)
{
    m_userSettingsCtrl.onEmbeddedWindowAttributes(attr);
    if (m_selectedWindow)
        m_selectedWindow->setWindowAttributes(m_userSettingsCtrl.getWindowAttributes());
}

void WindowController::report(std::string str)
{
    logger.trace("ses ");
    if (m_selectedWindow)
        m_selectedWindow->report(str);
}

void WindowController::hideWindows(WindowsMap wm)
{
    logger.trace("%s", __func__);
    executeOnWindows(wm, &Window::hide);
    if (!m_608Enabled)
        drawWindows();
}

void WindowController::reset()
{
	logger.trace("%s", __func__);
	deleteWindows(toWindowsMap(0xff));

}

void WindowController::formFeed()
{
    if (m_selectedWindow)
    {
        m_selectedWindow->clear();
        m_selectedWindow->setPenLocation(0, 0);
    }
}

void WindowController::carriageReturn()
{
    if (m_selectedWindow)
        m_selectedWindow->carriageReturn();
}

void WindowController::horizontalCarriageReturn()
{
    if (m_selectedWindow)
        m_selectedWindow->horizontalCarriageReturn();
}

void WindowController::clearWindow()
{
    if (m_selectedWindow)
        m_selectedWindow->clear();
}

void WindowController::backspace()
{
    if (m_selectedWindow)
        m_selectedWindow->backspace();
}

void WindowController::transparentSpace(bool nonbreaking)
{
    if (m_selectedWindow)
        m_selectedWindow->transparentSpace(nonbreaking);
}

void WindowController::setPenAttributes(PenAttributes attrs)
{
    m_userSettingsCtrl.onEmbeddedPenAttributes(attrs);

    const auto newPenAttributes = m_userSettingsCtrl.getPenAttributes();

    if (m_selectedWindow)
        m_selectedWindow->setPenAttributes(newPenAttributes);
}

// This will return the current attributes being used taking into account the override
// status of the text drawer
bool WindowController::activePenAttributes(PenAttributes &penAttributes)
{
    if (m_selectedWindow)
    {
        m_selectedWindow->activePenAttributes(penAttributes);
        return true;
    }
    return false;
}

// Set the pen attributes to overriden values - this means that the overriden text drawer will
// use it's own settings and not default window settings (pen colour). It will also not get
// updated by changes from setPenAttributes() which other wise affect all text drawers.
void WindowController::overridePenAttributes(PenAttributes penAttributes, bool midRow)
{
    m_userSettingsCtrl.onEmbeddedPenAttributes(penAttributes);
    m_userSettingsCtrl.onEmbeddedPenColor(penAttributes.pen_color);
    const auto newPenAttributes = m_userSettingsCtrl.getPenAttributes();

    if (m_selectedWindow)
        m_selectedWindow->overridePenAttributes(newPenAttributes, midRow);
}

void WindowController::setPenColor(PenColor color)
{
    m_userSettingsCtrl.onEmbeddedPenColor(color);

    const auto newPenColor = m_userSettingsCtrl.getPenColor();

    if (m_selectedWindow)
        m_selectedWindow->setPenColor(newPenColor);
}

void WindowController::setPenLocation(uint8_t row, uint8_t col)
{
    if (m_selectedWindow)
        m_selectedWindow->setPenLocation(row, col);
}

bool WindowController::hasText(int row)
{
    bool hasText = false;
    if (m_selectedWindow)
        hasText = m_selectedWindow->hasText(row);
    return hasText;
}

void WindowController::enable608(bool enabled)
{
    m_608Enabled = enabled;
}

void WindowController::setTabOffset(uint8_t offset)
{
    if (m_selectedWindow)
        m_selectedWindow->setTabOffset(offset);
}

Window* WindowController::createWindow(std::shared_ptr<Gfx> gfx, WindowDefinition windef)
{
    return new Window(gfx, m_fontCache, windef, m_608Enabled);
}

void WindowController::executeOnWindows(const WindowsMap& wm, std::function<void (Window*)> method)
{
    for (auto& window : m_windowsById)
    {
        if (window)
        {
            if (wm[window->ID()])
            {
                method(window.get());
            }
        }
    }
}

void WindowController::resetWindowTimeout(uint32_t timeout)
{
    m_windowTransition = std::chrono::steady_clock::now();
    m_windowTimeout = timeout;
}

bool WindowController::windowTimeout()
{
    bool timedOut = false;
    if (m_windowTimeout)
    {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_windowTransition).count();

        if (elapsedTime > m_windowTimeout)
        {
            logger.info("%s closed caption window timed out", __func__);

            for (auto &window : m_windowsById)
            {
                if (window)
                {
                    window->clear();
                }
            }

            m_windowTimeout = 0;
            timedOut = true;
            m_windowTimedout = timedOut;
        }

    }
    return timedOut;
}

void WindowController::executeOnAllWindows(std::function<void (Window*)> method)
{
    executeOnWindows(toWindowsMap(0xff), std::move(method));
}

void WindowController::processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet)
{
    logger.info("%s", __func__);
    using CcAttribType = protocol::PacketSetCCAttributes::CcAttribType;

    const auto attributesMap = packet.getAttributes();

    for(const auto& p : attributesMap)
    {
        CcAttribType type;
        uint32_t value;
        std::tie(type, value) = p;

        logger.info("new value for attibId[0x%x] = %d", (int)type, value);

        switch(type)
        {
            case CcAttribType::FONT_COLOR:          m_userSettingsCtrl.onUserDefinedTextColor(value);               break;
            case CcAttribType::FONT_OPACITY:        m_userSettingsCtrl.onUserDefinedTextOpacity(value);             break;
            case CcAttribType::BACKGROUND_COLOR:    m_userSettingsCtrl.onUserDefinedTextBgColor(value);             break;
            case CcAttribType::BACKGROUND_OPACITY:  m_userSettingsCtrl.onUserDefinedTextBgOpacity(value);           break;
            case CcAttribType::FONT_SIZE:           m_userSettingsCtrl.onUserDefinedPenSize(value);                 break;
            case CcAttribType::EDGE_TYPE:           m_userSettingsCtrl.onUserDefinedPenEdge(value);                 break;
            case CcAttribType::EDGE_COLOR:          m_userSettingsCtrl.onUserDefinedTextEdgeColor(value);           break;
            case CcAttribType::FONT_ITALIC:         m_userSettingsCtrl.onUserDefinedFontItalics(value);             break;
            case CcAttribType::FONT_UNDERLINE:      m_userSettingsCtrl.onUserDefinedFontUnderline(value);           break;
            case CcAttribType::WIN_COLOR:           m_userSettingsCtrl.onUserDefinedWindowFillColor(value);         break;
            case CcAttribType::WIN_OPACITY:         m_userSettingsCtrl.onUserDefinedWindowFillOpacity(value);       break;
            case CcAttribType::BORDER_COLOR:        m_userSettingsCtrl.onUserDefinedWindowBorderColor(value);       break;
            case CcAttribType::BORDER_TYPE:         m_userSettingsCtrl.onUserDefinedWindowBorderType(value);        break;
            case CcAttribType::FONT_STYLE:          m_userSettingsCtrl.onUserDefinedFontStyle(value);               break;
            default:
                logger.error("attibId[0x%x] not supported", (int)type);
        }
    }

    //execute on all, not just selected one to update already drawn windows
    const auto newPenColor = m_userSettingsCtrl.getPenColor();

    executeOnAllWindows([&](auto window){
        window->setPenColor(newPenColor);
    });

    const auto newPenAttributes = m_userSettingsCtrl.getPenAttributes();

    executeOnAllWindows([&](auto window){
        window->setPenAttributes(newPenAttributes);
    });

    const auto newWindowAttributes = m_userSettingsCtrl.getWindowAttributes();

    executeOnAllWindows([&](auto window){
        window->setWindowAttributes(newWindowAttributes);
    });

    logger.info("%s - done", __func__);
}

bool WindowController::getWindowDefinition(uint32_t id, WindowDefinition &wd)
{
    auto window = m_windowsById[id].get();
    if (window != nullptr)
    {
        window->getDefinition(wd);
        return true;
    }
    return false;
}

bool WindowController::getWindowDefinition(WindowDefinition &wd)
{
    if (m_selectedWindow)
    {
        m_selectedWindow->getDefinition(wd);
        return true;
    }
    return false;
}

void WindowController::updateWindowRowCount(uint32_t id, int rowCount, bool adjust)
{
    auto window = m_windowsById[id].get();
    if (window != nullptr)
    {
        window->updateRowCount(rowCount, adjust);
    }
}

bool WindowController::isWindowTimedout()
{
    return m_windowTimedout;
}

} // namespace cc
} // namespace subttxrend
