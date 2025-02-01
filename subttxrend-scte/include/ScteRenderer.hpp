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

#pragma once

#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Types.hpp>

#include "ScteSimpleBitmap.hpp"
#include "ScteRawBitmap.hpp"

namespace subttxrend
{
namespace scte
{

/**
 * SCTE bitmap renderer.
 */
class Renderer
{
public:

    Renderer(gfx::Window* gfxWindow);

    void show();
    void hide();

    void render(const SimpleBitmap& bm, size_t width, size_t heigth);
    void clearscreen();
    void update();


private:

    static unsigned yuv2rgb(Color color);
    void render(const RawBitmap::Data& bitmap, uint8_t* bytemap, Coords topLeft, Coords bottomRight, uint8_t onColor, uint8_t offColor, size_t width, size_t size);
    gfx::Window* const m_gfxWindow;

};

}   // namespace scte
}   // namespace subttxrend

