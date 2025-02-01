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

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <string>
namespace subttxrend
{
namespace cc
{

class CommandProcessor
{
public:
    virtual ~CommandProcessor() = default;
    virtual void formFeed() = 0;
    virtual void carriageReturn() = 0;
    virtual void horizontalCarriageReturn() = 0;
    virtual void clearWindow() = 0;
    virtual void backspace() = 0;
    virtual void transparentSpace(bool nonbreaking) = 0;
    virtual void report(std::string str) = 0;
    virtual void setCurrentWindow(uint8_t id) = 0;
    virtual void clearWindows(WindowsMap wm) = 0;
    virtual void displayWindows(WindowsMap wm) = 0;
    virtual void hideWindows(WindowsMap wm) = 0;
    virtual void toggleWindows(WindowsMap wm) = 0;
    virtual void deleteWindows(WindowsMap wm) = 0;
    virtual void reset() = 0;
    virtual void setPenAttributes(PenAttributes attrs) = 0;
    virtual void setPenColor(PenColor color) = 0;
    virtual void setPenLocation(uint8_t row, uint8_t col) = 0;
    virtual void setWindowAttributes(WindowAttributes attrs) = 0;
    virtual void defineWindow(const WindowDefinition &wd) = 0;
    virtual bool activePenAttributes(PenAttributes &penAttributes) = 0;
    virtual void overridePenAttributes(PenAttributes penAttributes, bool midRow) = 0;
    virtual bool getWindowDefinition(uint32_t id, WindowDefinition &wd) = 0;
    virtual bool getWindowDefinition(WindowDefinition &wd) = 0;
    virtual void setTabOffset(uint8_t offset) = 0;
    virtual void resetWindowTimeout(uint32_t timeout = 0) = 0;
    virtual bool hasText(int row) = 0;
    virtual void updateWindowRowCount(uint32_t id, int rowCount, bool adjust) = 0;
    virtual void enable608(bool enabled) = 0;
    virtual bool isWindowTimedout() = 0;
};

} // namespace cc
} // namespace subttxrend
