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
#include <string>

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>

#include "CcGfx.hpp"
#include "CcCommand.hpp"

namespace subttxrend
{
namespace cc
{

class Renderer: public Gfx
{
public:
    Renderer(gfx::Window* window);
    virtual ~Renderer() = default;

    void drawBackground(const Point& p, const Dimensions& dimensions, Color color) override;
    virtual void drawBorder(const Point& p, const Dimensions& dimensions, Color color,
                            Color br_color, WindowBorder border_type);
    void update() override;
    void clear() override;
    void show() override;
    void hide() override;

    gfx::Window* getInternalWindow() override;


private:
    gfx::Window* const m_window;
    gfx::PrerenderedFontCache mFontCache;
    common::Logger logger;
};

} // namespace cc
} // namespace subttxrend
