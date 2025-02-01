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

#pragma once

#include "CcCommand.hpp"
#include "CcCommandParser.hpp"
#include "CcCommandProcessor.hpp"

#include <chrono>
#include <memory>
#include <functional>

#include "CcGfx.hpp"
#include "CcWindow.hpp"
#include "CcCommand.hpp"
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>
#include "CcUserSettingsController.hpp"

namespace subttxrend
{
namespace cc
{

class WindowController : public CommandProcessor
{
public:
    explicit WindowController(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache);
    virtual ~WindowController() = default;

    void drawWindows();

    void setCurrentWindow(uint8_t id) override;
    void clearWindows(WindowsMap wm) override;
    void defineWindow(const WindowDefinition &wd) override;
    void deleteWindows(WindowsMap wm) override;
    void displayWindows(WindowsMap wm) override;
    void toggleWindows(WindowsMap wm) override;
    void setWindowAttributes(WindowAttributes attr) override;
    void report(std::string str) override;
    void hideWindows(WindowsMap wm) override;
    void reset() override;
    void setPenAttributes(PenAttributes attrs) override;
    void setPenColor(PenColor color) override;
    void setPenLocation(uint8_t row, uint8_t col) override;
    void formFeed() override;
    void carriageReturn() override;
    void horizontalCarriageReturn() override;
    void clearWindow() override;
    void backspace() override;
    void transparentSpace(bool nonbreaking) override;

    void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet);
    bool activePenAttributes(PenAttributes &penAttributes) override;
    void overridePenAttributes(PenAttributes penAttributes, bool midRow) override;
    bool getWindowDefinition(uint32_t id, WindowDefinition &wd) override;
    bool getWindowDefinition(WindowDefinition &wd) override;
    void setTabOffset(uint8_t offset) override;
    void resetWindowTimeout(uint32_t timeout = 0) override;
    bool hasText(int row) override;
    void updateWindowRowCount(uint32_t id, int rowCount, bool adjust) override;
    void enable608(bool enabled) override;
    bool isWindowTimedout() override;

protected:
    Window* createWindow(std::shared_ptr<Gfx> gfx, WindowDefinition windef);

private:
    std::unique_ptr<Window>& find(int id);
    void executeOnWindows(const WindowsMap& wm, std::function<void (Window*)>);
    void executeOnAllWindows(std::function<void (Window*)> method);
    bool hasFlashingText();
    bool redrawFlashingText();
    bool windowTimeout();

    std::array<std::unique_ptr<Window>, MAX_WINDOWS> m_windowsById;

    enum class Change
    {
        hide,   // window visibility is false, or window deleted
        show,   // window visible
        toggle, // toggle visibility
        clear   // window cleared but visibility is true
    };

    void executeOnWindowsWithMinimalDisplayTime(const WindowsMap& wm, std::function<void (Window*)>, Change direction);
    struct ChangeTime
    {
        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point time{};
        Change lastChange = Change::hide;
    };
    std::chrono::milliseconds calculateSleepForMinimalDisplayTime(Window* window, Change newDirection);

    std::array<ChangeTime, MAX_WINDOWS> m_timestampsById;
    std::shared_ptr<Gfx> m_gfx;
    std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;
    Window* m_selectedWindow;
    common::Logger logger;

    UserSettingsController m_userSettingsCtrl;
    std::chrono::steady_clock::time_point m_flashTransition;
    std::chrono::steady_clock::time_point m_windowTransition;
    FlashControl m_flashControl;
    uint32_t m_windowTimeout;
    bool m_608Enabled;
    bool m_windowTimedout;
};

} // namespace cc
} // namespace subttxrend
