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

#include <memory>
#include "CcWindow.hpp"
#include "CcTextDrawer.hpp"

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
//

ScreenInfo screenInfo708 = {1280, 720, 1050, 600, 210, 75, {{15, 24}, {30, 48}, {25, 40}, {8, 12}}};
ScreenInfo screenInfo608 = {1280, 720, 1050, 600, 32, 15,  {{15, 24}, {30, 48}, {25, 40}, {8, 12}}};
static const int HorizontalMargin = 5;
}

#define MAX_ROW_COUNT 12

Window::Window(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> font, WindowDefinition def, bool enable608):
    m_gfx(gfx), m_fontCache{font}, m_def(def), m_608Enabled(enable608),
    m_fonts(enable608 ? screenInfo608.fonts : screenInfo708.fonts),
    m_changed(false), m_visibilityChanged(false), logger("ClosedCaptions", "Window")
{
}

void Window::update(const WindowDefinition &def)
{

    if (m_def.id != def.id)
    {
        logger.error("Incorrect ids. got: %d, while ours is: %d", def.id, m_def.id);
        return;
    }

    if (m_def != def)
    {
        m_def = def;
        m_changed = true;
        logger.debug("window [%d] updated", m_def.id);
    }

    while (m_textDrawers.size() > MAX_ROW_COUNT)
    {
        logger.debug("text drawer size exceeds maximum  row count, deleting %d", (*m_textDrawers.begin())->row);
        m_textDrawers.erase(m_textDrawers.begin());
    }
}

Window::~Window()
{

}

void Window::transparentSpace(bool nonbreaking)
{
    ensureTextDrawer();
    m_textDrawers.back()->transparentSpace(nonbreaking);
}

bool Window::changed()
{
    return m_changed;
}

void Window::report(std::string str)
{
    if (m_def.pen_style.text_tag != subttxrend::cc::PenTextTag::NOT_TO_BE_DISPLAYED)
    {
        m_changed = true;
        ensureTextDrawer();
        m_textDrawers.back()->report(str, m_def);
    }
}

void Window::carriageReturn()
{
    m_changed = true;
    ensureTextDrawer();

    switch (m_def.win_style.scroll_direction)
    {
        case WindowSd::BOTTOM_TOP:
            scroll(m_textDrawers.back()->row + 1, 0);
        break;
        case WindowSd::TOP_BOTTOM:
            scroll(m_textDrawers.back()->row - 1, 0);
        break;
        case WindowSd::LEFT_RIGHT:
            scroll(0, m_textDrawers.back()-> column - 1);
        break;
        case WindowSd::RIGHT_LEFT:
            scroll(0, m_textDrawers.back()-> column + 1);
        break;
    }
}

void Window::horizontalCarriageReturn()
{
    m_changed = true;
    ensureTextDrawer();
    for(unsigned int citr = 0; citr <= m_textDrawers.back()->getText().length(); citr++)
    {
        backspace();
    }
    setPenLocation(m_textDrawers.back()->row, m_textDrawers.back()->column);
    // missing scrolling support OMWHMXI-480
}

void Window::backspace()
{
   auto tditer = m_textDrawers.rbegin();
   for (; tditer != m_textDrawers.rend(); ++tditer)
   {
       if ((*tditer)->backspace())
           return;
   }
   // hard to support
}

void Window::ensureTextDrawer()
{
    if (m_textDrawers.empty())
    {
        auto startingRow = m_def.win_style.scroll_direction == WindowSd::BOTTOM_TOP ? m_def.row_count - 1 : 0;
        m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, startingRow));
        m_textDrawers.back()->setPenAttributes(m_def.pen_style);
    }
}

bool Window::ignoreColumn()
{
    return m_def.win_style.justify != WindowJustify::LEFT and
            (m_def.win_style.print_direction == WindowPd::LEFT_RIGHT or
             m_def.win_style.print_direction == WindowPd::RIGHT_LEFT);
}

void Window::scroll(int row, int column)
{
    bool isLastRow = (m_def.win_style.scroll_direction == WindowSd::BOTTOM_TOP) ? (row == m_def.row_count)
            : (m_def.win_style.scroll_direction == WindowSd::TOP_BOTTOM) ? (row == -1) : (m_def.win_style.scroll_direction == WindowSd::RIGHT_LEFT) ? (column == m_def.col_count) : (m_def.win_style.scroll_direction == WindowSd::LEFT_RIGHT) ? (column == -1) : true ;

    logger.trace("%s row=%d col=%d scroll dir %d isLast=%d",
        __LOGGER_FUNC__,
        row,
        column,
        static_cast<int>(m_def.win_style.scroll_direction),
        isLastRow);

    m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, row, column));
    m_textDrawers.back()->setPenAttributes(m_def.pen_style);

    if (isLastRow)
    {
        for (auto& textDrawer: m_textDrawers)
        {
            if (m_def.win_style.scroll_direction == WindowSd::BOTTOM_TOP)
            {
                textDrawer->row -= 1;
            }
            else if (m_def.win_style.scroll_direction == WindowSd::TOP_BOTTOM)
            {
                textDrawer->row += 1;
            }
            else if (m_def.win_style.scroll_direction == WindowSd::RIGHT_LEFT)
            {
                textDrawer->column -= 1;
            }
            else
                textDrawer->column += 1;
        }
    }
    if (m_def.win_style.scroll_direction == WindowSd::BOTTOM_TOP)
    {
        // delete all textdrawers that have gone off the top
        while (m_textDrawers.begin() != m_textDrawers.end())
        {
            if ((*m_textDrawers.begin())->row >= 0)
            {
                break;
            }
            m_textDrawers.erase(m_textDrawers.begin());
        }
    }
    else if (m_def.win_style.scroll_direction == WindowSd::TOP_BOTTOM)
    {
        // delete all textdrawers that have gone off the bottom
        while (m_textDrawers.begin() != m_textDrawers.end())
        {
            if ((*m_textDrawers.begin())->row < m_def.row_count)
            {
                break;
            }
            m_textDrawers.erase(m_textDrawers.begin());
        }
    }
    else if ((m_def.win_style.scroll_direction == WindowSd::LEFT_RIGHT or m_def.win_style.scroll_direction == WindowSd::RIGHT_LEFT) and
          m_textDrawers.size() > static_cast<size_t>(m_def.col_count))
    {
        m_textDrawers.erase(m_textDrawers.begin());
    }
}

void Window::setPenLocation(int row, int column)
{
    logger.debug("%s row=%d col=%d (count: %d %d)", __LOGGER_FUNC__, row, column, m_def.row_count, m_def.col_count);

    if (row >= m_def.row_count || column >= m_def.col_count)
    {
        // just ignore out of window bounds
        return;
    }
    ensureTextDrawer();
    bool ignore_column = ignoreColumn();
    if (!m_textDrawers.back()->drawable())
    {
        m_textDrawers.back()->row = row;
    }
    if (row == m_textDrawers.back()->row)
    {
        if (ignore_column)
        {
            return;
        }

        // If the cursor is moving backwards, create a new drawer and overwrite
        // the current one
        // TBD - check the text is overwritten if no backgorund
        if (column < cursor().x)
        {
            m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, row, column));
            m_textDrawers.back()->setPenAttributes(m_def.pen_style);
        }

        if (m_textDrawers.back()->drawable())
        {
            m_textDrawers.back()->setColumn(column - m_textDrawers.back()->column);
        }
        else
        {
            m_textDrawers.back()->column = column;
        }
    }
    else
    {
        if (ignore_column)
        {
            column = 0;
        }
        m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, row, column));
        m_textDrawers.back()->setPenAttributes(m_def.pen_style);
    }
}

void Window::setTabOffset(int offset)
{
    logger.debug("%s offset=%d", __LOGGER_FUNC__, offset);

    ensureTextDrawer();
    setPenLocation(m_textDrawers.back()->row, cursor().x + offset);
}

void Window::setWindowAttributes(WindowAttributes attr)
{
    m_changed = m_def.visible;
    if (attr.print_direction != m_def.win_style.print_direction ||
        attr.scroll_direction != m_def.win_style.scroll_direction)
    {
        clear();
    }
    m_def.win_style = attr;
}

Point Window::cursor()
{
    Point point;

    point.x = m_textDrawers.back()->column;
    point.x += m_textDrawers.back()->getText().length();
    point.y = m_textDrawers.back()->row;

    return point;
}

void Window::setPenAttributes(PenAttributes penattrs)
{
    m_changed = m_def.visible;
    m_def.pen_style = penattrs;
    ensureTextDrawer();

    bool midrow = false;
    // If we have some text already we will create a new drawer
    if (m_textDrawers.back()->drawable())
    {
        Point position = cursor();
        m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, position.y, position.x));
        midrow = true;
    }
    m_textDrawers.back()->setPenAttributes(m_def.pen_style);
}

void Window::activePenAttributes(PenAttributes &penAttributes)
{
    ensureTextDrawer();
    m_textDrawers.back()->getPenAttributes(penAttributes);
    if (!m_textDrawers.back()->getPenOverride())
    {
        // if it hasn't been overridden, the pen colour used is the window pen colour
        penAttributes.pen_color = m_def.pen_style.pen_color;
    }
}

void Window::overridePenAttributes(PenAttributes penAttributes, bool midRow)
{
    ensureTextDrawer();

    // If we have some text already we will create a new drawer
    if (m_textDrawers.back()->drawable())
    {
        Point position = cursor();
        m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, position.y, position.x));
        m_textDrawers.back()->setPenAttributes(m_def.pen_style);
    }

    PenAttributes newPenAttributes = m_def.pen_style;
    newPenAttributes.underline = penAttributes.underline;
    newPenAttributes.italics = penAttributes.italics;
    newPenAttributes.flashing = penAttributes.flashing;
    newPenAttributes.pen_color.fg_color = penAttributes.pen_color.fg_color;
    newPenAttributes.pen_color.bg_color = penAttributes.pen_color.bg_color;

    m_textDrawers.back()->setPenAttributes(newPenAttributes);
    m_textDrawers.back()->setPenOverride(midRow);
}

void Window::setPenColor(PenColor pencolor)
{
    m_changed = m_def.visible;
    m_def.pen_style.pen_color = pencolor;
    ensureTextDrawer();

    bool midrow = false;
    // If we have some text already we will create a new drawer
    if (m_textDrawers.back()->drawable())
    {
        Point position = cursor();
        m_textDrawers.emplace_back(TextDrawer::create(m_gfx, m_fontCache, m_fonts, position.y, position.x));

        midrow = true;
    }
    m_textDrawers.back()->setPenAttributes(m_def.pen_style);
}

Point Window::calculateAnchorTopLeftPoint(const Dimensions& bDim)
{
    Point anchorCoords = calculate(m_608Enabled ? screenInfo608 : screenInfo708);
    switch(m_def.anchor_point)
    {
        case PenAnchorPoint::TOP_LEFT:
            break;
        case PenAnchorPoint::TOP_CENTER:
            anchorCoords.x = anchorCoords.x - bDim.w/2;
            break;
        case PenAnchorPoint::TOP_RIGHT:
            anchorCoords.x = anchorCoords.x - bDim.w;
            break;
        case PenAnchorPoint::MIDDLE_LEFT:
            anchorCoords.y = anchorCoords.y - bDim.h/2;
            break;
        case PenAnchorPoint::MIDDLE_CENTER:
            anchorCoords.x = anchorCoords.x - bDim.w/2;
            anchorCoords.y = anchorCoords.y - bDim.h/2;
            break;
        case PenAnchorPoint::MIDDLE_RIGHT:
            anchorCoords.x = anchorCoords.x - bDim.w;
            anchorCoords.y = anchorCoords.y - bDim.h/2;
            break;
        case PenAnchorPoint::BOTTOM_LEFT:
            anchorCoords.y = anchorCoords.y - bDim.h;
            break;
        case PenAnchorPoint::BOTTOM_CENTER:
            anchorCoords.x = anchorCoords.x - bDim.w/2;
            anchorCoords.y = anchorCoords.y - bDim.h;
            break;
        case PenAnchorPoint::BOTTOM_RIGHT:
            anchorCoords.x = anchorCoords.x - bDim.w;
            anchorCoords.y = anchorCoords.y - bDim.h;
            break;
    }
    return anchorCoords;
}

Point Window::justifyTextDrawer(const Point& anchorPoint, Rect& textRect, const Dimensions& windowDim)
{
    int x = anchorPoint.x;
    int y = anchorPoint.y + textRect.y;

    switch(m_def.win_style.justify)
    {
        case WindowJustify::LEFT:
            if (m_def.win_style.print_direction == WindowPd::RIGHT_LEFT)
                x += windowDim.w - textRect.w - HorizontalMargin;
            else
                x += textRect.x + HorizontalMargin;
        break;
        case WindowJustify::RIGHT:
            if (m_def.win_style.print_direction == WindowPd::RIGHT_LEFT)
                x += textRect.x + HorizontalMargin;
            else
                x += windowDim.w - textRect.w - HorizontalMargin;
        break;
        case WindowJustify::CENTER:
            x += (windowDim.w - textRect.w)/2;
        break;
        case WindowJustify::FULL:
            x += textRect.x + HorizontalMargin;
        break;
    };

    return {x, y};
}

