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

#include <vector>
#include <memory>

namespace subttxrend
{

namespace gfx
{
class Window;
} // namespace gfx

namespace cc
{

struct FontInfo
{
  int fontWidth{};
  int fontHeight{};
};

struct FontGroup
{
  FontInfo standard{};
  FontInfo extralarge{};
  FontInfo large{};
  FontInfo small{};
};

struct ScreenInfo
{
  int width{};
  int height{};
  int safeWidth{};
  int safeHeight{};
  int widthSegments{};
  int heightSegments{};
  FontGroup fonts{};
};

struct Dimensions
{
   int w{};
   int h{};

    bool operator==(const Dimensions &o) const
    {
        return w == o.w && h == o.h;
    }
};

struct Point
{
    int x{};
    int y{};

    bool operator==(const Point &o) const
    {
        return x == o.x && y == o.y;
    }
};

class Gfx
{
public:
    Gfx() = default;
    virtual ~Gfx() = default;

    virtual void drawBackground(const Point& p, const Dimensions& dimensions, Color color) = 0;
    virtual void drawBorder(const Point& p, const Dimensions& dimensions, Color color,
                            Color br_color, WindowBorder border_type) = 0;
    virtual void update() = 0;
    virtual void clear() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;

    virtual gfx::Window* getInternalWindow()
    {
        return nullptr;
    }
};

} // namespace cc
} // namespace subttxrend
