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
#include "CcCommand.hpp"
#include "CcGfx.hpp"
#include "CcTextDrawer.hpp"
#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace gfx
{

class PrerenderedFont;
class PrerenderedFontCache;

}

namespace cc
{

using TokenVector = std::vector<gfx::TextTokenData>;

class TextGfxDrawer: public TextDrawer
{
public:
    TextGfxDrawer(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache, FontGroup fonts, int row, int column);
    virtual ~TextGfxDrawer();

    Dimensions dimensions(const WindowPd print_direction) override;
    void draw(Point point, const WindowPd print_direction, WindowJustify justify) override;
    void report(std::string str, WindowDefinition windowDef) override;
    void clear() override;
    bool drawable() override;
    bool backspace() override;
    void setColumn(int column) override;
    void transparentSpace(bool nonbreaking) override;

    void setPenAttributes(PenAttributes penattrs) override;
    void getPenAttributes(PenAttributes &penattrs) override;
    void setPenOverride(bool midrow) override;
    bool getPenOverride() override;

    int fontHeight() override;
    int maxAdvance() override;

    void setMaxWidth(int width) override;
    const std::string& getText() override;

    void setFlashState(FlashControl state) override;

protected:
    const int shadowEdge = 2;
    size_t textLength();
    void popBackUtf8();
    Dimensions m_dimensions{};
    std::vector<gfx::TextTokenData> m_tokens;
    std::vector<bool> m_trasparent;

    std::shared_ptr<subttxrend::gfx::PrerenderedFont> m_font;
    common::Logger logger;
    bool m_overridePenAttributes;
    FlashControl m_flashState;
};


} // namespace cc
} // namespace subttxrend
