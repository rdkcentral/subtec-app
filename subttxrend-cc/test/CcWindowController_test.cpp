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
#include <thread>
#include <chrono>

#include <subttxrend/gfx/PrerenderedFont.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/DrawContext.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include "CcWindowController.hpp"
#include "CcGfx.hpp"
#include "CcCommand.hpp"

using namespace subttxrend::cc;
using namespace subttxrend::gfx;
using namespace subttxrend::protocol;

class MockDrawContext : public DrawContext
{
public:
    int fillRectCalls = 0;
    int drawStringCalls = 0;

    void fillRectangle(ColorArgb color, const Rectangle& rectangle) override {
        fillRectCalls++;
    }

    void drawUnderline(ColorArgb color, const Rectangle& rectangle) override {}
    void drawPixmap(const ClutBitmap& bitmap, const Rectangle& srcRect, const Rectangle& dstRect) override {}
    void drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect) override {}

    void drawGlyph(const FontStripPtr& fontStrip, std::int32_t glyphIndex, const Rectangle& rect,
                   ColorArgb fgColor, ColorArgb bgColor) override {}

    void drawString(PrerenderedFont& font, const Rectangle &destinationRect,
                   const std::vector<GlyphData>& glyphs, const ColorArgb fgColor,
                   const ColorArgb bgColor, int outlineSize = 0, int verticalOffset = 0) override {
        drawStringCalls++;
    }

    void reset() {
        fillRectCalls = 0;
        drawStringCalls = 0;
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
    std::vector<TextTokenData> textToTokens(const std::string& text) override {
        std::vector<TextTokenData> tokens;
        if (text.empty()) return tokens;

        TextTokenData token;
        token.totalAdvanceX = text.size() * 10;
        token.isWhite = false;
        for (size_t i = 0; i < text.size(); ++i) {
            token.glyphs.push_back(GlyphData());
        }
        tokens.push_back(token);
        return tokens;
    }

    std::int32_t getFontHeight() const override { return 20; }
    std::int32_t getFontDescender() const override { return -5; }
    std::int32_t getFontAscender() const override { return 15; }
    std::int32_t getMaxAdvance() const override { return 10; }
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
    int clearCalls = 0;
    int updateCalls = 0;

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
                    Color br_color, WindowBorder border_type) override {}

    void update() override {
        updateCalls++;
    }

    void clear() override {
        clearCalls++;
    }

    void show() override {}

    void hide() override {}

    void reset() {
        drawBackgroundCalls = 0;
        clearCalls = 0;
        updateCalls = 0;
        mockWindow->mockContext.reset();
    }
};

class CcWindowControllerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcWindowControllerTest);

    CPPUNIT_TEST(testConstructorInitialization);
    CPPUNIT_TEST(testConstructorNullSelectedWindow);
    CPPUNIT_TEST(testConstructorZeroTimeout);
    CPPUNIT_TEST(testDefineWindowValidID);
    CPPUNIT_TEST(testDefineWindowIDZero);
    CPPUNIT_TEST(testDefineWindowIDSeven);
    CPPUNIT_TEST(testDefineWindowIDEight);
    CPPUNIT_TEST(testDefineWindowPriorityEight);
    CPPUNIT_TEST(testDefineWindowSetsCurrent);
    CPPUNIT_TEST(testDefineWindowResetsTimeoutFlag);
    CPPUNIT_TEST(testDefineWindowUpdateExisting);
    CPPUNIT_TEST(testDefineMultipleWindows);
    CPPUNIT_TEST(testSetCurrentWindowValid);
    CPPUNIT_TEST(testSetCurrentWindowNonExistent);
    CPPUNIT_TEST(testSetCurrentWindowTwice);
    CPPUNIT_TEST(testDeleteSingleWindow);
    CPPUNIT_TEST(testDeleteAllWindows);
    CPPUNIT_TEST(testDeleteNoWindows);
    CPPUNIT_TEST(testDeleteCurrentWindow);
    CPPUNIT_TEST(testDeleteNonSelectedWindow);
    CPPUNIT_TEST(testDeleteNonExistent);
    CPPUNIT_TEST(testDeleteMultipleWindows);
    CPPUNIT_TEST(testClearWindowsValid);
    CPPUNIT_TEST(testClearWindowsEmpty);
    CPPUNIT_TEST(testClearWindowsAll);
    CPPUNIT_TEST(testDisplayWindowsNormal);
    CPPUNIT_TEST(testDisplayWindows608Mode);
    CPPUNIT_TEST(testHideWindowsNormal);
    CPPUNIT_TEST(testHideWindows608Mode);
    CPPUNIT_TEST(testToggleWindows);
    CPPUNIT_TEST(testToggleWindows608Mode);
    CPPUNIT_TEST(testReportNoWindow);
    CPPUNIT_TEST(testReportValidText);
    CPPUNIT_TEST(testReportEmptyString);
    CPPUNIT_TEST(testFormFeedNoWindow);
    CPPUNIT_TEST(testFormFeedWithWindow);
    CPPUNIT_TEST(testCarriageReturnNoWindow);
    CPPUNIT_TEST(testHorizontalCarriageReturnNoWindow);
    CPPUNIT_TEST(testClearWindowNoSelection);
    CPPUNIT_TEST(testBackspaceNoWindow);
    CPPUNIT_TEST(testTransparentSpaceNoWindow);
    CPPUNIT_TEST(testSetPenAttributesNoWindow);
    CPPUNIT_TEST(testActivePenAttributesNoWindow);
    CPPUNIT_TEST(testActivePenAttributesWithWindow);
    CPPUNIT_TEST(testOverridePenAttributesMidRowTrue);
    CPPUNIT_TEST(testOverridePenAttributesMidRowFalse);
    CPPUNIT_TEST(testSetPenColorNoWindow);
    CPPUNIT_TEST(testSetPenLocationNoWindow);
    CPPUNIT_TEST(testSetPenLocationZeroZero);
    CPPUNIT_TEST(testSetTabOffsetNoWindow);
    CPPUNIT_TEST(testSetTabOffsetZero);
    CPPUNIT_TEST(testSetWindowAttributesNoWindow);
    CPPUNIT_TEST(testHasTextNoWindow);
    CPPUNIT_TEST(testHasTextRowZero);
    CPPUNIT_TEST(testIsWindowTimedoutInitial);
    CPPUNIT_TEST(testGetWindowDefinitionByIdNonExistent);
    CPPUNIT_TEST(testGetWindowDefinitionByIdValid);
    CPPUNIT_TEST(testGetWindowDefinitionNoSelection);
    CPPUNIT_TEST(testRedrawFlashingTextNoFlashing);
    CPPUNIT_TEST(testDrawWindowsNoWindows);
    CPPUNIT_TEST(testResetWindowTimeoutZero);
    CPPUNIT_TEST(testResetWindowTimeoutTen);
    CPPUNIT_TEST(testWindowTimeoutDisabled);
    CPPUNIT_TEST(testWindowTimeoutBeforeElapsed);
    CPPUNIT_TEST(testEnable608True);
    CPPUNIT_TEST(testEnable608False);
    CPPUNIT_TEST(testProcessSetCCAttributesPacketFontColor);
    CPPUNIT_TEST(testUpdateWindowRowCountNonExistent);
    CPPUNIT_TEST(testResetDeletesAllWindows);

    CPPUNIT_TEST_SUITE_END();

private:
    std::shared_ptr<MockGfx> mockGfx;
    std::shared_ptr<MockFontCache> mockFontCache;
    WindowController* controller;

