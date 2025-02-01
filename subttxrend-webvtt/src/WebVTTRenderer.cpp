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
#include <cassert>
#include <cmath>

#include <WebVTTRenderer.hpp>
#include <WebVTTExceptions.hpp>

#include <subttxrend/gfx/Base64ToPixmap.hpp>
#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/Types.hpp>

#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace webvttengine
{

namespace { common::Logger g_logger("WebvttEngine", "WebVTTRenderer"); } // anonymous

inline std::shared_ptr<gfx::Window> lockGfxPtr(WinPtr ptr) {
    if (auto shptr = ptr.lock())
    {
        return shptr;
    }
    else
    {
        throw DisplayException("gfx::Window pointer is NULL");
    }
}

WebVTTRenderer::WebVTTRenderer(WinPtr gfxWindow, const WebVTTConfig &config)
        : m_gfxPtr(gfxWindow),
          m_config(config),
          m_reset(false) {
    assert(m_gfxPtr.lock());
    lockGfxPtr(m_gfxPtr)->setVisible(true);
}


void WebVTTRenderer::setRelatedVideoSize(gfx::Size relatedVideoSize) {
    g_logger.osdebug(__LOGGER_FUNC__);
    m_surfaceSize = relatedVideoSize;
}

void WebVTTRenderer::update() {
    g_logger.osdebug(__LOGGER_FUNC__);
    lockGfxPtr(m_gfxPtr)->update();
}

void WebVTTRenderer::clearscreen() {
    g_logger.osdebug(__LOGGER_FUNC__);
    lockGfxPtr(m_gfxPtr)->clear();
}

void WebVTTRenderer::resizeWindow() {
    g_logger.osdebug(__LOGGER_FUNC__);
    auto gfxPtr = lockGfxPtr(m_gfxPtr);
    if (m_surfaceSize != gfxPtr->getSize())
    {
        gfxPtr->setSize(m_surfaceSize);
    }
}

gfx::ColorArgb getUserdefinedWindowColor(const WebVTTAttributes &attributes)
{
    uint32_t windowColor = 0;

    if (attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_COLOR))
    {
        windowColor = attributes.getInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR);
    }
    if (attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_OPACITY))
    {
        WebVTTAttributes::Opacity windowOpacity = static_cast<WebVTTAttributes::Opacity>(attributes.getInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY));

        windowColor &= 0x00ffffff;

        uint8_t alpha = 0;
        switch (windowOpacity)
        {
            case WebVTTAttributes::Opacity::SOLID:
                alpha = 255;
                break;
            case WebVTTAttributes::Opacity::FLASHING:
                alpha = 250;
                break;
            case WebVTTAttributes::Opacity::TRANSLUCENT:
                alpha = 100;
                break;
            case WebVTTAttributes::Opacity::TRANSPARENT:
                alpha = 0;
                break;
        }

        windowColor |= (static_cast<uint32_t>(alpha) << 24);
    }
    return gfx::ColorArgb(windowColor);
}

/**
 * @brief Draws a background rectangle, then takes the formatting data that was
 * constructed in LineBuilder and draws the strings to the screen
 * The BG rectangle includes padding, so that needs to be accounted for.
 * 
 * @param line_list List of Lines from the LineBuilder
 * @param dc - DrawContext from the gfx module
 */
