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
#include <vector>
#include <memory>
#include <string>
#include "CcCommand.hpp"
#include "CcGfx.hpp"

namespace subttxrend
{
namespace gfx
{

class Window;
} // namespace gfx

namespace cc
{

class TextDrawer
{
public:
    TextDrawer(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache, FontGroup fonts,  int row = 0, int column = 0):
        column(column), row(row), m_gfx(gfx), m_fontCache{fontCache}, m_fonts(fonts), midrow(false), padding(0)
    {}
    virtual ~TextDrawer() = default;

    static std::unique_ptr<TextDrawer> create(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache, FontGroup fonts, int row = 0, int column = 0);

    virtual void report(std::string str, WindowDefinition windowDef) = 0;
    virtual void clear() = 0;
    virtual bool drawable() = 0;
    virtual void setColumn(int column) = 0;
    virtual void transparentSpace(bool nonbreaking) = 0;

    virtual Dimensions dimensions(const WindowPd print_direction) = 0;
    virtual void draw(Point point, const WindowPd print_direction, WindowJustify justify) = 0;

    virtual void setPenAttributes(PenAttributes penattrs) = 0;
    virtual void getPenAttributes(PenAttributes &penattrs) = 0;
    virtual void setPenOverride(bool midrow) = 0;
    virtual bool getPenOverride() = 0;

    virtual bool backspace() = 0;
    virtual int fontHeight() = 0;
    virtual int maxAdvance() = 0;

    virtual void setMaxWidth(int width) = 0;
    virtual const std::string& getText() = 0;

    virtual void setFlashState(FlashControl state) = 0;

    int column;
    int row;
    bool midrow;
    int padding;

protected:
    std::shared_ptr<Gfx> m_gfx;
    std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;
    FontGroup m_fonts;
    std::string m_text;
    int m_maxWidth{};
    PenAttributes m_attrs;
};

} // namespace cc
} // namespace subttxrend
