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

#include "Parser/IntermediateDocument.hpp"

#include <subttxrend/common/Logger.hpp>

#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/Types.hpp>

#include "Parser/IntermediateDocument.hpp"
#include "ValueConverter.hpp"

#include <memory>

namespace subttxrend
{
namespace common
{
class ConfigProvider;
}
namespace ttmlengine
{

/**
 * Ttml intermediate document drawer.
 */
class IntermediateDocDrawer
{

public:

    IntermediateDocDrawer(const common::ConfigProvider *configProvider,
                          const ValueConverter &valueConverter);

    /**
     * Draws document on window.
     *
     * @param doc
     * @param window
     */
    void draw(const IntermediateDocument& doc,
              gfx::DrawContext& dc);

    /**
     * Clears instance state.
     */
    void clearState();

private:

    /**
     * Current drawing state.
     */
    struct DrawingState
    {
        DrawingState(const gfx::Rectangle& rect);

        /** Region to draw in. */
        gfx::Rectangle m_regionRect;

        /** Pen horizontal position. */
        std::int32_t m_penX{0};

        /** Pen vertical position. */
        std::int32_t m_penY{0};

        //current line size
        gfx::Size m_lineSize{0, 0};

        //current line X offset
        std::int32_t m_lineXOffset{0};

        //current font
        std::shared_ptr<gfx::PrerenderedFont> m_font;

        //current font size
        int m_fontSize{0};

        //total height of all text lines per entity
        unsigned int m_totalTextHeight = 0;
    };

    /**
     * Tries to load given font, fallback to default one if requested font was unavailable.
     *
     */
    std::shared_ptr<gfx::PrerenderedFont> getFont(std::string requestedfontFamily,
                                                  int fontSize);

    /**
     * Draws single entity.
     *
     * @param doc
     * @param dc
     */
    void drawEntity(const IntermediateDocument::Entity& doc,
                    DrawingState& drawingState,
                    gfx::DrawContext& dc);

    /**
     * Draws single text line.
     *
     * @return
     *      True if line was drawn successfully.
     */
    bool drawLine(const IntermediateDocument::TextLine& textLine,
                  DrawingState& drawingState,
                  gfx::DrawContext& dc);

    /**
     * Draws document chunk.
     *
     * @return
     *      True if element was drawn successfully.
     */
    bool drawChunk(const IntermediateDocument::TextChunk& chunk,
                   DrawingState& drawingState,
                   gfx::DrawContext& dc);

    /**
     * Modifies drawing state with newline.
     *
     */
    void newline(DrawingState& drawingState);

    /**
     * Aligns pen to correct position depending on text and alignment in horizontal axis.
     *
     */
    void alignPenX(DrawingState& drawingState,
                   StyleSet::TextAlign textAlign);

    /**
     * Aligns pen to correct position depending on text and alignment in vertical axis.
     *
     */
    void alignPenY(DrawingState& drawingState,
                   StyleSet::DisplayAlign displayAlign,
                   int32_t margin);

    /**
     * Calculates line size based on provided font + document.
     * Length of the corresponding glyphs are calculated for all line characters.
     *
     */
    void getLineSize(const IntermediateDocument::TextLine& line,
                     const DomainValue& lineHeight,
                     DrawingState& drawingState);

    /**
     * Draws line background based on the provided parameters
     *
     */
    void drawLineBackground(const gfx::ColorArgb& backgroundColor,
                            const DrawingState& drawingState,
                            gfx::DrawContext& dc);

    /**
     * Gets the margin for vertical pen position based on outline thickness
     *
     */
    std::int32_t getMargin(IntermediateDocument::TextLine const& line);

private:
    /** Logger object. */
    subttxrend::common::Logger m_logger;

    /** Cached font. */
    gfx::PrerenderedFontCache m_fontCache;

    /** Value converter. */
    const ValueConverter& m_valueConverter;

    std::string m_forcedFont{};
    gfx::ColorArgb m_regionsFillColor{gfx::ColorArgb::TRANSPARENT};

};

}   // namespace ttmlengine
}   // namespace subttxrend
