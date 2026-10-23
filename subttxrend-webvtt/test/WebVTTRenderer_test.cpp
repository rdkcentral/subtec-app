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

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <WebVTTDocument.hpp>
#include <WebVTTExceptions.hpp>
#include <WebVTTStyle.hpp>
#include <WebVTTRenderer.hpp>

using namespace subttxrend::webvttengine;

namespace subttxrend
{
namespace gfx
{

const ColorArgb ColorArgb::TRANSPARENT(0, 0, 0, 0);

} // namespace gfx
} // namespace subttxrend

namespace
{

struct FillCall
{
    subttxrend::gfx::ColorArgb color;
    subttxrend::gfx::Rectangle rectangle;
};

struct DrawStringCall
{
    subttxrend::gfx::Rectangle rectangle;
    subttxrend::gfx::ColorArgb foreground;
    subttxrend::gfx::ColorArgb background;
    std::size_t glyphCount;
};

class RecordingDrawContext : public subttxrend::gfx::DrawContext
{
public:
    std::vector<FillCall> fillCalls;
    std::vector<DrawStringCall> drawStringCalls;

    void fillRectangle(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rectangle) override
    {
        fillCalls.push_back({color, rectangle});
    }

    void drawString(subttxrend::gfx::PrerenderedFont& font,
                    const subttxrend::gfx::Rectangle& rectangle,
                    const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                    const subttxrend::gfx::ColorArgb foreground,
                    const subttxrend::gfx::ColorArgb background,
                    int outlineSize = 0,
                    int verticalOffset = 0) override
    {
        (void)font;
        (void)outlineSize;
        (void)verticalOffset;
        drawStringCalls.push_back({rectangle, foreground, background, glyphs.size()});
    }

    void drawUnderline(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rectangle) override
    {
        (void)color;
        (void)rectangle;
    }

    void drawPixmap(const subttxrend::gfx::ClutBitmap& bitmap,
                    const subttxrend::gfx::Rectangle& srcRect,
                    const subttxrend::gfx::Rectangle& dstRect) override
    {
        (void)bitmap;
        (void)srcRect;
        (void)dstRect;
    }

    void drawBitmap(const subttxrend::gfx::Bitmap& bitmap,
                    const subttxrend::gfx::Rectangle& dstRect) override
    {
        (void)bitmap;
        (void)dstRect;
    }

    void drawGlyph(const subttxrend::gfx::FontStripPtr& fontStrip,
                   std::int32_t glyphIndex,
                   const subttxrend::gfx::Rectangle& rect,
                   subttxrend::gfx::ColorArgb fgColor,
                   subttxrend::gfx::ColorArgb bgColor) override
    {
        (void)fontStrip;
        (void)glyphIndex;
        (void)rect;
        (void)fgColor;
        (void)bgColor;
    }

    void reset()
    {
        fillCalls.clear();
        drawStringCalls.clear();
    }
};

class RecordingWindow : public subttxrend::gfx::Window
{
public:
    RecordingWindow(subttxrend::gfx::Size preferredSize,
                    subttxrend::gfx::Size currentSize)
        : m_bounds(0, 0, currentSize.m_w, currentSize.m_h),
          m_preferredSize(preferredSize),
          m_size(currentSize)
    {
    }

    void addKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        (void)listener;
    }

    void removeKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        (void)listener;
    }

    subttxrend::gfx::Rectangle getBounds() const override
    {
        return m_bounds;
    }

    subttxrend::gfx::DrawContext& getDrawContext() override
    {
        return drawContext;
    }

    subttxrend::gfx::Size getPreferredSize() const override
    {
        return m_preferredSize;
    }

    void setSize(const subttxrend::gfx::Size& newSize) override
    {
        sizeCalls.push_back(newSize);
        m_size = newSize;
        m_bounds = subttxrend::gfx::Rectangle(0, 0, newSize.m_w, newSize.m_h);
    }

    subttxrend::gfx::Size getSize() const override
    {
        return m_size;
    }

    void setVisible(bool visible) override
    {
        visibleCalls.push_back(visible);
    }

    void clear() override
    {
        ++clearCount;
    }

    void update() override
    {
        ++updateCount;
    }

    void setDrawDirection(subttxrend::gfx::DrawDirection dir) override
    {
        drawDirection = dir;
    }

    RecordingDrawContext drawContext;
    std::vector<bool> visibleCalls;
    std::vector<subttxrend::gfx::Size> sizeCalls;
    int clearCount {0};
    int updateCount {0};
    subttxrend::gfx::DrawDirection drawDirection {subttxrend::gfx::DrawDirection::LEFT_TO_RIGHT};

private:
    subttxrend::gfx::Rectangle m_bounds;
    subttxrend::gfx::Size m_preferredSize;
    subttxrend::gfx::Size m_size;
};

std::pair<CueSharedList, RegionMap> parseDocument(const std::string& text)
{
    std::istringstream stream(text);
    WebVTTDocument document;
    CueList uniqueCues;
    RegionMap regions;
    std::tie(uniqueCues, regions) = document.parseCueList(stream, 0);

    CueSharedList sharedCues;
    for (auto& cue : uniqueCues)
    {
        sharedCues.emplace_back(std::move(cue));
    }

    return std::make_pair(std::move(sharedCues), std::move(regions));
}

const FillCall* findFillByHeight(const RecordingDrawContext& drawContext, int height)
{
    for (const auto& fillCall : drawContext.fillCalls)
    {
        if (fillCall.rectangle.m_h == height)
        {
            return &fillCall;
        }
    }

    return nullptr;
}

std::string singleCueDocument(const std::string& cueText)
{
    return "WEBVTT\n\n00:00:00.000 --> 00:00:05.000\n" + cueText + "\n\n";
}

} // namespace

class WebVTTRendererTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(WebVTTRendererTest);
    CPPUNIT_TEST(testCtorShowsWindow);
    CPPUNIT_TEST(testShowHideReset);
    CPPUNIT_TEST(testClearscreenDelegates);
    CPPUNIT_TEST(testUpdateDelegates);
    CPPUNIT_TEST(testRenderUsesRelatedSize);
    CPPUNIT_TEST(testRenderUsesPreferredSize);
    CPPUNIT_TEST(testRenderSkipsResizeWhenSized);
    CPPUNIT_TEST(testRenderEmptyCueList);
    CPPUNIT_TEST(testWindowOpacityValues);
    CPPUNIT_TEST(testWindowColorKeepsAlpha);
    CPPUNIT_TEST(testClearStateKeepsClearPending);
    CPPUNIT_TEST(testUnderlineDecoration);
    CPPUNIT_TEST(testUniformEdgeStyle);
    CPPUNIT_TEST(testEdgeStyles);
    CPPUNIT_TEST(testExpiredWindowThrows);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testCtorShowsWindow()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(1280, 720));

        WebVTTRenderer renderer(window, WebVTTConfig{});

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(window->visibleCalls.front()));
    }

    void testShowHideReset()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        renderer.clearState();
        renderer.show();
        renderer.hide();
        renderer.show();

        auto document = parseDocument(singleCueDocument("visible"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(std::size_t(4), window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(window->visibleCalls[0]));
        CPPUNIT_ASSERT(static_cast<bool>(window->visibleCalls[1]));
        CPPUNIT_ASSERT(!static_cast<bool>(window->visibleCalls[2]));
        CPPUNIT_ASSERT(static_cast<bool>(window->visibleCalls[3]));
        CPPUNIT_ASSERT_EQUAL(0, window->clearCount);
    }

    void testClearscreenDelegates()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(1280, 720));
        WebVTTRenderer renderer(window, WebVTTConfig{});

        renderer.clearscreen();

        CPPUNIT_ASSERT_EQUAL(1, window->clearCount);
    }

    void testUpdateDelegates()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(1280, 720));
        WebVTTRenderer renderer(window, WebVTTConfig{});

        renderer.update();

        CPPUNIT_ASSERT_EQUAL(1, window->updateCount);
    }

    void testRenderUsesRelatedSize()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        auto document = parseDocument(singleCueDocument("related-size"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->sizeCalls.size());
        CPPUNIT_ASSERT(subttxrend::gfx::Size(640, 360) == window->sizeCalls.front());
        CPPUNIT_ASSERT(!window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(!window->drawContext.drawStringCalls.empty());
    }

    void testRenderUsesPreferredSize()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(800, 600), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        auto document = parseDocument(singleCueDocument("preferred-size"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->sizeCalls.size());
        CPPUNIT_ASSERT(subttxrend::gfx::Size(800, 600) == window->sizeCalls.front());
    }

    void testRenderSkipsResizeWhenSized()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(800, 600), subttxrend::gfx::Size(800, 600));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        auto document = parseDocument(singleCueDocument("already-sized"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT(window->sizeCalls.empty());
        CPPUNIT_ASSERT(!window->drawContext.drawStringCalls.empty());
    }

    void testRenderEmptyCueList()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        CueSharedList emptyCues;
        RegionMap emptyRegions;
        renderer.renderDocument(emptyCues, emptyRegions);

        CPPUNIT_ASSERT(window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->sizeCalls.size());
        CPPUNIT_ASSERT(subttxrend::gfx::Size(640, 360) == window->sizeCalls.front());
    }

    void testWindowOpacityValues()
    {
        struct OpacityCase
        {
            WebVTTAttributes::Opacity opacity;
            std::uint8_t expectedAlpha;
        };

        const OpacityCase opacityCases[] = {
            {WebVTTAttributes::Opacity::SOLID, 255},
            {WebVTTAttributes::Opacity::FLASHING, 250},
            {WebVTTAttributes::Opacity::TRANSLUCENT, 100},
            {WebVTTAttributes::Opacity::TRANSPARENT, 0}
        };

        for (const auto& opacityCase : opacityCases)
        {
            auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
            WebVTTRenderer renderer(window, WebVTTConfig{});
            renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

            WebVTTAttributes attributes;
            attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x00112233);
            attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY, static_cast<std::uint32_t>(opacityCase.opacity));
            renderer.setAttributes(attributes);

            auto document = parseDocument(singleCueDocument("opacity"));
            renderer.renderDocument(document.first, document.second);

            CPPUNIT_ASSERT(!window->drawContext.fillCalls.empty());
            CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(opacityCase.expectedAlpha, 0x11, 0x22, 0x33) == window->drawContext.fillCalls.front().color);
        }
    }

    void testWindowColorKeepsAlpha()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x7F112233);
        renderer.setAttributes(attributes);

        auto document = parseDocument(singleCueDocument("alpha"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT(!window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(0x7F, 0x11, 0x22, 0x33) == window->drawContext.fillCalls.front().color);
    }

    void testClearStateKeepsClearPending()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x00AABBCC);
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY, static_cast<std::uint32_t>(WebVTTAttributes::Opacity::SOLID));
        renderer.setAttributes(attributes);

        auto document = parseDocument(singleCueDocument("reset"));
        renderer.renderDocument(document.first, document.second);
        CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(255, 0xAA, 0xBB, 0xCC) == window->drawContext.fillCalls.front().color);

        window->drawContext.reset();
        renderer.clearState();
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(1, window->clearCount);
        CPPUNIT_ASSERT(!window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(0, 0, 0, 0) == window->drawContext.fillCalls.front().color);

        window->drawContext.reset();
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(2, window->clearCount);
    }

    void testUnderlineDecoration()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        auto document = parseDocument(singleCueDocument("<u>under</u>"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->drawContext.drawStringCalls.size());
        CPPUNIT_ASSERT(window->drawContext.drawStringCalls.front().glyphCount > 0U);

        const auto* underlineFill = findFillByHeight(window->drawContext, 2);
        CPPUNIT_ASSERT(underlineFill != nullptr);
        CPPUNIT_ASSERT(Style::kWhite == underlineFill->color);
        CPPUNIT_ASSERT_EQUAL(window->drawContext.drawStringCalls.front().rectangle.m_y + 36, underlineFill->rectangle.m_y);
        CPPUNIT_ASSERT_EQUAL(window->drawContext.drawStringCalls.front().rectangle.m_w, underlineFill->rectangle.m_w);
    }

    void testUniformEdgeStyle()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_STYLE, static_cast<std::uint32_t>(WebVTTAttributes::EdgeStyle::UNIFORM));
        attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, 0xFF102030);
        renderer.setAttributes(attributes);

        auto document = parseDocument(singleCueDocument("edge"));
        renderer.renderDocument(document.first, document.second);

        CPPUNIT_ASSERT_EQUAL(std::size_t(3), window->drawContext.drawStringCalls.size());
        CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(255, 0x10, 0x20, 0x30) == window->drawContext.drawStringCalls[0].foreground);
        CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(255, 0x10, 0x20, 0x30) == window->drawContext.drawStringCalls[1].foreground);
        CPPUNIT_ASSERT(Style::kWhite == window->drawContext.drawStringCalls[2].foreground);

        const auto& first = window->drawContext.drawStringCalls[0].rectangle;
        const auto& second = window->drawContext.drawStringCalls[1].rectangle;
        const auto& third = window->drawContext.drawStringCalls[2].rectangle;
        CPPUNIT_ASSERT_EQUAL(third.m_x - 1, first.m_x);
        CPPUNIT_ASSERT_EQUAL(third.m_y - 1, first.m_y);
        CPPUNIT_ASSERT_EQUAL(third.m_x + 1, second.m_x);
        CPPUNIT_ASSERT_EQUAL(third.m_y + 1, second.m_y);
    }

    void testEdgeStyles()
    {
        struct EdgeStyleCase
        {
            WebVTTAttributes::EdgeStyle edgeStyle;
            std::size_t expectedDrawCount;
            int shadowOffsetX;
            int shadowOffsetY;
        };

        const EdgeStyleCase edgeStyleCases[] = {
            {WebVTTAttributes::EdgeStyle::RAISED, 2, -2, -2},
            {WebVTTAttributes::EdgeStyle::DEPRESSED, 2, 2, 2},
            {WebVTTAttributes::EdgeStyle::LEFT_DROP_SHADOW, 2, -2, 0},
            {WebVTTAttributes::EdgeStyle::RIGHT_DROP_SHADOW, 2, 2, 0}
        };

        for (const auto& edgeStyleCase : edgeStyleCases)
        {
            auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
            WebVTTRenderer renderer(window, WebVTTConfig{});
            renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360));

            WebVTTAttributes attributes;
            attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_STYLE, static_cast<std::uint32_t>(edgeStyleCase.edgeStyle));
            attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, 0xFF405060);
            renderer.setAttributes(attributes);

            auto document = parseDocument(singleCueDocument("edge"));
            renderer.renderDocument(document.first, document.second);

            CPPUNIT_ASSERT_EQUAL(edgeStyleCase.expectedDrawCount, window->drawContext.drawStringCalls.size());
            CPPUNIT_ASSERT(subttxrend::gfx::ColorArgb(255, 0x40, 0x50, 0x60) == window->drawContext.drawStringCalls[0].foreground);
            CPPUNIT_ASSERT(Style::kWhite == window->drawContext.drawStringCalls[1].foreground);

            const auto& shadow = window->drawContext.drawStringCalls[0].rectangle;
            const auto& text = window->drawContext.drawStringCalls[1].rectangle;
            CPPUNIT_ASSERT_EQUAL(text.m_x + edgeStyleCase.shadowOffsetX, shadow.m_x);
            CPPUNIT_ASSERT_EQUAL(text.m_y + edgeStyleCase.shadowOffsetY, shadow.m_y);
        }
    }

    void testExpiredWindowThrows()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720), subttxrend::gfx::Size(320, 180));
        WebVTTRenderer renderer(window, WebVTTConfig{});
        window.reset();

        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x00123456);

        CPPUNIT_ASSERT_NO_THROW(renderer.setRelatedVideoSize(subttxrend::gfx::Size(640, 360)));
        CPPUNIT_ASSERT_NO_THROW(renderer.clearState());
        CPPUNIT_ASSERT_NO_THROW(renderer.setAttributes(attributes));
        CPPUNIT_ASSERT_THROW(renderer.show(), DisplayException);
        CPPUNIT_ASSERT_THROW(renderer.hide(), DisplayException);
        CPPUNIT_ASSERT_THROW(renderer.clearscreen(), DisplayException);
        CPPUNIT_ASSERT_THROW(renderer.update(), DisplayException);

        auto document = parseDocument(singleCueDocument("expired"));
        CPPUNIT_ASSERT_THROW(renderer.renderDocument(document.first, document.second), DisplayException);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTRendererTest);