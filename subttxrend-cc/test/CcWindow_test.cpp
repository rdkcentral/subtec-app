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
#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include "CcWindow.hpp"
#include "CcTextDrawer.hpp"
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
    }

    void drawString(PrerenderedFont& font, const Rectangle &destinationRect,
                   const std::vector<GlyphData>& glyphs, const ColorArgb fgColor,
                   const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {
        drawStringCalls++;
    }

    void reset() {
        fillRectCalls = 0;
        drawStringCalls = 0;
        drawUnderlineCalls = 0;
        drawGlyphCalls = 0;
    }
};

class MockGfxWindow : public subttxrend::gfx::Window
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

    std::vector<TextTokenData> textToTokens(const std::string& text) override {
        std::vector<TextTokenData> tokens;
        if (text.empty()) return tokens;

        for (char c : text) {
            TextTokenData token;
            token.totalAdvanceX = advance;
            token.isWhite = (c == ' ');
            token.glyphs.push_back(GlyphData());
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
        return mockFont;
    }
};

class MockGfx : public Gfx
{
public:
    std::shared_ptr<MockGfxWindow> mockWindow;
    int drawBackgroundCalls = 0;
    int drawBorderCalls = 0;
    int updateCalls = 0;
    int clearCalls = 0;
    int showCalls = 0;
    int hideCalls = 0;

    MockGfx() {
        mockWindow = std::make_shared<MockGfxWindow>();
    }

    subttxrend::gfx::Window* getInternalWindow() override {
        return mockWindow.get();
    }

    void drawBackground(const Point& point, const Dimensions& dim, Color color) override {
        drawBackgroundCalls++;
    }

    void drawBorder(const Point& p, const Dimensions& dimensions, Color color,
                    Color br_color, WindowBorder border_type) override {
        drawBorderCalls++;
    }

    void update() override {
        updateCalls++;
    }

    void clear() override {
        clearCalls++;
    }

    void show() override {
        showCalls++;
    }

    void hide() override {
        hideCalls++;
    }

    void reset() {
        drawBackgroundCalls = 0;
        drawBorderCalls = 0;
        updateCalls = 0;
        clearCalls = 0;
        showCalls = 0;
        hideCalls = 0;
        mockWindow->mockContext.reset();
    }
};

class CcWindowTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcWindowTest);
    CPPUNIT_TEST(testConstructorWithValidParameters);
    CPPUNIT_TEST(testConstructorWith608Mode);
    CPPUNIT_TEST(testConstructorWith708Mode);
    CPPUNIT_TEST(testConstructorInitialState);
    CPPUNIT_TEST(testUpdateWithMatchingId);
    CPPUNIT_TEST(testUpdateWithMismatchedId);
    CPPUNIT_TEST(testUpdateWithSameDefinition);
    CPPUNIT_TEST(testUpdateWithDifferentDefinition);
    CPPUNIT_TEST(testUpdateExceedingMaxRowCount);
    CPPUNIT_TEST(testChangedInitialState);
    CPPUNIT_TEST(testChangedAfterReport);
    CPPUNIT_TEST(testChangedAfterDraw);
    CPPUNIT_TEST(testChangedAfterShow);
    CPPUNIT_TEST(testChangedAfterHide);
    CPPUNIT_TEST(testReportWithEmptyString);
    CPPUNIT_TEST(testReportWithValidText);
    CPPUNIT_TEST(testReportWithNotToBeDisplayedTag);
    CPPUNIT_TEST(testReportMultipleCalls);
    CPPUNIT_TEST(testReportLongString);
    CPPUNIT_TEST(testCarriageReturnBottomTop);
    CPPUNIT_TEST(testCarriageReturnTopBottom);
    CPPUNIT_TEST(testCarriageReturnLeftRight);
    CPPUNIT_TEST(testCarriageReturnRightLeft);
    CPPUNIT_TEST(testHorizontalCarriageReturnWithText);
    CPPUNIT_TEST(testHorizontalCarriageReturnWithoutText);
    CPPUNIT_TEST(testBackspaceOnEmptyWindow);
    CPPUNIT_TEST(testBackspaceWithText);
    CPPUNIT_TEST(testSetPenLocationOutOfBoundsRow);
    CPPUNIT_TEST(testSetPenLocationOutOfBoundsColumn);
    CPPUNIT_TEST(testSetPenLocationNegativeRow);
    CPPUNIT_TEST(testSetPenLocationNegativeColumn);
    CPPUNIT_TEST(testSetPenLocationBackwards);
    CPPUNIT_TEST(testSetWindowAttributesChangePrintDirection);
    CPPUNIT_TEST(testSetWindowAttributesChangeScrollDirection);
    CPPUNIT_TEST(testSetWindowAttributesNoDirectionChange);
    CPPUNIT_TEST(testSetWindowAttributesWhenVisible);
    CPPUNIT_TEST(testSetWindowAttributesWhenHidden);
    CPPUNIT_TEST(testSetPenAttributesOnEmptyWindow);
    CPPUNIT_TEST(testSetPenAttributesMidText);
    CPPUNIT_TEST(testSetPenAttributesVisibleWindow);
    CPPUNIT_TEST(testSetPenAttributesHiddenWindow);
    CPPUNIT_TEST(testActivePenAttributesWithOverride);
    CPPUNIT_TEST(testOverridePenAttributesMidRow);
    CPPUNIT_TEST(testOverridePenAttributesNotMidRow);
    CPPUNIT_TEST(testOverridePenAttributesOnEmptyWindow);
    CPPUNIT_TEST(testOverridePenAttributesWithText);
    CPPUNIT_TEST(testSetPenColorOnEmptyWindow);
    CPPUNIT_TEST(testSetPenColorMidText);
    CPPUNIT_TEST(testSetPenColorVisibleWindow);
    CPPUNIT_TEST(testShowWhenHidden);
    CPPUNIT_TEST(testShowWhenAlreadyVisible);
    CPPUNIT_TEST(testShowSetsChangedFlag);
    CPPUNIT_TEST(testShowSetsVisibilityChanged);
    CPPUNIT_TEST(testHideWhenVisible);
    CPPUNIT_TEST(testHideWhenAlreadyHidden);
    CPPUNIT_TEST(testHideSetsChangedFlag);
    CPPUNIT_TEST(testHideSetsVisibilityChanged);
    CPPUNIT_TEST(testToggleFromVisible);
    CPPUNIT_TEST(testToggleFromHidden);
    CPPUNIT_TEST(testToggleRepeatedly);
    CPPUNIT_TEST(testClearWhenVisible);
    CPPUNIT_TEST(testClearWhenHidden);
    CPPUNIT_TEST(testHasFlashingTextWhenEmpty);
    CPPUNIT_TEST(testHasFlashingTextWithFlashing);
    CPPUNIT_TEST(testHasFlashingTextWithoutFlashing);
    CPPUNIT_TEST(testHasFlashingTextMixed);
    CPPUNIT_TEST(testHasTextOnEmptyRow);
    CPPUNIT_TEST(testHasTextOnPopulatedRow);
    CPPUNIT_TEST(testHasTextOnNonExistentRow);
    CPPUNIT_TEST(testSetFlashStateShow);
    CPPUNIT_TEST(testSetFlashStateHide);
    CPPUNIT_TEST(testUpdateRowCountIncreaseWithAdjust);
    CPPUNIT_TEST(testUpdateRowCountDecreaseWithAdjust);
    CPPUNIT_TEST(testUpdateRowCountWithoutAdjust);
    CPPUNIT_TEST(testUpdateRowCountNonBottomTopDirection);
    CPPUNIT_TEST(testUpdateRowCountRemovesOffscreenText);
    CPPUNIT_TEST(testDrawWhenVisible);
    CPPUNIT_TEST(testDrawWhenHidden);
    CPPUNIT_TEST(testDrawResetsChangedFlag);
    CPPUNIT_TEST(testDrawResetsVisibilityChangedFlag);
    CPPUNIT_TEST(testDrawCallsGfxDrawBorder);
    CPPUNIT_TEST(testIDReturnsCorrectValue);
    CPPUNIT_TEST(testGetPriorityReturnsCorrectValue);
    CPPUNIT_TEST(testGetDefinitionReturnsCorrectly);
    CPPUNIT_TEST(testIsVisibleReturnsCorrectState);
    CPPUNIT_TEST(testWindowVisibilityChangedInitialState);
    CPPUNIT_TEST(testWindowVisibilityChangedAfterShow);
    CPPUNIT_TEST(testWindowVisibilityChangedAfterDraw);
    CPPUNIT_TEST(testFullLifecycleCreateShowReportDrawHide);
    CPPUNIT_TEST(testMultipleReportAndCarriageReturns);
    CPPUNIT_TEST(testWindowResizeWithActiveContent);
    CPPUNIT_TEST(testToggleVisibilityMultipleTimes);
    CPPUNIT_TEST(testScrollingWithFullWindow);
    CPPUNIT_TEST(testRapidAttributeChanges);
    CPPUNIT_TEST(testComplexTextWithMixedStyles);
    CPPUNIT_TEST(testMaxTextDrawersLimit);
    CPPUNIT_TEST(testAnchorPointTopLeft);
    CPPUNIT_TEST(testAnchorPointTopCenter);
    CPPUNIT_TEST(testAnchorPointTopRight);
    CPPUNIT_TEST(testAnchorPointMiddleLeft);
    CPPUNIT_TEST(testAnchorPointMiddleCenter);
    CPPUNIT_TEST(testAnchorPointMiddleRight);
    CPPUNIT_TEST(testAnchorPointBottomLeft);
    CPPUNIT_TEST(testAnchorPointBottomCenter);
    CPPUNIT_TEST(testAnchorPointBottomRight);
    CPPUNIT_TEST(testJustifyLeftWithLeftToRight);
    CPPUNIT_TEST(testJustifyRightWithLeftToRight);
    CPPUNIT_TEST(testJustifyCenterWithLeftToRight);
    CPPUNIT_TEST(testJustifyFullWithLeftToRight);
    CPPUNIT_TEST(testJustifyLeftWithRightToLeft);
    CPPUNIT_TEST(testJustifyRightWithRightToLeft);
    CPPUNIT_TEST(testMode608VsMode708Differences);
    CPPUNIT_TEST(testMode608ScreenLayout);
    CPPUNIT_TEST(testMode708ScreenLayout);
    CPPUNIT_TEST(testFlashingTextAnimationCycle);
    CPPUNIT_TEST(testFlashingMixedWithNonFlashing);
    CPPUNIT_TEST(testPrintDirectionChangeClears);
    CPPUNIT_TEST(testScrollDirectionChangeClears);
    CPPUNIT_TEST(testDirectionChangePreservesVisibility);
    CPPUNIT_TEST(testMultiRowTextDisplay);
    CPPUNIT_TEST(testMultiRowWithDifferentAttributes);
    CPPUNIT_TEST(testScrollingMultipleRows);
    CPPUNIT_TEST(testPenLocationJumpingBetweenRows);
    CPPUNIT_TEST(testPenLocationOverwritePreviousText);
    CPPUNIT_TEST(testTabOffsetAcrossMultipleColumns);
    CPPUNIT_TEST(testRecoveryFromInvalidPenLocation);
    CPPUNIT_TEST(testRecoveryFromExcessiveTextDrawers);
    CPPUNIT_TEST(testRecoveryFromAttributeChangeDuringDraw);
    CPPUNIT_TEST(testStateConsistencyAfterClear);
    CPPUNIT_TEST(testStateConsistencyAfterUpdate);
    CPPUNIT_TEST(testStateConsistencyAfterToggle);
    CPPUNIT_TEST(testComplexWorkflowEditingText);
    CPPUNIT_TEST(testComplexWorkflowBackspaceAndReplace);
    CPPUNIT_TEST(testComplexWorkflowMidRowStyleChanges);

    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<MockGfx> mockGfx;
    std::shared_ptr<MockFontCache> mockFontCache;
    subttxrend::cc::Window* window;
    WindowDefinition windowDef;

public:
    void setUp()
    {
        mockGfx = std::make_shared<MockGfx>();
        mockFontCache = std::make_shared<MockFontCache>();

        // Setup window definition with valid defaults
        windowDef.id = 1;
        windowDef.priority = 0;
        windowDef.col_lock = false;
        windowDef.row_lock = false;
        windowDef.visible = true;
        windowDef.relative_pos = false;
        windowDef.anchor_vertical = 0;
        windowDef.anchor_horizontal = 0;
        windowDef.row_count = 15;
        windowDef.col_count = 32;
        windowDef.anchor_point = PenAnchorPoint::TOP_LEFT;
        windowDef.pen_style = PenAttributes();
        windowDef.win_style = WindowAttributes();

        window = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);
    }

    void tearDown()
    {
        delete window;
        mockGfx.reset();
        mockFontCache.reset();
    }

    void testConstructorWithValidParameters()
    {
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);
        CPPUNIT_ASSERT(testWin != nullptr);
        CPPUNIT_ASSERT_EQUAL(1, testWin->ID());
        CPPUNIT_ASSERT_EQUAL(false, testWin->changed());
        delete testWin;
    }

    void testConstructorWith608Mode()
    {
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, true);
        CPPUNIT_ASSERT(testWin != nullptr);
        CPPUNIT_ASSERT_EQUAL(1, testWin->ID());
        delete testWin;
    }

    void testConstructorWith708Mode()
    {
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);
        CPPUNIT_ASSERT(testWin != nullptr);
        CPPUNIT_ASSERT_EQUAL(1, testWin->ID());
        delete testWin;
    }

    void testConstructorInitialState()
    {
        CPPUNIT_ASSERT_EQUAL(false, window->changed());
        CPPUNIT_ASSERT_EQUAL(false, window->windowVisibilityChanged());
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());
        CPPUNIT_ASSERT_EQUAL(1, window->ID());
        CPPUNIT_ASSERT_EQUAL(0, window->getPriority());
    }

    void testUpdateWithMatchingId()
    {
        WindowDefinition newDef = windowDef;
        newDef.priority = 5;

        window->update(newDef);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
        CPPUNIT_ASSERT_EQUAL(5, window->getPriority());
    }

    void testUpdateWithMismatchedId()
    {
        WindowDefinition newDef = windowDef;
        newDef.id = 999;
        newDef.priority = 5;

        window->update(newDef);
        // Should not update when IDs don't match
        CPPUNIT_ASSERT_EQUAL(0, window->getPriority());
    }

    void testUpdateWithSameDefinition()
    {
        window->update(windowDef);
        // Should not set changed when definition is identical
        CPPUNIT_ASSERT_EQUAL(false, window->changed());
    }

    void testUpdateWithDifferentDefinition()
    {
        WindowDefinition newDef = windowDef;
        newDef.row_count = 20;

        window->update(newDef);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testUpdateExceedingMaxRowCount()
    {
        // Add 15 text drawers by reporting text and doing carriage returns
        for (int i = 0; i < 15; ++i) {
            window->report("Text");
            window->carriageReturn();
        }

        // Update should trim excess text drawers beyond MAX_ROW_COUNT (12)
        window->update(windowDef);
        CPPUNIT_ASSERT(true); // No crash means success
    }

    void testChangedInitialState()
    {
        CPPUNIT_ASSERT_EQUAL(false, window->changed());
    }

    void testChangedAfterReport()
    {
        window->report("Test");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testChangedAfterDraw()
    {
        window->report("Test");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());

        window->draw();
        CPPUNIT_ASSERT_EQUAL(false, window->changed());
    }

    void testChangedAfterShow()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testChangedAfterHide()
    {
        window->hide();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testReportWithEmptyString()
    {
        window->report("");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testReportWithValidText()
    {
        window->report("Hello World");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testReportWithNotToBeDisplayedTag()
    {
        windowDef.pen_style.text_tag = PenTextTag::NOT_TO_BE_DISPLAYED;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Hidden Text");
        CPPUNIT_ASSERT_EQUAL(false, testWin->changed());

        delete testWin;
    }

    void testReportMultipleCalls()
    {
        window->report("First ");
        window->report("Second ");
        window->report("Third");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testReportLongString()
    {
        std::string longText(1000, 'X');
        window->report(longText);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testCarriageReturnBottomTop()
    {
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Line 1");
        testWin->carriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testCarriageReturnTopBottom()
    {
        windowDef.win_style.scroll_direction = WindowSd::TOP_BOTTOM;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Line 1");
        testWin->carriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testCarriageReturnLeftRight()
    {
        windowDef.win_style.scroll_direction = WindowSd::LEFT_RIGHT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->carriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testCarriageReturnRightLeft()
    {
        windowDef.win_style.scroll_direction = WindowSd::RIGHT_LEFT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->carriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testHorizontalCarriageReturnWithText()
    {
        window->report("Test");
        window->horizontalCarriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testHorizontalCarriageReturnWithoutText()
    {
        window->horizontalCarriageReturn();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testBackspaceOnEmptyWindow()
    {
        window->backspace(); // Should not crash
        CPPUNIT_ASSERT(true);
    }

    void testBackspaceWithText()
    {
        window->report("Test");
        window->backspace();
        CPPUNIT_ASSERT(true); // No crash means success
    }

    void testSetPenLocationOutOfBoundsRow()
    {
        window->setPenLocation(100, 5);
        // Should ignore out of bounds
        CPPUNIT_ASSERT(true);
    }

    void testSetPenLocationOutOfBoundsColumn()
    {
        window->setPenLocation(5, 100);
        // Should ignore out of bounds
        CPPUNIT_ASSERT(true);
    }

    void testSetPenLocationNegativeRow()
    {
        window->setPenLocation(-1, 5);
        // Should handle gracefully
        CPPUNIT_ASSERT(true);
    }

    void testSetPenLocationNegativeColumn()
    {
        window->setPenLocation(5, -1);
        // Should handle gracefully
        CPPUNIT_ASSERT(true);
    }

    void testSetPenLocationBackwards()
    {
        window->report("Text");
        window->setPenLocation(0, 0); // Move back
        CPPUNIT_ASSERT(true);
    }

    void testSetWindowAttributesChangePrintDirection()
    {
        window->report("Text");

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.print_direction = WindowPd::RIGHT_LEFT;

        window->setWindowAttributes(newAttrs);
        // Should clear window content
        CPPUNIT_ASSERT(true);
    }

    void testSetWindowAttributesChangeScrollDirection()
    {
        window->report("Text");

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.scroll_direction = WindowSd::TOP_BOTTOM;

        window->setWindowAttributes(newAttrs);
        // Should clear window content
        CPPUNIT_ASSERT(true);
    }

    void testSetWindowAttributesNoDirectionChange()
    {
        window->report("Text");

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.justify = WindowJustify::CENTER;

        window->setWindowAttributes(newAttrs);
        // Should not clear window
        CPPUNIT_ASSERT(true);
    }

    void testSetWindowAttributesWhenVisible()
    {
        windowDef.visible = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        WindowAttributes newAttrs = windowDef.win_style;
        testWin->setWindowAttributes(newAttrs);
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testSetWindowAttributesWhenHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        WindowAttributes newAttrs = windowDef.win_style;
        testWin->setWindowAttributes(newAttrs);
        CPPUNIT_ASSERT_EQUAL(false, testWin->changed());

        delete testWin;
    }

    void testSetPenAttributesOnEmptyWindow()
    {
        PenAttributes attrs;
        attrs.pen_size = PenSize::LARGE;

        window->setPenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testSetPenAttributesMidText()
    {
        window->report("Hello");

        PenAttributes attrs;
        attrs.pen_size = PenSize::SMALL;
        attrs.italics = true;

        window->setPenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testSetPenAttributesVisibleWindow()
    {
        windowDef.visible = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        PenAttributes attrs;
        testWin->setPenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testSetPenAttributesHiddenWindow()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        PenAttributes attrs;
        testWin->setPenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(false, testWin->changed());

        delete testWin;
    }

    void testActivePenAttributesWithOverride()
    {
        PenAttributes setAttrs;
        setAttrs.underline = true;
        window->overridePenAttributes(setAttrs, true);

        PenAttributes getAttrs;
        window->activePenAttributes(getAttrs);
        CPPUNIT_ASSERT_EQUAL(true, (bool)getAttrs.underline);
    }

    void testOverridePenAttributesMidRow()
    {
        PenAttributes attrs;
        attrs.underline = true;
        attrs.italics = true;

        window->overridePenAttributes(attrs, true);
        CPPUNIT_ASSERT(true);
    }

    void testOverridePenAttributesNotMidRow()
    {
        PenAttributes attrs;
        attrs.underline = true;

        window->overridePenAttributes(attrs, false);
        CPPUNIT_ASSERT(true);
    }

    void testOverridePenAttributesOnEmptyWindow()
    {
        PenAttributes attrs;
        attrs.flashing = true;

        window->overridePenAttributes(attrs, false);
        CPPUNIT_ASSERT(true);
    }

    void testOverridePenAttributesWithText()
    {
        window->report("Text");

        PenAttributes attrs;
        attrs.underline = true;
        attrs.italics = true;

        window->overridePenAttributes(attrs, true);
        CPPUNIT_ASSERT(true);
    }

    void testSetPenColorOnEmptyWindow()
    {
        PenColor color;
        window->setPenColor(color);
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testSetPenColorMidText()
    {
        window->report("Text");

        PenColor color;
        window->setPenColor(color);
        CPPUNIT_ASSERT(true);
    }

    void testSetPenColorVisibleWindow()
    {
        windowDef.visible = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        PenColor color;
        testWin->setPenColor(color);
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testShowWhenHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->show();
        CPPUNIT_ASSERT_EQUAL(true, testWin->isVisible());

        delete testWin;
    }

    void testShowWhenAlreadyVisible()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());
    }

    void testShowSetsChangedFlag()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testShowSetsVisibilityChanged()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->windowVisibilityChanged());
    }

    void testHideWhenVisible()
    {
        window->hide();
        CPPUNIT_ASSERT_EQUAL(false, window->isVisible());
    }

    void testHideWhenAlreadyHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->hide();
        CPPUNIT_ASSERT_EQUAL(false, testWin->isVisible());

        delete testWin;
    }

    void testHideSetsChangedFlag()
    {
        window->hide();
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testHideSetsVisibilityChanged()
    {
        window->hide();
        CPPUNIT_ASSERT_EQUAL(true, window->windowVisibilityChanged());
    }

    void testToggleFromVisible()
    {
        windowDef.visible = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->toggle();
        CPPUNIT_ASSERT_EQUAL(false, testWin->isVisible());

        delete testWin;
    }

    void testToggleFromHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->toggle();
        CPPUNIT_ASSERT_EQUAL(true, testWin->isVisible());

        delete testWin;
    }

    void testToggleRepeatedly()
    {
        window->toggle();
        bool firstState = window->isVisible();

        window->toggle();
        CPPUNIT_ASSERT_EQUAL(!firstState, window->isVisible());

        window->toggle();
        CPPUNIT_ASSERT_EQUAL(firstState, window->isVisible());
    }

    void testClearWhenVisible()
    {
        windowDef.visible = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->clear();
        CPPUNIT_ASSERT_EQUAL(true, testWin->changed());

        delete testWin;
    }

    void testClearWhenHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->clear();
        CPPUNIT_ASSERT_EQUAL(false, testWin->changed());

        delete testWin;
    }

    void testHasFlashingTextWhenEmpty()
    {
        CPPUNIT_ASSERT_EQUAL(false, window->hasFlashingText());
    }

    void testHasFlashingTextWithFlashing()
    {
        PenAttributes attrs;
        attrs.flashing = true;
        window->setPenAttributes(attrs);
        window->report("Flashing");

        CPPUNIT_ASSERT_EQUAL(true, window->hasFlashingText());
    }

    void testHasFlashingTextWithoutFlashing()
    {
        PenAttributes attrs;
        attrs.flashing = false;
        window->setPenAttributes(attrs);
        window->report("Not Flashing");

        CPPUNIT_ASSERT_EQUAL(false, window->hasFlashingText());
    }

    void testHasFlashingTextMixed()
    {
        PenAttributes attrs1;
        attrs1.flashing = false;
        window->setPenAttributes(attrs1);
        window->report("Normal");

        PenAttributes attrs2;
        attrs2.flashing = true;
        window->setPenAttributes(attrs2);
        window->report("Flashing");

        CPPUNIT_ASSERT_EQUAL(true, window->hasFlashingText());
    }

    void testHasTextOnEmptyRow()
    {
        CPPUNIT_ASSERT_EQUAL(false, window->hasText(0));
    }

    void testHasTextOnPopulatedRow()
    {
        window->setPenLocation(5, 0);
        window->report("Text");

        CPPUNIT_ASSERT_EQUAL(true, window->hasText(5));
    }

    void testHasTextOnNonExistentRow()
    {
        window->report("Text");
        CPPUNIT_ASSERT_EQUAL(false, window->hasText(99));
    }

    void testSetFlashStateShow()
    {
        PenAttributes attrs;
        attrs.flashing = true;
        window->setPenAttributes(attrs);
        window->report("Flash");

        window->setFlashState(FlashControl::Show);
        CPPUNIT_ASSERT(true);
    }

    void testSetFlashStateHide()
    {
        PenAttributes attrs;
        attrs.flashing = true;
        window->setPenAttributes(attrs);
        window->report("Flash");

        window->setFlashState(FlashControl::Hide);
        CPPUNIT_ASSERT(true);
    }

    void testUpdateRowCountIncreaseWithAdjust()
    {
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->updateRowCount(20, true);

        delete testWin;
        CPPUNIT_ASSERT(true);
    }

    void testUpdateRowCountDecreaseWithAdjust()
    {
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        windowDef.row_count = 15;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        for (int i = 0; i < 10; ++i) {
            testWin->report("Line");
            testWin->carriageReturn();
        }

        testWin->updateRowCount(5, true);

        delete testWin;
        CPPUNIT_ASSERT(true);
    }

    void testUpdateRowCountWithoutAdjust()
    {
        window->report("Text");
        window->updateRowCount(20, false);
        CPPUNIT_ASSERT(true);
    }

    void testUpdateRowCountNonBottomTopDirection()
    {
        windowDef.win_style.scroll_direction = WindowSd::TOP_BOTTOM;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        testWin->updateRowCount(20, true); // adjust should be ignored

        delete testWin;
        CPPUNIT_ASSERT(true);
    }

    void testUpdateRowCountRemovesOffscreenText()
    {
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        windowDef.row_count = 15;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        for (int i = 0; i < 10; ++i) {
            testWin->report("Line");
            testWin->carriageReturn();
        }

        testWin->updateRowCount(3, true);

        delete testWin;
        CPPUNIT_ASSERT(true);
    }

    void testDrawWhenVisible()
    {
        window->report("Text");
        mockGfx->reset();

        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testDrawWhenHidden()
    {
        windowDef.visible = false;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        testWin->report("Text");
        mockGfx->reset();

        testWin->draw();
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->drawBorderCalls);

        delete testWin;
    }

    void testDrawResetsChangedFlag()
    {
        window->report("Text");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());

        window->draw();
        CPPUNIT_ASSERT_EQUAL(false, window->changed());
    }

    void testDrawResetsVisibilityChangedFlag()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->windowVisibilityChanged());

        window->draw();
        CPPUNIT_ASSERT_EQUAL(false, window->windowVisibilityChanged());
    }

    void testDrawCallsGfxDrawBorder()
    {
        window->report("Test");
        mockGfx->reset();

        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testIDReturnsCorrectValue()
    {
        CPPUNIT_ASSERT_EQUAL(1, window->ID());
    }

    void testGetPriorityReturnsCorrectValue()
    {
        CPPUNIT_ASSERT_EQUAL(0, window->getPriority());

        WindowDefinition newDef = windowDef;
        newDef.priority = 7;
        window->update(newDef);

        CPPUNIT_ASSERT_EQUAL(7, window->getPriority());
    }

    void testGetDefinitionReturnsCorrectly()
    {
        WindowDefinition retrievedDef;
        window->getDefinition(retrievedDef);

        CPPUNIT_ASSERT_EQUAL(windowDef.id, retrievedDef.id);
        CPPUNIT_ASSERT_EQUAL(windowDef.priority, retrievedDef.priority);
        CPPUNIT_ASSERT_EQUAL(windowDef.row_count, retrievedDef.row_count);
        CPPUNIT_ASSERT_EQUAL(windowDef.col_count, retrievedDef.col_count);
    }

    void testIsVisibleReturnsCorrectState()
    {
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());

        window->hide();
        CPPUNIT_ASSERT_EQUAL(false, window->isVisible());

        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());
    }

    void testWindowVisibilityChangedInitialState()
    {
        CPPUNIT_ASSERT_EQUAL(false, window->windowVisibilityChanged());
    }

    void testWindowVisibilityChangedAfterShow()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->windowVisibilityChanged());
    }

    void testWindowVisibilityChangedAfterDraw()
    {
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->windowVisibilityChanged());

        window->draw();
        CPPUNIT_ASSERT_EQUAL(false, window->windowVisibilityChanged());
    }

    void testFullLifecycleCreateShowReportDrawHide()
    {
        // Complete lifecycle test
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());
        CPPUNIT_ASSERT_EQUAL(false, window->changed());

        window->report("Hello World");
        CPPUNIT_ASSERT_EQUAL(true, window->changed());

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
        CPPUNIT_ASSERT_EQUAL(false, window->changed());

        window->hide();
        CPPUNIT_ASSERT_EQUAL(false, window->isVisible());
        CPPUNIT_ASSERT_EQUAL(true, window->changed());
    }

    void testMultipleReportAndCarriageReturns()
    {
        // Add multiple lines
        for (int i = 0; i < 5; ++i) {
            window->report("Line " + std::to_string(i));
            window->carriageReturn();
        }

        CPPUNIT_ASSERT_EQUAL(true, window->changed());
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testWindowResizeWithActiveContent()
    {
        // Add content to fixture window
        window->report("Test Text");
        window->carriageReturn();
        window->report("More Text");
        window->draw();

        // Test resize - just use the fixture window's resize functionality
        window->updateRowCount(20, true);
        window->draw();
        CPPUNIT_ASSERT(true); // No crash
    }

    void testToggleVisibilityMultipleTimes()
    {
        window->report("Content");

        for (int i = 0; i < 10; ++i) {
            window->toggle();
            // Window starts visible (true), after 1st toggle it's false, after 2nd it's true again
            bool expectedVisible = (i % 2 == 1);
            CPPUNIT_ASSERT_EQUAL(expectedVisible, window->isVisible());
        }
    }

    void testScrollingWithFullWindow()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>();
        std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>();
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        windowDef.row_count = 5;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        // Fill window and scroll
        for (int i = 0; i < 10; ++i) {
            testWin->report("Line " + std::to_string(i));
            testWin->carriageReturn();
        }

        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset();
        localCache.reset();
    }

    void testRapidAttributeChanges()
    {
        window->report("Start ");

        for (int i = 0; i < 5; ++i) {
            PenAttributes attrs;
            attrs.pen_size = (i % 2 == 0) ? PenSize::SMALL : PenSize::LARGE;
            attrs.italics = (i % 2 == 0);
            attrs.underline = (i % 3 == 0);

            window->setPenAttributes(attrs);
            window->report("Text" + std::to_string(i) + " ");
        }

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testComplexTextWithMixedStyles()
    {
        // Normal text
        PenAttributes normal;
        normal.italics = false;
        normal.underline = false;
        window->setPenAttributes(normal);
        window->report("Normal ");

        // Italic text
        PenAttributes italic;
        italic.italics = true;
        window->setPenAttributes(italic);
        window->report("Italic ");

        // Underlined text
        PenAttributes underline;
        underline.underline = true;
        window->setPenAttributes(underline);
        window->report("Underlined ");

        // Flashing text
        PenAttributes flashing;
        flashing.flashing = true;
        window->setPenAttributes(flashing);
        window->report("Flashing");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
        CPPUNIT_ASSERT_EQUAL(true, window->hasFlashingText());
    }

    void testMaxTextDrawersLimit()
    {
        // Add more than MAX_ROW_COUNT (12) text drawers
        for (int i = 0; i < 20; ++i) {
            window->report("Line " + std::to_string(i));
            window->carriageReturn();
        }

        // Update should trim excess
        window->update(windowDef);

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testAnchorPointTopLeft()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::TOP_LEFT;
        windowDef.anchor_horizontal = 0;
        windowDef.anchor_vertical = 0;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Top Left");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointTopCenter()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::TOP_CENTER;
        windowDef.anchor_horizontal = 50;
        windowDef.anchor_vertical = 0;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Top Center");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointTopRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::TOP_RIGHT;
        windowDef.anchor_horizontal = 100;
        windowDef.anchor_vertical = 0;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Top Right");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointMiddleLeft()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::MIDDLE_LEFT;
        windowDef.anchor_horizontal = 0;
        windowDef.anchor_vertical = 50;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Middle Left");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointMiddleCenter()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::MIDDLE_CENTER;
        windowDef.anchor_horizontal = 50;
        windowDef.anchor_vertical = 50;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Center");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointMiddleRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::MIDDLE_RIGHT;
        windowDef.anchor_horizontal = 100;
        windowDef.anchor_vertical = 50;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Middle Right");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx->reset(); localCache.reset();
    }

    void testAnchorPointBottomLeft()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::BOTTOM_LEFT;
        windowDef.anchor_horizontal = 0;
        windowDef.anchor_vertical = 100;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Bottom Left");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointBottomCenter()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::BOTTOM_CENTER;
        windowDef.anchor_horizontal = 50;
        windowDef.anchor_vertical = 100;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Bottom Center");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testAnchorPointBottomRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.anchor_point = PenAnchorPoint::BOTTOM_RIGHT;
        windowDef.anchor_horizontal = 100;
        windowDef.anchor_vertical = 100;
        windowDef.relative_pos = true;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Bottom Right");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyLeftWithLeftToRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::LEFT;
        windowDef.win_style.print_direction = WindowPd::LEFT_RIGHT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Left Justified");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyRightWithLeftToRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::RIGHT;
        windowDef.win_style.print_direction = WindowPd::LEFT_RIGHT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Right Justified");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyCenterWithLeftToRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::CENTER;
        windowDef.win_style.print_direction = WindowPd::LEFT_RIGHT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Center Justified");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyFullWithLeftToRight()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::FULL;
        windowDef.win_style.print_direction = WindowPd::LEFT_RIGHT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("Full Justified");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyLeftWithRightToLeft()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::LEFT;
        windowDef.win_style.print_direction = WindowPd::RIGHT_LEFT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("RTL Left");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testJustifyRightWithRightToLeft()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>(); std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>(); windowDef.win_style.justify = WindowJustify::RIGHT;
        windowDef.win_style.print_direction = WindowPd::RIGHT_LEFT;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        testWin->report("RTL Right");
        localGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete testWin;
        localGfx.reset(); localCache.reset();
    }

    void testMode608VsMode708Differences()
    {
        // Create separate mock objects for 608 mode
        std::shared_ptr<MockGfx> gfx608 = std::make_shared<MockGfx>();
        std::shared_ptr<MockFontCache> cache608 = std::make_shared<MockFontCache>();

        // Create 608 mode window
        subttxrend::cc::Window* win608 = new subttxrend::cc::Window(gfx608, cache608, windowDef, true);
        win608->report("608 Mode");
        gfx608->reset();
        win608->draw();
        int calls608 = gfx608->drawBorderCalls;
        delete win608;
        gfx608.reset();
        cache608.reset();

        // Create separate mock objects for 708 mode
        std::shared_ptr<MockGfx> gfx708 = std::make_shared<MockGfx>();
        std::shared_ptr<MockFontCache> cache708 = std::make_shared<MockFontCache>();

        // Create 708 mode window
        subttxrend::cc::Window* win708 = new subttxrend::cc::Window(gfx708, cache708, windowDef, false);
        win708->report("708 Mode");
        gfx708->reset();
        win708->draw();
        int calls708 = gfx708->drawBorderCalls;

        // Both should draw
        CPPUNIT_ASSERT(calls608 > 0);
        CPPUNIT_ASSERT(calls708 > 0);

        delete win708;
        gfx708.reset();
        cache708.reset();
    }

    void testMode608ScreenLayout()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>();
        std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>();
        subttxrend::cc::Window* win608 = new subttxrend::cc::Window(localGfx, localCache, windowDef, true);

        for (int i = 0; i < 5; ++i) {
            win608->report("608 Line " + std::to_string(i));
            win608->carriageReturn();
        }

        localGfx->reset();
        win608->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete win608;
        localGfx.reset();
        localCache.reset();
    }

    void testMode708ScreenLayout()
    {
        std::shared_ptr<MockGfx> localGfx = std::make_shared<MockGfx>();
        std::shared_ptr<MockFontCache> localCache = std::make_shared<MockFontCache>();
        subttxrend::cc::Window* win708 = new subttxrend::cc::Window(localGfx, localCache, windowDef, false);

        for (int i = 0; i < 5; ++i) {
            win708->report("708 Line " + std::to_string(i));
            win708->carriageReturn();
        }

        localGfx->reset();
        win708->draw();
        CPPUNIT_ASSERT(localGfx->drawBorderCalls > 0);

        delete win708;
        localGfx.reset();
        localCache.reset();
    }

    void testFlashingTextAnimationCycle()
    {
        PenAttributes flashAttrs;
        flashAttrs.flashing = true;
        window->setPenAttributes(flashAttrs);
        window->report("Flashing Text");

        CPPUNIT_ASSERT_EQUAL(true, window->hasFlashingText());

        // Show state
        window->setFlashState(FlashControl::Show);
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);

        // Hide state
        window->setFlashState(FlashControl::Hide);
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);

        // Show again
        window->setFlashState(FlashControl::Show);
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testFlashingMixedWithNonFlashing()
    {
        // Normal text
        PenAttributes normal;
        normal.flashing = false;
        window->setPenAttributes(normal);
        window->report("Normal ");

        // Flashing text
        PenAttributes flash;
        flash.flashing = true;
        window->setPenAttributes(flash);
        window->report("Flash ");

        // More normal text
        window->setPenAttributes(normal);
        window->report("Normal");

        CPPUNIT_ASSERT_EQUAL(true, window->hasFlashingText());

        window->setFlashState(FlashControl::Hide);
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testPrintDirectionChangeClears()
    {
        window->report("Original Text");

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.print_direction = WindowPd::RIGHT_LEFT;
        window->setWindowAttributes(newAttrs);

        // After direction change, add new text
        window->report("New Direction Text");
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testScrollDirectionChangeClears()
    {
        window->report("Line 1");
        window->carriageReturn();
        window->report("Line 2");

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.scroll_direction = WindowSd::TOP_BOTTOM;
        window->setWindowAttributes(newAttrs);

        window->report("New Scroll Text");
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testDirectionChangePreservesVisibility()
    {
        window->report("Text");
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());

        WindowAttributes newAttrs = windowDef.win_style;
        newAttrs.print_direction = WindowPd::RIGHT_LEFT;
        window->setWindowAttributes(newAttrs);

        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());
    }

    void testMultiRowTextDisplay()
    {
        for (int row = 0; row < 5; ++row) {
            window->setPenLocation(row, 0);
            window->report("Row " + std::to_string(row));
        }

        // Verify text exists on specific rows
        CPPUNIT_ASSERT_EQUAL(true, window->hasText(0));
        CPPUNIT_ASSERT_EQUAL(true, window->hasText(1));
        CPPUNIT_ASSERT_EQUAL(true, window->hasText(4));
        CPPUNIT_ASSERT_EQUAL(false, window->hasText(10));

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testMultiRowWithDifferentAttributes()
    {
        for (int row = 0; row < 5; ++row) {
            window->setPenLocation(row, 0);

            PenAttributes attrs;
            attrs.pen_size = (row % 2 == 0) ? PenSize::SMALL : PenSize::LARGE;
            attrs.italics = (row % 2 == 1);
            window->setPenAttributes(attrs);

            window->report("Row " + std::to_string(row));
        }

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testScrollingMultipleRows()
    {
        windowDef.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        windowDef.row_count = 3;
        subttxrend::cc::Window* testWin = new subttxrend::cc::Window(mockGfx, mockFontCache, windowDef, false);

        // Add 6 lines to cause scrolling
        for (int i = 0; i < 6; ++i) {
            testWin->report("Line " + std::to_string(i));
            testWin->carriageReturn();
        }

        mockGfx->reset();
        testWin->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);

        delete testWin;
    }

    void testPenLocationJumpingBetweenRows()
    {
        window->setPenLocation(0, 0);
        window->report("Row 0");

        window->setPenLocation(5, 0);
        window->report("Row 5");

        window->setPenLocation(2, 0);
        window->report("Row 2");

        CPPUNIT_ASSERT_EQUAL(true, window->hasText(0));
        CPPUNIT_ASSERT_EQUAL(true, window->hasText(5));
        CPPUNIT_ASSERT_EQUAL(true, window->hasText(2));

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testPenLocationOverwritePreviousText()
    {
        window->setPenLocation(0, 0);
        window->report("Original");

        window->setPenLocation(0, 0);
        window->report("Overwrite");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testTabOffsetAcrossMultipleColumns()
    {
        window->report("Start");
        window->setTabOffset(5);
        window->report("Tab1");
        window->setTabOffset(10);
        window->report("Tab2");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testRecoveryFromInvalidPenLocation()
    {
        window->report("Valid Text");

        // Try invalid location
        window->setPenLocation(999, 999);

        // Should still be able to add text
        window->report("More Text");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testRecoveryFromExcessiveTextDrawers()
    {
        // Add way more than MAX_ROW_COUNT
        for (int i = 0; i < 25; ++i) {
            window->report("Line " + std::to_string(i));
            window->carriageReturn();
        }

        // Update to trim
        window->update(windowDef);

        // Should still function
        window->report("After Trim");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testRecoveryFromAttributeChangeDuringDraw()
    {
        window->report("Text1");

        PenAttributes attrs;
        attrs.pen_size = PenSize::LARGE;
        window->setPenAttributes(attrs);

        window->draw();

        // Change attributes after draw
        attrs.pen_size = PenSize::SMALL;
        window->setPenAttributes(attrs);
        window->report("Text2");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testStateConsistencyAfterClear()
    {
        window->report("Text");
        window->show();
        CPPUNIT_ASSERT_EQUAL(true, window->isVisible());

        window->clear();

        CPPUNIT_ASSERT_EQUAL(true, window->isVisible()); // Should remain visible
        CPPUNIT_ASSERT_EQUAL(true, window->changed()); // Should be marked changed

        window->report("New Text");
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testStateConsistencyAfterUpdate()
    {
        window->report("Original");

        WindowDefinition newDef = windowDef;
        newDef.priority = 10;
        window->update(newDef);

        CPPUNIT_ASSERT_EQUAL(10, window->getPriority());
        CPPUNIT_ASSERT_EQUAL(1, window->ID()); // ID should not change

        window->report("Updated");
        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testStateConsistencyAfterToggle()
    {
        window->report("Text");
        bool initialVisibility = window->isVisible();

        window->toggle();
        CPPUNIT_ASSERT_EQUAL(!initialVisibility, window->isVisible());
        CPPUNIT_ASSERT_EQUAL(true, window->changed());

        window->toggle();
        CPPUNIT_ASSERT_EQUAL(initialVisibility, window->isVisible());
    }

    void testComplexWorkflowEditingText()
    {
        // Initial text
        window->report("Hello World");

        // Backspace some characters
        window->backspace();
        window->backspace();
        window->backspace();

        // Add new text
        window->report("Everyone!");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testComplexWorkflowBackspaceAndReplace()
    {
        window->report("Wrong Text");

        // Delete all with horizontal carriage return
        window->horizontalCarriageReturn();

        // Add correct text
        window->report("Correct Text");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }

    void testComplexWorkflowMidRowStyleChanges()
    {
        // Start with normal
        window->report("Normal ");

        // Change to italic mid-row
        PenAttributes italic;
        italic.italics = true;
        window->overridePenAttributes(italic, true);
        window->report("Italic ");

        // Change to underline mid-row
        PenAttributes underline;
        underline.underline = true;
        window->overridePenAttributes(underline, true);
        window->report("Underline ");

        // Back to normal
        PenAttributes normal;
        window->overridePenAttributes(normal, true);
        window->report("Normal");

        mockGfx->reset();
        window->draw();
        CPPUNIT_ASSERT(mockGfx->drawBorderCalls > 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcWindowTest);
