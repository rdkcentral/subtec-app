/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#pragma once

#include <memory>

#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/Types.hpp>

namespace subttxrend
{
namespace gfx
{

class KeyEventListener;

enum class DrawDirection
{
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
    TOP_BOTTOM,
    BOTTOM_TOP
};

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;

    virtual void addKeyEventListener(KeyEventListener* listener) = 0;
    virtual void removeKeyEventListener(KeyEventListener* listener) = 0;
    virtual Rectangle getBounds() const = 0;
    virtual DrawContext& getDrawContext() = 0;
    virtual Size getPreferredSize() const = 0;
    virtual void setSize(const Size& newSize) = 0;
    virtual Size getSize() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual void clear() = 0;
    virtual void update() = 0;
    virtual void setDrawDirection(DrawDirection dir) = 0;
};

using WindowPtr = std::shared_ptr<Window>;

} // namespace gfx
} // namespace subttxrend