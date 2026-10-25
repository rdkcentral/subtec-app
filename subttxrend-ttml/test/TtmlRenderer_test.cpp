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

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include <cppunit/extensions/HelperMacros.h>

#include "DataDumper.h"
#include "TtmlRenderer.hpp"
#include "Parser/IntermediateDocument.hpp"

using namespace subttxrend::ttmlengine;

namespace
{

const char* const kSinglePixelPngBase64 =
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGNgYGBgAAAABQABpfZFQAAAAABJRU5ErkJggg==";

const char* const kTwoByTwoPngBase64 =
    "iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFElEQVR4nGNkYPjfwMDAwMDEAAUAFicBg36MA2IAAAAASUVORK5CYII=";

struct FillCall
{
    subttxrend::gfx::ColorArgb color;
    subttxrend::gfx::Rectangle rectangle;
};

struct BitmapCall
{
    subttxrend::gfx::Size bitmapSize;
    subttxrend::gfx::Rectangle rectangle;
};

struct DrawStringCall
{
    std::size_t glyphCount = 0;
};

class RecordingDrawContext : public subttxrend::gfx::DrawContext
{
public:
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
        (void)destinationRect;
        (void)fgColor;
        (void)bgColor;
        (void)outlineSize;
        (void)verticalOffset;
        drawStringCalls.push_back(DrawStringCall{
            glyphs.size()});
    }

    std::vector<FillCall> fillCalls;
    std::vector<BitmapCall> bitmapCalls;
    std::vector<DrawStringCall> drawStringCalls;
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
        resizeCalls.push_back(newSize);
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
        (void)dir;
    }

    RecordingDrawContext drawContext;
    std::vector<bool> visibleCalls;
    std::vector<subttxrend::gfx::Size> resizeCalls;
    int clearCount {0};
    int updateCount {0};

private:
    subttxrend::gfx::Rectangle m_bounds;
    subttxrend::gfx::Size m_preferredSize;
    subttxrend::gfx::Size m_size;
};
} // namespace

class TtmlRendererTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(TtmlRendererTest);
    CPPUNIT_TEST(testShowHide);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testClearscreen);
    CPPUNIT_TEST(testRenderTextOnly);
    CPPUNIT_TEST(testRenderEmptyDoc);
    CPPUNIT_TEST(testSkipImageWithoutId);
    CPPUNIT_TEST(testSkipInvalidImage);
    CPPUNIT_TEST(testRenderMixedImages);
    CPPUNIT_TEST(testNoResizeForPartialImage);
    CPPUNIT_TEST(testResizeForFullscreenImage);
    CPPUNIT_TEST(testNoResizeWhenSurfaceMatches);
    CPPUNIT_TEST(testClampedSurface);
    CPPUNIT_TEST(testClearStateRedraw);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(200, 100),
                                                     subttxrend::gfx::Size(200, 100));
    }

    void tearDown() override
    {
        m_window.reset();
    }

protected:
    void testShowHide()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);

        renderer.show();
        renderer.hide();

        CPPUNIT_ASSERT_EQUAL(std::size_t(2), m_window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(m_window->visibleCalls[0]));
        CPPUNIT_ASSERT(!static_cast<bool>(m_window->visibleCalls[1]));
    }

    void testUpdate()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);

        renderer.update();
        renderer.update();

        CPPUNIT_ASSERT_EQUAL(2, m_window->updateCount);
    }

    void testClearscreen()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);

        renderer.clearscreen();
        renderer.clearscreen();

        CPPUNIT_ASSERT_EQUAL(2, m_window->clearCount);
    }

    void testRenderTextOnly()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        auto doc = makeDocument(makeTextEntity(
            makePercentRegion(0, 0, 10000, 10000),
            {makeLine({makeChunk("Renderer text", baseTextAttributes("center", "before", "sans-serif"))})}));

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(m_window->drawContext.bitmapCalls.empty());
        CPPUNIT_ASSERT(!m_window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.drawStringCalls.front().glyphCount > 0);
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
    }

    void testRenderEmptyDoc()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        IntermediateDocument doc;
        doc.setCellResolution(subttxrend::gfx::Size(20, 10));

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(m_window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.bitmapCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
    }

    void testSkipImageWithoutId()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 3000, 4000);
        entity.m_imageChunk.m_image = makeImage("", kSinglePixelPngBase64);

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(!entity.m_imageChunk.m_image || entity.m_imageChunk.m_image->getId().empty());
        CPPUNIT_ASSERT(m_window->drawContext.fillCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.bitmapCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
    }

    void testSkipInvalidImage()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 3000, 4000);
        entity.m_imageChunk.m_image = makeImage("img-invalid", "%%%not-base64%%%");

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(!doc.m_entites.front().m_imageChunk.m_bmp);
        CPPUNIT_ASSERT(m_window->drawContext.bitmapCalls.empty());
        CPPUNIT_ASSERT(m_window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
    }

    void testRenderMixedImages()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        IntermediateDocument::Entity invalidEntity;
        invalidEntity.m_region = makePercentRegion(0, 0, 5000, 5000);
        invalidEntity.m_imageChunk.m_image = makeImage("img-invalid", "not-valid-base64");

        IntermediateDocument::Entity validEntity;
        validEntity.m_region = makePercentRegion(5000, 0, 5000, 5000);
        validEntity.m_imageChunk.m_image = makeImage("img-valid", kSinglePixelPngBase64);

        IntermediateDocument doc;
        doc.setCellResolution(subttxrend::gfx::Size(20, 10));
        doc.m_entites.push_back(invalidEntity);
        doc.m_entites.push_back(validEntity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(!doc.m_entites[0].m_imageChunk.m_bmp);
        CPPUNIT_ASSERT(doc.m_entites[1].m_imageChunk.m_bmp);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_window->drawContext.bitmapCalls.size());
        assertRectangle(m_window->drawContext.bitmapCalls.front().rectangle, 100, 0, 100, 50);
        CPPUNIT_ASSERT_EQUAL(1, m_window->drawContext.bitmapCalls.front().bitmapSize.m_w);
        CPPUNIT_ASSERT_EQUAL(1, m_window->drawContext.bitmapCalls.front().bitmapSize.m_h);
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
    }

    void testNoResizeForPartialImage()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 3000, 4000);
        entity.m_imageChunk.m_image = makeImage("img-non-fullscreen", kTwoByTwoPngBase64);

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_window->drawContext.bitmapCalls.size());
        assertRectangle(m_window->drawContext.bitmapCalls.front().rectangle, 20, 20, 60, 40);
        CPPUNIT_ASSERT_EQUAL(2, m_window->drawContext.bitmapCalls.front().bitmapSize.m_w);
        CPPUNIT_ASSERT_EQUAL(2, m_window->drawContext.bitmapCalls.front().bitmapSize.m_h);
        CPPUNIT_ASSERT(m_window->resizeCalls.empty());
        CPPUNIT_ASSERT_EQUAL(200, m_window->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(100, m_window->getSize().m_h);
    }

    void testResizeForFullscreenImage()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1920, 1080),
                                                        subttxrend::gfx::Size(1920, 1080));
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(1920, 1080));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(0, 0, 10000, 10000);
        entity.m_imageChunk.m_image = makeImage("img-fullscreen", kTwoByTwoPngBase64);

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->resizeCalls.size());
        CPPUNIT_ASSERT_EQUAL(2, window->resizeCalls.front().m_w);
        CPPUNIT_ASSERT_EQUAL(2, window->resizeCalls.front().m_h);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->drawContext.bitmapCalls.size());
        assertRectangle(window->drawContext.bitmapCalls.front().rectangle, 0, 0, 2, 2);
    }

    void testNoResizeWhenSurfaceMatches()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(2, 2),
                                                        subttxrend::gfx::Size(2, 2));
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(2, 2));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(0, 0, 10000, 10000);
        entity.m_imageChunk.m_image = makeImage("img-match", kTwoByTwoPngBase64);

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(window->resizeCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->drawContext.bitmapCalls.size());
        assertRectangle(window->drawContext.bitmapCalls.front().rectangle, 0, 0, 2, 2);
    }

    void testClampedSurface()
    {
        auto window = std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1920, 1080),
                                                        subttxrend::gfx::Size(640, 360));
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(1920, 1080));

        IntermediateDocument::Entity entity;
        entity.m_region = makePercentRegion(1000, 2000, 3000, 4000);
        entity.m_imageChunk.m_image = makeImage("img-clamped", kSinglePixelPngBase64);

        auto doc = makeDocument(entity);

        renderer.renderDocument(doc);

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->resizeCalls.size());
        CPPUNIT_ASSERT_EQUAL(1280, window->resizeCalls.front().m_w);
        CPPUNIT_ASSERT_EQUAL(720, window->resizeCalls.front().m_h);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), window->drawContext.bitmapCalls.size());
        assertRectangle(window->drawContext.bitmapCalls.front().rectangle, 128, 144, 384, 288);
    }

    void testClearStateRedraw()
    {
        DataDumper dataDumper;
        TtmlRenderer renderer(nullptr, m_window.get(), dataDumper);
        renderer.setRelatedVideoSize(subttxrend::gfx::Size(200, 100));

        auto doc = makeDocument(makeTextEntity(
            makePercentRegion(0, 0, 10000, 10000),
            {makeLine({makeChunk("Renderer text", baseTextAttributes("center", "before", "sans-serif"))})}));

        renderer.renderDocument(doc);
        const auto firstDrawCount = m_window->drawContext.drawStringCalls.size();
        CPPUNIT_ASSERT(firstDrawCount > 0);

        m_window->drawContext.fillCalls.clear();
        m_window->drawContext.bitmapCalls.clear();
        m_window->drawContext.drawStringCalls.clear();

        renderer.clearState();
        renderer.renderDocument(doc);

        CPPUNIT_ASSERT(m_window->drawContext.bitmapCalls.empty());
        CPPUNIT_ASSERT_EQUAL(firstDrawCount, m_window->drawContext.drawStringCalls.size());
        CPPUNIT_ASSERT(m_window->drawContext.drawStringCalls.front().glyphCount > 0);
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
        doc.setCellResolution(subttxrend::gfx::Size(20, 10));
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

    std::shared_ptr<ImageElement> makeImage(const std::string& id,
                                            const std::string& base64) const
    {
        auto image = std::make_shared<ImageElement>();
        if (!id.empty())
        {
            image->parseAttribute("xml", "id", id);
        }
        image->appendText(base64);
        return image;
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

private:
    std::shared_ptr<RecordingWindow> m_window;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtmlRendererTest);