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


#include <cmath>
#include <memory>
#include <string>

#include "IntermediateDocDrawer.hpp"

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>

namespace subttxrend
{
namespace ttmlengine
{

namespace
{
/** Text rectangle horizontal margin.
    Calculated as percent of font size.
    The value 32,1% is used to give 26px for font 81px */
const auto BACKGROUND_RECT_MARGIN = DomainValue(DomainValue::Type::PERCENTAGE_HUNDREDTHS, 3210);

/** Default font to use if loading requested font failed. */
constexpr auto FALLBACK_FONT_NAME = "Bitstream Vera Sans Mono Bold";
}//anonymous namespace

IntermediateDocDrawer::IntermediateDocDrawer(const common::ConfigProvider *configProvider,
                                             const ValueConverter &valueConverter)
        :
        m_logger("TtmlEngine", "IntermediateDocDrawer", this),
        m_fontCache(),
        m_valueConverter(valueConverter)
{
    if (configProvider) {
        std::string forcedFont = configProvider->get("FORCE_FONT");
        if (! forcedFont.empty()) {
            m_forcedFont = forcedFont;
            m_logger.osinfo(__LOGGER_FUNC__, " using forced font: ", m_forcedFont);
        }

        auto regionsFillColor = configProvider->get("REGIONS_FILL_COLOR");
        if (! regionsFillColor.empty()) {
            try {
                auto value = static_cast<std::uint32_t>(std::stoul(regionsFillColor, 0, 0));
                m_regionsFillColor = gfx::ColorArgb{value};
                m_logger.osinfo(__LOGGER_FUNC__, " coloring regions with ", value);
            }
            catch(const std::invalid_argument& e) {
                m_logger.oswarning(__LOGGER_FUNC__, " could not convert REGIONS_FILL_COLOR: ", regionsFillColor);
            }
        }
    }
}

void IntermediateDocDrawer::draw(const IntermediateDocument& doc,
                                 gfx::DrawContext& dc)
{
    auto fullScreenRect = m_valueConverter.toTargetRectangle({DomainValue::Type::PERCENTAGE_HUNDREDTHS, 0},
                                            {DomainValue::Type::PERCENTAGE_HUNDREDTHS, 0},
                                            {DomainValue::Type::PERCENTAGE_HUNDREDTHS, 10000},
                                            {DomainValue::Type::PERCENTAGE_HUNDREDTHS, 10000});

    for (auto &entity : doc.m_entites)
    {
        DrawingState drawingState = { fullScreenRect };
        drawEntity(entity, drawingState, dc);
    }
}

IntermediateDocDrawer::DrawingState::DrawingState(const gfx::Rectangle& rect) :
        m_regionRect(rect)
{
    // noop
}

void IntermediateDocDrawer::drawEntity(const IntermediateDocument::Entity& entity,
                                       DrawingState& drawingState,
                                       gfx::DrawContext& dc)
{
    if (entity.m_region)
    {
        auto region = *entity.m_region;
        drawingState.m_regionRect = m_valueConverter.toTargetRectangle(region.getX(),
            region.getY(),
            region.getWidth(),
            region.getHeight());

        drawingState.m_penX = 0;
        drawingState.m_penY = 0;
    }

    if (entity.m_imageChunk.m_image && entity.m_imageChunk.m_bmp)
    {
        m_logger.ostrace(__LOGGER_FUNC__, " drawing image: ", entity.m_imageChunk.m_image->getId());
        dc.drawBitmap(*entity.m_imageChunk.m_bmp.get(), drawingState.m_regionRect);
    }
    else
    {
        m_logger.ostrace(__LOGGER_FUNC__, " lines ", entity.m_textLines.size());

        if (m_regionsFillColor != gfx::ColorArgb::TRANSPARENT) {
            dc.fillRectangle(m_regionsFillColor,
                             gfx::Rectangle {
                                     drawingState.m_regionRect.m_x,
                                     drawingState.m_regionRect.m_y,
                                     drawingState.m_regionRect.m_w,
                                     drawingState.m_regionRect.m_h });
        }

        if (entity.m_textLines.size() > 0) {

            for (auto const& textLine : entity.m_textLines)
            {
                if (!textLine.empty())
                {
                    auto const& style = textLine.front().m_style;
                    auto const cellHeightPx = m_valueConverter.getCellHeight();
                    auto const fontSize = m_valueConverter.sizeToPixels(style.getFontSize(), cellHeightPx);
                    auto const lineHeight = m_valueConverter.sizeToPixels(style.getLineHeight(), fontSize);

                    drawingState.m_totalTextHeight += lineHeight;
                }
            }

            auto const& firstLine = entity.m_textLines.front();
            auto const& lastLine = entity.m_textLines.back();

            auto const displayAlign = (!firstLine.empty()) ? firstLine.front().m_style.getDisplayAlign() : StyleSet::DisplayAlign::BEFORE;

            std::int32_t margin = 0;
            if (displayAlign == StyleSet::DisplayAlign::BEFORE) {
                margin = getMargin(firstLine);
            } else if (displayAlign == StyleSet::DisplayAlign::AFTER) {
                margin = getMargin(lastLine);
            }

            alignPenY(drawingState, displayAlign, margin);
        }

        for (auto &textLine : entity.m_textLines)
        {
            if (!drawLine(textLine, drawingState, dc))
            {
                return;
            }
        }
    }
}

bool IntermediateDocDrawer::drawLine(const IntermediateDocument::TextLine& textLine,
                                     DrawingState& drawingState,
                                     gfx::DrawContext& dc)
{
     m_logger.ostrace(__LOGGER_FUNC__, " chunks ", textLine.size());

    if (textLine.size() > 0)
    {
        //Font is based on the style of the first text chunk of the line
        // all subsequent text chunks use the same font
        const StyleSet& style = textLine[0].m_style;
        const auto cellHeightPx = m_valueConverter.getCellHeight();
        drawingState.m_fontSize = m_valueConverter.sizeToPixels(style.getFontSize(), cellHeightPx);
        drawingState.m_font = getFont(style.getFontFamily(), drawingState.m_fontSize);
        drawingState.m_margin = m_valueConverter.sizeToPixels(BACKGROUND_RECT_MARGIN, drawingState.m_fontSize);

        if (drawingState.m_font)
        {
            //Based on font style and size of the first text chunk - length of the whole line is calculated
            getLineSize(textLine, style.getLineHeight(), drawingState);
            m_logger.ostrace(__LOGGER_FUNC__, " font size: ", drawingState.m_fontSize,
                             " line text length: ", drawingState.m_lineSize.m_w,
                             " line height: ", drawingState.m_lineSize.m_h,
                            " background color", style.getBackgroundColor());

            alignPenX(drawingState, style.getTextAlign());
            // and background is drawn - with the background color of the first text chunk
            drawLineBackground(style.getBackgroundColor(), drawingState, dc);

            for (const IntermediateDocument::TextChunk& chunk : textLine)
            {
                m_logger.ostrace(__LOGGER_FUNC__, " drawing chunk ", chunk.m_text);
                auto t = m_logger.timing("drawChunk");

                if (!drawChunk(chunk, drawingState, dc))
                {
                    // no more space, no need to continue
                    return false;
                }
            }
            // TODO move outside "if (textLine.size() > 0)" to allow drawnig empty lines
            newline(drawingState);
        }
    }
    return true;
}

void IntermediateDocDrawer::newline(DrawingState& drawingState)
{
    drawingState.m_penX = 0;
    drawingState.m_penY += drawingState.m_lineSize.m_h;
}

void IntermediateDocDrawer::alignPenX(DrawingState& drawingState,
                                      StyleSet::TextAlign textAlign)
{
    const std::int32_t textLength = drawingState.m_lineSize.m_w;

    m_logger.ostrace(__LOGGER_FUNC__, " textAlign=", textAlign, " textLength=", textLength);

    if (textAlign == StyleSet::TextAlign::LEFT)
    {
        drawingState.m_penX = drawingState.m_margin;
    }
    else if (textAlign == StyleSet::TextAlign::RIGHT)
    {
        drawingState.m_penX = drawingState.m_regionRect.m_w - textLength - drawingState.m_margin;
    }
    else // defaults - "center"
    {
        drawingState.m_penX = std::ceil((drawingState.m_regionRect.m_w - textLength) / 2);
    }
}

void IntermediateDocDrawer::alignPenY(DrawingState& drawingState,
                                      StyleSet::DisplayAlign displayAlign,
                                      std::int32_t margin)
{
    const std::int32_t textHeight = drawingState.m_totalTextHeight;

    m_logger.ostrace(__LOGGER_FUNC__, " displayAlign=", displayAlign, " textHeight=", textHeight, " margin=", margin);
    if (displayAlign == StyleSet::DisplayAlign::BEFORE)
    {
        drawingState.m_penY = 0 + margin;
    }
    else if (displayAlign == StyleSet::DisplayAlign::AFTER)
    {
        drawingState.m_penY = drawingState.m_regionRect.m_h - textHeight - margin;
    }
    else // defaults - "center"
    {
        drawingState.m_penY = std::ceil((drawingState.m_regionRect.m_h - textHeight) / 2);
    }
}

std::shared_ptr<gfx::PrerenderedFont> IntermediateDocDrawer::getFont(std::string requestedfontFamily,
                                                                     int fontSize)
{
    std::shared_ptr<gfx::PrerenderedFont> font;
    auto const fontFamily = m_forcedFont.empty() ? requestedfontFamily : m_forcedFont;
    try
    {
        font = m_fontCache.getFont(fontFamily, fontSize, true);
    }
    catch (...)
    {
        try
        {
            m_logger.oswarning(
                    __LOGGER_FUNC__, " loading requested font \'", fontFamily, "\' failed, trying fallback font");
            font = m_fontCache.getFont(FALLBACK_FONT_NAME, fontSize);
        }
        catch (...)
        {
            // ups event fallback font failed, should not happen
            m_logger.oserror(__LOGGER_FUNC__,
                           " loading fallback font \'",
                           FALLBACK_FONT_NAME,
                           "\' failed, ttml subs would not be visible");
        }
    }
    return font;
}

void IntermediateDocDrawer::getLineSize(const IntermediateDocument::TextLine& line,
                                        const DomainValue& lineHeight,
                                        DrawingState& drawingState)
{
    assert(drawingState.m_font);
    std::int32_t textWidth = 0;
    std::int32_t textTrimmed = 0;
    bool textStarted = false;
    std::int32_t xOffset = 0;

    for (const IntermediateDocument::TextChunk& chunk : line)
    {
        std::vector<gfx::TextTokenData> tokens = drawingState.m_font->textToTokens(chunk.m_text);

        for (auto& token : tokens)
        {
            //Calculate line size without leading or trailing whitespace
            if (!textStarted)
            {
                if (token.isWhite)
                    xOffset += std::round(token.totalAdvanceX);
                else
                    textStarted = true;
            }

            if (textStarted)
                textWidth += std::round(token.totalAdvanceX);

            if (!token.isWhite)
                textTrimmed = textWidth;
        }
    }
    drawingState.m_lineSize.m_w = textTrimmed;
    drawingState.m_lineSize.m_h = m_valueConverter.sizeToPixels(lineHeight, drawingState.m_fontSize);
    drawingState.m_lineXOffset = xOffset;
}

void IntermediateDocDrawer::drawLineBackground(const gfx::ColorArgb& backgroundColor,
                                               const DrawingState& drawingState,
                                               gfx::DrawContext& dc)
{
    if (drawingState.m_lineSize.m_w > 0)
    {
        dc.fillRectangle(backgroundColor,
                         gfx::Rectangle {
                                 drawingState.m_regionRect.m_x + drawingState.m_lineXOffset + drawingState.m_penX - drawingState.m_margin,
                                 drawingState.m_regionRect.m_y + drawingState.m_penY,
                                 drawingState.m_lineSize.m_w + (2 * drawingState.m_margin),
                                 drawingState.m_lineSize.m_h });
    }
}

bool IntermediateDocDrawer::drawChunk(const IntermediateDocument::TextChunk& chunk,
                                      DrawingState& drawingState,
                                      gfx::DrawContext& dc)
{
    assert(drawingState.m_font);

    std::vector<gfx::TextTokenData> tokens = drawingState.m_font->textToTokens(chunk.m_text);

    m_logger.ostrace(__LOGGER_FUNC__,
                   " starting pen position: ",
                   drawingState.m_penX,
                   " ",
                   drawingState.m_penY,
                   " (w=",
                   drawingState.m_regionRect.m_w,
                   " h=",
                   drawingState.m_regionRect.m_h,
                   ")",
                   " tokens count: ",
                   tokens.size(),
                   ", '",
                   chunk.m_text,
                   "'");

    for (gfx::TextTokenData& token : tokens)
    {
        bool didNewline = false;

        if (token.forceNewline)
        {
            // TODO:  FEATURE tts:wrapOption support
            if ((drawingState.m_penX + std::round(token.totalAdvanceX)) > drawingState.m_regionRect.m_w)
            {
                newline(drawingState);
                didNewline = true;
            }
        }

        if (drawingState.m_penY > drawingState.m_regionRect.m_h)
        {
            m_logger.oswarning(__LOGGER_FUNC__,
                             " not enough space in region h=",
                             drawingState.m_regionRect.m_h,
                             " penY=",
                             drawingState.m_penY);
            // TODO FEATURE support for "overflow" property
            // no more space in the region; skip all remaining tokens
            return false;
        }

        if (token.isWhite)
        {
            // TODO: at the moment - if any whitespace was found, other whitespace after that will be discarded
            // TODO: would need more whitespace handling in textToTokens otherwise
            if (!didNewline)
            {
                drawingState.m_penX += std::round(token.totalAdvanceX);
            }
        }
        else
        {
            const StyleSet& style = chunk.m_style;
            const gfx::ColorArgb& fgColor = style.getColor();

            const gfx::Rectangle rectangle = {
                    drawingState.m_regionRect.m_x + drawingState.m_penX,
                    drawingState.m_regionRect.m_y + drawingState.m_penY,
                    drawingState.m_regionRect.m_w,
                    drawingState.m_regionRect.m_h };

            auto outline = style.getOutline();
            auto outlineSize = m_valueConverter.sizeToPixels(outline.getThickness(), drawingState.m_fontSize);
            int verticalOffset = (drawingState.m_lineSize.m_h - drawingState.m_fontSize) / 2;

            m_logger.ostrace(__LOGGER_FUNC__,
                           " drawing token: rect=[",
                           rectangle.m_x, // TODO operator<< (..., rect)
                           ' ',
                           rectangle.m_y,
                           ' ',
                           rectangle.m_w,
                           ' ',
                           rectangle.m_h,
                           "] total_advance=",
                           token.totalAdvanceX,
                           ", outline size=",
                           outlineSize,
                           ", vertical offset=",
                           verticalOffset);

            //first, draw the outline
            if (outlineSize > 0) {
                dc.drawString(*(drawingState.m_font.get()), rectangle, token.glyphs, outline.getColor(), gfx::ColorArgb::TRANSPARENT, outlineSize, verticalOffset);
            }
            //then draw the text on top of the outline
            dc.drawString(*(drawingState.m_font.get()), rectangle, token.glyphs, fgColor, gfx::ColorArgb::TRANSPARENT, 0, verticalOffset);

            drawingState.m_penX += std::round(token.totalAdvanceX);
        }
    }

    return true;
}

void IntermediateDocDrawer::clearState()
{
    m_fontCache.clear();
}

std::int32_t IntermediateDocDrawer::getMargin(IntermediateDocument::TextLine const& line)
{
    int32_t margin = 0;

    if (!line.empty()) {
        auto const outlineSize = line.front().m_style.getOutline().getThickness();
        auto const cellHeightPx = m_valueConverter.getCellHeight();
        auto const fontSize = m_valueConverter.sizeToPixels(line.front().m_style.getFontSize(), cellHeightPx);
        margin = m_valueConverter.sizeToPixels(outlineSize, fontSize);
    }

    return margin;
}


}
}
