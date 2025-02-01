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

#include "CcRenderer.hpp"

#include <cassert>
#include <vector>

namespace subttxrend
{
namespace cc
{

namespace
{
    //screen          safe          std     large   small
    //1920 x 1080 -> 1470 x 825     35x55 | 42x66 | 28x44
    //1280 x 720 -> 1050 x 600      25x40 | 30x48 | 20x32
    //720 x 486 -> 480 x 375 (4:3)  15x25 | 18x30 | 12x20
}

static const gfx::ColorArgb WHITE(255, 255, 255, 255);
static const gfx::ColorArgb BLACK(255, 0, 0, 0);


Renderer::Renderer(gfx::Window* window):
    m_window(window),
    logger("ClosedCaptions", "Renderer")
{
    assert(m_window);
}

void Renderer::drawBackground(const Point& point, const Dimensions& dimensions, Color color)
{
    auto& drawContext = m_window->getDrawContext();
    subttxrend::gfx::Rectangle where(point.x, point.y, dimensions.w, dimensions.h);

    drawContext.fillRectangle(gfx::ColorArgb(color), where);
}

void Renderer::drawBorder(const Point& point, const Dimensions& dimensions, Color bg_color,
                                Color br_color, WindowBorder border_type)
{
    int border = 4;
    auto& drawContext = m_window->getDrawContext();
    subttxrend::gfx::Rectangle borderRect(point.x, point.y, dimensions.w, dimensions.h);

    switch(border_type)
    {
        case WindowBorder::DEPRESSED:
            borderRect.m_x -= border;
            borderRect.m_y -= border;
            borderRect.m_w += border*2;
            borderRect.m_h += border*2;
            drawContext.fillRectangle(WHITE, borderRect);
            borderRect.m_w -= border/2;
            borderRect.m_h -= border/2;
            drawContext.fillRectangle(gfx::ColorArgb(br_color), borderRect);
            borderRect.m_x += border/2;
            borderRect.m_y += border/2;
            borderRect.m_w -= border;
            borderRect.m_h -= border;
            drawContext.fillRectangle(BLACK, borderRect);
            break;
        case WindowBorder::RAISED:
            borderRect.m_x -= border;
            borderRect.m_y -= border;
            borderRect.m_w += border*2;
            borderRect.m_h += border*2;
            drawContext.fillRectangle(BLACK, borderRect);
            borderRect.m_w -= border/2;
            borderRect.m_h -= border/2;
            drawContext.fillRectangle(gfx::ColorArgb(br_color), borderRect);
            borderRect.m_x += border/2;
            borderRect.m_y += border/2;
            borderRect.m_w -= border;
            borderRect.m_h -= border;
            drawContext.fillRectangle(WHITE, borderRect);
            break;
        case WindowBorder::SHADOW_LEFT:
            borderRect.m_x -= border;
            borderRect.m_y += border;
            drawContext.fillRectangle(gfx::ColorArgb(br_color), borderRect);
            break;
        case WindowBorder::SHADOW_RIGHT:
            borderRect.m_x += border;
            borderRect.m_y += border;
            drawContext.fillRectangle(gfx::ColorArgb(br_color), borderRect);
            break;
        case WindowBorder::UNIFORM:
            borderRect.m_x -= border/2;
            borderRect.m_y -= border/2;
            borderRect.m_w += border;
            borderRect.m_h += border;
            drawContext.fillRectangle(gfx::ColorArgb(br_color), borderRect);
            break;
        // Handle other border styles - fall through
        case WindowBorder::DEFAULT:
        default:
            break;
    }

    drawBackground(point, dimensions, bg_color);
}

void Renderer::update()
{
    m_window->update();
}

void Renderer::clear()
{
    m_window->clear();
}

void Renderer::show()
{
    m_window->setSize(gfx::Size{1920, 1080});
    m_window->setVisible(true);
}

void Renderer::hide()
{
    m_window->setVisible(false);
}

gfx::Window* Renderer::getInternalWindow()
{
    return m_window;
}

} // namespace cc
} // namespace subttxrend
