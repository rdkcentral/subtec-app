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
#include "CcCommandParser.hpp"
#include "CcCommandProcessor.hpp"
#include "CcUserData.hpp"
#include <vector>
#include <string>

using namespace subttxrend::cc;

// Mock CommandProcessor for testing - implements only methods actually called by CcCommand608Parser
class MockCommandProcessor : public CommandProcessor
{
public:
    struct CallRecord {
        std::string method;
        std::vector<int> intParams;
        std::vector<bool> boolParams;
        WindowDefinition windowDef;
        PenAttributes penAttrs;
        WindowsMap windowsMap;
    };

    std::vector<CallRecord> calls;
    WindowDefinition currentWindowDef;
    PenAttributes currentPenAttrs;
    bool windowTimedout = false;
    bool hasTextFlag = false;

    // Actually used methods (called by parser)
    void carriageReturn() override {
        calls.push_back({"carriageReturn", {}, {}});
    }

    void clearWindow() override {
        calls.push_back({"clearWindow", {}, {}});
    }

    void backspace() override {
        calls.push_back({"backspace", {}, {}});
    }

    void setCurrentWindow(uint8_t id) override {
        CallRecord rec;
        rec.method = "setCurrentWindow";
        rec.intParams.push_back(id);
        calls.push_back(rec);
        currentWindowDef.id = id;
    }

    void clearWindows(WindowsMap wm) override {
        CallRecord rec;
        rec.method = "clearWindows";
        rec.windowsMap = wm;
        calls.push_back(rec);
    }

    void displayWindows(WindowsMap wm) override {
        CallRecord rec;
        rec.method = "displayWindows";
        rec.windowsMap = wm;
        calls.push_back(rec);
    }

    void hideWindows(WindowsMap wm) override {
        CallRecord rec;
        rec.method = "hideWindows";
        rec.windowsMap = wm;
        calls.push_back(rec);
    }

    void deleteWindows(WindowsMap wm) override {
        CallRecord rec;
        rec.method = "deleteWindows";
        rec.windowsMap = wm;
        calls.push_back(rec);
    }

    void setPenLocation(uint8_t row, uint8_t col) override {
        CallRecord rec;
        rec.method = "setPenLocation";
        rec.intParams.push_back(row);
        rec.intParams.push_back(col);
        calls.push_back(rec);
    }

    void setWindowAttributes(WindowAttributes attrs) override {
        calls.push_back({"setWindowAttributes", {}, {}});
    }

    void defineWindow(const WindowDefinition &wd) override {
        CallRecord rec;
        rec.method = "defineWindow";
        rec.windowDef = wd;
        calls.push_back(rec);
        currentWindowDef = wd;
    }

    bool activePenAttributes(PenAttributes &penAttributes) override {
        penAttributes = currentPenAttrs;
        return true;
    }

    void overridePenAttributes(PenAttributes penAttributes, bool midRow) override {
        CallRecord rec;
        rec.method = "overridePenAttributes";
        rec.penAttrs = penAttributes;
        rec.boolParams.push_back(midRow);
        calls.push_back(rec);
        currentPenAttrs = penAttributes;
    }

    bool getWindowDefinition(uint32_t id, WindowDefinition &wd) override {
        wd = currentWindowDef;
        wd.id = id;
        return true;
    }

    bool getWindowDefinition(WindowDefinition &wd) override {
        wd = currentWindowDef;
        return currentWindowDef.id != 255;
    }

    void setTabOffset(uint8_t offset) override {
        CallRecord rec;
        rec.method = "setTabOffset";
        rec.intParams.push_back(offset);
        calls.push_back(rec);
    }

    void resetWindowTimeout(uint32_t timeout = 0) override {
        CallRecord rec;
        rec.method = "resetWindowTimeout";
        rec.intParams.push_back(timeout);
        calls.push_back(rec);
    }

    bool hasText(int row) override {
        return hasTextFlag;
    }

    void updateWindowRowCount(uint32_t id, int rowCount, bool adjust) override {
        CallRecord rec;
        rec.method = "updateWindowRowCount";
        rec.intParams.push_back(id);
        rec.intParams.push_back(rowCount);
        rec.boolParams.push_back(adjust);
        calls.push_back(rec);
    }

