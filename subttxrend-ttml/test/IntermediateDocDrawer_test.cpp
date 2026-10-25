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

#include <cppunit/extensions/HelperMacros.h>

#include "IntermediateDocDrawer.hpp"

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/gfx/DrawContext.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace subttxrend::ttmlengine;

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
    int outlineSize = 0;
    int verticalOffset = 0;
    std::size_t glyphCount = 0;
};

struct BitmapCall
{
    subttxrend::gfx::Size bitmapSize;
    subttxrend::gfx::Rectangle rectangle;
};

class RecordingDrawContext : public subttxrend::gfx::DrawContext
{
public:
    std::vector<FillCall> fillCalls;
    std::vector<DrawStringCall> drawStringCalls;
    std::vector<BitmapCall> bitmapCalls;

    void fillRectangle(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rectangle) override
    {
        fillCalls.push_back({color, rectangle});
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
        bitmapCalls.push_back({bitmap.getSize(), dstRect});
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

    void drawString(subttxrend::gfx::PrerenderedFont& font,
                    const subttxrend::gfx::Rectangle& destinationRect,
                    const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                    const subttxrend::gfx::ColorArgb fgColor,
                    const subttxrend::gfx::ColorArgb bgColor,
                    int outlineSize = 0,
                    int verticalOffset = 0) override
    {
        (void)font;
        drawStringCalls.push_back(DrawStringCall{
            destinationRect,
            fgColor,
            bgColor,
            outlineSize,
            verticalOffset,
            glyphs.size()});
    }

};

class MapConfigProvider : public subttxrend::common::ConfigProvider
{
public:
    void setValue(const std::string& key,
                  const std::string& value)
    {
        m_values[key] = value;
    }

protected:
    const char* getValue(const std::string& key) const override
    {
        const auto found = m_values.find(key);
        return found == m_values.end() ? nullptr : found->second.c_str();
    }

private:
    std::map<std::string, std::string> m_values;
};

} // namespace

class IntermediateDocDrawerTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( IntermediateDocDrawerTest );
    CPPUNIT_TEST(testDrawEmptyDoc);
    CPPUNIT_TEST(testDrawRegionFillColor);
    CPPUNIT_TEST(testDrawInvalidRegionFillColor);
    CPPUNIT_TEST(testDrawFullscreenWithoutRegion);
    CPPUNIT_TEST(testDrawImageEntity);
    CPPUNIT_TEST(testDrawImageWithoutBitmap);
    CPPUNIT_TEST(testDrawTextAlign);
    CPPUNIT_TEST(testDrawDisplayAlign);
    CPPUNIT_TEST(testDrawWhitespaceTrim);
    CPPUNIT_TEST(testDrawWhitespaceOnly);
    CPPUNIT_TEST(testDrawOutline);
    CPPUNIT_TEST(testDrawChunkColors);
    CPPUNIT_TEST(testClearStateRedraw);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_valueConverter.setSourceSize({200, 300});
        m_valueConverter.setCellResolution({20, 15});
    }

    void tearDown() override
    {
        // noop
    }

protected:

    void testDrawEmptyDoc()
    {
        RecordingDrawContext drawContext;
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        IntermediateDocument doc;

        drawer.draw(doc, drawContext);

        CPPUNIT_ASSERT(drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawRegionFillColor()
    {
        MapConfigProvider configProvider;
        configProvider.setValue("REGIONS_FILL_COLOR", "0x11223344");
        IntermediateDocDrawer drawer(&configProvider, m_valueConverter);
        RecordingDrawContext drawContext;

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 5000, 4000);

        drawer.draw(makeDocument(entity), drawContext);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), drawContext.fillCalls.size());
        CPPUNIT_ASSERT(drawContext.fillCalls[0].color == subttxrend::gfx::ColorArgb(0x11223344));
        assertRectangle(drawContext.fillCalls[0].rectangle, 20, 60, 100, 120);
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawInvalidRegionFillColor()
    {
        MapConfigProvider configProvider;
        configProvider.setValue("REGIONS_FILL_COLOR", "not-a-color");
        IntermediateDocDrawer drawer(&configProvider, m_valueConverter);
        RecordingDrawContext drawContext;

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 5000, 4000);

        drawer.draw(makeDocument(entity), drawContext);

        CPPUNIT_ASSERT(drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawFullscreenWithoutRegion()
    {
        MapConfigProvider configProvider;
        configProvider.setValue("REGIONS_FILL_COLOR", "0xAABBCCDD");
        IntermediateDocDrawer drawer(&configProvider, m_valueConverter);
        RecordingDrawContext drawContext;

        IntermediateDocument::Entity entity;

        drawer.draw(makeDocument(entity), drawContext);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), drawContext.fillCalls.size());
        CPPUNIT_ASSERT(drawContext.fillCalls[0].color == subttxrend::gfx::ColorArgb(0xAABBCCDD));
        assertRectangle(drawContext.fillCalls[0].rectangle, 0, 0, 200, 300);
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawImageEntity()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        RecordingDrawContext drawContext;

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 5000, 4000);
        entity.m_imageChunk.m_image = std::make_shared<ImageElement>();
        entity.m_imageChunk.m_image->parseAttribute("xml", "id", "img-1");
        entity.m_imageChunk.m_bmp = std::make_shared<subttxrend::gfx::Bitmap>(13, 17, 52);

        drawer.draw(makeDocument(entity), drawContext);

        CPPUNIT_ASSERT(drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), drawContext.bitmapCalls.size());
        assertRectangle(drawContext.bitmapCalls[0].rectangle, 20, 60, 100, 120);
        CPPUNIT_ASSERT(drawContext.bitmapCalls[0].bitmapSize == subttxrend::gfx::Size(13, 17));
    }

    void testDrawImageWithoutBitmap()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        RecordingDrawContext drawContext;

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 5000, 4000);
        entity.m_imageChunk.m_image = std::make_shared<ImageElement>();
        entity.m_imageChunk.m_image->parseAttribute("xml", "id", "img-2");

        drawer.draw(makeDocument(entity), drawContext);

        CPPUNIT_ASSERT(drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawTextAlign()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);

        RecordingDrawContext leftContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "before", "sans-serif"))})})), leftContext);

        RecordingDrawContext centerContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("center", "before", "sans-serif"))})})), centerContext);

        RecordingDrawContext rightContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("right", "before", "sans-serif"))})})), rightContext);

        assertTextWasDrawn(leftContext);
        assertTextWasDrawn(centerContext);
        assertTextWasDrawn(rightContext);

        CPPUNIT_ASSERT(leftContext.drawStringCalls.front().rectangle.m_x
                       < centerContext.drawStringCalls.front().rectangle.m_x);
        CPPUNIT_ASSERT(centerContext.drawStringCalls.front().rectangle.m_x
                       < rightContext.drawStringCalls.front().rectangle.m_x);
        CPPUNIT_ASSERT(leftContext.fillCalls.front().rectangle.m_x
                       < centerContext.fillCalls.front().rectangle.m_x);
        CPPUNIT_ASSERT(centerContext.fillCalls.front().rectangle.m_x
                       < rightContext.fillCalls.front().rectangle.m_x);
    }

    void testDrawDisplayAlign()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);

        RecordingDrawContext beforeContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "before", "sans-serif"))})})), beforeContext);

        RecordingDrawContext centerContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "center", "sans-serif"))})})), centerContext);

        RecordingDrawContext afterContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "after", "sans-serif"))})})), afterContext);

        assertTextWasDrawn(beforeContext);
        assertTextWasDrawn(centerContext);
        assertTextWasDrawn(afterContext);

        CPPUNIT_ASSERT(beforeContext.drawStringCalls.front().rectangle.m_y
                       < centerContext.drawStringCalls.front().rectangle.m_y);
        CPPUNIT_ASSERT(centerContext.drawStringCalls.front().rectangle.m_y
                       < afterContext.drawStringCalls.front().rectangle.m_y);
    }

    void testDrawWhitespaceTrim()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);

        RecordingDrawContext plainContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("AB", baseTextAttributes("left", "before", "sans-serif"))})})), plainContext);

        RecordingDrawContext spacedContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("  AB  ", baseTextAttributes("left", "before", "sans-serif"))})})), spacedContext);

        assertTextWasDrawn(plainContext);
        assertTextWasDrawn(spacedContext);

        CPPUNIT_ASSERT_EQUAL(plainContext.fillCalls.front().rectangle.m_w,
                             spacedContext.fillCalls.front().rectangle.m_w);
        CPPUNIT_ASSERT(spacedContext.fillCalls.front().rectangle.m_x
                       > plainContext.fillCalls.front().rectangle.m_x);
        CPPUNIT_ASSERT(spacedContext.drawStringCalls.front().rectangle.m_x
                       > plainContext.drawStringCalls.front().rectangle.m_x);
    }

    void testDrawWhitespaceOnly()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        RecordingDrawContext drawContext;

        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("     ", baseTextAttributes("left", "before", "sans-serif"))})})), drawContext);

        CPPUNIT_ASSERT(drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.bitmapCalls.empty());
    }

    void testDrawOutline()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);

        RecordingDrawContext noOutlineContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "before", "sans-serif"))})})), noOutlineContext);

        auto outlineAttributes = baseTextAttributes("left", "before", "sans-serif");
        outlineAttributes["textOutline"] = "red 2px";

        RecordingDrawContext outlineContext;
        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", outlineAttributes)})})), outlineContext);

        assertTextWasDrawn(noOutlineContext);
        assertTextWasDrawn(outlineContext);

        CPPUNIT_ASSERT(outlineContext.drawStringCalls.size() > noOutlineContext.drawStringCalls.size());
        CPPUNIT_ASSERT(outlineContext.drawStringCalls.front().outlineSize > 0);
        CPPUNIT_ASSERT(outlineContext.drawStringCalls.back().outlineSize == 0);
    }

    void testDrawChunkColors()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        RecordingDrawContext drawContext;

        auto firstAttributes = baseTextAttributes("left", "before", "sans-serif");
        firstAttributes["color"] = "yellow";
        auto secondAttributes = baseTextAttributes("left", "before", "sans-serif");
        secondAttributes["color"] = "cyan";

        drawer.draw(makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("AB", firstAttributes), makeChunk("CD", secondAttributes)})})), drawContext);

        CPPUNIT_ASSERT(drawContext.drawStringCalls.size() >= 2);
        CPPUNIT_ASSERT(drawContext.drawStringCalls[0].foreground == subttxrend::gfx::ColorArgb::YELLOW);
        CPPUNIT_ASSERT(drawContext.drawStringCalls[1].foreground == subttxrend::gfx::ColorArgb::CYAN);
    }

    void testClearStateRedraw()
    {
        IntermediateDocDrawer drawer(nullptr, m_valueConverter);
        auto doc = makeDocument(makeTextEntity(
            makePercentRegion(1000, 2000, 5000, 4000),
            {makeLine({makeChunk("ABCD", baseTextAttributes("left", "before", "sans-serif"))})}));

        RecordingDrawContext firstContext;
        drawer.draw(doc, firstContext);
        assertTextWasDrawn(firstContext);

        drawer.clearState();

        RecordingDrawContext secondContext;
        drawer.draw(doc, secondContext);
        assertTextWasDrawn(secondContext);

        CPPUNIT_ASSERT_EQUAL(firstContext.drawStringCalls.size(), secondContext.drawStringCalls.size());
        CPPUNIT_ASSERT_EQUAL(firstContext.fillCalls.size(), secondContext.fillCalls.size());
    }

    Attributes baseTextAttributes(const std::string& textAlign,
                                  const std::string& displayAlign,
                                  const std::string& fontFamily) const
    {
        return Attributes{
            {"backgroundColor", "black"},
            {"color", "white"},
            {"fontFamily", fontFamily},
            {"fontSize", "1c"},
            {"lineHeight", "1c"},
            {"textAlign", textAlign},
            {"displayAlign", displayAlign}
        };
    }

    StyleSet makeStyle(const Attributes& attributes) const
    {
        StyleSet style;
        style.merge(attributes);
        return style;
    }

    IntermediateDocument::TextChunk makeChunk(const std::string& text,
                                              const Attributes& attributes) const
    {
        IntermediateDocument::TextChunk chunk;
        chunk.m_text = text;
        chunk.m_style = makeStyle(attributes);
        return chunk;
    }

    IntermediateDocument::TextLine makeLine(std::initializer_list<IntermediateDocument::TextChunk> chunks) const
    {
        return IntermediateDocument::TextLine(chunks);
    }

    IntermediateDocument::Entity makeTextEntity(const std::shared_ptr<RegionElement>& region,
                                                std::initializer_list<IntermediateDocument::TextLine> lines) const
    {
        IntermediateDocument::Entity entity;
        entity.m_region = region;
        entity.m_textLines = std::vector<IntermediateDocument::TextLine>(lines);
        return entity;
    }

    IntermediateDocument makeDocument(const IntermediateDocument::Entity& entity) const
    {
        IntermediateDocument doc;
        doc.m_entites.push_back(entity);
        return doc;
    }

    std::shared_ptr<RegionElement> makePercentRegion(int x,
                                                     int y,
                                                     int width,
                                                     int height) const
    {
        return std::make_shared<RegionElement>(x, y, width, height);
    }

    void assertRectangle(const subttxrend::gfx::Rectangle& rectangle,
                         int x,
                         int y,
                         int width,
                         int height) const
    {
        CPPUNIT_ASSERT_EQUAL(x, rectangle.m_x);
        CPPUNIT_ASSERT_EQUAL(y, rectangle.m_y);
        CPPUNIT_ASSERT_EQUAL(width, rectangle.m_w);
        CPPUNIT_ASSERT_EQUAL(height, rectangle.m_h);
    }

    void assertTextWasDrawn(const RecordingDrawContext& drawContext) const
    {
        CPPUNIT_ASSERT(!drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(!drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(drawContext.drawStringCalls.front().glyphCount > 0);
    }

private:
    ValueConverter m_valueConverter;
};

CPPUNIT_TEST_SUITE_REGISTRATION( IntermediateDocDrawerTest );