void RenderCues(const linebuilder::LineList& line_list, gfx::DrawContext& dc, const WebVTTAttributes &attributes) {
    for (const auto &line : line_list) {
        auto &lineRectangle = line.lineRectangle;
        auto x = lineRectangle.m_x;
        auto y = lineRectangle.m_y;
        auto height = lineRectangle.m_h;

        WebVTTAttributes::EdgeStyle edgeStyle = WebVTTAttributes::EdgeStyle::NONE;
        int off_x = 0;
        int off_y = 0;
        const int shadowEdge = 2;

        if (attributes.isSet(WebVTTAttributes::AttributeType::EDGE_STYLE))
        {
            edgeStyle = static_cast<WebVTTAttributes::EdgeStyle>(attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_STYLE));

            switch(edgeStyle)
            {
                case WebVTTAttributes::EdgeStyle::RAISED:
                   x -= shadowEdge;
                   y -= shadowEdge;
                   off_x = -shadowEdge;
                   off_y = -shadowEdge;
                   break;
                case WebVTTAttributes::EdgeStyle::DEPRESSED:
                   x += shadowEdge;
                   y += shadowEdge;
                   off_x = shadowEdge;
                   off_y = shadowEdge;
                   break;
                case WebVTTAttributes::EdgeStyle::LEFT_DROP_SHADOW:
                   x -= shadowEdge;
                   off_x = -shadowEdge;
                   break;
                case WebVTTAttributes::EdgeStyle::RIGHT_DROP_SHADOW:
                   x += shadowEdge;
                   off_x = +shadowEdge;
                   break;
                case WebVTTAttributes::EdgeStyle::UNIFORM:
                   x -= shadowEdge/2;
                   y -= shadowEdge/2;
                   off_x = -shadowEdge/2;
                   off_y = -shadowEdge/2;
                   break;
                default:
                   break;
            }
        }

        gfx::ColorArgb windowColor = getUserdefinedWindowColor(attributes);
        dc.fillRectangle(windowColor, lineRectangle);

        for (const auto &token : line.tokenVector) {
            auto style = token.style;
            g_logger.osdebug(__LOGGER_FUNC__, " - Style: fs:", style.getFontStyle(),
                                                       " tc:", (int)style.textColour().m_r,
                                                       " fptr:", token.font.get());
            auto token_rectangle = gfx::Rectangle(x + line.linePaddingX, 
                                                  y, 
                                                  token.width, 
                                                  height);
            if(style.fontStyle() == Style::FontStyleType::kUnderline)
            {
                const double descenderAbs = std::abs(token.font.get()->getFontDescender());
                const int descenderBy4 = static_cast<int>(std::ceil(descenderAbs/4));
                const int underlineThickness = descenderBy4;
                const int distanceFromCharBottomBorder = descenderBy4;

                gfx::Rectangle where(
                    x + line.linePaddingX,
                    y + token.font.get()->getFontHeight() - underlineThickness - distanceFromCharBottomBorder,
                    token.width,
                    underlineThickness
                );

                dc.fillRectangle(gfx::ColorArgb(token.style.textColour()), where);
            }
            dc.fillRectangle(gfx::ColorArgb(token.style.bgColour()), token_rectangle);

            if (edgeStyle != WebVTTAttributes::EdgeStyle::NONE)
            {
                // Draw shadow/border first
                dc.drawString(*(token.font.get()), token_rectangle, token.token->glyphs,
                                token.style.edgeColour(), gfx::ColorArgb::TRANSPARENT);
                token_rectangle.m_x -= off_x;
                token_rectangle.m_y -= off_y;

                if(edgeStyle == WebVTTAttributes::EdgeStyle::UNIFORM)
                {
                    token_rectangle.m_x += shadowEdge/2;
                    token_rectangle.m_y += shadowEdge/2;
                    dc.drawString(*(token.font.get()), token_rectangle, token.token->glyphs,
                                    token.style.edgeColour(), gfx::ColorArgb::TRANSPARENT);
                    token_rectangle.m_x += off_x;
                    token_rectangle.m_y += off_y;
                }
                // Overwrite shadow/border with text
                dc.drawString(*(token.font.get()), token_rectangle, token.token->glyphs,
                                token.style.textColour(), gfx::ColorArgb::TRANSPARENT);
            }
            else
            {
                dc.drawString(*(token.font.get()), token_rectangle, token.token->glyphs, 
                                token.style.textColour(), gfx::ColorArgb::TRANSPARENT);
            }
            x += token.width;
        }
    }
}

/**
 * @brief Takes a list of lines from LineBuilder, resizes the window and
 * calls RenderCues to render the Lines to the surface
 * 
 * @param cuelist 
 */
void WebVTTRenderer::renderDocument(const CueSharedList &webvtt_list, const RegionMap &regions) {
    g_logger.osdebug(__LOGGER_FUNC__);
    auto gfxPtr = lockGfxPtr(m_gfxPtr);
    auto preferred_size = gfxPtr->getPreferredSize();
    if (preferred_size == DEFAULT_SURFACE_SIZE) {
        preferred_size = m_surfaceSize;
    }
    
    m_surfaceSize = preferred_size;
    g_logger.osinfo(__LOGGER_FUNC__, " - preferred size ", preferred_size.m_w, "x", preferred_size.m_h);
    resizeWindow();
    
    linebuilder::LineBuilder builder {preferred_size.m_w, preferred_size.m_h, m_config, m_attributes};
    RenderCues(builder.buildOutputLines(webvtt_list, regions), gfxPtr->getDrawContext(), m_attributes);
    
    if (m_reset) {
        g_logger.osinfo(__LOGGER_FUNC__, " - calling clearscreen after render");
        clearscreen();
    }
}

void WebVTTRenderer::show() {
    g_logger.osdebug(__LOGGER_FUNC__);
    m_reset.exchange(false);
    lockGfxPtr(m_gfxPtr)->setVisible(true);
}

void WebVTTRenderer::hide() {
    g_logger.osdebug(__LOGGER_FUNC__);
    lockGfxPtr(m_gfxPtr)->setVisible(false);
}

void WebVTTRenderer::clearState() {
    g_logger.osdebug(__LOGGER_FUNC__);
    m_reset.exchange(true);
    m_attributes.reset();
}

void WebVTTRenderer::setAttributes(const WebVTTAttributes &attributes) {
    m_attributes.update(attributes);
}

}   // namespace webvttengine
}   // namespace subttxrend