    void enable608(bool enabled) override {
        CallRecord rec;
        rec.method = "enable608";
        rec.boolParams.push_back(enabled);
        calls.push_back(rec);
    }

    bool isWindowTimedout() override {
        return windowTimedout;
    }

    // Character insertion (used for text rendering)
    void report(std::string str) override {
        CallRecord rec;
        rec.method = "report";
        calls.push_back(rec);
    }

    // Unused methods (must be implemented but never called by 608 parser)
    void formFeed() override {}
    void horizontalCarriageReturn() override {}
    void transparentSpace(bool) override {}
    void toggleWindows(WindowsMap) override {}
    void reset() override {}
    void setPenAttributes(PenAttributes) override {}
    void setPenColor(PenColor) override {}

    // Helper methods for testing
    void clearCalls() {
        calls.clear();
    }

    bool wasMethodCalled(const std::string& method) const {
        for (const auto& call : calls) {
            if (call.method == method) return true;
        }
        return false;
    }

    int getCallCount(const std::string& method = "") const {
        if (method.empty()) return calls.size();
        int count = 0;
        for (const auto& call : calls) {
            if (call.method == method) count++;
        }
        return count;
    }

    MockCommandProcessor() {
        currentWindowDef.id = 255;
        currentPenAttrs = PenAttributes();
    }
};

// Helper to add parity bit to make odd parity
uint8_t addParity(uint8_t data) {
    unsigned count = 0;
    for (unsigned bit = 1; bit <= 0x80; bit <<= 1) {
        if (data & bit) count++;
    }
    return (count % 2) ? data : (data | 0x80);
}

class CcCommand608ParserTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CcCommand608ParserTest);
    CPPUNIT_TEST(testConstructorAndSetProcessor);
    CPPUNIT_TEST(testReset608ClearsState);
    CPPUNIT_TEST(testSet608EnablesProcessing);
    CPPUNIT_TEST(testProcess608Data_ChannelNone_Rejected);
    CPPUNIT_TEST(testProcess608Data_WrongCcType_Rejected);
    CPPUNIT_TEST(testProcess608Data_ParityFailData1_Rejected);
    CPPUNIT_TEST(testProcess608Data_ParityFailData2_Rejected);
    CPPUNIT_TEST(testProcess608Data_NullPacket_Ignored);
    CPPUNIT_TEST(testProcess608Data_WrongFieldForChannel_Rejected);
    CPPUNIT_TEST(testProcess608Data_WrongChannel_Rejected);
    CPPUNIT_TEST(testProcess608Data_ServiceChange_TriggersReset);
    CPPUNIT_TEST(testProcess608Data_RU2_StateChange);
    CPPUNIT_TEST(testProcess608Data_RU3_StateChange);
    CPPUNIT_TEST(testProcess608Data_RU4_StateChange);
    CPPUNIT_TEST(testProcess608Data_RCL_PopOn1Mode);
    CPPUNIT_TEST(testProcess608Data_EOC_BufferSwap);
    CPPUNIT_TEST(testProcess608Data_RDC_PaintOn1Mode);
    CPPUNIT_TEST(testProcess608Data_CR_InRollUp_Scroll);
    CPPUNIT_TEST(testProcess608Data_EDM_ClearsVisible);
    CPPUNIT_TEST(testProcess608Data_ENM_ClearsBuffer);
    CPPUNIT_TEST(testProcess608Data_BS_Backspace);
    CPPUNIT_TEST(testProcess608Data_TR_TextMode);
    CPPUNIT_TEST(testProcess608Data_RepeatedControl_Suppressed);
    CPPUNIT_TEST(testProcess608Data_PAC_Row1);
    CPPUNIT_TEST(testProcess608Data_PAC_Row15);
    CPPUNIT_TEST(testProcess608Data_PAC_Row8);
    CPPUNIT_TEST(testProcess608Data_PAC_WithIndent);
    CPPUNIT_TEST(testProcess608Data_PAC_WithUnderline);
    CPPUNIT_TEST(testProcess608Data_StandardASCII);
    CPPUNIT_TEST(testProcess608Data_SpecialChar);
    CPPUNIT_TEST(testProcess608Data_ExtendedChar);
    CPPUNIT_TEST(testProcess608Data_ControlChar_Rejected);
    CPPUNIT_TEST(testProcess608Data_NoActiveWindow_NoAction);
    CPPUNIT_TEST(testProcess608Data_MultipleChars);
    CPPUNIT_TEST(testStateTransition_IdleToRollUp2);
    CPPUNIT_TEST(testStateTransition_IdleToPopOn1);
    CPPUNIT_TEST(testStateTransition_IdleToPaintOn1);
    CPPUNIT_TEST(testStateTransition_RollUp2ToRollUp3);
    CPPUNIT_TEST(testStateTransition_RollUpToPopOn);
    CPPUNIT_TEST(testStateTransition_PopOn1ToPopOn2);
    CPPUNIT_TEST(testStateTransition_PopOn2ToPaintOn2);
    CPPUNIT_TEST(testStateTransition_TextToCaptionMode);
    CPPUNIT_TEST(testWindowManagement_RowAllocation);
    CPPUNIT_TEST(testWindowManagement_LRUReplacement);
    CPPUNIT_TEST(testWindowManagement_VisibilityToggle);
    CPPUNIT_TEST(testWindowManagement_PositionConstraints);
    CPPUNIT_TEST(testStyleAttributes_MidrowColorChange);
    CPPUNIT_TEST(testStyleAttributes_BackgroundColor);
    CPPUNIT_TEST(testStyleAttributes_UnderlineToggle);
    CPPUNIT_TEST(testE2E_CompletePopOnCycle);
    CPPUNIT_TEST(testE2E_RollupScrolling);
    CPPUNIT_TEST(testE2E_ServiceChangeMidCaption);

    CPPUNIT_TEST_SUITE_END();

private:
    MockCommandProcessor* mock;
    CommandParser* parser;

    static const uint32_t CaptionChannel1 = 1000;
    static const uint32_t CaptionChannel2 = 1001;
    static const uint32_t TextChannel1 = 1004;

public:
    void setUp()
    {
        mock = new MockCommandProcessor();
        parser = new CommandParser();
        parser->setProcessor(mock);

        // Initialize window definition to allow operations
        mock->currentWindowDef.id = 0;
        mock->currentWindowDef.row_count = 2;
    }

    void tearDown()
    {
        delete parser;
        delete mock;
    }

    void testConstructorAndSetProcessor()
    {
        CommandParser p;
        MockCommandProcessor m;
        p.setProcessor(&m);
        CPPUNIT_ASSERT(true);
    }

    void testReset608ClearsState()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->reset608();

        CPPUNIT_ASSERT(mock->wasMethodCalled("enable608"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("deleteWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("resetWindowTimeout"));
    }

    void testSet608EnablesProcessing()
    {
        mock->clearCalls();
        parser->set608();

        CPPUNIT_ASSERT(mock->wasMethodCalled("enable608"));
        CPPUNIT_ASSERT_EQUAL(1, mock->getCallCount("enable608"));
    }

    void testProcess608Data_ChannelNone_Rejected()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(0, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42));

        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_WrongCcType_Rejected()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::DTVCC_CCP_START,
                              addParity(0x41), addParity(0x42));

        CPPUNIT_ASSERT(!mock->wasMethodCalled("report"));
        CPPUNIT_ASSERT(!mock->wasMethodCalled("defineWindow"));
    }

    void testProcess608Data_ParityFailData1_Rejected()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              0xC0, addParity(0x42));

        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_ParityFailData2_Rejected()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), 0xC0);

        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_NullPacket_Ignored()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              0x80, 0x80);

        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_WrongFieldForChannel_Rejected()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_2,
                              addParity(0x41), addParity(0x42));

        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_WrongChannel_Rejected()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel2, CcData::CEA_608_FIELD_1,
                              addParity(0x15), addParity(0x25)); // RU2 channel 2

        CPPUNIT_ASSERT(!mock->wasMethodCalled("report"));
        CPPUNIT_ASSERT(!mock->wasMethodCalled("defineWindow"));
    }

    void testProcess608Data_ServiceChange_TriggersReset()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel2, CcData::CEA_608_FIELD_1,
                              addParity(0x15), addParity(0x25)); // RU2 for channel 2

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("deleteWindows"));
    }

    void testProcess608Data_RU2_StateChange()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("displayWindows"));

        bool found = false;
        for (const auto& call : mock->calls) {
            if (call.method == "defineWindow") {
                found = (call.windowDef.row_count == 2);
                break;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    void testProcess608Data_RU3_StateChange()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x26)); // RU3

        bool found = false;
        for (const auto& call : mock->calls) {
            if (call.method == "defineWindow" && call.windowDef.row_count == 3) {
                found = true;
                break;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    void testProcess608Data_RU4_StateChange()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x27)); // RU4

        bool found = false;
        for (const auto& call : mock->calls) {
            if (call.method == "defineWindow" && call.windowDef.row_count == 4) {
                found = true;
                break;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    void testProcess608Data_RCL_PopOn1Mode()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
        CPPUNIT_ASSERT(mock->getCallCount("defineWindow") >= 1);
    }

    void testProcess608Data_EOC_BufferSwap()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2F)); // EOC

        CPPUNIT_ASSERT(mock->wasMethodCalled("displayWindows"));
    }

    void testProcess608Data_RDC_PaintOn1Mode()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x29)); // RDC

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testProcess608Data_CR_InRollUp_Scroll()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2D)); // CR

        CPPUNIT_ASSERT(mock->wasMethodCalled("carriageReturn"));
    }

    void testProcess608Data_EDM_ClearsVisible()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2C)); // EDM

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
    }

    void testProcess608Data_ENM_ClearsBuffer()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL (PopOn)
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2E)); // ENM

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
    }

    void testProcess608Data_BS_Backspace()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x21)); // BS

        CPPUNIT_ASSERT(mock->wasMethodCalled("backspace"));
    }

    void testProcess608Data_TR_TextMode()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(TextChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2A)); // TR

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
        bool found = false;
        for (const auto& call : mock->calls) {
            if (call.method == "defineWindow" && call.windowDef.row_count == 8) {
                found = true;
                break;
            }
        }
        CPPUNIT_ASSERT(found);
    }

    void testProcess608Data_RepeatedControl_Suppressed()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x21)); // BS
        int count1 = mock->getCallCount("backspace");

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x21)); // BS again
        int count2 = mock->getCallCount("backspace");

        CPPUNIT_ASSERT_EQUAL(count1, count2);
    }

    void testProcess608Data_PAC_Row1()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40));

        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testProcess608Data_PAC_Row15()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x70));

        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
    }

    void testProcess608Data_PAC_Row8()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x17), addParity(0x40));

        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
    }

    void testProcess608Data_PAC_WithIndent()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x52));

        CPPUNIT_ASSERT(mock->wasMethodCalled("setPenLocation"));
    }

    void testProcess608Data_PAC_WithUnderline()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x41));

        CPPUNIT_ASSERT(mock->wasMethodCalled("overridePenAttributes"));
    }

    void testProcess608Data_StandardASCII()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42));

        CPPUNIT_ASSERT(mock->wasMethodCalled("report"));
    }

    void testProcess608Data_SpecialChar()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        // Use a known Special character code (CEA-608 Special: 0x11 0x3A..0x3F)
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x3A));

        CPPUNIT_ASSERT(mock->wasMethodCalled("report"));
    }

    void testProcess608Data_ExtendedChar()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42));
        mock->clearCalls();

        // Use a valid Extended character pair (CEA-608 Extended: 0x12 0x3A..0x3F)
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x12), addParity(0x3A));

        // Extended characters should be treated as printable
        CPPUNIT_ASSERT(mock->wasMethodCalled("report"));
    }

    void testProcess608Data_ControlChar_Rejected()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x10), addParity(0x15));

        // Control chars should be rejected: no calls recorded
        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_NoActiveWindow_NoAction()
    {
        parser->set608();
        mock->currentWindowDef.id = 255;
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42));

        // With no active window, parser should perform no operations
        CPPUNIT_ASSERT_EQUAL(0, mock->getCallCount());
    }

    void testProcess608Data_MultipleChars()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42)); // AB
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x43), addParity(0x44)); // CD

        CPPUNIT_ASSERT(mock->wasMethodCalled("report"));
        CPPUNIT_ASSERT(mock->getCallCount("report") >= 2);
    }

    void testStateTransition_IdleToRollUp2()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
    }

    void testStateTransition_IdleToPopOn1()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testStateTransition_IdleToPaintOn1()
    {
        parser->set608();
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x29)); // RDC

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testStateTransition_RollUp2ToRollUp3()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x26)); // RU3

        CPPUNIT_ASSERT(mock->wasMethodCalled("updateWindowRowCount") ||
                      mock->wasMethodCalled("defineWindow"));
    }

    void testStateTransition_RollUpToPopOn()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testStateTransition_PopOn1ToPopOn2()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2F)); // EOC

        CPPUNIT_ASSERT(mock->wasMethodCalled("displayWindows"));
    }

    void testStateTransition_PopOn2ToPaintOn2()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL -> PopOn1
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2F)); // EOC -> PopOn2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x29)); // RDC

        CPPUNIT_ASSERT(mock->getCallCount() > 0);
    }

    void testStateTransition_TextToCaptionMode()
    {
        parser->set608();
        parser->process608Data(TextChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2A)); // TR (Text)
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testWindowManagement_RowAllocation()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL (PopOn)
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // Row 1
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x60)); // Row 2

        CPPUNIT_ASSERT(mock->getCallCount("setCurrentWindow") >= 2);
    }

    void testWindowManagement_LRUReplacement()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL (PopOn)

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // Row 1
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x60)); // Row 2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x12), addParity(0x40)); // Row 3
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x12), addParity(0x60)); // Row 4
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x15), addParity(0x40)); // Row 5

        CPPUNIT_ASSERT(mock->wasMethodCalled("setCurrentWindow"));
    }

    void testWindowManagement_VisibilityToggle()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2

        CPPUNIT_ASSERT(mock->wasMethodCalled("displayWindows"));

        mock->clearCalls();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL (PopOn)

        CPPUNIT_ASSERT(mock->wasMethodCalled("hideWindows") ||
                      mock->wasMethodCalled("defineWindow"));
    }

    void testWindowManagement_PositionConstraints()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // Row 1

        CPPUNIT_ASSERT(mock->wasMethodCalled("defineWindow"));
    }

    void testStyleAttributes_MidrowColorChange()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x2E));

        CPPUNIT_ASSERT(mock->wasMethodCalled("overridePenAttributes"));
    }

    void testStyleAttributes_BackgroundColor()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x10), addParity(0x20));

        CPPUNIT_ASSERT(mock->wasMethodCalled("overridePenAttributes"));
    }

    void testStyleAttributes_UnderlineToggle()
    {
        parser->set608();
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x2F));

        CPPUNIT_ASSERT(mock->wasMethodCalled("overridePenAttributes"));
    }

    void testE2E_CompletePopOnCycle()
    {
        parser->set608();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x20)); // RCL

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC row 1

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x54), addParity(0x45)); // TE
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x53), addParity(0x54)); // ST

        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2F)); // EOC

        CPPUNIT_ASSERT(mock->wasMethodCalled("displayWindows"));
    }

    void testE2E_RollupScrolling()
    {
        parser->set608();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x70)); // PAC row 15

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x4C), addParity(0x31)); // L1

        mock->clearCalls();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2D)); // CR

        CPPUNIT_ASSERT(mock->wasMethodCalled("carriageReturn"));

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x4C), addParity(0x32)); // L2

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x2D)); // CR

        CPPUNIT_ASSERT_EQUAL(2, mock->getCallCount("carriageReturn"));
    }

    void testE2E_ServiceChangeMidCaption()
    {
        parser->set608();

        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x14), addParity(0x25)); // RU2
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x11), addParity(0x40)); // PAC
        parser->process608Data(CaptionChannel1, CcData::CEA_608_FIELD_1,
                              addParity(0x41), addParity(0x42)); // AB

        int callsBefore = mock->getCallCount();

        parser->process608Data(CaptionChannel2, CcData::CEA_608_FIELD_1,
                              addParity(0x15), addParity(0x25)); // RU2 channel 2

        CPPUNIT_ASSERT(mock->wasMethodCalled("clearWindows"));
        CPPUNIT_ASSERT(mock->wasMethodCalled("deleteWindows"));

        CPPUNIT_ASSERT(mock->getCallCount() > callsBefore);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CcCommand608ParserTest);