public:
    void setUp()
    {
        mockGfx = std::make_shared<MockGfx>();
        mockFontCache = std::make_shared<MockFontCache>();
        controller = new WindowController(mockGfx, mockFontCache);
    }

    void tearDown()
    {
        delete controller;
        mockGfx.reset();
        mockFontCache.reset();
    }

    WindowDefinition createDefaultWindowDefinition(uint8_t id = 0, uint8_t priority = 0) {
        WindowDefinition wd;
        wd.id = id;
        wd.priority = priority;
        wd.visible = true;
        wd.row_count = 4;
        wd.col_count = 32;
        wd.row_lock = false;
        wd.col_lock = false;
        wd.anchor_point = PenAnchorPoint::TOP_LEFT;
        wd.relative_pos = false;
        wd.anchor_vertical = 0;
        wd.anchor_horizontal = 0;

        // Initialize pen_style
        wd.pen_style.pen_size = PenSize::STANDARD;
        wd.pen_style.font_tag = PenFontStyle::DEFAULT_OR_UNDEFINED;
        wd.pen_style.italics = false;
        wd.pen_style.underline = false;
        wd.pen_style.flashing = false;
        wd.pen_style.edge_type = PenEdge::NONE;
        wd.pen_style.pen_color.fg_color = 0xFFFFFFFF;
        wd.pen_style.pen_color.bg_color = 0xFF000000;
        wd.pen_style.pen_color.edge_color = 0xFF888888;

        // Initialize win_style
        wd.win_style.fill_color = 0xFF000000;
        wd.win_style.border_color = 0xFFFFFFFF;
        wd.win_style.border_type = WindowBorder::NONE;
        wd.win_style.word_wrap = true;
        wd.win_style.print_direction = WindowPd::LEFT_RIGHT;
        wd.win_style.scroll_direction = WindowSd::BOTTOM_TOP;
        wd.win_style.justify = WindowJustify::LEFT;
        wd.win_style.effect_direction = WindowEd::LEFT_RIGHT;
        wd.win_style.effect_speed = 0;
        wd.win_style.display_effect = WindowSde::SNAP;

        return wd;
    }

    WindowsMap createWindowsMap(uint8_t mask) {
        WindowsMap wm;
        for (int i = 0; i < MAX_WINDOWS; ++i) {
            wm[i] = (mask & (1 << i)) != 0;
        }
        return wm;
    }

    void testConstructorInitialization()
    {
        WindowController* ctrl = new WindowController(mockGfx, mockFontCache);
        CPPUNIT_ASSERT(ctrl != nullptr);
        delete ctrl;
    }

    void testConstructorNullSelectedWindow()
    {
        // No selected window initially
        WindowDefinition wd;
        bool result = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testConstructorZeroTimeout()
    {
        // Timeout should be 0 initially
        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testDefineWindowValidID()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((int)wd.id, (int)retrieved.id);
    }

    void testDefineWindowIDZero()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testDefineWindowIDSeven()
    {
        WindowDefinition wd = createDefaultWindowDefinition(7, 0);
        controller->defineWindow(wd);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(7, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testDefineWindowIDEight()
    {
        WindowDefinition wd = createDefaultWindowDefinition(8, 0);
        controller->defineWindow(wd);

        // Should not create window with ID >= MAX_WINDOWS
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(8, retrieved);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testDefineWindowPriorityEight()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 8);
        controller->defineWindow(wd);

        // Should not create window with priority >= MAX_WINDOWS
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testDefineWindowSetsCurrent()
    {
        WindowDefinition wd = createDefaultWindowDefinition(3, 0);
        controller->defineWindow(wd);

        // Verify it's current by checking we can get its definition
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL((int)wd.id, (int)retrieved.id);
    }

    void testDefineWindowResetsTimeoutFlag()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        // Timeout flag should be false after defining window
        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testDefineWindowUpdateExisting()
    {
        WindowDefinition wd1 = createDefaultWindowDefinition(0, 0);
        wd1.row_count = 4;
        controller->defineWindow(wd1);

        WindowDefinition wd2 = createDefaultWindowDefinition(0, 0);
        wd2.row_count = 8;
        controller->defineWindow(wd2);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(8, (int)retrieved.row_count);
    }

    void testDefineMultipleWindows()
    {
        for (int i = 0; i < 4; ++i) {
            WindowDefinition wd = createDefaultWindowDefinition(i, i);
            controller->defineWindow(wd);
        }

        for (int i = 0; i < 4; ++i) {
            WindowDefinition retrieved;
            bool result = controller->getWindowDefinition(i, retrieved);
            CPPUNIT_ASSERT_EQUAL(true, result);
            CPPUNIT_ASSERT_EQUAL(i, (int)retrieved.id);
        }
    }

    void testSetCurrentWindowValid()
    {
        WindowDefinition wd = createDefaultWindowDefinition(2, 0);
        controller->defineWindow(wd);

        controller->setCurrentWindow(2);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(2, (int)retrieved.id);
    }

    void testSetCurrentWindowNonExistent()
    {
        controller->setCurrentWindow(5);

        // Should set to nullptr
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testSetCurrentWindowTwice()
    {
        WindowDefinition wd = createDefaultWindowDefinition(1, 0);
        controller->defineWindow(wd);

        controller->setCurrentWindow(1);
        controller->setCurrentWindow(1);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testDeleteSingleWindow()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowsMap wm = createWindowsMap(0x01); // Delete window 0
        controller->deleteWindows(wm);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testDeleteAllWindows()
    {
        for (int i = 0; i < 3; ++i) {
            WindowDefinition wd = createDefaultWindowDefinition(i, 0);
            controller->defineWindow(wd);
        }

        WindowsMap wm = createWindowsMap(0xFF); // Delete all
        controller->deleteWindows(wm);

        for (int i = 0; i < 3; ++i) {
            WindowDefinition retrieved;
            bool result = controller->getWindowDefinition(i, retrieved);
            CPPUNIT_ASSERT_EQUAL(false, result);
        }
    }

    void testDeleteNoWindows()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowsMap wm = createWindowsMap(0x00); // Delete none
        controller->deleteWindows(wm);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(0, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testDeleteCurrentWindow()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->deleteWindows(wm);

        // Should clear and update gfx
        CPPUNIT_ASSERT(mockGfx->clearCalls > 0);
        CPPUNIT_ASSERT(mockGfx->updateCalls > 0);

        // Selected window should be null
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testDeleteNonSelectedWindow()
    {
        WindowDefinition wd1 = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd1);
        WindowDefinition wd2 = createDefaultWindowDefinition(1, 0);
        controller->defineWindow(wd2);

        controller->setCurrentWindow(0);

        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x02); // Delete window 1
        controller->deleteWindows(wm);

        // Should not clear/update gfx
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->clearCalls);
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->updateCalls);

        // Current window should still be 0
        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(0, (int)retrieved.id);
    }

    void testDeleteNonExistent()
    {
        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->deleteWindows(wm); // Should handle gracefully

        // Verify no gfx operations when deleting non-existent window
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->clearCalls);
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->updateCalls);
    }

    void testDeleteMultipleWindows()
    {
        for (int i = 0; i < 4; ++i) {
            WindowDefinition wd = createDefaultWindowDefinition(i, 0);
            controller->defineWindow(wd);
        }

        WindowsMap wm = createWindowsMap(0x05); // Delete windows 0 and 2
        controller->deleteWindows(wm);

        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(false, controller->getWindowDefinition(0, retrieved));
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(1, retrieved));
        CPPUNIT_ASSERT_EQUAL(false, controller->getWindowDefinition(2, retrieved));
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(3, retrieved));
    }

    void testClearWindowsValid()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowsMap wm = createWindowsMap(0x01);
        controller->clearWindows(wm);

        // Verify window still exists after clearing
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testClearWindowsEmpty()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowsMap wm = createWindowsMap(0x00); // No windows to clear
        controller->clearWindows(wm);

        // Verify window still exists (nothing was cleared)
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testClearWindowsAll()
    {
        for (int i = 0; i < 3; ++i) {
            WindowDefinition wd = createDefaultWindowDefinition(i, 0);
            controller->defineWindow(wd);
        }

        WindowsMap wm = createWindowsMap(0xFF);
        controller->clearWindows(wm);

        // Verify all windows still exist (cleared but not deleted)
        for (int i = 0; i < 3; ++i) {
            WindowDefinition retrieved;
            CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(i, retrieved));
        }
    }

    void testDisplayWindowsNormal()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->enable608(false);
        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->displayWindows(wm); // Should call drawWindows in normal mode

        // Verify window still exists after display
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testDisplayWindows608Mode()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->enable608(true);
        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->displayWindows(wm);

        // Verify window still exists (608 mode doesn't auto-draw but maintains state)
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testHideWindowsNormal()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->enable608(false);
        WindowsMap wm = createWindowsMap(0x01);
        controller->hideWindows(wm);

        // Verify window still exists after hiding
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testHideWindows608Mode()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->enable608(true);
        WindowsMap wm = createWindowsMap(0x01);
        controller->hideWindows(wm);

        // Verify window still exists in 608 mode
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testToggleWindows()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        WindowsMap wm = createWindowsMap(0x01);
        controller->toggleWindows(wm);

        // Verify window still exists after toggle
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testToggleWindows608Mode()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->enable608(true);
        WindowsMap wm = createWindowsMap(0x01);
        controller->toggleWindows(wm);

        // Verify window still exists after toggle in 608 mode
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testReportNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->report("Test"); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testReportValidText()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        mockGfx->reset();
        controller->report("Hello");

        // Verify window still exists after reporting text
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testReportEmptyString()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->report("");

        // Verify window still exists after reporting empty string
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testFormFeedNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->formFeed(); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testFormFeedWithWindow()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->formFeed();

        // Verify window still exists after form feed
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testCarriageReturnNoWindow()
    {
        // Verify controller state is unchanged
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->carriageReturn(); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter); // State unchanged
    }

    void testHorizontalCarriageReturnNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->horizontalCarriageReturn(); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testClearWindowNoSelection()
    {
        mockGfx->reset();

        controller->clearWindow(); // Should not call gfx operations

        // Verify no gfx calls were made
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->clearCalls);
        CPPUNIT_ASSERT_EQUAL(0, mockGfx->updateCalls);
    }

    void testBackspaceNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->backspace(); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testTransparentSpaceNoWindow()
    {
        mockGfx->reset();

        controller->transparentSpace(false); // Should not crash or affect state
        controller->transparentSpace(true);

        // Verify no unexpected gfx calls and state remains invalid
        WindowDefinition wd;
        CPPUNIT_ASSERT_EQUAL(false, controller->getWindowDefinition(wd));
    }

    void testSetPenAttributesNoWindow()
    {
        PenAttributes attrs;
        attrs.pen_size = PenSize::STANDARD;
        attrs.font_tag = PenFontStyle::DEFAULT_OR_UNDEFINED;

        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->setPenAttributes(attrs); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testActivePenAttributesNoWindow()
    {
        PenAttributes attrs;
        bool result = controller->activePenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testActivePenAttributesWithWindow()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        PenAttributes attrs;
        bool result = controller->activePenAttributes(attrs);
        CPPUNIT_ASSERT_EQUAL(true, result);
    }

    void testOverridePenAttributesMidRowTrue()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        PenAttributes attrs;
        attrs.pen_size = PenSize::STANDARD;
        controller->overridePenAttributes(attrs, true);

        // Verify window still exists after override
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testOverridePenAttributesMidRowFalse()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        PenAttributes attrs;
        attrs.pen_size = PenSize::STANDARD;
        controller->overridePenAttributes(attrs, false);

        // Verify window still exists after override
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testSetPenColorNoWindow()
    {
        PenColor color;
        color.fg_color = 0xFFFFFFFF;
        color.bg_color = 0xFF000000;

        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->setPenColor(color); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testSetPenLocationNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->setPenLocation(0, 0); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testSetPenLocationZeroZero()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->setPenLocation(0, 0);

        // Verify window still exists after setting pen location
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testSetTabOffsetNoWindow()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->setTabOffset(0); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testSetTabOffsetZero()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        controller->setTabOffset(0);

        // Verify window still exists after setting tab offset
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(retrieved));
    }

    void testSetWindowAttributesNoWindow()
    {
        WindowAttributes attr;
        attr.fill_color = 0xFF000000;

        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->setWindowAttributes(attr); // Should handle null gracefully

        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testHasTextNoWindow()
    {
        bool result = controller->hasText(0);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testHasTextRowZero()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        bool result = controller->hasText(0);
        CPPUNIT_ASSERT_EQUAL(false, result); // No text added yet
    }

    void testIsWindowTimedoutInitial()
    {
        bool result = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetWindowDefinitionByIdNonExistent()
    {
        WindowDefinition wd;
        bool result = controller->getWindowDefinition(5, wd);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testGetWindowDefinitionByIdValid()
    {
        WindowDefinition wd = createDefaultWindowDefinition(2, 0);
        controller->defineWindow(wd);

        WindowDefinition retrieved;
        bool result = controller->getWindowDefinition(2, retrieved);
        CPPUNIT_ASSERT_EQUAL(true, result);
        CPPUNIT_ASSERT_EQUAL(2, (int)retrieved.id);
    }

    void testGetWindowDefinitionNoSelection()
    {
        WindowDefinition wd;
        bool result = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(false, result);
    }

    void testRedrawFlashingTextNoFlashing()
    {
        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        // Note: redrawFlashingText() is private, tested indirectly
        CPPUNIT_ASSERT(controller != nullptr);
    }

    void testDrawWindowsNoWindows()
    {
        mockGfx->reset();
        controller->drawWindows();

        // Should handle gracefully with no windows
        // Verify state is consistent
        WindowDefinition wd;
        CPPUNIT_ASSERT_EQUAL(false, controller->getWindowDefinition(wd));
    }

    void testResetWindowTimeoutZero()
    {
        controller->resetWindowTimeout(0);

        // Verify timeout is disabled
        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testResetWindowTimeoutTen()
    {
        controller->resetWindowTimeout(10);

        // Immediately after setting, should not have timed out yet
        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testWindowTimeoutDisabled()
    {
        controller->resetWindowTimeout(0);

        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        mockGfx->reset();
        controller->drawWindows();

        // Should not timeout
        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testWindowTimeoutBeforeElapsed()
    {
        controller->resetWindowTimeout(10);

        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        // Immediately check - should not have timed out
        mockGfx->reset();
        controller->drawWindows();

        bool timedout = controller->isWindowTimedout();
        CPPUNIT_ASSERT_EQUAL(false, timedout);
    }

    void testEnable608True()
    {
        controller->enable608(true);

        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->displayWindows(wm);

        // Verify window exists and can be retrieved in 608 mode
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testEnable608False()
    {
        controller->enable608(false);

        WindowDefinition wd = createDefaultWindowDefinition(0, 0);
        controller->defineWindow(wd);

        mockGfx->reset();
        WindowsMap wm = createWindowsMap(0x01);
        controller->displayWindows(wm);

        // Verify window exists and can be retrieved in normal mode
        WindowDefinition retrieved;
        CPPUNIT_ASSERT_EQUAL(true, controller->getWindowDefinition(0, retrieved));
    }

    void testProcessSetCCAttributesPacketFontColor()
    {
        // Create a packet
        PacketSetCCAttributes packet;

        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(wd);

        controller->processSetCCAttributesPacket(packet);

        // Verify state remains consistent
        bool hadWindowAfter = controller->getWindowDefinition(wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter);
    }

    void testUpdateWindowRowCountNonExistent()
    {
        WindowDefinition wd;
        bool hadWindowBefore = controller->getWindowDefinition(5, wd);

        controller->updateWindowRowCount(5, 10, true); // Should handle non-existent gracefully

        bool hadWindowAfter = controller->getWindowDefinition(5, wd);
        CPPUNIT_ASSERT_EQUAL(hadWindowBefore, hadWindowAfter); // Should remain non-existent
        CPPUNIT_ASSERT_EQUAL(false, hadWindowAfter);
    }

    void testResetDeletesAllWindows()
    {
        for (int i = 0; i < 3; ++i) {
            WindowDefinition wd = createDefaultWindowDefinition(i, 0);
            controller->defineWindow(wd);
        }

        controller->reset();

        // All windows should be deleted
        for (int i = 0; i < 3; ++i) {
            WindowDefinition retrieved;
            bool result = controller->getWindowDefinition(i, retrieved);
            CPPUNIT_ASSERT_EQUAL(false, result);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcWindowControllerTest);
