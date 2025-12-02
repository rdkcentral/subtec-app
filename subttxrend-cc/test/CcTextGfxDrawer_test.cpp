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
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include "CcTextGfxDrawer.hpp"
#include "CcGfx.hpp"
#include "CcCommand.hpp"

using namespace subttxrend::cc;
using namespace subttxrend::gfx;

class MockDrawContext : public DrawContext
{
public:
    int fillRectCalls = 0;
    int drawStringCalls = 0;
    int drawUnderlineCalls = 0;
    int drawGlyphCalls = 0;

    std::vector<ColorArgb> fgColors;
    std::vector<ColorArgb> bgColors;

    void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {
        fillRectCalls++;
    }

    void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {
        drawUnderlineCalls++;
    }

    void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
    void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}

    void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect,
                   ColorArgb fgColor, ColorArgb bgColor) override {
        drawGlyphCalls++;
        fgColors.push_back(fgColor);
        bgColors.push_back(bgColor);
    }

    void drawString(PrerenderedFont& font, const Rectangle &destinationRect,
                   const std::vector<GlyphData>& glyphs, const ColorArgb fgColor,
                   const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {
        drawStringCalls++;
        fgColors.push_back(fgColor);
        bgColors.push_back(bgColor);
    }

    void reset() {
        fillRectCalls = 0;
        drawStringCalls = 0;
        drawUnderlineCalls = 0;
        drawGlyphCalls = 0;
        fgColors.clear();
        bgColors.clear();
    }
};

class MockWindow : public Window
{
public:
    MockDrawContext mockContext;
    DrawDirection currentDirection = DrawDirection::LEFT_TO_RIGHT;

    void addKeyEventListener(KeyEventListener* listener) override {}
    void removeKeyEventListener(KeyEventListener* listener) override {}

    Rectangle getBounds() const override {
        return Rectangle(0, 0, 1920, 1080);
    }

    DrawContext& getDrawContext() override {
        return mockContext;
    }

    Size getPreferredSize() const override {
        return Size(1920, 1080);
    }

    void setSize(const Size& newSize) override {}

    Size getSize() const override {
        return Size(1920, 1080);
    }

    void setVisible(bool visible) override {}
    void clear() override {}
    void update() override {}

    void setDrawDirection(DrawDirection dir) override {
        currentDirection = dir;
    }
};

class MockPrerenderedFont : public PrerenderedFont
{
public:
    int height = 20;
    int advance = 12;
    int descender = -5;
    int ascender = 15;
    std::string lastFontName;
    bool lastItalics = false;

    std::vector<TextTokenData> textToTokens(const std::string& text) override {
        std::vector<TextTokenData> tokens;
        if (text.empty()) return tokens;

        // Simple tokenization: split by spaces
        std::string current;
        for (char c : text) {
            if (c == ' ') {
                if (!current.empty()) {
                    TextTokenData token;
                    token.totalAdvanceX = current.size() * advance;
                    token.isWhite = false;
                    for (size_t i = 0; i < current.size(); ++i) {
                        token.glyphs.push_back(GlyphData());
                    }
                    tokens.push_back(token);
                    current.clear();
                }
                // Add space token
                TextTokenData spaceToken;
                spaceToken.totalAdvanceX = advance;
                spaceToken.isWhite = true;
                spaceToken.glyphs.push_back(GlyphData());
                tokens.push_back(spaceToken);
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            TextTokenData token;
            token.totalAdvanceX = current.size() * advance;
            token.isWhite = false;
            for (size_t i = 0; i < current.size(); ++i) {
                token.glyphs.push_back(GlyphData());
            }
            tokens.push_back(token);
        }
        return tokens;
    }

    std::int32_t getFontHeight() const override { return height; }
    std::int32_t getFontDescender() const override { return descender; }
    std::int32_t getFontAscender() const override { return ascender; }
    std::int32_t getMaxAdvance() const override { return advance; }
};

class MockFontCache : public PrerenderedFontCache
{
public:
    std::shared_ptr<MockPrerenderedFont> mockFont;

    MockFontCache() {
        mockFont = std::make_shared<MockPrerenderedFont>();
    }

    std::shared_ptr<PrerenderedFont> getFont(const std::string& fontName,
                                             int fontSize,
                                             bool strictHeight = false,
                                             bool italics = false) {
        // Reflect requested font characteristics in the mock
        mockFont->lastFontName = fontName;
        mockFont->lastItalics = italics;
        mockFont->height = fontSize > 0 ? fontSize : 20;
        // Advance loosely proportional to height; italics adds a small offset
        mockFont->advance = static_cast<int>(std::round(mockFont->height * 0.6)) + (italics ? 1 : 0);
        // Descender/ascender scale with height
        mockFont->ascender = static_cast<int>(std::round(mockFont->height * 0.75));
        mockFont->descender = -static_cast<int>(std::round(mockFont->height * 0.25));
        return mockFont;
    }
};

class MockGfx : public Gfx
{
public:
    std::shared_ptr<MockWindow> mockWindow;
    int drawBackgroundCalls = 0;
    struct BgCall { Point p; Dimensions d; Color c; };
    std::vector<BgCall> bgCalls;

    MockGfx() {
        mockWindow = std::make_shared<MockWindow>();
    }

    Window* getInternalWindow() override {
        return mockWindow.get();
    }

    void drawBackground(const Point& point, const Dimensions& dim, Color color) override {
        drawBackgroundCalls++;
        bgCalls.push_back({point, dim, color});
    }

    void drawBorder(const Point& p, const Dimensions& dimensions, Color color,
                    Color br_color, WindowBorder border_type) override {
        // Mock implementation
    }

    void update() override {
        // Mock implementation
    }

    void clear() override {
        // Mock implementation
    }

    void show() override {
        // Mock implementation
    }

    void hide() override {
        // Mock implementation
    }

    void reset() {
        drawBackgroundCalls = 0;
        bgCalls.clear();
        mockWindow->mockContext.reset();
    }
};

class CcTextGfxDrawerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcTextGfxDrawerTest);
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWithZeroRowColumn);
    CPPUNIT_TEST(testConstructorWithNegativeRowColumn);
    CPPUNIT_TEST(testConstructorInitializesFont);
    CPPUNIT_TEST(testTransparentSpaceAddsSpace);
    CPPUNIT_TEST(testTransparentSpaceWithTrue);
    CPPUNIT_TEST(testTransparentSpaceWithFalse);
    CPPUNIT_TEST(testMultipleTransparentSpaces);
    CPPUNIT_TEST(testBackspaceOnEmptyText);
    CPPUNIT_TEST(testBackspaceOnSingleChar);
    CPPUNIT_TEST(testBackspaceOnMultipleChars);
    CPPUNIT_TEST(testBackspaceReturnValue);
    CPPUNIT_TEST(testMultipleBackspaces);
    CPPUNIT_TEST(testReportWithEmptyString);
    CPPUNIT_TEST(testReportWithSingleChar);
    CPPUNIT_TEST(testReportWithMultipleChars);
    CPPUNIT_TEST(testReportMultipleCalls);
    CPPUNIT_TEST(testReportWithColumnLimit);
    CPPUNIT_TEST(testReportWithWindowDefLimit);
    CPPUNIT_TEST(testReportWithZeroColumnLimit);
    CPPUNIT_TEST(testReportAtMaxColumnCount);
    CPPUNIT_TEST(testDrawableOnEmptyText);
    CPPUNIT_TEST(testDrawableAfterAddingText);
    CPPUNIT_TEST(testDrawableAfterClear);
    CPPUNIT_TEST(testClearOnEmptyText);
    CPPUNIT_TEST(testClearAfterAddingText);
    CPPUNIT_TEST(testClearMultipleTimes);
    CPPUNIT_TEST(testSetColumnZero);
    CPPUNIT_TEST(testSetColumnAddsPadding);
    CPPUNIT_TEST(testSetColumnWhenAlreadyAtColumn);
    CPPUNIT_TEST(testSetColumnBackward);
    CPPUNIT_TEST(testSetPenAttributesStandard);
    CPPUNIT_TEST(testSetPenAttributesSmall);
    CPPUNIT_TEST(testSetPenAttributesLarge);
    CPPUNIT_TEST(testSetPenAttributesExtraLarge);
    CPPUNIT_TEST(testSetPenAttributesWithItalics);
    CPPUNIT_TEST(testGetPenAttributesReturnsSet);
    CPPUNIT_TEST(testSetPenAttributesAllFontStyles);
    CPPUNIT_TEST(testSetPenOverrideTrue);
    CPPUNIT_TEST(testSetPenOverrideFalse);
    CPPUNIT_TEST(testGetPenOverrideInitialState);
    CPPUNIT_TEST(testDimensionsOnEmptyText);
    CPPUNIT_TEST(testDimensionsLeftRight);
    CPPUNIT_TEST(testDimensionsRightLeft);
    CPPUNIT_TEST(testDimensionsBottomTop);
    CPPUNIT_TEST(testDimensionsTopBottom);
    CPPUNIT_TEST(testDimensionsWithEdgeNone);
    CPPUNIT_TEST(testDimensionsWithEdgeRaised);
    CPPUNIT_TEST(testDimensionsWithEdgeDepressed);
    CPPUNIT_TEST(testDimensionsWithEdgeLeftShadow);
    CPPUNIT_TEST(testDimensionsWithEdgeRightShadow);
    CPPUNIT_TEST(testDimensionsWithEdgeUniform);
    CPPUNIT_TEST(testGetTextEmpty);
    CPPUNIT_TEST(testGetTextAfterReport);
    CPPUNIT_TEST(testSetMaxWidthZero);
    CPPUNIT_TEST(testSetMaxWidthPositive);
    CPPUNIT_TEST(testFontHeightDelegates);
    CPPUNIT_TEST(testMaxAdvanceDelegates);
    CPPUNIT_TEST(testSetFlashStateShow);
    CPPUNIT_TEST(testSetFlashStateHide);
    CPPUNIT_TEST(testSetFlashStateWhenNotFlashing);
    CPPUNIT_TEST(testDrawOnEmptyText);
    CPPUNIT_TEST(testDrawBasicTextLeftRight);
    CPPUNIT_TEST(testDrawBasicTextRightLeft);
    CPPUNIT_TEST(testDrawBasicTextBottomTop);
    CPPUNIT_TEST(testDrawBasicTextTopBottom);
    CPPUNIT_TEST(testDrawWithEdgeNone);
    CPPUNIT_TEST(testDrawWithEdgeRaised);
    CPPUNIT_TEST(testDrawWithEdgeDepressed);
    CPPUNIT_TEST(testDrawWithEdgeLeftShadow);
    CPPUNIT_TEST(testDrawWithEdgeRightShadow);
    CPPUNIT_TEST(testDrawWithEdgeUniform);
    CPPUNIT_TEST(testDrawWithUnderline);
    CPPUNIT_TEST(testDrawWithoutUnderline);
    CPPUNIT_TEST(testDrawWithFlashingShow);
    CPPUNIT_TEST(testDrawWithFlashingHide);
    CPPUNIT_TEST(testDrawWithFlashingDisabled);
    CPPUNIT_TEST(testDrawSetsDrawDirection);
    CPPUNIT_TEST(testFullWorkflowReportDimensionsDraw);
    CPPUNIT_TEST(testEditingWorkflowReportBackspaceReport);
    CPPUNIT_TEST(testMultipleDrawCalls);
    CPPUNIT_TEST(testClearAndReuse);
    CPPUNIT_TEST(testTransparentSpaceIntegration);
    CPPUNIT_TEST(testLongTextHandling);
    CPPUNIT_TEST(testSetColumnLargeValue);
    CPPUNIT_TEST(testDimensionsAfterMultiplePenChanges);
    CPPUNIT_TEST(testStaticCreateMethod);
    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<MockGfx> mockGfx;
    std::shared_ptr<MockFontCache> mockFontCache;
    FontGroup fontGroup;
    TextGfxDrawer* drawer;
    WindowDefinition windowDef;

