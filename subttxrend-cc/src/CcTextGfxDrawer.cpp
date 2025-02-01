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

#include <string>
#include <memory>
#include <algorithm>
#include <cmath>

#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>

#include "CcTextGfxDrawer.hpp"
#include "CcWindow.hpp"

namespace subttxrend
{
namespace cc
{

namespace {

const int NUL_CHAR = 0b10000000;
const int NUL_CHAR_MASK = 0b11000000;

const char* FONT_NAME = {"Bitstream Vera Sans Mono"};//{"Liberation Mono Bold"};//

#define MAX_COLUMN_COUNT 32

gfx::DrawDirection convertPrintDirection(WindowPd print_direction)
{
    switch(print_direction)
    {
        case WindowPd::BOTTOM_TOP:
            return gfx::DrawDirection::BOTTOM_TOP;
        case WindowPd::TOP_BOTTOM:
            return gfx::DrawDirection::TOP_BOTTOM;
        case WindowPd::LEFT_RIGHT:
            return gfx::DrawDirection::LEFT_TO_RIGHT;
        case WindowPd::RIGHT_LEFT:
            return gfx::DrawDirection::RIGHT_TO_LEFT;
        default:
            return gfx::DrawDirection::BOTTOM_TOP;
    }
}

}

std::unique_ptr<TextDrawer> TextDrawer::create(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache, FontGroup fonts, int row, int column)
{
    return std::make_unique<TextGfxDrawer>(gfx, fontCache, fonts, row, column);
}

TextGfxDrawer::TextGfxDrawer(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> fontCache, FontGroup fonts, int row, int column):
    TextDrawer(gfx, fontCache, fonts, row, column),
    m_flashState(FlashControl::Show),
    logger("ClosedCaptions", "TextGfxDrawer"),
    m_overridePenAttributes(false)
{
    m_font = m_fontCache->getFont(FONT_NAME, fonts.standard.fontHeight, true);

    logger.trace("%s: row=%d col=%d", __LOGGER_FUNC__, row, column);
}

TextGfxDrawer::~TextGfxDrawer()
{

}

void TextGfxDrawer::popBackUtf8()
{
    if(m_text.empty())
        return;

    auto cp = m_text.data() + m_text.size();
    while(--cp >= m_text.data() && ((*cp & NUL_CHAR_MASK) == NUL_CHAR)) {}
    if(cp >= m_text.data())
    {
        m_text.resize(cp - m_text.data());
        auto len = textLength();
        if (m_trasparent.size() > len)
            m_trasparent.resize(len);
    }
}

void TextGfxDrawer::transparentSpace(bool)
{
    // We ignore nonbreaking as there is no automatic word wrap
    // in 708E
    m_text += " ";
    m_trasparent.resize(textLength());
    m_trasparent.back() = true;
}

bool TextGfxDrawer::backspace()
{
    if (m_text.size())
    {
        popBackUtf8();
        return true;
    }
    return false;
}

void TextGfxDrawer::report(std::string str, WindowDefinition windowDef)
{
    int maxColumns = MAX_COLUMN_COUNT;
    if (windowDef.col_count < maxColumns)
        maxColumns = windowDef.col_count;
    if ((int)textLength() < maxColumns)
        m_text += str;
    logger.trace("[%s: m_text: %s", __func__, m_text.c_str());
}

bool TextGfxDrawer::drawable()
{
    return m_text.size();
}

size_t TextGfxDrawer::textLength()
{
    size_t len = 0;
    auto s = m_text.begin();
    while (s != m_text.end())
    {
        len += (*s & NUL_CHAR_MASK) != NUL_CHAR;
        s++;
    }
    return len;
}

void TextGfxDrawer::setColumn(int column)
{
    size_t len = textLength();
    if (len < static_cast<size_t>(column))
    {
        for(auto i = len; i < static_cast<size_t>(column); ++i)
        {
            m_text += " ";
        }
    }
}

void TextGfxDrawer::setPenAttributes(PenAttributes penattrs)
{
    int size = m_fonts.standard.fontHeight;
    switch(penattrs.pen_size)
    {
    case PenSize::STANDARD:
        size = m_fonts.standard.fontHeight;
        break;
    case PenSize::EXTRALARGE:
        size = m_fonts.extralarge.fontHeight;
        break;
    case PenSize::LARGE:
        size = m_fonts.large.fontHeight;
        break;
    case PenSize::SMALL:
        size = m_fonts.small.fontHeight;
        break;
    }
    std::string fontName;
    switch(penattrs.font_tag)
    {
    case PenFontStyle::CASUAL_FONT_TYPE:
        fontName = "cinecavD casual";
        break;
    case PenFontStyle::CURSIVE_FONT_TYPE:
        fontName = "cinecavD script";
        break;
    case PenFontStyle::MONOSPACED_WITHOUT_SERIFS:
        fontName = "cinecavD mono";
        break;
    case PenFontStyle::MONOSPACED_WITH_SERIFS:
        fontName = "cinecavD type";
        break;
    case PenFontStyle::PROPORTIONALLY_SPACED_WITHOUT_SERIFS:
        fontName = "cinecavD sans";
        break;
    case PenFontStyle::PROPORTIONALLY_SPACED_WITH_SERIFS:
        fontName = "cinecavD serif";
        break;
    case PenFontStyle::SMALL_CAPITALS:
        fontName = "cinecavD smallcaps";
        break;
    case PenFontStyle::DEFAULT_OR_UNDEFINED:
    default:
        fontName = "cinecavD mono";
        break;
    }

    m_font = m_fontCache->getFont(fontName, size, true, penattrs.italics);
    m_attrs = penattrs;
}


void TextGfxDrawer::getPenAttributes(PenAttributes &penattrs)
{
    penattrs = m_attrs;
}

void TextGfxDrawer::setPenOverride(bool midRow)
{
    m_overridePenAttributes = true;
    midrow = midRow;
}

bool TextGfxDrawer::getPenOverride()
{
    return m_overridePenAttributes;
}

Dimensions TextGfxDrawer::dimensions(const WindowPd print_direction)
{
    m_tokens = m_font->textToTokens(m_text);

    int width = 0;
    int height = 0;
    if (print_direction == WindowPd::BOTTOM_TOP or print_direction == WindowPd::TOP_BOTTOM)
    {
        width = m_font->getMaxAdvance();
        for (auto& token: m_tokens)
        {
            height += token.glyphs.size() * m_font->getFontHeight();
        }
    }
    else
    {
        height = m_font->getFontHeight();
        for (auto& token: m_tokens)
        {
            width += token.totalAdvanceX;
        }
    }

    switch(m_attrs.edge_type)
    {
        case PenEdge::DEPRESSED:
            width += shadowEdge;
            height += shadowEdge;
            break;
        case PenEdge::RAISED:
            width += shadowEdge;
            height += shadowEdge;
            break;
        case PenEdge::LEFT_DROP_SHADOW:
            width += shadowEdge;
            break;
        case PenEdge::RIGHT_DROP_SHADOW:
            width += shadowEdge;
            break;
        case PenEdge::UNIFORM:
            width += shadowEdge;
            height += shadowEdge;
            break;
        default:
            break;
    }
    m_dimensions = {width, height};
    return m_dimensions;
}

void TextGfxDrawer::clear()
{
    m_text = std::string();
    m_tokens.clear();
}

void TextGfxDrawer::setMaxWidth(int width)
{
    m_maxWidth = width;
}

const std::string& TextGfxDrawer::getText()
{
    return m_text;
}

void TextGfxDrawer::draw(Point point, const WindowPd print_direction, WindowJustify justify)
{
    int off_x = 0;
    int off_y = 0;
    int off_w = 0;
    int off_h = 0;
    const PenColor activePencolor = m_attrs.pen_color;

    logger.info("%s text:[%s] TS: %zu x=%d y=%d, color:[0x%08x, 0x%08x, 0x%08x], ul:%d|it:%d|fl:%d|et:%d",
        __LOGGER_FUNC__, m_text.c_str(), m_tokens.size(), point.x, point.y,
        activePencolor.fg_color, activePencolor.bg_color, activePencolor.edge_color, (int)m_attrs.underline, (int)m_attrs.italics, (int)m_attrs.flashing, (int)m_attrs.edge_type);

    m_gfx->getInternalWindow()->setDrawDirection(convertPrintDirection(print_direction));

    switch(m_attrs.edge_type)
    {
        case PenEdge::RAISED:
            point.x -= shadowEdge;
            point.y -= shadowEdge;
            off_x = -shadowEdge;
            off_y = -shadowEdge;
            off_w = shadowEdge;
            off_h = shadowEdge;
            break;
        case PenEdge::DEPRESSED:
            point.x += shadowEdge;
            point.y += shadowEdge;
            off_x = shadowEdge;
            off_y = shadowEdge;
            off_w = shadowEdge;
            off_h = shadowEdge;
            break;
        case PenEdge::LEFT_DROP_SHADOW:
            point.x -= shadowEdge;
            off_x = -shadowEdge;
            off_w = shadowEdge;
            break;
        case PenEdge::RIGHT_DROP_SHADOW:
            point.x += shadowEdge;
            off_x = +shadowEdge;
            off_w = shadowEdge;
            break;
        case PenEdge::UNIFORM:
            point.x -= shadowEdge/2;
            point.y -= shadowEdge/2;
            off_x = -shadowEdge/2;
            off_y = -shadowEdge/2;
            off_w = shadowEdge;
            off_h = shadowEdge;
            break;
        default:
            break;
    }

    int y = point.y;
    int x = point.x;
    int whiteSpaceCount = 0;
    int textSize = 0;
    for (auto& token: m_tokens)
    {
        if (token.isWhite)
        {
            whiteSpaceCount += 1;
        }
        else
        {
            textSize += token.totalAdvanceX;
        }
    }
    if (whiteSpaceCount == 0)
        whiteSpaceCount = 1;
    int whiteSpaceSize = (m_maxWidth - textSize)/whiteSpaceCount;

    auto window = m_gfx->getInternalWindow();
    auto& context = window->getDrawContext();
    size_t trans_position = 0;
    for (auto& token: m_tokens)
    {
        gfx::Rectangle rect{x, y, static_cast<int>(token.totalAdvanceX) + 1, m_font->getFontHeight()};
        trans_position += token.glyphs.size();
        int32_t width = token.totalAdvanceX;
        if (token.isWhite && justify == WindowJustify::FULL)
        {
            width = whiteSpaceSize;
        }
        if (&token == &m_tokens.back())
        {
            width += padding;
        }

        Dimensions tmp{width + off_w, m_font->getFontHeight() + off_h};
        if (print_direction == WindowPd::BOTTOM_TOP or print_direction == WindowPd::TOP_BOTTOM)
        {
            tmp.w = m_font->getMaxAdvance();
            tmp.h = token.glyphs.size() * m_font->getFontHeight();
            if (print_direction == WindowPd::BOTTOM_TOP)
                y += m_font->getFontHeight();
        }
        if (!(trans_position - 1 < m_trasparent.size() and m_trasparent[trans_position-1]))
        {
            m_gfx->drawBackground(Point{x - off_x, y - off_y}, tmp, activePencolor.bg_color);
        }

        if (!m_attrs.flashing || (m_flashState == FlashControl::Show))
        {
            if(print_direction == WindowPd::BOTTOM_TOP)
            {
                rect.m_h = token.glyphs.size() * m_font->getFontHeight();
            }
            if(m_attrs.edge_type != PenEdge::NONE)
            {
                // Draw shadow/border first
                context.drawString(*m_font.get(), rect, token.glyphs, gfx::ColorArgb(activePencolor.edge_color), gfx::ColorArgb(0x00000000));
                rect.m_x -= off_x;
                rect.m_y -= off_y;
                if(m_attrs.edge_type == PenEdge::UNIFORM)
                {
                    rect.m_x += shadowEdge/2;
                    rect.m_y += shadowEdge/2;
                    context.drawString(*m_font.get(), rect, token.glyphs, gfx::ColorArgb(activePencolor.edge_color), gfx::ColorArgb(0x00000000));
                    rect.m_x += off_x;
                    rect.m_y += off_y;
                }
                // Overwrite shadow/border with text
                context.drawString(*m_font.get(), rect, token.glyphs, gfx::ColorArgb(activePencolor.fg_color), gfx::ColorArgb(0x00000000));
            }
            else
            {
                context.drawString(*m_font.get(), rect, token.glyphs, gfx::ColorArgb(activePencolor.fg_color), gfx::ColorArgb(0x00000000));
            }

            if(m_attrs.underline)
            {
                const double descenderAbs = std::abs(m_font->getFontDescender());
                const int descenderBy4 = static_cast<int>(std::ceil(descenderAbs/4));
                const int underlineThickness = descenderBy4;
                const int distanceFromCharBottomBorder = descenderBy4;

                gfx::Rectangle where(
                    rect.m_x,
                    rect.m_y + m_font->getFontHeight() - underlineThickness - distanceFromCharBottomBorder,
                    rect.m_w,
                    underlineThickness
                );

                context.drawUnderline(gfx::ColorArgb(activePencolor.fg_color), where);
            }
        }

        if (print_direction == WindowPd::BOTTOM_TOP or print_direction == WindowPd::TOP_BOTTOM)
        {
            y += m_font->getFontHeight() * token.glyphs.size();
        }
        else
        {
            x += width;
        }
    }
}

int TextGfxDrawer::fontHeight()
{
    return m_font->getFontHeight();
}
int TextGfxDrawer::maxAdvance()
{
    return m_font->getMaxAdvance();
}

void TextGfxDrawer::setFlashState(FlashControl state)
{
    if (m_attrs.flashing)
    {
        m_flashState = state;
    }
}

} // namespace cc
} // namespace subttxrend
