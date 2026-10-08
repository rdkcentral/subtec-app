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
#include "GfxTtxGridCell.hpp"

using namespace subttxrend::ttxt;

class GfxTtxGridCellTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxTtxGridCellTest);
    CPPUNIT_TEST(testConstructor_InitializesStateCorrectly);
    CPPUNIT_TEST(testConstructor_MultipleInstances);
    CPPUNIT_TEST(testClear_WithBackgroundColorZero);
    CPPUNIT_TEST(testClear_WithNonZeroBackgroundColor);
    CPPUNIT_TEST(testClear_WithMaxUint8Value);
    CPPUNIT_TEST(testClear_OnAlreadyClearedCell);
    CPPUNIT_TEST(testClear_OnCellWithModifiedState);
    CPPUNIT_TEST(testClear_HiddenUntilEnabled);
    CPPUNIT_TEST(testSetEnabled_TrueOnDisabledCell);
    CPPUNIT_TEST(testSetEnabled_FalseOnEnabledCell);
    CPPUNIT_TEST(testSetEnabled_TrueOnAlreadyEnabledCell);
    CPPUNIT_TEST(testSetEnabled_FalseOnAlreadyDisabledCell);
    CPPUNIT_TEST(testSetEnabled_TrueOnHiddenCell);
    CPPUNIT_TEST(testSetEnabled_EffectOnIsEnabled);
    CPPUNIT_TEST(testSetHidden_FalseOnVisibleCell);
    CPPUNIT_TEST(testSetHidden_TrueOnVisibleCell);
    CPPUNIT_TEST(testSetHidden_TrueOnAlreadyHiddenCell);
    CPPUNIT_TEST(testSetHidden_InteractionWithSetEnabled);
    CPPUNIT_TEST(testSetCharacter_WithNullCharacter);
    CPPUNIT_TEST(testSetCharacter_WithASCIICharacter);
    CPPUNIT_TEST(testSetCharacter_WithMaxUint16Value);
    CPPUNIT_TEST(testSetCharacter_WithUnicodeCharacter);
    CPPUNIT_TEST(testSetCharacter_WithSameCharacterTwice);
    CPPUNIT_TEST(testSetSize_MinimumValidSize);
    CPPUNIT_TEST(testSetSize_ZeroSize);
    CPPUNIT_TEST(testSetSize_MaximumSize);
    CPPUNIT_TEST(testSetSize_AsymmetricSize);
    CPPUNIT_TEST(testSetSize_WithSameValuesTwice);
    CPPUNIT_TEST(testSetSize_ChangingOnlyXMultiplier);
    CPPUNIT_TEST(testSetSize_ChangingOnlyYMultiplier);
    CPPUNIT_TEST(testSetSize_AfterClear);
    CPPUNIT_TEST(testSetColors_BothColorsZero);
    CPPUNIT_TEST(testSetColors_BothColorsMax);
    CPPUNIT_TEST(testSetColors_DifferentColors);
    CPPUNIT_TEST(testSetColors_WithSameValuesTwice);
    CPPUNIT_TEST(testSetColors_ChangingOnlyForeground);
    CPPUNIT_TEST(testSetColors_ChangingOnlyBackground);
    CPPUNIT_TEST(testSetColors_AfterClear);
    CPPUNIT_TEST(testMarkChanged_OnAlreadyDirtyCell);
    CPPUNIT_TEST(testMarkChanged_MultipleTimesConsecutively);
    CPPUNIT_TEST(testMarkChangedByColor_WithMatchingForegroundColor);
    CPPUNIT_TEST(testMarkChangedByColor_WithMatchingBackgroundColor);
    CPPUNIT_TEST(testMarkChangedByColor_WithMatchingBothColors);
    CPPUNIT_TEST(testMarkChangedByColor_WithNonMatchingColor);
    CPPUNIT_TEST(testMarkChangedByColor_OnClearedCell);
    CPPUNIT_TEST(testStartRedraw_OnDirtyVisibleCell);
    CPPUNIT_TEST(testStartRedraw_OnCleanVisibleCell);
    CPPUNIT_TEST(testStartRedraw_OnDirtyHiddenCell);
    CPPUNIT_TEST(testStartRedraw_OnHiddenCellTwice);
    CPPUNIT_TEST(testStartRedraw_CalledTwiceConsecutively);
    CPPUNIT_TEST(testStartRedraw_OnDisabledButDirtyCell);
    CPPUNIT_TEST(testStartRedraw_AfterMarkChanged);
    CPPUNIT_TEST(testStartRedraw_AfterSetEnabled);
    CPPUNIT_TEST(testIntegration_FullLifecycle);
    CPPUNIT_TEST(testIntegration_ClearRequestsRedraw);
    CPPUNIT_TEST(testIntegration_ClearResetsComparisonState);
    CPPUNIT_TEST(testIntegration_SetEnabledToggleMarksDirty);
    CPPUNIT_TEST(testIntegration_HiddenBlocksRedraw);
    CPPUNIT_TEST(testIntegration_MultiplePropertyChangesBeforeRedraw);
    CPPUNIT_TEST(testIntegration_MarkChangedOverridesIdempotency);
    CPPUNIT_TEST(testIntegration_NoChangeKeepsDirtyFalse);
    CPPUNIT_TEST(testIntegration_MultipleClearsWithDifferentValues);
    CPPUNIT_TEST(testEdgeCase_AsymmetricExtremeSize);
    CPPUNIT_TEST(testEdgeCase_MaximumContrastColors);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override {}
    void tearDown() override{}