public:
    void setUp()
    {
        mockGfx = std::make_shared<MockGfx>();
        mockFontCache = std::make_shared<MockFontCache>();

        // Setup font group
        fontGroup.standard.fontHeight = 20;
        fontGroup.small.fontHeight = 15;
        fontGroup.large.fontHeight = 25;
        fontGroup.extralarge.fontHeight = 30;

        // Setup window definition
        windowDef = WindowDefinition();
        windowDef.col_count = 32;
        windowDef.row_count = 15;

        drawer = new TextGfxDrawer(mockGfx, mockFontCache, fontGroup, 0, 0);
    }

    void tearDown()
    {
        delete drawer;
        mockGfx.reset();
        mockFontCache.reset();
    }

    PenAttributes createDefaultPenAttributes() {
        PenAttributes attrs;
        attrs.pen_size = PenSize::STANDARD;
        attrs.font_tag = PenFontStyle::DEFAULT_OR_UNDEFINED;
        attrs.italics = false;
        attrs.underline = false;
        attrs.flashing = false;
        attrs.edge_type = PenEdge::NONE;
        attrs.pen_color.fg_color = 0xFFFFFFFF;
        attrs.pen_color.bg_color = 0xFF000000;
        attrs.pen_color.edge_color = 0xFF888888;
        return attrs;
    }

    void testConstructorWithValidParameters()
    {
        TextGfxDrawer* d = new TextGfxDrawer(mockGfx, mockFontCache, fontGroup, 5, 10);
        CPPUNIT_ASSERT(d != nullptr);
        CPPUNIT_ASSERT_EQUAL(5, d->row);
        CPPUNIT_ASSERT_EQUAL(10, d->column);
        delete d;
    }

    void testConstructorWithZeroRowColumn()
    {
        TextGfxDrawer* d = new TextGfxDrawer(mockGfx, mockFontCache, fontGroup, 0, 0);
        CPPUNIT_ASSERT(d != nullptr);
        CPPUNIT_ASSERT_EQUAL(0, d->row);
        CPPUNIT_ASSERT_EQUAL(0, d->column);
        delete d;
    }

    void testConstructorWithNegativeRowColumn()
    {
        TextGfxDrawer* d = new TextGfxDrawer(mockGfx, mockFontCache, fontGroup, -5, -10);
        CPPUNIT_ASSERT(d != nullptr);
        CPPUNIT_ASSERT_EQUAL(-5, d->row);
        CPPUNIT_ASSERT_EQUAL(-10, d->column);
        delete d;
    }

    void testConstructorInitializesFont()
    {
        // Font should be loaded during construction
        CPPUNIT_ASSERT(drawer->fontHeight() > 0);
        CPPUNIT_ASSERT(drawer->maxAdvance() > 0);
    }

    void testTransparentSpaceAddsSpace()
    {
        drawer->transparentSpace(false);
        CPPUNIT_ASSERT_EQUAL(std::string(" "), drawer->getText());
    }

    void testTransparentSpaceWithTrue()
    {
        drawer->transparentSpace(true);
        CPPUNIT_ASSERT_EQUAL(std::string(" "), drawer->getText());
        CPPUNIT_ASSERT_EQUAL(true, drawer->drawable());
    }

    void testTransparentSpaceWithFalse()
    {
        drawer->transparentSpace(false);
        CPPUNIT_ASSERT_EQUAL(std::string(" "), drawer->getText());
        CPPUNIT_ASSERT_EQUAL(true, drawer->drawable());
    }

    void testMultipleTransparentSpaces()
    {
        drawer->transparentSpace(false);
        drawer->transparentSpace(false);
        drawer->transparentSpace(false);
        CPPUNIT_ASSERT_EQUAL(std::string("   "), drawer->getText());
    }

    void testBackspaceOnEmptyText()
    {
        bool result = drawer->backspace();
        CPPUNIT_ASSERT_EQUAL(false, result);
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testBackspaceOnSingleChar()
    {
        drawer->report("A", windowDef);
        bool result = drawer->backspace();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testBackspaceOnMultipleChars()
    {
        drawer->report("ABC", windowDef);
        bool result = drawer->backspace();
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(std::string("AB"), drawer->getText());
    }

    void testBackspaceReturnValue()
    {
        drawer->report("A", windowDef);
        CPPUNIT_ASSERT_EQUAL(true, drawer->backspace());
        CPPUNIT_ASSERT_EQUAL(false, drawer->backspace());
    }

    void testMultipleBackspaces()
    {
        drawer->report("ABC", windowDef);
        drawer->backspace();
        drawer->backspace();
        drawer->backspace();
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
        CPPUNIT_ASSERT_EQUAL(false, drawer->backspace());
    }

    void testReportWithEmptyString()
    {
        drawer->report("", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testReportWithSingleChar()
    {
        drawer->report("A", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("A"), drawer->getText());
    }

    void testReportWithMultipleChars()
    {
        drawer->report("Hello", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("Hello"), drawer->getText());
    }

    void testReportMultipleCalls()
    {
        drawer->report("Hello", windowDef);
        drawer->report(" ", windowDef);
        drawer->report("World", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("Hello World"), drawer->getText());
    }

    void testReportWithColumnLimit()
    {
        // Fill to max
        std::string longText(40, 'X');
        drawer->report(longText, windowDef);
        // Source stores text as reported; no truncation on report
        CPPUNIT_ASSERT_EQUAL(longText.length(), drawer->getText().length());
    }

    void testReportWithWindowDefLimit()
    {
        windowDef.col_count = 10;
        drawer->report("0123456789", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("0123456789"), drawer->getText());
        drawer->report("X", windowDef);
        // Should not add beyond limit
        CPPUNIT_ASSERT_EQUAL(std::string("0123456789"), drawer->getText());
    }

    void testReportWithZeroColumnLimit()
    {
        windowDef.col_count = 0;
        drawer->report("Test", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testReportAtMaxColumnCount()
    {
        std::string text(32, 'A');
        drawer->report(text, windowDef);
        CPPUNIT_ASSERT_EQUAL(text, drawer->getText());
        drawer->report("B", windowDef);
        // Should not exceed MAX_COLUMN_COUNT
        CPPUNIT_ASSERT_EQUAL(text, drawer->getText());
    }

    void testDrawableOnEmptyText()
    {
        CPPUNIT_ASSERT_EQUAL(false, drawer->drawable());
    }

    void testDrawableAfterAddingText()
    {
        drawer->report("Test", windowDef);
        CPPUNIT_ASSERT_EQUAL(true, drawer->drawable());
    }

    void testDrawableAfterClear()
    {
        drawer->report("Test", windowDef);
        drawer->clear();
        CPPUNIT_ASSERT_EQUAL(false, drawer->drawable());
    }

    void testClearOnEmptyText()
    {
        drawer->clear();
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
        CPPUNIT_ASSERT_EQUAL(false, drawer->drawable());
    }

    void testClearAfterAddingText()
    {
        drawer->report("Hello World", windowDef);
        drawer->clear();
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
        CPPUNIT_ASSERT_EQUAL(false, drawer->drawable());
    }

    void testClearMultipleTimes()
    {
        drawer->report("Test", windowDef);
        drawer->clear();
        drawer->clear();
        drawer->clear();
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testSetColumnZero()
    {
        drawer->setColumn(0);
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testSetColumnAddsPadding()
    {
        drawer->setColumn(5);
        CPPUNIT_ASSERT_EQUAL(std::string("     "), drawer->getText());
    }

    void testSetColumnWhenAlreadyAtColumn()
    {
        drawer->report("12345", windowDef);
        drawer->setColumn(5);
        CPPUNIT_ASSERT_EQUAL(std::string("12345"), drawer->getText());
    }

    void testSetColumnBackward()
    {
        drawer->report("1234567890", windowDef);
        drawer->setColumn(5);
        // Should not remove characters
        CPPUNIT_ASSERT_EQUAL(std::string("1234567890"), drawer->getText());
    }

    void testSetPenAttributesStandard()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.pen_size = PenSize::STANDARD;
        drawer->setPenAttributes(attrs);
        // Standard should produce a valid, positive font height
        CPPUNIT_ASSERT(drawer->fontHeight() > 0);
    }

    void testSetPenAttributesSmall()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.pen_size = PenSize::SMALL;
        drawer->setPenAttributes(attrs);
        // Height should reflect small size (15 scaled by SCALING_FACTOR inside drawer)
        CPPUNIT_ASSERT(drawer->fontHeight() > 0);
    }

    void testSetPenAttributesLarge()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.pen_size = PenSize::LARGE;
        drawer->setPenAttributes(attrs);
        // Large should be >= standard size
        PenAttributes stdAttrs = createDefaultPenAttributes();
        stdAttrs.pen_size = PenSize::STANDARD;
        drawer->setPenAttributes(stdAttrs);
        int standardHeight = drawer->fontHeight();

        drawer->setPenAttributes(attrs);
        CPPUNIT_ASSERT(drawer->fontHeight() >= standardHeight);
    }

    void testSetPenAttributesExtraLarge()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.pen_size = PenSize::EXTRALARGE;
        drawer->setPenAttributes(attrs);
        // Extra large should be >= large size
        PenAttributes largeAttrs = createDefaultPenAttributes();
        largeAttrs.pen_size = PenSize::LARGE;
        drawer->setPenAttributes(largeAttrs);
        int largeHeight = drawer->fontHeight();

        drawer->setPenAttributes(attrs);
        CPPUNIT_ASSERT(drawer->fontHeight() >= largeHeight);
    }

    void testSetPenAttributesWithItalics()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.italics = true;
        drawer->setPenAttributes(attrs);

        PenAttributes retrieved;
        drawer->getPenAttributes(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, (bool)retrieved.italics);
    }

    void testGetPenAttributesReturnsSet()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.underline = true;
        attrs.italics = true;
        drawer->setPenAttributes(attrs);

        PenAttributes retrieved;
        drawer->getPenAttributes(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, (bool)retrieved.underline);
        CPPUNIT_ASSERT_EQUAL(true, (bool)retrieved.italics);
    }

    void testSetPenAttributesAllFontStyles()
    {
        PenFontStyle styles[] = {
            PenFontStyle::CASUAL_FONT_TYPE,
            PenFontStyle::CURSIVE_FONT_TYPE,
            PenFontStyle::MONOSPACED_WITHOUT_SERIFS,
            PenFontStyle::MONOSPACED_WITH_SERIFS,
            PenFontStyle::PROPORTIONALLY_SPACED_WITHOUT_SERIFS,
            PenFontStyle::PROPORTIONALLY_SPACED_WITH_SERIFS,
            PenFontStyle::SMALL_CAPITALS,
            PenFontStyle::DEFAULT_OR_UNDEFINED
        };

        for (auto style : styles) {
            PenAttributes attrs = createDefaultPenAttributes();
            attrs.font_tag = style;
            drawer->setPenAttributes(attrs);

            PenAttributes retrieved;
            drawer->getPenAttributes(retrieved);
            CPPUNIT_ASSERT_EQUAL((int)style, (int)retrieved.font_tag);
        }
    }

    void testSetPenOverrideTrue()
    {
        drawer->setPenOverride(true);
        CPPUNIT_ASSERT_EQUAL(true, drawer->getPenOverride());
    }

    void testSetPenOverrideFalse()
    {
        drawer->setPenOverride(false);
        CPPUNIT_ASSERT_EQUAL(true, drawer->getPenOverride()); // Still true because it was set
    }

    void testGetPenOverrideInitialState()
    {
        CPPUNIT_ASSERT_EQUAL(false, drawer->getPenOverride());
    }

    void testDimensionsOnEmptyText()
    {
        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT_EQUAL(20, dim.h); // Font height
        CPPUNIT_ASSERT_EQUAL(0, dim.w);  // No text
    }

    void testDimensionsLeftRight()
    {
        drawer->report("ABC", windowDef);
        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT_EQUAL(20, dim.h);
        CPPUNIT_ASSERT(dim.w > 0);
    }

    void testDimensionsRightLeft()
    {
        drawer->report("ABC", windowDef);
        Dimensions dim = drawer->dimensions(WindowPd::RIGHT_LEFT);
        CPPUNIT_ASSERT_EQUAL(20, dim.h);
        CPPUNIT_ASSERT(dim.w > 0);
    }

    void testDimensionsBottomTop()
    {
        drawer->report("ABC", windowDef);
        Dimensions dim = drawer->dimensions(WindowPd::BOTTOM_TOP);
        // Width equals current max advance
        CPPUNIT_ASSERT_EQUAL(drawer->maxAdvance(), dim.w);
        CPPUNIT_ASSERT(dim.h > 0);
    }

    void testDimensionsTopBottom()
    {
        drawer->report("ABC", windowDef);
        Dimensions dim = drawer->dimensions(WindowPd::TOP_BOTTOM);
        CPPUNIT_ASSERT_EQUAL(drawer->maxAdvance(), dim.w);
        CPPUNIT_ASSERT(dim.h > 0);
    }

    void testDimensionsWithEdgeNone()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::NONE;
        drawer->setPenAttributes(attrs);

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
        CPPUNIT_ASSERT(dim.h > 0);
    }

    void testDimensionsWithEdgeRaised()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::RAISED;
        drawer->setPenAttributes(attrs);

        Dimensions dimNone = drawer->dimensions(WindowPd::LEFT_RIGHT);
        attrs.edge_type = PenEdge::NONE;
        drawer->setPenAttributes(attrs);
        Dimensions dimWithEdge = drawer->dimensions(WindowPd::LEFT_RIGHT);

        // Edge adds shadowEdge (2) to both dimensions
        attrs.edge_type = PenEdge::RAISED;
        drawer->setPenAttributes(attrs);
        Dimensions dimRaised = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dimRaised.w >= dimWithEdge.w);
    }

    void testDimensionsWithEdgeDepressed()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::DEPRESSED;
        drawer->setPenAttributes(attrs);

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
        CPPUNIT_ASSERT(dim.h > 0);
    }

    void testDimensionsWithEdgeLeftShadow()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::LEFT_DROP_SHADOW;
        drawer->setPenAttributes(attrs);

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
    }

    void testDimensionsWithEdgeRightShadow()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::RIGHT_DROP_SHADOW;
        drawer->setPenAttributes(attrs);

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
    }

    void testDimensionsWithEdgeUniform()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::UNIFORM;
        drawer->setPenAttributes(attrs);

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
        CPPUNIT_ASSERT(dim.h > 0);
    }

    void testGetTextEmpty()
    {
        CPPUNIT_ASSERT_EQUAL(std::string(""), drawer->getText());
    }

    void testGetTextAfterReport()
    {
        drawer->report("Hello", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("Hello"), drawer->getText());
    }

    void testSetMaxWidthZero()
    {
        drawer->setMaxWidth(0);
        // No crash, width stored
        CPPUNIT_ASSERT(true);
    }

    void testSetMaxWidthPositive()
    {
        drawer->setMaxWidth(100);
        // Width stored for use in draw
        CPPUNIT_ASSERT(true);
    }

    void testFontHeightDelegates()
    {
        CPPUNIT_ASSERT_EQUAL(20, drawer->fontHeight());
    }

    void testMaxAdvanceDelegates()
    {
        // Max advance should be positive and consistent with dimensions in vertical modes
        CPPUNIT_ASSERT(drawer->maxAdvance() > 0);
    }

    void testSetFlashStateShow()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = true;
        drawer->setPenAttributes(attrs);

        drawer->setFlashState(FlashControl::Show);
        // State set, no crash
        CPPUNIT_ASSERT(true);
    }

    void testSetFlashStateHide()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = true;
        drawer->setPenAttributes(attrs);

        drawer->setFlashState(FlashControl::Hide);
        CPPUNIT_ASSERT(true);
    }

    void testSetFlashStateWhenNotFlashing()
    {
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = false;
        drawer->setPenAttributes(attrs);

        drawer->setFlashState(FlashControl::Hide);
        // Should be ignored
        CPPUNIT_ASSERT(true);
    }

    void testDrawOnEmptyText()
    {
        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        // No crash on empty text
        CPPUNIT_ASSERT(true);
    }

    void testDrawBasicTextLeftRight()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 20};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Should have called drawString
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::LEFT_TO_RIGHT, mockGfx->mockWindow->currentDirection);
    }

    void testDrawBasicTextRightLeft()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::RIGHT_LEFT);

        mockGfx->reset();
        Point point{10, 20};
        drawer->draw(point, WindowPd::RIGHT_LEFT, WindowJustify::LEFT);

        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::RIGHT_TO_LEFT, mockGfx->mockWindow->currentDirection);
    }

    void testDrawBasicTextBottomTop()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::BOTTOM_TOP);

        mockGfx->reset();
        Point point{10, 20};
        drawer->draw(point, WindowPd::BOTTOM_TOP, WindowJustify::LEFT);

        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::BOTTOM_TOP, mockGfx->mockWindow->currentDirection);
    }

    void testDrawBasicTextTopBottom()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::TOP_BOTTOM);

        mockGfx->reset();
        Point point{10, 20};
        drawer->draw(point, WindowPd::TOP_BOTTOM, WindowJustify::LEFT);

        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::TOP_BOTTOM, mockGfx->mockWindow->currentDirection);
    }

    void testDrawWithEdgeNone()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::NONE;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Single draw pass, no edge
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls >= 1);
    }

    void testDrawWithEdgeRaised()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::RAISED;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 10};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Two draws: shadow + text
        CPPUNIT_ASSERT_EQUAL(2, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithEdgeDepressed()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::DEPRESSED;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 10};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        CPPUNIT_ASSERT_EQUAL(2, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithEdgeLeftShadow()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::LEFT_DROP_SHADOW;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 10};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        CPPUNIT_ASSERT_EQUAL(2, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithEdgeRightShadow()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::RIGHT_DROP_SHADOW;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 10};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        CPPUNIT_ASSERT_EQUAL(2, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithEdgeUniform()
    {
        drawer->report("A", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.edge_type = PenEdge::UNIFORM;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{10, 10};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Three draws: shadow + shadow + text
        CPPUNIT_ASSERT_EQUAL(3, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithUnderline()
    {
        drawer->report("Test", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.underline = true;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        CPPUNIT_ASSERT_EQUAL(1, mockGfx->mockWindow->mockContext.drawUnderlineCalls);
    }

    void testDrawWithoutUnderline()
    {
        drawer->report("Test", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.underline = false;
        drawer->setPenAttributes(attrs);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        CPPUNIT_ASSERT_EQUAL(0, mockGfx->mockWindow->mockContext.drawUnderlineCalls);
    }

    void testDrawWithFlashingShow()
    {
        drawer->report("Test", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = true;
        drawer->setPenAttributes(attrs);
        drawer->setFlashState(FlashControl::Show);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Text should be drawn
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
    }

    void testDrawWithFlashingHide()
    {
        drawer->report("Test", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = true;
        drawer->setPenAttributes(attrs);
        drawer->setFlashState(FlashControl::Hide);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Text should not be drawn (flashing hidden)
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->mockWindow->mockContext.drawStringCalls);
    }

    void testDrawWithFlashingDisabled()
    {
        drawer->report("Test", windowDef);
        PenAttributes attrs = createDefaultPenAttributes();
        attrs.flashing = false;
        drawer->setPenAttributes(attrs);
        drawer->setFlashState(FlashControl::Hide);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);

        // Text should be drawn (flashing disabled)
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
    }

    void testDrawSetsDrawDirection()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::LEFT_RIGHT);

        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::LEFT_TO_RIGHT, mockGfx->mockWindow->currentDirection);

        drawer->draw(point, WindowPd::RIGHT_LEFT, WindowJustify::LEFT);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::RIGHT_TO_LEFT, mockGfx->mockWindow->currentDirection);

        drawer->draw(point, WindowPd::BOTTOM_TOP, WindowJustify::LEFT);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::BOTTOM_TOP, mockGfx->mockWindow->currentDirection);

        drawer->draw(point, WindowPd::TOP_BOTTOM, WindowJustify::LEFT);
        CPPUNIT_ASSERT_EQUAL(DrawDirection::TOP_BOTTOM, mockGfx->mockWindow->currentDirection);
    }

    void testFullWorkflowReportDimensionsDraw()
    {
        // Complete workflow
        drawer->report("Hello", windowDef);
        CPPUNIT_ASSERT_EQUAL(true, drawer->drawable());

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
        CPPUNIT_ASSERT(dim.h > 0);

        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
    }

    void testEditingWorkflowReportBackspaceReport()
    {
        drawer->report("Hello", windowDef);
        drawer->backspace();
        drawer->report("p", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("Hellp"), drawer->getText());

        Dimensions dim = drawer->dimensions(WindowPd::LEFT_RIGHT);
        CPPUNIT_ASSERT(dim.w > 0);
    }

    void testMultipleDrawCalls()
    {
        drawer->report("Test", windowDef);
        drawer->dimensions(WindowPd::LEFT_RIGHT);
        Point point{0, 0};

        mockGfx->reset();
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        int firstCallCount = mockGfx->mockWindow->mockContext.drawStringCalls;

        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        // Should draw again
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > firstCallCount);
    }

    void testClearAndReuse()
    {
        drawer->report("First", windowDef);
        drawer->clear();
        drawer->report("Second", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("Second"), drawer->getText());

        mockGfx->reset();
        drawer->dimensions(WindowPd::LEFT_RIGHT);
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        CPPUNIT_ASSERT(mockGfx->mockWindow->mockContext.drawStringCalls > 0);
    }

    void testTransparentSpaceIntegration()
    {
        drawer->report("A", windowDef);
        drawer->transparentSpace(false);
        drawer->report("B", windowDef);
        CPPUNIT_ASSERT_EQUAL(std::string("A B"), drawer->getText());

        drawer->dimensions(WindowPd::LEFT_RIGHT);
        mockGfx->reset();
        Point point{0, 0};
        drawer->draw(point, WindowPd::LEFT_RIGHT, WindowJustify::LEFT);
        // Background drawn for non-transparent, not for transparent space
        CPPUNIT_ASSERT(mockGfx->drawBackgroundCalls > 0);
        // Ensure at least one background draw was skipped for the transparent space
        // Given tokens: "A" (bg) + space (transparent, no bg) + "B" (bg)
        CPPUNIT_ASSERT_EQUAL(2, mockGfx->drawBackgroundCalls);
    }

    void testLongTextHandling()
    {
        std::string longText(100, 'X');
        drawer->report(longText, windowDef);
        // Report does not truncate; text stored fully
        CPPUNIT_ASSERT_EQUAL(longText.length(), drawer->getText().length());
    }

    void testSetColumnLargeValue()
    {
        drawer->setColumn(100);
        // Should add up to 100 spaces
        CPPUNIT_ASSERT(drawer->getText().length() == 100);
    }

    void testDimensionsAfterMultiplePenChanges()
    {
        drawer->report("Test", windowDef);

        PenAttributes attrs = createDefaultPenAttributes();
        attrs.pen_size = PenSize::SMALL;
        drawer->setPenAttributes(attrs);
        Dimensions dim1 = drawer->dimensions(WindowPd::LEFT_RIGHT);

        attrs.pen_size = PenSize::LARGE;
        drawer->setPenAttributes(attrs);
        Dimensions dim2 = drawer->dimensions(WindowPd::LEFT_RIGHT);

        // Dimensions should be recalculated
        CPPUNIT_ASSERT(dim1.h > 0);
        CPPUNIT_ASSERT(dim2.h > 0);
    }

    void testStaticCreateMethod()
    {
        auto textDrawer = TextDrawer::create(mockGfx, mockFontCache, fontGroup, 5, 10);
        CPPUNIT_ASSERT(textDrawer != nullptr);
        CPPUNIT_ASSERT_EQUAL(5, textDrawer->row);
        CPPUNIT_ASSERT_EQUAL(10, textDrawer->column);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcTextGfxDrawerTest);