void Window::drawTextDrawers(const Point& anchorPoint, std::vector<Rect>& tdRects, const Dimensions& windowDim)
{
    auto rectIter = tdRects.begin();
    for (auto& textDrawer: m_textDrawers)
    {
        auto point = justifyTextDrawer({anchorPoint.x, anchorPoint.y}, *rectIter, windowDim);
        textDrawer->setMaxWidth(windowDim.w);
        textDrawer->draw(point, m_def.win_style.print_direction, m_def.win_style.justify);
        rectIter++;
    }
}

Dimensions Window::calculateRects4Text(std::vector<Rect>& tdRects)
{
    Dimensions biggestDimensions{0, 0};
    int textRelativeY = 0;

    auto maxFontHeight = 0;
    auto maxAdvance = 0;
    switch (m_def.pen_style.pen_size)
    {
        case PenSize::SMALL:
            maxFontHeight = m_fonts.small.fontHeight;
            maxAdvance =  m_fonts.small.fontWidth;
            break;
        case PenSize::STANDARD:
            maxFontHeight = m_fonts.standard.fontHeight;
            maxAdvance =  m_fonts.standard.fontWidth;
            break;
        case PenSize::LARGE:
            maxFontHeight = m_fonts.large.fontHeight;
            maxAdvance =  m_fonts.large.fontWidth;
            break;
        case PenSize::EXTRALARGE:
            maxFontHeight = m_fonts.extralarge.fontHeight;
            maxAdvance =  m_fonts.extralarge.fontWidth;
            break;
    }
    auto previousTextDrawer = m_textDrawers.end();
    for (auto textDrawer = m_textDrawers.begin(); textDrawer != m_textDrawers.end();
            previousTextDrawer = textDrawer, textDrawer++)
    {
        const auto dim = (*textDrawer)->dimensions(m_def.win_style.print_direction);
        const Dimensions dimWithMargin{dim.w + HorizontalMargin, dim.h};
        auto textRelativeX =  0;

        textRelativeX = ((*textDrawer)->column * (*textDrawer)->maxAdvance());
        textRelativeY = ((*textDrawer)->row * (*textDrawer)->fontHeight());
        maxFontHeight = std::max(maxFontHeight, (*textDrawer)->fontHeight());
        maxAdvance = std::max(maxAdvance, (*textDrawer)->maxAdvance());

        // If we enable the ability to change text size per row then we need to make sure
        // that this row does not overwrite the row above (ie cant do row * this-row-font-height)

        (*textDrawer)->padding = 0;

        // If this text drawer row is assigned to the same row as the last one,
        // position it straight after the last one
        if ((previousTextDrawer != m_textDrawers.end()) &&
            ((*textDrawer)->row == (*previousTextDrawer)->row))
        {
            // Only add the horizontal margin to the last drawer in the row
            tdRects.back().w -= HorizontalMargin;

            // Pad the last text rect if necessary to avoid gaps in the background
            if ((tdRects.back().x + tdRects.back().w) < textRelativeX)
            {
                (*previousTextDrawer)->padding = textRelativeX - (tdRects.back().x + tdRects.back().w);
            }
        }

        tdRects.push_back({textRelativeX, textRelativeY, dimWithMargin.w, dimWithMargin.h});

        biggestDimensions.w = std::max(biggestDimensions.w, dimWithMargin.w+textRelativeX);
        biggestDimensions.h = std::max(biggestDimensions.h, dimWithMargin.h+textRelativeY);
    }

    biggestDimensions.h = std::max(biggestDimensions.h, maxFontHeight * m_def.row_count);
    biggestDimensions.w = std::max(biggestDimensions.w, maxAdvance * m_def.col_count);
    return biggestDimensions;
}

void Window::draw()
{
    m_changed = false;
    m_visibilityChanged = false;
    if (!m_def.visible)
    {
        return;
    }

    std::vector<Rect> tdRects;
    Dimensions txDrawer{0, 0};
    // *************************************************************
    //  !!! THIS CODE DID NOT WORK AS INTENDED !!!
    //
    // It seems that the loop below changes pen size
    // for the use in next renderings, not for the current
    // one. Also it does not reset the pen size to the
    // previous value for the text which fit into the window.
    // For now we use the configured pen size and accept it may not
    // fit into the screen.
    // *************************************************************
    // do
    // {
    //     if (tdRects.size())
    //     {
    //         if (m_def.pen_style.pen_size == PenSize::STANDART)
    //         {
    //             m_def.pen_style.pen_size = PenSize::SMALL;
    //         }
    //         else if (m_def.pen_style.pen_size == PenSize::LARGE)
    //         {
    //             m_def.pen_style.pen_size = PenSize::STANDART;
    //         }
    //         else
    //         {
    //             break; /// nothing we can do more
    //         }
    //         tdRects.clear();
    //     }
        txDrawer = calculateRects4Text(tdRects);

    // } while(txDrawer.h > screenInfo.safeHeight || txDrawer.w > screenInfo.safeWidth);
    Dimensions windowDimensions{txDrawer.w + 2 * HorizontalMargin,
                                txDrawer.h};


    const auto anchorPoint = calculateAnchorTopLeftPoint(windowDimensions);

    logger.debug("%s window dimensions (%d %d) anchor point (%d %d)",
        __LOGGER_FUNC__,
        windowDimensions.w,
        windowDimensions.h,
        anchorPoint.x,
        anchorPoint.y);

    m_gfx->drawBorder(anchorPoint, windowDimensions, m_def.win_style.fill_color, m_def.win_style.border_color,
                      m_def.win_style.border_type);

    drawTextDrawers(anchorPoint, tdRects, windowDimensions);
}

Point Window::calculate(const ScreenInfo& screenInfo)
{
    Point ret;
    const auto screenSafeHeight = [&]{
    if(m_608Enabled && m_textDrawers.size()>0 && m_textDrawers.back())
    {
        auto fH = m_textDrawers.back()->fontHeight();
        logger.info("%s fontHeight - %d",__LOGGER_FUNC__,fH);
        if (fH == m_fonts.small.fontHeight || fH == m_fonts.standard.fontHeight || fH == m_fonts.extralarge.fontHeight)
        {
           fH = m_fonts.large.fontHeight;
           logger.info("%s fontHeight adjusted - %d",__LOGGER_FUNC__,fH);
        }
        return std::min(screenInfo.height, fH * screenInfo.heightSegments);
     } else
        return screenInfo.safeHeight;
    }();

    ret.x = (screenInfo.width - screenInfo.safeWidth)/2;
    ret.y = (screenInfo.height - screenSafeHeight)/2;
    if (m_def.relative_pos)
    {
        ret.x += ((screenInfo.safeWidth * m_def.anchor_horizontal) / 100);
        ret.y += ((screenSafeHeight * m_def.anchor_vertical) / 100);
    }
    else
    {
        ret.x += screenInfo.safeWidth/screenInfo.widthSegments * m_def.anchor_horizontal;
        ret.y += screenSafeHeight/screenInfo.heightSegments * m_def.anchor_vertical;
    }
    return ret;
}

void Window::show()
{
    logger.debug("show(%d) ", ID());

    m_changed = true;
    m_visibilityChanged = true;
    m_def.visible = true;
}

void Window::hide()
{
    logger.debug("hide(%d) ", ID());

    m_changed = true;
    m_visibilityChanged = true;
    m_def.visible = false;
}

void Window::toggle()
{
    logger.debug("toggle(%d): %d", ID(), m_def.visible);

    m_changed = true;
    m_visibilityChanged = true;
    m_def.visible = !m_def.visible;
}

void Window::clear()
{
    logger.debug("clear(%d) ", ID());
    m_changed = m_def.visible;
    m_textDrawers.clear();
}

bool Window::hasFlashingText()
{
    PenAttributes penattrs;
    for (auto& textDrawer: m_textDrawers)
    {
        textDrawer->getPenAttributes(penattrs);
        if (penattrs.flashing)
        {
            break;
        }
    }
    return penattrs.flashing;
}

bool Window::hasText(int row)
{
    for (auto& textDrawer: m_textDrawers)
    {
        if ((textDrawer->row == row) && textDrawer->drawable())
        {
            return true;
        }
    }
    return false;
}

void Window::setFlashState(FlashControl state)
{
    for (auto& textDrawer: m_textDrawers)
    {
        textDrawer->setFlashState(state);
    }
}

void Window::updateRowCount(int rowCount, bool adjust)
{
    if (m_def.win_style.scroll_direction != WindowSd::BOTTOM_TOP)
    {
        adjust = false; //not supported
    }

    if (adjust)
    {
        int offset = rowCount - m_def.row_count;

        if (offset > 0) // rows increasing
        {
            // Adjust the text rows down (ie the window grows upwards and we maintain the current row position)
            for (auto iter = m_textDrawers.begin(); iter != m_textDrawers.end(); )
            {
                (*iter)->row += offset;
                iter++;
            }
        }
        else
        {
            // Adjust the text rows up to maintain the current row (remove any rows that go off the top)
            for (auto iter = m_textDrawers.begin(); iter != m_textDrawers.end(); )
            {
                (*iter)->row += offset;

                if ((*iter)->row < 0)
                {
                    iter = m_textDrawers.erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }
    }

    m_def.row_count = rowCount;
}

} // namespace cc
} // namespace subttxrend