protected:
    void testConstructor_InitializesStateCorrectly()
    {
        GfxTtxGridCell cell;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be disabled after construction",
                                    false, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be null after construction",
                                    static_cast<std::uint16_t>(0), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 1 after construction",
                                    static_cast<std::uint8_t>(1), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 1 after construction",
                                    static_cast<std::uint8_t>(1), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 0 after construction",
                                    static_cast<std::uint8_t>(0), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 0 after construction",
                                    static_cast<std::uint8_t>(0), cell.getBgColor());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Freshly constructed cell should request one redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());
    }

    void testConstructor_MultipleInstances()
    {
        GfxTtxGridCell cell1;
        GfxTtxGridCell cell2;

        cell1.setCharacter(65);
        cell1.setEnabled(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell2 should remain disabled",
                                    false, cell2.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell2 character should remain null",
                                    static_cast<std::uint16_t>(0), cell2.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell1 character should be changed",
                                    static_cast<std::uint16_t>(65), cell1.getChar());
    }

    void testClear_WithBackgroundColorZero()
    {
        GfxTtxGridCell cell;
        cell.clear(0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be disabled after clear",
                                    false, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be null after clear",
                                    static_cast<std::uint16_t>(0), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 1 after clear",
                                    static_cast<std::uint8_t>(1), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 1 after clear",
                                    static_cast<std::uint8_t>(1), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 0",
                                    static_cast<std::uint8_t>(0), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 0",
                                    static_cast<std::uint8_t>(0), cell.getBgColor());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cleared cell should request one redraw once visible",
                                    true, cell.startRedraw());
    }

    void testClear_WithNonZeroBackgroundColor()
    {
        GfxTtxGridCell cell;
        cell.clear(127);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 127",
                                    static_cast<std::uint8_t>(127), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 127",
                                    static_cast<std::uint8_t>(127), cell.getBgColor());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cleared cell should request one redraw once visible",
                                    true, cell.startRedraw());
    }

    void testClear_WithMaxUint8Value()
    {
        GfxTtxGridCell cell;
        cell.clear(255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 255",
                                    static_cast<std::uint8_t>(255), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 255",
                                    static_cast<std::uint8_t>(255), cell.getBgColor());
    }

    void testClear_OnAlreadyClearedCell()
    {
        GfxTtxGridCell cell;
        cell.clear(0);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First clear() should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.clear(0);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Second clear() should request redraw once visible",
                                    true, cell.startRedraw());
    }

    void testClear_OnCellWithModifiedState()
    {
        GfxTtxGridCell cell;
        cell.setEnabled(true);
        cell.setCharacter(123);
        cell.setSize(5, 6);
        cell.setColors(10, 20);

        cell.clear(50);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be disabled",
                                    false, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be reset",
                                    static_cast<std::uint16_t>(0), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be reset",
                                    static_cast<std::uint8_t>(1), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be reset",
                                    static_cast<std::uint8_t>(1), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 50",
                                    static_cast<std::uint8_t>(50), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 50",
                                    static_cast<std::uint8_t>(50), cell.getBgColor());
    }

    void testClear_HiddenUntilEnabled()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.clear(10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("clear() should suppress redraw until hidden state is cleared",
                                    false, cell.startRedraw());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setEnabled(false) should make a cleared cell redrawable",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());
    }

    void testSetEnabled_TrueOnDisabledCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setEnabled(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be enabled",
                                    true, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetEnabled_FalseOnEnabledCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setEnabled(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Enabling should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setEnabled(false);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be disabled",
                                    false, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetEnabled_TrueOnAlreadyEnabledCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setEnabled(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First enable should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setEnabled(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should remain enabled",
                                    true, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when no change occurs",
                                    false, cell.startRedraw());
    }

    void testSetEnabled_FalseOnAlreadyDisabledCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setEnabled(false);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should remain disabled",
                                    false, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when no change occurs",
                                    false, cell.startRedraw());
    }

    void testSetEnabled_TrueOnHiddenCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw",
                                    false, cell.startRedraw());

        cell.setEnabled(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be enabled",
                                    true, cell.isEnabled());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty (hidden cleared + enabled)",
                                    true, cell.startRedraw());
    }

    void testSetEnabled_EffectOnIsEnabled()
    {
        GfxTtxGridCell cell;

        cell.setEnabled(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("isEnabled should return true",
                                    true, cell.isEnabled());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("isEnabled should return false",
                                    false, cell.isEnabled());
    }

    void testSetHidden_FalseOnVisibleCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setHidden(false);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("setHidden(false) should suppress redraw with current implementation",
                                    false, cell.startRedraw());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setEnabled(false) should clear hidden state",
                                    true, cell.startRedraw());
    }

    void testSetHidden_TrueOnVisibleCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setHidden(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw",
                                    false, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should continue suppressing redraw",
                    false, cell.startRedraw());
    }

    void testSetHidden_TrueOnAlreadyHiddenCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw (first call)",
                                    false, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw (second call)",
                                    false, cell.startRedraw());

        cell.setHidden(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw after repeated hide",
                                    false, cell.startRedraw());
    }

    void testSetHidden_InteractionWithSetEnabled()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should suppress redraw",
                                    false, cell.startRedraw());

        cell.setEnabled(true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("setEnabled(true) should clear hidden state",
                                    true, cell.startRedraw());
    }

    void testSetCharacter_WithNullCharacter()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100); // Set to non-zero first
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting character to non-zero should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setCharacter(0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be 0",
                                    static_cast<std::uint16_t>(0), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetCharacter_WithASCIICharacter()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setCharacter(65); // 'A'

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be 65",
                                    static_cast<std::uint16_t>(65), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetCharacter_WithMaxUint16Value()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setCharacter(65535);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be 65535",
                                    static_cast<std::uint16_t>(65535), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetCharacter_WithUnicodeCharacter()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setCharacter(0x263A); // Unicode smiley

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Character should be 0x263A",
                                    static_cast<std::uint16_t>(0x263A), cell.getChar());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetCharacter_WithSameCharacterTwice()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(123);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First character set should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setCharacter(123);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when same character is set",
                                    false, cell.startRedraw());
    }

    void testSetSize_MinimumValidSize()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setSize(1, 1);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 1",
                                    static_cast<std::uint8_t>(1), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 1",
                                    static_cast<std::uint8_t>(1), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty (same as initial)",
                                    false, cell.startRedraw());
    }

    void testSetSize_ZeroSize()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setSize(0, 0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 0",
                                    static_cast<std::uint8_t>(0), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 0",
                                    static_cast<std::uint8_t>(0), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetSize_MaximumSize()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setSize(255, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 255",
                                    static_cast<std::uint8_t>(255), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 255",
                                    static_cast<std::uint8_t>(255), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetSize_AsymmetricSize()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setSize(10, 20);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 10",
                                    static_cast<std::uint8_t>(10), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 20",
                                    static_cast<std::uint8_t>(20), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetSize_WithSameValuesTwice()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setSize(5, 6);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First size set should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setSize(5, 6);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when same size is set",
                                    false, cell.startRedraw());
    }

    void testSetSize_ChangingOnlyXMultiplier()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setSize(5, 6);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First size set should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setSize(7, 6);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 7",
                                    static_cast<std::uint8_t>(7), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should remain 6",
                                    static_cast<std::uint8_t>(6), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetSize_ChangingOnlyYMultiplier()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setSize(5, 6);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First size set should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setSize(5, 8);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should remain 5",
                                    static_cast<std::uint8_t>(5), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 8",
                                    static_cast<std::uint8_t>(8), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetSize_AfterClear()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setSize(10, 10);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting size should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.clear(0);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After clear(), cell should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setSize(5, 5);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 5",
                                    static_cast<std::uint8_t>(5), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 5",
                                    static_cast<std::uint8_t>(5), cell.getYMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_BothColorsZero()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(100, 100); // Set to non-zero first
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(0, 0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 0",
                                    static_cast<std::uint8_t>(0), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 0",
                                    static_cast<std::uint8_t>(0), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_BothColorsMax()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setColors(255, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 255",
                                    static_cast<std::uint8_t>(255), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 255",
                                    static_cast<std::uint8_t>(255), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_DifferentColors()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setColors(127, 200);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 127",
                                    static_cast<std::uint8_t>(127), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 200",
                                    static_cast<std::uint8_t>(200), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_WithSameValuesTwice()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(50, 60);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(50, 60);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when same colors are set",
                                    false, cell.startRedraw());
    }

    void testSetColors_ChangingOnlyForeground()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(50, 60);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(70, 60);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 70",
                                    static_cast<std::uint8_t>(70), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should remain 60",
                                    static_cast<std::uint8_t>(60), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_ChangingOnlyBackground()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(50, 60);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("First setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(50, 80);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should remain 50",
                                    static_cast<std::uint8_t>(50), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 80",
                                    static_cast<std::uint8_t>(80), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testSetColors_AfterClear()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(100, 100);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.clear(50);
        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After clear(), cell should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(25, 75);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground color should be 25",
                                    static_cast<std::uint8_t>(25), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background color should be 75",
                                    static_cast<std::uint8_t>(75), cell.getBgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testMarkChanged_OnAlreadyDirtyCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100); // Make it dirty

        cell.markChanged();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should remain dirty",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());
    }

    void testMarkChanged_MultipleTimesConsecutively()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.markChanged();
        cell.markChanged();
        cell.markChanged();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty",
                                    true, cell.startRedraw());
    }

    void testMarkChangedByColor_WithMatchingForegroundColor()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(100, 200);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.markChangedByColor(100);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty when fg color matches",
                                    true, cell.startRedraw());
    }

    void testMarkChangedByColor_WithMatchingBackgroundColor()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(100, 200);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.markChangedByColor(200);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty when bg color matches",
                                    true, cell.startRedraw());
    }

    void testMarkChangedByColor_WithMatchingBothColors()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(150, 150);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.markChangedByColor(150);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty when color matches both fg and bg",
                                    true, cell.startRedraw());
    }

    void testMarkChangedByColor_WithNonMatchingColor()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(100, 200);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.markChangedByColor(50);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should not be dirty when color doesn't match",
                                    false, cell.startRedraw());
    }

    void testMarkChangedByColor_OnClearedCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.clear(0);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After clear(), cell should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.markChangedByColor(0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be dirty (both fg and bg are 0)",
                                    true, cell.startRedraw());
    }

    void testStartRedraw_OnDirtyVisibleCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100); // Make it dirty

        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return true for dirty visible cell",
                                    true, result);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should be clean after startRedraw",
                                    false, cell.startRedraw());
    }

    void testStartRedraw_OnCleanVisibleCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return false for clean cell",
                                    false, result);
    }

    void testStartRedraw_OnDirtyHiddenCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);

        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return false for hidden cell",
                                    false, result);
    }

    void testStartRedraw_OnHiddenCellTwice()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);

        bool first = cell.startRedraw();
        bool second = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return false for hidden cell (first call)",
                                    false, first);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return false for hidden cell (second call)",
                                    false, second);
    }

    void testStartRedraw_CalledTwiceConsecutively()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100); // Make it dirty

        bool firstResult = cell.startRedraw();
        bool secondResult = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("First call should return true",
                                    true, firstResult);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Second call should return false",
                                    false, secondResult);
    }

    void testStartRedraw_OnDisabledButDirtyCell()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setEnabled(false);
        cell.setCharacter(100); // Make it dirty

        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Disabled cells can still redraw if dirty and not hidden",
                                    true, result);
    }

    void testStartRedraw_AfterMarkChanged()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.markChanged();
        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return true after markChanged",
                                    true, result);
    }

    void testStartRedraw_AfterSetEnabled()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setEnabled(true);
        bool result = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("startRedraw should return true after setEnabled change",
                                    true, result);
    }

    void testIntegration_FullLifecycle()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setEnabled(true);
        cell.setCharacter(65);
        cell.setSize(2, 3);
        cell.setColors(10, 20);

        bool shouldRedraw = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell should need redraw after changes",
                                    true, shouldRedraw);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("All properties should be set correctly",
                                    true,
                                    cell.isEnabled() &&
                                    cell.getChar() == 65 &&
                                    cell.getXMultiplier() == 2 &&
                                    cell.getYMultiplier() == 3 &&
                                    cell.getFgColor() == 10 &&
                                    cell.getBgColor() == 20);
    }

    void testIntegration_ClearRequestsRedraw()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.clear(100);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After clear(), the cell should request redraw once visible",
                                    true, cell.startRedraw());
    }

    void testIntegration_ClearResetsComparisonState()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting character should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.clear(0);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After clear(), cell should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setCharacter(100);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Setting character to 100 after clear should mark dirty",
                                    true, cell.startRedraw());
    }

    void testIntegration_SetEnabledToggleMarksDirty()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("No change should not request redraw",
                                    false, cell.startRedraw());

        cell.setEnabled(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Enabling should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());
        cell.setEnabled(false);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Toggling enabled state should mark dirty",
                                    true, cell.startRedraw());
    }

    void testIntegration_HiddenBlocksRedraw()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setHidden(true);
        cell.setCharacter(100);
        cell.setColors(50, 60);

        bool shouldRedraw = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Hidden cell should not redraw even if dirty",
                                    false, shouldRedraw);
    }

    void testIntegration_MultiplePropertyChangesBeforeRedraw()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);

        cell.setCharacter(65);
        cell.setSize(2, 2);
        cell.setColors(10, 20);

        bool shouldRedraw = cell.startRedraw();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Multiple changes should result in one dirty flag",
                                    true, shouldRedraw);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());
    }

    void testIntegration_MarkChangedOverridesIdempotency()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setColors(50, 60);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setColors should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setColors(50, 60); // Same values, won't mark dirty
        cell.markChanged(); // Force dirty

        CPPUNIT_ASSERT_EQUAL_MESSAGE("markChanged should set dirty even with no actual change",
                                    true, cell.startRedraw());
    }

    void testIntegration_NoChangeKeepsDirtyFalse()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.setCharacter(100);
        cell.setSize(5, 5);
        cell.setColors(10, 20);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Initial changes should request redraw",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.setCharacter(100);
        cell.setSize(5, 5);
        cell.setColors(10, 20);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("No actual changes should keep dirty false",
                                    false, cell.startRedraw());
    }

    void testIntegration_MultipleClearsWithDifferentValues()
    {
        GfxTtxGridCell cell;
        makeVisibleAndClean(cell);
        cell.clear(10);

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("clear(10) should request redraw once visible",
                                    true, cell.startRedraw());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("After redraw, cell should be clean",
                                    false, cell.startRedraw());

        cell.clear(20);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Colors should change to 20",
                                    static_cast<std::uint8_t>(20), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background should also be 20",
                                    static_cast<std::uint8_t>(20), cell.getBgColor());

        cell.setEnabled(false);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("clear(20) should request redraw once visible",
                                    true, cell.startRedraw());
    }

    void testEdgeCase_AsymmetricExtremeSize()
    {
        GfxTtxGridCell cell;

        cell.setSize(1, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("X multiplier should be 1",
                                    static_cast<std::uint8_t>(1), cell.getXMultiplier());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Y multiplier should be 255",
                                    static_cast<std::uint8_t>(255), cell.getYMultiplier());
    }

    void testEdgeCase_MaximumContrastColors()
    {
        GfxTtxGridCell cell;

        cell.setColors(0, 255);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Foreground should be 0",
                                    static_cast<std::uint8_t>(0), cell.getFgColor());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Background should be 255",
                                    static_cast<std::uint8_t>(255), cell.getBgColor());
    }

private:
    void makeVisibleAndClean(GfxTtxGridCell& cell)
    {
        cell.setEnabled(false);

        if (cell.startRedraw())
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Baseline setup should consume a single pending redraw",
                                        false, cell.startRedraw());
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxTtxGridCellTest);