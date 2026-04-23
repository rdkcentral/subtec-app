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
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/FontStrip.hpp>

#include <ttxdecoder/DecodedPage.hpp>
#include <ttxdecoder/Engine.hpp>
#include <ttxdecoder/Types.hpp>

#include "GfxConfig.hpp"
#include "GfxTtxClut.hpp"
#include "GfxTtxGrid.hpp"
#include "GfxTtxGridModel.hpp"

using namespace subttxrend::ttxt;

class MockDrawContext : public subttxrend::gfx::DrawContext
{
public:
    MockDrawContext() : fillRectangleCalled(0), drawGlyphCalled(0)
    {
    }

    void fillRectangle(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rect) override
    {
        (void)rect;
        ++fillRectangleCalled;
        lastFillColor = color;
    }

    void drawUnderline(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rect) override
    {
        (void)color;
        (void)rect;
    }

    void drawPixmap(const subttxrend::gfx::ClutBitmap& bitmap,
                    const subttxrend::gfx::Rectangle& src,
                    const subttxrend::gfx::Rectangle& dst) override
    {
        (void)bitmap;
        (void)src;
        (void)dst;
    }

    void drawBitmap(const subttxrend::gfx::Bitmap& bitmap,
                    const subttxrend::gfx::Rectangle& dst) override
    {
        (void)bitmap;
        (void)dst;
    }

    void drawGlyph(const subttxrend::gfx::FontStripPtr& fontStrip,
                   std::int32_t glyphIndex,
                   const subttxrend::gfx::Rectangle& dst,
                   subttxrend::gfx::ColorArgb fgColor,
                   subttxrend::gfx::ColorArgb bgColor) override
    {
        (void)fontStrip;
        (void)glyphIndex;
        (void)dst;
        (void)fgColor;
        (void)bgColor;
        ++drawGlyphCalled;
    }

    void drawString(subttxrend::gfx::PrerenderedFont& font,
                    const subttxrend::gfx::Rectangle& dst,
                    const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                    subttxrend::gfx::ColorArgb fgColor,
                    subttxrend::gfx::ColorArgb bgColor,
                    int xOffset,
                    int yOffset) override
    {
        (void)font;
        (void)dst;
        (void)glyphs;
        (void)fgColor;
        (void)bgColor;
        (void)xOffset;
        (void)yOffset;
    }

    void reset()
    {
        fillRectangleCalled = 0;
        drawGlyphCalled = 0;
        lastFillColor = subttxrend::gfx::ColorArgb::TRANSPARENT;
    }

    int fillRectangleCalled;
    int drawGlyphCalled;
    subttxrend::gfx::ColorArgb lastFillColor;
};

class MockFontStrip : public subttxrend::gfx::FontStrip
{
public:
    explicit MockFontStrip(bool loadFontResult) : m_loadFontResult(loadFontResult) {}

    bool loadFont(const std::string& fontName,
                  const subttxrend::gfx::Size& charSize,
                  const subttxrend::gfx::FontStripMap& charMap) override
    {
        (void)fontName;
        (void)charSize;
        (void)charMap;
        return m_loadFontResult;
    }

    bool loadGlyph(std::int32_t glyphIndex,
                   const std::uint8_t* data,
                   const std::size_t size) override
    {
        (void)glyphIndex;
        (void)data;
        (void)size;
        return true;
    }

private:
    bool m_loadFontResult;
};

class MockGfxEngine : public subttxrend::gfx::Engine
{
public:
    enum class FontStripBehavior
    {
        RETURN_NULL,
        RETURN_STRIP_LOAD_FAIL
    };

    explicit MockGfxEngine(FontStripBehavior behavior) : m_behavior(behavior) {}

    void init(const std::string& displayName = {}) override
    {
        (void)displayName;
    }

    void shutdown() override {}
    void execute() override {}

    subttxrend::gfx::WindowPtr createWindow() override
    {
        return nullptr;
    }

    subttxrend::gfx::FontStripPtr createFontStrip(const subttxrend::gfx::Size& glyphSize,
                                                  const std::size_t glyphCount) override
    {
        (void)glyphSize;
        (void)glyphCount;

        switch (m_behavior)
        {
        case FontStripBehavior::RETURN_NULL:
            return nullptr;
        case FontStripBehavior::RETURN_STRIP_LOAD_FAIL:
            return std::make_shared<MockFontStrip>(false);
        default:
            return nullptr;
        }
    }

    void attach(subttxrend::gfx::WindowPtr window) override
    {
        (void)window;
    }

    void detach(subttxrend::gfx::WindowPtr window) override
    {
        (void)window;
    }

private:
    FontStripBehavior m_behavior;
};

class FakeTtxEngine : public ttxdecoder::Engine
{
public:
    FakeTtxEngine()
    {
        m_charsetMapping.fill(0);
        // Map 0x20..0x7F to themselves (96 entries)
        for (std::size_t i = 0; i < m_charsetMapping.size(); ++i)
        {
            m_charsetMapping[i] = static_cast<std::uint16_t>(0x20 + i);
        }
    }

    void resetAcquisition() override {}
    std::uint32_t process() override { return 0; }
    bool addPesPacket(const std::uint8_t* packet, std::uint16_t length) override
    {
        (void)packet;
        (void)length;
        return true;
    }
    void setCurrentPageId(const ttxdecoder::PageId& pageId) override { (void)pageId; }
    ttxdecoder::PageId getNextPageId(const ttxdecoder::PageId& inputPageId) const override
    {
        (void)inputPageId;
        return ttxdecoder::PageId();
    }
    ttxdecoder::PageId getPrevPageId(const ttxdecoder::PageId& inputPageId) const override
    {
        (void)inputPageId;
        return ttxdecoder::PageId();
    }
    ttxdecoder::PageId getPageId(ttxdecoder::PageIdType type) const override
    {
        (void)type;
        return ttxdecoder::PageId();
    }
    const ttxdecoder::DecodedPage& getPage() const override
    {
        return m_page;
    }
    void setNavigationMode(ttxdecoder::NavigationMode navigationMode) override
    {
        (void)navigationMode;
    }
    ttxdecoder::NavigationState getNavigationState() const override
    {
        return ttxdecoder::NavigationState::DEFAULT;
    }
    void getTopLinkText(std::uint16_t magazine,
                        std::uint32_t linkType,
                        std::uint16_t* linkbuffer) const override
    {
        (void)magazine;
        (void)linkType;
        (void)linkbuffer;
    }
    std::uint8_t getScreenColorIndex() const override { return 0; }
    std::uint8_t getRowColorIndex(std::uint8_t row) const override
    {
        (void)row;
        return 0;
    }
    bool getColors(std::array<std::uint32_t, 16>& colors) const override
    {
        colors.fill(0);
        return true;
    }
    const ttxdecoder::CharsetMappingArray& getCharsetMapping(ttxdecoder::Charset charset) const override
    {
        (void)charset;
        return m_charsetMapping;
    }
    void setCharsetMapping(ttxdecoder::Charset charset,
                           const ttxdecoder::CharsetMappingArray& mappingArray) override
    {
        (void)charset;
        (void)mappingArray;
    }
    void setDefaultPrimaryNationalCharset(std::uint8_t index,
                                          ttxdecoder::NationalCharset charset) override
    {
        (void)index;
        (void)charset;
    }
    void setDefaultSecondaryNationalCharset(std::uint8_t index,
                                            ttxdecoder::NationalCharset charset) override
    {
        (void)index;
        (void)charset;
    }
    uint8_t getPageControlInfo() const override { return 0; }
    void setIgnorePts(bool ignorePts) override { (void)ignorePts; }

private:
    ttxdecoder::DecodedPage m_page;
    ttxdecoder::CharsetMappingArray m_charsetMapping;
};

class GfxTtxGridTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxTtxGridTest);
    CPPUNIT_TEST(testDraw_NoModeSettings_DoesNotDraw);
    CPPUNIT_TEST(testAddModeSettings_ZeroRows_DoesNotDraw);
    CPPUNIT_TEST(testDraw_WithOneRowMode_DrawsExactlyWidthCellsOnce);
    CPPUNIT_TEST(testDraw_SecondCallWithoutChanges_DoesNotRedraw);
    CPPUNIT_TEST(testClearModeSettings_PreventsDrawingWhenCellsDirty);
    CPPUNIT_TEST(testDraw_BgAlphaZero_AffectsFillColorForEnabledCell);
    CPPUNIT_TEST(testDraw_BgAlphaMax_AffectsFillColorForEnabledCell);
    CPPUNIT_TEST(testInit_CreateFontStripFailure_Throws);
    CPPUNIT_TEST(testInit_LoadFontFailure_Throws);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override {}
    void tearDown() override {}

protected:
    void testDraw_NoModeSettings_DoesNotDraw()
    {
        GfxTtxGridModel model(Size(40, 25));
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);

        CPPUNIT_ASSERT_EQUAL(0, ctx.fillRectangleCalled);
        CPPUNIT_ASSERT_EQUAL(0, ctx.drawGlyphCalled);
    }

    void testAddModeSettings_ZeroRows_DoesNotDraw()
    {
        GfxTtxGridModel model(Size(40, 25));
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 0), Rect(0, 0, 400, 100));

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);

        CPPUNIT_ASSERT_EQUAL(0, ctx.fillRectangleCalled);
    }

    void testDraw_WithOneRowMode_DrawsExactlyWidthCellsOnce()
    {
        // By default cells are hidden; use a public model API to make a known
        // subset of cells visible without needing a real decoder page.
        const int width = 40;
        GfxTtxGridModel model(Size(width, 25));
        model.clearAll(false);
        model.refreshSelection(true, 0); // enables 4 cells in row 0
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 1), Rect(0, 0, 400, 100));

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);

        CPPUNIT_ASSERT_EQUAL(4, ctx.fillRectangleCalled);
    }

    void testDraw_SecondCallWithoutChanges_DoesNotRedraw()
    {
        const int width = 40;
        GfxTtxGridModel model(Size(width, 25));
        model.clearAll(false);
        model.refreshSelection(true, 0);
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 1), Rect(0, 0, 400, 100));

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);
        CPPUNIT_ASSERT_EQUAL(4, ctx.fillRectangleCalled);

        ctx.reset();
        grid.draw(ZoomMode::NONE, ctx, 255);
        CPPUNIT_ASSERT_EQUAL(0, ctx.fillRectangleCalled);
        CPPUNIT_ASSERT_EQUAL(0, ctx.drawGlyphCalled);
    }

    void testClearModeSettings_PreventsDrawingWhenCellsDirty()
    {
        const int width = 40;
        GfxTtxGridModel model(Size(width, 25));
        model.markAllAsChanged();

        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 1), Rect(0, 0, 400, 100));
        grid.clearModeSettings();

        // Cells are dirty, but without mode settings draw should not iterate any rows.
        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);

        CPPUNIT_ASSERT_EQUAL(0, ctx.fillRectangleCalled);
    }

    void testDraw_BgAlphaZero_AffectsFillColorForEnabledCell()
    {
        const int width = 40;
        GfxTtxGridModel model(Size(width, 25));
        model.clearAll(false);
        model.refreshSelection(true, 0); // "PAUS" -> enables first 4 cells without requiring Engine

        GfxTtxClut clut;
        clut.resetColors();
        // Make background color have non-zero RGB so alpha blending is observable.
        clut.setColor(GfxTtxClut::COLOR_INDEX_BLACK, 0xFF112233);

        GfxTtxGrid grid(model, clut);
        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 1), Rect(0, 0, 400, 100));

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 0);

        // Last drawn cell is (x=0,y=0) which is enabled by refreshSelection.
        CPPUNIT_ASSERT(ctx.lastFillColor == subttxrend::gfx::ColorArgb(0x00112233));
    }

    void testDraw_BgAlphaMax_AffectsFillColorForEnabledCell()
    {
        const int width = 40;
        GfxTtxGridModel model(Size(width, 25));
        model.clearAll(false);
        model.refreshSelection(true, 0);

        GfxTtxClut clut;
        clut.resetColors();
        clut.setColor(GfxTtxClut::COLOR_INDEX_BLACK, 0xFF112233);

        GfxTtxGrid grid(model, clut);
        grid.addModeSettings(ZoomMode::NONE, RowRange(0, 1), Rect(0, 0, 400, 100));

        MockDrawContext ctx;
        grid.draw(ZoomMode::NONE, ctx, 255);

        CPPUNIT_ASSERT(ctx.lastFillColor == subttxrend::gfx::ColorArgb(0xFF112233));
    }

    void testInit_CreateFontStripFailure_Throws()
    {
        GfxTtxGridModel model(Size(40, 25));
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        GfxConfig config;

        FakeTtxEngine ttxEngine;
        auto gfxEngine = std::make_shared<MockGfxEngine>(MockGfxEngine::FontStripBehavior::RETURN_NULL);

        CPPUNIT_ASSERT_THROW(grid.init(ttxEngine, gfxEngine, config), std::logic_error);
    }

    void testInit_LoadFontFailure_Throws()
    {
        GfxTtxGridModel model(Size(40, 25));
        GfxTtxClut clut;
        GfxTtxGrid grid(model, clut);

        GfxConfig config;

        FakeTtxEngine ttxEngine;
        auto gfxEngine = std::make_shared<MockGfxEngine>(MockGfxEngine::FontStripBehavior::RETURN_STRIP_LOAD_FAIL);

        CPPUNIT_ASSERT_THROW(grid.init(ttxEngine, gfxEngine, config), std::logic_error);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxTtxGridTest);